#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio_config_front.h"

// Peripheral definitions
static const LightsGPIOPeripheral s_front_peripherals[] = {
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HORN] = { //
    .address = {.port = GPIO_PORT_B, .pin = 11 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HIGH_BEAMS_RIGHT] = { //
    .address = { .port = GPIO_PORT_B, .pin = 1 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HIGH_BEAMS_LEFT] = { //
    .address = { .port = GPIO_PORT_B, .pin = 15 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LOW_BEAMS_RIGHT] = { //
    .address = { .port = GPIO_PORT_B, .pin = 2 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LOW_BEAMS_LEFT] = { //
    .address = { .port = GPIO_PORT_A, .pin = 8 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_DRL_RIGHT] = { //
    .address = { .port = GPIO_PORT_B, .pin = 0 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_DRL_LEFT] = { //
    .address = { .port = GPIO_PORT_A, .pin = 10 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_LEFT_INDICATOR] = {
    .address = { .port = GPIO_PORT_B, .pin = 14 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LEFT_TURN] = { //
    .address = { .port = GPIO_PORT_A, .pin = 9 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_RIGHT_INDICATOR] = { //
    .address = { .port = GPIO_PORT_B, .pin = 12 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_RIGHT_TURN] = { //
    .address = { .port = GPIO_PORT_A, .pin = 10 }, //
    .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW, //
  }, //
};

// An array containing event-to-peripheral-set mappings.
static LightsGPIOEventMapping s_front_event_mappings[] = {
  { .event_id = LIGHTS_EVENT_HORN,                                                     //
    .peripheral_mapping =
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HORN) },                 //
  { .event_id = LIGHTS_EVENT_HIGH_BEAMS,                                               //
    .peripheral_mapping =
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HIGH_BEAMS_RIGHT) |      //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HIGH_BEAMS_LEFT) },      //
  { .event_id = LIGHTS_EVENT_LOW_BEAMS,                                                //
    .peripheral_mapping =
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LOW_BEAMS_RIGHT) |       //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LOW_BEAMS_LEFT) },       //
  { .event_id = LIGHTS_EVENT_DRL,                                                      //
    .peripheral_mapping =
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_DRL_RIGHT) |             //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_DRL_LEFT) },             //
  { .event_id = LIGHTS_EVENT_SIGNAL_LEFT,                                              //
    .peripheral_mapping =
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_LEFT_INDICATOR) |   //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LEFT_TURN) },            //
  { .event_id = LIGHTS_EVENT_SIGNAL_RIGHT,                                             //
    .peripheral_mapping =
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_RIGHT_INDICATOR) |  //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_RIGHT_TURN) },           //
  { .event_id = LIGHTS_EVENT_SIGNAL_HAZARD,                                            //
    .peripheral_mapping =
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_LEFT_INDICATOR) |   //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LEFT_TURN) |             //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_RIGHT_INDICATOR) |  //
      LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_RIGHT_TURN) },           //
};

static const LightsGPIO s_lights_gpio_front = {
  .peripherals = s_front_peripherals,                              //
  .num_peripherals= SIZEOF_ARRAY(s_front_peripherals),            //
  .event_mappings = s_front_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_front_event_mappings),  //
};

LightsGPIO *lights_config_front_load(void) {
  return &s_lights_gpio_front;
}
