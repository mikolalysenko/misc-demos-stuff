//Standard include
#include <map>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <cstring>

//Timing
#include <sys/time.h>

//Project files
#include "misc.h"
#include "shader.h"
#include "framebuffer.h"
#include "kernel.h"

//Namespace aliasing
using namespace std;

//Constants
#define WINDOW_NAME		"Cells"
#define NUM_FRAME_BUFFERS	2
#define NUM_TARGETS		1
#define FONT			GLUT_BITMAP_HELVETICA_18

//Simulation parameters
#define SLIME_SOURCE		"slime.cg"
#define CELL_SOURCE		"cell.cg"
#define BACTERIA_ROWS		256
#define BACTERIA_COLS		256
#define NUM_BACTERIA		(BACTERIA_ROWS * BACTERIA_COLS)
#define BACTERIA_LENGTH		8

enum TOOL_TYPE
{
	TOOL_ERASER,
	TOOL_SLIME,
	TOOL_CHEMO,
	TOOL_BUGS
};

const char* TOOL_NAMES[] =
{
	"Eraser",
	"Slime",
	"Chemical",
	"Bacteria",
	NULL
};

typedef pair<int, int> pos_t;

//A single bacteria
struct Bacteria
{
	float x, y;
	float angle;
	float r;
};

//A bacteria body
struct BacteriaCell
{
	pos_t	body[BACTERIA_LENGTH];

	void render();
	void update(pos_t);
};


//Application globals
GLint XRes		= 640;
GLint YRes		= 480;
ScreenBuffer* screen	= NULL;
vector<FrameBuffer*> buffers(NUM_FRAME_BUFFERS);
int cur_buffer		= 0;
int mx, my, px, py;
bool drawing;


//Slime shader
Kernel * slime_kernel	= NULL;
CGparameter	cg_slime_decay,
		cg_chemo_decay,
		cg_chemo_diffuse;
float		*chemo_buffer = NULL;

//Cell shader
Kernel * 	cell_kernel = NULL;
CGparameter	cell_chem_buffer,
		cell_slime_weight,
		cell_slime_rate,
		cell_chemo_weight;
vector<FrameBuffer *> cell_buffer(2);
int 		cur_cell_buffer = 0;
Bacteria *	state_buffer = NULL;
vector<BacteriaCell *> cell_bodies(NUM_BACTERIA);


//Simulation parameters
float slime_rate	= 0.3;
float slime_decay	= 0.01;
float slime_weight	= 10.0;
float chemo_rate	= 0.4;
float chemo_decay	= 0.02;
float chemo_weight	= 1.0;
float chemo_diffuse	= 0.5;
int active_bacteria	= 10;

//Display options
bool show_chem_trails	= true;
bool show_fps		= true;
bool show_stats		= true;
TOOL_TYPE cur_tool	= TOOL_BUGS;

//Prototypes
void resize(int, int);

/**
 * Draw some text on the screen.
 */
void drawText(pos_t pos, const char* msg)
{
	glPushMatrix();
	glLoadIdentity();

	pos_t cur_pos = pos;
	for(const char* c = msg; *c != '\0'; c++)
	{
		if(*c == '\n')
		{
			cur_pos.first = pos.first;
			cur_pos.second -= 18;
			continue;
		}

		glRasterPos2i(cur_pos.first, cur_pos.second);
		glutBitmapCharacter(FONT, *c);
		cur_pos.first += glutBitmapWidth(FONT, *c);
	}

	glPopMatrix();
}

/**
 * Update frames per second
 */
void updateFPS()
{
	static int frames = 0;
	static int timebase = 0;
	static float fps = 0.0f;

	char fps_string[100];
	int time = glutGet(GLUT_ELAPSED_TIME);

	frames++;
	if(time - timebase > 1000)
	{
		fps = frames *  1000.0 / (time - timebase);
		timebase = time;
		frames = 0;
	}

	if(show_fps)
	{
		sprintf(fps_string, "FPS: %5.5f", fps);
		drawText(pos_t(10, 10), fps_string);
	}
}

void update_UI()
{
	char	buffer[1024];
	
	updateFPS();

	if(show_stats)
	{
		sprintf(buffer, 
		"Bacteria: %d\nKs: %3.3f, Ws: %3.3f, Ys: %3.3f\nKc: %3.3f, Wc: %3.3f, Yc: %3.3f, Dc: %3.3f\nTool: %s (use 1, 2, 3, 4 to switch)",
		active_bacteria,
		slime_rate, slime_weight, slime_decay,
		chemo_rate, chemo_weight, chemo_decay, chemo_diffuse,
		TOOL_NAMES[cur_tool]);

		drawText(pos_t(10, YRes - 20), buffer);
	}
}

void drawToolStuff()
{
	if(cur_tool == TOOL_BUGS)
	{
		if(active_bacteria >= NUM_BACTERIA)
			return;

		int i = active_bacteria++;	
	
		//Set body values & state values
		state_buffer[i].x = mx;
		state_buffer[i].y = my;
		state_buffer[i].angle = atan2(my - py, mx - px);
		state_buffer[i].r = randf();

		for(int j=0; j<BACTERIA_LENGTH; j++)
		{
			int px = state_buffer[i].x + (int)(cos(state_buffer[i].angle) * (float)j);
			int py = state_buffer[i].y + (int)(sin(state_buffer[i].angle) * (float)j);
			cell_bodies[i]->body[j] = pos_t((XRes + px) % XRes, (YRes + py) % YRes);
		}
	}
	else
	{
		glDisable(GL_BLEND);

		switch(cur_tool)
		{
			case TOOL_ERASER:
				glColor4f(0, 0, 0, 0);
				glLineWidth(20);
			break;

			case TOOL_SLIME:
				glColor4f(0, 10, 0, 0);
				glLineWidth(10);
			break;

			case TOOL_CHEMO:
				glColor4f(0, 0, 50, 0);
				glLineWidth(20);
			break;

			default: break;
		}


		glBegin(GL_LINES);
			glVertex2i(mx, my);
			glVertex2i(px, py);
		glEnd();
	}
}


/**
 * Render a bacteria
 */
void BacteriaCell::render()
{
	glBegin(GL_LINE_STRIP);
	for(int i=0; i<BACTERIA_LENGTH; i++)
	{
		if(body[i].first == 0 || body[i].second == 0)
			break;
		glVertex2i(body[i].first, body[i].second);
	}
	glEnd();
}

void BacteriaCell::update(pos_t head)
{
	if(head == body[0])
		return;

	for(int i=BACTERIA_LENGTH - 1; i>0; i--)
	{
		body[i] = body[i-1];
	}

	body[0] = head;
}


/**
 * Initialize sand & shaders
 */
void init_cells()
{
	//Initialize shaders
	Shader::initShaders();
	resize(XRes, YRes);

	//Initialize slime shader
	slime_kernel = new Kernel(SLIME_SOURCE, NUM_TARGETS);
	cg_slime_decay = slime_kernel->getParam("slime_decay");
	cg_chemo_decay = slime_kernel->getParam("chemo_decay");
	cg_chemo_diffuse = slime_kernel->getParam("chemo_diffuse");

	//Initialize the cell shader
	cell_kernel = new Kernel(CELL_SOURCE, 1);
	cell_chem_buffer = cell_kernel->getParam("chem_buffer");
	cell_slime_rate = cell_kernel->getParam("slime_rate");
	cell_slime_weight = cell_kernel->getParam("slime_weight");
	cell_chemo_weight = cell_kernel->getParam("chemo_weight");

	state_buffer = (Bacteria*)calloc(NUM_BACTERIA, sizeof(Bacteria));

	for(int i=0; i<NUM_BACTERIA; i++)
	{
		cell_bodies[i] = new BacteriaCell();

		//Set body values & state values
		state_buffer[i].x = rand() % XRes;
		state_buffer[i].y = rand() % YRes;
		state_buffer[i].angle = randf() * M_PI * 2.0;
		state_buffer[i].r = randf();

		for(int j=0; j<BACTERIA_LENGTH; j++)
		{
			int px = state_buffer[i].x + (int)(cos(state_buffer[i].angle) * (float)j);
			int py = state_buffer[i].y + (int)(sin(state_buffer[i].angle) * (float)j);
			cell_bodies[i]->body[j] = pos_t((XRes + px) % XRes, (YRes + py) % YRes);
		}
	}


	//Initialize GPU state buffers
	for(int i=0; i<2; i++)
	{
		cell_buffer[i] = new FrameBuffer(BACTERIA_ROWS, BACTERIA_COLS, 1);
		cell_buffer[i]->begin();
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glRasterPos2i(0, 0);
		glDrawPixels(BACTERIA_ROWS, BACTERIA_COLS, GL_RGBA, GL_FLOAT, state_buffer);
	}
}

void update_bacteria()
{
	int next_cell_buffer = cur_cell_buffer ^ 1;

	//Step 1: Initialize random variables.
	for(int i=0; i<NUM_BACTERIA; i++)
	{
		state_buffer[i].r = randf();
	}

	//   1a: copy data to gpu.
	cell_buffer[cur_cell_buffer]->begin();
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, cell_buffer[cur_cell_buffer]->texture(0));
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, BACTERIA_ROWS, BACTERIA_COLS, GL_RGBA, GL_FLOAT, state_buffer);


	//Step 2: Apply shader.
	cgGLSetTextureParameter(cell_chem_buffer, buffers[cur_cell_buffer]->texture(0));
	cgGLEnableTextureParameter(cell_chem_buffer);
	cgSetParameter1f(cell_slime_rate, slime_rate);
	cgSetParameter1f(cell_slime_weight, slime_weight);
	cgSetParameter1f(cell_chemo_weight, chemo_weight);
	cell_kernel->apply(cell_buffer[next_cell_buffer], cell_buffer[cur_cell_buffer]);

	//Step 3: Read back cell information
	cell_buffer[next_cell_buffer]->begin();
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadPixels(0, 0, BACTERIA_ROWS, BACTERIA_COLS, GL_RGBA, GL_FLOAT, (GLvoid*)state_buffer);

	//Step 4: Update bodies
	for(int i=0; i<NUM_BACTERIA; i++)
	{
		cell_bodies[i]->update(pos_t(
			(XRes + (int) state_buffer[i].x) % XRes, 
			(YRes + (int) state_buffer[i].y) % YRes));
	}

	cur_cell_buffer = next_cell_buffer;
}

void draw_cells()
{
	//Update bacteria
	update_bacteria();

	//Apply slime shader
	cgSetParameter1f(cg_slime_decay, slime_decay);
	cgSetParameter1f(cg_chemo_decay, chemo_decay);
	cgSetParameter1f(cg_chemo_diffuse, chemo_diffuse);

	int next_buffer = (cur_buffer + 1) % NUM_FRAME_BUFFERS;
	slime_kernel->apply(buffers[next_buffer], buffers[cur_buffer]);
	cur_buffer = next_buffer;

	//Add slime & chemo trails to current buffer
	buffers[cur_buffer]->begin();
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glLineWidth(1);
	

	glColor4f(0, slime_rate, chemo_rate, 0);
	for(int i=0; i<active_bacteria; i++)
	{
		cell_bodies[i]->render();
	}

	//Update user tool selections
	if(drawing)
		drawToolStuff();

	buffers[cur_buffer]->end();
	
	//Display result.
	screen->begin();
	glDisable(GL_BLEND);
	glClear(GL_COLOR_BUFFER_BIT);

	if(show_chem_trails)
	{
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, buffers[cur_buffer]->texture(0));
		screen->drawRect();
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
	}
	glLineWidth(1);
	glColor4f(1,1,1,1);
	for(int i=0; i<active_bacteria; i++)
	{
		cell_bodies[i]->render();
	}


	update_UI();

	screen->end();
}

void idle()
{
	glutPostRedisplay();
}

void handle_special_input(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_F1:
			show_fps = !show_fps;
		break;

		case GLUT_KEY_F2:
			show_stats = !show_stats;
		break;
	
		case GLUT_KEY_F3:
			show_chem_trails = !show_chem_trails;
		break;

		default:
		break;			
	}

}

void handle_input(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 27:	//Escape
			exit(1);
		break;

		case '1':
			cur_tool = TOOL_ERASER;
		break;

		case '2':
			cur_tool = TOOL_SLIME;
		break;

		case '3':
			cur_tool = TOOL_CHEMO;
		break;

		case '4':
			cur_tool = TOOL_BUGS;
		break;

		case '-':
			active_bacteria -= 50;
			if(active_bacteria < 0) active_bacteria = 0;
		break;

		case '+':
			active_bacteria += 50;
			if(active_bacteria >= NUM_BACTERIA) active_bacteria = NUM_BACTERIA;
		break;

		case 'w':
			slime_rate += 0.05;
		break;

		case 'W':
			slime_rate -= 0.05;
			if(slime_rate < 0) slime_rate = 0.001;
		break;

		case 'v':
			chemo_diffuse += 0.05;
			if(chemo_diffuse > 1.0) chemo_diffuse = 1.0;
		break;

		case 'V':
			chemo_diffuse -= 0.05;
			if(chemo_diffuse < 0) chemo_diffuse = 0;
		break;

		case 'x':
			slime_decay -= 0.005;
			if(slime_decay < 0) slime_decay = 0.0;
		break;

		case 'X':
			slime_decay += 0.005;
			if(slime_decay > 1) slime_decay = 1.0;
		break;

		case 'c':
			chemo_decay -= 0.005;
			if(chemo_decay < 0) chemo_decay = 0.0;
		break;

		case 'C':
			chemo_decay += 0.005;
			if(chemo_decay > 1.0) chemo_decay = 1.0;
		break;

		case 's':
			slime_weight += 0.5;
		break;

		case 'S':
			slime_weight -= 0.5;
			if(slime_weight < 0) slime_weight = 0;
		break;

		case 'd':
			chemo_weight += 0.5;
		break;

		case 'D':
			chemo_weight -= 0.5;
			if(chemo_weight < 0) chemo_weight = 0;
		break;

		case 'e':
			chemo_rate += 0.05;
		break;

		case 'E':
			chemo_rate -= 0.05;
			if(chemo_rate < 0) chemo_rate = 0.001;
		break;


		default:
		break;
	}
}

void handle_mouse(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON)
	if(state == GLUT_DOWN)
	{
		drawing = true;
		mx = px = x;
		my = py = YRes - y;
	}
	else
	{
		drawing = false;
	}
}

void handle_move(int x, int y)
{
	px = mx;
	py = my;
	mx = x;
	my = YRes - y;

}

void resize(int w, int h)
{
	bool resized = (w != XRes || h != YRes);

	//Set the new height
	XRes = w;
	YRes = h;

	//Possibly resize screen buffer
	if(screen != NULL && resized)
	{
		delete screen;
		screen = NULL;
	}
	if(screen == NULL)
		screen = new ScreenBuffer(XRes, YRes);

	//Reallocate frame buffers
	for(int i=0; i<NUM_FRAME_BUFFERS; i++)
	{
		if(buffers[i] != NULL && resized)
		{
			delete buffers[i];
			buffers[i] = NULL;
		}
		
		if(buffers[i] == NULL)
		{
			buffers[i] = new FrameBuffer(XRes, YRes, NUM_TARGETS);
		}
	}

	//Set up projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, XRes, 0.0, YRes);
	
	//Set up model matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, XRes, YRes);

	glDisable(GL_DEPTH_TEST);

	//Reallocate chemo buffer
	if(chemo_buffer != NULL && resized)
	{
		free(chemo_buffer);
		chemo_buffer = NULL;
	}

	if(chemo_buffer == NULL)
	{
		chemo_buffer = (float*)calloc(4 * XRes * YRes, sizeof(float));
	}
}


int main(int argc, char** argv)
{
	//Initialize glut
	glutInit(&argc, argv);

	//Handle any user arguments

	//Create main window
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(XRes, YRes);
	glutCreateWindow(WINDOW_NAME);

	//Initialize glew
	GLenum err = glewInit();
	if(GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}

	//Initialize sand
	init_cells();

	//Set up callbacks
	glutDisplayFunc(draw_cells);
	glutReshapeFunc(resize);
	glutKeyboardFunc(handle_input);
	glutSpecialFunc(handle_special_input);
	glutMouseFunc(handle_mouse);
	glutMotionFunc(handle_move);
	glutIdleFunc(idle);

	//Create any needed menus

	//Run glut and go!
	glutMainLoop();

	return 0;
}

