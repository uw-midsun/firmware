#include "misc.h"

#include "gpio.h"
#include "lights_config.h"

static const GPIOAddress s_board_type_address = { .pin = 13, .port = GPIO_PORT_B };

static const GPIOAddress s_addresses_front[] = {
  [FRONT_LIGHT_HORN] = { .pin = 11, .port = GPIO_PORT_B },                  //
  [FRONT_LIGHT_HIGH_BEAMS_RIGHT] = { .pin = 1, .port = GPIO_PORT_B },       //
  [FRONT_LIGHT_HIGH_BEAMS_LEFT] = { .pin = 15, .port = GPIO_PORT_B },       //
  [FRONT_LIGHT_LOW_BEAMS_RIGHT] = { .pin = 2, .port = GPIO_PORT_B },        //
  [FRONT_LIGHT_LOW_BEAMS_LEFT] = { .pin = 8, .port = GPIO_PORT_A },         //
  [FRONT_LIGHT_DRL_RIGHT] = { .pin = 0, .port = GPIO_PORT_B },              //
  [FRONT_LIGHT_DRL_LEFT] = { .pin = 10, .port = GPIO_PORT_A },              //
  [FRONT_LIGHT_SIDE_LEFT_INDICATOR] = { .pin = 14, .port = GPIO_PORT_B },   //
  [FRONT_LIGHT_LEFT_TURN] = { .pin = 9, .port = GPIO_PORT_A },              //
  [FRONT_LIGHT_SIDE_RIGHT_INDICATOR] = { .pin = 12, .port = GPIO_PORT_B },  //
  [FRONT_LIGHT_RIGHT_TURN] = { .pin = 10, .port = GPIO_PORT_A },            //
};

static const GPIOAddress s_addresses_rear[] = {
  [REAR_LIGHT_STROBE] = { .pin = 11, .port = GPIO_PORT_B },            //
  [REAR_LIGHT_RIGHT_BRAKE] = { .pin = 1, .port = GPIO_PORT_B },        //
  [REAR_LIGHT_RIGHT_OUTER_BRAKE] = { .pin = 2, .port = GPIO_PORT_B },  //
  [REAR_LIGHT_LEFT_BRAKE] = { .pin = 15, .port = GPIO_PORT_B },        //
  [REAR_LIGHT_LEFT_OUTER_BRAKE] = { .pin = 8, .port = GPIO_PORT_A },   //
  [REAR_LIGHT_CENTRE_BRAKE] = { .pin = 14, .port = GPIO_PORT_B },      //
  [REAR_LIGHT_LEFT_OUTER_TURN] = { .pin = 10, .port = GPIO_PORT_A },   //
  [REAR_LIGHT_LEFT_TURN] = { .pin = 9, .port = GPIO_PORT_A },          //
  [REAR_LIGHT_RIGHT_OUTER_TURN] = { .pin = 0, .port = GPIO_PORT_B },   //
  [REAR_LIGHT_RIGHT_TURN] = { .pin = 10, .port = GPIO_PORT_B },        //
};

static const GPIOSettings s_gpio_settings_out = { .direction = GPIO_DIR_OUT,
                                                  .state = GPIO_STATE_HIGH,
                                                  .resistor = GPIO_RES_NONE,
                                                  .alt_function = GPIO_ALTFN_NONE };

static const GPIOSettings s_gpio_settings_in = {
  .direction = GPIO_DIR_IN, .resistor = GPIO_RES_NONE, .alt_function = GPIO_ALTFN_NONE
};

static LightsConfig s_config = { .board_type_address = &s_board_type_address,
                                 .addresses_front = s_addresses_front,
                                 .addresses_rear = s_addresses_rear,
                                 .num_addresses_front = SIZEOF_ARRAY(s_addresses_front),
                                 .num_addresses_rear = SIZEOF_ARRAY(s_addresses_rear),
                                 .gpio_settings_in = &s_gpio_settings_in,
                                 .gpio_settings_out = &s_gpio_settings_out };

LightsConfig *lights_config_load(void) {
  return &s_config;
}
