// SDL/OpenGL starter thing
// Mikola Lysenko
//
// This just contains a bunch of boilerplate initialization code, which is abstracted from the actual callbacks for the application
//

//Basic engine stuff
#include "common/sys_includes.h"
#include "common/input.h"
#include "common/physics.h"

//STL
#include <cstdlib>
#include <cstdio>

//Project files
#include "project/game.h"

//Namespace aliasing
using namespace std;
using namespace Common;
using namespace Game;

//Application parameters
SDL_Surface * window;


//Runs the main loop
void main_loop()
{

	float last_time = (float)SDL_GetTicks() / 1000.0f;
	
	//Initialize subsystems
	key_init();
	phys_init();
	
	//Initialize project specific stuff
	Game::init();
	
	while(true)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_VIDEORESIZE:
					XRes = event.resize.w;
					YRes = event.resize.h;
				
					window = SDL_SetVideoMode(XRes, YRes, 32, 
						SDL_HWSURFACE | 
						(fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE) |
						SDL_OPENGL |
						SDL_HWPALETTE);
				break;
				
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_ESCAPE)
						exit(0);
				break;
				
				case SDL_QUIT:
					exit(0);
				
				default: break;
			}
		}
		
		
		//Update input
		key_update();
		
		//Update game
		Game::update();
		
		// Start simulation (non blocking)
		scene->simulate(delta_t);
		
		//Draw 3D component
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
		glViewport(0, 0, XRes-1, YRes-1);
	
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov, (float)XRes / (float)YRes, z_near, z_far);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		Game::draw();
		
		//Draw HUD overlays
		glClear(GL_DEPTH_BUFFER_BIT);
		
		glMatrixMode(GL_PROJECTION);
		glOrtho(0, 1, 1, 0, -1, 1);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		Game::overlays();
		
		//Swap buffers and blit to screen
		glFlush();
		SDL_GL_SwapBuffers();
		
		//Wait for physics calculations to finish
		scene->flushStream();
		scene->fetchResults(NX_RIGID_BODY_FINISHED, true);
		
		//Synchronize frame rate
		last_time += delta_t * 1000.;
		long long compute_time = (long long)last_time - (long long)SDL_GetTicks();
		if(compute_time > 0)
			SDL_Delay(compute_time);
	}
}

//Program start point
int main(int argc, char** argv)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_SetVideoMode(XRes, YRes, 32, 
		SDL_HWSURFACE | 
		(fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE) |
		SDL_OPENGL |
		SDL_HWPALETTE);
	
	if(window == NULL)
	{
		printf("Couldn't create window: %s\n", SDL_GetError());
		exit(1);
	}

	GLenum err = glewInit();
	if(err != GLEW_OK)
	{
		printf("Couldn't initialize glew: %s\n", glewGetErrorString(err));
		exit(1);
	}
	
	main_loop();
	
	
	return 0;
}

