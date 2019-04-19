#include "button_led.h"

#include "button_led_fsm.h"
#include "fsm.h"
#include "gpio.h"

static Fsm s_button_led_fsms[NUM_CENTER_CONSOLE_BUTTON_LEDS];

void button_led_init(GpioExpanderStorage *storage, GpioExpanderPin *pins) {
  button_led_fsm_init();

  // Create FSMs for each LED output
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_BPS], storage,
                        CENTER_CONSOLE_BUTTON_LED_BPS, pins[CENTER_CONSOLE_BUTTON_LED_BPS],
                        "BPSIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_PWR], storage,
                        CENTER_CONSOLE_BUTTON_LED_PWR, pins[CENTER_CONSOLE_BUTTON_LED_PWR],
                        "PowerIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_REVERSE], storage,
                        CENTER_CONSOLE_BUTTON_LED_REVERSE, pins[CENTER_CONSOLE_BUTTON_LED_REVERSE],
                        "ReverseIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_NEUTRAL], storage,
                        CENTER_CONSOLE_BUTTON_LED_NEUTRAL, pins[CENTER_CONSOLE_BUTTON_LED_NEUTRAL],
                        "NeutralIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_DRIVE], storage,
                        CENTER_CONSOLE_BUTTON_LED_DRIVE, pins[CENTER_CONSOLE_BUTTON_LED_DRIVE],
                        "DriveIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_DRL], storage,
                        CENTER_CONSOLE_BUTTON_LED_DRL, pins[CENTER_CONSOLE_BUTTON_LED_DRL],
                        "DRLIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS], storage,
                        CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS,
                        pins[CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS], "LowBeams");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_HAZARDS], storage,
                        CENTER_CONSOLE_BUTTON_LED_HAZARDS, pins[CENTER_CONSOLE_BUTTON_LED_HAZARDS],
                        "Hazards");
}

bool button_led_process_event(const Event *e) {
  bool processed = false;
  for (size_t i = 0; i < NUM_CENTER_CONSOLE_BUTTON_LEDS; ++i) {
    processed |= fsm_process_event(&s_button_led_fsms[i], e);
  }
  return processed;
}
