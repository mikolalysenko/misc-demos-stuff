// OpenCV Optical Flow demo
// Based on the Stanford example
// Mikola Lysenko
//
// Some notes:
//	F1 - Toggle FPS
//	F3 - Cycle camera display
//
//

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
#define WINDOW_NAME		"Flow"

//Optical flow components
#define MAX_FEATURES	512
#define FLOW_ITERATIONS	20
#define FLOW_EPS		0.5f
#define FEATURE_QUALITY	0.1f
#define FEATURE_DISTANCE	0.1f
#define NOISE_CUTOFF	300.0f

//Particle stuff
#define GRAVITY			10.0f
#define MAX_PARTICLES	1024
#define GRID_X			32
#define GRID_Y			32
#define VELOCITY_SCALE	10.0f

//Base offset for frustum
#define BASE_X			0
#define BASE_Y			0
#define BASE_Z			-2

//Frustum parameters
#define NEAR		0.1
#define FAR			1000.0
#define FOCAL		1.0

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


//Open CV Stuff
CvCapture * camera;


//Optical flow
IplImage * cam_frame = NULL;
IplImage * prev_frame = NULL;
IplImage * cur_frame = NULL;
IplImage * eig_image = NULL;
IplImage * temp_image = NULL;
IplImage * pyramid1 = NULL;
IplImage * pyramid2 = NULL;

//Open GL stuff
GLuint tex_id;

//Particle stuff
struct Vec2
{
	float x, y;
};

struct Particle
{
	Vec2 p, v;
	float r, g, b;
};

Vec2 velocity_field[GRID_X][GRID_Y];
Particle particles[MAX_PARTICLES];
int num_particles = 0;

bool emit_particles = false;


//Prototypes
void resize(int, int);

void query_flow(IplImage ** img)
{
	cam_frame = cvQueryFrame(camera);
	
	//Create image
	if(*img == NULL)
	{
		CvSize frame_size;
		frame_size.height = cam_frame->height;
		frame_size.width  = cam_frame->width;
		*img = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
	}
	
	//Convert image format
	cvConvertImage(cam_frame, *img, 0);
}


/**
 * Initialize the application
 */
void init_app()
{
	//Initialize timer
	srand(time(NULL));
	
	//Create camera
	camera = cvCreateCameraCapture(CV_CAP_ANY);
	if(!camera)
	{
		printf("Webcam not found\n");
		exit(1);
	}
	
	//Allocate prev frame
	cam_frame = cvQueryFrame(camera);
	
	CvSize frame_size;
	frame_size.height = cam_frame->height;
	frame_size.width  = cam_frame->width;

	eig_image = cvCreateImage(frame_size, IPL_DEPTH_32F, 1);
	temp_image = cvCreateImage(frame_size, IPL_DEPTH_32F, 1);
	pyramid1 = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
	pyramid2 = cvCreateImage(frame_size, IPL_DEPTH_8U, 1);
	
	//Read old frame
	query_flow(&prev_frame);
	
	
	//Create display texture
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_id);
	void * pixels = calloc(640 * 480, 3);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
	
	//Create display
	resize(XRes, YRes);
}

void spawn_particle(float x, float y)
{
	if(num_particles >= MAX_PARTICLES)
		return;
	
	Particle * part = &particles[num_particles++];
	
	part->p.x = x;
	part->p.y = y;
	part->v.x = 0;
	part->v.y = 0;
	part->r = drand48();
	part->g = drand48();
	part->b = drand48();
}

/**
 * Update the application logic
 */
void update_app()
{
	if(emit_particles)
	{
		for(int i=0; i<30; i++)
			spawn_particle(rand() % cam_frame->width, rand() % cam_frame->height);
	}
	
	for(int i=0; i<num_particles; i++)
	{
		//Kill dead particles
		while(num_particles > i && 
			(particles[i].p.x < 0 || particles[i].p.x >= cam_frame->width ||
			 particles[i].p.y < 0 || particles[i].p.y >= cam_frame->height) )
		{
			particles[i] = particles[--num_particles];
		}
		
		int gx = max(0, min(GRID_X, (int)(particles[i].p.x * GRID_X / cam_frame->width)));
		int gy = max(0, min(GRID_Y, (int)(particles[i].p.y * GRID_Y / cam_frame->height)));
		
		Vec2 vel = velocity_field[gx][gy];
		
		//Update position
		particles[i].p.x += particles[i].v.x * delta_t;
		particles[i].p.y += particles[i].v.y * delta_t;
		particles[i].v.x += VELOCITY_SCALE * vel.x * delta_t;
		particles[i].v.y += (VELOCITY_SCALE * vel.y + GRAVITY) * delta_t;
	}
	
	
	//Zero out velocity field
	for(int i=0; i<GRID_X; i++)
	for(int j=0; j<GRID_Y; j++)
	{
		velocity_field[i][j].x = 0.0f;
		velocity_field[i][j].y = 0.0f;
	}
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

	glDisable(GL_DEPTH_TEST);
	
	glViewport(0, 0, XRes, YRes);
	
	//Process webcam input
	query_flow(&cur_frame);

	
	//Grab initial features
	int num_features = MAX_FEATURES;
	CvPoint2D32f prev_features[MAX_FEATURES];
	
	cvGoodFeaturesToTrack(
		prev_frame, 
		eig_image, 
		temp_image, 
		prev_features, 
		&num_features, 
		FEATURE_QUALITY,
		FEATURE_DISTANCE);
	
	//Features from previous structure
	CvPoint2D32f cur_features[MAX_FEATURES];
	char optical_flow_found_feature[MAX_FEATURES];
	float optical_flow_feature_error[MAX_FEATURES];

	CvSize optical_flow_window = cvSize(3, 3);
	
	//Set termination criteria
	CvTermCriteria optical_flow_termination_criteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 );

	cvCalcOpticalFlowPyrLK(
		prev_frame, cur_frame,
		pyramid1, pyramid2,
		prev_features, cur_features,
		num_features,
		optical_flow_window,
		5,
		optical_flow_found_feature,
		optical_flow_feature_error,
		optical_flow_termination_criteria,
		0);
	
	//Set up transform for 3d stuff
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(1, 0, 1, 0);

	//Set up model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1.0f / (float)cam_frame->width, 1.0f / (float)cam_frame->height, 1.0f);
	
	//Disable depth test
	glDisable(GL_DEPTH_TEST);

	//Draw camera frame
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glColor3f(1,1,1);
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 
		0, 0, 0, 
		cam_frame->width, cam_frame->height, 
		GL_BGR, 
		GL_UNSIGNED_BYTE, 
		cam_frame->imageData);
	
	glBegin(GL_QUADS);
	
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		
		glTexCoord2f(0, 480);
		glVertex2f(0, 480);
	
		glTexCoord2f(640, 480);
		glVertex2f(640, 480);

		glTexCoord2f(640, 0);
		glVertex2f(640, 0);

	glEnd();
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	
	
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	for(int i=0; i<num_features; i++)
	{
		if(optical_flow_found_feature[i] == 0)
			continue;

		Vec2 vel;
		vel.x = cur_features[i].x - prev_features[i].x;
		vel.y = cur_features[i].y - prev_features[i].y;

		if(sqrt(vel.x * vel.x + vel.y * vel.y) > NOISE_CUTOFF)
			continue;
		
		glVertex2f(prev_features[i].x, prev_features[i].y);
		glVertex2f(cur_features[i].x, cur_features[i].y);
		
		
		
		
		
		float x0 = (GRID_X * prev_features[i].x) / cam_frame->width;
		float y0 = (GRID_Y * prev_features[i].y) / cam_frame->height;
		
		float x1 = (GRID_X * cur_features[i].x) / cam_frame->width;
		float y1 = (GRID_Y * cur_features[i].y) / cam_frame->height;
		
		float dx = x1 - x0;
		float dt = 1.0f / dx;
		
		for(float t=0; t<1.0; t+=dt)
		{
			int gx = x0 * (1.0 - t) + x1 * t;
			int gy = y0 * (1.0 - t) + y1 * t;
			
			if(gx < 0 || gx >= GRID_X || gy < 0 || gy >= GRID_Y)
				break;
			
			velocity_field[gx][gy].x += vel.x;
			velocity_field[gx][gy].y += vel.y;
		}
	}
	glEnd();

	
	glPointSize(3);
	glBegin(GL_POINTS);
	for(int i=0; i<num_particles; i++)
	{
		Particle * part = &particles[i];
		
		glColor3f(part->r, part->g, part->b);
		glVertex2f(part->p.x, part->p.y);
	}
	glEnd();

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

	//Swap feature buffers
	IplImage * tmp_frame = cur_frame;
	cur_frame = prev_frame;
	prev_frame = tmp_frame;
	
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
			emit_particles = !emit_particles;
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
		
		case ' ':
			for(int i=0; i<30; i++)
			spawn_particle(rand() % cam_frame->width, rand() % cam_frame->height);
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

