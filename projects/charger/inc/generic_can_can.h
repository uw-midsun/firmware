#pragma once

#include "generic_can.h"

#define NUM_GENERIC_CAN_CAN_RX_HANDLERS 5

typedef struct GenericCanCan {
  GenericCan base;
  GenericCanRxStorage rx_storage[NUM_GENERIC_CAN_CAN_RX_HANDLERS];
} GenericCanCan;

StatusCode generic_can_can_init(GenericCanCan* out);
