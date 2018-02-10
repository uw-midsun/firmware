#pragma once
// Module for using ads1015.
// GPIO and Interrupt, I2C should be initiated.
// The user also needs to create a struct ADS1015Data which would persist accross functions.
// Start the ads1015 using ads1015_init and then configure channels using ads1015_config_channel.
// Read raw and converted values using ads1015_read_raw and ads1015_read_converted.
#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "status.h"

typedef enum {
  ADS1015_ADDRESS_GND = 0,
  ADS1015_ADDRESS_VDD,
  ADS1015_ADDRESS_SDA,
  ADS1015_ADDRESS_SCL,
  NUM_ADS1015_ADDRESSES,
} Ads1015Address;

typedef enum {
  ADS1015_CHANNEL_0 = 0,
  ADS1015_CHANNEL_1,
  ADS1015_CHANNEL_2,
  ADS1015_CHANNEL_3,
  NUM_ADS1015_CHANNELS,
} Ads1015Channel;

typedef void (*Ads1015Callback)(Ads1015Channel channel, void *context);

typedef struct Ads1015Storage {
  I2CPort i2c_port;
  Ads1015Address i2c_addr;
  GPIOAddress ready_pin;
  uint16_t channel_readings[NUM_ADS1015_CHANNELS];
  Ads1015Channel current_channel;
  bool channel_enable[NUM_ADS1015_CHANNELS];
  Ads1015Callback channel_callback[NUM_ADS1015_CHANNELS];
  void *callback_context[NUM_ADS1015_CHANNELS];
} Ads1015Storage;

StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GPIOAddress *ready_pin);

StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context);

StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, uint16_t *reading);

StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading);
