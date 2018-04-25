#pragma once
// Motor Controller FSM
//
// Requires the following to be initialized:
//  - CAN Interval
//  - Event Queue
//  - FIFO
//  - Generic CAN
#include "event_queue.h"
#include "fifo.h"
#include "fsm.h"
#include "generic_can.h"
#include "status.h"

#include "motor_controller.h"

typedef struct {
  uint16_t throttle;        // 0 - denominator
  uint16_t direction;       // exported enums
  uint16_t cruise_control;  // cm/s => 0 when disabled
  uint16_t brake_state;     // exported
} DriverControlsData;

typedef enum {
  MOTOR_CONTROLLER_FSM_STATE_TORQUE_CONTROL_FORWARD = 0,
  MOTOR_CONTROLLER_FSM_STATE_TORQUE_CONTROL_REVERSE,
  MOTOR_CONTROLLER_FSM_STATE_CRUISE_CONTROL_FORWARD,
  MOTOR_CONTROLLER_FSM_STATE_BRAKING,
  NUM_MOTOR_CONTROLLER_FSM_STATES
} MotorControllerFsmState;

typedef struct {
  MotorControllerBusMeasurement bus_meas_left;
  MotorControllerBusMeasurement bus_meas_right;

  MotorControllerVelocityMeasurement velocity_meas_left;
  MotorControllerVelocityMeasurement velocity_meas_right;

  MotorControllerSinkMotorTempMeasurement sink_motor_meas_left;
  MotorControllerSinkMotorTempMeasurement sink_motor_meas_right;

  MotorControllerOdometerBusAmpHoursMeasurement odo_bus_meas_left;
  MotorControllerOdometerBusAmpHoursMeasurement odo_bus_meas_right;
} MotorControllerMeasurement;

typedef struct {
  GenericCan *generic_can;
  MotorControllerMeasurement *measurement;
  Fifo *fifo;
} MotorControllerFsmStorage;

// Initializes the FSM
StatusCode motor_controller_fsm_init(const MotorControllerFsmStorage *);

// Run an iteration of the Motor Controller FSM
StatusCode motor_controller_fsm_process_event(Event *e);
