#pragma once
// Module for using ADS1015.
//  I2C, GPIO, Interrupt, and Soft Timers should be initialized.
// The user also needs to create a struct ADS1015Storage which would persist accross functions.
// Start the ads1015 using ads1015_init and then configure channels using ads1015_config_channel.
// Read raw and converted values using ads1015_read_raw and ads1015_read_converted.
//
// Uses a soft timer as a watchdog to detect if
#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
#include "status.h"
#include "soft_timer.h"

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

// The callback runs after each conversion from the channel.
typedef void (*Ads1015Callback)(Ads1015Channel channel, void *context);

typedef struct Ads1015Storage {
  I2CPort i2c_port;
  uint8_t i2c_addr;
  GPIOAddress ready_pin;
  int16_t channel_readings[NUM_ADS1015_CHANNELS];
  Ads1015Channel current_channel;
  uint8_t channel_bitset;
  uint8_t pending_channel_bitset;
  Ads1015Callback channel_callback[NUM_ADS1015_CHANNELS];
  void *callback_context[NUM_ADS1015_CHANNELS];

  SoftTimerID watchdog_timer;
  bool had_interrupt;
} Ads1015Storage;

// Initiates ads1015 by setting up registers and enabling ALRT/RDY Pin.
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GPIOAddress *ready_pin);

// Enable/disables a channel, and registers a callback on the channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context);

// Reads raw 12 bit conversion results which are expressed in two's complement format.
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, int16_t *reading);

// Reads conversion value in mVolt.
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading);
