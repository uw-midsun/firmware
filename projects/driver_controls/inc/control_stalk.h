#pragma once
// Module for interfacing with the Audi S7 control stalks
// Requires ADS1015, GPIO expander, and event queue to be initialized
//
// Monitors ADS1015 + MCP23008 over I2C and raises events on input state change
// Raises INPUT_EVENT_CONTROL_STALK_* events with empty data fields.
//
// ADS1015
// * A0: Distance
// * A1: CC Speed
// * A2: CC Cancel/Resume (Soft)
// * A3: Turn Signals
// MCP23008 (active-low)
// * A0: CC Set
// * A1: CC On/Off (Hard)
// * A2: Lane Assist
// * A3: Headlight (forward)
// * A4: Headlight (back)
#include <assert.h>
#include "ads1015.h"
#include "gpio_expander.h"
#include "soft_timer.h"

// Resistor divider value in ohms
#define CONTROL_STALK_RESISTOR 3800

// Minimum number of samples for a change to be valid
// 1600 SPS / 4 channels = 400 SPS -> 2.5ms/sample = 25ms debounce
#define CONTROL_STALK_DEBOUNCE_COUNTER 10

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
  size_t debounce_counter[CONTROL_STALK_ANALOG_INPUTS];
} ControlStalk;

// Registers callbacks for analog/digital inputs. |ads1015| and |expander| should be initialized.
StatusCode control_stalk_init(ControlStalk *stalk, Ads1015Storage *ads1015,
                              GpioExpanderStorage *expander);
