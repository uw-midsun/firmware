#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio_config.h"
#include "lights_gpio_config_front.h"
#include "lights_gpio_config_rear.h"

static const LightsGpio *s_lights_gpio;

StatusCode lights_gpio_config_init(LightsConfigBoardType board_type) {
  s_lights_gpio = (board_type == LIGHTS_CONFIG_BOARD_TYPE_FRONT)
                      ? lights_gpio_config_front_load()
                      : lights_gpio_config_rear_load();
  return STATUS_CODE_OK;
}

LightsGpio *lights_gpio_config_load(void) {
  return s_lights_gpio;
}
