#include "button_led.h"

#include "button_led_fsm.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"

#include "log.h"
static Fsm s_button_led_fsms[NUM_CENTER_CONSOLE_BUTTON_LEDS] = { 0 };

void button_led_init(GpioExpanderStorage *storage, GpioExpanderPin *pins) {
  button_led_fsm_init();

  // Create FSMs for each LED output
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_BPS], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_BPS, pins[CENTER_CONSOLE_BUTTON_LED_BPS],
                        "BPSIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_PWR], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER, pins[CENTER_CONSOLE_BUTTON_LED_PWR],
                        "PowerIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_DRL], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL, pins[CENTER_CONSOLE_BUTTON_LED_DRL],
                        "DRLIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM,
                        pins[CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS], "LowBeams");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_HAZARDS], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS,
                        pins[CENTER_CONSOLE_BUTTON_LED_HAZARDS], "Hazards");
}

bool button_led_process_event(const Event *e) {
  bool processed = false;
  for (size_t i = 0; i < NUM_CENTER_CONSOLE_BUTTON_LEDS; ++i) {
    // These are handled in the Radio FSM
    if (i == CENTER_CONSOLE_BUTTON_LED_REVERSE || i == CENTER_CONSOLE_BUTTON_LED_NEUTRAL ||
        i == CENTER_CONSOLE_BUTTON_LED_DRIVE) {
      continue;
    }
    processed |= fsm_process_event(&s_button_led_fsms[i], e);
  }
  return processed;
}
