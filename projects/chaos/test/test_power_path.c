#include "power_path.h"

#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "chaos_events.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_POWER_PATH_ADC_PERIOD_US 1000

#define TEST_POWER_PATH_AUX_CURRENT_VAL 1
#define TEST_POWER_PATH_AUX_VOLTAGE_VAL 2
#define TEST_POWER_PATH_DCDC_CURRENT_VAL 3
#define TEST_POWER_PATH_DCDC_VOLTAGE_VAL 4

static uint16_t prv_aux_current_convert(uint16_t value) {
  return TEST_POWER_PATH_AUX_CURRENT_VAL;
}

static uint16_t prv_aux_voltage_convert(uint16_t value) {
  return TEST_POWER_PATH_AUX_VOLTAGE_VAL;
}

static uint16_t prv_dcdc_current_convert(uint16_t value) {
  return TEST_POWER_PATH_DCDC_CURRENT_VAL;
}

static uint16_t prv_dcdc_voltage_convert(uint16_t value) {
  return TEST_POWER_PATH_DCDC_VOLTAGE_VAL;
}

static PowerPathCfg s_ppc = { .enable_pin = { .port = 0, .pin = 0 },
                              .shutdown_pin = { .port = 0, .pin = 1 },
                              .aux_bat = { .id = POWER_PATH_SOURCE_ID_AUX_BAT,
                                           .uv_ov_pin = { .port = 0, .pin = 2 },
                                           .voltage_pin = { .port = 0, .pin = 3 },
                                           .current_pin = { .port = 0, .pin = 4 },
                                           .readings = { .voltage = 0, .current = 0 },
                                           .current_convert = prv_aux_current_convert,
                                           .voltage_convert = prv_aux_voltage_convert,
                                           .period_us = 0,
                                           .timer_id = SOFT_TIMER_INVALID_TIMER,
                                           .monitoring_active = false },
                              .dcdc = { .id = POWER_PATH_SOURCE_ID_DCDC,
                                        .uv_ov_pin = { .port = 0, .pin = 5 },
                                        .voltage_pin = { .port = 0, .pin = 6 },
                                        .current_pin = { .port = 0, .pin = 7 },
                                        .readings = { .voltage = 0, .current = 0 },
                                        .current_convert = prv_dcdc_current_convert,
                                        .voltage_convert = prv_dcdc_voltage_convert,
                                        .period_us = 0,
                                        .timer_id = SOFT_TIMER_INVALID_TIMER,
                                        .monitoring_active = false } };

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  adc_init(ADC_MODE_SINGLE);
  TEST_ASSERT_OK(power_path_init(&s_ppc));
}

void teardown_test(void) {}

void test_power_path_uv_ov(void) {
  TEST_ASSERT_OK(power_path_enable(&s_ppc));

  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.aux_bat, TEST_POWER_PATH_ADC_PERIOD_US));
  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.dcdc, TEST_POWER_PATH_ADC_PERIOD_US));

  gpio_it_trigger_interrupt(&s_ppc.aux_bat.uv_ov_pin);
  Event e = { 0, 0 };
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_UV_OV, e.id);
  TEST_ASSERT_EQUAL(POWER_PATH_SOURCE_ID_AUX_BAT, e.data);

  gpio_it_trigger_interrupt(&s_ppc.dcdc.uv_ov_pin);
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(CHAOS_EVENT_CAN_UV_OV, e.id);
  TEST_ASSERT_EQUAL(POWER_PATH_SOURCE_ID_DCDC, e.data);

  TEST_ASSERT_OK(power_path_disable(&s_ppc));
}

void test_power_path_adcs(void) {
  TEST_ASSERT_OK(power_path_enable(&s_ppc));

  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.aux_bat, TEST_POWER_PATH_ADC_PERIOD_US));
  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.dcdc, TEST_POWER_PATH_ADC_PERIOD_US));

  delay_us(TEST_POWER_PATH_ADC_PERIOD_US + TEST_POWER_PATH_ADC_PERIOD_US / 10);

  PowerPathVCReadings readings = { 0, 0 };
  TEST_ASSERT_OK(power_path_read_source(&s_ppc.aux_bat, &readings));
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_AUX_CURRENT_VAL, readings.current);
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_AUX_VOLTAGE_VAL, readings.voltage);
  TEST_ASSERT_OK(power_path_read_source(&s_ppc.dcdc, &readings));
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_DCDC_CURRENT_VAL, readings.current);
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_DCDC_VOLTAGE_VAL, readings.voltage);

  TEST_ASSERT_OK(power_path_source_monitor_disable(&s_ppc.aux_bat));
  TEST_ASSERT_OK(power_path_source_monitor_disable(&s_ppc.dcdc));

  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, power_path_read_source(&s_ppc.aux_bat, &readings));
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, power_path_read_source(&s_ppc.dcdc, &readings));

  TEST_ASSERT_OK(power_path_disable(&s_ppc));
}
