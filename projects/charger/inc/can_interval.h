#pragma once
// Module for periodically sending CAN messages.

#include <stdint.h>

#include "generic_can.h"
#include "generic_can_msg.h"
#include "soft_timer.h"
#include "status.h"

#define CAN_INTERVAL_POOL_SIZE 5

typedef struct CanInterval {
  GenericCan *can;
  GenericCanMsg *msg;
  SoftTimerID timer_id;
  uint32_t period;
} CanInterval;

void can_interval_init(void);

StatusCode can_interval_factory(const GenericCan *can, const GenericCanMsg *msg, uint32_t period,
                                CanInterval *interval);

StatusCode can_interval_free(CanInterval *interval);

StatusCode can_interval_send_now(CanInterval *interval);

StatusCode can_interval_enable(CanInterval *interval);

StatusCode can_interval_disable(CanInterval *interval);
