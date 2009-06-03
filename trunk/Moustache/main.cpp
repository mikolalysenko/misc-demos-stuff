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
#define WINDOW_NAME		"Moustache"
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

//Image filenames
#define NUM_IMAGES		4
const char* IMAGES[] = 
{
	"moustache.png",
	"shoopdawoop.png",
	"vendetta.png",
	"glasses.png"
};

//Application parameters
GLint XRes			= 640;
GLint YRes			= 480;
bool show_fps		= true;
int last_clock 		= 0;
double delta_t		= 0.0;

//Open CV Stuff
CvCapture * camera;
CvHaarClassifierCascade * cascade;
CvMemStorage * storage;
IplImage * gray_image = NULL;
IplImage * small_image = NULL;

//Open GL stuff
GLuint tex_id;

//Stupid dress up effects
bool show_stache = true;
int cur_stache = 0;
GLuint stache_textures[NUM_IMAGES];
int stache_width[NUM_IMAGES];
int stache_height[NUM_IMAGES];


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
	
	//Load in moustache images
	for(int i=0; i<NUM_IMAGES; i++)
	{
		printf("initializing stache[%d]: %s\n", i, IMAGES[i]);
		
		IplImage * stache_img = cvLoadImage(IMAGES[i]);
		if(!stache_img || stache_img->depth != IPL_DEPTH_8U)
		{
			printf("Couldn't find moustache image: %s\n", IMAGES[i]);
			exit(1);
		}
		
		//Allocate texture id
		glGenTextures(1, &stache_textures[i]);

		//Store width/height
		stache_width[i] = stache_img->width;
		stache_height[i] = stache_img->height;
		
		//Convert image
		char * stache_data = (char*)malloc(sizeof(char) * 4 * stache_img->width * stache_img->height);
		
		for(int j=0; j<stache_img->height; j++)
		for(int i=0; i<stache_img->width; i++)
		{
			int ioff = 3 * (i + j * stache_img->width);
			int toff = 4 * (i + j * stache_img->width);
			
			stache_data[toff] = stache_img->imageData[ioff + 2];
			stache_data[toff + 1] = stache_img->imageData[ioff + 1];
			stache_data[toff + 2] = stache_img->imageData[ioff];
			stache_data[toff + 3] = 
				(stache_img->imageData[ioff] == stache_img->imageData[0] &&
				 stache_img->imageData[ioff + 1] == stache_img->imageData[1] &&
				 stache_img->imageData[ioff + 2] == stache_img->imageData[2]) ?
				0 : 0xFF;
		}
		
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, stache_textures[i]);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 
			0, 
			GL_RGBA, 
			stache_img->width, stache_img->height, 0, 
			GL_RGBA, 
			GL_UNSIGNED_BYTE, 
			stache_data);
		
		free(stache_data);
		cvReleaseImage(&stache_img);
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
 * Render a single frame
 */
void draw_app()
{
	int cur_clock = glutGet(GLUT_ELAPSED_TIME);
	delta_t = (double)(cur_clock - last_clock) / 1000.0;
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

	//Set up transform for 3d stuff
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(1, 0, 1, 0);

	//Set up model view matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glScalef(1.0f / (float)frame->width, 1.0f / (float)frame->height, 1.0f);
	
	//Disable depth test
	glDisable(GL_DEPTH_TEST);

	//Draw camera frame
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_id);
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
	
	//Draw features
	if(show_stache)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, stache_textures[cur_stache]);
		
		glBegin(GL_QUADS);
		for(int i=0; i<num_features; i++)
		{
			CvRect * r = (CvRect*)cvGetSeqElem(features, i);
			
			float x = r->x * VIEW_SCALE;
			float y = r->y * VIEW_SCALE;
			float h = r->height * VIEW_SCALE;
			float w = r->width * VIEW_SCALE;
			
			glTexCoord2f(0, 0);
			glVertex2f(x, y);
			
			glTexCoord2f(stache_width[cur_stache], 0);
			glVertex2f(x + w, y);
			
			glTexCoord2f(stache_width[cur_stache], stache_height[cur_stache]);
			glVertex2f(x + w, y + h);
			
			glTexCoord2f(0, stache_height[cur_stache]);
			glVertex2f(x, y + h);
		}
		glEnd();
		
		glDisable(GL_BLEND);
	}
	
	glDisable(GL_TEXTURE_RECTANGLE_ARB);

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

		case GLUT_KEY_F3:
			show_stache = !show_stache;
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
		
		case 'a':
			cur_stache = (cur_stache + 1) % NUM_IMAGES;
		break;
		
		case 'z':
			cur_stache = (cur_stache - 1 + NUM_IMAGES) % NUM_IMAGES;
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

