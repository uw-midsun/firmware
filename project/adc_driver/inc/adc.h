#pragma once

#include "stm32f0xx.h"	
#include "gpio.h"

#include <stdbool.h>

// Driver for the STM32's onboard ADC

// Additional settings to be used for initialization
typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
} ADCMode;

// typedef enum {} ADCSampleTime;

// Initialize the onboard ADC
void adc_init(ADCMode adc_mode);

// Initialize the ADC channels corresponding to the given pin (Returns false if the pin is not mapped to an ADC channel)
bool adc_init_pin(GPIOAddress* address);

// Get readings from the ADC
uint16_t adc_read();
uint16_t adc_read_periodic();
