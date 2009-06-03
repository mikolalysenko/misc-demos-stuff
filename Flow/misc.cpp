#include <cstdio>
#include <cstdlib>
#include "misc.h"

using namespace std;


void * loadFile(const char* filename, size_t *size)
{
	FILE * fin = fopen(filename, "r");

	if(fin == NULL)
		return NULL;

	//Determine file size
	fseek(fin, 0, SEEK_END);
	*size = ftell(fin);
	fseek(fin, 0, SEEK_SET);

	//Allocate result
	void * result = malloc(*size);

	//Read in the file
	fread(result, *size, 1, fin);
	
	//Close and return
	fclose(fin);

	return result;
}


Color * loadBMP(const char * filename, size_t * width, size_t * height, int mask)
{
	size_t size;
	char * data = (char*)loadFile(filename, &size);
	
	if(data == NULL) return NULL;

	if(*(short*)data != 19778 ||
	  *(short*)(data + 29) != 24)
	{
		free(data);
		return NULL;
	}

	*width = *(size_t*)(data + 19);
	*height = *(size_t*)(data + 23);

	//Read in the result
	size_t offset = *(size_t*)(data + 11);
	char * bits = (char*)(data + offset);
	Color * result = (Color*)malloc(sizeof(Color) * (*width) * (*height));

	for(size_t i=0; i<(*width) * (*height); i++, bits+=3)
	{
		int c = ((int)bits[0]<<16) + ((int)bits[1]<<8) + (int)bits[2];

		if(c == mask)
		{
			result[i] = Color();
		}
		else
		{
			result[i] = Color((float)bits[0] / 255.0f,
					  (float)bits[1] / 255.0f,
					  (float)bits[2] / 255.0f);
		}
	}
	
	free(data);
	return result;
}

/**
 * Generate a random float (0,1)
 */
float randf()
{
	return (float)rand() / (float)RAND_MAX;
}

/**
 * Draw some text on the screen.
 */
void drawText(pos_t pos, const char* msg)
{
	glLoadIdentity();

	pos_t cur_pos = pos;
	for(const char* c = msg; *c != '\0'; c++)
	{
		if(*c == '\n')
		{
			cur_pos.first = pos.first;
			cur_pos.second -= 18;
			continue;
		}

		glRasterPos2i(cur_pos.first, cur_pos.second);
		glutBitmapCharacter(FONT, *c);
		cur_pos.first += glutBitmapWidth(FONT, *c);
	}
}

/**
 * Update frames per second
 */
void updateFPS()
{
	static int frames = 0;
	static int timebase = 0;
	static float fps = 0.0f;

	char fps_string[100];
	int time = glutGet(GLUT_ELAPSED_TIME);

	frames++;
	if(time - timebase > 1000)
	{
		fps = frames *  1000.0 / (time - timebase);
		timebase = time;
		frames = 0;
	}

	if(show_fps)
	{
		glColor4f(1, 1, 1, 1);
		sprintf(fps_string, "FPS: %5.5f", fps);
		drawText(pos_t(10, 10), fps_string);
	}
}
