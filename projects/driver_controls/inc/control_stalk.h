#pragma once
// Module for interfacing with the Audi S7 control stalks
// Requires ADS1015 and GPIO expander to be initialized
//
// Monitors ADS1015 + MCP23008 over I2C and raises events on input state change
// ADS1015
// * A0: Distance
// * A1: CC Speed
// * A2: CC Cancel/Resume (Soft)
// * A3: Turn Signals
// MCP23008
// * A0: CC Set
// * A1: CC On/Off (Hard)
// * A2: Lane Assist
// * A3: Headlight (forward)
// * A4: Headlight (back)
#include "ads1015.h"
#include "gpio_expander.h"
#include <assert.h>

// Resistor divider value in ohms
#define CONTROL_STALK_RESISTOR 3800

#define CONTROL_STALK_ANALOG_INPUTS 4
static_assert(CONTROL_STALK_ANALOG_INPUTS <= NUM_ADS1015_CHANNELS,
              "Control stalk analog inputs larger than number of ADS1015 channels!");
#define CONTROL_STALK_DIGITAL_INPUTS 5
static_assert(CONTROL_STALK_DIGITAL_INPUTS <= NUM_GPIO_EXPANDER_PINS,
              "Control stalk digital inputs larger than number of MCP23008 pins!");

// Describes the state of the non-fixed resistor
typedef enum ControlStalkState {
  CONTROL_STALK_STATE_FLOATING = 0,
  CONTROL_STALK_STATE_681_OHMS,
  CONTROL_STALK_STATE_2181_OHMS,
  NUM_CONTROL_STALK_STATES
} ControlStalkState;

typedef struct ControlStalk {
  Ads1015Storage *ads1015;
  GpioExpanderStorage *expander;
  ControlStalkState states[CONTROL_STALK_ANALOG_INPUTS];
} ControlStalk;

StatusCode control_stalk_init(ControlStalk *stalk, Ads1015Storage *ads1015, GpioExpanderStorage *expander);
