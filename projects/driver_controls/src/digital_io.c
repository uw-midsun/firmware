#include <stddef.h>
#include <stdio.h>

#include "digital_io.h"
#include "driver_io.h"
#include "gpio_it.h"
#include "input_event.h"
#include "gpio_expander.h"

// Digital device identifiers
typedef enum {
  DIGITAL_IO_DEVICE_POWER_SWITCH = 0,
  DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,
  DIGITAL_IO_DEVICE_HEADLIGHT,
  DIGITAL_IO_DEVICE_HAZARD_LIGHT,
  DIGITAL_IO_BRAKING_REGEN_INC,
  DIGITAL_IO_BRAKING_REGEN_DEC,
  DIGITAL_IO_DEVICE_CRUISE_CONTROL,
  DIGITAL_IO_DEVICE_CRUISE_CONTROL_INC,
  DIGITAL_IO_DEVICE_CRUISE_CONTROL_DEC,
  DIGITAL_IO_DEVICE_TURN_SIGNAL,
  DIGITAL_IO_DEVICE_HORN,
  DIGITAL_IO_DEVICE_PUSH_TO_TALK,
  NUM_DIGITAL_IO_DEVICE
} DigitalIODevice;

// Store the id of the device as well as the id of the event the device raises
typedef struct DigitalIOData {
  DigitalIODevice id;
  InputEvent event;
} DigitalIOData;

// Index the objects using their respective pins
static DigitalIOData s_center_console_data[DRIVER_IO_NUM_ADDRESSES];
static DigitalIOData s_steering_input_data[NUM_GPIO_EXPANDER_PINS];

// Genarate the event based on the identity of the triggering device
static uint16_t prv_get_event(DigitalIOData data, GPIOState state) {
  uint16_t event_id = data.event;

  if (data.id == DIGITAL_IO_DEVICE_DIRECTION_SELECTOR && state == GPIO_STATE_HIGH) {
    event_id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  } else if (data.id == DIGITAL_IO_DEVICE_TURN_SIGNAL && state == GPIO_STATE_HIGH) {
    event_id = INPUT_EVENT_TURN_SIGNAL_NONE;
  }

  return event_id;
}

static void prv_input_callback(const GPIOAddress *address, void *context) {
  DigitalIOData *data = (DigitalIOData *)context;

  GPIOState state = { 0 };
  gpio_get_state(address, &state);

  event_raise(prv_get_event(*data, state), 0);
}

static void prv_steering_wheel_callback(GPIOExpanderPin pin, GPIOState state, void *context) {
  InputEvent event_id = 0;

  // Turn into lookup table
  switch (pin) {
    case DRIVER_IO_RIGHT_TURN_SIGNAL_PIN:
      event_id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
      break;
    case DRIVER_IO_LEFT_TURN_SIGNAL_PIN:
      event_id = INPUT_EVENT_TURN_SIGNAL_LEFT;
      break;
    case DRIVER_IO_CRUISE_CONTROL_PIN:
      event_id = INPUT_EVENT_CRUISE_CONTROL;
      break;
    case DRIVER_IO_CRUISE_CONTROL_INC_PIN:
      event_id = INPUT_EVENT_CRUISE_CONTROL_INC;
      break;
    case DRIVER_IO_CRUISE_CONTROL_DEC_PIN:
      event_id = INPUT_EVENT_CRUISE_CONTROL_DEC;
      break;
    case DRIVER_IO_PUSH_TO_TALK_PIN:
      event_id = INPUT_EVENT_PUSH_TO_TALK;
      break;
    case DRIVER_IO_BMS_FAULT_PIN:
      event_id = INPUT_EVENT_BMS_FAULT;
      break;
    case DRIVER_IO_HORN_PIN:
      event_id = INPUT_EVENT_HORN;
      break;
    default:
      break;
  }

  event_raise(event_id, 0);
}

// Configure driver devices with their individual settings
void digital_io_init(void) {
  // Initialize the static array with device information
  s_center_console_data[DRIVER_IO_POWER_SWITCH_PIN] = (DigitalIOData){
        .id = DIGITAL_IO_DEVICE_POWER_SWITCH,
        .event = INPUT_EVENT_POWER
      };

  s_center_console_data[DRIVER_IO_DIR_SELECT_PIN_FORWARD] = (DigitalIOData){
        .id = DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,
        .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE
      };

  s_center_console_data[DRIVER_IO_DIR_SELECT_PIN_REVERSE] = (DigitalIOData){
        .id = DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,
        .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE
      };

  s_center_console_data[DRIVER_IO_HEADLIGHT_PIN] = (DigitalIOData){
        .id = DIGITAL_IO_DEVICE_HEADLIGHT,
        .event = INPUT_EVENT_HEADLIGHT
      };

  s_center_console_data[DRIVER_IO_HAZARD_LIGHT_PIN] = (DigitalIOData){
        .id = DIGITAL_IO_DEVICE_HAZARD_LIGHT,
        .event = INPUT_EVENT_HAZARD_LIGHT
      };

  s_center_console_data[DRIVER_IO_BRAKING_REGEN_INC_PIN] = (DigitalIOData){
        .id = DIGITAL_IO_BRAKING_REGEN_INC,
        .event = INPUT_EVENT_BRAKING_REGEN_INC
      };

  s_center_console_data[DRIVER_IO_BRAKING_REGEN_DEC_PIN] = (DigitalIOData){
        .id = DIGITAL_IO_BRAKING_REGEN_DEC,
        .event = INPUT_EVENT_BRAKING_REGEN_DEC
      };

  // Define array to store configuration settings for each pin
  struct {
    GPIOAddress address;
    InterruptEdge edge;
  } console_inputs[] = {
    { .address = DRIVER_IO_POWER_SWITCH, .edge = INTERRUPT_EDGE_RISING },
    { .address = DRIVER_IO_DIR_SELECT_FORWARD, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = DRIVER_IO_DIR_SELECT_REVERSE, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = DRIVER_IO_HEADLIGHT, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = DRIVER_IO_HAZARD_LIGHT, .edge = INTERRUPT_EDGE_RISING },
    { .address = DRIVER_IO_BRAKING_REGEN_INC, .edge = INTERRUPT_EDGE_RISING },
    { .address = DRIVER_IO_BRAKING_REGEN_DEC, .edge = INTERRUPT_EDGE_RISING }
  };

  GPIOSettings gpio_settings = {.direction = GPIO_DIR_IN, .state = GPIO_STATE_LOW };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  // Initialize Center Console Inputs
  for (uint8_t i = 0; i < SIZEOF_ARRAY(console_inputs); i++) {
    uint8_t pin = console_inputs[i].address.pin;

    gpio_init_pin(&console_inputs[i].address, &gpio_settings);
    gpio_it_register_interrupt(&console_inputs[i].address, &it_settings, console_inputs[i].edge,
                               prv_input_callback, &s_center_console_data[pin]);
  }

  // Initialize Steering Wheel inputs
  for (uint8_t i = 0; i < NUM_GPIO_EXPANDER_PINS; i++) {
    gpio_expander_init_pin(i, &gpio_settings);
    gpio_expander_register_callback(i, prv_steering_wheel_callback, NULL);
  }
}
