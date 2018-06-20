#pragma once
// Handles a bunch of inputs to determine whether to turn the brakes on.
// Requires CAN, event queue to be initialized.
//
// Transmits CAN messages to Lights on brake state changes.
//
// Potential variables:
// - Power state
// - Mech brake state
// - Direction
// - Throttle state
// - Speed
//
// The brake lights will be on if:
// (Power == Drive) &&
//  (Mech Brake == Engaged ||
//   (Direction != Neutral && Throttle == Brake && Speed > x))
//
#include "event_queue.h"
#include "status.h"

// Minimum speed for brake lights to turn on due to regen braking
// Arbitrarily set to ~1mph
#define BRAKE_SIGNAL_MIN_SPEED_CMS 45

typedef enum {
  BRAKE_SIGNAL_INPUT_POWER_STATE = 0,
  BRAKE_SIGNAL_INPUT_MECH_BRAKE,
  BRAKE_SIGNAL_INPUT_DIRECTION,
  BRAKE_SIGNAL_INPUT_THROTTLE,
  BRAKE_SIGNAL_INPUT_SPEED,
  NUM_BRAKE_SIGNAL_INPUTS,
} BrakeSignalInput;

StatusCode brake_signal_init(void);

// Processes events to determine whether the brake light should be on or not
bool brake_signal_process_event(const Event *e);
