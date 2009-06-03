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

//Namespace aliasing
using namespace std;

//Constants
#define WINDOW_NAME		"Tree Thing"

//Application parameters
GLint XRes			= 640;
GLint YRes			= 480;
float fov			= 45.0f;
float z_near		= 0.5f;
float z_far			= 1200.0f;
bool paused			= false;
bool show_fps		= true;
int last_clock 		= 0;
double delta_t		= 0.0;

double t = 0.0;
double theta_0 = 0.0,
		phi_0 = 20.0,
		phi_1 = 100.0,
		phi_2 = 200.0;



//Prototypes
void resize(int, int);


/**
 * Initialize the application
 */
void init_app()
{
	//Initialize timer
	srand(time(NULL));
		
	//Create display
	resize(XRes, YRes);
}


/**
 * Update the application logic
 */
void update_app()
{
	t += 1 * delta_t;
	
	theta_0 = 30 + 10.0 * cos(t);
	phi_0 = cos(1.7 * t) * 50;
	phi_1 = sin(1.9 * t) * 55 + 120;
	phi_2 = cos(1.5 * t) * 45 + 240;
}

void draw_flower()
{
	glPointSize(5);
	glBegin(GL_POINTS);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
	glEnd();
}

void draw_seg(int level)
{
	if(level == 0)
	{
		draw_flower();
		return;
	}
	
	glBegin(GL_LINES);
		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 1, 0);
	glEnd();
	
	glTranslatef(0, 1, 0);
	glScalef(0.8, 0.8, 0.8);
	
	glPushMatrix();
	glRotatef(phi_0, 0, 1, 0);
	glRotatef(theta_0, 0, 0, 1);
	draw_seg(level-1);
	glPopMatrix();
	
	glPushMatrix();
	glRotatef(phi_1, 0, 1, 0);
	glRotatef(theta_0, 0, 0, 1);
	draw_seg(level-1);
	glPopMatrix();
	
	glPushMatrix();
	glRotatef(phi_2, 0, 1, 0);
	glRotatef(theta_0, 0, 0, 1);
	draw_seg(level-1);
	glPopMatrix();
}

void draw_tree()
{
	glColor3f(0, 1, 0);
	
	glTranslatef(0, -2, -8);
	draw_seg(7);
}

/**
 * Render a single frame
 */
void draw_app()
{
	int cur_clock = glutGet(GLUT_ELAPSED_TIME);
	delta_t = (double)(cur_clock - last_clock) / 1000.0;
	if(!paused)
	{
		update_app();
	}
	last_clock = cur_clock;


	//Clear the screen
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glViewport(0, 0, XRes, YRes);
	
	//Set up view matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (double)XRes / (double)YRes, z_near, z_far);
	
	//Set up modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//Turn on culling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	//Draw the tree
	draw_tree();
	
	
	//Draw HUD overlays
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, XRes, 0, YRes);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Disable culling
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	updateFPS();

	glutSwapBuffers();
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
			paused = !paused;
		break;
		
		case GLUT_KEY_F3:
		break;
		
		case GLUT_KEY_LEFT:
		break;

		case GLUT_KEY_RIGHT:
		break;

		case GLUT_KEY_UP:
		break;

		case GLUT_KEY_DOWN:
		break;

		default:
		break;			
	}
}


void handle_special_up(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_LEFT:
		break;

		case GLUT_KEY_RIGHT:
		break;

		case GLUT_KEY_UP:
		break;

		case GLUT_KEY_DOWN:
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
		
		default:
		break;
	}
}


void handle_mouse(int button, int state, int x, int y)
{
}

void handle_move(int x, int y)
{
}

void resize(int w, int h)
{
	//Set the new height
	XRes = w;
	YRes = h;
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

	//Initialize sand
	init_app();

	//Set up callbacks
//	glutIgnoreKeyRepeat(true);
	glutDisplayFunc(draw_app);
	glutReshapeFunc(resize);
	glutKeyboardFunc(handle_input);
	glutSpecialFunc(handle_special_input);
	glutSpecialUpFunc(handle_special_up);
	glutMouseFunc(handle_mouse);
	glutMotionFunc(handle_move);
	glutIdleFunc(idle);

	//Create any needed menus

	//Run glut and go!
	glutMainLoop();

	return 0;
}

