// OpenCV Head Tracking for VR
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
#define WINDOW_NAME		"VR Test"
#define FACE_XML		"haarcascade_frontalface_alt2.xml"
#define VIEW_SCALE		2

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

enum DRAW_STATE
{
	DRAW_CAMERA,
	DRAW_CAMERA_VR,
	DRAW_VR,
	
	NUM_DRAW_STATES
};
DRAW_STATE draw_state = DRAW_CAMERA_VR;

float 	eye_x = 0,
		eye_y = 0,
		eye_z = -1;

//Open CV Stuff
CvCapture * camera;
CvHaarClassifierCascade * cascade;
CvMemStorage * storage;
IplImage * gray_image = NULL;
IplImage * small_image = NULL;

//Open GL stuff
GLuint tex_id;


//Prototypes
void resize(int, int);


/**
 * Initialize the application
 */
void init_app()
{
	//Initialize timer
	srand(time(NULL));
	
	//Allocate memory for OpenCV stuff
	storage = cvCreateMemStorage(0);
	if(!storage)
	{
		printf("Failed to initialze OpenCV storage\n");
		exit(1);
	}
	
	//Create face tracker kernel
	cascade = (CvHaarClassifierCascade*) cvLoad(FACE_XML, 0, 0, 0);
	
	if(!cascade)
	{
		printf("Failed to load face tracker\n");
		exit(1);
	}
	
	//Create camera
	camera = cvCreateCameraCapture(CV_CAP_ANY);
	if(!camera)
	{
		printf("Webcam not found\n");
		exit(1);
	}
	
	//Create display texture
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_id);
	void * pixels = calloc(640 * 480, 3);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
	
	//Create display
	resize(XRes, YRes);
}

/**
 * Update the application logic
 */
void update_app()
{	
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
	IplImage * frame = cvQueryFrame(camera);

	if(gray_image == NULL)
	{
		gray_image = cvCreateImage(
			cvSize(frame->width, frame->height),
			IPL_DEPTH_8U,
			1);
	}
	
	if(small_image == NULL)
	{
		small_image = cvCreateImage(
			cvSize(frame->width / VIEW_SCALE, frame->height / VIEW_SCALE),
			IPL_DEPTH_8U,
			1);
	}
	
	cvCvtColor(frame, gray_image, CV_BGR2GRAY);
	cvResize(gray_image, small_image, CV_INTER_LINEAR);
	
	//Perform face detection
	CvSeq* features = cvHaarDetectObjects(
		small_image, 
		cascade, 
		storage,
		1.1, 2, 
		CV_HAAR_DO_CANNY_PRUNING,
		cvSize(30,30));
	int num_features = (features ? features->total : 0);

	if(num_features >= 1)
	{
		//Spotted a face!
		CvRect * rect = (CvRect*)cvGetSeqElem(features, 0);
		
		float c_x = (rect->x + rect->width * 0.5) / small_image->width * 2.0f - 1.0f;
		float c_y = (rect->y + rect->height * 0.5) / small_image->height * 2.0f - 1.0f;
				
		float h_size = (float)rect->width / (float)small_image->width;
		float v_size = (float)rect->height / pow((float)small_image->height, 2.0f) * (float) small_image->width;
		
		//Adjust aspect ratio
		c_y *= -(float)small_image->width / (float)small_image->height;
		
		float d_atten = 1.0f / v_size;
		
		//Compute camera coordinates
		eye_x = d_atten * c_x;
		eye_y = d_atten * c_y;
		eye_z = d_atten * -1.0f;
	}
	
	if(draw_state != DRAW_CAMERA)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, (float)XRes / (float)YRes, NEAR, FAR);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		gluLookAt(
			eye_x, eye_y, eye_z,
			0, 0, 0,
			0, 1, 0);
		
		glEnable(GL_DEPTH_TEST);
		
		
		
		glPushMatrix();
		glTranslatef(0, 0, -1);
		glColor3f(1, 1, 1);
		glutWireTeapot(0.5);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(2, 0, 5);
		glColor3f(0, 1, 0);
		glutWireTorus(0.1, 1, 10, 10);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(-5, 3, 8);
		glColor3f(1, 0, 0);
		glutWireSphere(1.2, 10, 10);
		glPopMatrix();
		
		glPushMatrix();
		glColor3f(0, 0, 1);
		glTranslatef(-4, -2, 6);
		glRotatef(70, 0, 0.5, 0.5);
		glutWireCone(0.5f, 1.0f, 10, 10);
		glPopMatrix();

static float theta = 0.0f;

		glPushMatrix();
		glTranslatef(0, -3, 6);
		glRotatef(theta, 0, 0, 1);
		glTranslatef(1, 0, 0);
		glColor3f(1, 1, 0);
		glutWireSphere(0.1f, 10, 10);
		glPopMatrix();
		
		
		glPushMatrix();
		glColor3f(1, 0, 1);
		glRotatef(theta * 0.3, 0, 1, 0);
		glTranslatef(15, 0, 0);
		glRotatef(theta * 3, 0.5, 0, 0.5);
		glutWireDodecahedron();
		glPopMatrix();
		
		
		glPushMatrix();
		glColor3f(0, 1, 1);
		glTranslatef(1, 10 * cos(theta * 0.01), 15);
		glRotatef(theta, 1, 1, 1);
		glutWireCube(1.0);
		glPopMatrix();
		
		theta += 5.0f;
	}
	
	if(draw_state != DRAW_VR)
	{
		//Set up transform for 3d stuff
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(1, 0, 1, 0);

		//Set up model view matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		if(draw_state == DRAW_CAMERA_VR)
		{
			glScalef(0.15, 0.15, 1);
		}
		
		glScalef(1.0f / (float)frame->width, 1.0f / (float)frame->height, 1.0f);
		
		//Disable depth test
		glDisable(GL_DEPTH_TEST);

		//Draw camera frame
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glColor3f(1,1,1);
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, frame->width, frame->height, GL_BGR, GL_UNSIGNED_BYTE, frame->imageData);
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
		
		//Draw features
		glPointSize(10);
		glBegin(GL_POINTS);
		glColor3f(1, 0, 0);
		for(int i=0; i<num_features; i++)
		{
			CvRect * r = (CvRect*)cvGetSeqElem(features, i);
			
			float x = (r->x + r->width * 0.5) * VIEW_SCALE;
			float y = (r->y + r->height * 0.5) * VIEW_SCALE;
			
			glVertex2f(x, y);
		}
		glEnd();
	}


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
			draw_state = (DRAW_STATE)((draw_state + 1) % NUM_DRAW_STATES);
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
	glutIgnoreKeyRepeat(true);
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

