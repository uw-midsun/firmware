#include "control_stalk.h"

#include <string.h>

#include "adc.h"
#include "config.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"

#include "can_transmit.h"

#define STEERING_ADC_PERIOD_MILLIS 50u

// Analog Inputs
typedef struct {
  // CAN events to raise
  EventId can_event[NUM_CONTROL_STALK_STATES];
  // Pin of the Analog Input
  GpioAddress address;
} SteeringAdcInput;

// Digital Inputs
typedef struct {
  // CAN events to raise
  EventId can_event[NUM_GPIO_STATES];
  // Pin of the Digital Input
  GpioAddress pin;
} SteeringDigitalInput;

static ControlStalkState s_stalk_state[NUM_ADC_CHANNELS] = { NUM_CONTROL_STALK_STATES };

// TODO: Move this to configuration in the init function
static SteeringDigitalInput s_digital_inputs[] = {
  {
      .pin = STEERING_CONFIG_PIN_HORN,
      .can_event =
          {
              [GPIO_STATE_HIGH] = EE_STEERING_INPUT_HORN_PRESSED,  //
              [GPIO_STATE_LOW] = EE_STEERING_INPUT_HORN_RELEASED   //
          },
  },
  {
      .pin = STEERING_CONFIG_PIN_CC_ON_OFF,
      .can_event =
          {
              [GPIO_STATE_HIGH] = EE_STEERING_INPUT_CC_ON_OFF_PRESSED,  //
              [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_ON_OFF_RELEASED   //
          },
  },
  {
      .pin = STEERING_CONFIG_PIN_CC_SET,
      .can_event =
          {
              [GPIO_STATE_HIGH] = EE_STEERING_INPUT_CC_SET_PRESSED,  //
              [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_SET_RELEASED   //
          },
  }
};

static SteeringAdcInput s_analog_inputs[] = {
  {
      .address = STEERING_CONFIG_PIN_CC_SPEED,
      .can_event =
          {
              [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_INPUT_CC_SPEED_NEUTRAL,  //
              [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_INPUT_CC_SPEED_MINUS,    //
              [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_INPUT_CC_SPEED_PLUS     //
          },
  },
  {
      .address = STEERING_CONFIG_PIN_CC_CANCEL_RESUME,
      .can_event =
          {
              [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_INPUT_CC_CANCEL_RESUME_NEUTRAL,  //
              [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_INPUT_CC_CANCEL_RESUME_CANCEL,   //
              [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_INPUT_CC_CANCEL_RESUME_RESUME   //
          },
  },
  {
      .address = STEERING_CONFIG_PIN_TURN_SIGNAL_STALK,
      .can_event =
          {
              [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_INPUT_TURN_SIGNAL_STALK_NONE,   //
              [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_INPUT_TURN_SIGNAL_STALK_RIGHT,  //
              [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_INPUT_TURN_SIGNAL_STALK_LEFT   //
          },
  },
  {
      .address = STEERING_CONFIG_PIN_CC_DISTANCE,
      .can_event =
          {
              [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_INPUT_EVENT_CC_DISTANCE_NEUTRAL,  //
              [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_INPUT_EVENT_CC_DISTANCE_MINUS,    //
              [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_INPUT_EVENT_CC_DISTANCE_PLUS     //
          },
  }
};

static void prv_adc_callback(SoftTimerId timer_id, void *context) {
  for (size_t i = 0; i < SIZEOF_ARRAY(s_analog_inputs); ++i) {
    AdcChannel channel = NUM_ADC_CHANNELS;
    adc_get_channel(s_analog_inputs[i].address, &channel);

    uint16_t reading = 0;
    adc_read_converted(channel, &reading);

    ControlStalkState state = CONTROL_STALK_STATE_FLOATING;
    if (reading <= CONTROL_STALK_681_OHMS_THRESHOLD) {
      state = CONTROL_STALK_STATE_681_OHMS;
    } else if (reading <= CONTROL_STALK_2181_OHMS_THRESHOLD) {
      state = CONTROL_STALK_STATE_2181_OHMS;
    }

    // If the previous state has changed, then we send an input
    if (s_stalk_state[channel] != state) {
      // Raise event on the master via a CAN message
      CAN_TRANSMIT_STEERING_EVENT(s_analog_inputs[i].can_event[state], 0);
    }
    s_stalk_state[channel] = state;
  }

  // Start next soft timer
  soft_timer_start_millis(STEERING_ADC_PERIOD_MILLIS, prv_adc_callback, s_analog_inputs, NULL);
}

void prv_gpio_callback(const GpioAddress *address, void *context) {
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);

  EventId *can_event = context;
  // Raise event on the master via a CAN message
  CAN_TRANSMIT_STEERING_EVENT(can_event[(size_t)state], 0);
}

StatusCode control_stalk_init(void) {
  memset(s_stalk_state, NUM_CONTROL_STALK_STATES, sizeof(s_stalk_state));

  // Initialize all GPIO pins for Digital Inputs
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(s_digital_inputs); ++i) {
    status_ok_or_return(gpio_init_pin(&s_digital_inputs[i].pin, &digital_input_settings));

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,      //
      .priority = INTERRUPT_PRIORITY_NORMAL  //
    };
    // Initialize GPIO Interrupts to raise events to change
    status_ok_or_return(gpio_it_register_interrupt(&s_digital_inputs[i].pin, &interrupt_settings,
                                                   INTERRUPT_EDGE_RISING_FALLING, prv_gpio_callback,
                                                   s_digital_inputs[i].can_event));
  }

  // Initialize all GPIO pins for Analog Inputs
  GpioSettings adc_input_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(s_analog_inputs); ++i) {
    status_ok_or_return(gpio_init_pin(&s_analog_inputs[i].address, &adc_input_settings));
    AdcChannel channel = NUM_ADC_CHANNELS;

    status_ok_or_return(adc_get_channel(s_analog_inputs[i].address, &channel));
    status_ok_or_return(adc_set_channel(channel, true));
  }

  status_ok_or_return(
      soft_timer_start_millis(STEERING_ADC_PERIOD_MILLIS, prv_adc_callback, s_analog_inputs, NULL));

  return STATUS_CODE_OK;
}
