#include "charger_controller.h"

#include <stdbool.h>
#include <stdint.h>

#include "can_interval.h"
#include "charger_events.h"
#include "delay.h"
#include "event_queue.h"
#include "generic_can_hw.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CHARGER_MAX_VOLTAGE 100
#define TEST_CHARGER_MAX_CURRENT 200

// Opposite of the ones in the charger so that
#define TEST_CHARGER_EXPECTED_DLC 5
#define TEST_CHARGER_EXPECTED_TX_ID 0x18FF50E5
#define TEST_CHARGER_EXPECTED_RX_ID 0x1806E5F4

static uint8_t s_counter;
static GenericCanHw s_can;
static ChargerState s_expected_state;

static void prv_send_status(ChargerStatus status) {
  const ChargerControllerRxData tx_data = {
    .data_impl =
        {
            .voltage = TEST_CHARGER_MAX_VOLTAGE,
            .current = TEST_CHARGER_MAX_VOLTAGE,
            .status_flags =
                {
                    .raw = status.raw,
                },
        },
  };

  const GenericCanMsg response = {
    .id = TEST_CHARGER_EXPECTED_TX_ID,
    .dlc = TEST_CHARGER_EXPECTED_DLC,
    .extended = true,
    .data = tx_data.raw_data,
  };

  generic_can_tx((GenericCan *)&s_can, &response);
}

static void prv_rx_handler(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("CB triggered\n");
  ChargerStatus *status = context;
  ChargerControllerTxData data = { .raw_data = msg->data };
  TEST_ASSERT_EQUAL(TEST_CHARGER_EXPECTED_RX_ID, msg->id);
  TEST_ASSERT_EQUAL(TEST_CHARGER_MAX_CURRENT, data.data_impl.max_current);
  TEST_ASSERT_EQUAL(TEST_CHARGER_MAX_VOLTAGE, data.data_impl.max_voltage);
  TEST_ASSERT_EQUAL(s_expected_state, data.data_impl.charging);
  TEST_ASSERT_EQUAL(TEST_CHARGER_EXPECTED_DLC, msg->dlc);
  TEST_ASSERT_TRUE(msg->extended);

  prv_send_status(*status);

  s_counter++;
}

void setup_test(void) {
  s_counter = 0;
  s_expected_state = NUM_CHARGER_STATES;
  interrupt_init();
  gpio_init();
  soft_timer_init();
  can_interval_init();

  const CANHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  TEST_ASSERT_OK(generic_can_hw_init(&s_can, &can_hw_settings, CHARGER_EVENT_CAN_FAULT));
}

void teardown_test(void) {}

void test_charger_controller(void) {
  GenericCan *can = (GenericCan *)&s_can;
  ChargerStatus returned_status = { 0 };

  TEST_ASSERT_OK(generic_can_register_rx(can, prv_rx_handler, GENERIC_CAN_EMPTY_MASK,
                                         TEST_CHARGER_EXPECTED_RX_ID, true, &returned_status));

  ChargerSettings settings = {
    .max_voltage = TEST_CHARGER_MAX_VOLTAGE,
    .max_current = TEST_CHARGER_MAX_CURRENT,
    .can = can,  // Use pure HW can for both CAN and CAN UART since Extended support is needed.
    .can_uart = can,
  };

  Event e = { 0, 0 };
  ChargerStatus status = { 0 };

  // Starts in stop mode
  s_expected_state = CHARGER_STATE_STOP;
  TEST_ASSERT_OK(charger_controller_init(&settings, &status));

  // Let the callback trigger
  delay_ms(250);

  // Start the charger
  s_expected_state = CHARGER_STATE_START;
  TEST_ASSERT_OK(charger_controller_set_state(CHARGER_STATE_START));
  // Let the callback trigger
  delay_ms(250);

  s_expected_state = CHARGER_STATE_STOP;
  returned_status.hw_fault = true;
  prv_send_status(returned_status);
  // Let the send occur
  delay_ms(250);

  TEST_ASSERT_EQUAL(returned_status.raw, status.raw);
  delay_ms(250);

  // Prevent starting under a fault.
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, charger_controller_set_state(CHARGER_STATE_START));

  returned_status.hw_fault = false;
  prv_send_status(returned_status);
  // Let the send occur
  delay_ms(250);

  s_expected_state = CHARGER_STATE_START;
  TEST_ASSERT_OK(charger_controller_set_state(CHARGER_STATE_START));
  // Let the callback trigger twice
  delay_ms(1250);

  TEST_ASSERT_EQUAL(5, s_counter);
}

void test_charger_controller_status(void) {
  ChargerStatus status = {
    .hw_fault = false,
    .over_temp = false,
    .input_voltage = false,
  };

  TEST_ASSERT_TRUE(charger_controller_is_safe(status));
  // Check all combinations of the first 3 flags return false;
  for (uint8_t i = 1; i < (1U << 3U); i++) {
    LOG_DEBUG("%u\n", i);
    status.raw = i;
    TEST_ASSERT_FALSE(charger_controller_is_safe(status));
  }
}
