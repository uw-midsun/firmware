#pragma once
#include "fsm.h"
#include "gpio_expander.h"
#include "status.h"

typedef struct {
  GpioExpanderPin drive_pin;
  GpioExpanderPin neutral_pin;
  GpioExpanderPin reverse_pin;
} ButtonLedRadioSettings;

void button_led_radio_fsm_init(void);

StatusCode button_led_radio_fsm_create(Fsm *fsm, GpioExpanderStorage *storage,
                                       ButtonLedRadioSettings *settings, const char *fsm_name);
