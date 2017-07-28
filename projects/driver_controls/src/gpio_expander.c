#include "gpio_expander.h"

#include <stdbool.h>

#include "i2c.h"
#include "gpio_it.h"

// MCP23008 I2C address (Datasheet Figure 1-4)
#define GPIO_EXPANDER_ADDRESS   0x20

// MCP23008 register addresses (Datasheet Table 1-3)
#define GPIO_EXPANDER_IODIR     0x0
#define GPIO_EXPANDER_IPOL      0x1
#define GPIO_EXPANDER_GPINTEN   0x2
#define GPIO_EXPANDER_DEFVAL    0x3
#define GPIO_EXPANDER_INTCON    0x4
#define GPIO_EXPANDER_IOCON     0x5
#define GPIO_EXPANDER_GPPU      0x6
#define GPIO_EXPANDER_INTF      0x7
#define GPIO_EXPANDER_INTCAP    0x8
#define GPIO_EXPANDER_GPIO      0x9
#define GPIO_EXPANDER_OLAT      0xA

#define NUM_GPIO_EXPANDER_REGISTERS 11

typedef struct GPIOExpanderInterrupt {
  GPIOExpanderPin pin;
  GPIOExpanderCallback callback;
  void *context;
} GPIOExpanderInterrupt;

static GPIOAddress s_address;
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

static void prv_update_registers() {
  i2c_read_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_INTCAP,
                &s_register[GPIO_EXPANDER_INTCAP], 1);
  i2c_read_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPIO,
                &s_register[GPIO_EXPANDER_GPIO], 1);
}

static void prv_interrupt_handler(GPIOAddress *address, void *context) {
  GPIOExpanderPin current_pin;

  // Update interrupt flag register.
  i2c_read_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_INTF,
                &s_register[GPIO_EXPANDER_INTF], 1);

  while (s_register[GPIO_EXPANDER_INTF] != 0) {
    current_pin = __builtin_ffs(s_register[GPIO_EXPANDER_INTF]) - 1;
    s_interrupts[current_pin].callback(current_pin, s_interrupts[current_pin].context);

    s_register[GPIO_EXPANDER_INTF] &= ~(1 << current_pin);
  }

  // Clear the interrupt by reading the port registers
  prv_update_registers();
}

static void prv_set_bit(uint8_t *data, GPIOExpanderPin pin, bool bit) {
  if (bit) {
    *data |= (1 << pin);
  } else {
    *data &= ~(1 << pin);
  }
}

StatusCode gpio_expander_init(GPIOAddress address) {
  s_address = address;

  // Configure the pin to receive interrupts from the MCP23008
  GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_RISING_FALLING,
                              prv_interrupt_handler, NULL);

  // Configure the I2C lines
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .sda = { GPIO_PORT_B, 9 },
    .scl = { GPIO_PORT_B, 8 }
  };

  i2c_init(I2C_PORT_1, &settings);

  for (uint8_t i = 0; i < NUM_GPIO_EXPANDER_REGISTERS; i++) {
    i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, i, &s_register[i], 1);
  }

  prv_update_registers();

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_init_pin(GPIOExpanderPin pin, GPIOSettings *settings) {
  if (pin >= NUM_GPIO_EXPANDER_PIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the direction of the data I/O

  prv_set_bit(&s_register[GPIO_EXPANDER_IODIR], pin, (settings->direction == GPIO_DIR_IN));
  i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_IODIR,
                &s_register[GPIO_EXPANDER_IODIR], 1);

  if (settings->direction == GPIO_DIR_OUT) {
    // Set the output state if the pin is an output
    prv_set_bit(&s_register[GPIO_EXPANDER_GPIO], pin, (settings->state == GPIO_STATE_HIGH));
    i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPIO,
                  &s_register[GPIO_EXPANDER_GPIO], 1);
  } else {
    // Enable interrupts if the pin is an input
    prv_set_bit(&s_register[GPIO_EXPANDER_GPINTEN], pin, true);
    i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPINTEN,
                  &s_register[GPIO_EXPANDER_GPINTEN], 1);

    // Configure so that any change in value triggers an interrupt
    prv_set_bit(&s_register[GPIO_EXPANDER_INTCON], pin, false);
    i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_INTCON,
                  &s_register[GPIO_EXPANDER_INTCON], 1);
  }

  prv_update_registers();

  return STATUS_CODE_OK;
}

bool gpio_expander_get_state(GPIOExpanderPin pin) {
  prv_update_registers();
  bool state = (s_register[GPIO_EXPANDER_GPIO] >> pin) & 1;

  return state;
}

StatusCode gpio_expander_set_state(GPIOExpanderPin pin, bool new_state) {
  uint8_t io_dir = s_register[GPIO_EXPANDER_IODIR];
  // Return if the pin is not configured as an output
  if ((io_dir >> pin) & 1) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_set_bit(&s_register[GPIO_EXPANDER_GPIO], pin, new_state);
  i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPIO,
                &s_register[GPIO_EXPANDER_GPIO], 1);

  return STATUS_CODE_OK;
}

StatusCode gpio_expander_register_callback(GPIOExpanderPin pin, GPIOExpanderCallback callback,
                                           void *context) {
  if (pin >= NUM_GPIO_EXPANDER_PIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_interrupts[pin].callback = callback;
  s_interrupts[pin].context = context;

  return STATUS_CODE_OK;
}
