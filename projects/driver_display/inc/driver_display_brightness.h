#pragma once
// Module used to control the brightness of the driver display screens.
// Also defines display screen parameters such as frequency and number of screens

#include "adc.h"
#include "gpio.h"
#include "pwm.h"
#include "soft_timer.h"

#define DRIVER_DISPLAY_CONFIG_NUM_SCREENS 2

#define DRIVER_DISPLAY_CONFIG_SCREEN1_FREQ_HZ 30000  // Frequency in Hz (subject to change)
#define DRIVER_DISPLAY_CONFIG_SCREEN2_FREQ_HZ 30000  // Frequency in Hz (subject to change)

#define DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD 5

typedef enum {
  DRIVER_DISPLAY_SCREEN1 = 0,
  DRIVER_DISPLAY_SCREEN2,
} DriverDisplayBrightnessScreen;

typedef struct {
  GPIOAddress address;
  GPIOSettings settings;
  PWMTimer timer;
  uint16_t frequency_hz;  // frequency in Hz
} DriverDisplayBrightnessScreenData;

typedef struct {
  GPIOAddress address;
  GPIOSettings settings;
  ADCChannel channel;
  uint16_t max;
  uint16_t min;
  uint16_t percent_reading;
} DriverDisplayBrightnessSensorData;

typedef struct {
  DriverDisplayBrightnessScreenData *screen_data;
  DriverDisplayBrightnessSensorData sensor_data;
} DriverDisplayBrightnessData;

void driver_display_brightness_init(DriverDisplayBrightnessData *brightness_data);
