#include "sender.h"
#include <inttypes.h>
#include "can.h"
#include "log.h"
#include "soft_timer.h"

typedef struct {
  uint32_t timeout_us;
  uint32_t last_success;
  CANMessage msg;
} SenderStorage;

static volatile SenderStorage s_sender = { 0 };

static void prv_tx_cb(SoftTimerId timer_id, void *context) {
  StatusCode ret = can_transmit(&s_sender.msg, NULL);
  if (status_ok(ret)) {
    s_sender.msg.data_u32[0]++;
  }

  soft_timer_start(s_sender.timeout_us, prv_tx_cb, NULL, NULL);
}

static void prv_periodic_tx_cb(SoftTimerId timer_id, void *context) {
  uint32_t diff = s_sender.msg.data_u32[0] - s_sender.last_success;
  s_sender.last_success = s_sender.msg.data_u32[0];

  printf("TX'd %" PRIu32 " (%" PRIu32 ")\n", diff, s_sender.last_success);

  soft_timer_start_seconds(1, prv_periodic_tx_cb, NULL, NULL);
}

void sender_init(uint16_t msg_id) {
  s_sender.msg.msg_id = msg_id;
  s_sender.msg.dlc = 8;
  s_sender.timeout_us = 100;
  soft_timer_start(s_sender.timeout_us, prv_tx_cb, NULL, NULL);
  soft_timer_start_seconds(1, prv_periodic_tx_cb, NULL, NULL);
}
