#ifndef SHADER_H
#define SHADER_H

#include "misc.h"


/**
 * A Shader is a program which is executed on pixels or vertices.  In this
 * case, we are restricting all operations to pxiels, since the vertex
 * shaders are not quite so useful for most general purpose computaitons.
 */
class Shader
{
public:

	Shader(const char* source_file);
	~Shader();

	//Begin executing the shader.
	void begin();

	//Finish executing the shader.
	void end();

	//Returns a mutable parameter within the shader.
	CGparameter getParam(const char* name);

	//Initializes the shader system.  Must be called before any shaders are created.
	static void initShaders();

private:
	static CGcontext context;
	
	CGprofile profile;
	CGprogram program;

	static void cgErrorCallback();
};

#endif

