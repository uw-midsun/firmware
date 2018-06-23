#include <stddef.h>
#include <stdio.h>

#include "interrupt.h"

#include "driver_display_brightness.h"

int main(void) {
  // Init everything to be used
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_CONTINUOUS);

  // Populate data for brightness module (temporarily here before calibration is implemented)
  DriverDisplayBrightnessScreenData screen_data[] = {
    [DRIVER_DISPLAY_SCREEN1] = { .address = { GPIO_PORT_A, 7 },
                                 .settings = { .direction = GPIO_DIR_OUT,
                                               .state = GPIO_STATE_HIGH,
                                               .resistor = GPIO_RES_PULLUP,
                                               .alt_function = GPIO_ALTFN_4 },
                                 .timer = PWM_TIMER_14,
                                 .frequency_hz = DRIVER_DISPLAY_CONFIG_SCREEN1_FREQ_HZ },
    [DRIVER_DISPLAY_SCREEN2] = { .address = { GPIO_PORT_A, 4 },
                                 .settings = { .direction = GPIO_DIR_OUT,
                                               .state = GPIO_STATE_HIGH,
                                               .resistor = GPIO_RES_PULLUP,
                                               .alt_function = GPIO_ALTFN_4 },
                                 .timer = PWM_TIMER_14,
                                 .frequency_hz = DRIVER_DISPLAY_CONFIG_SCREEN2_FREQ_HZ }
  };

  DriverDisplayBrightnessSensorData sensor_data = { .address = { .port = GPIO_PORT_A, .pin = 0 },
                                                    .settings = { .direction = GPIO_DIR_IN,
                                                                  .state = GPIO_STATE_LOW,
                                                                  .resistor = GPIO_RES_NONE,
                                                                  .alt_function =
                                                                      GPIO_ALTFN_ANALOG },
                                                    .max = 4095,
                                                    .min = 0,
                                                    .percent_reading = 50 };

  DriverDisplayBrightnessData brightness_data = { .screen_data = screen_data,
                                                  .sensor_data = sensor_data };

  // Initialize the brightness module
  driver_display_brightness_init(&brightness_data);

  while (true) {
    // Do stuff
  }
  return 0;
}
