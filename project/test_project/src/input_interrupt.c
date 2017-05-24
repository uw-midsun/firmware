#include "input_interrupt.h"

/*
	TODO:
			- Make sure the interrupt works properly
*/

void input_callback(GPIOAddress* address, void* context) {
	gpio_toggle_state(context);
}	
