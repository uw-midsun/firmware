#pragma once

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
} ADS1015Address;

typedef enum {
  ADS1015_CHANNEL_0 = 0,
  ADS1015_CHANNEL_1,
  ADS1015_CHANNEL_2,
  ADS1015_CHANNEL_3,
  NUM_ADS1015_CHANNELS,
} ADS1015Channel;

typedef void (*ADS1015Callback)(const GPIOAddress *address, void *context);

//struct ADS1015Data;
typedef struct ADS1015Data {
  I2CPort i2c_port;
  ADS1015Address i2c_addr;
  GPIOAddress *ready_pin;
  uint8_t channel_readings[NUM_ADS1015_CHANNELS][2];
  ADS1015Channel current_channel;
  bool channel_enable[NUM_ADS1015_CHANNELS];
  ADS1015Callback channel_callback[NUM_ADS1015_CHANNELS];
  void *callback_context[NUM_ADS1015_CHANNELS];
} ADS1015Data;

StatusCode ads1015_init(ADS1015Data *data, I2CPort i2c_port, ADS1015Address i2c_addr,
                        GPIOAddress *ready_pin);

StatusCode ads1015_configure_channel(ADS1015Data *data, ADS1015Channel channel, bool enable,
                                     ADS1015Callback callback, void *context);

StatusCode ads1015_read_raw(ADS1015Data *data, ADS1015Channel channel, uint16_t *reading);

StatusCode ads1015_read_converted(ADS1015Data *data, ADS1015Channel channel, int16_t *reading);
