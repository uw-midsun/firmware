#pragma once

#include <stdbool.h>

#include "can.h"
#include "event_queue.h"
#include "status.h"

// TODO(ELEC-105): Export this somewhere so driver controls has access.
typedef enum ChaosCanPowerState {
  CHAOS_CAN_POWER_STATE_IDLE = 0,
  CHAOS_CAN_POWER_STATE_CHARGE,
  CHAOS_CAN_POWER_STATE_DRIVE,
  NUM_CHAOS_CAN_POWER_STATES,
} ChaosCanPowerState;

// Initialize CAN and register all relevant handlers.
StatusCode chaos_can_init(CANSettings *settings);

bool chaos_can_process_event(const Event *e);
