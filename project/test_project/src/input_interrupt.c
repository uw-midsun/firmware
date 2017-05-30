#include "input_interrupt.h"
#include "gpio.h"

#include <stdio.h>

void input_callback (GPIOAddress* address, void* context) {
	GPIOState key_pressed; 
	gpio_get_value(address, &key_pressed);

	printf("Interrupt triggered by P%c%d - button was %s\n", 
		(uint8_t)(address->port+65),
		address->pin,
		(key_pressed) ? "pressed" : "released"
	);

	debounce(address, key_pressed);
	return;
}

