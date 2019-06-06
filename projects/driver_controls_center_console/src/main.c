#include <stdbool.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#include "adc.h"
#include "center_console_event.h"
#include "exported_enums.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

#include "log.h"

#include "config.h"

#include "center_console.h"

#include "bms_heartbeat.h"
#include "button_led.h"
#include "button_led_fsm.h"
#include "button_led_radio.h"
#include "center_console_flags.h"
#include "exported_enums.h"

static CanStorage s_can_storage = { 0 };
static GpioExpanderStorage s_expander;

#define CENTER_CONSOLE_5V_RAIL_MONITOR_PERIOD_MILLIS 100

static void prv_adc_monitor(SoftTimerId timer_id, void *context) {
  uint16_t *rail_monitor_5v = context;

  GpioAddress monitor_5v = CENTER_CONSOLE_CONFIG_PIN_5V_MONITOR;
  AdcChannel adc_channel_5v_monitor = NUM_ADC_CHANNELS;
  adc_get_channel(monitor_5v, &adc_channel_5v_monitor);

  adc_read_converted(adc_channel_5v_monitor, rail_monitor_5v);

  // TODO: Any logic here to monitor 5V rail and raise appropriate event?

  soft_timer_start_millis(CENTER_CONSOLE_5V_RAIL_MONITOR_PERIOD_MILLIS, prv_adc_monitor, context,
                          NULL);
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
    .tx = CENTER_CONSOLE_CONFIG_PIN_CAN_TX,
    .rx = CENTER_CONSOLE_CONFIG_PIN_CAN_RX,
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  // Use I/O Expander for button LEDs
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,                   //
    .sda = CENTER_CONSOLE_CONFIG_PIN_I2C_SDA,  //
    .scl = CENTER_CONSOLE_CONFIG_PIN_I2C_SDL,  //
  };
  status_ok_or_return(i2c_init(I2C_PORT_2, &settings));
  // Configure the expander to be output only
  status_ok_or_return(gpio_expander_init(&s_expander, I2C_PORT_2, GPIO_EXPANDER_ADDRESS_0, NULL));

  // Initialize normal toggle buttons
  ButtonLedGpioExpanderPins expander_pins = {
    .bps_indicator = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_BPS,
    .power_indicator = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_POWER,
    .lights_drl = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRL,
    .lights_low_beams = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_LOW_BEAMS,
    .lights_hazards = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_HAZARDS,
  };
  status_ok_or_return(button_led_init(&s_expander, &expander_pins));
  // Initialize radio button groups
  ButtonLedRadioSettings radio_settings = {
    .reverse_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_REVERSE,
    .neutral_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_NEUTRAL,
    .drive_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRIVE,
  };
  status_ok_or_return(button_led_radio_init(&s_expander, &radio_settings));

  CenterConsoleStorage cc_storage = {
    .momentary_switch_lights_low_beam =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_LOW_BEAM,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM,
        },
    .momentary_switch_lights_drl =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_DRL,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL,
        },
    .toggle_switch_lights_hazards =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_HAZARDS,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS,
        },
    .radio_button_drive =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_DRIVE,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE,
        },
    .radio_button_neutral =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_NEUTRAL,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL,
        },
    .radio_button_reverse =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_REVERSE,
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE,
        },
    .toggle_switch_power =
        {
            .pin_address = CENTER_CONSOLE_CONFIG_PIN_POWER,      //
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER,  //
        },
  };
  status_ok_or_return(center_console_init(&cc_storage));

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

  adc_init(ADC_MODE_SINGLE);

  uint16_t rail_monitor_5v = 0u;
  status_ok_or_return(adc_get_channel(monitor_5v, &adc_channel_5v_monitor));
  status_ok_or_return(adc_set_channel(adc_channel_5v_monitor, true));
  status_ok_or_return(soft_timer_start_millis(CENTER_CONSOLE_5V_RAIL_MONITOR_PERIOD_MILLIS,
                                              prv_adc_monitor, (void *)&rail_monitor_5v, NULL));

  const GpioSettings enable_output_rail = {
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

  Event e = { 0 };

  // Start BMS heartbeat handler
  bms_heartbeat_init();
  while (true) {
    center_console_poll(&cc_storage);

    while (status_ok(event_process(&e))) {
      can_process_event(&e);

      button_led_process_event(&e);
      button_led_radio_process_event(&e);
    }
  }
}
