#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"

// This is the address used to send messages to the gps chip
static GPIOAddress s_tx_address = {
  .port = 5;//
  .pin = 1;//Todo: update numbers
};

static const UARTPort port = UART_PORT_1;

// This is the address used to recieve messages to the gps chip
static GPIOAddress s_rx_address = {
  .port = 4;//
  .pin = 1;//Todo: update numbers
};

static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context){

}

static UARTSettings s_settings = {
  .baudrate = 9600,//
  .rx_handler = s_nmea_read,

  .tx = s_tx_address,
  .rx = s_rx_address,
  .alt_fn = GPIO_ALTFN_NONE,
};

static UARTStorage s_storage = { 0 };

StatusCode evm_gps_init(void) {
  uart_init(port, s_settings, s_storage)
  return STATUS_CODE_UNIMPLEMENTED;
}
