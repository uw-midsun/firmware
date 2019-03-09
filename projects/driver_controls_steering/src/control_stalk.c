#include "control_stalk.h"
#include <string.h>
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "sc_cfg.h"
#include "sc_input_event.h"

// The A6 control stalks have two types of inputs: analog and digital.
// Analog inputs are either floating, 2.1818 kOhms to GND, or 681 Ohms to GND. We tie a resistor
//  to 3V to create a resistor divider and observe the voltage for state changes.
// ^^ Still Revelevant? Assuming yes because the control stalk has not changed

// Digital inputs are either floating or connected to GND. We use a pull-up resistor to make it an
//  active-low digital input.
//
// Inputs are as follows:
// * A0: AN0_CC_DISTANCE
// * A1: AN1_CC_SPEED
// * A2: AN2_CC_CANCEL/RESUME
// * A3: AN3_TURN_SIGNAL_STALK
// * A4: CC_SET
// * A5: CC_ON/OFF
// * A6: LANE_ASSIST
// * A7: HIGH_BEAM_FWD
// * B0: HIGH_BEAM_BACK

// 4096 codes for +/-4.096V -> LSB = 2mV
#define CONTROL_STALK_THRESHOLD(ohms) ((1 << 12) / 2 * (ohms) / ((CONTROL_STALK_RESISTOR) + (ohms)))
// 2k181 +10% resistor = ~2k4, -10% = 1k963
#define CONTROL_STALK_2181_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(2400)
// 681 +10% resistor = ~750, -10% = 613
#define CONTROL_STALK_681_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(750)

static const GpioAddress s_analog_gpio_addresses[CONTROL_STALK_ANALOG_INPUTS] = {
  SC_CFG_AN0_CC_DISTANCE_PIN,
  SC_CFG_AN1_CC_SPEED_PIN,
  SC_CFG_AN2_CC_CANCEL_RESUME_PIN,
  SC_CFG_AN3_CC_TURN_SIGNAL_PIN,
};

static const GpioAddress s_digital_gpio_addresses[CONTROL_STALK_DIGITAL_INPUTS] = {
  SC_CFG_CC_SET_PIN,        SC_CFG_CC_ON_OFF_PIN,      SC_CFG_LANE_ASSIST_PIN,
  SC_CFG_HIGH_BEAM_FWD_PIN, SC_CFG_HIGH_BEAM_BACK_PIN,
};

// ADC channel to Event ID
static const EventId s_analog_mapping[CONTROL_STALK_ANALOG_INPUTS][NUM_CONTROL_STALK_STATES] = {
  {
      INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL,
      INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_MINUS,
      INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_PLUS,
  },
  {
      INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL,
      INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS,
      INPUT_EVENT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS,
  },
  {
      INPUT_EVENT_CONTROL_STALK_ANALOG_CC_DIGITAL,
      INPUT_EVENT_CONTROL_STALK_ANALOG_CC_CANCEL,
      INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME,
  },
  {
      INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE,
      INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT,
      INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT,
  },
};

// Active-low
static const EventId s_digital_mapping[CONTROL_STALK_DIGITAL_INPUTS][NUM_GPIO_STATES] = {
  {
      INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_SET_RELEASED,
  },
  {
      INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_ON,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_OFF,
  },
  {
      // TODO(ELEC-461): revert when mech brake added
      // INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, INPUT_EVENT_MECHANICAL_BRAKE_RELEASED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_RELEASED,
      // Mech brake has been added so this should be good (?)
  },
  {
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED,
  },
  {
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_PRESSED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_RELEASED,
  },
};

static void prv_analog_cb(AdcChannel channel, void *context) {
  ControlStalk *stalk = context;
  uint16_t reading = 0;
  adc_read_raw(channel, &reading);

  ControlStalkState state = CONTROL_STALK_STATE_FLOATING;
  if (reading <= CONTROL_STALK_681_OHMS_THRESHOLD) {
    state = CONTROL_STALK_STATE_681_OHMS;
  } else if (reading <= CONTROL_STALK_2181_OHMS_THRESHOLD) {
    state = CONTROL_STALK_STATE_2181_OHMS;
  }

  if (stalk->states[channel] != state) {
    stalk->debounce_counter[channel] = 0;
    stalk->states[channel] = state;
  } else {
    stalk->debounce_counter[channel]++;
  }

  if (stalk->debounce_counter[channel] == CONTROL_STALK_DEBOUNCE_COUNTER_THRESHOLD) {
    event_raise(s_analog_mapping[channel][state], 0);
  }
}

void prv_digital_cb(const GpioAddress *address, void *context) {
  EventId *inputEvents = context;
  GpioState state;
  gpio_get_state(address, &state);
  event_raise(inputEvents[state], 0);
}

StatusCode control_stalk_init(ControlStalk *stalk) {
  memset(stalk, 0, sizeof(*stalk));

  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_PULLUP,
  };
  const InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  // Initialize Analog Pins
  for (size_t pin = 0; pin < CONTROL_STALK_ANALOG_INPUTS; pin++) {
    status_ok_or_return(gpio_init_pin(&s_analog_gpio_addresses[pin], &gpio_settings));
  }

  // Initialize Digital Pins with Interrupts
  for (size_t pin = 0; pin < CONTROL_STALK_DIGITAL_INPUTS; pin++) {
    status_ok_or_return(gpio_init_pin(&s_digital_gpio_addresses[pin], &gpio_settings));
    status_ok_or_return(gpio_it_register_interrupt(&s_digital_gpio_addresses[pin],
                                                   &interrupt_settings, INTERRUPT_EDGE_FALLING,
                                                   prv_digital_cb, s_digital_mapping[pin]));
  }

  for (AdcChannel channel = 0; channel < CONTROL_STALK_ANALOG_INPUTS; channel++) {
    stalk->states[channel] = NUM_CONTROL_STALK_STATES;
    adc_set_channel(channel, true);
    adc_register_callback(channel, prv_analog_cb, stalk);
  }

  return STATUS_CODE_OK;
}
