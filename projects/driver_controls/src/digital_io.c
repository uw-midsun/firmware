#include <stddef.h>

#include "digital_io.h"
#include "driver_io.h"
#include "gpio_it.h"
#include "input_event.h"

// Digital device identifiers
typedef enum {
  DIGITAL_IO_DEVICE_POWER_SWITCH = 0,
  DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,
  DIGITAL_IO_DEVICE_CRUISE_CONTROL,
  DIGITAL_IO_DEVICE_CRUISE_CONTROL_INC,
  DIGITAL_IO_DEVICE_CRUISE_CONTROL_DEC,
  DIGITAL_IO_DEVICE_TURN_SIGNAL,
  DIGITAL_IO_DEVICE_HAZARD_LIGHT,
  DIGITAL_IO_DEVICE_HORN,
  DIGITAL_IO_DEVICE_PUSH_TO_TALK,
  NUM_DIGITAL_IO_DEVICE
} DigitalIODevice;

// Store the id of the device as well as the id of the event the device raises
typedef struct DigitalIOData {
  DigitalIODevice id;
  InputEvent event;
} DigitalIOData;

// Configuration settings for the digital pins
typedef struct DigitalIOSettings {
  GPIOAddress address;
  InterruptEdge edge;
  DigitalIOData *data;
} DigitalIOSettings;

// Index the objects using their respective pins
static DigitalIOData s_input_data[DRIVER_IO_NUM_ADDRESSES];

// Genarate the event based on the identity of the triggering device
static uint16_t prv_get_event(DigitalIOData *digital_io_data, GPIOState state) {
  uint16_t event_id = digital_io_data->event;

  if (event_id == DIGITAL_IO_DEVICE_DIRECTION_SELECTOR && state == GPIO_STATE_HIGH) {
    event_id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  } else if (event_id == DIGITAL_IO_DEVICE_TURN_SIGNAL && state == GPIO_STATE_HIGH) {
    event_id = INPUT_EVENT_TURN_SIGNAL_NONE;
  }

  return event_id;
}

static void prv_input_callback(const GPIOAddress *address, void *context) {
  DigitalIOData *data = context;

  GPIOState state = { 0 };
  gpio_get_state(address, &state);
  event_raise(prv_get_event(data, state), 0);
}

static void prv_init_pin(DigitalIOSettings *settings, GPIOSettings *gpio_settings) {
  InterruptSettings it_settings = {
    INTERRUPT_TYPE_INTERRUPT,  //
    INTERRUPT_PRIORITY_LOW,    //
  };

  gpio_init_pin(&settings->address, gpio_settings);
  gpio_it_register_interrupt(&settings->address, &it_settings, settings->edge, prv_input_callback,
                             settings->data);
}

// Configure driver devices with their individual settings
void digital_io_init(void) {
  // Initialize the static array with device information
  s_input_data[DRIVER_IO_POWER_SWITCH_PIN] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_POWER_SWITCH,  //
    .event = INPUT_EVENT_POWER,            //
  };

  s_input_data[DRIVER_IO_DIR_SELECT_PIN_FORWARD] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,     //
    .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,  //
  };

  s_input_data[DRIVER_IO_DIR_SELECT_PIN_REVERSE] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,       //
    .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE,  //
  };

  s_input_data[DRIVER_IO_CRUISE_CONTROL_PIN] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_CRUISE_CONTROL,  //
    .event = INPUT_EVENT_CRUISE_CONTROL,     //
  };

  s_input_data[DRIVER_IO_CRUISE_CONTROL_INC_PIN] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_CRUISE_CONTROL_INC,  //
    .event = INPUT_EVENT_CRUISE_CONTROL_INC,     //
  };

  s_input_data[DRIVER_IO_CRUISE_CONTROL_DEC_PIN] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_CRUISE_CONTROL_DEC,  //
    .event = INPUT_EVENT_CRUISE_CONTROL_DEC,     //
  };

  s_input_data[DRIVER_IO_TURN_SIGNAL_PIN_RIGHT] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_TURN_SIGNAL,     //
    .event = INPUT_EVENT_TURN_SIGNAL_RIGHT,  //
  };

  s_input_data[DRIVER_IO_TURN_SIGNAL_PIN_LEFT] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_TURN_SIGNAL,    //
    .event = INPUT_EVENT_TURN_SIGNAL_LEFT,  //
  };

  s_input_data[DRIVER_IO_HAZARD_LIGHT_PIN] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_HAZARD_LIGHT,  //
    .event = INPUT_EVENT_HAZARD_LIGHT,     //
  };

  s_input_data[DRIVER_IO_HORN_PIN] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_HORN,  //
    .event = INPUT_EVENT_HORN,     //
  };

  s_input_data[DRIVER_IO_PUSH_TO_TALK_PIN] = (DigitalIOData){
    .id = DIGITAL_IO_DEVICE_PUSH_TO_TALK,  //
    .event = INPUT_EVENT_PUSH_TO_TALK,     //
  };

  // Array to store configuration settings for each pin
  DigitalIOSettings digital_inputs[] = {
    {.address = DRIVER_IO_POWER_SWITCH, .edge = INTERRUPT_EDGE_RISING },
    {.address = DRIVER_IO_DIR_SELECT_FORWARD, .edge = INTERRUPT_EDGE_RISING_FALLING },
    {.address = DRIVER_IO_DIR_SELECT_REVERSE, .edge = INTERRUPT_EDGE_RISING_FALLING },
    {.address = DRIVER_IO_CRUISE_CONTROL_PORT, .edge = INTERRUPT_EDGE_RISING },
    {.address = DRIVER_IO_CRUISE_CONTROL_INC, .edge = INTERRUPT_EDGE_RISING },
    {.address = DRIVER_IO_CRUISE_CONTROL_DEC, .edge = INTERRUPT_EDGE_RISING },
    {.address = DRIVER_IO_TURN_SIGNAL_RIGHT, .edge = INTERRUPT_EDGE_RISING_FALLING },
    {.address = DRIVER_IO_TURN_SIGNAL_LEFT, .edge = INTERRUPT_EDGE_RISING_FALLING },
    {.address = DRIVER_IO_HAZARD_LIGHT, .edge = INTERRUPT_EDGE_RISING },
    {.address = DRIVER_IO_HORN, .edge = INTERRUPT_EDGE_RISING_FALLING },
    {.address = DRIVER_IO_PUSH_TO_TALK, .edge = INTERRUPT_EDGE_RISING_FALLING }
  };

  GPIOSettings settings = {
    .direction = GPIO_DIR_IN,  //
    .state = GPIO_STATE_LOW,   //
  };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(digital_inputs); i++) {
    uint8_t pin = digital_inputs[i].address.pin;

    digital_inputs[i].data = &s_input_data[pin];
    prv_init_pin(&digital_inputs[i], &settings);
  }
}
