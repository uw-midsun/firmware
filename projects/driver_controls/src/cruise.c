#include "cruise.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "input_event.h"

static CruiseStorage s_cruise_storage;

static StatusCode prv_handle_motor_velocity(const CANMessage *msg, void *context,
                                            CANAckStatus *ack_reply) {
  CruiseStorage *cruise = context;

  int32_t left = 0, right = 0;
  CAN_UNPACK_MOTOR_VELOCITY(msg, (uint32_t *)&left, (uint32_t *)&right);
  // If we ever overflow we have bigger problems
  cruise->current_speed_cms = MAX((left + right) / 2, 0);

  return STATUS_CODE_OK;
}

static void prv_offset_target(CruiseStorage *cruise, int16_t offset) {
  cruise->target_speed_cms += offset;
  if (cruise->target_speed_cms < 0) {
    // Don't allow a negative cruise speed
    cruise->target_speed_cms = 0;
  }
}

StatusCode cruise_init(CruiseStorage *cruise) {
  cruise->target_speed_cms = 0;

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY, prv_handle_motor_velocity, cruise);

  return STATUS_CODE_OK;
}

StatusCode cruise_set_target_cms(CruiseStorage *cruise, int16_t target) {
  if (target < 0) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  cruise->target_speed_cms = target;
  return STATUS_CODE_OK;
}

int16_t cruise_get_target_cms(CruiseStorage *cruise) {
  return cruise->target_speed_cms;
}

bool cruise_handle_event(CruiseStorage *cruise, const Event *e) {
  // TODO: support hold?
  if (e->id == INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS) {
    prv_offset_target(cruise, CRUISE_OFFSET_CMS);
  } else if (e->id == INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS) {
    prv_offset_target(cruise, -CRUISE_OFFSET_CMS);
  } else if (e->id == INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED) {
    cruise->target_speed_cms = cruise->current_speed_cms;
  } else {
    return false;
  }

  return true;
}

CruiseStorage *cruise_global(void) {
  return &s_cruise_storage;
}
