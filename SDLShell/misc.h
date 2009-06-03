#ifndef MISC_H
#define MISC_H

#include <map>

//Common headers
#include <cassert>

#ifdef __APPLE__
//Stupid apple directory structure...
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <SDL/sdl.h>
//#include "SDLmain.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <SDL.h>
#endif

//App vars
extern int XRes, YRes;

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

struct Color
{
	float r, g, b, a;
	Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
	Color(float r_, float g_, float b_) : r(r_), g(g_), b(b_), a(1.0f) {}
	Color(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}
};

//Load a file into memory
void * loadFile(const char* filename, size_t * size);
Color * loadBMP(const char* filename, size_t * width, size_t * height, int mask = -1);

#endif

