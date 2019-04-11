#include "cruise.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "cc_input_event.h"
#include "log.h"
#include "misc.h"

static CruiseStorage s_cruise_storage;

static StatusCode prv_handle_motor_velocity(const CanMessage *msg, void *context,
                                            CanAckStatus *ack_reply) {
  CruiseStorage *cruise = context;

  int16_t left = 0, right = 0;
  CAN_UNPACK_MOTOR_VELOCITY(msg, (uint16_t *)&left, (uint16_t *)&right);
  // If we ever overflow we have bigger problems
  cruise->current_speed_cms = MAX((left + right) / 2, 0);

  return event_raise(INPUT_EVENT_SPEED_UPDATE, (uint16_t)cruise->current_speed_cms);
}

static void prv_timer_cb(SoftTimerId timer_id, void *context) {
  CruiseStorage *cruise = context;

  cruise_set_target_cms(cruise, cruise->target_speed_cms + cruise->offset_cms);

  // __builtin_clz(0) has undefined behavior, so we increment first
  size_t index = 32 - (size_t)__builtin_clz(++cruise->repeat_counter);
  // Always require at least 1ms delay
  uint32_t timeout_ms = MAX((uint32_t)1, 1000 / index);

  soft_timer_start_millis(timeout_ms, prv_timer_cb, cruise, &cruise->repeat_timer);
}

StatusCode cruise_init(CruiseStorage *cruise) {
  cruise->target_speed_cms = 0;
  cruise->current_speed_cms = 0;
  cruise->repeat_counter = 0;
  cruise->repeat_timer = SOFT_TIMER_INVALID_TIMER;

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY, prv_handle_motor_velocity, cruise);

  return STATUS_CODE_OK;
}

StatusCode cruise_set_target_cms(CruiseStorage *cruise, int16_t target) {
  if (target < 0) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  cruise->target_speed_cms = target;
  cruise->target_speed_cms = MAX(0, cruise->target_speed_cms);
  cruise->target_speed_cms = MIN(CRUISE_MAX_TARGET_CMS, cruise->target_speed_cms);

  return CAN_TRANSMIT_CRUISE_TARGET(cruise->target_speed_cms);
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
      // Fall-through since it's handled using the same timer
    case INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS:
      // reset just in case we switched directly
      soft_timer_cancel(cruise->repeat_timer);
      cruise->repeat_counter = 0;

      cruise->offset_cms = CRUISE_OFFSET_CMS;
      if (e->id == INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS) {
        cruise->offset_cms = -CRUISE_OFFSET_CMS;
      }

      // Immediately run callback - note that we require a delay > 0 or it might not run
      soft_timer_start(SOFT_TIMER_MIN_TIME_US, prv_timer_cb, cruise, &cruise->repeat_timer);
      break;
    case INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED:
      cruise_set_target_cms(cruise, cruise->current_speed_cms);
      break;
    default:
      return false;
  }

  return true;
}

CruiseStorage *cruise_global(void) {
  return &s_cruise_storage;
}
