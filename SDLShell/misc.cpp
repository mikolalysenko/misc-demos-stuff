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
