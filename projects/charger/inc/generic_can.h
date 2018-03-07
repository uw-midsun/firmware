#pragma once

#include <stdint.h>

#include "generic_can_msg.h"
#include "status.h"

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
} GenericCan;

StatusCode generic_can_tx(const GenericCan *can, const GenericCanMsg *msg);

StatusCode generic_can_register_rx(GenericCan *can, GenericCanRx rx_handler, uint32_t id,
                                   void *context);

StatusCode generic_can_enable_rx(GenericCan *can, uint32_t id);

StatusCode generic_can_disable_rx(GenericCan *can, uint32_t id);
