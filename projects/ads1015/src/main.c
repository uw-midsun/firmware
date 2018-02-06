#include <ads1015_def.h>
#include <stdint.h>
#include <stdio.h>

#include "ads1015.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
static StatusCode prv_read(I2CPort i2c_port, ADS1015Address i2c_addr, uint8_t reg, uint8_t *read) {
  status_ok_or_return(i2c_write(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, &reg, 1));
  status_ok_or_return(i2c_read(i2c_port, i2c_addr | ADS1015_I2C_ADDRESS_GND, read, 2));
  return STATUS_CODE_OK;
}

int convert(int dec) {
  if (dec == 0) {
    return 0;
  } else {
    return (dec % 2 + 10 * convert(dec / 2));
  }
}

void printmine(ADS1015Data *data){
  printf("channel readings:\n %d %d channel0\n %d %d channel1\n %d %d channel2\n %d %d channel3\n",
         data->channel_readings[0][0], data->channel_readings[0][1], 
         data->channel_readings[1][0], data->channel_readings[1][1],
         data->channel_readings[2][0], data->channel_readings[2][1],
         data->channel_readings[3][0], data->channel_readings[3][1] );
  printf("enabled channels: %d %d %d %d\n", data->channel_enable[0], data->channel_enable[1],
         data->channel_enable[2], data->channel_enable[3]);     
  printf("current channel: %d \n", data->current_channel);
}

int main() {
  GPIOAddress ready_pin = { GPIO_PORT_B, 2 };

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST, .scl = { GPIO_PORT_B, 10 }, .sda = { GPIO_PORT_B, 11 }
  };
  uint8_t ads1015_setup_config[] = { ADS1015_ADDRESS_POINTER_CONFIG, CONFIG_REGISTER_MSB_0,
                                     CONFIG_REGISTER_LSB };
  uint8_t write_config = ADS1015_ADDRESS_POINTER_CONFIG;
  uint8_t write_conv = ADS1015_ADDRESS_POINTER_CONV;
  uint8_t write_lo = ADS1015_ADDRESS_POINTER_LO_THRESH;
  uint8_t write_hi = ADS1015_ADDRESS_POINTER_HI_THRESH;

  gpio_init();
  interrupt_init();
  gpio_it_init();
  ADS1015Data data;

  i2c_init(I2C_PORT_2, &i2c_settings);
  
  ads1015_init(&data, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_0, true, NULL, NULL);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_1, true, NULL, NULL);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_2, true, NULL, NULL);
  ads1015_configure_channel(&data, ADS1015_CHANNEL_3, true, NULL, NULL);
 
  int16_t adc_data[NUM_ADS1015_CHANNELS];
  uint8_t x[2];
  uint8_t y[2];
  while (1) {
    for (uint8_t i = 0; i < NUM_ADS1015_CHANNELS; i++) {
      ads1015_read_raw(&data, i, &adc_data[i]);
    }
     printf("[ %d\t%d\t%d\t%d ]\n", adc_data[ADS1015_CHANNEL_0], adc_data[ADS1015_CHANNEL_1],
         adc_data[ADS1015_CHANNEL_2], adc_data[ADS1015_CHANNEL_3]);
     printmine(&data);
     prv_read(data.i2c_port, data.i2c_addr, write_conv, x);
     printf("conversionreg: %d - %d \n", convert(x[0]), convert(x[1]));
     prv_read(I2C_PORT_2, data.i2c_addr, write_config, y);
     printf("config    reg: %d - %d\n", convert(y[0]), convert(y[1]));
     prv_read(I2C_PORT_2, data.i2c_addr, write_lo, y);
     printf("lo    reg: %d - %d\n", convert(y[0]), convert(y[1]));
     prv_read(I2C_PORT_2, data.i2c_addr, write_hi, y);
     printf("hi    reg: %d - %d\n", convert(y[0]), convert(y[1]));
     /*
    i2c_write(I2C_PORT_2, ADS1015_ADDRESS_GND | ADS1015_I2C_ADDRESS_GND, ads1015_setup_config, 3);
    



   uint8_t reset = ADS1015_RESET_BYTE;
    status_ok_or_return(i2c_write(I2C_PORT_2, ADS1015_I2C_GENERAL_CALL, &reset, 1));
    i2c_write(I2C_PORT_2, ADS1015_ADDRESS_GND | ADS1015_I2C_ADDRESS_GND, &write_config, 1);
   // i2c_read(I2C_PORT_2, ADS1015_I2C_ADDRESS_GND, x, 2);
    i2c_read(I2C_PORT_2, ADS1015_ADDRESS_GND | ADS1015_I2C_ADDRESS_GND,
               x, 2);
    
    printmine(&data);
    //i2c_read(I2C_PORT_2, ADS1015_ADDRESS_GND, &x[0], 1);
   // printf("%d thisone \n", convert(x[0]));
    


  */
  }
}