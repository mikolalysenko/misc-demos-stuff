#ifndef MISC_H
#define MISC_H

//Common headers
#include <cassert>

//GLEW
#include <GL/glew.h>

//GLUT
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//CG
#include <Cg/cg.h>
#include <Cg/cgGL.h>

extern GLint XRes, YRes;


//Test for Open GL errors
extern void checkGLErrors(const char* label);

//Generates a random float
extern float randf();

//Compute min and max respectively
template<typename T> T min(T& a, T& b) { return (a < b) ? a : b; }
template<typename T> T max(T& a, T& b) { return (a > b) ? a : b; }

#endif

