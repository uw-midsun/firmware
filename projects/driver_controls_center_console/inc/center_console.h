#pragma once
// The Center Console module handles all the logic for the buttons presses
// for buttons mounted in the Center Console. It raises CAN events to the
// Driver Controls master.
//
// Requires the following to be initialized:
// - CAN
// - GPIO
// - GPIO IT
#include "exported_enums.h"
#include "gpio.h"
#include "status.h"

// CenterConsoleInput corresponds to the sources that handle input on a GPIO
// pin and raises the appropriate Events
typedef struct {
  // GPIO pin the button is mapped to
  GpioAddress pin_address;
  // CAN event to raise
  EECenterConsoleDigitalInput can_event;
} CenterConsoleInput;

// Storage structure
typedef struct {
  // Momentary Switches are switches that are active on press. They may or may
  // not exhibit toggle behaviour, but from a physical perspective they don't
  // stay pressed.
  CenterConsoleInput momentary_switch_lights_low_beam;
  CenterConsoleInput momentary_switch_lights_drl;

  // Toggle Switches are switches that are used as Toggle buttons. They remain
  // pressed from a physical perspective.
  CenterConsoleInput toggle_switch_lights_hazards;
  CenterConsoleInput toggle_switch_power;

  // Momentary Switches that are used as Radio Button Group. Only one of the
  // elements in this group are Active at a time.
  CenterConsoleInput radio_button_drive;
  CenterConsoleInput radio_button_neutral;
  CenterConsoleInput radio_button_reverse;

  // Used to store the previous state of the power pin, due to a HW bug where
  // 2 pins are used that share the same EXTI line.
  //
  // TODO(HW-200): Once the hardware bug gets fixed, then remove this variable,
  // as this should be handled by the GPIO IT.
  GpioState prev_state;
} CenterConsoleStorage;

// Initialize the center console
StatusCode center_console_init(CenterConsoleStorage *storage);

// Poll for the power pin state. This should be called on each iteration of the
// main loop.
//
// Since LOW_BEAM (PA0) and POWER (PB0) share the same EXTI line, we can't
// register a separate ISR to handle based on (Port, Pin). The EXTIx line
// is multiplexed, and so you can't trigger the interrupt from multiple
// ports.
//
// TODO(HW-200): Once the HW fix is in, this should be removed, as this should
// be handled by the GPIO IT.
void center_console_poll(CenterConsoleStorage *storage);
