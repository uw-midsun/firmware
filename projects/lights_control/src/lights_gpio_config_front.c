#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio.h"
#include "lights_gpio_config.h"
#include "lights_gpio_config_front.h"

// A mapping of peripheral to its GPIO address.
static const GPIOAddress s_addresses_front[] = {
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HORN] = { .port = GPIO_PORT_B, .pin = 11 },                  //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HIGH_BEAMS_RIGHT] = { .port = GPIO_PORT_B, .pin = 1 },       //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_HIGH_BEAMS_LEFT] = { .port = GPIO_PORT_B, .pin = 15 },       //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LOW_BEAMS_RIGHT] = { .port = GPIO_PORT_B, .pin = 2 },        //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LOW_BEAMS_LEFT] = { .port = GPIO_PORT_A, .pin = 8 },         //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_DRL_RIGHT] = { .port = GPIO_PORT_B, .pin = 0 },              //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_DRL_LEFT] = { .port = GPIO_PORT_A, .pin = 10 },              //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_LEFT_INDICATOR] = { .port = GPIO_PORT_B, .pin = 14 },   //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_LEFT_TURN] = { .port = GPIO_PORT_A, .pin = 9 },              //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_SIDE_RIGHT_INDICATOR] = { .port = GPIO_PORT_B, .pin = 12 },  //
  [LIGHTS_GPIO_CONFIG_FRONT_PERIPHERAL_RIGHT_TURN] = { .port = GPIO_PORT_A, .pin = 10 },            //
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
  .peripherals = s_addresses_front,                              //
  .num_peripherals= SIZEOF_ARRAY(s_addresses_front),            //
  .event_mappings = s_front_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_front_event_mappings),  //
};

LightsGPIO *lights_config_front_load(void) {
  return &s_lights_gpio_front;
}
