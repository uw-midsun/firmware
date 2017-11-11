#pragma once
// CAN HW Interface
// Requires GPIO and interrupts to be initialized.
//
// Used to initiate CAN TX and RX directly through the MCU, without any
// preprocessing or queues.
// Note that none of our systems currently support more than one CAN interface
// natively.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gpio.h"
#include "status.h"

// Used to process HW events within the CAN ISR, ideally as short as possible.
typedef void (*CANHwEventHandlerCb)(void *context);

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

typedef enum {
  CAN_HW_BITRATE_125KBPS,
  CAN_HW_BITRATE_250KBPS,
  CAN_HW_BITRATE_500KBPS,
  CAN_HW_BITRATE_1000KBPS,
  NUM_CAN_HW_BITRATES
} CANHwBitrate;

typedef struct CANHwSettings {
  GPIOAddress tx;
  GPIOAddress rx;
  CANHwBitrate bitrate;
  bool loopback;
} CANHwSettings;

// Initializes CAN using the specified settings.
StatusCode can_hw_init(const CANHwSettings *settings);

// Registers a callback for the given event
StatusCode can_hw_register_callback(CANHwEvent event, CANHwEventHandlerCb callback, void *context);

StatusCode can_hw_add_filter(uint16_t mask, uint16_t filter);

CANHwBusStatus can_hw_bus_status(void);

StatusCode can_hw_transmit(uint16_t id, const uint8_t *data, size_t len);

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(uint16_t *id, uint64_t *data, size_t *len);
