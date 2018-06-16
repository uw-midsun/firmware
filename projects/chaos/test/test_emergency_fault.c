#include "emergency_fault.h"

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "chaos_events.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define NUM_CAN_RX_HANDLERS 2

static EmergencyFaultStorage s_em_storage;
static CANStorage s_storage;
static CANRxHandler s_rx_handlers[NUM_CAN_RX_HANDLERS];

// Handler that allows for injecting ack responses.
static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  (void)msg;
  CANAckStatus *status = context;
  *ack_reply = *status;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  emergency_fault_clear(&s_em_storage);
  interrupt_init();
  gpio_init();
  soft_timer_init();

  CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_storage, &settings, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
  emergency_fault_init(&s_em_storage);
}

void teardown_test(void) {}

// Tests emergency_fault_process_event which implicitly calls emergency_fault_send and
// emergency_fault_clear.
void test_emergency_fault(void) {
  CANAckStatus ack_status = CAN_ACK_STATUS_OK;
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_FAULT,
                                         prv_rx_handler, &ack_status));

  // Send once and ACK.
  Event e = { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY, .data = 0 };
  emergency_fault_process_event(&s_em_storage, &e);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  delay_ms(EMERGENCY_FAULT_BACKOFF_MS);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Fail once then succeed
  ack_status = CAN_ACK_STATUS_INVALID;
  e.id = CHAOS_EVENT_SEQUENCE_EMERGENCY;
  emergency_fault_process_event(&s_em_storage, &e);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  ack_status = CAN_ACK_STATUS_OK;
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  delay_ms(EMERGENCY_FAULT_BACKOFF_MS);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

  // Fail until cancelled.
  ack_status = CAN_ACK_STATUS_INVALID;
  e.id = CHAOS_EVENT_SEQUENCE_EMERGENCY;
  emergency_fault_process_event(&s_em_storage, &e);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CHAOS_EVENT_CAN_TX, CHAOS_EVENT_CAN_RX);
  e.id = CHAOS_EVENT_SEQUENCE_IDLE;
  emergency_fault_process_event(&s_em_storage, &e);
  delay_ms(EMERGENCY_FAULT_BACKOFF_MS * 3);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
