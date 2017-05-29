#pragma once

#include "stm32f0xx.h"	
#include "gpio.h"

#include <stdbool.h>

/* Driver for the STM32's onboard ADC. 

	External Channel mapping
	Pin		ADC Channel
    PA0     ADC_IN0
    PA1     ADC_IN1
    PA2     ADC_IN2
    PA3     ADC_IN3
    PA4     ADC_IN4
    PA5     ADC_IN5
    PA6     ADC_IN6
    PA7     ADC_IN7
    PB0     ADC_IN8
    PB1     ADC_IN9
    PC0     ADC_IN10
    PC1     ADC_IN11
    PC2     ADC_IN12
    PC3     ADC_IN13
    PC4     ADC_IN14
    PC5     ADC_IN15
	
*/

// Additional settings to be used for initialization
typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
} ADCMode;

typedef enum {
  ADC_SAMPLE_RATE_1 = 0,	// 1.5 cycles
  ADC_SAMPLE_RATE_2, 		// 7.5 cycles
  ADC_SAMPLE_RATE_3,		// 13.5 cycles
  ADC_SAMPLE_RATE_4,		// 28.5 cycles
  ADC_SAMPLE_RATE_5,		// 41.5 cycles
  ADC_SAMPLE_RATE_6,		// 55.5 cycles
  ADC_SAMPLE_RATE_7,		// 71.5 cycles
  ADC_SAMPLE_RATE_8			// 239.5 cycles
} ADCSampleRate;

// Initialize the onboard ADC in the specified conversion mode
void adc_init(ADCMode adc_mode);

// Initializes a pin for ADC, returns false if pin is invalid
bool adc_init_pin(GPIOAddress* address, ADCSampleRate adc_sample_rate);

// Returns the current ADC signal as a 12-bit integer. Uses the system voltage as a parameter
uint16_t adc_read(uint16_t max_voltage);
