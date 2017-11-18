#include "gps.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gpio.h"
#include "misc.h"
#include "nmea.h"
#include "status.h"
#include "uart_mcu.h"

// How many handlers can we possibly need?
#define GPS_HANDLER_ARRAY_LENGTH 5

static const UARTPort port = UART_PORT_2;
static GPSHandler gps_handler[GPS_HANDLER_ARRAY_LENGTH] = { 0 };
static GGAHandler gga_handler[GPS_HANDLER_ARRAY_LENGTH] = { 0 };
static uint32_t init = 0;
static UARTStorage s_storage = { 0 };

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

StatusCode add_gps_handler(GPSHandler handler, size_t *index) {
  for (uint16_t i = 0; i < GPS_HANDLER_ARRAY_LENGTH; i++) {
    if (gps_handler[i] == NULL) {
      gps_handler[i] = handler;
      // This cast won't be an issue, there's no way "i" will be big enought to
      // cause an overflow
      if (index) {
        *index = i;
      }
      return STATUS_CODE_OK;
    }
  }
  return STATUS_CODE_UNKNOWN;
}

StatusCode add_gga_handler(GGAHandler handler, size_t *index) {
  for (uint16_t i = 0; i < GPS_HANDLER_ARRAY_LENGTH; i++) {
    if (gga_handler[i] == NULL) {
      gga_handler[i] = handler;
      // This cast won't be an issue, there's no way "i" will be big enought to
      // cause an overflow
      if (index) {
        *index = i;
      }
      return STATUS_CODE_OK;
    }
  }
  return STATUS_CODE_UNKNOWN;
}

StatusCode remove_gps_handler(size_t index) {
  if (index >= GPS_HANDLER_ARRAY_LENGTH) return STATUS_CODE_UNKNOWN;
  gps_handler[index] = NULL;
  return STATUS_CODE_OK;
}

StatusCode remove_gga_handler(size_t index) {
  if (index >= GPS_HANDLER_ARRAY_LENGTH) return STATUS_CODE_UNKNOWN;
  gga_handler[index] = NULL;
  return STATUS_CODE_OK;
}

StatusCode evm_gps_init(UARTSettings *uart_settings) {
  memset(&gps_handler, 0, SIZEOF_ARRAY(gps_handler));
  memset(&gga_handler, 0, SIZEOF_ARRAY(gga_handler));
  memset(&s_storage, 0, sizeof(s_storage));

  uart_settings->rx_handler = s_nmea_read;
  // Makes sure that status codes are handled
  status_ok_or_return(uart_init(port, uart_settings, &s_storage));

  uint8_t data[2] = { 42, 24 };

  status_ok_or_return(uart_tx(port, data, SIZEOF_ARRAY(data)));
  init = 1;
  return STATUS_CODE_OK;
}
