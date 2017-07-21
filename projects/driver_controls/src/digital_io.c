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
static DigitalIOData s_input_data[] = {
  [DRIVER_IO_POWER_SWITCH_PIN] = {
    .id = DIGITAL_IO_DEVICE_POWER_SWITCH,
    .event = INPUT_EVENT_POWER },

  [DRIVER_IO_DIRECTION_SELECTOR_PIN_FORWARD] = {
    .id = DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,
    .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE },

  [DRIVER_IO_DIRECTION_SELECTOR_PIN_REVERSE] = {
    .id = DIGITAL_IO_DEVICE_DIRECTION_SELECTOR,
    .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE },

  [DRIVER_IO_CRUISE_CONTROL_PIN] = {
    .id = DIGITAL_IO_DEVICE_CRUISE_CONTROL,
    .event = INPUT_EVENT_CRUISE_CONTROL },

  [DRIVER_IO_CRUISE_CONTROL_INC_PIN] = {
    .id = DIGITAL_IO_DEVICE_CRUISE_CONTROL_INC,
    .event = INPUT_EVENT_CRUISE_CONTROL_INC },

  [DRIVER_IO_CRUISE_CONTROL_DEC_PIN] = {
    .id = DIGITAL_IO_DEVICE_CRUISE_CONTROL_DEC,
    .event = INPUT_EVENT_CRUISE_CONTROL_DEC },

  [DRIVER_IO_TURN_SIGNAL_PIN_RIGHT] = {
    .id = DIGITAL_IO_DEVICE_TURN_SIGNAL,
    .event = INPUT_EVENT_TURN_SIGNAL_RIGHT },

  [DRIVER_IO_TURN_SIGNAL_PIN_LEFT] = {
    .id = DIGITAL_IO_DEVICE_TURN_SIGNAL,
    .event = INPUT_EVENT_TURN_SIGNAL_LEFT },

  [DRIVER_IO_HAZARD_LIGHT_PIN] = {
    .id = DIGITAL_IO_DEVICE_HAZARD_LIGHT,
    .event = INPUT_EVENT_HAZARD_LIGHT },
};

// Genarate the event based on the identity of the triggering device
static void prv_get_event(DigitalIOData *digital_io_data, Event *e, GPIOState state) {
  switch (digital_io_data->id) {
    case DIGITAL_IO_DEVICE_DIRECTION_SELECTOR:
      if (state != GPIO_STATE_LOW) {
        e->id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
        return;
      }
    case DIGITAL_IO_DEVICE_TURN_SIGNAL:
      if (state != GPIO_STATE_LOW) {
        e->id = INPUT_EVENT_TURN_SIGNAL_NONE;
        return;
      }
    case DIGITAL_IO_DEVICE_POWER_SWITCH:
    case DIGITAL_IO_DEVICE_CRUISE_CONTROL:
    case DIGITAL_IO_DEVICE_CRUISE_CONTROL_INC:
    case DIGITAL_IO_DEVICE_CRUISE_CONTROL_DEC:
    case DIGITAL_IO_DEVICE_HAZARD_LIGHT:
      e->id = digital_io_data->event;
      return;
  }
}

static void prv_input_callback(GPIOAddress *address, void *context) {
  GPIOState state;
  Event e;

  DigitalIOData *data = context;

  gpio_get_value(address, &state);
  prv_get_event(data, &e, state);
  event_raise(e.id, e.data);
}

static void prv_init_pin(DigitalIOSettings *settings, GPIOSettings *gpio_settings) {
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&settings->address, gpio_settings);
  gpio_it_register_interrupt(&settings->address, &it_settings, settings->edge,
                              prv_input_callback, settings->data);
}

// Configure driver devices with their individual settings
void digital_io_init() {
  DigitalIOSettings digital_inputs[] = {
    { .address = {
        DRIVER_IO_POWER_SWITCH_PORT,
        DRIVER_IO_POWER_SWITCH_PIN
      },
      .edge = INTERRUPT_EDGE_RISING },

    { .address = {
        DRIVER_IO_DIRECTION_SELECTOR_PORT,
        DRIVER_IO_DIRECTION_SELECTOR_PIN_FORWARD
      },
      .edge = INTERRUPT_EDGE_RISING_FALLING },

    { .address = {
        DRIVER_IO_DIRECTION_SELECTOR_PORT,
        DRIVER_IO_DIRECTION_SELECTOR_PIN_REVERSE
      },
      .edge = INTERRUPT_EDGE_RISING_FALLING },

    { .address = {
        DRIVER_IO_CRUISE_CONTROL_PORT,
        DRIVER_IO_CRUISE_CONTROL_PIN
      },
      .edge = INTERRUPT_EDGE_RISING },

    { .address = {
        DRIVER_IO_CRUISE_CONTROL_INC_PORT,
        DRIVER_IO_CRUISE_CONTROL_INC_PIN
      },
      .edge = INTERRUPT_EDGE_RISING },

    { .address = {
        DRIVER_IO_CRUISE_CONTROL_DEC_PORT,
        DRIVER_IO_CRUISE_CONTROL_DEC_PIN
      },
      .edge = INTERRUPT_EDGE_RISING },

    { .address = {
        DRIVER_IO_TURN_SIGNAL_PORT,
        DRIVER_IO_TURN_SIGNAL_PIN_RIGHT
      },
      .edge = INTERRUPT_EDGE_RISING_FALLING },

    { .address = {
        DRIVER_IO_TURN_SIGNAL_PORT,
        DRIVER_IO_TURN_SIGNAL_PIN_LEFT
      },
      .edge = INTERRUPT_EDGE_RISING_FALLING },

    { .address = {
        DRIVER_IO_HAZARD_LIGHT_PORT,
        DRIVER_IO_HAZARD_LIGHT_PIN
      },
      .edge = INTERRUPT_EDGE_RISING }
  };

  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(digital_inputs); i++) {
    uint8_t pin = digital_inputs[i].address.pin;

    digital_inputs[i].data = &s_input_data[pin];
    prv_init_pin(&digital_inputs[i], &settings);
  }
}
