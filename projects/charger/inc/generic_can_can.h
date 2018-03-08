#pragma once
// Module to abstract the hardware CAN variant such that it shares an interface with UART CAN.
// Requires can to be running.

#include "generic_can.h"

#define NUM_GENERIC_CAN_CAN_RX_HANDLERS 5

typedef struct GenericCanCan {
  GenericCan base;
  GenericCanRxStorage rx_storage[NUM_GENERIC_CAN_CAN_RX_HANDLERS];
} GenericCanCan;

// Initializes the generic CAN for the hardware CAN.
StatusCode generic_can_can_init(GenericCanCan* out);
