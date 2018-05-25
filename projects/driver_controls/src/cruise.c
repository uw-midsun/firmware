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

static void prv_timer_cb(SoftTimerID timer_id, void *context) {
  CruiseStorage *cruise = context;

  cruise->target_speed_cms += cruise->offset_cms;
  if (cruise->target_speed_cms < 0) {
    // Don't allow a negative cruise speed
    cruise->target_speed_cms = 0;
  }

  // TODO: don't hardcode the period
  soft_timer_start_millis(5000, prv_timer_cb, cruise, &cruise->repeat_timer);
}

StatusCode cruise_init(CruiseStorage *cruise) {
  cruise->target_speed_cms = 0;
  cruise->current_speed_cms = 0;
  cruise->repeat_timer = SOFT_TIMER_INVALID_TIMER;

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
  switch (e->id) {
    case INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL:
      soft_timer_cancel(cruise->repeat_timer);
      cruise->repeat_timer = SOFT_TIMER_INVALID_TIMER;
      break;
    case INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS:
    case INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS:
      // Run callback immediately
      soft_timer_cancel(cruise->repeat_timer);
      cruise->offset_cms = CRUISE_OFFSET_CMS;
      if (e->id == INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS) {
        cruise->offset_cms = -CRUISE_OFFSET_CMS;
      }
      soft_timer_start(0, prv_timer_cb, cruise, &cruise->repeat_timer);
      break;
    case INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED:
      cruise->target_speed_cms = cruise->current_speed_cms;
      break;
    default:
      return false;
  }

  return true;
}

CruiseStorage *cruise_global(void) {
  return &s_cruise_storage;
}
