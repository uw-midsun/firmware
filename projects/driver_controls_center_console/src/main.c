#include <stdbool.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#include "adc.h"
#include "exported_enums.h"
#include "input_event.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

#include "log.h"

#include "config.h"

#include "bms_heartbeat.h"
#include "button_led.h"
#include "button_led_fsm.h"
#include "button_led_radio.h"
#include "center_console_flags.h"
#include "exported_enums.h"

static CanStorage s_can_storage = { 0 };
static GpioExpanderStorage s_expander;

// CenterConsoleInput corresponds to the Events that are raised resulting from
// a input from a GPIO pin
typedef struct {
  // GPIO pin the button is mapped to
  GpioAddress pin_address;
  // CAN event to raise
  EECenterConsoleDigitalInput can_event;
} CenterConsoleInput;

static GpioExpanderPin s_expander_pin[NUM_CENTER_CONSOLE_BUTTON_LEDS] = {
  [CENTER_CONSOLE_BUTTON_LED_BPS] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_BPS,
  [CENTER_CONSOLE_BUTTON_LED_PWR] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_POWER,
  [CENTER_CONSOLE_BUTTON_LED_REVERSE] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_REVERSE,
  [CENTER_CONSOLE_BUTTON_LED_NEUTRAL] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_NEUTRAL,
  [CENTER_CONSOLE_BUTTON_LED_DRIVE] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRIVE,
  [CENTER_CONSOLE_BUTTON_LED_DRL] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRL,
  [CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_LOW_BEAMS,
  [CENTER_CONSOLE_BUTTON_LED_HAZARDS] = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_HAZARDS
};

// Momentary Switches that are used as Toggle buttons
static CenterConsoleInput s_momentary_switch_input[] = {
  {
      .pin_address = CENTER_CONSOLE_CONFIG_PIN_LOW_BEAM,
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM,
  },
  {
      .pin_address = CENTER_CONSOLE_CONFIG_PIN_DRL,
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL,
  },
};
// TODO: Move this into the toggle button array once the pin gets moved to a
// different EXTI line.
static CenterConsoleInput s_power_input = {
  .pin_address = CENTER_CONSOLE_CONFIG_PIN_POWER,      //
  .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER,  //
};

// Toggle Switches that are used as Toggle buttons
static CenterConsoleInput s_toggle_switch_input[] = { {
    .pin_address = CENTER_CONSOLE_CONFIG_PIN_HAZARDS,
    .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS,
} };

// Momentary Switches that are used as Radio Button Group
static CenterConsoleInput s_radio_button_group[] = {
  {
      .pin_address = CENTER_CONSOLE_CONFIG_PIN_DRIVE,
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE,
  },
  {
      .pin_address = CENTER_CONSOLE_CONFIG_PIN_NEUTRAL,
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL,
  },
  {
      .pin_address = CENTER_CONSOLE_CONFIG_PIN_REVERSE,
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE,
  }
};

void prv_gpio_toggle_callback(const GpioAddress *address, void *context) {
  // Simple toggles just toggle state with a shared event,
  // CENTER_CONSOLE_EVENT_BUTTON_TOGGLE_STATE, with the data field as the
  // actual IO we are toggling
  CenterConsoleInput *toggle_button = context;

  // Raise event via CAN message
  CAN_TRANSMIT_CENTER_CONSOLE_EVENT(toggle_button->can_event, 0);
  LOG_DEBUG("Center Console\n");
}

void prv_gpio_radio_callback(const GpioAddress *address, void *context) {
  // Radio buttons switch state by using the event id and passing 0 data
  CenterConsoleInput *toggle_button = context;

  // Raise event via CAN message
  CAN_TRANSMIT_CENTER_CONSOLE_EVENT(toggle_button->can_event, 0);
}

#define CENTER_CONSOLE_5V_RAIL_MONITOR_PERIOD_MILLIS 100

void prv_adc_monitor(SoftTimerId timer_id, void *context) {
  uint16_t *rail_monitor_5v = context;

  GpioAddress monitor_5v = CENTER_CONSOLE_CONFIG_PIN_5V_MONITOR;
  AdcChannel adc_channel_5v_monitor = NUM_ADC_CHANNELS;
  adc_get_channel(monitor_5v, &adc_channel_5v_monitor);

  adc_read_converted(adc_channel_5v_monitor, rail_monitor_5v);

  // TODO: Any logic here to monitor 5V rail and raise appropriate event

  soft_timer_start_millis(CENTER_CONSOLE_5V_RAIL_MONITOR_PERIOD_MILLIS, prv_adc_monitor, context,
                          NULL);
}

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

// Callback to update the Lights LEDs using CAN
static StatusCode prv_lights_state_rx_handler(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  uint8_t light_id = 0;
  uint8_t light_state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &light_id, &light_state);

  if (light_state >= NUM_EE_LIGHT_STATES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }
  if (light_id >= NUM_EE_LIGHT_TYPES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  EventId state_to_local_event_map[] = {
    [EE_LIGHT_STATE_ON] = CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON,
    [EE_LIGHT_STATE_OFF] = CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF,
  };

  // If it's one of the Light statuses we care about then process it
  switch (light_id) {
    case EE_LIGHT_TYPE_LOW_BEAMS:
      event_raise(state_to_local_event_map[light_state], EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM);
      break;
    case EE_LIGHT_TYPE_DRL:
      event_raise(state_to_local_event_map[light_state], EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL);
      break;
    case EE_LIGHT_TYPE_SIGNAL_HAZARD:
      event_raise(state_to_local_event_map[light_state], EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS);
      break;
    default:
      return STATUS_CODE_OK;
  }

  return STATUS_CODE_OK;
}

// Callback to update the Power LED
static StatusCode prv_power_state_rx_handler(const CanMessage *msg, void *context,
                                             CanAckStatus *ack_reply) {
  uint8_t power_state = 0;
  CAN_UNPACK_POWER_STATE(msg, &power_state);

  switch (power_state) {
    case EE_POWER_STATE_DRIVE:
      event_raise(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER);
      break;
    default:
      event_raise(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER);
      break;
  }

  return STATUS_CODE_OK;
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
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CENTER_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTER_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTER_CONSOLE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  // Set up Center Console Buttons
  GpioSettings button_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(s_momentary_switch_input); ++i) {
    status_ok_or_return(
        gpio_init_pin(&s_momentary_switch_input[i].pin_address, &button_input_settings));

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,       //
      .priority = INTERRUPT_PRIORITY_NORMAL,  //
    };
    // Initialize GPIO Interrupts to raise events to change LED status
    status_ok_or_return(gpio_it_register_interrupt(
        &s_momentary_switch_input[i].pin_address, &interrupt_settings, INTERRUPT_EDGE_RISING,
        prv_gpio_toggle_callback, &s_momentary_switch_input[i]));
  }

  // Enable RX handler to update car Lights LEDs
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_STATE, prv_lights_state_rx_handler, NULL));

  // TODO: Move this into the above once this gets fixed in hardware
  status_ok_or_return(gpio_init_pin(&s_power_input.pin_address, &button_input_settings));

  // Enable RX handler to update power state LED
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_power_state_rx_handler, NULL));

  // We need a special case for the Hazards button, since it is a toggle switch
  for (size_t i = 0; i < SIZEOF_ARRAY(s_toggle_switch_input); ++i) {
    status_ok_or_return(
        gpio_init_pin(&s_toggle_switch_input[i].pin_address, &button_input_settings));

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,       //
      .priority = INTERRUPT_PRIORITY_NORMAL,  //
    };
    // Initialize GPIO Interrupts to raise events to change LED status
    status_ok_or_return(gpio_it_register_interrupt(
        &s_toggle_switch_input[i].pin_address, &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
        prv_gpio_toggle_callback, &s_toggle_switch_input[i]));
  }

  for (size_t i = 0; i < SIZEOF_ARRAY(s_radio_button_group); ++i) {
    status_ok_or_return(
        gpio_init_pin(&s_radio_button_group[i].pin_address, &button_input_settings));

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,       //
      .priority = INTERRUPT_PRIORITY_NORMAL,  //
    };
    // Initialize GPIO Interrupts to raise events to change LED status
    status_ok_or_return(gpio_it_register_interrupt(
        &s_radio_button_group[i].pin_address, &interrupt_settings, INTERRUPT_EDGE_RISING,
        prv_gpio_radio_callback, &s_radio_button_group[i]));
  }

  // Enable RX handler to update Direction LEDs
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_OUTPUT, prv_direction_rx_handler, NULL));

  // Use I/O Expander for button LEDs
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,                   //
    .sda = CENTER_CONSOLE_CONFIG_PIN_I2C_SDA,  //
    .scl = CENTER_CONSOLE_CONFIG_PIN_I2C_SDL,  //
  };
  status_ok_or_return(i2c_init(I2C_PORT_2, &settings));
  // Configure the expander to be output only
  status_ok_or_return(gpio_expander_init(&s_expander, I2C_PORT_2, GPIO_EXPANDER_ADDRESS_0, NULL));
  for (size_t i = 0; i < SIZEOF_ARRAY(s_expander_pin); ++i) {
    // Start with all buttons with low
    GpioSettings output_settings = {
      .direction = GPIO_DIR_OUT,  //
      .state = GPIO_STATE_LOW,    //
    };
    status_ok_or_return(gpio_expander_init_pin(&s_expander, s_expander_pin[i], &output_settings));
  }

  // Initialize normal toggle buttons
  button_led_init(&s_expander, s_expander_pin);
  // Initialize radio button groups
  ButtonLedRadioSettings radio_settings = {
    .reverse_pin = s_expander_pin[CENTER_CONSOLE_BUTTON_LED_REVERSE],
    .neutral_pin = s_expander_pin[CENTER_CONSOLE_BUTTON_LED_NEUTRAL],
    .drive_pin = s_expander_pin[CENTER_CONSOLE_BUTTON_LED_DRIVE],
  };
  button_led_radio_init(&s_expander, &radio_settings);

  // Enable 5V monitor
  GpioSettings adc_input_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };
  GpioAddress monitor_5v = CENTER_CONSOLE_CONFIG_PIN_5V_MONITOR;
  status_ok_or_return(gpio_init_pin(&monitor_5v, &adc_input_settings));
  AdcChannel adc_channel_5v_monitor = NUM_ADC_CHANNELS;

  // TODO: Why doesn't continuous mode work smh
  adc_init(ADC_MODE_SINGLE);

  uint16_t rail_monitor_5v = 0u;
  status_ok_or_return(adc_get_channel(monitor_5v, &adc_channel_5v_monitor));
  status_ok_or_return(adc_set_channel(adc_channel_5v_monitor, true));
  /* adc_register_callback(ADC_CHANNEL_9, prv_adc_monitor, (void *)&rail_monitor_5v); */
  status_ok_or_return(soft_timer_start_millis(CENTER_CONSOLE_5V_RAIL_MONITOR_PERIOD_MILLIS,
                                              prv_adc_monitor, (void *)&rail_monitor_5v, NULL));

  GpioSettings enable_output_rail = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
#ifdef CENTER_CONSOLE_FLAG_ENABLE_5V_RAIL
  // Enable 5V rail
  GpioAddress rail_5v = CENTER_CONSOLE_CONFIG_PIN_5V_ENABLE;
  status_ok_or_return(gpio_init_pin(&rail_5v, &enable_output_rail));
#endif

#ifdef CENTER_CONSOLE_FLAG_ENABLE_DISPLAY
  // Enable Driver Display
  GpioAddress display_rail = CENTER_CONSOLE_CONFIG_PIN_DISPLAY_ENABLE;
  status_ok_or_return(gpio_init_pin(&display_rail, &enable_output_rail));
#endif

  // Since LOW_BEAM (PA0) and POWER (PB0) share the same EXTI line, we can't
  // register a separate ISR to handle based on (Port, Pin). The EXTIx line
  // is multiplexed, and so you can't trigger the interrupt from multiple
  // ports.
  //
  // As a workaround, we poll in the main loop
  GpioAddress addr = CENTER_CONSOLE_CONFIG_PIN_POWER;
  GpioState prev_state = GPIO_STATE_LOW;
  volatile GpioState curr_state = GPIO_STATE_LOW;

  Event e = { 0 };

  // Start BMS heartbeat handler
  bms_heartbeat_init();
  while (true) {
    gpio_get_state(&addr, &curr_state);
    // LOW -> HI
    if (curr_state == GPIO_STATE_HIGH && prev_state == GPIO_STATE_LOW) {
      // Raise event via CAN message
      CAN_TRANSMIT_CENTER_CONSOLE_EVENT(s_power_input.can_event, 0);
      LOG_DEBUG("State: %d\n", curr_state);
    }
    prev_state = curr_state;

    while (status_ok(event_process(&e))) {
      can_process_event(&e);

      button_led_process_event(&e);
      button_led_radio_process_event(&e);
    }
  }
}
