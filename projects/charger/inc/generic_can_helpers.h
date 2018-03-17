#pragma once
// Helpers for common code used in generic_can implementations.

#include <stdbool.h>
#include <stdint.h>

#include "generic_can.h"
#include "status.h"

// NOTE: Callers are expected to validate and sanitize |can| to avoid dereferencing null or other
// out of bounds memory!

// Registers |rx_handler| for |id| to |can| which will be passed |context| when triggered.
StatusCode generic_can_helpers_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                                           void *context);

// Sets |state| for the RX handler corresponding to |id| in |can|.
StatusCode generic_can_helpers_set_rx(GenericCan *can, uint32_t id, bool state);
