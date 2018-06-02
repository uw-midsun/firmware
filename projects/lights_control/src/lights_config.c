#include "lights_config.h"
#include "lights_gpio.h"

const GPIOAddress s_board_type_address = {
  .port = GPIO_PORT_B,
  .pin = 13
};

StatusCode lights_config_get_board_type(LightsConfigBoardType *board_type) {
  GPIOState state = 0;
  status_ok_or_return(gpio_get_state(&s_board_type_address, &state));
  *board_type = (state == GPIO_STATE_HIGH) ? LIGHTS_CONFIG_BOARD_TYPE_FRONT :
                                             LIGHTS_CONFIG_BOARD_TYPE_REAR;

}

