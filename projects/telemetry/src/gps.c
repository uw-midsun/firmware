#include "status.h"
#include "gps.h"
#include "gpio.h"
// This is the address used to send messages to the gps chip
static GPIOAddress s_tx = {
  .port = 5;//
  .pin = 1;//Todo: update numbers
};

// This is the address used to recieve messages to the gps chip
static GPIOAddress s_rx = {
  .port = 4;//
  .pin = 1;//Todo: update numbers
};
static UARTSettings s_read_from_chip = {
  .baudrate = 9600,//
  .rx_handler = s_nmea_read,
  .tx = s_tx,
  .rx = s_rx,
  .alt_fn = GPIO_ALTFN_NONE,
};
static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context){
  
}
StatusCode evm_gps_init(void) {
  return STATUS_CODE_UNIMPLEMENTED;
  // uart_init(UARTPort uart, s_read_from_chip, UARTStorage *storage)
}
// uart_tx(UARTPort uart, uint8_t *tx_data, size_t len);