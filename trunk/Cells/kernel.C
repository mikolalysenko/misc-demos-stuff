//Standard includes
#include <vector>
#include <sstream>
#include <iostream>
#include <cassert>

//Project
#include "misc.h"
#include "framebuffer.h"
#include "shader.h"
#include "kernel.h"

using namespace std;


Kernel::Kernel(const char* source, int n_buffers) 
	: shader(source), shader_buffers(n_buffers)
{
	//Bind parameters
	for(size_t i=0; i<shader_buffers.size(); i++)
	{
		stringstream bname(stringstream::in | stringstream::out);
		bname << "buffer" << i ;
		shader_buffers[i] = shader.getParam(bname.str().c_str());	
	}
}

Kernel::~Kernel()
{
	//Release parameters?
}

void Kernel::apply(Buffer *dest, Buffer *src)
{
	//Begin writing to destination
	dest->begin();
	shader.begin();

	//Initialize parameters & clear buffers
	for(size_t i=0; i<shader_buffers.size(); i++)
	{
		cgGLSetTextureParameter(shader_buffers[i], src->texture(i));
		cgGLEnableTextureParameter(shader_buffers[i]);	
	}

	//Execute the shader
	dest->drawRect();

	//Finish up
	shader.end();
	dest->end();
}
