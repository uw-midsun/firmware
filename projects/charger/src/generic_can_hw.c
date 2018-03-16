#include "generic_can_hw.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can_hw.h"
#include "event_queue.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

#define CAN_BUS_OFF_RECOVERY_TIME_MS 500

static GenericCanInterface s_interface;

// CANHwEventHandlerCb: Rx Occurred
static void prv_rx_handler(void *context) {
  GenericCanHw *gch = (GenericCanHw *)context;
  GenericCanMsg rx_msg = { 0 };
  while (can_hw_receive(&rx_msg.id, &rx_msg.extended, &rx_msg.data, &rx_msg.dlc)) {
    for (size_t i = 0; i < NUM_GENERIC_CAN_HW_RX_HANDLERS; i++) {
      if (rx_msg.id == gch->rx_storage[i].id && gch->rx_storage[i].rx_handler != NULL &&
          gch->rx_storage[i].enabled) {
        gch->rx_storage[i].rx_handler(&rx_msg, gch->rx_storage[i].context);
        break;
      }
    }
  }
}

// CANHwEventHandlerCb: Fault Occurred
static void prv_bus_error_timeout_handler(SoftTimerID timer_id, void *context) {
  (void)timer_id;
  GenericCanHw *gch = context;

  // Note that bus errors have never been tested.
  CANHwBusStatus status = can_hw_bus_status();

  if (status == CAN_HW_BUS_STATUS_OFF) {
    event_raise(gch->fault_event, 0);
  }
}

static void prv_bus_error_handler(void *context) {
  GenericCanHw *gch = context;

  soft_timer_start_millis(CAN_BUS_OFF_RECOVERY_TIME_MS, prv_bus_error_timeout_handler, gch, NULL);
}

// tx
static StatusCode prv_tx(const GenericCan *can, const GenericCanMsg *msg) {
  GenericCanHw *gch = (GenericCanHw *)can;
  if (gch->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanHw.");
  } else if (can == NULL || msg == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  return can_hw_transmit(msg->id, msg->extended, (uint8_t *)&msg->data, msg->dlc);
}

// register_rx
static StatusCode prv_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                                  void *context) {
  GenericCanHw *gch = (GenericCanHw *)can;
  if (gch->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanHw.");
  } else if (can == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  for (size_t i = 0; i < NUM_GENERIC_CAN_HW_RX_HANDLERS; i++) {
    if (gch->rx_storage[i].rx_handler == NULL && !gch->rx_storage[i].enabled) {
      gch->rx_storage[i].id = id;
      gch->rx_storage[i].rx_handler = rx_handler;
      gch->rx_storage[i].context = context;
      gch->rx_storage[i].enabled = true;
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
}

static StatusCode prv_set_rx(GenericCan *can, uint32_t id, bool state) {
  GenericCanHw *gch = (GenericCanHw *)can;
  if (gch->base.interface != &s_interface) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "GenericCan not aligned to GenericCanHw.");
  }

  for (size_t i = 0; i < NUM_GENERIC_CAN_HW_RX_HANDLERS; i++) {
    if (gch->rx_storage[i].id == id && gch->rx_storage[i].rx_handler != NULL) {
      gch->rx_storage[i].enabled = state;
      return STATUS_CODE_OK;
    }
  }
  return status_code(STATUS_CODE_UNINITIALIZED);
}

// enable_rx
static StatusCode prv_enable_rx(GenericCan *can, uint32_t id) {
  return prv_set_rx(can, id, true);
}

// disable_rx
static StatusCode prv_disable_rx(GenericCan *can, uint32_t id) {
  return prv_set_rx(can, id, false);
}

StatusCode generic_can_hw_init(const CANHwSettings *settings, EventID fault_event,
                               GenericCanHw *out) {
  s_interface.tx = prv_tx;
  s_interface.register_rx = prv_register_rx;
  s_interface.disable_rx = prv_disable_rx;
  s_interface.enable_rx = prv_enable_rx;

  memset(out->rx_storage, 0, sizeof(GenericCanRx) * NUM_GENERIC_CAN_HW_RX_HANDLERS);

  out->base.interface = &s_interface;
  out->fault_event = fault_event;
  status_ok_or_return(can_hw_init(settings));

  can_hw_register_callback(CAN_HW_EVENT_MSG_RX, prv_rx_handler, out);
  can_hw_register_callback(CAN_HW_EVENT_BUS_ERROR, prv_bus_error_handler, out);
  return STATUS_CODE_OK;
}
