#pragma once

#include "event_queue.h"
#include "status.h"
#include "gpio.h"

// lights_gpio requires lights_gpio_config to be initialized first.

// Used for making peripheral maps
#define LIGHTS_GPIO_PERIPHERAL_BIT(peripheral_index) ((1) << (peripheral_index))

// Bitset where every bit maps to a peripheral.
typedef uint16_t LightsGPIOPeripheralMapping;

// This module abstracts the operation on gpio pins. It simplifies control of various peripherals
// by relating them to events. It needs to be initialized first to set up all the gpio pins.

typedef enum {
  LIGHTS_GPIO_STATE_OFF = 0, //
  LIGHTS_GPIO_STATE_ON, //
  NUM_LIGHTS_GPIO_STATES //
} LightsGPIOState;

// Mapping from an event to a set of peripherals.
typedef struct LightsGPIOEventMapping {
  EventID event_id;
  LightsGPIOPeripheralMapping peripheral_mapping;
} LightsGPIOEventMapping;

typedef struct LightsGPIO {
  GPIOAddress *peripherals; 
  uint8_t num_peripherals; // Number of peripherals
  LightsGPIOEventMapping *event_mappings;
  uint8_t num_event_mappings;
} LightsGPIO;

// Initializes all the GPIO peripherals.
StatusCode lights_gpio_init(LightsGPIO *lights_gpio);

// Sets the state of every peripheral related to a specific event. Turns off the peripheral light
// if event's data is 0, and on if event's data is anything else.
StatusCode lights_gpio_process_event(LightsGPIO *lights_gpio, const Event *e);

