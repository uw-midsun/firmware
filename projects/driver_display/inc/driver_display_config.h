#pragma once
// Configuration module for the brightness module and photo sensor calibration module
// Defines screen constants and update periods to control frequency of brightness changes

#include "driver_display_brightness.h"

// Frequency for the screens (All the same frequency)
#define DRIVER_DISPLAY_CONFIG_SCREEN_FREQ_HZ 30000
// Period of time before sampling the photodiode information
#define DRIVER_DISPLAY_CONFIG_UPDATE_PERIOD_S 5

// Loads the screen settings to be used in brightness modules
const DriverDisplayBrightnessSettings *driver_display_config_load(void);
