#ifndef GAME_H
#define GAME_H

#include <Eigen/Core>
#include <Eigen/Geometry>


//Game configuration variables
namespace Game
{
	//Screen resolution
	extern bool fullscreen;
	extern int XRes, YRes;
	
	extern bool quit;
	
	//Camera variables
	extern float fov, z_near, z_far;
	
	//Time quantum
	extern float delta_t;
	

	//Initialization function
	void init();
	
	//Update callback
	void update(float deltat);
	
	//Rendering
	void draw();
	
	//HUD stuff / overlays
	void overlays();
};

#endif


