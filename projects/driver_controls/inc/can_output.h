#pragma once

// Module for broadcasting CAN messages
#include <stdint.h>
#include "status.h"

// Device ID definitions
typedef enum {
  CAN_DEVICE_ID_POWER = 0,
  CAN_DEVICE_ID_PEDAL,
  CAN_DEVICE_ID_DIRECTION_SELECTOR,
  CAN_DEVICE_ID_TURN_SIGNAL,
  CAN_DEVICE_ID_HAZARD_LIGHT,
  CAN_DEVICE_ID_MECHANICAL_BRAKE,
  CAN_DEVICE_ID_HORN,
  CAN_DEVICE_ID_PUSH_TO_TALK,
} CANDeviceID;

// Output function to clip and prepare data to be broacasted over CAN
void can_output_transmit(uint8_t device_id, uint8_t device_state, uint16_t device_data);
