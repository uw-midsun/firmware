#pragma once

// Module provider configurations for lights_can. Needs to be initialized first.
#include "can.h"
#include "gpio.h"

#include "lights_can.h"

// Loads configuration blob for lights_can.
const LightsCanSettings *lights_can_config_load(void);
