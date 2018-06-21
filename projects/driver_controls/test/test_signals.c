#include "hazards_fsm.h"
#include "headlight_fsm.h"
#include "horn_fsm.h"
#include "turn_signal_fsm.h"

typedef enum {
  TEST_SIGNALS_FSM_HEADLIGHT = 0,
  TEST_SIGNALS_FSM_TURN_SIGNALS,
  TEST_SIGNALS_FSM_HAZARDS,
  TEST_SIGNALS_FSM_HORN,
  NUM_TEST_SIGNALS_FSMS,
} TestSignalsFsm;

EventArbiterStorage s_arbiter_storage;
static FSM s_fsms[NUM_TEST_SIGNALS_FSMS];
static CANStorage s_can_storage;

static StatusCode prv_light_state_cb(const CANMessage *msg, void *context,
                                     CANAckStatus *ack_reply) {
  uint8_t light_id = 0, state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &light_id, &state);
  TEST_ASSERT_EQUAL(EE_LIGHT_TYPE_BRAKES, light_id);
  s_brake_state = state;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  TEST_ASSERT_OK(ret);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_STATE, prv_light_state_cb, NULL);

  event_arbiter_init(&s_arbiter_storage);
}

void teardown_test(void) {}

void test_signals_horn(void) {}
