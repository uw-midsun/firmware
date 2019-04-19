#include "button_led_radio.h"

#include "button_led_radio_fsm.h"
#include "fsm.h"

static Fsm s_button_led_radio_fsms = { 0 };

void button_led_radio_init(GpioExpanderStorage *storage, ButtonLedRadioSettings *settings) {
  button_led_radio_fsm_init();

  button_led_radio_fsm_create(&s_button_led_radio_fsms, storage, settings, "RadioButtonFsm");
}

bool button_led_radio_process_event(const Event *e) {
  bool processed = false;
  processed |= fsm_process_event(&s_button_led_radio_fsms, e);
  return processed;
}
