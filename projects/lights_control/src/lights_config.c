#include "misc.h"

#include "gpio.h"
#include "lights_config.h"
#include "lights_events.h"

#define CONVERT_TO_MASK(index) 1 << index

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

static const GPIOSettings s_gpio_settings_out = { .direction = GPIO_DIR_OUT,
                                                  .state = GPIO_STATE_HIGH,
                                                  .resistor = GPIO_RES_NONE,
                                                  .alt_function = GPIO_ALTFN_NONE };

static const GPIOSettings s_gpio_settings_in = {
  .direction = GPIO_DIR_IN, .resistor = GPIO_RES_NONE, .alt_function = GPIO_ALTFN_NONE
};

static uint16_t s_front_event_mappings[] = {
  [LIGHTS_EVENT_HORN] = CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HORN),  //
  [LIGHTS_EVENT_HIGH_BEAMS] = CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_RIGHT) |
                              CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_HIGH_BEAMS_LEFT),  //
  [LIGHTS_EVENT_LOW_BEAMS] = CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_RIGHT) |
                             CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LOW_BEAMS_LEFT),  //
  [LIGHTS_EVENT_DRL] = CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_DRL_RIGHT) |
                       CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_DRL_LEFT),  //
  [LIGHTS_EVENT_SIGNAL_LEFT] = CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR) |
                               CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN),  //
  [LIGHTS_EVENT_SIGNAL_RIGHT] = CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR) |
                                CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN),  //
  [LIGHTS_EVENT_SIGNAL_HAZARD] = CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_LEFT_INDICATOR) |
                                 CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_LEFT_TURN) |
                                 CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_SIDE_RIGHT_INDICATOR) |
                                 CONVERT_TO_MASK(LIGHTS_CONFIG_FRONT_LIGHT_RIGHT_TURN),  //
};

static uint16_t s_rear_event_mappings[] = {
  [LIGHTS_EVENT_STROBE] = CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_STROBE),  //
  [LIGHTS_EVENT_BRAKES] = CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_BRAKE) |
                          CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_BRAKE) |
                          CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_BRAKE) |
                          CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_BRAKE) |
                          CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_CENTRE_BRAKE),  //
  [LIGHTS_EVENT_SIGNAL_LEFT] = CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN) |
                               CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN),  //
  [LIGHTS_EVENT_SIGNAL_RIGHT] = CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN) |
                                CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN),  //
  [LIGHTS_EVENT_SIGNAL_HAZARD] = CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_OUTER_TURN) |
                                 CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_LEFT_TURN) |
                                 CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_OUTER_TURN) |
                                 CONVERT_TO_MASK(LIGHTS_CONFIG_REAR_LIGHT_RIGHT_TURN),  //
};

static LightsConfig s_config = { .gpio_settings_out = &s_gpio_settings_out };

StatusCode lights_config_init(void) {
  // initializing board type pin
  status_ok_or_return(gpio_init_pin(&s_board_type_address, &s_gpio_settings_in));
  // reading the state to know the board type
  GPIOState state;
  status_ok_or_return(gpio_get_state(&s_board_type_address, &state));
  const LightsConfigBoardType board_type =
      (state) ? LIGHTS_CONFIG_BOARD_TYPE_FRONT : LIGHTS_CONFIG_BOARD_TYPE_REAR;
  s_config.board_type = &board_type;
  s_config.addresses = (state) ? s_addresses_front : s_addresses_rear;
  const uint8_t num_addresses =
      (state) ? SIZEOF_ARRAY(s_addresses_front) : SIZEOF_ARRAY(s_addresses_rear);
  s_config.num_addresses = &num_addresses;
  s_config.event_mappings = (state) ? s_front_event_mappings : s_rear_event_mappings;
  return STATUS_CODE_OK;
}

LightsConfig *lights_config_load(void) {
  return &s_config;
}
