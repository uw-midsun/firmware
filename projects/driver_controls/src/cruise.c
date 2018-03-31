#include "cruise.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "input_event.h"

static CruiseStorage s_cruise_storage;

static void prv_handle_motor_velocity(const CANMessage *msg, void *context,
                                      CANAckStatus *ack_reply) {
  CruiseStorage *cruise = context;

  // TODO(ELEC-354): store motor velocity - probably an average of >0
  // CAN_UNPACK_MOTOR_VELOCITY(msg);
  // cruise->target_speed_ms = ;
}

StatusCode cruise_init(CruiseStorage *cruise) {
  cruise->source = CRUISE_SOURCE_MOTOR_CONTROLLER;
  cruise->target_speed_cms = 0;

  (void)prv_handle_motor_velocity;
  // can_register_rx_handler(CAN_MESSAGE_MOTOR_VELOCITY, prv_handle_motor_velocity, cruise);

  return STATUS_CODE_OK;
}

StatusCode cruise_set_source(CruiseStorage *cruise, CruiseSource source) {
  cruise->source = source;

  return STATUS_CODE_OK;
}

StatusCode cruise_offset(CruiseStorage *cruise, int16_t offset) {
  if (cruise->source != CRUISE_SOURCE_STORED_VALUE) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  cruise->target_speed_cms += offset;
  if (cruise->target_speed_cms < 0) {
    // Don't allow a negative cruise speed
    cruise->target_speed_cms = 0;
  }

  return STATUS_CODE_OK;
}

int16_t cruise_get_target(CruiseStorage *cruise) {
  return cruise->target_speed_cms;
}

bool cruise_handle_event(CruiseStorage *cruise, const Event *e) {
  if (e->id == INPUT_EVENT_CRUISE_CONTROL_INC) {
    cruise_offset(cruise, CRUISE_OFFSET_CMS);
  } else if (e->id == INPUT_EVENT_CRUISE_CONTROL_DEC) {
    cruise_offset(cruise, -CRUISE_OFFSET_CMS);
  } else {
    return false;
  }

  return true;
}

CruiseStorage *cruise_global(void) {
  return &s_cruise_storage;
}
