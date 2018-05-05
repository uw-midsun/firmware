#include <string.h>

#include "motor_controller_fsm.h"

#include "can.h"
#include "can_interval.h"
#include "can_unpack.h"
#include "config.h"
#include "critical_section.h"
#include "exported_enums.h"
#include "fifo.h"
#include "motor_controller.h"
#include "motor_controller_interface_events.h"

#include "log.h"
#define NUM_CAN_INTERVALS 2
static FSM s_fsm;

static uint32_t s_base_address[NUM_CAN_INTERVALS] = { MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR,
                                                      MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR };
static CanInterval *s_intervals[NUM_CAN_INTERVALS] = { 0 };

static CanInterval *s_left_interval = NULL;
static CanInterval *s_right_interval = NULL;

// Generic "dumb" handler to copy telemetry values to a buffer
static void prv_handle_slave_msg(const GenericCanMsg *msg, void *context) {
  memcpy(context, &msg->data, MIN(msg->dlc, (size_t)8));
}

static StatusCode prv_push_drive(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  MotorControllerFsmStorage *storage = (MotorControllerFsmStorage *)context;
  DriverControlsData data = { 0 };

  CAN_UNPACK_MOTOR_CONTROLS(msg, &data.throttle, &data.direction, &data.cruise_control,
                            &data.brake_state);

  status_ok_or_return(fifo_push(&storage->fifo, &data));

  return event_raise(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, 0);
}

static bool prv_mechanical_braking_guard(const FSM *fsm, const Event *e, void *context) {
  MotorControllerFsmStorage *storage = (MotorControllerFsmStorage *)context;
  DriverControlsData data = { 0 };

  fifo_peek(&storage->fifo, &data);
  if (data.brake_state != DRIVER_CONTROLS_BRAKE_DISENGAGED) {
    // The pop will happen in the output function
    return true;
  }
  return false;
}

static bool prv_torque_control_forward_guard(const FSM *fsm, const Event *e, void *context) {
  // We can only go forward if the direction selector is set to Forward and the
  // Current is a valid value
  MotorControllerFsmStorage *storage = (MotorControllerFsmStorage *)context;
  DriverControlsData data = { 0 };

  fifo_peek(&storage->fifo, &data);
  if (data.brake_state == DRIVER_CONTROLS_BRAKE_DISENGAGED &&
      data.direction == DRIVER_CONTROLS_FORWARD && data.cruise_control == 0) {
    // The pop will happen in the output function
    return true;
  }
  return false;
}

static bool prv_torque_control_reverse_guard(const FSM *fsm, const Event *e, void *context) {
  // We can only go reverse if the direction selector is set to Reverse and the
  // Current is a valid value, and cruise control is disabled
  MotorControllerFsmStorage *storage = (MotorControllerFsmStorage *)context;
  DriverControlsData data = { 0 };

  fifo_peek(&storage->fifo, &data);
  if (data.brake_state == DRIVER_CONTROLS_BRAKE_DISENGAGED &&
      data.direction == DRIVER_CONTROLS_REVERSE && data.cruise_control == 0) {
    // The pop will happen in the output function
    return true;
  }
  return false;
}

static bool prv_cruise_control_forward_guard(const FSM *fsm, const Event *e, void *context) {
  // We can only go Cruise Control if the direction selector is set to Forward
  // and the Current is a valid value
  MotorControllerFsmStorage *storage = (MotorControllerFsmStorage *)context;
  DriverControlsData data = { 0 };

  fifo_peek(&storage->fifo, &data);
  if (data.brake_state == DRIVER_CONTROLS_BRAKE_DISENGAGED &&
      data.direction == DRIVER_CONTROLS_FORWARD && data.cruise_control != 0) {
    // The pop will happen in the output function
    return true;
  }
  return false;
}

// Motor Controller state definitions

// Note: Cruise Control is invalid in reverse
FSM_DECLARE_STATE(state_torque_control_forward);
FSM_DECLARE_STATE(state_torque_control_reverse);
FSM_DECLARE_STATE(state_cruise_control_forward);
FSM_DECLARE_STATE(state_mechanical_braking);

// Motor Controller transition table definitions

FSM_STATE_TRANSITION(state_torque_control_forward) {
  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_forward_guard, state_torque_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_reverse_guard, state_torque_control_reverse);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_cruise_control_forward_guard, state_cruise_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, prv_mechanical_braking_guard,
                             state_mechanical_braking);
}

FSM_STATE_TRANSITION(state_torque_control_reverse) {
  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_forward_guard, state_torque_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_reverse_guard, state_torque_control_reverse);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_cruise_control_forward_guard, state_cruise_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, prv_mechanical_braking_guard,
                             state_mechanical_braking);
}

FSM_STATE_TRANSITION(state_cruise_control_forward) {
  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_forward_guard, state_torque_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_reverse_guard, state_torque_control_reverse);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_cruise_control_forward_guard, state_cruise_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, prv_mechanical_braking_guard,
                             state_mechanical_braking);
}

FSM_STATE_TRANSITION(state_mechanical_braking) {
  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_forward_guard, state_torque_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_torque_control_reverse_guard, state_torque_control_reverse);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO,
                             prv_cruise_control_forward_guard, state_cruise_control_forward);

  FSM_ADD_GUARDED_TRANSITION(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, prv_mechanical_braking_guard,
                             state_mechanical_braking);
}

// Output functions
static void prv_torque_control_mode(FSM *fsm, const Event *e, void *context) {
  volatile MotorControllerFsmStorage *storage = (volatile MotorControllerFsmStorage *)context;
  DriverControlsData data = { 0 };
  StatusCode status = fifo_pop(&storage->fifo, &data);

  float desired_torque = CONVERT_THROTTLE_READING_TO_PERCENTAGE((float)data.throttle);

  if (data.brake_state == DRIVER_CONTROLS_BRAKE_ENGAGED) {
    // If the brake is enaged, then ensure that we send 0 torque
    desired_torque = 0;
  }

  float desired_direction = 0;
  if (data.direction == DRIVER_CONTROLS_REVERSE) {
    desired_direction = MOTOR_CONTROLLER_REVERSE_VELOCITY_MPS;
  } else {
    // In case we receive an invalid direction, it's probably safer to go forward
    // rather than backwards
    desired_direction = MOTOR_CONTROLLER_FORWARD_VELOCITY_MPS;
  }

  // To run the motor in torque control mode, set the velocity to an
  // unobtainable value such as 100 m/s. Set the current to a value that is
  // proportional to your accelerator pedal position.
  MotorControllerDriveCmd cmd = {
    .current_percentage = desired_torque,
    .velocity = desired_direction,
  };

  bool disabled = critical_section_start();
  for (size_t i = 0; i < NUM_CAN_INTERVALS; ++i) {
    GenericCanMsg msg = {
      .id = MOTOR_CONTROLLER_DRIVE_COMMAND_ID(s_base_address[i]),
      .data = 0,
      .dlc = sizeof(cmd),
      .extended = false,
    };
    memcpy(&msg.data, &cmd, sizeof(cmd));

    // Copy the new messages and overwrite the existing CAN intervals
    s_intervals[i]->msg = msg;

    // Trigger a transmission
    can_interval_send_now(s_intervals[i]);
  }
  critical_section_end(disabled);
}

static void prv_cruise_control_mode(FSM *fsm, const Event *e, void *context) {
  MotorControllerFsmStorage *storage = (MotorControllerFsmStorage *)context;
  DriverControlsData data = { 0 };

  fifo_pop(&storage->fifo, &data);
  float desired_speed = MOTOR_CONTOLLER_VELOCITY_CMPS_TO_MPS(data.cruise_control);

  // We perform cruise control by setting velocity mode on one motor, then
  // copying the current setpoint to the other motor

  // To run the motor in velocity (cruise) control mode, set the current to your
  // maximum desired acceleration force (usually 100%), and set the velocity to
  // the desired speed.
  MotorControllerDriveCmd cmd_left = {
    .current_percentage = 1.0f,
    .velocity = desired_speed,
  };

  // Update Left Motor Controller message
  GenericCanMsg left_msg = {
    .id = MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR),
    .data = 0,
    .dlc = sizeof(cmd_left),
    .extended = false,
  };
  memcpy(&left_msg.data, &cmd_left, sizeof(cmd_left));
  s_intervals[0]->msg = left_msg;

  // Read the current setpoint from the left Motor Controller and send it to
  // the Right Motor Controller
  MotorControllerPowerCmd cmd_right = {
    .current_percentage =
        storage->measurement.bus_meas_left.current / MOTOR_CONTROLLER_CURRENT_LIMIT,
  };
  GenericCanMsg right_msg = {
    .id = MOTOR_CONTROLLER_POWER_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR),
    .data = 0,
    .dlc = sizeof(cmd_right),
    .extended = false,
  };
  memcpy(&right_msg.data, &cmd_right, sizeof(cmd_right));
  s_intervals[1]->msg = right_msg;

  // Trigger a transmission
  can_interval_send_now(s_intervals[0]);
  can_interval_send_now(s_intervals[1]);
}

StatusCode motor_controller_fsm_init(const MotorControllerFsmStorage *storage) {
  status_ok_or_return(
      generic_can_register_rx(storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
                              MOTOR_CONTROLLER_BUS_MEASUREMENT(MOTOR_CONTROLLER_LEFT_ADDR), false,
                              (void *)&storage->measurement.bus_meas_left));
  status_ok_or_return(
      generic_can_register_rx(storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
                              MOTOR_CONTROLLER_BUS_MEASUREMENT(MOTOR_CONTROLLER_RIGHT_ADDR), false,
                              (void *)&storage->measurement.bus_meas_right));

  status_ok_or_return(
      generic_can_register_rx(storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
                              MOTOR_CONTROLLER_VELOCITY_MEASUREMENT(MOTOR_CONTROLLER_LEFT_ADDR),
                              false, (void *)&storage->measurement.velocity_meas_left));
  status_ok_or_return(
      generic_can_register_rx(storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
                              MOTOR_CONTROLLER_VELOCITY_MEASUREMENT(MOTOR_CONTROLLER_RIGHT_ADDR),
                              false, (void *)&storage->measurement.velocity_meas_right));

  status_ok_or_return(generic_can_register_rx(
      storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CONTROLLER_SINK_MOTOR_TEMPERATURE_MEASUREMENT(MOTOR_CONTROLLER_LEFT_ADDR), false,
      (void *)&storage->measurement.sink_motor_meas_left));
  status_ok_or_return(generic_can_register_rx(
      storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CONTROLLER_SINK_MOTOR_TEMPERATURE_MEASUREMENT(MOTOR_CONTROLLER_RIGHT_ADDR), false,
      (void *)&storage->measurement.sink_motor_meas_right));

  status_ok_or_return(generic_can_register_rx(
      storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CONTROLLER_ODOMETER_BUS_AMP_HOURS_MEASUREMENT(MOTOR_CONTROLLER_LEFT_ADDR), false,
      (void *)&storage->measurement.odo_bus_meas_left));
  status_ok_or_return(generic_can_register_rx(
      storage->generic_can, prv_handle_slave_msg, GENERIC_CAN_EMPTY_MASK,
      MOTOR_CONTROLLER_ODOMETER_BUS_AMP_HOURS_MEASUREMENT(MOTOR_CONTROLLER_RIGHT_ADDR), false,
      (void *)&storage->measurement.odo_bus_meas_right));

  // Register CAN handlers for the driver controls messages
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLS, prv_push_drive, storage));

  fifo_init(&storage->fifo, storage->fifo_data);

  fsm_init(&storage->fsm, "Motor Controller FSM", &state_torque_control_forward, storage);

  fsm_state_init(state_torque_control_forward, prv_torque_control_mode);
  fsm_state_init(state_torque_control_reverse, prv_torque_control_mode);
  fsm_state_init(state_cruise_control_forward, prv_cruise_control_mode);
  fsm_state_init(state_mechanical_braking, prv_torque_control_mode);

  // Initialize the CAN Intervals
  memset((void *)&s_intervals, 0, sizeof(s_intervals[0]) * SIZEOF_ARRAY(s_intervals));
  GenericCanMsg msg = { 0 };

  status_ok_or_return(can_interval_factory(storage->generic_can, &msg,
                                           MOTOR_CONTROLLER_MESSAGE_INTERVAL_US, &s_intervals[0]));
  status_ok_or_return(can_interval_factory(storage->generic_can, &msg,
                                           MOTOR_CONTROLLER_MESSAGE_INTERVAL_US, &s_intervals[1]));

  return STATUS_CODE_OK;
}

StatusCode motor_controller_fsm_process_event(Event *e) {
  return fsm_process_event(&s_fsm, e);
}
