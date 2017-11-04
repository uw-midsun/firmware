#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"
#include "misc.h"
#include "nmea.h"

// How many handlers can we possibly need?
#define GPS_HANDLER_ARRAY_LENGTH 5

static const UARTPort port = UART_PORT_2;
static GPSHandler gps_handler[GPS_HANDLER_ARRAY_LENGTH] = {0};
static GGAHandler gga_handler[GPS_HANDLER_ARRAY_LENGTH] = {0};
static uint32_t init = 0;

int32_t add_gps_handler(GPSHandler handler) {
  for (uint32_t i = 0; i < GPS_HANDLER_ARRAY_LENGTH; i++) {
    if (gps_handler[i] == NULL) {
      gps_handler[i] = handler;
      // This cast won't be an issue, there's no way "i" will be big enought to cause an overflow
      return (int32_t) i;
    }
  }
  return -1;
}

int32_t add_gga_handler(GGAHandler handler) {
  for (uint32_t i = 0; i < GPS_HANDLER_ARRAY_LENGTH; i++) {
    if (gga_handler[i] == NULL) {
      gga_handler[i] = handler;
      // This cast won't be an issue, there's no way "i" will be big enought to cause an overflow
      return (int32_t) i;
    }
  }
  return -1;
}

void remove_gps_handler(uint32_t index) {
  if (index >= GPS_HANDLER_ARRAY_LENGTH) return;
  gps_handler[index] = NULL;
}

void remove_gga_handler(uint32_t index) {
  if (index >= GPS_HANDLER_ARRAY_LENGTH) return;
  gga_handler[index] = NULL;
}

static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {
  if (init == 0) return;
  // Check that the context is correct
  NMEAResult r = parse_nmea_sentence(rx_arr, len);

  for (uint32_t i = 0; i < GPS_HANDLER_ARRAY_LENGTH; i++) {
    if (gps_handler[i] != NULL) {
      (*gps_handler[i])(r);
    }
    if (gga_handler[i] != NULL && r.gga.message_id == GGA) {
      (*gga_handler[i])(r.gga);
    }
  }
}

static UARTStorage s_storage = { 0 };

StatusCode evm_gps_init(UARTSettings* uart_settings) {
  uart_settings->rx_handler = s_nmea_read;
  // Makes sure that status codes are handled
  status_ok_or_return(uart_init(port, uart_settings, &s_storage));

  uint8_t data[2] = { 42, 24 };

  status_ok_or_return(uart_tx(port, data, SIZEOF_ARRAY(data)));
  init = 1;
  return STATUS_CODE_OK;
}
