#pragma once

#include "gpio.h"

#define INPUT_DEVICES 10
#define OUTPUT_DEVICES 1

typedef struct Devices {
	GPIOAddress* inputs;
	GPIOAddress* outputs;
	uint8_t num_inputs;
	uint8_t num_outputs;
} Devices;

void device_init(Devices* devices);