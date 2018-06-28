#include "driver_display_brightness_config.h"

static const DriverDisplayBrightnessSettings settings = {
  .screen_address = { [DRIVER_DISPLAY_BRIGHTNESS_SCREEN1] = { GPIO_PORT_A, 7 },
                      [DRIVER_DISPLAY_BRIGHTNESS_SCREEN2] = { GPIO_PORT_A, 4 } },
  .adc_address = { .port = GPIO_PORT_A, .pin = 0 },
  .timer = PWM_TIMER_14,
  .frequency_hz = DRIVER_DISPLAY_CONFIG_SCREEN_FREQ_HZ,
  .update_period_s = DRIVER_DISPLAY_CONFIG_UPDATE_PERIOD_S
};

const DriverDisplayBrightnessSettings *driver_display_brightness_config_load(void) {
  return &settings;
}
