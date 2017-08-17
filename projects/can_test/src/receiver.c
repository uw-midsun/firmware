#include "receiver.h"
#include "can.h"
#include "log.h"

static volatile uint32_t s_prev_val = 0;
static volatile uint32_t s_counter = 0;

static StatusCode prv_handle_can_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  s_counter++;

  if (msg->data_u32[0] != s_prev_val + 1) {
    printf("RX error (expected %d, got %d)\n", s_prev_val + 1, msg->data_u32[0]);
  }

  s_prev_val = msg->data_u32[0];

  return STATUS_CODE_OK;
}

static void prv_periodic_rx_cb(SoftTimerID timer_id, void *context) {
  printf("RX'd %d (%d)\n", s_counter, s_prev_val);
  s_counter = 0;

  soft_timer_start_seconds(1, prv_periodic_rx_cb, NULL, NULL);
}

void receiver_init(void) {
  can_register_rx_default_handler(prv_handle_can_rx, NULL);

  soft_timer_start_seconds(1, prv_periodic_rx_cb, NULL, NULL);
}