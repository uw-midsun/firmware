// temporary header file - we will improve this
#pragma once

#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"

// initialize the termistor
StatusCode thermistor_converter_init(void);

// get the temperature
uint16_t thermistor_converter_get_temp(void);
