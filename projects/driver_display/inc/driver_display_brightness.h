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
  DRIVER_DISPLAY_BRIGHTNESS_NUM_SCREENS
} DriverDisplayBrightnessScreen;

typedef struct DriverDisplayBrightnessSettings{
  GPIOAddress* screen_address;
  GPIOAddress adc_address;
  PWMTimer timer;
  uint16_t frequency_hz;
  uint32_t update_period_s;
} DriverDisplayBrightnessSettings;

typedef struct DriverDisplayBrightnessCalibrationData{
  uint16_t max;
  uint16_t min;
} DriverDisplayBrightnessCalibrationData;

typedef struct DriverDisplayBrightnessStorage{
  DriverDisplayBrightnessSettings settings;
  DriverDisplayBrightnessCalibrationData calibration_data;
  uint16_t percent_reading;
  ADCChannel adc_channel;
} DriverDisplayBrightnessStorage;


DriverDisplayBrightnessStorage *driver_display_brightness_global(void);

StatusCode driver_display_brightness_init(DriverDisplayBrightnessStorage* storage, DriverDisplayBrightnessSettings settings,
  DriverDisplayBrightnessCalibrationData calibration_data);
