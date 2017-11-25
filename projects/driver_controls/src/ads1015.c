#include <ads1015.h>
#include <stdbool.h>
#include <stdio.h>

#include "ads1015_bitmasks.h"
#include "delay.h"
#include "gpio_it.h"
#include "i2c.h"

#define SWAP_UINT16(x) (uint16_t)(((uint16_t)(x) >> 8) | ((uint16_t)(x) << 8))

// Union definition to help process registers data
typedef struct ADS1015Register {
  uint8_t data[2];
} ADS1015Register;

// Since the ADS1015 stores data in two's complement, an int16_t will be used to hold
// the raw readings
typedef struct ADS1015Interrupt {
  ADS1015Callback callback;
  void *context;
  int16_t reading;
} ADS1015Interrupt;

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
  // We must make a write to the address pointer register to specify the register to be read
  // The address pointer register is one byte large
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_ADDRESS, &reg, 1));

  // Read from the register specified in the address pointer register
  status_ok_or_return(i2c_read(i2c_port, ADS1015_I2C_ADDRESS, data, 2));

  return STATUS_CODE_OK;
}

// Initiates a write as outlined in section 8.5.3 of the datasheet
static StatusCode prv_write(I2CPort i2c_port, uint8_t reg, uint8_t *data) {
  // For writes, the input data must be sent in the same frame as the address pointer register
  uint8_t write_data[] = { reg, data[0], data[1] };

  // After writing to the address pointer register, we can continue sending the data bytes without
  // a STOP or a repeated START condition
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_ADDRESS, write_data, 3));
  return STATUS_CODE_OK;
}

static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
  ADS1015Register reg;

  // Obtain ADC readings
  prv_read(s_i2c_port, ADS1015_CONVERSION_REGISTER, reg.data);

  s_interrupts[s_current_channel].reading = (reg.data[0] << 4) | (reg.data[1] >> 4);

  if (s_interrupts[s_current_channel].callback != NULL) {
    s_interrupts[s_current_channel].callback(s_current_channel,
                                             s_interrupts[s_current_channel].context);
  }

  // Update so that the ADC reads from the next channel
  s_current_channel = (s_current_channel + 1) % NUM_ADS1015_CHANNELS;

  // Obtain the current value for the config register
  prv_read(s_i2c_port, ADS1015_CONFIG_REGISTER, reg.data);

  // Set bits 12-14 based on the current channel
  reg.data[0] = ADS1015_CONFIG_MUX(reg.data[0], s_current_channel);
  prv_write(s_i2c_port, ADS1015_CONFIG_REGISTER, reg.data);
}

StatusCode ads1015_init(I2CPort i2c_port, GPIOAddress ready_pin) {
  s_i2c_port = i2c_port;
  s_current_channel = ADS1015_CHANNEL_0;

  // Configure the given GPIO address as a conversion ready pin
  const GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_PULLUP, GPIO_ALTFN_NONE };
  const InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&ready_pin, &gpio_settings);
  gpio_it_register_interrupt(&ready_pin, &it_settings, INTERRUPT_EDGE_FALLING,
                             prv_interrupt_handler, NULL);

  // Reset the internal registers to their default values
  uint8_t reset = ADS1015_RESET_BYTE;
  status_ok_or_return(i2c_write(i2c_port, 0, &reset, 1));

  ADS1015Register reg;

  // Read config register and write the proper configuration settings
  status_ok_or_return(prv_read(s_i2c_port, ADS1015_CONFIG_REGISTER, reg.data));

  reg.data[0] = ADS1015_CONFIG_MODE_CONT(reg.data[0]);
  reg.data[0] = ADS1015_FULL_SCALE_4096(reg.data[0]);
  reg.data[0] = ADS1015_CONFIG_MUX(reg.data[0], s_current_channel);
  reg.data[1] = ADS1015_CONFIG_COMP_QUE_ONE(reg.data[1]);
  reg.data[1] = ADS1015_CONFIG_DR_1600_SPS(reg.data[1]);

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

StatusCode ads1015_register_callback(ADS1015Channel channel, ADS1015Callback callback,
                                     void *context) {
  status_ok_or_return(prv_channel_valid(channel));

  s_interrupts[channel].callback = callback;
  s_interrupts[channel].context = context;

  return STATUS_CODE_OK;
}

StatusCode ads1015_read_raw(ADS1015Channel channel, int16_t *reading) {
  status_ok_or_return(prv_channel_valid(channel));

  if (reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *reading = s_interrupts[channel].reading;
  return STATUS_CODE_OK;
}

StatusCode ads1015_read_converted(ADS1015Channel channel, int16_t *reading) {
  int16_t raw_reading;

  status_ok_or_return(ads1015_read_raw(channel, &raw_reading));

  *reading = (raw_reading * ADS1015_REFERENCE_VOLTAGE_4096) / 2047;

  return STATUS_CODE_OK;
}
