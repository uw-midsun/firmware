#include <stddef.h>

#include "driver_io.h"
#include "gpio_it.h"
#include "input_interrupt.h"

static void prv_init_pin(DriverIODevice *driver_io) {
  GPIOSettings gpio_settings = { driver_io->direction, GPIO_STATE_LOW,
                                  GPIO_RES_NONE, driver_io->alt_function };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&driver_io->address, &gpio_settings);

  if (driver_io->callback != NULL) {
    gpio_it_register_interrupt(&driver_io->address, &it_settings, driver_io->edge,
                                driver_io->callback, driver_io->context);
  }
}

void driver_io_init() {
  gpio_init();
  interrupt_init();
  gpio_it_init();

  // Configure driver devices with their individual settings
  DriverIODevice inputs[] = {
    { .address = { GPIO_PORT_C, 0 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 1 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_ANALOG,
      .callback = input_callback },

    { .address = { GPIO_PORT_B, 2 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_B, 3 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 4 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 5 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 6 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 7 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 8 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 9 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 10 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback }
  };

  DriverIODevice outputs[] = {
    { .address = { GPIO_PORT_C, 11 },
      .direction = GPIO_DIR_OUT,
      .alt_function = GPIO_ALTFN_NONE }
  };

  for (uint8_t i = 0; i < sizeof(inputs)/sizeof(*inputs); i++) {
    prv_init_pin(&inputs[i]);
  }

  for (uint8_t i = 0; i < sizeof(outputs)/sizeof(*outputs); i++) {
    prv_init_pin(&outputs[i]);
  }

  // Initialize analog inputs
  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(ADC_CHANNEL_11, true);
  adc_register_callback(ADC_CHANNEL_11, pedal_callback, NULL);
}
