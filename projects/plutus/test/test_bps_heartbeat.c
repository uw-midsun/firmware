#include "bps_heartbeat.h"
#include "can.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "plutus_cfg.h"
#include "soft_timer.h"
#include "test_helpers.h"

#define TEST_BPS_HEARTBEAT_PERIOD_MS 50
#define TEST_BPS_HEARTBEAT_NUM_CAN_RX_HANDLERS 5

static CANStorage s_can;
static CANRxHandler s_rx_handlers[TEST_BPS_HEARTBEAT_NUM_CAN_RX_HANDLERS];
static BpsHeartbeatStorage s_bps_heartbeat;
static EERelayState s_relay_state;
static CANAckStatus s_ack_status;
static EEBpsHeartbeatState s_heartbeat_state;

StatusCode TEST_MOCK(sequenced_relay_set_state)(SequencedRelayStorage *storage,
                                                EERelayState state) {
  LOG_DEBUG("Relay state: %d\n", state);
  s_relay_state = state;

  return STATUS_CODE_OK;
}

static StatusCode prv_bps_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  *ack_reply = s_ack_status;

  uint8_t state = NUM_EE_BPS_HEARTBEAT_STATES;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &state);
  LOG_DEBUG("ACK request: BPS state %d\n", state);
  s_heartbeat_state = state;

  return STATUS_CODE_OK;
}

typedef enum {
  TEST_BPS_HEARTBEAT_EVENT_CAN_TX = 0,
  TEST_BPS_HEARTBEAT_EVENT_CAN_RX,
  TEST_BPS_HEARTBEAT_EVENT_CAN_FAULT,
} TestBpsHeartbeatEvent;

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_PLUTUS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_BPS_HEARTBEAT_EVENT_CAN_RX,
    .tx_event = TEST_BPS_HEARTBEAT_EVENT_CAN_TX,
    .fault_event = TEST_BPS_HEARTBEAT_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_can, &settings, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_bps_rx, NULL));

  // Closed relay for testing since we want to make sure it opens on fault.
  s_relay_state = EE_RELAY_STATE_CLOSE;
  s_ack_status = CAN_ACK_STATUS_OK;
  s_heartbeat_state = NUM_EE_BPS_HEARTBEAT_STATES;
  bps_heartbeat_init(&s_bps_heartbeat, NULL, TEST_BPS_HEARTBEAT_PERIOD_MS,
                     CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_PLUTUS));
}

void teardown_test(void) {}

void test_bps_heartbeat_can(void) {
  // No faults by default - the heartbeat state should be good
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BPS_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BPS_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, s_ack_status);
  TEST_ASSERT_EQUAL(EE_RELAY_STATE_CLOSE, s_relay_state);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_OK, s_heartbeat_state);

  // Pretend something bad happened
  // We support a number of grace ACK timeouts, so loop
  s_ack_status = CAN_ACK_STATUS_TIMEOUT;

  for (size_t i = 0; i < PLUTUS_CFG_HEARTBEAT_MAX_ACK_FAILS; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BPS_HEARTBEAT_EVENT_CAN_TX,
                                      TEST_BPS_HEARTBEAT_EVENT_CAN_RX);
  }

  // Next BPS heartbeat will have faulted
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BPS_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BPS_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_TIMEOUT, s_ack_status);
  TEST_ASSERT_EQUAL(EE_RELAY_STATE_OPEN, s_relay_state);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_FAULT, s_heartbeat_state);
}

void test_bps_heartbeat_basic(void) {
  // No faults by default - the heartbeat state should be good
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BPS_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BPS_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_OK, s_ack_status);
  TEST_ASSERT_EQUAL(EE_RELAY_STATE_CLOSE, s_relay_state);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_OK, s_heartbeat_state);

  // Raise fault - immediately update
  LOG_DEBUG("Raising fault\n");
  bps_heartbeat_raise_fault(&s_bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BPS_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BPS_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_RELAY_STATE_OPEN, s_relay_state);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_FAULT, s_heartbeat_state);

  // Try clearing the fault
  LOG_DEBUG("Clearing fault\n");
  bps_heartbeat_clear_fault(&s_bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  delay_ms(TEST_BPS_HEARTBEAT_PERIOD_MS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_BPS_HEARTBEAT_EVENT_CAN_TX,
                                    TEST_BPS_HEARTBEAT_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(EE_RELAY_STATE_OPEN, s_relay_state);
  TEST_ASSERT_EQUAL(EE_BPS_HEARTBEAT_STATE_OK, s_heartbeat_state);
}
