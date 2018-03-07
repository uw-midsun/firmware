#pragma once

#include <stdint.h>

#include "can_uart.h"
#include "generic_can.h"
#include "status.h"

#define NUM_GENERIC_CAN_UART_RX_HANDLERS 5

typedef struct GenericCanUart {
  GenericCan base;
  CanUart* can_uart;
  GenericCanRxStorage rx_storage[NUM_GENERIC_CAN_UART_RX_HANDLERS];
} GenericCanUart;

StatusCode generic_can_uart_init(UARTPort port, GenericCanUart* out);
