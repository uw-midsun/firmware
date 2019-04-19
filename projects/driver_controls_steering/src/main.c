#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "status.h"
#include "wait.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"

#include "exported_enums.h"

#include "config.h"
#include "input_event.h"
static CanStorage s_can_storage;
#define STEERING_ADC_PERIOD_MILLIS 50u

// Describes the state of the non-fixed resistor
typedef enum ControlStalkState {
  CONTROL_STALK_STATE_FLOATING = 0,
  CONTROL_STALK_STATE_681_OHMS,
  CONTROL_STALK_STATE_2181_OHMS,
  NUM_CONTROL_STALK_STATES
} ControlStalkState;

typedef struct {
  AdcChannel channel;
  GpioAddress address;
  EventId input_event[NUM_CONTROL_STALK_STATES];
} SteeringAdcInput;

typedef struct {
  GpioAddress pin;
  EventId input_event[NUM_GPIO_STATES];
} SteeringDigitalInput;

// We only need the following inputs from the Steering Board:
// * CC_SPEED: Used to increase/decrease the current cruise target
// * CC_SET: Used to set the cruise target to the current speed
// * CC_ON/OFF: Used to enable/disable cruise control
// * TURN_SIGNAL_STALK: Used to designate left/right turn signals
static SteeringDigitalInput s_digital_inputs[] = {
  {
      .pin = STEERING_CONFIG_PIN_HORN,
      .input_event =
          {
              [GPIO_STATE_HIGH] = EE_STEERING_DIGITAL_INPUT_HORN_PRESSED,  //
              [GPIO_STATE_LOW] = EE_STEERING_DIGITAL_INPUT_HORN_RELEASED   //
          },
  },
  {
      .pin = STEERING_CONFIG_PIN_CC_ON_OFF,
      .input_event =
          {
              [GPIO_STATE_HIGH] = EE_STEERING_DIGITAL_INPUT_CC_ON_OFF_PRESSED,  //
              [GPIO_STATE_LOW] = EE_STEERING_DIGITAL_INPUT_CC_ON_OFF_RELEASED   //
          },
  },
  {
      .pin = STEERING_CONFIG_PIN_CC_SET,
      .input_event =
          {
              [GPIO_STATE_HIGH] = EE_STEERING_DIGITAL_INPUT_CC_SET_PRESSED,  //
              [GPIO_STATE_LOW] = EE_STEERING_DIGITAL_INPUT_CC_SET_RELEASED   //
          },
  }
};

static SteeringAdcInput s_analog_inputs[] = {
  {
      .address = STEERING_CONFIG_PIN_CC_SPEED,
      .channel = ADC_CHANNEL_1,
      .input_event =
          {
              [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_ANALOG_INPUT_CC_SPEED_NEUTRAL,  //
              [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_ANALOG_INPUT_CC_SPEED_MINUS,    //
              [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_ANALOG_INPUT_CC_SPEED_PLUS      //
          },
  },
  {
      .address = STEERING_CONFIG_PIN_CC_CANCEL_RESUME,
      .channel = ADC_CHANNEL_2,
      .input_event =
          {
              [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_ANALOG_INPUT_CC_CANCEL_RESUME_NEUTRAL,  //
              [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_ANALOG_INPUT_CC_CANCEL_RESUME_CANCEL,    //
              [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_ANALOG_INPUT_CC_CANCEL_RESUME_RESUME      //
          },
  },
  {
      .address = STEERING_CONFIG_PIN_TURN_SIGNAL_STALK,
      .channel = ADC_CHANNEL_3,
      .input_event =
          {
              [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_ANALOG_INPUT_TURN_SIGNAL_STALK_NONE,   //
              [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_ANALOG_INPUT_TURN_SIGNAL_STALK_RIGHT,  //
              [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_ANALOG_INPUT_TURN_SIGNAL_STALK_LEFT    //
          },
  }
};

void prv_gpio_callback(const GpioAddress *address, void *context) {
  // TODO: Do we need to debounce this?
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);

  EventId *input_event = context;
  // Raise event via CAN message
  CAN_TRANSMIT_STEERING_EVENT(input_event[(size_t)state], 0);
}

// Resistor divider value in ohms
#define CONTROL_STALK_RESISTOR 3800
// 4096 codes for +/-4.096V -> LSB = 2mV
#define CONTROL_STALK_THRESHOLD(ohms) ((1 << 12) / 2 * (ohms) / ((CONTROL_STALK_RESISTOR) + (ohms)))
// 2k181 +10% resistor = ~2k4, -10% = 1k963
#define CONTROL_STALK_2181_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(2400)
// 681 +10% resistor = ~750, -10% = 613
#define CONTROL_STALK_681_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(750)
ControlStalkState stalk_state[NUM_ADC_CHANNELS] = {CONTROL_STALK_STATE_FLOATING};

void prv_adc_callback(AdcChannel adc_channel, void *context) {
  EventId *input_event = context;

  uint16_t reading = 0;
  adc_read_converted(adc_channel, &reading);

  ControlStalkState state = CONTROL_STALK_STATE_FLOATING;
  if (reading <= CONTROL_STALK_681_OHMS_THRESHOLD) {
    state = CONTROL_STALK_STATE_681_OHMS;
  } else if (reading <= CONTROL_STALK_2181_OHMS_THRESHOLD) {
    state = CONTROL_STALK_STATE_2181_OHMS;
  }

  // TODO: Do we need to debounce this?
  // If the previous state has changed, then we send an input
  if (stalk_state[adc_channel] != state) {
    CAN_TRANSMIT_STEERING_EVENT(input_event[state], 0);
  }
  stalk_state[adc_channel] = state;
}

int main() {
  // Standard module inits
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // CAN initialization
  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = STEERING_EVENT_CAN_RX,
    .tx_event = STEERING_EVENT_CAN_TX,
    .fault_event = STEERING_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  // Enable Analog Inputs
  GpioSettings adc_input_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };
  adc_init(ADC_MODE_CONTINUOUS);
  for (size_t i = 0; i < SIZEOF_ARRAY(s_analog_inputs); ++i) {
    gpio_init_pin(&s_analog_inputs[i].address, &adc_input_settings);
    adc_set_channel(s_analog_inputs[i].channel, true);

    adc_register_callback(s_analog_inputs[i].channel, prv_adc_callback, s_analog_inputs[i].input_event);
  }

  // Enable Digital Inputs
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(s_digital_inputs); ++i) {
    gpio_init_pin(&s_digital_inputs[i].pin, &digital_input_settings);

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,      //
      .priority = INTERRUPT_PRIORITY_NORMAL  //
    };
    // Initialize GPIO Interrupts to raise events to change
    gpio_it_register_interrupt(&s_digital_inputs[i].pin, &interrupt_settings,
                               INTERRUPT_EDGE_RISING_FALLING, prv_gpio_callback,
                               s_digital_inputs[i].input_event);
  }

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    wait();

    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }

  return 0;
}
