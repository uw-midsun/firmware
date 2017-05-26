#include "gpio.h"

#include <stdbool.h>

#define DEVICE_STATES 9 
#define INPUT_DEVICES 11 

// Define ISRs for each of the input pins (To be completed later)

void input_callback(GPIOAddress* address, void* context);
