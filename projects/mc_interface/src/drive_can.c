#include "drive_can.h"
#include "can.h"
#include "can_unpack.h"

static StatusCode prv_handle_drive(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  MotorControllerStorage *controller = context;
  int16_t pedal, direction, cruise, mech_brake;

  status_ok_or_return(CAN_UNPACK_MOTOR_CONTROLS(msg, (uint16_t *)&pedal, (uint16_t *)&direction,
                                                (uint16_t *)&cruise, (uint16_t *)&mech_brake));

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

StatusCode drive_can_init(MotorControllerStorage *controller) {
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_CONTROLS, prv_handle_drive, controller);
}
