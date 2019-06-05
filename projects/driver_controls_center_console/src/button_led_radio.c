#include "button_led_radio.h"

#include "button_led_radio_fsm.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "fsm.h"
#include "input_event.h"

static Fsm s_button_led_radio_fsms = { 0 };

// Used to update the direction LED indicators
static StatusCode prv_direction_rx_handler(const CanMessage *msg, void *context,
                                           CanAckStatus *ack_reply) {
  uint16_t throttle = 0;
  uint16_t direction = 0;
  uint16_t cruise_control = 0;
  uint16_t mech_brake = 0;
  CAN_UNPACK_DRIVE_OUTPUT(msg, &throttle, &direction, &cruise_control, &mech_brake);

  EventId can_to_local_event_map[] = {
    [EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL] = CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_NEUTRAL,
    [EE_DRIVE_OUTPUT_DIRECTION_FORWARD] = CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_DRIVE,
    [EE_DRIVE_OUTPUT_DIRECTION_REVERSE] = CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_REVERSE,
  };
  if (direction >= SIZEOF_ARRAY(can_to_local_event_map)) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  event_raise(can_to_local_event_map[direction], 0);
  return STATUS_CODE_OK;
}

StatusCode button_led_radio_init(GpioExpanderStorage *storage, ButtonLedRadioSettings *settings) {
  button_led_radio_fsm_init();

  button_led_radio_fsm_create(&s_button_led_radio_fsms, storage, settings, "RadioButtonFsm");

  // Enable RX handler to update Direction LEDs
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, prv_direction_rx_handler, NULL));

  return STATUS_CODE_OK;
}

bool button_led_radio_process_event(const Event *e) {
  bool processed = false;
  processed |= fsm_process_event(&s_button_led_radio_fsms, e);
  return processed;
}
