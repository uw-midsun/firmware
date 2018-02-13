#include "ads1015.h"
// All the important data about ADS1015 is stored in ADS1015Storage storage by different functions.
// Ads1015_init writes the desired settings to registers and inits the GPIO pin (ALRT/RDY). (This
// pin is asserted when a conversion result is ready). Ads1015_init registers prv_interrupt_handler
// as the callback for this pin. Once the interrupt is raised, it means that a conversion has been
// completed. prv_interrupt_handler reads the conversion values from the registers and stores them
// in storage, and then switches the channel to the next enabled one.
// The user could access conversion data by the provided read functions. These
// functions get their data from storage and not directly by reading the registers. Also look at
// ads1015_def for more information on macros and how registers are setup.
// Reading and writing to registers is done through I2C interface. Look at section 8.5.3 of the
// datasheet for the timing diagrams.
#include <status.h>
#include <stdio.h>
#include <string.h>
#include "ads1015_def.h"
#include "gpio_it.h"

// Writes to register given upper and lower bytes.
static StatusCode prv_setup_register(Ads1015Storage *storage, uint8_t reg, uint8_t msb,
                                     uint8_t lsb) {
  uint8_t ads1015_setup_register[] = { reg, msb, lsb };
  return i2c_write(storage->i2c_port, storage->i2c_addr + ADS1015_I2C_BASE_ADDRESS,
                   ads1015_setup_register, SIZEOF_ARRAY(ads1015_setup_register));
}

// Reads the register and stores the value in the given array.
static StatusCode prv_read_register(I2CPort i2c_port, Ads1015Address i2c_addr, uint8_t reg,
                                    uint8_t *rx_data, size_t rx_len) {
  status_ok_or_return(i2c_write(i2c_port, i2c_addr + ADS1015_I2C_BASE_ADDRESS, &reg, sizeof(reg)));

  status_ok_or_return(i2c_read(i2c_port, i2c_addr + ADS1015_I2C_BASE_ADDRESS, rx_data, rx_len));
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
  storage->channel_enable[channel] = true;
  return STATUS_CODE_OK;
}

// This function is registered as the callback for ALRT/RDY Pin.
// Reads and stores the conversion value in storage, and switches to the next enabled channel.
static void prv_interrupt_handler(const GPIOAddress *address, void *context) {
  Ads1015Storage *storage = context;
  Ads1015Channel current_channel = storage->current_channel;
  // The following if statement prevents recording of data for a channel if never enabled.
  // In particular the very first time inside ads1015_init a certain channel(0) is set.
  // However the user has to use ads1015_configure_channel to enable that channel.
  if (storage->channel_enable[current_channel]) {
    uint8_t read_conv_register[2];
    prv_read_register(storage->i2c_port, storage->i2c_addr, ADS1015_ADDRESS_POINTER_CONV,
                      read_conv_register, SIZEOF_ARRAY(read_conv_register));

    // Following line puts the two read bytes into an int16.
    // 4 LSB's are not part of the result hence the bitshift.
    storage->channel_readings[current_channel] =
        ((read_conv_register[0] << 8) | read_conv_register[1]) >>
        ADS1015_NUM_RESERVED_BITS_CONV_REG;
    // Runs the users callback if not NULL.
    if (storage->channel_callback[current_channel] != NULL) {
      storage->channel_callback[current_channel](storage->current_channel,
                                                 storage->callback_context[current_channel]);
    }
    // Update so that the ADS1015 reads from the next channel.
    do {
      current_channel = (current_channel + 1) % NUM_ADS1015_CHANNELS;
    } while (!storage->channel_enable[current_channel]);
    prv_set_channel(storage, current_channel);
  } else {
    storage->channel_readings[current_channel] = ADS1015_DISABLED_CHANNEL_READING;
  }
}

// Initiates ads1015 by setting up registers and enabling ALRT/RDY Pin.
// It also registers the interrupt handler on ALRT/RDY pin.
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  if ((storage == NULL) || (ready_pin == NULL)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));
  storage->i2c_port = i2c_port;
  storage->i2c_addr = i2c_addr;
  storage->ready_pin = *ready_pin;
  // Set up config register.
  status_ok_or_return(prv_setup_register(storage, ADS1015_ADDRESS_POINTER_CONFIG,
                                         ADS1015_CONFIG_REGISTER_MSB(ADS1015_CHANNEL_0),
                                         ADS1015_CONFIG_REGISTER_LSB));
  // Set up hi/lo-thresh registers. This setup enables the ALRT/RDY pin.
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
  status_ok_or_return(gpio_init_pin(&storage->ready_pin, &gpio_settings));
  return gpio_it_register_interrupt(&storage->ready_pin, &it_settings, INTERRUPT_EDGE_RISING,
                                    prv_interrupt_handler, storage);
}

// This function enable/disables channels, and registers callbacks for each channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context) {
  if (storage == NULL || channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->channel_enable[channel] = enable;
  storage->channel_callback[channel] = callback;
  storage->callback_context[channel] = context;
  if (!enable) {
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }
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
  status_ok_or_return(ads1015_read_raw(storage, channel, reading));
  *reading = (*reading) * ADS1015_LSB_SIZE(ADS1015_FSR_4096);
  return STATUS_CODE_OK;
}
