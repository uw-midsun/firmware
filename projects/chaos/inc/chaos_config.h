#pragma once

#include "gpio.h"
#include "power_path.h"

#define CHAOS_CONFIG_GPIO_OPEN_DELAY_US 100
#define CHAOS_CONFIG_POWER_PATH_PERIOD_MS 2500

typedef struct ChaosConfig {
  PowerPathCfg power_path;
  // Roof
  const GpioAddress telemetry_power;
  const GpioAddress array_sense_power;
  const GpioAddress rear_camera_power;
  // Front
  const GpioAddress themis_power;
  const GpioAddress driver_display_power;
  const GpioAddress front_lights_power;
  // Misc
  const GpioAddress battery_box_power;
  const GpioAddress motor_interface_power;
  const GpioAddress rear_lights_power;
  const GpioAddress pjb_fan;
  const GpioAddress charger_power;
  // Unused but available pins
  const GpioAddress spare_protected_power;
  const GpioAddress spare_unprotected_power;
  // Debug LEDs
  // const GpioAddress led1_power;
  // const GpioAddress led2_power;
  // const GpioAddress led3_power;
} ChaosConfig;

// Returns a lazy static pointer to the global ChaosConfig struct.
ChaosConfig *chaos_config_load(void);
