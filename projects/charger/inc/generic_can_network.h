#pragma once
// Module to abstract the network CAN variant such that it shares an interface with UART CAN.
// Requires network layer CAN to be running.

#include "generic_can.h"

typedef struct GenericCanNetwork {
  GenericCan base;
} GenericCanNetwork;

// Initializes the generic CAN for the hardware CAN.
StatusCode generic_can_network_init(GenericCanNetwork *can_network);

// NOTE: this utilizes **Network Layer CAN** not Hardware Layer! Support is limited to network layer
// rules! If you wish to send extended ID messages use the generic_can_hw module.
//
// For generic_can_register_rx |mask| is unused and |filter| is the expected message id.
