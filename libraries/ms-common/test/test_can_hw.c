#include "can_hw.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

static volatile size_t s_msg_rx;
static volatile uint16_t s_rx_id;
static volatile uint64_t s_rx_data;
static volatile size_t s_rx_len;

static void prv_handle_rx(void *context) {
  while (can_hw_receive(&s_rx_id, &s_rx_data, &s_rx_len)) {
    s_msg_rx++;
  }
}

static void prv_wait_rx(size_t wait_for) {
  size_t expected = s_msg_rx + wait_for;

  while (s_msg_rx != expected) {
  }
}

void setup_test(void) {
  interrupt_init();

  CANHwSettings can_settings = {
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .loopback = true,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_hw_init(&can_settings);
  can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_handle_rx, NULL);
  s_msg_rx = 0;
  s_rx_id = 0;
  s_rx_data = 0;
  s_rx_len = 0;
  LOG_DEBUG("CAN initialized\n");
}

void teardown_test(void) {}

void test_can_hw_loop(void) {
  uint16_t tx_id = 0x01;
  uint64_t tx_data = 0x1122334455667788;
  size_t tx_len = 8;

  StatusCode ret = can_hw_transmit(tx_id, (uint8_t *)&tx_data, tx_len);
  TEST_ASSERT_OK(ret);

  prv_wait_rx(1);

  TEST_ASSERT_EQUAL(tx_id, s_rx_id);
  TEST_ASSERT_EQUAL(tx_data, s_rx_data);
  TEST_ASSERT_EQUAL(tx_len, s_rx_len);
}

void test_can_hw_filter(void) {
  // Mask 0b11, require 0b01
  can_hw_add_filter(0x03, 0x01);

  // 0b0011 - fail
  StatusCode ret = can_hw_transmit(0x3, 0, 0);
  TEST_ASSERT_OK(ret);

  // 0b0101 - pass
  ret = can_hw_transmit(0x5, 0, 0);
  TEST_ASSERT_OK(ret);

  // 0b00111001 - pass
  ret = can_hw_transmit(0x39, 0, 0);
  TEST_ASSERT_OK(ret);

  prv_wait_rx(2);

  TEST_ASSERT_EQUAL(2, s_msg_rx);
  TEST_ASSERT_EQUAL(0x39, s_rx_id);
}
