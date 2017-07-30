#include "gpio.h"
#include "soft_timer.h"
#include "interrupt.h"
#include "can.h"

#define CAN_TEST_NUM_RX_HANDLERS 5

typedef enum {
  CAN_TEST_EVENT_RX = 0,
  CAN_TEST_EVENT_TX,
  CAN_TEST_EVENT_FAULT
} CANTestEvent;

static GPIOAddress s_led = { GPIO_PORT_B, 3 };
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[CAN_TEST_NUM_RX_HANDLERS];

static StatusCode prv_handle_can_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  uint16_t *prev_msg = context;

  if (msg->msg_id != (*prev_msg + 1) % CAN_MSG_MAX_IDS) {
    printf("RX %d (expected %d)\n", msg->msg_id, (*prev_msg + 1) % CAN_MSG_MAX_IDS);
  }

  *prev_msg = msg->msg_id;
  // printf("RX %d\n", msg->msg_id, msg->source_id);
  // printf("> Data 0x%x%x\n", msg->data_u32[1], msg->data_u32[0]);

  return STATUS_CODE_OK;
}

static void prv_timeout_cb(SoftTimerID timer_id, void *context) {
  CANMessage *msg = context;
  msg->msg_id = (msg->msg_id + 1) % CAN_MSG_MAX_IDS;
  msg->data++;

  // printf("TX %d\n", msg->msg_id);
  StatusCode ret = can_transmit(msg, NULL);
  if (ret != STATUS_CODE_OK) {
    printf("> CAN failed to TX! - %d\n", ret);
  }

  soft_timer_start_millis(2, prv_timeout_cb, msg, NULL);
}

static void prv_hello_world(SoftTimerID timer_id, void *context) {
  printf("Hello - %d\n", *(uint16_t *)context);
  gpio_toggle_state(&s_led);
  soft_timer_start_seconds(1, prv_hello_world, context, NULL);
}

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW
  };
  gpio_init_pin(&s_led, &gpio_settings);

  CANSettings can_settings = {
    .device_id = 0x4,
    .bus_speed = 250,
    .rx_event = CAN_TEST_EVENT_RX,
    .tx_event = CAN_TEST_EVENT_TX,
    .fault_event = CAN_TEST_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&can_settings, &s_can_storage, s_rx_handlers, CAN_TEST_NUM_RX_HANDLERS);
  uint16_t prev_msg = 0;
  can_register_rx_default_handler(prv_handle_can_rx, &prev_msg);

  CANMessage msg = {
    .msg_id = 0x1,
    .dlc = 8,
    .data = 0
  };
  soft_timer_start_millis(1, prv_timeout_cb, &msg, NULL);
  soft_timer_start_seconds(1, prv_hello_world, &prev_msg, NULL);

  while (true) {
    Event e = { 0 };
    while (status_ok(event_process(&e))) {
      bool success = fsm_process_event(CAN_FSM, &e);
    }
  }

  return 0;
}
