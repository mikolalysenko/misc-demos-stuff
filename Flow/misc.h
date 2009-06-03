#ifndef MISC_H
#define MISC_H

#include <map>

//Common headers
#include <cassert>

//GLUT
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenCV/OpenCV.h>
#else
#include <GL/glut.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#endif


//Config stuff
#define FONT GLUT_BITMAP_HELVETICA_18

//App vars
extern GLint XRes, YRes;

//Define a screen position
typedef std::pair<int, int> pos_t;

//App state variables
extern bool show_fps;
extern bool paused;

//Test for Open GL errors
extern void checkGLErrors(const char* label);

//Generates a random float
extern float randf();

//Compute min and max respectively
template<typename T> T min(T& a, T& b) { return (a < b) ? a : b; }
template<typename T> T max(T& a, T& b) { return (a > b) ? a : b; }

extern void drawText(pos_t pos, const char* msg);
extern void updateFPS();

struct Color
{
	float r, g, b, a;
	Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	Color(float r_, float g_, float b_) : r(r_), g(g_), b(b_), a(1.0f) {}
	Color(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}

	void set() { glColor4f(r, g, b, a); }
	static Color rand() { return Color(randf(), randf(), randf()); }
};


//Load a file into memory
void * loadFile(const char* filename, size_t * size);
Color * loadBMP(const char* filename, size_t * width, size_t * height, int mask = -1);

#endif

