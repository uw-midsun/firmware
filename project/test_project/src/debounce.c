#include "debounce.h"

//
#include <stdio.h>
//

/* TODO:
		- Once you've implemented the debouncer, find a pushbutton to use 
		  to test the thing
		- Use for loops for now. Use actual time delay later
		- Just get it working, good code comes later
*/

void debounce (GPIOAddress *address, GPIOState key_pressed) {
	
	uint16_t count = (key_pressed) ? HOLD_TIME_PRESSED : HOLD_TIME_RELEASED;
	
	GPIOState current_state;

	while (count > 0) {	
		for (uint8_t i = 0; i < SAMPLING_INTERVAL; i++) { 
			gpio_get_value(address, &current_state);
			//printf(" - %d\n", current_state);
			//printf(" - %d\n", count);
		}
		
		if (current_state == key_pressed) {
			count--;
		} else {
			count = (key_pressed) ? HOLD_TIME_PRESSED : HOLD_TIME_RELEASED;
		}
	}
	
	return;	
}
