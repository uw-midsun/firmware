#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"

static const UARTPort port = UART_PORT_1;

static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context){

}

static UARTSettings s_settings = {
  .baudrate = 9600,//
  .rx_handler = s_nmea_read,

  .tx = { .port = GPIO_PORT_A , .pin = 1 },
  .rx = { .port = GPIO_PORT_A , .pin = 1 },
  .alt_fn = GPIO_ALTFN_NONE,
};

static UARTStorage s_storage = { 0 };

StatusCode evm_gps_init(void) {
  uart_init(port, s_settings, s_storage)
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
