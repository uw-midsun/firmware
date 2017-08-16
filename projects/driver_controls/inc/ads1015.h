#pragma once

#include "gpio.h"
#include "i2c.h"
#include "status.h"

// Driver for the ADS1015 ADC

typedef enum {
  ADS1015_CHANNEL_0,
  ADS1015_CHANNEL_1,
  ADS1015_CHANNEL_2,
  ADS1015_CHANNEL_3,
  NUM_ADS1015_CHANNELS
} ADS1015Channel;

typedef void (*ADS1015Callback)(ADS1015Channel channel, void *context);

// Start up the device with an initialized I2C port
StatusCode ads1015_init(I2CPort i2c_port, GPIOAddress address);

// Register a callback function to be called when the specified channel completes a conversion
StatusCode ads1015_register_callback(ADS1015Channel channel,
                                      ADS1015Callback callback, void *context);

// Obtain the raw 12-bit value read by the specified channel
StatusCode ads1015_read_raw(ADS1015Channel channel, uint16_t *reading);

// Obtain the converted value at the specified channel
StatusCode ads1015_read_converted(ADS1015Channel channel, uint16_t *reading);
