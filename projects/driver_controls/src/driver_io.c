#include <stddef.h>

#include "adc.h"
#include "gpio_it.h"
#include "driver_io.h"
#include "status.h"

// Arbitrary thresholds for gas pedal
#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

typedef struct DriverIOSettings {
  GPIOAddress address;
  InterruptEdge edge;
  DriverIOData data;
} DriverIOSettings;

static void prv_init_pin(DriverIOSettings *settings, GPIOSettings *gpio_settings) {
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&settings->address, gpio_settings);
  gpio_it_register_interrupt(&settings->address, &it_settings, settings->edge,
                              NULL, &settings->data);
}

void driver_io_init() {
  // Configure driver devices with their individual settings
  DriverIOSettings digital_inputs[] = {
    { .address = { GPIO_PORT_C, 0 }, .edge = INTERRUPT_EDGE_RISING,
      .data = { 
        .id = DRIVER_IO_POWER_SWITCH, 
        .event = INPUT_EVENT_POWER
      }
    },

    { .address = { GPIO_PORT_B, 2 }, .edge = INTERRUPT_EDGE_RISING_FALLING,
      .data = {
        .id = DRIVER_IO_DIRECTION_SELECTOR,
        .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE
      } 
    },

    { .address = { GPIO_PORT_B, 3 }, .edge = INTERRUPT_EDGE_RISING_FALLING
      .data = {
        .id = DRIVER_IO_DIRECTION_SELECTOR,
        .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE
      }
    },

    { .address = { GPIO_PORT_C, 4 }, .edge = INTERRUPT_EDGE_RISING
      .data = {
        .id = DRIVER_IO_CRUISE_CONTROL,
        .event = INPUT_EVENT_CRUISE_CONTROL
      }
    },

    { .address = { GPIO_PORT_C, 5 }, .edge = INTERRUPT_EDGE_RISING
      .data = {
        .id = DRIVER_IO_CRUISE_CONTROL_INC,
        .event = INPUT_EVENT_CRUISE_CONTROL_INC
      }
    },

    { .address = { GPIO_PORT_C, 6 }, .edge = INTERRUPT_EDGE_RISING
      .data = {
        .id = DRIVER_IO_CRUISE_CONTROL_DEC,
        .event = INPUT_EVENT_CRUISE_CONTROL_DEC
      }
    },

    { .address = { GPIO_PORT_C, 7 }, .edge = INTERRUPT_EDGE_RISING_FALLING
      .data = {
        .id = DRIVER_IO_TURN_SIGNAL,
        .event = INPUT_EVENT_TURN_SIGNAL_LEFT
      }
    },

    { .address = { GPIO_PORT_C, 8 }, .edge = INTERRUPT_EDGE_RISING_FALLING
      .data = {
        .id = DRIVER_IO_TURN_SIGNAL,
        .event = INPUT_EVENT_TURN_SIGNAL_RIGHT
      }
    },

    { .address = { GPIO_PORT_C, 9 }, .edge = INTERRUPT_EDGE_RISING
      .data = {
        .id = DRIVER_IO_HAZARD_LIGHT,
        .event = INPUT_EVENT_HAZARD_LIGHT
      }
    },

    { .address = { GPIO_PORT_C, 10 }, .edge = INTERRUPT_EDGE_RISING_FALLING
      .data = {
        .id = DRIVER_IO_MECHANICAL_BRAKE,
        .event = INPUT_EVENT_MECHANICAL_BRAKE
      }
    },
  };

  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(digital_inputs); i++) {
    prv_init_pin(&digital_inputs[i]);
  }
}
