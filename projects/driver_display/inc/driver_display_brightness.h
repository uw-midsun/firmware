#pragma once
// Module used to control the brightness of the driver display screens.
// The display configuration must be called first to initialize the GPIO pins to be used (e.g. ADC,
// PWM).

#include "adc.h"
#include "gpio.h"
#include "pwm.h"
#include "pwm_mcu.h"

typedef struct {
  uint16_t max_brightness;
  uint16_t min_brightness;
  uint16_t range_brightness;
  void *persistStorage;
} DriverDisplayBrightnessData;

void driver_display_brightness_calibration(GPIOAddress adc_address);

void driver_display_brightness_init();

void driver_display_brightness_config(GPIOAddress adc_address);

void driver_display_brightness_callback(ADCChannel adc_channel, void *context);

void driver_display_brightness_read(GPIOAddress adc_address);
