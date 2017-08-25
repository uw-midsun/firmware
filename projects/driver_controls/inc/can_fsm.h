#pragma once

// FSM for transmitting driver input data over CAN

#include "fsm.h"
#include "input_event.h"

// Device IDs for CAN output events
typedef enum {
  CAN_DEVICE_ID_POWER = NUM_INPUT_EVENT,
  CAN_DEVICE_ID_PEDAL,
  CAN_DEVICE_ID_DIRECTION_SELECTOR,
  CAN_DEVICE_ID_TURN_SIGNAL,
  CAN_DEVICE_ID_HAZARD_LIGHT,
  CAN_DEVICE_ID_MECHANICAL_BRAKE,
  CAN_DEVICE_ID_HORN,
  CAN_DEVICE_ID_PUSH_TO_TALK,
} CANDeviceID;

StatusCode can_fsm_init(FSM *fsm);
