#include "ads1015.h"

#include <stdbool.h>
#include <stdio.h>

#include "gpio_it.h"

// ADS1015 I2C address definition (Datasheet Section 8.5.1.1)
#define ADS1015_I2C_ADDRESS             0x48

// ADS1015 I2C reset byte (Datasheet Section 8.5.1.2)
#define ADS1015_RESET_BYTE              0x06

// Register pointer definitions (Datasheet Table 4)
#define ADS1015_CONVERSION_REGISTER     0x00
#define ADS1015_CONFIG_REGISTER         0x01
#define ADS1015_LO_THRESH_REGISTER      0x02
#define ADS1015_HI_THRESH_REGISTER      0x03

// ADS1015 Bitmasks
#define ADS1015_CONFIG_MODE             0x100

#define ADS1015_CONFIG_MUX_AIN0         0x4000
#define ADS1015_CONFIG_MUX_AIN1         0x5000
#define ADS1015_CONFIG_MUX_AIN2         0x6000
#define ADS1015_CONFIG_MUX_AIN3         0x7000

// Number of successive conversions required before asserting ALERT/RDY
#define ADS1015_CONFIG_COMP_QUE_ONE     0x1
#define ADS1015_CONFIG_COMP_QUE_TWO     0x2
#define ADS1015_CONFIG_COMP_QUE_FOUR    0x4

// Union definition to help process 16-bit registers
// Due to endian-ness, index 0 refers to the most significant byte
typedef union ADS1015Register {
  uint16_t raw;
  uint8_t data[2];
} ADS1015Register;

typedef struct ADS1015Interrupt {
  ADS1015Callback callback;
  void *context;
  int16_t reading;
} ADS1015Interrupt;

static GPIOAddress s_address;
static ADS1015Channel s_current_channel;

static ADS1015Interrupt s_interrupts[NUM_ADS1015_CHANNELS];

static StatusCode prv_channel_valid(ADS1015Channel channel) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

// Initiates a read as outlined in section 8.5.3 of the datasheet
static StatusCode prv_read(I2CPort i2c_port, uint8_t reg, uint8_t *data) {
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_ADDRESS, &reg, 1));
  status_ok_or_return(i2c_read(i2c_port, ADS1015_I2C_ADDRESS, data, 2));

  return STATUS_CODE_OK;
}

// Initiates a write as outlined in section 8.5.3 of the datasheet
static StatusCode prv_write(I2CPort i2c_port, uint8_t reg, uint8_t *data) {
  // For writes, the input data must be sent in the same frame as the address pointer register
  uint8_t write_data[] = { reg, data[0], data[1] };
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_ADDRESS, write_data, 3));

  return STATUS_CODE_OK;
}

static void prv_interrupt_handler(GPIOAddress *address, void *context) {
  ADS1015Register reg;

  // Obtain ADC readings
  status_ok_or_return(prv_read(I2C_PORT_1, ADS1015_CONVERSION_REGISTER, reg.data));
  s_interrupts[s_current_channel].reading = (reg.data[0] << 4) | (reg.data[1] >> 4);

  if (s_interrupts[s_current_channel].callback != NULL) {
    s_interrupts[s_current_channel].callback(s_current_channel,
                                              s_interrupts[s_current_channel].context);
  }

  // Update so that the ADC reads from the next channel
  s_current_channel = (s_current_channel + 1) % NUM_ADS1015_CHANNELS;

  // Obtain the current value for the config register
  status_ok_or_return(prv_read(I2C_PORT_1, ADS1015_CONFIG_REGISTER, reg.data));

  // Set bits 12-14 based on the current channel
  reg.data[0] &= 0x81;

  switch (s_current_channel) {
    case ADS1015_CHANNEL_0:
      reg.raw |= (ADS1015_CONFIG_MUX_AIN0 >> 8);
      break;
    case ADS1015_CHANNEL_1:
      reg.raw |= (ADS1015_CONFIG_MUX_AIN1 >> 8);
      break;
    case ADS1015_CHANNEL_2:
      reg.raw |= (ADS1015_CONFIG_MUX_AIN2 >> 8);
      break;
    case ADS1015_CHANNEL_3:
      reg.raw |= (ADS1015_CONFIG_MUX_AIN3 >> 8);
      break;
  }

  status_ok_or_return(prv_write(I2C_PORT_1, ADS1015_CONFIG_REGISTER, reg.data));

  return STATUS_CODE_OK;
}

StatusCode ads1015_init(I2CPort i2c_port, GPIOAddress address) {
  // Reset the internal registers to their default values
  uint8_t reset = ADS1015_RESET_BYTE;
  status_ok_or_return(i2c_write(i2c_port, 0, &reset, 1));

  // Initialize the ADC in continuous mode
  ADS1015Register config;
  status_ok_or_return(prv_read(I2C_PORT_1, ADS1015_CONFIG_REGISTER, config.data));
  printf("data = %#x%x\n", config.data[0], config.data[1]);
  config.data[0] &= ~(ADS1015_CONFIG_MODE >> 8);
  printf("data = %#x%x\n", config.data[0], config.data[1]);
  status_ok_or_return(prv_write(I2C_PORT_1, ADS1015_CONFIG_REGISTER, config.data));

  // Configure the given GPIO address as a conversion ready pin
  GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_FALLING,
                              prv_interrupt_handler, NULL);

  // Conversion ready must be initialized by setting the MSB of the HI_THRESH and LO_THRESH to 1
  // and 0 respectively
  uint8_t data[] = { 0x80, 0x00 };
  prv_write(I2C_PORT_1, ADS1015_HI_THRESH_REGISTER, data);

  data[0] = 0x00;
  prv_write(I2C_PORT_1, ADS1015_LO_THRESH_REGISTER, data);

  // Enable ALERT/RDY pin
  data[1] = ADS1015_CONFIG_COMP_QUE_FOUR;
  prv_write(I2C_PORT_1, ADS1015_CONFIG_REGISTER, data);

  // Initialize interrupt callbacks
  for (uint8_t i = 0; i < NUM_ADS1015_CHANNELS; i++) {
    s_interrupts[i].callback = NULL;
    s_interrupts[i].context = NULL;
    s_interrupts[i].reading = 0;
  }

  return STATUS_CODE_OK;
}

StatusCode ads1015_register_callback(ADS1015Channel channel,
                                      ADS1015Callback callback, void *context) {
  status_ok_or_return(prv_channel_valid(channel));

  s_interrupts[channel].callback = callback;
  s_interrupts[channel].context = context;

  return STATUS_CODE_OK;
}

StatusCode ads1015_read_raw(ADS1015Channel channel, uint16_t *reading) {
  status_ok_or_return(prv_channel_valid(channel));

  *reading = s_interrupts[channel].reading;
}

StatusCode ads1015_read_converted(ADS1015Channel channel, uint16_t *reading) { }
