#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio.h"
#include "lights_gpio_config.h"
#include "lights_gpio_config_rear.h"

// Output definitions.
static const LightsGpioOutput s_rear_outputs[] = {
  // clang-format off
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_STROBE] = {
    .address = { .port = GPIO_PORT_B, .pin = 11 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_BRAKE] = {
    .address = { .port = GPIO_PORT_B, .pin = 1 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_OUTER_BRAKE] = {
    .address = { .port = GPIO_PORT_B, .pin = 2 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_BRAKE] = {
    .address = { .port = GPIO_PORT_B, .pin = 15 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_OUTER_BRAKE] = {
    .address = { .port = GPIO_PORT_A, .pin = 8 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_CENTRE_BRAKE] = {
    .address = { .port = GPIO_PORT_B, .pin = 14 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_OUTER_TURN] = {
    .address = { .port = GPIO_PORT_A, .pin = 10 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_TURN] = {
    .address = { .port = GPIO_PORT_A, .pin = 9 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_OUTER_TURN] = {
    .address = { .port = GPIO_PORT_B, .pin = 0 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  },
  [LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_TURN] = {
    .address = { .port = GPIO_PORT_B, .pin = 10 },
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,
  }
  // clang-format off
};

static LightsGpioEventMapping s_rear_event_mappings[] = {
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_STROBE,  //
    .output_mapping = LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_STROBE) },
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,  //
    .output_mapping =             //
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_BRAKE) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_OUTER_BRAKE) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_BRAKE) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_OUTER_BRAKE) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_CENTRE_BRAKE) },
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT,  //
    .output_mapping =                        //
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_OUTER_TURN) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_TURN) },
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT,  //
    .output_mapping =                         //
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_OUTER_TURN) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_TURN) },
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD,  //
    .output_mapping =                          //
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_OUTER_TURN) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_LEFT_TURN) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_OUTER_TURN) |
    LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_REAR_OUTPUT_RIGHT_TURN) },
};

static const LightsGpio s_lights_gpio_rear = {
  .outputs = s_rear_outputs,                          //
  .num_outputs = SIZEOF_ARRAY(s_rear_outputs),        //
  .event_mappings = s_rear_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_rear_event_mappings),  //
};

LightsGpio *lights_config_rear_load(void) {
  return &s_lights_gpio_rear;
}
