#pragma once
// Module for abstracting CAN implementation details.

#include <stdint.h>

#include "generic_can_msg.h"
#include "status.h"

#define NUM_GENERIC_CAN_RX_HANDLERS 5

struct GenericCan;

typedef void (*GenericCanRx)(const GenericCanMsg *msg, void *context);

typedef struct GenericCanRxStorage {
  uint32_t id;
  GenericCanRx rx_handler;
  void *context;
  bool enabled;
} GenericCanRxStorage;

typedef struct GenericCanInterface {
  StatusCode (*tx)(const struct GenericCan *can, const GenericCanMsg *msg);
  // Doesn't support ACKable messages (defaults to enabled).
  StatusCode (*register_rx)(struct GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                            void *context);
  StatusCode (*enable_rx)(struct GenericCan *can, uint32_t id);
  StatusCode (*disable_rx)(struct GenericCan *can, uint32_t id);
} GenericCanInterface;

typedef struct GenericCan {
  GenericCanInterface *interface;
  GenericCanRxStorage rx_storage[NUM_GENERIC_CAN_RX_HANDLERS];
} GenericCan;

// Usage:
//
// GenericCan uses dynamic dispatch where each struct for an implementation variant of CAN carries
// with it an implementation of tx, register_rx, enable_rx and disable_rx. To use a given
// implementation one simply needs to pass the specific type of GenericCan<Network|Hw|Uart> to one
// of the following functions. Since all three of those types use GenericCan as a base by casting to
// (GenericCan *). It is possible to derive the interface function pointers without knowing about
// the rest of the data carried in the struct. This allows an object oriented approach where the
// remaining variable for a given implementation are "private/protected" meanwhile the interface is
// consistent for all variants of GenericCan.
//
// Example:
//
// GenericCanUart can_uart = { 0 };
// generic_can_uart_init(&can_uart, UART_PORT_1);
//
// // Transmit
// GenericCanMsg my_msg = { 0 };
//
// // Populate...
//
// generic_can_tx((GenericCan *)&can_uart, &my_msg);
//
// The primary advantage of this is that if you build a function that relies on CAN it can accept
// any GenericCan implementation and be ignorant to its underlying behavior. For example:
//
// static uint64_t s_data;
//
// void send_internal_data(const GenericCan *can) {
//   GenericCanMsg my_msg = { .data = &s_data };
//   generic_can_tx(&can, &my_msg);
// }
//
// In this way the data can be sent via UART, CAN Hw or CAN Network without the caller knowing which
// specific CAN variant it is dealing with!

// Transmits a GenericCanMsg.
//
// Note: if backed by CAN Network |msg| needs to be compatible with CANMessage and its |source_id|
// will automatically be set to the ID of the broadcaster as determined in |device_id| of a
// CANSettings struct. Also |type| must be CAN_MSG_TYPE_DATA as this TX mode does not support acks
// (therefore its CANMessage.msg_id must be in the range [0, 2047].
StatusCode generic_can_tx(const GenericCan *can, const GenericCanMsg *msg);

// Registers a |rx_handler| (enabled by default) to a |raw_id|.
StatusCode generic_can_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t raw_id,
                                   void *context);

// Enables the stored GenericCanRx for |raw_id|.
StatusCode generic_can_enable_rx(GenericCan *can, uint32_t raw_id);

// Disables the stored GenericCanRx for |raw_id|.
StatusCode generic_can_disable_rx(GenericCan *can, uint32_t raw_id);
