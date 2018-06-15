#include "control_stalk.h"
#include <string.h>
#include "input_event.h"
#include "log.h"

// The A6 control stalks have two types of inputs: analog and digital.
// Analog inputs are either floating, 2.1818 kOhms to GND, or 681 Ohms to GND. We tie a resistor
//  to 3V to create a resistor divider and observe the voltage for state changes.
// Digital inputs are either floating or connected to GND. We use a pull-up resistor to make it an
//  active-low digital input.
//
// We map the inputs as follows:
// ADS1015 (analog)
// * A0: Distance
// * A1: CC Speed
// * A2: CC Cancel/Resume (Soft)
// * A3: Turn Signals
// MCP23008 (digital, active-low)
// * A0: CC Set
// * A1: CC On/Off (Hard)
// * A2: Lane Assist
// * A3: Headlight (forward)
// * A4: Headlight (back)
// * A5: Horn

// 4096 codes for +/-4.096V -> LSB = 2mV
#define CONTROL_STALK_THRESHOLD(ohms) ((1 << 12) / 2 * (ohms) / ((CONTROL_STALK_RESISTOR) + (ohms)))
// 2k181 +10% resistor = ~2k4, -10% = 1k963
#define CONTROL_STALK_2181_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(2400)
// 681 +10% resistor = ~750, -10% = 613
#define CONTROL_STALK_681_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(750)

// ADC channel to Event ID
static const EventID s_analog_mapping[CONTROL_STALK_ANALOG_INPUTS][NUM_CONTROL_STALK_STATES] = {
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
static const EventID s_digital_mapping[CONTROL_STALK_DIGITAL_INPUTS][NUM_GPIO_STATES] = {
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
    INPUT_EVENT_MECHANICAL_BRAKE_PRESSED,
    INPUT_EVENT_MECHANICAL_BRAKE_RELEASED,
      // INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED,
      // INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_RELEASED,
  },
  {
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_PRESSED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_FWD_RELEASED,
  },
  {
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_PRESSED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HEADLIGHT_BACK_RELEASED,
  },
  {
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HORN_PRESSED,
      INPUT_EVENT_CONTROL_STALK_DIGITAL_HORN_RELEASED,
  }
};

static void prv_analog_cb(Ads1015Channel channel, void *context) {
  ControlStalk *stalk = context;
  int16_t reading = 0;
  ads1015_read_raw(stalk->ads1015, channel, &reading);

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

void prv_digital_cb(GpioExpanderPin pin, GPIOState state, void *context) {
  ControlStalk *stalk = context;
  event_raise(s_digital_mapping[pin][state], 0);
}

StatusCode control_stalk_init(ControlStalk *stalk, Ads1015Storage *ads1015,
                              GpioExpanderStorage *expander) {
  memset(stalk, 0, sizeof(*stalk));
  stalk->ads1015 = ads1015;
  stalk->expander = expander;

  for (Ads1015Channel channel = 0; channel < CONTROL_STALK_ANALOG_INPUTS; channel++) {
    stalk->states[channel] = NUM_CONTROL_STALK_STATES;
    ads1015_configure_channel(stalk->ads1015, channel, true, prv_analog_cb, stalk);
  }

  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_PULLUP,
  };

  for (GpioExpanderPin pin = 0; pin < CONTROL_STALK_DIGITAL_INPUTS; pin++) {
    status_ok_or_return(gpio_expander_init_pin(expander, pin, &gpio_settings));
    status_ok_or_return(gpio_expander_register_callback(expander, pin, prv_digital_cb, stalk));
  }

  return STATUS_CODE_OK;
}
