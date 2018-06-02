
// int16_t percentage_converter(int16_t reading_in_lsb, int16_t zero_value, int16_t hundred_value) {
//   int16_t percentage;
//   if (zero_value > hundred_value) {
//     percentage = ((100 * (reading_in_lsb - hundred_value)) / (hundred_value - zero_value)) + 100;
//   } else {
//     percentage = (100 * (reading_in_lsb - zero_value)) / (hundred_value - zero_value);
//   }

//   if (percentage < 0) {
//     percentage = 0;
//   } else if (percentage > 100) {
//     percentage = 100;
//   }

//   event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage);

//   return percentage;
// }

// static void prv_callback_channel(Ads1015Channel channel, void *context) {
//   Ads1015Storage *storage = context;
//   int16_t reading = 0;
//   ads1015_read_raw(storage, channel, &reading);
//   int16_t percentage = percentage_converter(reading, 1253, 418);
//   printf("%d %d\n", reading, percentage);
// }

// int main(void) {
//   Ads1015Storage storage;

//   gpio_init();
//   interrupt_init();
//   gpio_it_init();
//   soft_timer_init();
//   I2CSettings i2c_settings = {
//     .speed = I2C_SPEED_FAST,
//     .scl = { .port = GPIO_PORT_B, .pin = 10 },
//     .sda = { .port = GPIO_PORT_B, .pin = 11 },
//   };
//   i2c_init(I2C_PORT_2, &i2c_settings);
//   GPIOAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };

//   ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

//   ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback_channel, &storage);

//   while (true) {
//   }

//   return 0;
// }
#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "magnetic_brake_event_generator.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"

static MagneticCalibrationData data;

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  MagneticCalibrationData *data = context;
  // ads1015_read_raw(data->storage, channel, data->reading);
}

Ads1015Storage storage;

gpio_init();
interrupt_init();
gpio_it_init();
soft_timer_init();

I2CSettings i2c_settings = {
  .speed = I2C_SPEED_FAST,
  .scl = { .port = GPIO_PORT_B, .pin = 10 },
  .sda = { .port = GPIO_PORT_B, .pin = 11 },
};

i2c_init(I2C_PORT_2, &i2c_settings);
GPIOAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };

ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback_channel, &data);

data.reading = 30000;

MagneticBrakeSettings brake_settings = {
  .percentage_threshold = 60000,
  .zero_value = 418,
  .hundred_value = 1253,
  .min_allowed_range = 0,
  .max_allowed_range = (1 << 12),
};

magnetic_brake_event_generator_init(&data, &brake_settings);

percentage_converter(&data, &brake_settings);

printf("%d %d\n", data.reading, data.percentage);

while (true) {
}
