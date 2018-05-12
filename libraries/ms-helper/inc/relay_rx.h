#pragma once
// Module for interfacing to CAN for CAN controlled relays.
// Requires CAN and GPIO to be initialized.

#include "can_msg_defs.h"
#include "exported_enums.h"
#include "gpio.h"
#include "status.h"

typedef struct RelayRxStorage {
  GPIOAddress relay_addr;
  SystemCanMessage msg_id;  // Unused but helpful in debugging.
  EEChaosCmdRelayState state;
} RelayRxStorage;

// Initialize the relay module to use the supplied |relay_storage| of |size| to hold relay
// configurations.
StatusCode relay_rx_init(RelayRxStorage *relay_storage, size_t size);

// Relay RX init configures |relay_addr| to control a relay local to a board in response to
// commands in the form of a CAN message. This initializer configures the pin, registers a CAN RX
// handler for the message, and handles internal state
//
// NOTE: we explicitly don't constrain |msg_id|. In theory we could force this to be a value
// defined for one of the relays in codegen-tooling. But this module could be used for any CAN
// controlled GPIO that uses a message format of a single uint8_t. Also note that the storage is
// also extensible for this reason.
StatusCode relay_rx_configure_handler(GPIOAddress relay_addr, SystemCanMessage msg_id);
