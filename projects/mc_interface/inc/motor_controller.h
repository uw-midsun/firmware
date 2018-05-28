#pragma once
// Interfaces with WaveSculptor20 motor controllers
//
// Expects a CAN UART slave
// Repeats the last received message every 50ms - seems to result in smoother transitions
//
// In cruise mode, the first motor controller is picked as a master and set to velocity control.
// Its current reading is copied over to all other motor controllers to allow them to handle turns.
#include "exported_enums.h"
#include "generic_can.h"

#define MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS 50
// Arbitrary timeout after 5 TX periods without receiving a setpoint update
#define MOTOR_CONTROLLER_WATCHDOG_COUNTER 5

typedef enum {
  MOTOR_CONTROLLER_LEFT,
  MOTOR_CONTROLLER_RIGHT,
  NUM_MOTOR_CONTROLLERS
} MotorController;

typedef enum {
  MOTOR_CONTROLLER_MODE_TORQUE = 0,
  MOTOR_CONTROLLER_MODE_VELOCITY,
  NUM_MOTOR_CONTROLLER_MODES,
} MotorControllerMode;

typedef uint32_t MotorControllerCanId;

typedef struct MotorControllerSettings {
  GenericCan *can_uart;
  struct {
    // WaveSculptor address
    MotorControllerCanId motor_controller;
    // Driver Controls address
    MotorControllerCanId interface;
  } ids[NUM_MOTOR_CONTROLLERS];
  // Maximum bus current (should be programmed into the motor controllers)
  // Used to copy current setpoint from primary cruise controller
  float max_bus_current;
} MotorControllerSettings;

typedef struct MotorControllerStorage {
  MotorControllerSettings settings;
  // For cruise control
  float cruise_current_percentage;
  bool cruise_is_braking;

  // Stored to repeat
  float target_velocity_ms;
  float target_current_percentage;
  MotorControllerMode target_mode;

  // TODO: record speed for TX

  size_t timeout_counter;
} MotorControllerStorage;

// |settings.can_uart| should be initialized to an instance of CAN UART
StatusCode motor_controller_init(MotorControllerStorage *controller,
                                 const MotorControllerSettings *settings);

StatusCode motor_controller_set_throttle(MotorControllerStorage *controller, int16_t throttle,
                                         EEDriveOutputDirection direction);

StatusCode motor_controller_set_cruise(MotorControllerStorage *controller, int16_t speed_cms);
