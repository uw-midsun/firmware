#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio_config_front.h"

// Output definitions.
static const LightsGpioOutput s_front_outputs[] = {
  // clang-format and lint are inconsistent here. clang-format puts a line break before the { in
  // every entry, while lint does not allow that, and requires that { be put in the end of the
  // last line.
  // clang-format off
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_HORN] = {
    .address = { .port = GPIO_PORT_B, .pin = 11 }, // LED 2
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_HIGH_BEAMS_RIGHT] = {
    .address = { .port = GPIO_PORT_B, .pin = 1 }, // LED 6
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_HIGH_BEAMS_LEFT] = {
    .address = { .port = GPIO_PORT_B, .pin = 15 }, // LED 10
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_LOW_BEAMS_RIGHT] = {
    .address = { .port = GPIO_PORT_B, .pin = 2 }, // LED 8
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_LOW_BEAMS_LEFT] = {
    .address = { .port = GPIO_PORT_A, .pin = 8 }, // LED 12
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_DRL_RIGHT] = {
    .address = { .port = GPIO_PORT_B, .pin = 0 }, // LED 7
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_DRL_LEFT] = {
    .address = { .port = GPIO_PORT_A, .pin = 10 }, // LED 11
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_SIDE_LEFT_INDICATOR] = {
    .address = { .port = GPIO_PORT_B, .pin = 14 }, // LED 4
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_LEFT_TURN] = {
    .address = { .port = GPIO_PORT_A, .pin = 9 }, // LED 13
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_SIDE_RIGHT_INDICATOR] = {
    .address = { .port = GPIO_PORT_B, .pin = 12 }, // LED 1
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  [LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_RIGHT_TURN] = {
    .address = { .port = GPIO_PORT_B, .pin = 10 }, // LED 9
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_HIGH,
  },
  // clang-format off
};

// An array containing mappings from peripheral to sets of outputs.
static LightsGpioEventMapping s_front_event_mappings[] = {
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_HORN,  //
    .output_mapping =
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_HORN) },  //
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    .output_mapping =
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_HIGH_BEAMS_RIGHT) |  //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_HIGH_BEAMS_LEFT) },  //
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    .output_mapping =
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_LOW_BEAMS_RIGHT) |  //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_LOW_BEAMS_LEFT) },  //
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    .output_mapping =
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_DRL_RIGHT) |  //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_DRL_LEFT) },  //
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT,
    .output_mapping =
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_SIDE_LEFT_INDICATOR) |  //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_LEFT_TURN) },           //
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT,
    .output_mapping =
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_SIDE_RIGHT_INDICATOR) |  //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_RIGHT_TURN) },           //
  { .peripheral = LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD,
    .output_mapping =
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_SIDE_LEFT_INDICATOR) |   //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_LEFT_TURN) |             //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_SIDE_RIGHT_INDICATOR) |  //
        LIGHTS_GPIO_OUTPUT_BIT(LIGHTS_GPIO_CONFIG_FRONT_OUTPUT_RIGHT_TURN) },           //
};

static const LightsGpio s_lights_gpio_front = {
  .outputs = s_front_outputs,                          //
  .num_outputs = SIZEOF_ARRAY(s_front_outputs),        //
  .event_mappings = s_front_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_front_event_mappings),  //
};

LightsGpio *lights_gpio_config_front_load(void) {
  return &s_lights_gpio_front;
}
