#pragma once

#include "gpio.h"

typedef struct Devices {
	GPIOAddress* inputs;
	GPIOAddress* outputs;
	uint8_t max_inputs;
	uint8_t max_outputs;
} Devices;

void device_init(Devices* devices);