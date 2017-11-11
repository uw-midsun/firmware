#include <stddef.h>
#include <stdio.h>

#include "center_console_io.h"
#include "debounce.h"
#include "driver_io.h"
#include "gpio_expander.h"
#include "gpio_it.h"
#include "input_event.h"

// Digital device identifiers
typedef enum {
  CENTER_CONSOLE_IO_DEVICE_POWER_SWITCH = 0,
  CENTER_CONSOLE_IO_DEVICE_DIRECTION_SELECTOR,
  CENTER_CONSOLE_IO_DEVICE_HEADLIGHT,
  CENTER_CONSOLE_IO_DEVICE_HAZARD_LIGHT,
  CENTER_CONSOLE_IO_DEVICE_BRAKING_REGEN_INC,
  CENTER_CONSOLE_IO_DEVICE_BRAKING_REGEN_DEC
} CenterConsoleIODevice;

// Store the id of the device as well as the id of the event the device raises
typedef struct CenterConsoleIOData {
  CenterConsoleIODevice id;
  InputEvent event;
} CenterConsoleIOData;

typedef struct CenterConsoleIOSettings {
  GPIOAddress address;
  InterruptEdge edge;
} CenterConsoleIOSettings;

// Index the objects using their respective pins
static CenterConsoleIOData s_center_console_data[DRIVER_IO_NUM_ADDRESSES];

static void prv_center_console_callback(const GPIOAddress *address, void *context) {
  CenterConsoleIOData *data = (CenterConsoleIOData *)context;

  GPIOState state = { 0 };
  gpio_get_state(address, &state);
  debounce(*address);

  // Check the devices with multiple pins to ensure the correct event
  uint16_t event_id = data->event;

  if (data->id == CENTER_CONSOLE_IO_DEVICE_DIRECTION_SELECTOR && state == GPIO_STATE_HIGH) {
    event_id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  }

  event_raise(event_id, 0);
}

// Configure driver devices with their individual settings
void center_console_io_init(void) {
  // Initialize the static array with device information
  s_center_console_data[DRIVER_IO_POWER_SWITCH_PIN] =
      (CenterConsoleIOData){ .id = CENTER_CONSOLE_IO_DEVICE_POWER_SWITCH,
                             .event = INPUT_EVENT_POWER };

  s_center_console_data[DRIVER_IO_DIR_SELECT_PIN_FORWARD] =
      (CenterConsoleIOData){ .id = CENTER_CONSOLE_IO_DEVICE_DIRECTION_SELECTOR,
                             .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE };

  s_center_console_data[DRIVER_IO_DIR_SELECT_PIN_REVERSE] =
      (CenterConsoleIOData){ .id = CENTER_CONSOLE_IO_DEVICE_DIRECTION_SELECTOR,
                             .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE };

  s_center_console_data[DRIVER_IO_HEADLIGHT_PIN] =
      (CenterConsoleIOData){ .id = CENTER_CONSOLE_IO_DEVICE_HEADLIGHT,
                             .event = INPUT_EVENT_HEADLIGHT };

  s_center_console_data[DRIVER_IO_HAZARD_LIGHT_PIN] =
      (CenterConsoleIOData){ .id = CENTER_CONSOLE_IO_DEVICE_HAZARD_LIGHT,
                             .event = INPUT_EVENT_HAZARD_LIGHT };

  s_center_console_data[DRIVER_IO_BRAKING_REGEN_INC_PIN] =
      (CenterConsoleIOData){ .id = CENTER_CONSOLE_IO_DEVICE_BRAKING_REGEN_INC,
                             .event = INPUT_EVENT_BRAKING_REGEN_INC };

  s_center_console_data[DRIVER_IO_BRAKING_REGEN_DEC_PIN] =
      (CenterConsoleIOData){ .id = CENTER_CONSOLE_IO_DEVICE_BRAKING_REGEN_DEC,
                             .event = INPUT_EVENT_BRAKING_REGEN_DEC };

  // Define array to store configuration settings for each pin
  CenterConsoleIOSettings console_inputs[] = {
    { .address = DRIVER_IO_POWER_SWITCH, .edge = INTERRUPT_EDGE_RISING },
    { .address = DRIVER_IO_DIR_SELECT_FORWARD, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = DRIVER_IO_DIR_SELECT_REVERSE, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = DRIVER_IO_HEADLIGHT, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = DRIVER_IO_HAZARD_LIGHT, .edge = INTERRUPT_EDGE_RISING },
    { .address = DRIVER_IO_BRAKING_REGEN_INC, .edge = INTERRUPT_EDGE_RISING },
    { .address = DRIVER_IO_BRAKING_REGEN_DEC, .edge = INTERRUPT_EDGE_RISING }
  };

  GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN, .state = GPIO_STATE_LOW };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  // Initialize Center Console Inputs
  for (uint8_t i = 0; i < SIZEOF_ARRAY(console_inputs); i++) {
    uint8_t pin = console_inputs[i].address.pin;

    gpio_init_pin(&console_inputs[i].address, &gpio_settings);
    gpio_it_register_interrupt(&console_inputs[i].address, &it_settings, console_inputs[i].edge,
                               prv_center_console_callback, &s_center_console_data[pin]);
  }
}
