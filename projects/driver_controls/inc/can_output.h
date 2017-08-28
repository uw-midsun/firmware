#pragma once

// Module for broadcasting CAN messages
#include <stdint.h>
#include "status.h"
#include "event_arbiter.h"

// Device ID definitions
typedef enum {
  CAN_OUTPUT_DEVICE_POWER = 0,
  CAN_OUTPUT_DEVICE_PEDAL,
  CAN_OUTPUT_DEVICE_DIRECTION_SELECTOR,
  CAN_OUTPUT_DEVICE_TURN_SIGNAL,
  CAN_OUTPUT_DEVICE_HAZARD_LIGHT,
  CAN_OUTPUT_DEVICE_MECHANICAL_BRAKE,
  CAN_OUTPUT_DEVICE_HORN,
  CAN_OUTPUT_DEVICE_PUSH_TO_TALK,
  NUM_CAN_OUTPUT_DEVICES
} CANOutputDeviceID;

// Output function to clip and prepare data to be broacasted over CAN
void can_output_transmit(FSM *fsm, EventArbiterOutputData data);
