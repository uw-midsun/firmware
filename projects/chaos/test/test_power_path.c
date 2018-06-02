#include "power_path.h"

#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "delay.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_POWER_PATH_NUM_RX_HANDLERS 1

#define TEST_POWER_PATH_ADC_PERIOD_MS 50

#define TEST_POWER_PATH_AUX_CURRENT_VAL 1
#define TEST_POWER_PATH_AUX_UV_VAL 2
#define TEST_POWER_PATH_DCDC_CURRENT_VAL 3
#define TEST_POWER_PATH_DCDC_UV_VAL 4

static uint16_t prv_aux_current_convert(uint16_t value) {
  return TEST_POWER_PATH_AUX_CURRENT_VAL;
}

static uint16_t prv_aux_undervoltage_convert(uint16_t value) {
  return TEST_POWER_PATH_AUX_UV_VAL;
}

static uint16_t prv_dcdc_current_convert(uint16_t value) {
  return TEST_POWER_PATH_DCDC_CURRENT_VAL;
}

static uint16_t prv_dcdc_undervoltage_convert(uint16_t value) {
  return TEST_POWER_PATH_DCDC_UV_VAL;
}

static volatile uint8_t s_dcdc_ov = UINT8_MAX;
static volatile uint8_t s_dcdc_uv = UINT8_MAX;
static volatile uint8_t s_aux_ov = UINT8_MAX;
static volatile uint8_t s_aux_uv = UINT8_MAX;

static StatusCode prv_handle_uvov(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  LOG_DEBUG("Handled\n");
  CAN_UNPACK_OVUV_DCDC_AUX(msg, &s_dcdc_ov, &s_dcdc_uv, &s_aux_ov, &s_aux_uv);
  return STATUS_CODE_OK;
}

static PowerPathCfg s_ppc = { .enable_pin = { .port = 0, .pin = 0 },
                              .shutdown_pin = { .port = 0, .pin = 1 },
                              .aux_bat = { .id = POWER_PATH_SOURCE_ID_AUX_BAT,
                                           .uv_ov_pin = { .port = 0, .pin = 2 },
                                           .voltage_pin = { .port = 0, .pin = 3 },
                                           .current_pin = { .port = 0, .pin = 4 },
                                           .readings = { .voltage = 0, .current = 0 },
                                           .current_convert_fn = prv_aux_current_convert,
                                           .voltage_convert_fn = prv_aux_undervoltage_convert,
                                           .period_millis = 0,
                                           .timer_id = SOFT_TIMER_INVALID_TIMER,
                                           .monitoring_active = false },
                              .dcdc = { .id = POWER_PATH_SOURCE_ID_DCDC,
                                        .uv_ov_pin = { .port = 0, .pin = 5 },
                                        .voltage_pin = { .port = 0, .pin = 6 },
                                        .current_pin = { .port = 0, .pin = 7 },
                                        .readings = { .voltage = 0, .current = 0 },
                                        .current_convert_fn = prv_dcdc_current_convert,
                                        .voltage_convert_fn = prv_dcdc_undervoltage_convert,
                                        .period_millis = 0,
                                        .timer_id = SOFT_TIMER_INVALID_TIMER,
                                        .monitoring_active = false } };

static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[TEST_POWER_PATH_NUM_RX_HANDLERS];

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  adc_init(ADC_MODE_SINGLE);

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  TEST_ASSERT_OK(
      can_init(&can_settings, &s_can_storage, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers)));
  TEST_ASSERT_OK(power_path_init(&s_ppc));
}

void teardown_test(void) {}

void test_power_path_uv_ov(void) {
  volatile CANMessage rx_msg = { 0 };
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_OVUV_DCDC_AUX, prv_handle_uvov, &rx_msg);

  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.aux_bat, TEST_POWER_PATH_ADC_PERIOD_MS));
  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.dcdc, TEST_POWER_PATH_ADC_PERIOD_MS));

  delay_us(TEST_POWER_PATH_ADC_PERIOD_US + TEST_POWER_PATH_ADC_PERIOD_US / 10);

  gpio_it_trigger_interrupt(&s_ppc.aux_bat.uv_ov_pin);

  volatile Event e = { 0 };
  volatile StatusCode status = NUM_STATUS_CODES;

  // TX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  // RX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  TEST_ASSERT_EQUAL(false, s_dcdc_uv);
  TEST_ASSERT_EQUAL(false, s_dcdc_ov);
  TEST_ASSERT_EQUAL(true, s_aux_uv);
  TEST_ASSERT_EQUAL(false, s_aux_ov);

  gpio_it_trigger_interrupt(&s_ppc.dcdc.uv_ov_pin);

  // TX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
  // RX
  do {
    status = event_process(&e);
  } while (status != STATUS_CODE_OK);
  TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));

  TEST_ASSERT_EQUAL(true, s_dcdc_uv);
  TEST_ASSERT_EQUAL(false, s_dcdc_ov);
  TEST_ASSERT_EQUAL(false, s_aux_uv);
  TEST_ASSERT_EQUAL(false, s_aux_ov);

  TEST_ASSERT_OK(power_path_source_monitor_disable(&s_ppc.aux_bat));
  TEST_ASSERT_OK(power_path_source_monitor_disable(&s_ppc.dcdc));
}

void test_power_path_adcs(void) {
  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.aux_bat, TEST_POWER_PATH_ADC_PERIOD_US));
  TEST_ASSERT_OK(power_path_source_monitor_enable(&s_ppc.dcdc, TEST_POWER_PATH_ADC_PERIOD_US));

  delay_ms(TEST_POWER_PATH_ADC_PERIOD_MS + TEST_POWER_PATH_ADC_PERIOD_MS / 10);

  PowerPathVCReadings readings = { 0, 0 };
  TEST_ASSERT_OK(power_path_read_source(&s_ppc.aux_bat, &readings));
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_AUX_CURRENT_VAL, readings.current);
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_AUX_UV_VAL, readings.voltage);
  TEST_ASSERT_OK(power_path_read_source(&s_ppc.dcdc, &readings));
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_DCDC_CURRENT_VAL, readings.current);
  TEST_ASSERT_EQUAL(TEST_POWER_PATH_DCDC_UV_VAL, readings.voltage);

  TEST_ASSERT_OK(power_path_source_monitor_disable(&s_ppc.aux_bat));
  TEST_ASSERT_OK(power_path_source_monitor_disable(&s_ppc.dcdc));

  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, power_path_read_source(&s_ppc.aux_bat, &readings));
  TEST_ASSERT_EQUAL(STATUS_CODE_UNINITIALIZED, power_path_read_source(&s_ppc.dcdc, &readings));
}
