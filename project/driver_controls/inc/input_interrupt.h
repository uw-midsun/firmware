#pragma once

#include "gpio.h"
#include "driver_state.h"

void pedal_callback(ADCChannel adc_channel, void *context);

void input_callback(GPIOAddress *address, void *context);

