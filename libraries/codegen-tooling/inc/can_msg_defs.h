#pragma once

#include <stdbool.h>

#include "can_msg.h"

// For setting the CAN device
typedef enum {
  SYSTEM_CAN_DEVICE_PLUTUS = 0,
  SYSTEM_CAN_DEVICE_CHAOS = 1,
  SYSTEM_CAN_DEVICE_TELEMETRY = 2,
  SYSTEM_CAN_DEVICE_LIGHTS_FRONT = 3,
  SYSTEM_CAN_DEVICE_LIGHTS_REAR = 4,
  SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER = 5,
  SYSTEM_CAN_DEVICE_DRIVER_CONTROLS = 6,
  SYSTEM_CAN_DEVICE_DRIVER_DISPLAY = 7,
  SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT = 8,
  SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR = 9,
  SYSTEM_CAN_DEVICE_SENSOR_BOARD = 10,
  SYSTEM_CAN_DEVICE_CHARGER = 11,
  NUM_SYSTEM_CAN_DEVICES = 12
} SystemCanDevice;

// For setting the CAN message ID
typedef enum {
  SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT = 0,
  SYSTEM_CAN_MESSAGE_BATTERY_RELAY = 1,
  SYSTEM_CAN_MESSAGE_MAIN_RELAY = 2,
  SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR = 3,
  SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT = 4,
  SYSTEM_CAN_MESSAGE_POWER_STATE = 5,
  SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT = 6,
  SYSTEM_CAN_MESSAGE_OVUV_DCDC_AUX = 16,
  SYSTEM_CAN_MESSAGE_MC_ERROR_LIMITS = 17,
  SYSTEM_CAN_MESSAGE_MOTOR_CONTROLS = 18,
  SYSTEM_CAN_MESSAGE_LIGHTS_STATES = 24,
  SYSTEM_CAN_MESSAGE_HORN = 25,
  SYSTEM_CAN_MESSAGE_MECHANICAL_BRAKE = 26,
  SYSTEM_CAN_MESSAGE_CHARGING_REQ = 27,
  SYSTEM_CAN_MESSAGE_CHARGING_PERMISSION = 28,
  SYSTEM_CAN_MESSAGE_BATTERY_SOC = 31,
  SYSTEM_CAN_MESSAGE_BATTERY_VCT = 32,
  SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_VC = 35,
  SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY_L = 36,
  SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY_R = 37,
  SYSTEM_CAN_MESSAGE_MOTOR_TEMPS = 38,
  SYSTEM_CAN_MESSAGE_MOTOR_AMP_HR = 39,
  SYSTEM_CAN_MESSAGE_ODOMETER = 40,
  SYSTEM_CAN_MESSAGE_AUX_DCDC_VC = 43,
  SYSTEM_CAN_MESSAGE_DCDC_TEMPS = 44,
  SYSTEM_CAN_MESSAGE_SOLAR_DATA_FRONT = 45,
  SYSTEM_CAN_MESSAGE_SOLAR_DATA_REAR = 46,
  SYSTEM_CAN_MESSAGE_LINEAR_ACCELERATION = 51,
  SYSTEM_CAN_MESSAGE_ANGULAR_ROTATION = 52,
  NUM_SYSTEM_CAN_MESSAGES = 29
} SystemCanMessage;
