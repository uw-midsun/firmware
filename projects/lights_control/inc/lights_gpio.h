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
} LightsGpioState;

// Polarity definitions.
typedef enum {
  LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  NUM_LIGHTS_GPIO_POLARITIES
} LightsGpioPolarity;

// Bit-set where every bit maps to an output.
typedef uint16_t LightsGpioOutputBitset;

typedef struct LightsGpioOutput {
  GpioAddress address;
  LightsGpioPolarity polarity;
} LightsGpioOutput;

// Mapping from an peripheral to a set of outputs.
typedef struct LightsGpioEventMapping {
  LightsEventGpioPeripheral peripheral;
  LightsGpioOutputBitset output_mapping;
} LightsGpioEventMapping;

typedef struct LightsGpio {
  LightsGpioOutput *outputs;
  uint8_t num_outputs;  // Number of outputs
  LightsGpioEventMapping *event_mappings;
  uint8_t num_event_mappings;
} LightsGpio;

// Initializes all the GPIO pins. All lights are initialized to off.
StatusCode lights_gpio_init(const LightsGpio *lights_gpio);

// Processes LIGHTS_EVENT_GPIO_* events. Sets the GPIO states of all the outputs corresponding to
// the peripheral shown in the data field of the event.
StatusCode lights_gpio_process_event(const LightsGpio *lights_gpio, const Event *e);
