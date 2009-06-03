#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "misc.h"

#define TEXTURE_PRECISION	GL_RGBA16F_ARB
#define MAX_BUFFERS		16


/**
 * A Buffer represents a set targets for shader data.  Kernels may operate
 * on Buffers by transforming their contents.  A buffer may be read to or
 * written to depending on its use.
 */
class Buffer
{
public:
	Buffer() {}
	virtual ~Buffer() {}

	//Begin drawing to buffer.
	virtual void begin() = 0;

	//Finish drawing to buffer.
	virtual void end() = 0;

	//Draw a rectangle which fills the entire buffer.
	virtual void drawRect() = 0;
	
	//Returns the nth render target within this buffer.
	virtual GLuint texture(int n) = 0;
};

/**
 * A FrameBuffer is an off screen buffer, which may contain multiple render
 * targets.
 */
class FrameBuffer : public Buffer
{
public:
	FrameBuffer(int w, int h, int n);
	virtual ~FrameBuffer();
	
	virtual void begin();
	virtual void end();
	virtual void drawRect();

	virtual GLuint texture(int n);
	
private:
	
	GLint width;
	GLint height;
	int num_buffers;

	GLuint fbo;
	GLuint tex[MAX_BUFFERS];
};

/**
 * A ScreenBuffer contains only a single render target and may not be used
 * as a data source.
 */
class ScreenBuffer : public Buffer
{
public:
	ScreenBuffer(int w, int h) : width(w), height(h) {}
	virtual ~ScreenBuffer() {}

	virtual void begin();
	virtual void end();
	virtual void drawRect();

	virtual GLuint texture(int n) { assert(false); return 0; }

private:
	GLint width;
	GLint height;
};


#endif
