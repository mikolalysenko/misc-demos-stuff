#ifndef INPUT_H
#define INPUT_H

namespace Common
{
	//Initialize the keyboard routines
	void key_init();
	void key_update();
	
	//Keyboard functions
	bool key_down(int k);
	bool key_was_down(int k);
	bool key_up(int k);
	bool key_was_up(int k);
	bool key_press(int k);
	bool key_release(int k);
};

#endif
