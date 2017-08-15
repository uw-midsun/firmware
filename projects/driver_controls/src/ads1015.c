#include "ads1015.h"
#include <stdbool.h>
#include <stdio.h>

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

// Union definition to help process 16-bit registers
// Due to endian-ness, index 0 refers to the most significant byte
typedef union ADS1015Register {
  uint16_t raw;
  uint8_t data[2];
} ADS1015Register;

static GPIOAddress s_address;

static StatusCode prv_read(I2CPort i2c_port, uint8_t reg, uint8_t *data) {
  // Update the address pointer register with the proper value for the target register
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_ADDRESS, &reg, 1));

  status_ok_or_return(i2c_read(i2c_port, ADS1015_I2C_ADDRESS, data, 2));

  return STATUS_CODE_OK;
}
static StatusCode prv_write(I2CPort i2c_port, uint8_t reg, uint8_t *data) {
  //TODO: DOCUMENT
  // Update the address pointer register and write the data to the proper address
  uint8_t write_data[] = { reg, data[0], data[1] };
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_ADDRESS, write_data, 3));

  return STATUS_CODE_OK;
}

StatusCode ads1015_init(I2CPort i2c_port, GPIOAddress address, ADS1015Mode op_mode) {
  s_address = address;

  // Reset the internal registers to their default values 
  uint8_t reset = ADS1015_RESET_BYTE;
  status_ok_or_return(i2c_write(i2c_port, 0, &reset, 1));

  // Configure the device operating mode 

  ADS1015Register config;
  status_ok_or_return(prv_read(I2C_PORT_1, ADS1015_CONFIG_REGISTER, config.data));

  switch (op_mode) {
    case ADS1015_MODE_SINGLE_SHOT:
      config.data[0] |= (ADS1015_CONFIG_MODE >> 8);
    case ADS1015_MODE_CONTINUOUS:
      config.data[0] &= ~(ADS1015_CONFIG_MODE >> 8);
  }

  status_ok_or_return(prv_write(I2C_PORT_1, ADS1015_CONFIG_REGISTER, config.data));

  return STATUS_CODE_OK;
}

StatusCode ads1015_register_callback(ADS1015Channel channel, ADS1015Callback callback, void *context) {

}

StatusCode ads1015_read_raw(ADS1015Channel channel, uint16_t *reading) {
  // Select the channel to convert
  ADS1015Register reg;
  status_ok_or_return(prv_read(I2C_PORT_1, ADS1015_CONFIG_REGISTER, reg.data));

  //printf("data = %#x%x -> ", reg.data[0], reg.data[1]);
  // Clear bits 12-14
  reg.raw &= 0x8FFF;

  // Set mux bits to the correct setting based on the selected channel
  switch (channel) {
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

  //printf("%#x%x\n", reg.data[0], reg.data[1]);

  status_ok_or_return(prv_write(I2C_PORT_1, ADS1015_CONFIG_REGISTER, reg.data));

  // Obtain ADC readings
  status_ok_or_return(prv_read(I2C_PORT_1, ADS1015_CONVERSION_REGISTER, reg.data));

  // Data is stored in the 12 most significant bits
  *reading = (reg.raw >> 4);
  printf("%d\n", *reading);
  return STATUS_CODE_OK;
}

StatusCode ads1015_read_converted(ADS1015Channel channel, uint16_t *reading) {

}