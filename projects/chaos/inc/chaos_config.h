#pragma once

#include "gpio.h"
#include "power_path.h"

typedef struct ChaosConfig {
  PowerPathCfg power_path;
  // Roof
  const GPIOAddress telemetry_power;
  const GPIOAddress array_sense_power;
  const GPIOAddress rear_camera_power;
  // Front
  const GPIOAddress themis_power;
  const GPIOAddress driver_display_power;
  const GPIOAddress front_lights_power;
  // Misc
  const GPIOAddress battery_box_power;
  const GPIOAddress motor_interface_power;
  const GPIOAddress rear_lights_power;
  // Unused but available pins
  // const GPIOAddress spare_protected_power;
  // const GPIOAddress spare_unprotected_power
  //
  // Debug LEDs
  // const GPIOAddress led1_power;
  // const GPIOAddress led2_power;
  // const GPIOAddress led3_power;
} ChaosConfig;

// Returns a lazy static pointer to the global ChaosConfig struct.
ChaosConfig* chaos_config_load(void);
