#include <stddef.h>

#include "gpio_it.h"
#include "driver_io.h"
#include "driver_callback.h"

// Arbitrary thresholds for gas pedal
#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

typedef struct DriverIOSettings {
  GPIOAddress address;
  GPIODir direction;
  GPIOAltFn alt_function;
  InterruptEdge edge;
  DriverIOCallback callback;
  DriverIOData data;
} DriverIOSettings;

static void prv_init_pin(DriverIOSettings *settings) {
  GPIOSettings gpio_settings = { settings->direction, GPIO_STATE_LOW,
                                  GPIO_RES_NONE, settings->alt_function };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&settings->address, &gpio_settings);

  if (settings->callback != NULL) {
    gpio_it_register_interrupt(&settings->address, &it_settings, settings->edge,
                                settings->callback, &settings->data);
  }
}

void driver_io_init() {
  gpio_init();
  interrupt_init();
  gpio_it_init();

  // Configure driver devices with their individual settings
  DriverIOSettings digital_inputs[] = {
    { .address = { GPIO_PORT_C, 0 }, .edge = INTERRUPT_EDGE_RISING,},
    { .address = { GPIO_PORT_B, 2 }, .edge = INTERRUPT_EDGE_RISING_FALLING, },
    { .address = { GPIO_PORT_B, 3 }, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = { GPIO_PORT_C, 4 }, .edge = INTERRUPT_EDGE_RISING },
    { .address = { GPIO_PORT_C, 5 }, .edge = INTERRUPT_EDGE_RISING },
    { .address = { GPIO_PORT_C, 6 }, .edge = INTERRUPT_EDGE_RISING },
    { .address = { GPIO_PORT_C, 7 }, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = { GPIO_PORT_C, 8 }, .edge = INTERRUPT_EDGE_RISING_FALLING },
    { .address = { GPIO_PORT_C, 9 }, .edge = INTERRUPT_EDGE_RISING },
    { .address = { GPIO_PORT_C, 10 }, .edge = INTERRUPT_EDGE_RISING_FALLING },
  };

  DriverIOData data[] = {
    { .id = DRIVER_IO_POWER_SWITCH, .event = INPUT_EVENT_POWER },
    { .id = DRIVER_IO_DIRECTION_SELECTOR, .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE },
    { .id = DRIVER_IO_DIRECTION_SELECTOR, .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE },
    { .id = DRIVER_IO_CRUISE_CONTROL, .event = INPUT_EVENT_CRUISE_CONTROL },
    { .id = DRIVER_IO_CRUISE_CONTROL_INC, .event = INPUT_EVENT_CRUISE_CONTROL_INC },
    { .id = DRIVER_IO_CRUISE_CONTROL_DEC, .event = INPUT_EVENT_CRUISE_CONTROL_DEC },
    { .id = DRIVER_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_LEFT },
    { .id = DRIVER_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_RIGHT },
    { .id = DRIVER_IO_HAZARD_LIGHT, .event = INPUT_EVENT_HAZARD_LIGHT },
    { .id = DRIVER_IO_MECHANICAL_BRAKE, .event = INPUT_EVENT_MECHANICAL_BRAKE }
  };

  for (uint8_t i = 0; i < sizeof(digital_inputs)/sizeof(*digital_inputs); i++) {
    digital_inputs[i].direction = GPIO_DIR_IN;
    digital_inputs[i].alt_function = GPIO_ALTFN_NONE;
    digital_inputs[i].callback = driver_callback_input;
    digital_inputs[i].data = data[i];
    prv_init_pin(&digital_inputs[i]);
  }

  DriverIOSettings analog_inputs[] = {
    { .address = { GPIO_PORT_C, 1 }, .edge = INTERRUPT_EDGE_RISING_FALLING }
  };

  for (uint8_t i = 0; i < sizeof(analog_inputs)/sizeof(*analog_inputs); i++) {
    analog_inputs[i].direction = GPIO_DIR_IN;
    analog_inputs[i].alt_function = GPIO_ALTFN_ANALOG;
    analog_inputs[i].callback = NULL;
    prv_init_pin(&analog_inputs[i]);
  }

  // Initialize analog inputs
  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(ADC_CHANNEL_11, true);
  adc_register_callback(ADC_CHANNEL_11, driver_callback_pedal, NULL);
}
