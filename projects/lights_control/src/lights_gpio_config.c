#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio_config.h"

static const LightsGPIO *s_lights_gpio;

StatusCode lights_config_init(LightsGPIOConfigBoardType board_type) {
  s_lights_gpio = (board_type == LIGHTS_GPIO_CONFIG_BOARD_TYPE_FRONT) ? lights_config_front_load()
                                                            : lights_config_rear_load();
  return STATUS_CODE_OK;
}

LightsGPIO* lights_config_load(void) {
  return s_lights_gpio;
}

