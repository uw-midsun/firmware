#include "chaos_config.h"

#include <inttypes.h>

#include "gpio_mcu.h"
#include "log.h"
#include "soft_timer.h"

#define CHAOS_CONFIG_VALUE_TO_MICROVOLTS(millivolts) ((millivolts)*1000)

// ##### Current Equation #####
// I = V / (Gain * R)

// DCDC:
// - 80 mV / A = 80 uV / mA
#define CHAOS_CONFIG_DCDC_CURRENT_CONVERT(val) (((uint32_t)(val)) / 80)

// AUX BAT:
// - 300 mV / A = 300 uV / maA
#define CHAOS_CONFIG_AUX_BAT_CURRENT_CONVERT(val) (((uint32_t)(val)) / 300)

// DCDC:
// Calibrated at 12V
#define CHAOS_CONFIG_DCDC_VOLTAGE_CONVERT(val) (((uint32_t)(val)) * 531 / 100)

// AUX BAT:
// Calibrated at 12V
#define CHAOS_CONFIG_AUX_BAT_VOLTAGE_CONVERT(val) (((uint32_t)(val)) * 532 / 100)

static uint16_t prv_convert_aux_bat_current(uint16_t value) {
  return CHAOS_CONFIG_AUX_BAT_CURRENT_CONVERT(CHAOS_CONFIG_VALUE_TO_MICROVOLTS(value));
}

static uint16_t prv_convert_dcdc_current(uint16_t value) {
  return CHAOS_CONFIG_DCDC_CURRENT_CONVERT(CHAOS_CONFIG_VALUE_TO_MICROVOLTS(value));
}

static uint16_t prv_convert_aux_bat_voltage(uint16_t value) {
  return CHAOS_CONFIG_AUX_BAT_VOLTAGE_CONVERT(value);
}

static uint16_t prv_convert_dcdc_voltage(uint16_t value) {
  return CHAOS_CONFIG_DCDC_VOLTAGE_CONVERT(value);
}

static uint16_t prv_convert_dcdc_temp(uint16_t value) {
  // TODO(ECE-626): Should convert value to actual ...
  return value;
}

// clang-format off
static ChaosConfig s_config = {
  .power_path = {
    .enable_pin = { GPIO_PORT_B, 8 },
    .shutdown_pin = { GPIO_PORT_A, 8 },
    .aux_bat = {
      .id = POWER_PATH_SOURCE_ID_AUX_BAT,
      .uv_ov_pin = { GPIO_PORT_A, 9 },
      .voltage_pin = { GPIO_PORT_A, 1 },
      .current_pin = { GPIO_PORT_A, 3 },
      .current_convert_fn = prv_convert_aux_bat_current,
      .voltage_convert_fn = prv_convert_aux_bat_voltage,
      .period_millis = CHAOS_CONFIG_POWER_PATH_PERIOD_MS,
      .timer_id = SOFT_TIMER_INVALID_TIMER,
    },
    .dcdc = {
      .id = POWER_PATH_SOURCE_ID_DCDC,
      .uv_ov_pin = { GPIO_PORT_A, 10 },
      .voltage_pin = { GPIO_PORT_A, 0 },
      .current_pin = { GPIO_PORT_A, 2 },
      .temperature1_pin = {GPIO_PORT_A, 4},
      .temperature2_pin = {GPIO_PORT_A, 5},
      .current_convert_fn = prv_convert_dcdc_current,
      .voltage_convert_fn = prv_convert_dcdc_voltage,
      .temperature_convert_fn = prv_convert_dcdc_temp,
      .period_millis = CHAOS_CONFIG_POWER_PATH_PERIOD_MS,
      .timer_id = SOFT_TIMER_INVALID_TIMER,
    }
  },
  .charger_power = {GPIO_PORT_B, 5},
  .telemetry_power = { GPIO_PORT_B, 14 },
  .array_sense_power = { GPIO_PORT_B, 13 },
  .rear_camera_power = { GPIO_PORT_B, 12 },
  .themis_power = { GPIO_PORT_B, 11 },
  .driver_display_power = { GPIO_PORT_B, 10 },
  .front_lights_power = { GPIO_PORT_B, 2 },
  .battery_box_power = { GPIO_PORT_B, 0 },
  .motor_interface_power = { GPIO_PORT_A, 7 },
  // should have always been 15 was previously pointing to a spare ...?
  .rear_lights_power = { GPIO_PORT_B, 15 }, // REMAP
  .pjb_fan = { GPIO_PORT_B, 9 }, // REMAP

  .spare_protected_power1 = { GPIO_PORT_A, 6 }, // REMAP
  .spare_protected_power2 = { GPIO_PORT_B, 4 }, // NEW
  .spare_unprotected_power1 = { GPIO_PORT_B, 1 }, // REMAP
  .spare_unprotected_power2 = { GPIO_PORT_C, 13 }, // NEW
};
// clang-format on

ChaosConfig *chaos_config_load(void) {
  return &s_config;
}
