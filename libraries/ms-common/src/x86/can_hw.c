#include "can_hw.h"

StatusCode can_hw_init(const CANHwSettings *settings) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

// Registers a callback for the given event
StatusCode can_hw_register_callback(CANHwEvent event, CANHwEventHandlerCb callback, void *context) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

StatusCode can_hw_add_filter(uint16_t mask, uint16_t filter) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

CANHwBusStatus can_hw_bus_status(void) {
  return CAN_HW_BUS_STATUS_OFF;
}

StatusCode can_hw_transmit(uint16_t id, const uint8_t *data, size_t len) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(uint16_t *id, uint64_t *data, size_t *len) {
  return false;
}
