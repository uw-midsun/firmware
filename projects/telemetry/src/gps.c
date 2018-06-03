#include "gps.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gpio.h"
#include "interrupt.h"  // For enabling interrupts.
#include "log.h"
#include "misc.h"
#include "nmea.h"
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "status.h"
#include "uart_mcu.h"

// Static variables for GPS chip utilities below

static UARTStorage s_storage;

// If set to false, the driver assumes that the GPS will send data by default once
// powered on. If set to true, the GPS driver will perform initialization after
// power on.
static bool s_evm_initialize_by_gpio = false;

// Just some constants so that the max length of raw data can be set
#define s_max_gga_length 80

// Just some constants to get the actual length
static size_t s_gga_length = 0;
static size_t s_vtg_length = 0;

// Stores raw NMEA messages sent by the chip
static volatile uint8_t s_gga_data[s_max_gga_length];

static evm_gps_gga_sentence s_gga_sentence;

// Methods for GPS utilities

static void prv_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {
  EVM_GPS_NMEA_MESSAGE_ID message_id = EVM_GPS_UNKNOWN;
  StatusCode ret = evm_gps_get_nmea_sentence_type(rx_arr, len, &message_id);

  if (message_id == EVM_GPS_GGA) {
    if (len < s_max_gga_length) {
      strncpy(s_gga_data, (char *)rx_arr, len);
      s_gga_data[len] = 0;
      s_gga_length = len;
    }
  } else if (message_id == EVM_GPS_VTG) {
    if (len < s_max_vtg_length) {
      strncpy(s_vtg_data, (char *)rx_arr, len);
      s_vtg_data[len] = 0;
      s_vtg_length = len;
    }
  }
}

// This is useful as a method for callbacks
static void prv_chip_power_state_on(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;
  gpio_set_state(settings->pin_power, GPIO_STATE_HIGH);
}

static void prv_chip_power_state_off(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;
  gpio_set_state(settings->pin_power, GPIO_STATE_LOW);
}

// Stage 2: set ON/OFF line to low
static void prv_gps_init_stage_2(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;
  gpio_set_state(settings->pin_on_off, GPIO_STATE_LOW);

  // The GPS chip should be on now
}

// Stage 1 of initialization: pull high on ON/OFF line
static void prv_gps_init_stage_1(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;

  gpio_set_state(settings->pin_on_off, GPIO_STATE_HIGH);
  soft_timer_start_millis(100, prv_gps_init_stage_2, settings, NULL);
}

StatusCode evm_gps_validate_settings(evm_gps_settings *settings) {
  // Making sure all settings as passed in
  // Maybe this should be made into its own method?
  if (!settings) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "The 'settings' argument is null\n");
  }
  if (!settings->uart_settings) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "The 'settings->uart_settings' argument is null\n");
  }
  if (!settings->settings_power) {
    return status_msg(STATUS_CODE_INVALID_ARGS,
                      "The 'settings->settings_power' argument is null\n");
  }
  if (!settings->pin_power) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "The 'settings->pin_power' argument is null\n");
  }
  if (!settings->pin_on_off) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "The 'settings->pin_on_off' argument is null\n");
  }
  return STATUS_CODE_OK;
}

// Initialization of this chip is described on page 10 of:
// https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
StatusCode evm_gps_init(evm_gps_settings *settings) {
  // Validates settings
  status_ok_or_return(evm_gps_validate_settings(settings));

  // Initializes UART
  settings->uart_settings->rx_handler = &prv_nmea_read;
  StatusCode ret = uart_init(settings->port, settings->uart_settings, &s_storage);

  // Initializes the power pin
  status_ok_or_return(gpio_init_pin(settings->pin_power, settings->settings_power));

  // Provides power to the GPS
  prv_chip_power_state_on(0, settings);

  // Initializes the GPS by pulling the ON OFF pin to high for 100 ms
  if (s_evm_initialize_by_gpio) {
    status_ok_or_return(gpio_init_pin(settings->pin_on_off, settings->settings_on_off));
    status_ok_or_return(soft_timer_start_seconds(1, prv_gps_init_stage_1, settings, NULL));
  }

  return STATUS_CODE_OK;
}

// Implementing shut down here:
// From page 25 of:
// https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
StatusCode evm_gps_clean_up(evm_gps_settings *settings) {
  // The char array below should read $PSRF117,16*0B\r\n
  uint8_t *message = (uint8_t *)"$PSRF117,16*0B\r\n";

  status_ok_or_return(uart_tx(settings->port, message, strlen((char *)message)));
  status_ok_or_return(soft_timer_start_seconds(1, prv_chip_power_state_off, settings, NULL));

  memset(&s_storage, 0, sizeof(s_storage));

  return STATUS_CODE_OK;
}
