#include "gpio.h"

#include <stdbool.h>

#define DEVICE_STATES 9 
#define INPUT_DEVICES 11 

// Define ISRs for each of the input pins

/* TODO: 
		- Come up with better names 
		- The lookup table should (probably?) be implemented here
*/

void input_callback(GPIOAddress* address, void* context);
