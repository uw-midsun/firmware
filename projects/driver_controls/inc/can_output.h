#pragma once

// Module for broadcasting CAN messages
#include <stdint.h>
#include "status.h"
#include "event_arbiter.h"

// Message ID definitions for the input devices
typedef enum {
  CAN_OUTPUT_MESSAGE_POWER = 0,
  CAN_OUTPUT_MESSAGE_PEDAL,
  CAN_OUTPUT_MESSAGE_DIRECTION_SELECTOR,
  CAN_OUTPUT_MESSAGE_LIGHTS,
  CAN_OUTPUT_MESSAGE_MECHANICAL_BRAKE,
  CAN_OUTPUT_MESSAGE_HORN,
  CAN_OUTPUT_MESSAGE_PUSH_TO_TALK,
  NUM_CAN_OUTPUT_MESSAGES
} CANOutputMessage;

// Output function to prepare message to broadcast over CAN
void can_output_transmit(EventArbiterOutputData data);
