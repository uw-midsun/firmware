#pragma once
// Module used to control the brightness levels of the driver display screens.
// The module periodically samples adc values from the photosensor
// and adjusts the brightness levels accordingly through PWM
//
// This module requires pwm, gpios and soft timers to be initialized before it can be run.
// Calibration module must be run to update upper and lower bounds of calibration data
// Note that the brightness levels are bounded from 0-100% and values are capped to that range
// to ensure that we do not have over 100% brightness

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
  GpioAddress screen_address[NUM_DRIVER_DISPLAY_BRIGHTNESS_SCREENS];
  GpioAddress adc_address;
  PwmTimer timer;
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
  AdcChannel adc_channel;
  volatile bool reading_ok_flag;
} DriverDisplayBrightnessStorage;

// Initializes the brightness module
// GPIOs, PWM and ADC should be initialized beforehand.
// Controls all display screen brightness levels and automatically adjusts based on ambient
// brightness.
StatusCode driver_display_brightness_init(
    DriverDisplayBrightnessStorage *storage, const DriverDisplayBrightnessSettings *settings,
    const DriverDisplayBrightnessCalibrationData *calibration_data);
