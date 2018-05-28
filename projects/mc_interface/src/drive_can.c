#include "drive_can.h"
#include <stddef.h>
#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"

static StatusCode prv_handle_drive(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  MotorControllerStorage *controller = context;
  int16_t pedal = 0, direction = 0, cruise = 0, mech_brake = 0;

  status_ok_or_return(CAN_UNPACK_MOTOR_CONTROLS(msg, (uint16_t *)&pedal, (uint16_t *)&direction,
                                                (uint16_t *)&cruise, (uint16_t *)&mech_brake));

  // Basic input validation
  if (direction < 0 || direction >= NUM_EE_DRIVE_OUTPUT_DIRECTIONS || cruise < 0 ||
      pedal < -EE_DRIVE_OUTPUT_DENOMINATOR || pedal > EE_DRIVE_OUTPUT_DENOMINATOR ||
      mech_brake < 0 || mech_brake > EE_DRIVE_OUTPUT_DENOMINATOR) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (mech_brake > EE_DRIVE_OUTPUT_MECH_THRESHOLD) {
    // Mechanical brake is active - force into coast/regen
    motor_controller_set_throttle(controller, MIN(0, pedal), direction);
  } else if (cruise > 0) {
    // Enter cruise state
    motor_controller_set_cruise(controller, cruise);
  } else {
    motor_controller_set_throttle(controller, pedal, direction);
  }

  return STATUS_CODE_OK;
}

static void prv_handle_speed(int16_t speed_cms[], size_t num_speeds, void *context) {
  CAN_TRANSMIT_MOTOR_VELOCITY((uint16_t)speed_cms[0], (uint16_t)speed_cms[1]);
}

StatusCode drive_can_init(MotorControllerStorage *controller) {
  motor_controller_set_speed_cb(controller, prv_handle_speed, NULL);
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLS, prv_handle_drive, controller);
}
