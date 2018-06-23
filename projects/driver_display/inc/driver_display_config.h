#pragma once

#include "driver_display_brightness.h"

#define DRIVER_DISPLAY_CONFIG_SCREEN_FREQ_HZ 30000
#define DRIVER_DISPLAY_CONFIG_UPDATE_PERIOD_S 5

const DriverDisplayBrightnessSettings *driver_display_config_load(void);
