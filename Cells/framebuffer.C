#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cassert>

#include "misc.h"
#include "framebuffer.h"

using namespace std;

//A list of all 16 attachment points
GLenum FBO_ATTACH_POINTS[] =
{
	GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1_EXT,
	GL_COLOR_ATTACHMENT2_EXT,
	GL_COLOR_ATTACHMENT3_EXT,
	GL_COLOR_ATTACHMENT4_EXT,
	GL_COLOR_ATTACHMENT5_EXT,
	GL_COLOR_ATTACHMENT6_EXT,
	GL_COLOR_ATTACHMENT7_EXT,
	GL_COLOR_ATTACHMENT8_EXT,
	GL_COLOR_ATTACHMENT9_EXT,
	GL_COLOR_ATTACHMENT10_EXT,
	GL_COLOR_ATTACHMENT11_EXT,
	GL_COLOR_ATTACHMENT12_EXT,
	GL_COLOR_ATTACHMENT13_EXT,
	GL_COLOR_ATTACHMENT14_EXT,
	GL_COLOR_ATTACHMENT15_EXT,
};


FrameBuffer::FrameBuffer(int w, int h, int n) : width(w), height(h), num_buffers(n)
{
	//Do bounds checking
	assert(n > 0 && n < MAX_BUFFERS);
	assert(w > 0);
	assert(h > 0);

	//Make sure extensions are supported

	//Initialize frame buffer
	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

	//Generate texture and disable mip-mapping
	glGenTextures(n, tex);

	//Attach & initialize each texture
	for(int i=0; i<n; i++)
	{
		//Bind the texture
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex[i]);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 
			0,
			TEXTURE_PRECISION,
			width, height,
			0,
			GL_RGBA,
			GL_FLOAT,
			0);

		//Attach texture to frame buffer
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT + i,
			GL_TEXTURE_RECTANGLE_ARB,
			tex[i],
			0);
	}

	//Verify frame buffer was initialized properly
	if(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		fprintf(stderr, "Error initializing frame buffer");
		exit(1);
	}

	//Clear out buffer
	for(int i=0; i<n; i++)
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	//Unbind textures/frame buffer
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

FrameBuffer::~FrameBuffer()
{
	glDeleteTextures(num_buffers, tex);
	glDeleteFramebuffersEXT(1, &fbo);
}

void FrameBuffer::begin()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	glDrawBuffers(num_buffers, FBO_ATTACH_POINTS);
}

void FrameBuffer::end()
{
}

void FrameBuffer::drawRect()
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(width, 0.0f);
		glVertex2f(width, 0.0f);
		glTexCoord2f(width, height);
		glVertex2f(width, height);
		glTexCoord2f(0.0f, height);
		glVertex2f(0.0f, height);
	glEnd();
}

GLuint FrameBuffer::texture(int n)
{
	assert(n >= 0 && n < num_buffers);
	return tex[n];
}

void ScreenBuffer::begin()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDrawBuffer(GL_BACK);	
}

void ScreenBuffer::end()
{
	glutSwapBuffers();
}

void ScreenBuffer::drawRect()
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(width, 0.0f);
		glVertex2f(width, 0.0f);
		glTexCoord2f(width, height);
		glVertex2f(width, height);
		glTexCoord2f(0.0f, height);
		glVertex2f(0.0f, height);
	glEnd();
}

