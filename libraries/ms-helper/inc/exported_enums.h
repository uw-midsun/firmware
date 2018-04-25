#pragma once
// Exported enums that are shared between boards
// (e.g. for data transmitted over CAN)

typedef enum {
  DRIVER_CONTROLS_FORWARD,
  DRIVER_CONTROLS_REVERSE,
} ExportedEnumsDriverControlsDirection;

typedef enum {
  DRIVER_CONTROLS_BRAKE_DISENGAGED,
  DRIVER_CONTROLS_BRAKE_ENGAGED,
} ExportedEnumsDriverControlsBrakeState;

#define DRIVER_CONTROLS_PEDAL_DENOMINATOR 2048

#define CONVERT_THROTTLE_READING_TO_PERCENTAGE(raw_throttle) \
  ((float)(raw_throttle) / DRIVER_CONTROLS_PEDAL_DENOMINATOR)
