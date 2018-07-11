#include "solar_sense_config.h"
#include "log.h"

static SolarSenseConfig s_config = {
  .i2c_port = I2C_PORT_2,
};

// We'll ignore mode_sel_1 because there's only two solar masters.
static const GPIOAddress s_mode_sel_0 = { .port = GPIO_PORT_A, .pin = 9 };
static const GPIOAddress s_mode_sel_1 = { .port = GPIO_PORT_A, .pin = 10 };

StatusCode solar_sense_config_init(void) {
  // Getting board type.
  GPIOState state = NUM_GPIO_STATES;
  StatusCode status = gpio_get_state(&s_mode_sel_0, &state);
  if (!status_ok(status)) {
    LOG_DEBUG("Error getting board type.\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }
  s_config.board = (state == GPIO_STATE_HIGH) ? SOLAR_SENSE_CONFIG_BOARD_FRONT :
                                                 SOLAR_SENSE_CONFIG_BOARD_REAR;
  return STATUS_CODE_OK;
}


SolarSenseConfig *solar_sense_config_load() {
  return &s_config;
}


