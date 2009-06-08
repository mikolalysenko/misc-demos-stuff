#ifndef GAME_H
#define GAME_H

#include "common/sys_includes.h"

//Game configuration variables
namespace Game
{
	//Screen resolution
	extern bool fullscreen;
	extern int XRes, YRes;
	
	//Camera variables
	extern float fov, z_near, z_far;
	
	//Time quantum
	extern float delta_t;
	

	//Initialization function
	void init();
	
	//Update callback
	void update();
	
	//Rendering
	void draw();
	
	//HUD stuff / overlays
	void overlays();
};

#endif


