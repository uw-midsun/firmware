#include "ads1015.h"
#include <ads1015_def.h>
#include <stdio.h>

#include <gpio_it.h>
/*
typedef struct ADS1015Data {
  I2CPort i2c_port,
  ADS1015Address i2c_addr,
  GPIOAddress *ready_pin,
  uint8_t channel_readings[4][2] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };
  ADS1015Channel current_channel,
  bool channel_enable[] = { false, false, false, false },
  ADS1015Callback channel_callback[] = { NULL, NULL, NULL, NULL },
  void *callback_context[] = { NULL, NULL, NULL, NULL },
} ADS1015Data;
*/
static StatusCode prv_read(I2CPort i2c_port, ADS1015Address i2c_addr, uint8_t reg, uint8_t *read) {
  status_ok_or_return(i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, &reg, 1));
  status_ok_or_return(i2c_read(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, read, 2));
  return STATUS_CODE_OK;
}

static ADS1015Channel prv_get_channel(uint8_t config_register_msb) {
  config_register_msb = config_register_msb >> 4;
  config_register_msb &= (0x7);
  if (config_register_msb == 4) {
    return ADS1015_CHANNEL_0;
  } else if (config_register_msb == 5) {
    return ADS1015_CHANNEL_1;
  } else if (config_register_msb == 6) {
    return ADS1015_CHANNEL_2;
  } else {
    return ADS1015_CHANNEL_3;
  }
}

static void prv_set_channel(I2CPort i2c_port, ADS1015Address i2c_addr, ADS1015Channel channel) {
  uint8_t config_register_msb_channels[] = { CONFIG_REGISTER_MSB_0, CONFIG_REGISTER_MSB_1,
                                             CONFIG_REGISTER_MSB_2, CONFIG_REGISTER_MSB_3 };

  uint8_t config_register_switch_channel[] = { ADS1015_ADDRESS_POINTER_CONFIG,
                                               config_register_msb_channels[channel],
                                               CONFIG_REGISTER_LSB };

  i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, config_register_switch_channel, 3);
}

static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
 // for(int i=0; i<10; i++){printf("interrupt");}
  ADS1015Data *data = context;
  // read config register and get current channel
  uint8_t config_register[2];
  prv_read(data->i2c_port, data->i2c_addr, ADS1015_ADDRESS_POINTER_CONFIG, config_register);
  //i2c_read_reg(data->i2c_port, data->i2c_addr | ADS1015_I2C_ADDRESS_GND,
  //               ADS1015_ADDRESS_POINTER_CONFIG, &config_register_msb, 1);
  ADS1015Channel current_channel = prv_get_channel(config_register[0]);
  // read the conversion value and store it in data
  uint8_t read_conv_register[2];
  prv_read(data->i2c_port, data->i2c_addr, ADS1015_ADDRESS_POINTER_CONV, read_conv_register);
  //i2c_read_reg(data->i2c_port, data->i2c_addr | ADS1015_I2C_ADDRESS_GND,
   //            ADS1015_ADDRESS_POINTER_CONV, read_conv_register, 2);
  data->channel_readings[current_channel][0] = read_conv_register[0];
  data->channel_readings[current_channel][1] = read_conv_register[1];
  // Update so that the ADS1015 reads from the next channel
  do {
    current_channel = (current_channel + 1) % NUM_ADS1015_CHANNELS;
  } while (!data->channel_enable[current_channel]);
  prv_set_channel(data->i2c_port, data->i2c_addr, current_channel);
  data->current_channel = current_channel;
}

StatusCode ads1015_init(ADS1015Data *data, I2CPort i2c_port, ADS1015Address i2c_addr, GPIOAddress *ready_pin) {
  data->i2c_port = i2c_port;
  data->i2c_addr = i2c_addr;
  data->ready_pin = ready_pin;
  for (int i=0; i<4; i++) data->channel_enable[i] = false;
  for (int i=0; i<4; i++){
    data->channel_readings[i][0] = 0;
    data->channel_readings[i][1] = 0;
  } 
  uint8_t reset = ADS1015_RESET_BYTE;
  status_ok_or_return(i2c_write(i2c_port, ADS1015_I2C_GENERAL_CALL, &reset, 1));

  uint8_t ads1015_setup_config[] = { ADS1015_ADDRESS_POINTER_CONFIG, CONFIG_REGISTER_MSB_0, CONFIG_REGISTER_LSB };
  status_ok_or_return(
    i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, ads1015_setup_config, 3));

  uint8_t ads1015_setup_lo_thresh[] = { ADS1015_ADDRESS_POINTER_LO_THRESH,
                                        ADS1015_LO_THRESH_REGISTER_MSB,
                                        ADS1015_LO_THRESH_REGISTER_MSB };
  status_ok_or_return(
    i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, ads1015_setup_lo_thresh, 3));

  uint8_t ads1015_setup_hi_thresh[] = { ADS1015_ADDRESS_POINTER_HI_THRESH,
                                        ADS1015_HI_THRESH_REGISTER_MSB,
                                        ADS1015_HI_THRESH_REGISTER_MSB };
  status_ok_or_return(
    i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, ads1015_setup_hi_thresh, 3));

  return STATUS_CODE_OK;
}



StatusCode ads1015_configure_channel(ADS1015Data *data, ADS1015Channel channel, bool enable,
                                     ADS1015Callback callback, void *context) {
  // if no channels were already enabled, set to given channel
  if (!data->channel_enable[0] && !data->channel_enable[1] && 
      !data->channel_enable[2] && !data->channel_enable[3]) {
    prv_set_channel(data->i2c_port, data->i2c_addr, channel);
    data->current_channel = channel;
  }

  data->channel_enable[channel] = enable;
  data->channel_callback[channel] = callback;
  data->callback_context[channel] = context;
  // ALERT/RDY continues to require a pullup resistor.
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,   //
  };

  gpio_init_pin(data->ready_pin, &gpio_settings);
  InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  status_ok_or_return(
    gpio_it_register_interrupt(data->ready_pin, &it_settings, INTERRUPT_EDGE_FALLING, prv_interrupt_handler, data));
  
  return STATUS_CODE_OK;
}


StatusCode ads1015_read_raw(ADS1015Data *data, ADS1015Channel channel, int16_t *reading) {
  *reading = ((data->channel_readings[channel][0] << 8) | (data->channel_readings[channel][1])) >> 4;
  //data->channel_readings[channel][1];
  return STATUS_CODE_OK;
}
