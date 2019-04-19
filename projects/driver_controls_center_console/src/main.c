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

#include "log.h"

#include "config.h"

#include "button_led.h"
#include "button_led_fsm.h"
#include "button_led_radio.h"

static CanStorage s_can_storage = { 0 };
static GpioExpanderStorage s_expander;

#define CENTER_CONSOLE_CAN_OUTPUT_PERIOD_MILLIS 50u

typedef struct {
  // GPIO pin
  GpioAddress address;
  // CAN event to raise
  EECenterConsoleDigitalInput can_event;
  // Local event to raise
  CenterConsoleEventsButton local_event;
  // The reference number for the pin
  CenterConsoleButtonLed button;
} CenterConsoleInput;

static GpioExpanderPin s_expander_pin[NUM_CENTER_CONSOLE_BUTTON_LEDS] = {
  [CENTER_CONSOLE_BUTTON_LED_BPS] = GPIO_EXPANDER_PIN_0,
  [CENTER_CONSOLE_BUTTON_LED_PWR] = GPIO_EXPANDER_PIN_1,
  [CENTER_CONSOLE_BUTTON_LED_REVERSE] = GPIO_EXPANDER_PIN_2,
  [CENTER_CONSOLE_BUTTON_LED_NEUTRAL] = GPIO_EXPANDER_PIN_3,
  [CENTER_CONSOLE_BUTTON_LED_DRIVE] = GPIO_EXPANDER_PIN_4,
  [CENTER_CONSOLE_BUTTON_LED_DRL] = GPIO_EXPANDER_PIN_5,
  [CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS] = GPIO_EXPANDER_PIN_6,
  [CENTER_CONSOLE_BUTTON_LED_HAZARDS] = GPIO_EXPANDER_PIN_7,
};

// Toggle buttons are either on/off
static CenterConsoleInput s_toggle_button_input[] = {
  {
      .address = CENTER_CONSOLE_CONFIG_PIN_LOW_BEAM,           //
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM,   //
      .local_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM  //
  },
  {
      .address = CENTER_CONSOLE_CONFIG_PIN_HAZARDS,           //
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS,   //
      .local_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS  //
  },
  {
      .address = CENTER_CONSOLE_CONFIG_PIN_DRL,           //
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL,   //
      .local_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL  //
  },
  {
      .address = CENTER_CONSOLE_CONFIG_PIN_POWER,           //
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER,   //
      .local_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER  //
  }
};

static CenterConsoleInput s_radio_button_group[] = {
  {
      .address = CENTER_CONSOLE_CONFIG_PIN_DRIVE,               //
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE,       //
      .local_event = CENTER_CONSOLE_EVENT_BUTTON_DRIVE_PRESSED  //
  },
  {
      .address = CENTER_CONSOLE_CONFIG_PIN_NEUTRAL,               //
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL,       //
      .local_event = CENTER_CONSOLE_EVENT_BUTTON_NEUTRAL_PRESSED  //
  },
  {
      .address = CENTER_CONSOLE_CONFIG_PIN_REVERSE,               //
      .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE,       //
      .local_event = CENTER_CONSOLE_EVENT_BUTTON_REVERSE_PRESSED  //
  }
};

void prv_gpio_toggle_callback(const GpioAddress *address, void *context) {
  // Simple toggles just toggle state with a shared event,
  // CENTER_CONSOLE_EVENT_BUTTON_TOGGLE_STATE, with the data field as the
  // actual IO we are toggling
  CenterConsoleInput *toggle_button = context;
  event_raise(CENTER_CONSOLE_EVENT_BUTTON_TOGGLE_STATE, toggle_button->local_event);

  // Raise event via CAN message
  CAN_TRANSMIT_CENTER_CONSOLE_EVENT(toggle_button->can_event, 0);
}

void prv_gpio_radio_callback(const GpioAddress *address, void *context) {
  // Radio buttons switch state by using the event id and passing 0 data
  CenterConsoleInput *toggle_button = context;
  event_raise(toggle_button->local_event, 0);

  // Raise event via CAN message
  CAN_TRANSMIT_CENTER_CONSOLE_EVENT(toggle_button->can_event, 0);
}

void prv_adc_monitor(AdcChannel adc_channel, void *context) {
  uint16_t *rail_monitor_5v = context;
  adc_read_converted(adc_channel, rail_monitor_5v);

  LOG_DEBUG("adc\n");  // TODO: Any logic here to monitor 5V rail and raise appropriate event
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
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,  // TODO: Change
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CENTER_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTER_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTER_CONSOLE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  GpioSettings button_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(s_toggle_button_input); ++i) {
    gpio_init_pin(&s_toggle_button_input[i].address, &button_input_settings);

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,      //
      .priority = INTERRUPT_PRIORITY_NORMAL  //
    };
    // Initialize GPIO Interrupts to raise events to change LED status
    gpio_it_register_interrupt(&s_toggle_button_input[i].address, &interrupt_settings,
                               INTERRUPT_EDGE_RISING, prv_gpio_toggle_callback,
                               &s_toggle_button_input[i]);
  }
  for (size_t i = 0; i < SIZEOF_ARRAY(s_radio_button_group); ++i) {
    gpio_init_pin(&s_radio_button_group[i].address, &button_input_settings);

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,      //
      .priority = INTERRUPT_PRIORITY_NORMAL  //
    };
    // Initialize GPIO Interrupts to raise events to change LED status
    gpio_it_register_interrupt(&s_radio_button_group[i].address, &interrupt_settings,
                               INTERRUPT_EDGE_RISING, prv_gpio_radio_callback,
                               &s_radio_button_group[i]);
  }

  // Use I/O Expander for button LEDs
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,     //
    .sda = { GPIO_PORT_B, 11 },  //
    .scl = { GPIO_PORT_B, 10 },  //
  };
  i2c_init(I2C_PORT_2, &settings);
  // Configure the expander to be output only
  gpio_expander_init(&s_expander, I2C_PORT_2, GPIO_EXPANDER_ADDRESS_0, NULL);
  for (size_t i = 0; i < SIZEOF_ARRAY(s_expander_pin); ++i) {
    GpioSettings output_settings = {
      .direction = GPIO_DIR_OUT,  //
      .state = GPIO_STATE_HIGH,   //
    };
    gpio_expander_init_pin(&s_expander, s_expander_pin[i], &output_settings);
  }
  // Initialize normal toggle buttons
  button_led_init(&s_expander, s_expander_pin);
  // Initialize radio button groups
  ButtonLedRadioSettings radio_settings = {
    .reverse_pin = s_expander_pin[CENTER_CONSOLE_BUTTON_LED_REVERSE],
    .neutral_pin = s_expander_pin[CENTER_CONSOLE_BUTTON_LED_NEUTRAL],
    .drive_pin = s_expander_pin[CENTER_CONSOLE_BUTTON_LED_DRIVE]
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
  gpio_init_pin(&monitor_5v, &adc_input_settings);

  // TODO: Why doesn't continuous mode work smh
  // adc_init(ADC_MODE_CONTINUOUS);

  // uint16_t rail_monitor_5v = 0u;
  // adc_set_channel(ADC_CHANNEL_9, true);
  // adc_register_callback(ADC_CHANNEL_9, prv_adc_monitor, (void *)&rail_monitor_5v);

  GpioSettings enable_output_rail = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  // Enable 5V rail
  GpioAddress rail_5v = CENTER_CONSOLE_CONFIG_PIN_5V_ENABLE;
  gpio_init_pin(&rail_5v, &enable_output_rail);

  // Enable Driver Display
  GpioAddress display_rail = CENTER_CONSOLE_CONFIG_PIN_DISPLAY_ENABLE;
  gpio_init_pin(&display_rail, &enable_output_rail);

  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);

      button_led_process_event(&e);
      button_led_radio_process_event(&e);
    }
  }
}
