#include "gpio_expander.h"
#include <stdbool.h>
#include <string.h>
#include "gpio_it.h"
#include "i2c.h"
#include "mcp23008.h"

// IO interrupt occurred
static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
  GpioExpanderStorage *expander = context;
  uint8_t intf = 0;
  uint8_t gpio = 0;

  // Read the contents of the interrupt flag and GPIO registers - INTCAP may have missed a change
  i2c_read_reg(expander->port, expander->addr, MCP23008_INTF, &intf, 1);
  i2c_read_reg(expander->port, expander->addr, MCP23008_GPIO, &gpio, 1);

  // Identify all pins with a pending interrupt and execute their callbacks
  while (intf != 0) {
    GpioExpanderPin current_pin = (GpioExpanderPin)(__builtin_ffs(intf) - 1);

    if (expander->callbacks[current_pin].func != NULL) {
      expander->callbacks[current_pin].func(current_pin, (gpio >> current_pin) & 1,
                                            expander->callbacks[current_pin].context);
    }

    intf &= ~(1 << current_pin);
  }
}

// Set a specific bit in a given register
static void prv_set_bit(GpioExpanderStorage *expander, uint8_t reg, GpioExpanderPin pin, bool bit) {
  uint8_t data = 0;
  i2c_read_reg(expander->port, expander->addr, reg, &data, 1);
  if (bit) {
    data |= (1 << pin);
  } else {
    data &= ~(1 << pin);
  }
  i2c_write_reg(expander->port, expander->addr, reg, &data, 1);
}

StatusCode gpio_expander_init(GpioExpanderStorage *expander, I2CPort port, GpioExpanderAddress addr,
                              const GPIOAddress *interrupt_pin) {
  if (addr >= NUM_GPIO_EXPANDER_ADDRESSES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(expander, 0, sizeof(*expander));
  expander->port = port;
  expander->addr = MCP23008_ADDRESS + addr;

  // Configure the pin to receive interrupts from the MCP23008
  GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(interrupt_pin, &gpio_settings);
  gpio_it_register_interrupt(interrupt_pin, &it_settings, INTERRUPT_EDGE_FALLING,
                             prv_interrupt_handler, expander);

  // Set default configuration: input, interrupt on change, active-low interrupt
  uint8_t cfg[] = {
    0xFF,  // IODIR
    0x0,   // IPOL
    0x0,   // GPINTEN
    0x0,   // DEFVAL
    0x0,   // INTCON
    0x0,   // IOCON
    0x0,   // GPPU
  };
  return i2c_write_reg(expander->port, expander->addr, MCP23008_IODIR, cfg, SIZEOF_ARRAY(cfg));
}

StatusCode gpio_expander_init_pin(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                  const GPIOSettings *settings) {
  if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Set the direction of the data I/O
  prv_set_bit(expander, MCP23008_IODIR, pin, (settings->direction == GPIO_DIR_IN));
  // Disable interrupt in case it was set before
  prv_set_bit(expander, MCP23008_GPINTEN, pin, false);

  gpio_expander_set_state(expander, pin, settings->state);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_get_state(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                   GPIOState *state) {
  if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Update our GPIO register value and read the pin state
  uint8_t data = 0;
  status_ok_or_return(i2c_read_reg(expander->port, expander->addr, MCP23008_GPIO, &data, 1));

  *state = (data >> pin) & 1;

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_set_state(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                   GPIOState state) {
  if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Write to the GPIO register - updates OLAT (ignored if set to input)
  prv_set_bit(expander, MCP23008_GPIO, pin, state);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_register_callback(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                           GpioExpanderCallbackFn callback, void *context) {
  if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  expander->callbacks[pin].func = callback;
  expander->callbacks[pin].context = context;

  // Enable interrupt
  prv_set_bit(expander, MCP23008_GPINTEN, pin, true);

  return STATUS_CODE_OK;
}
