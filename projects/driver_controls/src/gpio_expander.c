#include "gpio_expander.h"

#include <stdbool.h>

#include "i2c.h"
#include "gpio_it.h"
#include "gpio_expander_config.h"

typedef struct GPIOExpanderInterrupt {
  GPIOExpanderCallback callback;
  void *context;
} GPIOExpanderInterrupt;

static GPIOAddress s_address;
static I2CPort s_i2c_port;

static GPIOExpanderInterrupt s_interrupts[NUM_GPIO_EXPANDER_PIN];

// Default register values (Datasheet Table 1-3)
static uint8_t s_register[NUM_GPIO_EXPANDER_REGISTERS] = {
  [GPIO_EXPANDER_IODIR]   = 0xff,
  [GPIO_EXPANDER_IPOL]    = 0x00,
  [GPIO_EXPANDER_GPINTEN] = 0x00,
  [GPIO_EXPANDER_DEFVAL]  = 0x00,
  [GPIO_EXPANDER_INTCON]  = 0x00,
  [GPIO_EXPANDER_IOCON]   = 0x00,
  [GPIO_EXPANDER_GPPU]    = 0x00,
  [GPIO_EXPANDER_INTF]    = 0x00,
  [GPIO_EXPANDER_INTCAP]  = 0x00,
  [GPIO_EXPANDER_GPIO]    = 0x00,
  [GPIO_EXPANDER_OLAT]    = 0x00
};

static StatusCode prv_pin_is_valid(GPIOExpanderPin pin) {
  if (pin >= NUM_GPIO_EXPANDER_PIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return STATUS_CODE_OK;
}

static void prv_update_registers() {
  // INTCAP and GPIO are the only registers that need to be read when updating, as they are the
  // only registers that can change without program intervention
  i2c_read_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_INTCAP,
                &s_register[GPIO_EXPANDER_INTCAP], 1);
  i2c_read_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPIO,
                &s_register[GPIO_EXPANDER_GPIO], 1);
}

static void prv_interrupt_handler(GPIOAddress *address, void *context) {
  // Update interrupt flag register.
  i2c_read_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_INTF,
                &s_register[GPIO_EXPANDER_INTF], 1);

  // Identify all pins with a pending interrupt and execute their callbacks
  GPIOExpanderPin current_pin;
  while (s_register[GPIO_EXPANDER_INTF] != 0) {
    current_pin = __builtin_ffs(s_register[GPIO_EXPANDER_INTF]) - 1;

    if (s_interrupts[current_pin].callback != NULL) {
      s_interrupts[current_pin].callback(current_pin, s_interrupts[current_pin].context);
    }

    s_register[GPIO_EXPANDER_INTF] &= ~(1 << current_pin);
  }

  // Clear the interrupt by reading the port registers
  prv_update_registers();
}

// Set the value of a specific bit in a register
static void prv_set_bit(uint8_t *data, GPIOExpanderPin pin, bool bit) {
  if (bit) {
    *data |= (1 << pin);
  } else {
    *data &= ~(1 << pin);
  }
}

StatusCode gpio_expander_init(GPIOAddress address, I2CPort i2c_port) {
  // Set the static variables
  s_address = address;
  s_i2c_port = i2c_port;

  // Configure the pin to receive interrupts from the MCP23008
  GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_FALLING,
                              prv_interrupt_handler, NULL);

  // Initialize the interrupt callbacks to NULL
  for (uint8_t i = 0; i < NUM_GPIO_EXPANDER_PIN; i++) {
    s_interrupts[i].callback = NULL;
  }

  // Use Sequential write to set each register to the default value
  i2c_write_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_IODIR,
                &s_register[GPIO_EXPANDER_IODIR], 11);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_init_pin(GPIOExpanderPin pin, GPIOSettings *settings) {
  StatusCode valid = prv_pin_is_valid(pin);
  status_ok_or_return(valid);

  if (pin >= NUM_GPIO_EXPANDER_PIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the direction of the data I/O
  prv_set_bit(&s_register[GPIO_EXPANDER_IODIR], pin, (settings->direction == GPIO_DIR_IN));
  i2c_write_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_IODIR,
                &s_register[GPIO_EXPANDER_IODIR], 1);

  if (settings->direction == GPIO_DIR_OUT) {
    // Set the output state if the pin is an output
    prv_set_bit(&s_register[GPIO_EXPANDER_GPIO], pin, (settings->state == GPIO_STATE_HIGH));
    i2c_write_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPIO,
                  &s_register[GPIO_EXPANDER_GPIO], 1);
  } else {
    // Enable interrupts if the pin is an input
    prv_set_bit(&s_register[GPIO_EXPANDER_GPINTEN], pin, true);
    i2c_write_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPINTEN,
                  &s_register[GPIO_EXPANDER_GPINTEN], 1);

    // Configure so that any change in value triggers an interrupt
    prv_set_bit(&s_register[GPIO_EXPANDER_INTCON], pin, false);
    i2c_write_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_INTCON,
                  &s_register[GPIO_EXPANDER_INTCON], 1);
  }

  return STATUS_CODE_OK;
}

bool gpio_expander_get_state(GPIOExpanderPin pin) {
  StatusCode valid = prv_pin_is_valid(pin);
  status_ok_or_return(valid);

  // Update our GPIO register value and read the pin state
  prv_update_registers();
  bool state = (s_register[GPIO_EXPANDER_GPIO] >> pin) & 1;

  return state;
}

StatusCode gpio_expander_set_state(GPIOExpanderPin pin, bool new_state) {
  StatusCode valid = prv_pin_is_valid(pin);
  status_ok_or_return(valid);

  // Return if the pin is configured as an input
  uint8_t io_dir = s_register[GPIO_EXPANDER_IODIR];
  if ((io_dir >> pin) & 1) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the output latch to the new logical state
  prv_set_bit(&s_register[GPIO_EXPANDER_OLAT], pin, new_state);
  i2c_write_reg(s_i2c_port, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_OLAT,
                &s_register[GPIO_EXPANDER_OLAT], 1);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_register_callback(GPIOExpanderPin pin, GPIOExpanderCallback callback,
                                            void *context) {
  StatusCode valid = prv_pin_is_valid(pin);
  status_ok_or_return(valid);

  s_interrupts[pin].callback = callback;
  s_interrupts[pin].context = context;

  return STATUS_CODE_OK;
}
