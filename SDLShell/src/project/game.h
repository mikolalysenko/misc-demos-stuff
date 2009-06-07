#ifndef GAME_H
#define GAME_H


//Game configuration variables
namespace Game
{
	//Screen resolution/camera variables
	extern bool fullscreen;
	extern int XRes, YRes;
	extern float fov, z_near, z_far;

	//Initialization function
	void init();
	
	//Update callback
	void update(float delta_t);
	
	//Rendering
	void draw();
	
	//HUD stuff / overlays
	void overlays();
};

#endif


