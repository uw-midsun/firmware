#include "ads1015.h"
#include <stdio.h>
#include "ads1015_def.h"
#include "gpio_it.h"

static void prv_check_status(StatusCode code) {
  if (!code) {
    status_msg(code, "failed");
  }
}

// reset the internal registers and enter a power-down state
static void prv_reset(ADS1015Data *data) {
  uint8_t reset = ADS1015_RESET_BYTE;
  StatusCode code = i2c_write(data->i2c_port, ADS1015_I2C_GENERAL_CALL, &reset, sizeof(reset));
  prv_check_status(code);
}

// writes to register given upper and lower bytes
static void prv_setup_register(ADS1015Data *data, uint8_t reg, uint8_t msb, uint8_t lsb) {
  uint8_t ads1015_setup_register[] = { reg, msb, lsb };
  StatusCode code = i2c_write(data->i2c_port, data->i2c_addr | ADS1015_I2C_ADDRESS_GND,
                              ads1015_setup_register, SIZEOF_ARRAY(ads1015_setup_register));
  prv_check_status(code);
}

// reads the register and stores the value in the given array(2 bytes)
static StatusCode prv_read(I2CPort i2c_port, ADS1015Address i2c_addr, uint8_t reg, uint8_t *read, size_t size) {
  StatusCode code = i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, &reg, sizeof(reg));
  prv_check_status(code);
  code = i2c_read(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, read, size);
  prv_check_status(code);
  return STATUS_CODE_OK;
}

// gets current channel based on the upper byte of config register
static ADS1015Channel prv_get_channel(uint8_t config_register_msb) {
  uint8_t channel = (config_register_msb >> 4) & 0x7;    // 4 <= channel <= 7
  channel -= 4;  // adjusts the register value to channels' enum: 0 <= channel <= 3
  return channel; 
}

// switches to the the given channel by writing to config reg
static void prv_set_channel(ADS1015Data *data, ADS1015Channel channel) {
  uint8_t config_register_msb_channels[] = { CONFIG_REGISTER_MSB_0, CONFIG_REGISTER_MSB_1,
                                             CONFIG_REGISTER_MSB_2, CONFIG_REGISTER_MSB_3 };

  prv_setup_register(data, ADS1015_ADDRESS_POINTER_CONFIG, config_register_msb_channels[channel],
                     CONFIG_REGISTER_LSB);
}

// This function is registered as the callback for ALRT/RDY Pin.
// reads and stores the conversion value in data, and switches to the next enabled channel. 
static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
   ADS1015Data *data = context;

   uint8_t read_conv_register[2];
   prv_read(data->i2c_port, data->i2c_addr, ADS1015_ADDRESS_POINTER_CONV, read_conv_register,
            SIZEOF_ARRAY(read_conv_register));

   uint8_t config_register[2];
   prv_read(data->i2c_port, data->i2c_addr, ADS1015_ADDRESS_POINTER_CONFIG, config_register,
            SIZEOF_ARRAY(config_register));

   ADS1015Channel current_channel = prv_get_channel(config_register[0]);
   data->channel_readings[current_channel][0] = read_conv_register[0];
   data->channel_readings[current_channel][1] = read_conv_register[1];
   // runs the users callback if not NULL
   if (data->channel_callback[current_channel]) {
     data->channel_callback[current_channel](data->ready_pin,
                                             data->callback_context[current_channel]);
  }
  // Update so that the ADS1015 reads from the next channel
  do {
    current_channel = (current_channel + 1) % NUM_ADS1015_CHANNELS;
  } while (!data->channel_enable[current_channel]);
  prv_set_channel(data, current_channel);
  data->current_channel = current_channel;
}

// initiates ads1015 by setting up registers and enabling ALRT/RDY Pin
StatusCode ads1015_init(ADS1015Data *data, I2CPort i2c_port, ADS1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  data->i2c_port = i2c_port;
  data->i2c_addr = i2c_addr;
  data->ready_pin = ready_pin;
  for (uint8_t channel = 0; channel < NUM_ADS1015_CHANNELS; channel++) {
    data->channel_enable[channel] = false;
    data->channel_readings[channel][0] = 0;
    data->channel_readings[channel][1] = 0;
    data->channel_callback[channel] = NULL;
    data->callback_context[channel] = NULL;
  }

  prv_reset(data);
  prv_setup_register(data, ADS1015_ADDRESS_POINTER_CONFIG, CONFIG_REGISTER_MSB_0,
                     CONFIG_REGISTER_LSB); // sets the starting channel to channel 0
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

// This function enable/disables channels, and registers callbacks for each channel.
// It also registers the interrupt handler on ALRT/RDY pin.
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

// reads raw 12 bit conversion results which are expressed in two's complement format
StatusCode ads1015_read_raw(ADS1015Data *data, ADS1015Channel channel, uint16_t *reading) {
  *reading =
      ((data->channel_readings[channel][0] << 8) | (data->channel_readings[channel][1])) >> 4;
  // data->channel_readings[channel][1];
  return STATUS_CODE_OK;
}

// sign extension for two's complement of numbers in n(< 16) bit register to 16 bit register 
static uint16_t prv_sign_ext_16bits(uint8_t num_bits, uint16_t twos_comp_num) {
  uint16_t msb = (1 << (num_bits - 1)) & twos_comp_num;
  msb = msb >> (num_bits - 1);
  uint8_t diff = 16 - num_bits;
  if (msb == 1) {
    uint16_t result = (((1 << diff) - 1) << num_bits) | twos_comp_num;
    return result;
  } else {
    return twos_comp_num;
  }
}

// convert from two's complement format to to signed int
static int16_t prv_twos_comp_16bits(uint16_t twos_comp_num) {
  uint16_t msb = twos_comp_num & (1 << 15);
  uint16_t mask = ~msb;
  return ((twos_comp_num & mask) - msb);
}

// reads conversion value in mVolt
StatusCode ads1015_read_converted(ADS1015Data *data, ADS1015Channel channel, int16_t *reading) {
  uint16_t raw_12bit;
  ads1015_read_raw(data, channel, &raw_12bit);
  int16_t temp = prv_twos_comp_16bits(prv_sign_ext_16bits(12, raw_12bit));
  *reading = temp * ADS1015_FSR_4096 / (1 << 12);
  return STATUS_CODE_OK;
}

