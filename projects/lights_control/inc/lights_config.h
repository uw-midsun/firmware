#pragma once

// Config for various modules. Probes the GPIO to figure out board type (front or back).
// Needs GPIO to be initialized.

#include "status.h"
#include "can.h"
#include "lights_can.h"
#include "lights_can_config.h"
#include "lights_gpio.h"
#include "lights_gpio_config.h"
#include "lights_signal_fsm.h"
#include "lights_blinker.h"

typedef enum {
  LIGHTS_CONFIG_BOARD_TYPE_FRONT = 0,
  LIGHTS_CONFIG_BOARD_TYPE_REAR,
  NUM_LIGHTS_CONFIG_BOARD_TYPES
} LightsConfigBoardType;

typedef struct LightsConfig {
  const CANSettings *can_settings;
  const LightsCanSettings *lights_can_settings;
  const LightsGpio *lights_gpio;
  LightsSignalFsm *signal_fsm;
  LightsCanStorage *lights_can_storage;
  LightsBlinkerDuration signal_blinker_duration;
  uint32_t sync_count;
} LightsConfig;

StatusCode lights_config_init();

LightsConfig *lights_config_load();

