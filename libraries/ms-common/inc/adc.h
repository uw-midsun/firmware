#pragma once

#include "stm32f0xx.h"	
#include "gpio.h"

#include <stdbool.h>

#define REG_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define REG_TO_BINARY(byte)  \
  (byte & 0x8000 ? '1' : '0'), \
  (byte & 0x4000 ? '1' : '0'), \
  (byte & 0x2000 ? '1' : '0'), \
  (byte & 0x1000 ? '1' : '0'), \
  (byte & 0x0800 ? '1' : '0'), \
  (byte & 0x0400 ? '1' : '0'), \
  (byte & 0x0200 ? '1' : '0'), \
  (byte & 0x0100 ? '1' : '0'), \
  (byte & 0x0080 ? '1' : '0'), \
  (byte & 0x0040 ? '1' : '0'), \
  (byte & 0x0020 ? '1' : '0'), \
  (byte & 0x0010 ? '1' : '0'), \
  (byte & 0x0008 ? '1' : '0'), \
  (byte & 0x0004 ? '1' : '0'), \
  (byte & 0x0002 ? '1' : '0'), \
  (byte & 0x0001 ? '1' : '0') 

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

// Sets the continuous mode sample rate for the given pin.
bool adc_init_pin(GPIOAddress* address, ADCSampleRate adc_sample_rate);

// Returns the current ADC signal as a 12-bit integer. Uses the system voltage as a parameter
uint16_t adc_read(GPIOAddress* address, uint16_t max);
