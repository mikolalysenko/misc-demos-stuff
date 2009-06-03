#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstdio>

#include "misc.h"
#include "shader.h"

using namespace std;

CGcontext Shader::context = NULL;

void Shader::cgErrorCallback()
{
	CGerror last_error = cgGetError();

	if(last_error)
	{
		fprintf(stderr, "%s\n", cgGetErrorString(last_error));
		fprintf(stderr, "%s\n", cgGetLastListing(context));
		exit(1);
	}
}

void Shader::initShaders()
{
	context = cgCreateContext();
	cgGLRegisterStates(context);
	cgSetErrorCallback(cgErrorCallback);
	cgSetAutoCompile(context, CG_COMPILE_IMMEDIATE);
}

Shader::Shader(const char* source)
{
	//Create profile
	profile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
	cgGLSetOptimalOptions(profile);

	//Compile and instantiate
	program = cgCreateProgramFromFile(
		context,
		CG_SOURCE,
		source,
		profile,
		NULL,
		NULL);
	
	cgGLLoadProgram(program);
}

Shader::~Shader()
{
	cgDestroyProgram(program);
}


void Shader::begin()
{
	cgGLEnableProfile(profile);
	cgGLBindProgram(program);
}

void Shader::end()
{
	cgGLDisableProfile(profile);
}

CGparameter Shader::getParam(const char* name)
{
	return cgGetNamedParameter(program, name);
}
