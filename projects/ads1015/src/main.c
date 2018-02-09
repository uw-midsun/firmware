#include <stdint.h>
#include <stdio.h>
#include "ads1015.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"

int main() {
  GPIOAddress ready_pin = { GPIO_PORT_B, 2 };

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST, .scl = { GPIO_PORT_B, 10 }, .sda = { GPIO_PORT_B, 11 }
  };
  gpio_init();
  interrupt_init();
  gpio_it_init();
  ADS1015Data data;
  i2c_init(I2C_PORT_2, &i2c_settings);
  void f1(const GPIOAddress *address, void *context) {
    ADS1015Data *data = context;
    printf("converted from channel %d\n", data->current_channel);
  }
  ads1015_init(&data, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_0, true, f1, &data);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_1, true, f1, &data);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_2, false, f1, &data);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_3, false, f1, &data);
  int16_t adc_data[NUM_ADS1015_CHANNELS];
  while (1) {
    for (uint8_t i = 0; i < NUM_ADS1015_CHANNELS; i++) {
      ads1015_read_converted(&data, i, &adc_data[i]);
    }
    printf("[ %d\t%d\t%d\t%d ]\n", adc_data[ADS1015_CHANNEL_0], adc_data[ADS1015_CHANNEL_1],
           adc_data[ADS1015_CHANNEL_2], adc_data[ADS1015_CHANNEL_3]);
  }
}
