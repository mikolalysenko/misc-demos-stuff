//Generic system includes.  These tend to be different on OS X because Macs are retarded.
#ifndef SYS_INCLUDE_H
#define SYS_INCLUDE_H

//These names are different from system to system
#ifdef __APPLE__

#include <GLUT/glut.h>
#include <OpenGL/glext.h>


#else

#include <GL/glew.h>
#include <GL/glui.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <GL/glext.h>

#endif

#include <SDL/SDL.h>


#endif
