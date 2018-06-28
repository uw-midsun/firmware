#include <stddef.h>
#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"

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

  // Init can <-> uart
  const CanUart *can_uart = driver_display_config_load_can_uart();
  uart_init(can_uart->uart, driver_display_config_load_uart(), &s_uart_storage);
  can_hw_init(driver_display_config_load_can());
  can_uart_init(can_uart);
  can_uart_enable_passthrough(can_uart);

  // Test calibration
  // TODO(ELEC-434): add persist layer so that the calibration does not need to be run everytime
  driver_display_calibration_init(driver_display_brightness_config_load(), &s_calibration_data,
                                  &s_calibration_storage);
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_UPPER_BOUND);
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_LOWER_BOUND);

  // Temp for debugging
  // printf("upper bound: %d \n", calibration_storage.data->max);
  // printf("lower bound: %d \n", calibration_storage.data->min);

  // Init brightness module
  driver_display_brightness_init(&s_brightness_storage, driver_display_brightness_config_load(),
                                 &s_calibration_data);

  while (true) {
    // If the photodiode information was unable to be read
    if (s_brightness_storage.reading_ok_flag == false) {
      LOG_CRITICAL("Failed to read photosensor ADC");
    }
  }
  return 0;
}
