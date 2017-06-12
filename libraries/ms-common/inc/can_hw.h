#pragma once
// CAN HW Interface
//
// Used to initiate CAN TX and RX directly through the MCU, without any preprocessing or queues.
// Note that none of our systems currently support more than one CAN interface natively.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_hw_mcu.h"
#include "status.h"

// Used to process HW events within the CAN ISR, ideally as short as possible.
typedef void (*CANHwEventHandlerCb)(void *context);

typedef struct CANHwEventHandler {
  CANHwEventHandlerCb callback;
  void *context;
} CANHwEventHandler;

typedef enum {
  CAN_HW_EVENT_TX_READY = 0,
  CAN_HW_EVENT_MSG_RX,
  CAN_HW_EVENT_BUS_ERROR,
  NUM_CAN_HW_EVENTS
} CANHwEvent;

typedef enum {
  CAN_HW_BUS_STATUS_OK = 0,
  CAN_HW_BUS_STATUS_ERROR,
  CAN_HW_BUS_STATUS_OFF
} CANHwBusStatus;

typedef struct CANHwConfig {
  CANHwBase base;
  uint16_t bus_speed; // in kbps
  uint8_t num_filters;
  bool loopback;
  CANHwEventHandler handlers[NUM_CAN_HW_EVENTS];
} CANHwConfig;

// Initializes the specified CAN HW instance against the native CAN interface
StatusCode can_hw_init(CANHwConfig *can_hw, uint16_t bus_speed, bool loopback);

// Registers a callback for the given event
StatusCode can_hw_register_callback(CANHwConfig *can_hw, CANHwEvent event,
                                    CANHwEventHandlerCb callback, void *context);

StatusCode can_hw_add_filter(CANHwConfig *can_hw, uint16_t mask, uint16_t filter);

CANHwBusStatus can_hw_bus_status(const CANHwConfig *can_hw);

StatusCode can_hw_transmit(const CANHwConfig *can_hw, uint16_t id, uint8_t *data, size_t len);

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(const CANHwConfig *can_hw, uint16_t *id, uint64_t *data, size_t *len);
