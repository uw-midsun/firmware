#pragma once

// Callback functions for GPIO device interrupts

// Once an event has been detected, the proper ISR will be called to determine the Event ID based
// on the triggering address so that the correct event can be raised in the priority queue.

#include "adc.h"
#include "gpio.h"

// Since the gas pedal is monitored through ADC interrupts, a separate callback is needed
void pedal_callback(ADCChannel adc_channel, void *context);

// Main ISR for GPIO device interrupts
void input_callback(GPIOAddress *address, void *context);
