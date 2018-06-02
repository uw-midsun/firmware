#pragma once
// Controls the brightness of the driver display screen through photo-sensor

#include "adc.h"
#include "gpio.h"
#include "pwm.h"
#include "pwm_mcu.h"

void driver_display_brightness_config(GPIOAddress adc_address);

void driver_display_brightness_callback(ADCChannel adc_channel, void *context);

void driver_display_brightness_read(GPIOAddress adc_address);
