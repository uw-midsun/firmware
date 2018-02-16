#include "ads1015.h"
// This module provides the functions for initiating ADS1015, and configuring
// and reading from its channels. The general idea is that the ALERT/RDY pin
// asserts whenever a conversion result is ready, and after storing the result
// the channel is switched to the next and ADS1015 is restarted for the new conversion.
// If no channels are enabled, the interrupt on ALERT/RDY pin is masked.
#include <status.h>
#include <stdio.h>
#include <string.h>
#include "ads1015_def.h"
#include "gpio_it.h"

// Updates the channel_enable_bitset when a channel is enabled/disabled.
static void prv_mark_channel_enabled(Ads1015Channel channel, bool enable,
                                     uint8_t *channel_enable_bitset) {
  if (enable) {
    *channel_enable_bitset |= 1 << channel;
  } else {
    *channel_enable_bitset &= ~(1 << channel);
  }
}

// Writes to register given upper and lower bytes.
static StatusCode prv_setup_register(Ads1015Storage *storage, uint8_t reg, uint8_t msb,
                                     uint8_t lsb) {
  uint8_t ads1015_setup_register[] = { reg, msb, lsb };
  return i2c_write(storage->i2c_port, storage->i2c_addr, ads1015_setup_register,
                   SIZEOF_ARRAY(ads1015_setup_register));
}

// Reads the register and stores the value in the given array.
static StatusCode prv_read_register(I2CPort i2c_port, uint8_t i2c_addr, uint8_t reg,
                                    uint8_t *rx_data, size_t rx_len) {
  status_ok_or_return(i2c_write(i2c_port, i2c_addr, &reg, sizeof(reg)));

  status_ok_or_return(i2c_read(i2c_port, i2c_addr, rx_data, rx_len));
  return STATUS_CODE_OK;
}

// Switches to the given channel by writing to config register.
static StatusCode prv_set_channel(Ads1015Storage *storage, Ads1015Channel channel) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_CONFIG,
                                         ADS1015_CONFIG_REGISTER_MSB(channel),
                                         ADS1015_CONFIG_REGISTER_LSB));
  storage->current_channel = channel;
  return STATUS_CODE_OK;
}

// This function is registered as the callback for ALRT/RDY Pin.
// Reads and stores the conversion value in storage, and switches to the next enabled channel.
// Also if there is a callback on a channel, it will be run here.
static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
  Ads1015Storage *storage = context;
  Ads1015Channel current_channel = storage->current_channel;
  uint8_t channel_enable_bitset = storage->channel_enable_bitset;
  uint8_t read_conv_register[2] = { 0, 0 };

  prv_read_register(storage->i2c_port, storage->i2c_addr, ADS1015_ADDRESS_POINTER_CONV,
                    read_conv_register, SIZEOF_ARRAY(read_conv_register));
  // Following line puts the two read bytes into an int16.
  // 4 LSB's are not part of the result hence the bitshift.
  storage->channel_readings[current_channel] =
      ((read_conv_register[0] << 8) | read_conv_register[1]) >> ADS1015_NUM_RESERVED_BITS_CONV_REG;

  // Runs the users callback if not NULL.
  if (storage->channel_callback[current_channel] != NULL) {
    storage->channel_callback[current_channel](current_channel,
                                               storage->callback_context[current_channel]);
  }

  prv_mark_channel_enabled(current_channel, false, &storage->channel_enable_bitset_helper);
  if (storage->channel_enable_bitset_helper == ADS1015_BITSET_EMPTY) {
    storage->channel_enable_bitset_helper = channel_enable_bitset;
  }
  current_channel = __builtin_ffs(storage->channel_enable_bitset_helper) - 1;
  // Update so that the ADS1015 reads from the next channel.
  prv_set_channel(storage, current_channel);
}

// Initiates ads1015 by setting up registers and enabling ALRT/RDY Pin.
// It also registers the interrupt handler on ALRT/RDY pin.
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  if ((storage == NULL) || (ready_pin == NULL)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));
  for (uint8_t i = 0; i < NUM_ADS1015_CHANNELS; i++) {
    storage->channel_readings[i] = ADS1015_DISABLED_CHANNEL_READING;
  }
  storage->i2c_port = i2c_port;
  storage->i2c_addr = i2c_addr + ADS1015_I2C_BASE_ADDRESS;
  storage->ready_pin = *ready_pin;
  // Set up config register.
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_CONFIG,
                                         ADS1015_CONFIG_REGISTER_MSB(ADS1015_CHANNEL_0),
                                         ADS1015_CONFIG_REGISTER_LSB));
  // Set up hi/lo-thresh registers. This particular setup enables the ALRT/RDY pin.
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_LO_THRESH,
                                         ADS1015_LO_THRESH_REGISTER_MSB,
                                         ADS1015_LO_THRESH_REGISTER_LSB));
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_HI_THRESH,
                                         ADS1015_HI_THRESH_REGISTER_MSB,
                                         ADS1015_HI_THRESH_REGISTER_LSB));
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,  //
  };
  InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  status_ok_or_return(gpio_init_pin(ready_pin, &gpio_settings));
  status_ok_or_return(gpio_it_register_interrupt(ready_pin, &it_settings, INTERRUPT_EDGE_RISING,
                                                 prv_interrupt_handler, storage));
  // Mask the interrupt until channels are enabled by the user.
  return gpio_it_mask_interrupt(ready_pin, true);
}

// This function enable/disables channels, and registers callbacks for each channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context) {
  if (storage == NULL || channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (storage->channel_enable_bitset == ADS1015_BITSET_EMPTY && enable) {
    // Activate the interrupt since the first channel is being enabled.
    status_ok_or_return(gpio_it_mask_interrupt(&storage->ready_pin, false));
    status_ok_or_return(prv_set_channel(storage, channel));
  } else if (!enable) {
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }

  prv_mark_channel_enabled(channel, enable, &storage->channel_enable_bitset);
  if (storage->channel_enable_bitset != ADS1015_BITSET_EMPTY) {
    storage->current_channel = __builtin_ffs(storage->channel_enable_bitset) - 1;
  } else {
    // Mask interrupt if all channels are disabled.
    status_ok_or_return(gpio_it_mask_interrupt(&storage->ready_pin, true));
  }
  storage->channel_enable_bitset_helper = storage->channel_enable_bitset;
  storage->channel_callback[channel] = callback;
  storage->callback_context[channel] = context;
  return STATUS_CODE_OK;
}

// Reads raw 12 bit conversion results which are expressed in two's complement format.
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  *reading = storage->channel_readings[channel];
  return STATUS_CODE_OK;
}

// Reads conversion value in mVolt.
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  int16_t raw_reading = ADS1015_READ_UNSUCCESSFUL;
  status_ok_or_return(ads1015_read_raw(storage, channel, &raw_reading));
  *reading = (raw_reading * ADS1015_FSR_4096) / ADS1015_LSB_SIZE_FACTOR;
  return STATUS_CODE_OK;
}
