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
	
	//Camera variables
	extern float fov, z_near, z_far;
	extern Eigen::Transform3d camera;	//Camera frame
	
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


