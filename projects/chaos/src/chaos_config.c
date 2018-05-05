#include "chaos_config.h"

#include <inttypes.h>

#include "gpio_mcu.h"
#include "log.h"
#include "soft_timer.h"

// ##### Voltage Conversion #####
// 12 Bit ADC from 0 V to 3 V
#define CHAOS_CONFIG_VALUE_TO_MILLIVOLTS(val) ((val)*3000 / (((uint16_t)1 << 12) - 1))
#define CHAOS_CONFIG_VALUE_TO_MICROVOLTS(val) \
  (((uint32_t)(val)) * 3000000 / (((uint16_t)1 << 12) - 1))

// ##### Current Equation #####
// I = V / (Gain * R)

// DCDC:
// - R = 0.003 Ohms
// - Gain = 50
#define CHAOS_CONFIG_DCDC_CURRENT_CONVERT(val) (((uint32_t)(val)) * 20 / 3)

// AUX BAT:
// - R = 0.006 Ohms
// - Gain = 50
#define CHAOS_CONFIG_AUX_BAT_CURRENT_CONVERT(val) (((uint32_t)(val)) * 20 / 6)

// ##### Voltage Equation #####
// V_out = V_in * (R_1 + R_2) / R_1

// DCDC:
// - R_1 = 140 kOhm
// - R_2 = 470 kOhm
#define CHAOS_CONFIG_DCDC_VOLTAGE_CONVERT(val) (((uint32_t)(val)) * (470 + 140) / 140)

// AUX BAT:
// - R_1 = 140 kOhm
// - R_2 = 470 kOhm
#define CHAOS_CONFIG_AUX_BAT_VOLTAGE_CONVERT(val) (((uint32_t)(val)) * (470 + 140) / 140)

static uint16_t prv_convert_aux_bat_current(uint16_t value) {
  LOG_DEBUG("Aux Bat Current: %ld\n",
            CHAOS_CONFIG_AUX_BAT_CURRENT_CONVERT(CHAOS_CONFIG_VALUE_TO_MICROVOLTS(value)));
  return CHAOS_CONFIG_AUX_BAT_CURRENT_CONVERT(CHAOS_CONFIG_VALUE_TO_MICROVOLTS(value));
}

static uint16_t prv_convert_dcdc_current(uint16_t value) {
  LOG_DEBUG("DCDC Current: %ld\n",
            CHAOS_CONFIG_DCDC_CURRENT_CONVERT(CHAOS_CONFIG_VALUE_TO_MICROVOLTS(value)));
  return CHAOS_CONFIG_DCDC_CURRENT_CONVERT(CHAOS_CONFIG_VALUE_TO_MICROVOLTS(value));
}

static uint16_t prv_convert_aux_bat_voltage(uint16_t value) {
  LOG_DEBUG("Aux Bat Voltage: %ld\n",
            CHAOS_CONFIG_AUX_BAT_VOLTAGE_CONVERT(CHAOS_CONFIG_VALUE_TO_MILLIVOLTS(value)));
  return CHAOS_CONFIG_AUX_BAT_VOLTAGE_CONVERT(CHAOS_CONFIG_VALUE_TO_MILLIVOLTS(value));
}

static uint16_t prv_convert_dcdc_voltage(uint16_t value) {
  LOG_DEBUG("DCDC Voltage: %ld\n",
            CHAOS_CONFIG_DCDC_VOLTAGE_CONVERT(CHAOS_CONFIG_VALUE_TO_MILLIVOLTS(value)));
  return CHAOS_CONFIG_DCDC_VOLTAGE_CONVERT(CHAOS_CONFIG_VALUE_TO_MILLIVOLTS(value));
}

static ChaosConfig s_config = {
  .power_path = { .enable_pin = { GPIO_PORT_B, 8 },
                  .shutdown_pin = { GPIO_PORT_A, 8 },
                  .aux_bat =
                      {
                          .id = POWER_PATH_SOURCE_ID_AUX_BAT,
                          .uv_ov_pin = { GPIO_PORT_A, 9 },
                          .voltage_pin = { GPIO_PORT_A, 1 },
                          .current_pin = { GPIO_PORT_A, 3 },
                          .current_convert_fn = prv_convert_aux_bat_current,
                          .voltage_convert_fn = prv_convert_aux_bat_voltage,
                          .period_us = CHAOS_CONFIG_POWER_PATH_PERIOD_US,
                          .timer_id = SOFT_TIMER_INVALID_TIMER,
                      },
                  .dcdc =
                      {
                          .id = POWER_PATH_SOURCE_ID_DCDC,
                          .uv_ov_pin = { GPIO_PORT_A, 10 },
                          .voltage_pin = { GPIO_PORT_A, 0 },
                          .current_pin = { GPIO_PORT_A, 2 },
                          .current_convert_fn = prv_convert_dcdc_current,
                          .voltage_convert_fn = prv_convert_dcdc_voltage,
                          .period_us = CHAOS_CONFIG_POWER_PATH_PERIOD_US,
                          .timer_id = SOFT_TIMER_INVALID_TIMER,
                      } },
  .telemetry_power = { GPIO_PORT_B, 14 },
  .array_sense_power = { GPIO_PORT_B, 13 },
  .rear_camera_power = { GPIO_PORT_B, 12 },
  .themis_power = { GPIO_PORT_B, 11 },
  .driver_display_power = { GPIO_PORT_B, 10 },
  .front_lights_power = { GPIO_PORT_B, 2 },
  .battery_box_power = { GPIO_PORT_B, 0 },
  .motor_interface_power = { GPIO_PORT_A, 7 },
  .rear_lights_power = { GPIO_PORT_B, 9 },
  .pjb_fan = { GPIO_PORT_A, 6 },
  .spare_protected_power = { GPIO_PORT_B, 1 },
  .spare_unprotected_power = { GPIO_PORT_B, 9 },
};

ChaosConfig *chaos_config_load(void) {
  return &s_config;
}
