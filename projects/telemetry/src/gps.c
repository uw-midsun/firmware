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
#include "util.h"
#include "status.h"
#include "uart_mcu.h"

// How many handlers can we possibly need?
#define GPS_HANDLER_ARRAY_LENGTH 5

static evm_gps_gga_handler s_gga_handler[GPS_HANDLER_ARRAY_LENGTH] = { 0 };
static UARTStorage s_storage;

// This is so that initializing multiple times does not cause bugs.
// To clean up you must call evm_gps_clean_up(EvmSettings *settings)
static bool s_initialized = false;

static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {

  LOG_DEBUG("Read NMEA message\n");

  if (!status_ok(evm_gps_is_valid_nmea(rx_arr, len))) {
    LOG_DEBUG("Invalid nmea message\n");
    return;
  }

  EVM_GPS_NMEA_MESSAGE_ID message_id;
  evm_gps_get_nmea_sentence_type(rx_arr, &message_id);

  if (message_id == EVM_GPS_GGA) {
    evm_gps_gga_sentence r = evm_gps_parse_nmea_gga_sentence(rx_arr, len);
    for (uint32_t i = 0; i < GPS_HANDLER_ARRAY_LENGTH; i++) {
      if (s_gga_handler[i] != NULL) {
        (*s_gga_handler[i])(r);
      }
    }
  }
}

StatusCode evm_gps_add_gga_handler(evm_gps_gga_handler handler, size_t *index) {
  for (uint16_t i = 0; i < GPS_HANDLER_ARRAY_LENGTH; i++) {
    if (s_gga_handler[i] == NULL) {
      s_gga_handler[i] = handler;
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

StatusCode evm_gps_remove_gga_handler(size_t index) {
  if (index >= GPS_HANDLER_ARRAY_LENGTH) return STATUS_CODE_UNKNOWN;
  s_gga_handler[index] = NULL;
  return STATUS_CODE_OK;
}

void prv_stop_ON_OFF(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;
  gpio_set_state(settings->pin_on_off, GPIO_STATE_LOW);
  LOG_DEBUG("No longer pulling ON OFF\n");
  // Here we should wait for one second
  // During this period of time we should hear something from the GPS chip
  // If not, start again from pull_ON_OFF
}

// This is useful as a method for callbacks
void prv_chip_power_state_on(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;

  LOG_DEBUG("Sending power to GPS chip\n");

  gpio_set_state(settings->pin_power, GPIO_STATE_HIGH);
}

// This is useful as a method for callbacks
void prv_chip_power_state_off(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;
  LOG_DEBUG("Turning power off for GPS chip\n");
  gpio_set_state(settings->pin_power, GPIO_STATE_LOW);
}

// This callback should start the initialization sequence
void prv_pull_ON_OFF(SoftTimerID timer_id, void *context) {
  evm_gps_settings *settings = context;

  LOG_DEBUG("Pulling ON OFF\n");

  // Can't really return a StatusCode here. Might need a messy work around / leave it?
  gpio_set_state(settings->pin_on_off, GPIO_STATE_HIGH);
  soft_timer_start_millis(100, prv_stop_ON_OFF, settings, NULL);
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
  LOG_DEBUG("Entering GPS initialization\n");
  if (s_initialized) return STATUS_CODE_OK;
  LOG_DEBUG("Starting GPS initialization\n");

  status_ok_or_return(evm_gps_validate_settings(settings));
  LOG_DEBUG("Settings are valid\n");

  settings->uart_settings->rx_handler = &s_nmea_read;
  // Makes sure that status codes are handled
  uart_init(*(settings->port), settings->uart_settings, &s_storage);
  LOG_DEBUG("UART initialized\n");
  // These should already be initialized, but we do so anyway, to make sure

  status_ok_or_return(gpio_init_pin(settings->pin_power, settings->settings_power));
  status_ok_or_return(gpio_init_pin(settings->pin_on_off, settings->settings_on_off));

  // From the documentation: Power needs to be on for one second before continuing
  prv_chip_power_state_on(0, settings);
  status_ok_or_return(soft_timer_start_seconds(1, prv_pull_ON_OFF, settings, NULL));
  LOG_DEBUG("Initialized\n");
  s_initialized = true;
  return STATUS_CODE_OK;
}

// Implementing shut down here:
// From page 25 of:
// https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
StatusCode evm_gps_clean_up(evm_gps_settings *settings) {

  // This string is taken from the pdf mentioned above. The \r\n may not be necessary depending on
  // if the uart library adds it automatically or not.
  // The char array below should read $PSRF117,16*0B\r\n
  uint8_t message[] = { '$', 'P', 'S', 'R', 'F', '1', '1',  '7',
                        ',', '1', '6', '*', '0', 'B'};

  status_ok_or_return(uart_tx(*(settings->port), (uint8_t *) &message, sizeof(message)));
  LOG_DEBUG("Sent shutdown message to GPS chip\n");
  status_ok_or_return(soft_timer_start_seconds(1, prv_chip_power_state_off, settings, NULL));

  memset(&s_gga_handler, 0, SIZEOF_ARRAY(s_gga_handler));
  memset(&s_storage, 0, sizeof(s_storage));

  s_initialized = false;
  return STATUS_CODE_OK;
}

// out must a pointer to at least 24 chars
void disable_message_type(EVM_GPS_NMEA_MESSAGE_ID type, char *out) {
    char *format = "$PSRF103,0%d,00,00,01*";
    snprintf(out, 24, format, type);
    char checksum[3];
    evm_gps_compute_checksum(out, checksum);
    strncat(out, checksum, 2);
}
