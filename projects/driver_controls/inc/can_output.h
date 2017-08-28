#pragma once

// Module for broadcasting CAN messages
#include <stdint.h>
#include "status.h"

// Device ID definitions
typedef enum {
  CAN_OUTPUT_DEVICE_ID_POWER = 0,
  CAN_OUTPUT_DEVICE_ID_PEDAL,
  CAN_OUTPUT_DEVICE_ID_DIRECTION_SELECTOR,
  CAN_OUTPUT_DEVICE_ID_TURN_SIGNAL,
  CAN_OUTPUT_DEVICE_ID_HAZARD_LIGHT,
  CAN_OUTPUT_DEVICE_ID_MECHANICAL_BRAKE,
  CAN_OUTPUT_DEVICE_ID_HORN,
  CAN_OUTPUT_DEVICE_ID_PUSH_TO_TALK,
  NUM_CAN_OUTPUT_DEVICES
} CANOutputDeviceID;

// Output function to clip and prepare data to be broacasted over CAN
void can_output_transmit(uint8_t device_id, uint8_t device_state, uint16_t device_data);
