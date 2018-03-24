#include "generic_can_helpers.h"

#include <stdbool.h>
#include <stdint.h>

#include "generic_can.h"
#include "status.h"

StatusCode generic_can_helpers_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                                           void *context) {
  for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
    if (can->rx_storage[i].rx_handler == NULL && !can->rx_storage[i].enabled) {
      can->rx_storage[i].id = id;
      can->rx_storage[i].rx_handler = rx_handler;
      can->rx_storage[i].context = context;
      can->rx_storage[i].enabled = true;
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
}

StatusCode generic_can_helpers_set_rx(GenericCan *can, uint32_t id, bool state) {
  for (size_t i = 0; i < NUM_GENERIC_CAN_RX_HANDLERS; i++) {
    if (can->rx_storage[i].id == id && can->rx_storage[i].rx_handler != NULL) {
      can->rx_storage[i].enabled = state;
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_UNINITIALIZED);
}
