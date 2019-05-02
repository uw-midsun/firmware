#pragma once
// Module for interfacing with the Audi A6 control stalk
//
// Requires:
//  - ADC
//  - Event Queue
//  - GPIO
//  - GPIO Interrupts
//  - soft timer

#include "status.h"

// Resistor divider value in ohms
#define CONTROL_STALK_RESISTOR 1000
// 4096 codes for +/-4.096V -> LSB = 2mV
#define CONTROL_STALK_THRESHOLD(ohms) ((1 << 12) * (ohms) / ((CONTROL_STALK_RESISTOR) + (ohms)))
// 2k181 +10% resistor = ~2k4, -10% = 1k963
#define CONTROL_STALK_2181_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(1963)
// 681 +10% resistor = ~750, -10% = 613
#define CONTROL_STALK_681_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(613)

// Describes the state of the non-fixed resistor
typedef enum ControlStalkState {
  CONTROL_STALK_STATE_FLOATING = 0,
  CONTROL_STALK_STATE_681_OHMS,
  CONTROL_STALK_STATE_2181_OHMS,
  NUM_CONTROL_STALK_STATES
} ControlStalkState;

// Initialize the control stalk
StatusCode control_stalk_init(void);
