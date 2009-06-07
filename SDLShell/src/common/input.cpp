#include <algorithm>

//Common headers
#include "common/sys_includes.h"
#include "common/input.h"

using namespace std;

namespace Common
{
//Maximum number of keys
const int KEY_MAX = 65536;

//Key data
int active_buffer;
Uint8 key_array[2][KEY_MAX];

//Initialize keyboard
void key_init()
{
	active_buffer = 0;
	fill(key_array[0], &key_array[0][KEY_MAX], 0);
	fill(key_array[1], &key_array[1][KEY_MAX], 0);
}

//Key polling
void key_update()
{
	Uint8 *keys = SDL_GetKeyState(NULL);
	active_buffer ^= 1;
	copy(keys, keys + KEY_MAX, key_array[active_buffer]);
}

//Keyboard polling
bool key_down(int k)		{ return key_array[active_buffer][k]; }
bool key_up(int k)			{ return !key_down(k); }
bool key_was_down(int k)	{ return key_array[active_buffer^1][k]; }
bool key_was_up(int k)		{ return !key_was_down(k); }
bool key_press(int k)		{ return key_down(k) && key_was_up(k); }
bool key_release(int k)		{ return key_up(k) && key_was_down(k); }

};


