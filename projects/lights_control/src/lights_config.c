#include "misc.h"

#include "gpio.h"
#include "lights_config.h"
#include "lights_events.h"

#define LIGHTS_CONFIG_TO_MASK(index) ((1) << (index))

static const GPIOAddress s_board_type_address = { .pin = 13, .port = GPIO_PORT_B };

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

static uint16_t s_front_event_mappings[][2] = {
  { LIGHTS_EVENT_HORN, LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HORN) },  //
  { LIGHTS_EVENT_HIGH_BEAMS,
    LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_RIGHT) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_LEFT) },  //
  { LIGHTS_EVENT_LOW_BEAMS,
    LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_RIGHT) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_LEFT) },  //
  { LIGHTS_EVENT_DRL, LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_DRL_RIGHT) |
                          LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_DRL_LEFT) },  //
  { LIGHTS_EVENT_SIGNAL_LEFT, LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR) |
                                  LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN) },  //
  { LIGHTS_EVENT_SIGNAL_RIGHT,
    LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN) },  //
  { LIGHTS_EVENT_SIGNAL_HAZARD,
    LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN) },  //
};

static uint16_t s_rear_event_mappings[][2] = {
  { LIGHTS_EVENT_STROBE, LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_STROBE) },  //
  { LIGHTS_EVENT_BRAKES, LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_BRAKE) |
                             LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_BRAKE) |
                             LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_BRAKE) |
                             LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_BRAKE) |
                             LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_CENTRE_BRAKE) },  //
  { LIGHTS_EVENT_SIGNAL_LEFT, LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN) |
                                  LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN) },  //
  { LIGHTS_EVENT_SIGNAL_RIGHT, LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN) |
                                   LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN) },  //
  { LIGHTS_EVENT_SIGNAL_HAZARD,
    LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN) |
        LIGHTS_CONFIG_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN) },  //
};

static LightsConfig s_config_front = {
  .board_type = LIGHTS_CONFIG_BOARD_TYPE_FRONT,                  //
  .addresses = s_addresses_front,                                //
  .num_addresses = SIZEOF_ARRAY(s_addresses_front),              //
  .event_mappings = s_front_event_mappings,                      //
  .num_supported_events = SIZEOF_ARRAY(s_front_event_mappings),  //
};

static LightsConfig s_config_rear = {
  .board_type = LIGHTS_CONFIG_BOARD_TYPE_REAR,                  //
  .addresses = s_addresses_rear,                                //
  .num_addresses = SIZEOF_ARRAY(s_addresses_rear),              //
  .event_mappings = s_rear_event_mappings,                      //
  .num_supported_events = SIZEOF_ARRAY(s_rear_event_mappings),  //
};

static LightsConfig *s_config;

StatusCode lights_config_init(void) {
  GPIOSettings settings = {
    .direction = GPIO_DIR_IN,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };

  // initializing board type pin
  status_ok_or_return(gpio_init_pin(&s_board_type_address, &settings));
  // reading the state to know the board type
  GPIOState state;
  status_ok_or_return(gpio_get_state(&s_board_type_address, &state));
  s_config = (state) ? &s_config_front : &s_config_rear;
  return STATUS_CODE_OK;
}

LightsConfig *lights_config_load(void) {
  return s_config;
}
