// SDL/OpenGL starter thing
// Mikola Lysenko
//
// Some notes:
//	F1 - Toggle FPS
//	F2 - Toggle fullscreen
//
//

//Include cstdlib
#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>

//Project files
#include "misc.h"

//Namespace aliasing
using namespace std;

//Constants
#define WINDOW_NAME		"SDL Shell"

//Application parameters
SDL_Surface * window;
int XRes			= 640;
int YRes			= 480;
bool paused			= false;
bool fullscreen		= false;
float fov			= 45.0f;
float z_near		= 0.5f;
float z_far			= 1200.0f;

//Key state modifiers


void update_game(float delta_t)
{
}

void draw_game()
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glViewport(0, 0, XRes-1, YRes-1);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (float)XRes / (float)YRes, z_near, z_far);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glColor3f(1, 1, 1);
	glBegin(GL_TRIANGLES);
		glVertex3f(-1, -1, -4);
		glVertex3f(1, -1, -4);
		glVertex3f(0, 1, -4);
	glEnd();
	
	glFlush();
}

void game_loop()
{
	float last_time = (float)SDL_GetTicks() / 1000.0f;
	
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
		
		//Update game
		float cur_time = (float)SDL_GetTicks() / 1000.0f;
		float delta_time = cur_time - last_time;
		last_time = cur_time;
		if(!paused)
			update_game(delta_time);
		
		//Draw game
		draw_game();
		SDL_GL_SwapBuffers();
	}
}

int main(int argc, char** argv)
{
	srand(time(NULL));

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


	
	game_loop();
	
	return 0;
}

