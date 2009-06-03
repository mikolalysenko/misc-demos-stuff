#ifndef KERNEL_H
#define KERNEL_H

#include <vector>

#include "framebuffer.h"
#include "shader.h"

/**
 * A Kernel is a type of shader which only operates on buffers.  The kernel is
 * applied to each pixel within the source buffer, and stores the result into
 * each pixel within the result buffer.
 */
class Kernel
{
public:
	Kernel(const char * source, int n_buffers);
	~Kernel();

	//Executes the kernel on the source, storing the result in dest.
	void apply(Buffer *dest, Buffer *source);

	//Returns a mutable parameter within the shader.
	CGparameter getParam(const char* name) { return shader.getParam(name); }

private:
	Shader shader;
	std::vector<CGparameter> shader_buffers;
};

#endif

