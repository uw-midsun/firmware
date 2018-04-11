#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio_config.h"

#define LIGHTS_CONFIG_TO_MASK(index) ((1) << (index))

typedef enum {
  LIGHTS_CONFIG_REAR_LIGHT_STROBE = 0,
  LIGHTS_CONFIG_REAR_LIGHT_RIGHT_BRAKE,
  LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_BRAKE,
  LIGHTS_CONFIG_REAR_LIGHT_LEFT_BRAKE,
  LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_BRAKE,
  LIGHTS_CONFIG_REAR_LIGHT_CENTRE_BRAKE,
  LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN,
  LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN,
  LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN,
  LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN,
  NUM_REAR_LIGHTS,
} LightsConfigRearLights;

static const GPIOAddress s_addresses_rear[] = {
  [LIGHTS_CONFIG_REAR_LIGHT_STROBE] = { .port = GPIO_PORT_B, .pin = 11 },            //
  [LIGHTS_CONFIG_REAR_LIGHT_RIGHT_BRAKE] = { .port = GPIO_PORT_B, .pin = 1 },        //
  [LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_BRAKE] = { .port = GPIO_PORT_B, .pin = 2 },  //
  [LIGHTS_CONFIG_REAR_LIGHT_LEFT_BRAKE] = { .port = GPIO_PORT_B, .pin = 15 },        //
  [LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_BRAKE] = { .port = GPIO_PORT_A, .pin = 8 },   //
  [LIGHTS_CONFIG_REAR_LIGHT_CENTRE_BRAKE] = { .port = GPIO_PORT_B, .pin = 14 },      //
  [LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN] = { .port = GPIO_PORT_A, .pin = 10 },   //
  [LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN] = { .port = GPIO_PORT_A, .pin = 9 },          //
  [LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN] = { .port = GPIO_PORT_B, .pin = 0 },   //
  [LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN] = { .port = GPIO_PORT_B, .pin = 10 },        //
};

static LightsEventMapping s_rear_event_mappings[] = {
  { .event_id = LIGHTS_EVENT_STROBE_BLINK,                                         //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_STROBE) },            //
  { .event_id = LIGHTS_EVENT_BRAKES,                                               //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_BRAKE) |        //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_BRAKE) |  //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_BRAKE) |         //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_BRAKE) |   //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_CENTRE_BRAKE) },      //
  { .event_id = LIGHTS_EVENT_SIGNAL_LEFT_BLINK,                                    //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN) |    //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN) },         //
  { .event_id = LIGHTS_EVENT_SIGNAL_RIGHT_BLINK,                                   //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN) |   //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN) },        //
  { .event_id = LIGHTS_EVENT_SIGNAL_HAZARD_BLINK,                                  //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN) |    //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN) |          //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN) |   //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN) },        //
};

static LightsConfig s_config_rear = {
  .board_type = LIGHTS_CONFIG_BOARD_TYPE_REAR,                //
  .addresses = s_addresses_rear,                              //
  .num_addresses = SIZEOF_ARRAY(s_addresses_rear),            //
  .event_mappings = s_rear_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_rear_event_mappings),  //
};

LightsConfig *lights_config_rear_load(void) {
  return &s_config_rear;
}
