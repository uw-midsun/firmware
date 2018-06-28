#include "driver_display_config.h"

static const UARTSettings uart_settings = {
  .baudrate = DRIVER_DISPLAY_CONFIG_UART_BAUDRATE,  //
  .tx = DRIVER_DISPLAY_CONFIG_UART_TX,              //
  .rx = DRIVER_DISPLAY_CONFIG_UART_RX,              //
  .alt_fn = DRIVER_DISPLAY_CONFIG_UART_ALTFN        //
};

static const CANHwSettings can_hw_settings = {
  .tx = { .port = GPIO_PORT_A, .pin = 12 },  //
  .rx = { .port = GPIO_PORT_A, .pin = 11 },  //
  .bitrate = DRIVER_DISPLAY_CONFIG_CAN_BITRATE,          //
  .loopback = false,                         //
};

static const CanUart can_uart = {
  .uart = DRIVER_DISPLAY_CONFIG_UART_PORT,  //
  .rx_cb = NULL,                // Ignore RX'd messages from the master
  .context = NULL               //
};

UARTSettings* driver_display_config_load_uart(void){
  return &uart_settings;
}

CANHwSettings* driver_display_config_load_can(void){
  return &can_hw_settings;
}

CanUart* driver_display_config_load_can_uart(void){
  return &can_uart;
}
