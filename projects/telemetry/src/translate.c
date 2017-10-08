#include "translate.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

static StatusCode prv_can_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  UARTPort uart = (UARTPort)context;
  uint8_t buffer[] = "RX[\0\0\0\0\0\0\0\0\0\0\0]\n";

  CANId id = {
    .source_id = msg->source_id,  //
    .msg_id = msg->msg_id,        //
    .type = msg->type,            //
  };

  memcpy(&buffer[3], &id.raw, sizeof(id.raw));
  memcpy(&buffer[5], &msg->dlc, 1);
  memcpy(&buffer[6], &msg->data, sizeof(msg->data));

  uart_tx(uart, buffer, sizeof(buffer) - 1);

  return STATUS_CODE_OK;
}

// Note that this function has not been tested.
void prv_uart_rx(const uint8_t *rx_arr, size_t len, void *context) {
  UARTPort uart = (UARTPort)context;

  CANMessage msg = {.type = CAN_MSG_TYPE_DATA };
  if (len == 15 && rx_arr[0] == 'T' && rx_arr[1] == 'X' && rx_arr[2] == '[' && rx_arr[14] == ']') {
    memcpy(&msg.msg_id, &rx_arr[3], sizeof(msg.msg_id));
    memcpy(&msg.dlc, &rx_arr[5], sizeof(msg.data));
    memcpy(&msg.data, &rx_arr[6], sizeof(msg.data));
    StatusCode ret = can_transmit(&msg, NULL);

    uint8_t buffer[] = "TX ret \0\n";
    memcpy(&buffer[7], &ret, 1);
    uart_tx(uart, buffer, sizeof(buffer) - 1);
    LOG_DEBUG("Requested CAN TX - msg %d: %d\n", msg.msg_id, ret);
  }
}

StatusCode translate_init(UARTPort uart) {
  can_register_rx_default_handler(prv_can_rx, (void *)uart);
  uart_set_rx_handler(uart, prv_uart_rx, (void *)uart);

  return STATUS_CODE_OK;
}
