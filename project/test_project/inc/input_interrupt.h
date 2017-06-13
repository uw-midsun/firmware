#pragma once

#include "adc.h"
#include "gpio.h"
#include "driver_state.h"
#include "fsm.h"
#include "stm32f0xx.h"

#define INPUT_DEVICES 10
#define OUTPUT_DEVICES 1
#define MAX_SPEED 200

#define COAST_THRESHOLD 40
#define DRIVE_THRESHOLD 130
#define SAMPLE_AVERAGE 20

// Prints the given number as a binary value
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x8000 ? '1' : '0'), \
  (byte & 0x4000 ? '1' : '0'), \
  (byte & 0x2000 ? '1' : '0'), \
  (byte & 0x1000 ? '1' : '0'), \
  (byte & 0x800 ? '1' : '0'), \
  (byte & 0x400 ? '1' : '0'), \
  (byte & 0x200 ? '1' : '0'), \
  (byte & 0x100 ? '1' : '0'), \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

void input_callback(GPIOAddress* address, FSMGroup* fsm_group);

