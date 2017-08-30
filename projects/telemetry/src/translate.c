#include "translate.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define TRANSLATE_BUFFER_LEN 64

static StatusCode prv_can_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  // Note that this currently responds to ACKs
  UARTPort uart = (UARTPort)context;
  char buffer[TRANSLATE_BUFFER_LEN] = { 0 };

  CANId id = {
    .source_id = msg->source_id,  //
    .msg_id = msg->msg_id,        //
    .type = msg->type,            //
  };

  size_t buffer_tx_len =
      (size_t)snprintf(buffer, TRANSLATE_BUFFER_LEN, "RX[%" PRIu16 " %" PRIu32 " %" PRIu32 "]\n",
                       id.raw, msg->data_u32[1], msg->data_u32[0]);
  uart_tx(uart, (uint8_t *)buffer, buffer_tx_len);

  return STATUS_CODE_OK;
}

void prv_uart_rx(const uint8_t *rx_arr, size_t len, void *context) {
  CANMessage msg = {.type = CAN_MSG_TYPE_DATA };
  int success = sscanf((const char *)rx_arr, "TX[%" PRIu16 " %" PRIu32 " %" PRIu32 "]", &msg.msg_id,
                       &msg.data_u32[1], &msg.data_u32[0]);
  if (success == 3) {
    printf("TX: msg_id: %" PRIu16 " data 0x%" PRIx32 "%" PRIx32 "\n", msg.msg_id, msg.data_u32[1],
           msg.data_u32[0]);
    can_transmit(&msg, NULL);
  }
}

StatusCode translate_init(UARTPort uart) {
  can_register_rx_default_handler(prv_can_rx, (void *)uart);
  uart_set_rx_handler(uart, prv_uart_rx, NULL);

  return STATUS_CODE_OK;
}
