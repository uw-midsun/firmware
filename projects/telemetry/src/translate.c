#include "translate.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static StatusCode prv_can_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  // Note that this currently responds to ACKs
  UARTPort uart = (UARTPort)context;
  uint8_t buffer[] = "RX[\0\0\0\0\0\0\0\0\0\0]\n";

  CANId id = {
    .source_id = msg->source_id,  //
    .msg_id = msg->msg_id,        //
    .type = msg->type,            //
  };

  memcpy(&buffer[3], &id.raw, sizeof(id.raw));
  memcpy(&buffer[5], &msg->data, sizeof(msg->data));

  uart_tx(uart, buffer, sizeof(buffer) - 1);

  return STATUS_CODE_OK;
}

void prv_uart_rx(const uint8_t *rx_arr, size_t len, void *context) {
  CANMessage msg = {.type = CAN_MSG_TYPE_DATA };
  if (len == 14 && rx_arr[0] == 'T' && rx_arr[1] == 'X' && rx_arr[2] == '[' && rx_arr[13] == ']') {
    memcpy(&msg.msg_id, &rx_arr[3], sizeof(msg.msg_id));
    memcpy(&msg.data, &rx_arr[5], sizeof(msg.data));
    can_transmit(&msg, NULL);
  }
}

StatusCode translate_init(UARTPort uart) {
  can_register_rx_default_handler(prv_can_rx, (void *)uart);
  uart_set_rx_handler(uart, prv_uart_rx, NULL);

  return STATUS_CODE_OK;
}
