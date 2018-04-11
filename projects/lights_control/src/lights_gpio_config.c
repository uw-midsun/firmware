#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio_config.h"

LightsConfig *lights_config_front_load(void);
LightsConfig *lights_config_rear_load(void);

static LightsConfig *s_config;

StatusCode lights_config_init(LightsConfigBoardDescriptor get_board_type) {
  LightsConfigBoardType board_type;
  status_ok_or_return(get_board_type(&board_type));
  s_config = (board_type == LIGHTS_CONFIG_BOARD_TYPE_FRONT) ? lights_config_front_load()
                                                            : lights_config_rear_load();
  return STATUS_CODE_OK;
}

LightsConfig *lights_config_load(void) {
  return s_config;
}
