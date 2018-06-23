#pragma once
// Module used to control the brightness of the driver display screens.
// Also defines display screen parameters such as frequency and number of screens

#include "adc.h"
#include "gpio.h"
#include "pwm.h"
#include "soft_timer.h"
#include "status.h"

typedef enum {
  DRIVER_DISPLAY_BRIGHTNESS_SCREEN1 = 0,
  DRIVER_DISPLAY_BRIGHTNESS_SCREEN2,
  NUM_DRIVER_DISPLAY_BRIGHTNESS_SCREENS
} DriverDisplayBrightnessScreen;

typedef struct DriverDisplayBrightnessSettings {
  GPIOAddress screen_address[NUM_DRIVER_DISPLAY_BRIGHTNESS_SCREENS];
  GPIOAddress adc_address;
  PWMTimer timer;
  uint16_t frequency_hz;
  uint32_t update_period_s;
} DriverDisplayBrightnessSettings;

typedef struct DriverDisplayBrightnessCalibrationData {
  uint16_t max;
  uint16_t min;
} DriverDisplayBrightnessCalibrationData;

typedef struct DriverDisplayBrightnessStorage {
  DriverDisplayBrightnessSettings *settings;
  DriverDisplayBrightnessCalibrationData *calibration_data;
  ADCChannel adc_channel;
} DriverDisplayBrightnessStorage;

StatusCode driver_display_brightness_init(
    DriverDisplayBrightnessStorage *storage, const DriverDisplayBrightnessSettings *settings,
    const DriverDisplayBrightnessCalibrationData *calibration_data);
