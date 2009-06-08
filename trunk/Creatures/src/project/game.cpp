//Engine stuff
#include "common/sys_includes.h"
#include "common/input.h"

//Project stuff
#include "project/game.h"

//STL stuff
#include <iostream>
#include <vector>

using namespace std;
using namespace Common;

namespace Game
{

//Config variables
int XRes			= 640;
int YRes			= 480;
bool fullscreen		= false;
float fov			= 45.0f;
float z_near		= 0.5f;
float z_far			= 1200.0f;
float delta_t		= 1. / 60.;

//Initialization
void init()
{
}



//Update the game state
void update()
{
}


//Draw stuff
void draw()
{
	if(key_down(SDLK_a))
	{
		glBegin(GL_TRIANGLES);
		
		glVertex3f(-1, -1, -5);
		glVertex3f(1, -1, -5);
		glVertex3f(1, 1, -5);
		
		glEnd();
	}

}


//Handle overlays
void overlays()
{
}

};

