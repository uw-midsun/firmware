#pragma once

#include "adc.h"
#include "gpio.h"
#include "driver_state.h"
#include "fsm.h"
#include "stm32f0xx.h"

#include <stdbool.h>

#define DEVICE_STATES 10 
#define INPUT_DEVICES 8 
#define OUTPUT_DEVICES 4
#define MAX_SPEED 200
#define PEDAL_THRESHOLD 80

//
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
//

// Define ISRs for each of the input pins (To be completed later)
/*
	Pin		Device
	0		Power
	1		Gas
	2		Brakes
	3		Direction Selector 1
	4		Direction Selector 2
	5		Cruise Control On/Off
	6		Cruise Control Inc
	7		Cruise Control Dec
	8		
	9		
	10		
	11		
	12		
	13		
	14		
	15		
*/
void input_callback (GPIOAddress* address, FSMGroup* fsm_group);

