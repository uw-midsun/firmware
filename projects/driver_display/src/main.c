#include <stddef.h>
#include <stdio.h>

#include "delay.h"
#include "interrupt.h"

#include "driver_display_brightness.h"
#include "driver_display_brightness_config.h"
#include "driver_display_calibration.h"
#include "driver_display_config.h"

static UARTStorage s_uart_storage;
static DriverDisplayCalibrationStorage s_calibration_storage;
static DriverDisplayBrightnessCalibrationData s_calibration_data;
static DriverDisplayBrightnessStorage s_brightness_storage;

int main(void) {
  // Init everything to be used
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_CONTINUOUS);

  // Test can <-> uart
  uart_init(DRIVER_DISPLAY_CONFIG_UART_PORT, driver_display_config_load_uart(), &s_uart_storage);
  can_hw_init(driver_display_config_load_can());
  can_uart_init(driver_display_config_load_can_uart());
  can_uart_enable_passthrough(driver_display_config_load_can_uart());

  // Test calibration
  driver_display_calibration_init(driver_display_brightness_config_load(), &s_calibration_data,
                                  &s_calibration_storage);
  driver_display_calibration_lower_bound(&s_calibration_storage);
  driver_display_calibration_upper_bound(&s_calibration_storage);

  // Temp for debugging
  // printf("upper bound: %d \n", calibration_storage.data->max);
  // printf("lower bound: %d \n", calibration_storage.data->min);

  // Test brightness module
  driver_display_brightness_init(&s_brightness_storage, driver_display_brightness_config_load(),
                                 &s_calibration_data);

  while (true) {
    // Do stuff
  }
  return 0;
}
