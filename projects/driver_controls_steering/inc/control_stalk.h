#pragma once
// Module for interfacing with the Audi A6 control stalk
//
// Requires:
//  - ADC
//  - Event Queue
//  - GPIO
//  - GPIO Interrupts
//  - soft timer

#include "adc.h"
#include "event_queue.h"
#include "gpio.h"
#include "status.h"

// Resistor divider value in ohms
#define CONTROL_STALK_RESISTOR_OHMS 1000u
// 4096 codes for +/-4.096V -> LSB = 2mV
#define CONTROL_STALK_THRESHOLD(ohms) \
  ((1u << 12) * (ohms) / ((CONTROL_STALK_RESISTOR_OHMS) + (ohms)))
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

// Identifier for each Digital Input
typedef enum {
  CONTROL_STALK_DIGITAL_INPUT_ID_HORN = 0,
  CONTROL_STALK_DIGITAL_INPUT_ID_CC_ON_OFF,
  CONTROL_STALK_DIGITAL_INPUT_ID_SET,
  NUM_CONTROL_STALK_DIGITAL_INPUT_IDS,
} ControlStalkDigitalInputId;

// Identifier for each Analog Input
typedef enum {
  CONTROL_STALK_ANALOG_INPUT_ID_CC_SPEED = 0,
  CONTROL_STALK_ANALOG_INPUT_ID_CC_CANCEL_RESUME,
  CONTROL_STALK_ANALOG_INPUT_ID_TURN_SIGNAL_STALK,
  CONTROL_STALK_ANALOG_INPUT_ID_CC_DISTANCE,
  NUM_CONTROL_STALK_ANALOG_INPUT_IDS,
} ControlStalkAnalogInputId;

// Analog Inputs
typedef struct {
  // CAN events to raise
  EventId can_event[NUM_CONTROL_STALK_STATES];
  // Pin of the Analog Input
  GpioAddress address;
} ControlStalkAnalogInput;

// Digital Inputs
typedef struct {
  // CAN events to raise
  EventId can_event[NUM_GPIO_STATES];
  // Pin of the Digital Input
  GpioAddress pin;
} ControlStalkDigitalInput;

typedef struct {
  // Digital Inputs
  ControlStalkDigitalInput digital_config[NUM_CONTROL_STALK_DIGITAL_INPUT_IDS];

  // Analog Inputs
  ControlStalkAnalogInput analog_config[NUM_CONTROL_STALK_ANALOG_INPUT_IDS];

  // States of the analog inputs
  ControlStalkState prev_analog_state[NUM_ADC_CHANNELS];
} ControlStalkStorage;

// Initialize the control stalk
StatusCode control_stalk_init(ControlStalkStorage *storage);
