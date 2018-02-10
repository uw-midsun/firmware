#include "ads1015.h"
#include <status.h>
#include <stdio.h>
#include "ads1015_def.h"
#include "gpio_it.h"

// reset the internal registers and enter a power-down state
static StatusCode prv_reset(Ads1015Storage *storage) {
  uint8_t reset = ADS1015_RESET_BYTE;
  return i2c_write(storage->i2c_port, ADS1015_I2C_GENERAL_CALL, &reset, sizeof(reset));
}

// writes to register given upper and lower bytes
static StatusCode prv_setup_register(Ads1015Storage *storage, uint8_t reg, uint8_t msb,
                                     uint8_t lsb) {
  uint8_t ads1015_setup_register[] = { reg, msb, lsb };
  return i2c_write(storage->i2c_port, storage->i2c_addr + ADS1015_I2C_BASE_ADDRESS,
                   ads1015_setup_register, SIZEOF_ARRAY(ads1015_setup_register));
}

// reads the register and stores the value in the given array(2 bytes)
static StatusCode prv_read(I2CPort i2c_port, Ads1015Address i2c_addr, uint8_t reg, uint8_t *read,
                           size_t size) {
  status_ok_or_return(i2c_write(i2c_port, i2c_addr + ADS1015_I2C_BASE_ADDRESS, &reg, sizeof(reg)));

  status_ok_or_return(i2c_read(i2c_port, i2c_addr + ADS1015_I2C_BASE_ADDRESS, read, size));
  return STATUS_CODE_OK;
}

// gets current channel based on the upper byte of config register
static Ads1015Channel prv_get_channel(uint8_t config_register_msb) {
  uint8_t channel = (config_register_msb >> 4) & 0x7;  // 4 <= channel <= 7
  channel -= 4;  // adjusts the register value to channels' enum: 0 <= channel <= 3
  return channel;
}

// switches to the the given channel by writing to config reg
static StatusCode prv_set_channel(Ads1015Storage *storage, Ads1015Channel channel) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  storage->current_channel = channel;
  storage->channel_enable[channel] = true;
  return prv_setup_register(storage, ADS1015_ADDRESS_POINTER_CONFIG, CONFIG_REGISTER_MSB(channel),
                            CONFIG_REGISTER_LSB);
}

// This function is registered as the callback for ALRT/RDY Pin.
// reads and stores the conversion value in storage, and switches to the next enabled channel.
static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
  Ads1015Storage *storage = context;

  uint8_t read_conv_register[2];
  prv_read(storage->i2c_port, storage->i2c_addr, ADS1015_ADDRESS_POINTER_CONV, read_conv_register,
           SIZEOF_ARRAY(read_conv_register));

  uint8_t config_register[2];
  prv_read(storage->i2c_port, storage->i2c_addr, ADS1015_ADDRESS_POINTER_CONFIG, config_register,
           SIZEOF_ARRAY(config_register));

  Ads1015Channel current_channel = prv_get_channel(config_register[0]);
  // following line puts two read bytes into a uint16. 4 LSB's should be taken out.
  // conversion value is only 12 bits.
  storage->channel_readings[current_channel] =
      ((read_conv_register[0] << 8) | read_conv_register[1]) >> 4;
  // runs the users callback if not NULL
  if (storage->channel_callback[current_channel]) {
    storage->channel_callback[current_channel](storage->current_channel,
                                               storage->callback_context[current_channel]);
  }
  // Update so that the ADS1015 reads from the next channel
  do {
    current_channel = (current_channel + 1) % NUM_ADS1015_CHANNELS;
  } while (!storage->channel_enable[current_channel]);
  prv_set_channel(storage, current_channel);
  storage->current_channel = current_channel;
}

// initiates ads1015 by setting up registers and enabling ALRT/RDY Pin
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  if (!storage || !ready_pin) {
    return STATUS_CODE_INVALID_ARGS;
  }
  storage->i2c_port = i2c_port;
  storage->i2c_addr = i2c_addr;
  storage->ready_pin = *ready_pin;
  for (uint8_t channel = 0; channel < NUM_ADS1015_CHANNELS; channel++) {
    storage->channel_enable[channel] = false;
    storage->channel_readings[channel] = 0;
    storage->channel_callback[channel] = NULL;
    storage->callback_context[channel] = NULL;
  }

  status_ok_or_return(prv_reset(storage));
  status_ok_or_return(prv_setup_register(
      storage, ADS1015_ADDRESS_POINTER_CONFIG, CONFIG_REGISTER_MSB(ADS1015_CHANNEL_0),
      CONFIG_REGISTER_LSB));  // sets the starting channel to channel 0
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_LO_THRESH,
                                         ADS1015_LO_THRESH_REGISTER_MSB,
                                         ADS1015_LO_THRESH_REGISTER_LSB));
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_HI_THRESH,
                                         ADS1015_HI_THRESH_REGISTER_MSB,
                                         ADS1015_HI_THRESH_REGISTER_LSB));

  // ALERT/RDY continues to require a pullup resistor.
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,  //
  };
  return gpio_init_pin(&storage->ready_pin, &gpio_settings);
}

// This function enable/disables channels, and registers callbacks for each channel.
// It also registers the interrupt handler on ALRT/RDY pin.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context) {
  if (!storage) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // if no channels were already enabled, set to given channel
  if (!storage->channel_enable[0] && !storage->channel_enable[1] && !storage->channel_enable[2] &&
      !storage->channel_enable[3]) {
    status_ok_or_return(prv_set_channel(storage, channel));
    storage->current_channel = channel;
  }

  storage->channel_enable[channel] = enable;
  storage->channel_callback[channel] = callback;
  storage->callback_context[channel] = context;

  InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  return gpio_it_register_interrupt(&storage->ready_pin, &it_settings, INTERRUPT_EDGE_RISING,
                                    prv_interrupt_handler, storage);
}

// reads raw 12 bit conversion results which are expressed in two's complement format
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, uint16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  *reading = storage->channel_readings[channel];
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
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint16_t raw_12bit;
  status_ok_or_return(ads1015_read_raw(storage, channel, &raw_12bit));
  int16_t temp = prv_twos_comp_16bits(prv_sign_ext_16bits(12, raw_12bit));
  *reading = temp * ADS1015_FSR_4096 / (1 << 12);
  return STATUS_CODE_OK;
}
