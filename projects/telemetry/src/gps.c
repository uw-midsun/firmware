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
#include "can_transmit.h"

#include "log.h"

// Two structs to store data and settings. Since the GPS should only be initialized once
static GpsSettings *s_settings = NULL;
static GpsStorage *s_storage = NULL;

// Here's what we probably want to send through CAN:
// Time: (8 + 8 + 8 + 16 = 40 bits)
// MSL Altitude: (16 + 8 = 24 bits)

// Long + N/S: (16 + 8 + 16 + 8 = 48 bits)
// Satellites Used: (8 bits)

// Lat + E/W: (16 + 8 + 16 + 8 = 48 bits)
// Pos Fix Indicator: (8 bits)

// Speed Over Ground: (16 + 16 = 32 bits)
// HDOP: (16 + 16 = 32 bits)

// Time: hh, mm, ss, sss
// Long

  NmeaGgaSentence gga_result = { 0 };
  NmeaVtgSentence vtg_result = { 0 };
  
// This method will be called every time the GPS sends data.
static void prv_gps_callback(const uint8_t *rx_arr, size_t len, void *context) {
  NmeaMessageId messageId = NMEA_MESSAGE_ID_UNKNOWN;
  nmea_sentence_type((char *)rx_arr, &messageId);

  if (messageId == NMEA_MESSAGE_ID_GGA) {  // GGA message
    strncpy((char *)s_storage->gga_data, (char *)rx_arr, GPS_MAX_NMEA_LENGTH);
    
    nmea_get_gga_sentence(s_storage->gga_data, &gga_result);
    CAN_TRANSMIT_GPS_UTC_TIME(gga_result.time.hh, gga_result.time.mm, gga_result.time.ss, gga_result.time.sss); 
    LOG_DEBUG("sent\n");
  } else if (messageId == NMEA_MESSAGE_ID_VTG) {  // VTG message
    strncpy((char *)s_storage->vtg_data, (char *)rx_arr, GPS_MAX_NMEA_LENGTH);

    nmea_get_vtg_sentence(s_storage->vtg_data, &vtg_result);

    for(int i = 0; i < GPS_MAX_NMEA_LENGTH; i++) {
      printf("%c", s_storage->vtg_data[i]);
    }
  }

}

// Initialization of this chip is described on page 10 of:
// https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
StatusCode gps_init(GpsSettings *settings, GpsStorage *storage) {
    LOG_DEBUG("2\n"); 
  if (s_settings != NULL) {
    LOG_DEBUG("No Settings\n"); 
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Cannot reinitialize GPS\n");
  }
      LOG_DEBUG("1\n"); 

  // Initialize both structs
  s_settings = settings;
  s_storage = storage;

  GpioSettings telemetry_settings_gpio_general = {
    .direction = GPIO_DIR_OUT,  // The pin needs to output.
    .state = GPIO_STATE_LOW,    // Start in the "off" state.
    .alt_function = GPIO_ALTFN_1,
  };

  // Initializes UART callback
  uart_set_rx_handler(s_settings->uart_port, prv_gps_callback, NULL);

  // Initializes the pins
  StatusCode ret = gpio_init_pin(s_settings->pin_on_off, &telemetry_settings_gpio_general);

  if (!status_ok(ret)) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Error initializing GPIO Pins\n");
  }

  // Ensure that ON/OFF pulse happens at least 1s after power on
  delay_s(1);

  // Pull high on ON/OFF line
  ret |= gpio_set_state(s_settings->pin_on_off, GPIO_STATE_HIGH);
  delay_ms(100);

  // Set ON/OFF line to low
  ret |= gpio_set_state(s_settings->pin_on_off, GPIO_STATE_LOW);
  delay_ms(900);

  if (!status_ok(ret)) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Error turning on GPS Module\n");
  }

  // Turning off messages we don't need
  char *ggl_off = GPS_GLL_OFF;
  uart_tx(s_settings->uart_port, (uint8_t *)ggl_off, strlen(ggl_off));

  char *gsa_off = GPS_GSA_OFF;
  uart_tx(s_settings->uart_port, (uint8_t *)gsa_off, strlen(gsa_off));

  char *gsv_off = GPS_GSV_OFF;
  uart_tx(s_settings->uart_port, (uint8_t *)gsv_off, strlen(gsv_off));

  char *rmc_off = GPS_RMC_OFF;
  uart_tx(s_settings->uart_port, (uint8_t *)rmc_off, strlen(rmc_off));

  return STATUS_CODE_OK;
}

// Retrieve most recent GGA message
StatusCode gps_get_gga_data(uint8_t **result) {
  if (s_settings == NULL) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "GPS module is uninitialized.\n");
  }
  *result = s_storage->gga_data;
  return STATUS_CODE_OK;
}

// Retrieve most recent VTG message
StatusCode gps_get_vtg_data(uint8_t **result) {
  if (s_settings == NULL) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "GPS module is uninitialized.\n");
  }
  *result = s_storage->vtg_data;
  return STATUS_CODE_OK;
}
