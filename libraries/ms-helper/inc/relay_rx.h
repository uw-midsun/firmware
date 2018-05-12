#pragma once
// Module for interfacing to CAN for CAN controlled relays.
//
// Requires CAN to be initialized.
//
// Allows registering a RelayRxHandler that is wrapped by a CANRxHandlerCb which is properly
// configured to unpack relay messages and handle faults. This RelayRxHandler is expected to execute
// the modifications to any GPIO pins or other mechanisms to trigger state changes to relays.

#include "can_msg_defs.h"
#include "exported_enums.h"
#include "status.h"

// Wrapped by a CANRxHandlerCb.
typedef StatusCode (*RelayRxHandler)(SystemCanMessage msg_id, EEChaosCmdRelayState state,
                                     void *context);

typedef struct RelayRxStorage {
  RelayRxHandler handler;
  SystemCanMessage msg_id;  // Unused but helpful in debugging.
  EEChaosCmdRelayState curr_state;
  void *context;
} RelayRxStorage;

// Initialize the relay module to use the supplied |relay_storage| of |size| to hold relay
// configurations.
StatusCode relay_rx_init(RelayRxStorage *relay_storage, size_t size);

// Relay RX init configures |RelayRxHandler| to be triggered when |msg_id| is received. This handler
// should alter a relay or relays to be in-keeping with the expected state. In the event of a
// failure the status code should propagate back to the CANRxHandler.
//
// NOTE: we explicitly don't constrain |msg_id|. In theory we could force this to be a value
// defined for one of the relays in codegen-tooling. But this module could be used for any CAN
// controlled element that uses a message format of a single uint8_t. Also note that the
// |relay_storage| is also extensible for this reason.
StatusCode relay_rx_configure_handler(SystemCanMessage msg_id, RelayRxHandler handler,
                                      void *context);