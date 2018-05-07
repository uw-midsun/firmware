#pragma once

#include "event_queue.h"
#include "gpio.h"
#include "status.h"

// lights_gpio requires lights_gpio_config to be initialized first.

// This module abstracts the operation on gpio pins. It simplifies control of various peripherals
// by relating them to events. It needs to be initialized first to set up all the gpio pins.

// Used for making peripheral maps
#define LIGHTS_GPIO_PERIPHERAL_BIT(peripheral_index) ((1) << (peripheral_index))

typedef enum {
  LIGHTS_GPIO_STATE_OFF = 0,
  LIGHTS_GPIO_STATE_ON,
  NUM_LIGHTS_GPIO_STATES
} LightsGPIOState;

typedef enum {
  LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  NUM_LIGHTS_GPIO_POLARITIES
} LightsGPIOPolarity;

// Bitset where every bit maps to a peripheral.
typedef uint16_t LightsGPIOPeripheralMapping;

typedef struct LigthsGPIOPeripheral {
  GPIOAddress address;
  LightsGPIOPolarity polarity;
} LightsGPIOPeripheral;

// Mapping from an event to a set of peripherals.
typedef struct LightsGPIOEventMapping {
  EventID event_id;
  LightsGPIOPeripheralMapping peripheral_mapping;
} LightsGPIOEventMapping;

typedef struct LightsGPIO {
  LightsGPIOPeripheral *peripherals;
  uint8_t num_peripherals;  // Number of peripherals
  LightsGPIOEventMapping *event_mappings;
  uint8_t num_event_mappings;
} LightsGPIO;

// Initializes all the GPIO peripherals.
StatusCode lights_gpio_init(const LightsGPIO *lights_gpio);

// Sets the state of every peripheral related to a specific event. Turns off the peripheral light
// if event's data is 0, and on if event's data is anything else.
StatusCode lights_gpio_process_event(const LightsGPIO *lights_gpio, const Event *e);
