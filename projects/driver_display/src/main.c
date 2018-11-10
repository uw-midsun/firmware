#include <stddef.h>
#include <stdio.h>

#include "crc32.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "persist.h"

#include "driver_display_brightness.h"
#include "driver_display_brightness_config.h"
#include "driver_display_calibration.h"
#include "driver_display_config.h"

static UartStorage s_uart_storage;
static PersistStorage s_persist_storage;
static DriverDisplayCalibrationStorage s_calibration_storage;
static DriverDisplayBrightnessCalibrationData s_calibration_data;
static DriverDisplayBrightnessStorage s_brightness_storage;

int main(void) {
  // Init everything to be used
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_CONTINUOUS);

  crc32_init();
  flash_init();

  // Init can <-> uartp
  const CanUart *can_uart = driver_display_config_load_can_uart();
  uart_init(can_uart->uart, driver_display_config_load_uart(), &s_uart_storage);
  can_hw_init(driver_display_config_load_can());
  can_uart_init(can_uart);
  can_uart_enable_passthrough(can_uart);

  FlashCalibrationStorage flash_storage = { 0 };

  // Load data from persist layer
  persist_init(&s_persist_storage, CALIBRATION_FLASH_PAGE, &flash_storage, sizeof(flash_storage), false);
  persist_ctrl_periodic(&s_persist_storage, false);

  LOG_DEBUG("Loaded settings max: %d min: %d\n", flash_storage.data.max, flash_storage.data.min);

  s_calibration_data.max = flash_storage.data.max;
  s_calibration_data.min = flash_storage.data.min;

#ifndef DRIVER_DISPLAY_CONFIG_DISABLE_CALIBRATION
  driver_display_calibration_init(driver_display_brightness_config_load(), &s_calibration_data,
                                  &s_calibration_storage);
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_UPPER_BOUND);
  driver_display_calibration_bounds(&s_calibration_storage, DRIVER_DISPLAY_CALIBRATION_LOWER_BOUND);
#endif
  
  // Init brightness module
  driver_display_brightness_init(&s_brightness_storage, driver_display_brightness_config_load(),
                                 &s_calibration_data);

  while (true) {
    // If the photodiode information was unable to be read
    if (s_brightness_storage.reading_ok_flag == false) {
      LOG_DEBUG("Failed to read photosensor ADC");
    }
  }
  return 0;
}
