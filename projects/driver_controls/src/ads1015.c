#include "ads1015.h"

#include <stdbool.h>
#include <stdio.h>

#include "i2c.h"
#include "gpio_it.h"
#include "ads1015_bitmasks.h"

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
static I2CPort s_i2c_port;
static ADS1015Channel s_current_channel;

static ADS1015Interrupt s_interrupts[NUM_ADS1015_CHANNELS];

static StatusCode prv_channel_valid(ADS1015Channel channel) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
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
  status_ok_or_return(prv_read(s_i2c_port, ADS1015_CONVERSION_REGISTER, reg.data));
  s_interrupts[s_current_channel].reading = (reg.data[0] << 4) | (reg.data[1] >> 4);

  if (s_interrupts[s_current_channel].callback != NULL) {
    s_interrupts[s_current_channel].callback(s_current_channel,
                                              s_interrupts[s_current_channel].context);
  }

  // Update so that the ADC reads from the next channel
  s_current_channel = (s_current_channel + 1) % NUM_ADS1015_CHANNELS;

  // Obtain the current value for the config register
  status_ok_or_return(prv_read(s_i2c_port, ADS1015_CONFIG_REGISTER, reg.data));

  // Set bits 12-14 based on the current channel
  switch (s_current_channel) {
    case ADS1015_CHANNEL_0:
      reg.data[0] = ADS1015_CONFIG_MUX_AIN0(reg.data[0]);
      break;
    case ADS1015_CHANNEL_1:
      reg.data[0] = ADS1015_CONFIG_MUX_AIN1(reg.data[0]);
      break;
    case ADS1015_CHANNEL_2:
      reg.data[0] = ADS1015_CONFIG_MUX_AIN2(reg.data[0]);
      break;
    case ADS1015_CHANNEL_3:
      reg.data[0] = ADS1015_CONFIG_MUX_AIN3(reg.data[0]);
      break;
  }

  status_ok_or_return(prv_write(s_i2c_port, ADS1015_CONFIG_REGISTER, reg.data));

  /*
  printf("[ %d\t%d\t%d\t%d ]\n",
          s_interrupts[ADS1015_CHANNEL_0].reading,
          s_interrupts[ADS1015_CHANNEL_1].reading,
          s_interrupts[ADS1015_CHANNEL_2].reading,
          s_interrupts[ADS1015_CHANNEL_3].reading);
  */
  return STATUS_CODE_OK;
}

StatusCode ads1015_init(I2CPort i2c_port, GPIOAddress address) {
  s_i2c_port = i2c_port;

  // Configure the given GPIO address as a conversion ready pin
  GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_FALLING,
                              prv_interrupt_handler, NULL);

  // Reset the internal registers to their default values
  uint8_t reset = ADS1015_RESET_BYTE;
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_ADDRESS, &reset, 1));

  ADS1015Register reg;

  // Read config register and write the proper configuration settings
  status_ok_or_return(prv_read(s_i2c_port, ADS1015_CONFIG_REGISTER, reg.data));

  reg.data[0] = ADS1015_CONFIG_MODE_CONT(reg.data[0]);
  //reg.data[0] = ADS1015_FULL_SCALE_4096(reg.data[0]);
  reg.data[1] = ADS1015_CONFIG_COMP_QUE_FOUR(reg.data[1]);
  reg.data[1] = ADS1015_CONFIG_DR_128_SPS(reg.data[1]);

  status_ok_or_return(prv_write(s_i2c_port, ADS1015_CONFIG_REGISTER, reg.data));

  // Conversion ready must be initialized by setting the MSB of the HI_THRESH and LO_THRESH to 1
  // and 0 respectively
  reg.data[0] = ADS1015_HI_THRESH_RDY;
  status_ok_or_return(prv_write(s_i2c_port, ADS1015_HI_THRESH_REGISTER, reg.data));
  reg.data[0] = ADS1015_LO_THRESH_RDY;
  status_ok_or_return(prv_write(s_i2c_port, ADS1015_LO_THRESH_REGISTER, reg.data));

  // Initialize interrupt callbacks
  for (uint8_t i = 0; i < NUM_ADS1015_CHANNELS; i++) {
    s_interrupts[i].callback = NULL;
    s_interrupts[i].context = NULL;
    s_interrupts[i].reading = 0;
  }

  return STATUS_CODE_OK;
}

StatusCode ads1015_register_callback(ADS1015Channel channel, ADS1015Callback callback, void *context) {
  status_ok_or_return(prv_channel_valid(channel));

  s_interrupts[channel].callback = callback;
  s_interrupts[channel].context = context;

  return STATUS_CODE_OK;
}

StatusCode ads1015_read_raw(ADS1015Channel channel, int16_t *reading) {
  status_ok_or_return(prv_channel_valid(channel));

  if (reading == NULL) {
    return status_code(STATUS_CODE_EMPTY);
  }

  *reading = s_interrupts[channel].reading;
  return STATUS_CODE_OK;
}

StatusCode ads1015_read_converted(ADS1015Channel channel, int16_t *reading) {
  int16_t raw_reading;

  StatusCode ret = ads1015_read_raw(channel, &raw_reading);

  if (ret != STATUS_CODE_OK) {
    return ret;
  }

  *reading = (raw_reading * ADS1015_REFERENCE_VOLTAGE_4096)/2047;

  return ret;
}
