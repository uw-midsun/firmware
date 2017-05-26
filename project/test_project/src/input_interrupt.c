#include "input_interrupt.h"
#include "gpio.h"

void input_callback(GPIOAddress* address, void* context) {
	gpio_toggle_state(&context[i]);
}	
