#pragma once

// FSM for transmitting driver input data over CAN

#include "fsm.h"

typedef enum {
  CAN_DEVICE_ID_POWER,
  CAN_DEVICE_ID_PEDAL,
  CAN_DEVICE_ID_DIRECTION_SELECTOR,
  CAN_DEVICE_ID_TURN_SIGNAL,
  CAN_DEVICE_ID_HAZARD_LIGHT,
  CAN_DEVICE_ID_MECHANICAL_BRAKE,
  CAN_DEVICE_ID_HORN,
  CAN_DEVICE_ID_PUSH_TO_TALK
} CANDeviceID;

// Initialize the FSM for CAN
StatusCode can_fsm_init(FSM *fsm);

// Clip and prepare event data to be broacasted over CAN
StatusCode can_fsm_transmit(CANDeviceID device_id, uint8_t device_state, uint16_t device_data);
