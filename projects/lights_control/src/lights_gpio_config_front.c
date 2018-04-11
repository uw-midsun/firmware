#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio_config.h"

#define LIGHTS_CONFIG_TO_MASK(index) ((1) << (index))

typedef enum {
  LIGHTS_CONFIG_FRONT_LIGHT_HORN = 0,
  LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_RIGHT,
  LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_LEFT,
  LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_RIGHT,
  LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_LEFT,
  LIGHTS_CONFIG_FRONT_LIGHT_DRL_RIGHT,
  LIGHTS_CONFIG_FRONT_LIGHT_DRL_LEFT,
  LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR,
  LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN,
  LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR,
  LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN,
  NUM_FRONT_LIGHTS
} LightsConfigFrontLights;

static const GPIOAddress s_addresses_front[] = {
  [LIGHTS_CONFIG_FRONT_LIGHT_HORN] = { .port = GPIO_PORT_B, .pin = 11 },                  //
  [LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_RIGHT] = { .port = GPIO_PORT_B, .pin = 1 },       //
  [LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_LEFT] = { .port = GPIO_PORT_B, .pin = 15 },       //
  [LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_RIGHT] = { .port = GPIO_PORT_B, .pin = 2 },        //
  [LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_LEFT] = { .port = GPIO_PORT_A, .pin = 8 },         //
  [LIGHTS_CONFIG_FRONT_LIGHT_DRL_RIGHT] = { .port = GPIO_PORT_B, .pin = 0 },              //
  [LIGHTS_CONFIG_FRONT_LIGHT_DRL_LEFT] = { .port = GPIO_PORT_A, .pin = 10 },              //
  [LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR] = { .port = GPIO_PORT_B, .pin = 14 },   //
  [LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN] = { .port = GPIO_PORT_A, .pin = 9 },              //
  [LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR] = { .port = GPIO_PORT_B, .pin = 12 },  //
  [LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN] = { .port = GPIO_PORT_A, .pin = 10 },            //
};

static LightsEventMapping s_front_event_mappings[] = {
  { .event_id = LIGHTS_EVENT_HORN,                                                     //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HORN) },                 //
  { .event_id = LIGHTS_EVENT_HIGH_BEAMS,                                               //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_RIGHT) |      //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_LEFT) },      //
  { .event_id = LIGHTS_EVENT_LOW_BEAMS,                                                //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_RIGHT) |       //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_LEFT) },       //
  { .event_id = LIGHTS_EVENT_DRL,                                                      //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_DRL_RIGHT) |             //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_DRL_LEFT) },             //
  { .event_id = LIGHTS_EVENT_SIGNAL_LEFT,                                              //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR) |   //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN) },            //
  { .event_id = LIGHTS_EVENT_SIGNAL_RIGHT,                                             //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR) |  //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN) },           //
  { .event_id = LIGHTS_EVENT_SIGNAL_HAZARD,                                            //
    .bitset = LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR) |   //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN) |             //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR) |  //
              LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN) },           //
};

static LightsConfig s_config_front = {
  .board_type = LIGHTS_CONFIG_BOARD_TYPE_FRONT,                //
  .addresses = s_addresses_front,                              //
  .num_addresses = SIZEOF_ARRAY(s_addresses_front),            //
  .event_mappings = s_front_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_front_event_mappings),  //
};

LightsConfig *lights_config_front_load(void) {
  return &s_config_front;
}
