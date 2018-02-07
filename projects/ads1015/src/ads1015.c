#include "ads1015.h"
#include <ads1015_def.h>
#include <stdio.h>

#include <gpio_it.h>

int16_t prv_twos_comp(int16_t x, uint8_t num_bit) {
  uint8_t msb = x & (1 << (num_bit - 1));
  uint8_t mask = ~(1 << (num_bit - 1));
  return ((x & mask) - msb);
}


static void prv_check_status(StatusCode code) {
  if (!code) {
    status_msg(code, "failed");
  }
}

static void prv_reset(ADS1015Data *data) {
  uint8_t reset = ADS1015_RESET_BYTE;
  StatusCode code = i2c_write(data->i2c_port, ADS1015_I2C_GENERAL_CALL, &reset, 1);
  prv_check_status(code);
}

static void prv_setup_register(ADS1015Data *data, uint8_t reg, uint8_t msb, uint8_t lsb) {
  uint8_t ads1015_setup_register[] = { reg, msb, lsb };
  StatusCode code = i2c_write(data->i2c_port, data->i2c_addr | ADS1015_I2C_ADDRESS_GND,
                              ads1015_setup_register, 3);
  prv_check_status(code);
}

static StatusCode prv_read(I2CPort i2c_port, ADS1015Address i2c_addr, uint8_t reg, uint8_t *read) {
  // write the address of desired register to address pointer register
  StatusCode code = i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, &reg, 1);
  prv_check_status(code);
  code = i2c_read(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, read, 2);
  prv_check_status(code);
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

static void prv_set_channel(ADS1015Data *data, ADS1015Channel channel) {
  uint8_t config_register_msb_channels[] = { CONFIG_REGISTER_MSB_0, CONFIG_REGISTER_MSB_1,
                                             CONFIG_REGISTER_MSB_2, CONFIG_REGISTER_MSB_3 };

  prv_setup_register(data, ADS1015_ADDRESS_POINTER_CONFIG, config_register_msb_channels[channel],
                     CONFIG_REGISTER_LSB);
}

static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
   ADS1015Data *data = context;
  // read the conversion value and store it in data
  uint8_t read_conv_register[2];
  prv_read(data->i2c_port, data->i2c_addr, ADS1015_ADDRESS_POINTER_CONV, read_conv_register);
  // read config register and get current channel
  uint8_t config_register[2];
  prv_read(data->i2c_port, data->i2c_addr, ADS1015_ADDRESS_POINTER_CONFIG, config_register);

  ADS1015Channel current_channel = prv_get_channel(config_register[0]);

  data->channel_readings[current_channel][0] = read_conv_register[0];
  data->channel_readings[current_channel][1] = read_conv_register[1];
  // Update so that the ADS1015 reads from the next channel
  do {
    current_channel = (current_channel + 1) % NUM_ADS1015_CHANNELS;
  } while (!data->channel_enable[current_channel]);
  prv_set_channel(data, current_channel);
  data->current_channel = current_channel;
}

StatusCode ads1015_init(ADS1015Data *data, I2CPort i2c_port, ADS1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  data->i2c_port = i2c_port;
  data->i2c_addr = i2c_addr;
  data->ready_pin = ready_pin;
  for (int channel = 0; channel < NUM_ADS1015_CHANNELS; channel++) {
    data->channel_enable[channel] = false;
    data->channel_readings[channel][0] = 0;
    data->channel_readings[channel][1] = 0;
    data->channel_callback[channel] = NULL;
    data->callback_context[channel] = NULL;
  }

  prv_reset(data);
  prv_setup_register(data, ADS1015_ADDRESS_POINTER_CONFIG, CONFIG_REGISTER_MSB_0,
                     CONFIG_REGISTER_LSB);
  prv_setup_register(data, ADS1015_ADDRESS_POINTER_LO_THRESH, ADS1015_LO_THRESH_REGISTER_MSB,
                     ADS1015_LO_THRESH_REGISTER_LSB);
  prv_setup_register(data, ADS1015_ADDRESS_POINTER_HI_THRESH, ADS1015_HI_THRESH_REGISTER_MSB,
                     ADS1015_HI_THRESH_REGISTER_LSB);

  // ALERT/RDY continues to require a pullup resistor.
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,  //
  };
  gpio_init_pin(data->ready_pin, &gpio_settings);
  return STATUS_CODE_OK;
}

StatusCode ads1015_configure_channel(ADS1015Data *data, ADS1015Channel channel, bool enable,
                                     ADS1015Callback callback, void *context) {
  // if no channels were already enabled, set to given channel
  if (!data->channel_enable[0] && !data->channel_enable[1] && !data->channel_enable[2] &&
      !data->channel_enable[3]) {
    prv_set_channel(data, channel);
    data->current_channel = channel;
  }

  data->channel_enable[channel] = enable;
  data->channel_callback[channel] = callback;
  data->callback_context[channel] = context;

  InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  StatusCode code = gpio_it_register_interrupt(data->ready_pin, &it_settings, INTERRUPT_EDGE_RISING,
                                    prv_interrupt_handler, data);
  prv_check_status(code);
  return STATUS_CODE_OK;
}

StatusCode ads1015_read_raw(ADS1015Data *data, ADS1015Channel channel, int16_t *reading) {
  *reading =
      ((data->channel_readings[channel][0] << 8) | (data->channel_readings[channel][1])) >> 4;
  // data->channel_readings[channel][1];
  return STATUS_CODE_OK;
}

StatusCode ads1015_read_converted(ADS1015Data *data, ADS1015Channel channel, int16_t *reading) {
  ads1015_read_raw(data, channel, reading);
 // *reading *= (ADS1015_REFERENCE_VOLTAGE_4096 * 2 /4096);
  *reading = prv_twos_comp(*reading, 12);
  return STATUS_CODE_OK;
}

