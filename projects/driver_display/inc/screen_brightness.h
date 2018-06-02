#pragma once

#include "adc.h"

#define MAX_BRIGHTNESS 297000

void test_callback(ADCChannel adc_channel, void *context);

void read_brightness(void);

uint16_t map_brightness(uint16_t adc_reading);
