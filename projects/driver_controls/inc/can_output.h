#pragma once

// Module for broadcasting CAN messages
#include <stdint.h>
#include "status.h"
#include "event_arbiter.h"

// Device ID definitions
typedef enum {
  CAN_OUTPUT_MESSAGE_POWER = 0,
  CAN_OUTPUT_MESSAGE_PEDAL,
  CAN_OUTPUT_MESSAGE_DIRECTION_SELECTOR,
  CAN_OUTPUT_MESSAGE_TURN_SIGNAL,
  CAN_OUTPUT_MESSAGE_HAZARD_LIGHT,
  CAN_OUTPUT_MESSAGE_MECHANICAL_BRAKE,
  CAN_OUTPUT_MESSAGE_HORN,
  CAN_OUTPUT_MESSAGE_PUSH_TO_TALK,
  NUM_CAN_OUTPUT_DEVICES
} CANOutputMessageID;

// Output function to clip and prepare data to be broacasted over CAN
void can_output_transmit(EventArbiterOutputData data);
