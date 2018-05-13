#pragma once
// This module abstracts the operation on gpio pins. It simplifies control of gpio pins relating to
// various outputs by accepting LIGHTS_EVENT_GPIO_* events with the data field set to the
// specific peripheral.

#include "event_queue.h"
#include "gpio.h"
#include "status.h"

#include "lights_events.h"

// Used for making output bit-sets.
#define LIGHTS_GPIO_OUTPUT_BIT(output_index) ((1) << (output_index))

// State definitions.
typedef enum {
  LIGHTS_GPIO_STATE_OFF = 0,
  LIGHTS_GPIO_STATE_ON,
  NUM_LIGHTS_GPIO_STATES
} LightsGPIOState;

// Polarity definitions.
typedef enum {
  LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  NUM_LIGHTS_GPIO_POLARITIES
} LightsGPIOPolarity;

// Bit-set where every bit maps to an output.
typedef uint16_t LightsGPIOOutputBitset;

typedef struct LightsGPIOOutput {
  GPIOAddress address;
  LightsGPIOPolarity polarity;
} LightsGPIOOutput;

// Mapping from an event to a set of outputs.
typedef struct LightsGPIOEventMapping {
  LightsEventGPIOPeripheral peripheral;
  LightsGPIOOutputBitset output_mapping;
} LightsGPIOEventMapping;

typedef struct LightsGPIO {
  LightsGPIOOutput *outputs;
  uint8_t num_outputs;  // Number of outputs
  LightsGPIOEventMapping *event_mappings;
  uint8_t num_event_mappings;
} LightsGPIO;

// Initializes all the GPIO pins. All lights are initialized to off.
StatusCode lights_gpio_init(const LightsGPIO *lights_gpio);

// Processes LIGHTS_EVENT_GPIO_* events. Sets the GPIO states of all the outputs corresponding to
// the peripheral shown in the data field of the event.
StatusCode lights_gpio_process_event(const LightsGPIO *lights_gpio, const Event *e);
