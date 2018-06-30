#include "gpio_expander.h"
#include <stdbool.h>
#include <string.h>
#include "gpio_it.h"
#include "i2c.h"
#include "mcp23008.h"

static void prv_poll_timeout(SoftTimerID timer_id, void *context) {
  GpioExpanderStorage *expander = context;
  uint8_t gpio = 0;

  // Read the contents of the GPIO registers
  i2c_read_reg(expander->port, expander->addr, MCP23008_GPIO, &gpio, 1);

  uint8_t changed = expander->prev_state ^ gpio;
  expander->prev_state = gpio;
  // Looping through every single bit, and adjusting the count accordingly.
  for (uint8_t i = 0; i < NUM_GPIO_EXPANDER_ADDRESSES; i++) {
    if (changed & (1 << i)) {
      expander->stability_count[i] = 0;
    } else {
      expander->stability_count[i]++;
      if (expander->stability_count[i] == GPIO_EXPANDER_STABILITY_MEASURE) {
        // If after increment any of the pins reach their steady count, fire callbacks.
        uint8_t last_bit = (expander->last_stable >> i) & 1;
        uint8_t current_bit = (gpio >> i) & 1;
        if (last_bit != current_bit) {
          if (expander->callbacks[i].func != NULL) {
            expander->callbacks[i].func(i, (gpio >> i) & 1, expander->callbacks[i].context);
          }
          expander->last_stable ^= (1 << i);
        }
        expander->stability_count[i] = 0;
      }
    }
  }
  soft_timer_start_millis(GPIO_EXPANDER_POLL_PERIOD_MS, prv_poll_timeout, expander, NULL);
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
  if (expander == NULL || addr >= NUM_GPIO_EXPANDER_ADDRESSES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(expander, 0, sizeof(*expander));
  expander->port = port;
  expander->addr = MCP23008_ADDRESS + addr;

  // If we don't have an interrupt pin registered, this will be an output-only IO expander.
  if (interrupt_pin != NULL) {
    soft_timer_start_millis(GPIO_EXPANDER_POLL_PERIOD_MS, prv_poll_timeout, expander, NULL);
  }

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
  if (expander == NULL || settings == NULL ||
      (settings->resistor != GPIO_RES_NONE && settings->resistor != GPIO_RES_PULLUP)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Set the direction of the data I/O
  prv_set_bit(expander, MCP23008_IODIR, pin, (settings->direction == GPIO_DIR_IN));
  // Set the pull-up
  prv_set_bit(expander, MCP23008_GPPU, pin, (settings->resistor == GPIO_RES_PULLUP));
  // Disable interrupt in case it was set before
  prv_set_bit(expander, MCP23008_GPINTEN, pin, false);

  gpio_expander_set_state(expander, pin, settings->state);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_get_state(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                   GPIOState *state) {
  if (expander == NULL || state == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Update our GPIO register value and read the pin state
  uint8_t data = 0;
  status_ok_or_return(i2c_read_reg(expander->port, expander->addr, MCP23008_GPIO, &data, 1));

  *state = (GPIOState)((data >> pin) & 1);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_set_state(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                   GPIOState state) {
  if (expander == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Write to the GPIO register - updates OLAT (ignored if set to input)
  prv_set_bit(expander, MCP23008_GPIO, pin, state);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_register_callback(GpioExpanderStorage *expander, GpioExpanderPin pin,
                                           GpioExpanderCallbackFn callback, void *context) {
  if (expander == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (pin >= NUM_GPIO_EXPANDER_PINS) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  expander->callbacks[pin].func = callback;
  expander->callbacks[pin].context = context;

  // Enable interrupt
  prv_set_bit(expander, MCP23008_GPINTEN, pin, true);

  return STATUS_CODE_OK;
}
