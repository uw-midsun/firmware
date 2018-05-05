#include "chaos_config.h"

#include "gpio_mcu.h"
#include "log.h"
#include "soft_timer.h"

static uint16_t prv_convert_aux_bat_current(uint16_t value) {
  LOG_DEBUG("Aux Bat Current: %d\n", value);
  return value;
}

static uint16_t prv_convert_aux_bat_voltage(uint16_t value) {
  LOG_DEBUG("Aux Bat Voltage: %d\n", value);
  return value;
}

static uint16_t prv_convert_dcdc_current(uint16_t value) {
  LOG_DEBUG("DCDC Current: %d\n", value);
  return value;
}

static uint16_t prv_convert_dcdc_voltage(uint16_t value) {
  LOG_DEBUG("DCDC Voltage: %d\n", value);
  return value;
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
