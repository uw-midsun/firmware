#include "gps.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "nmea.h"
#include "status.h"
#include "uart.h"
#include "xbee.h"

// A large struct to store data and settings. Since the GPS should only be initialized once
static GpsSettings *s_settings = NULL;

static NmeaGgaSentence *s_gga_data = NULL;
static NmeaVtgSentence *s_vtg_data = NULL;

// This method will be called every time the GPS sends data.
static void prv_gps_callback(const uint8_t *rx_arr, size_t len, void *context) {
  NmeaMessageId messageId = NMEA_MESSAGE_ID_UNKNOWN;
  nmea_sentence_type((char *)&rx_arr, &messageId);
  if (messageId == NMEA_MESSAGE_ID_GGA) {
    nmea_get_gga_sentence((char *)&rx_arr, s_gga_data);
  } else if (messageId == NMEA_MESSAGE_ID_VTG) {
    nmea_get_vtg_sentence((char *)&rx_arr, s_vtg_data);
  }
}

// The GPS power line will be connected to a 3V pin. This method will
// control if that pin is providing power or not.
static void prv_gps_set_power_state(bool powered) {
  if (powered) {
    gpio_set_state(s_settings->pin_power, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(s_settings->pin_power, GPIO_STATE_LOW);
  }
}

// Stage 1 of initialization: pull high on ON/OFF line
static void prv_gps_init_stage_1() {
  gpio_set_state(s_settings->pin_on_off, GPIO_STATE_HIGH);
}

// Stage 2: set ON/OFF line to low
static void prv_gps_init_stage_2() {
  gpio_set_state(s_settings->pin_on_off, GPIO_STATE_LOW);
}

// Initialization of this chip is described on page 10 of:
// https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
StatusCode gps_init(GpsSettings *settings) {
  if (s_settings != NULL) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Cannot reinitialize GPS\n");
  }
  s_settings = settings;

  // Initializes UART
  StatusCode ret =
      uart_init(s_settings->port, s_settings->uart_settings, &s_settings->uart_storage);
  uart_set_rx_handler(s_settings->port, prv_gps_callback, NULL);

  // Initializes the pins
  status_ok_or_return(gpio_init_pin(s_settings->pin_power, s_settings->settings_power));
  status_ok_or_return(gpio_init_pin(s_settings->pin_on_off, s_settings->settings_on_off));

  prv_gps_set_power_state(true);
  delay_s(1);
  prv_gps_init_stage_1();
  delay_ms(100);
  prv_gps_init_stage_2();
  delay_ms(900);

  // Turning off messages we don't need
  char *ggl_off = GPS_GLL_OFF;
  uart_tx(s_settings->port, (uint8_t *)&ggl_off, strlen(ggl_off));

  char *gsa_off = GPS_GSA_OFF;
  uart_tx(s_settings->port, (uint8_t *)&gsa_off, strlen(gsa_off));

  char *gsv_off = GPS_GSV_OFF;
  uart_tx(s_settings->port, (uint8_t *)&gsv_off, strlen(gsv_off));

  char *rmc_off = GPS_RMC_OFF;
  uart_tx(s_settings->port, (uint8_t *)&rmc_off, strlen(rmc_off));

  return STATUS_CODE_OK;
}

StatusCode gps_get_gga_data(NmeaGgaSentence* result) {
  if (s_settings == NULL) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "GPS module is uninitialized.");
  }
  result = s_gga_data;
  return STATUS_CODE_OK;
}

StatusCode gps_get_vtg_data(NmeaVtgSentence* result) {
  if (s_settings == NULL) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "GPS module is uninitialized.");
  }
  result = s_vtg_data;
  return STATUS_CODE_OK;
}
