#include "gps.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "uart.h"
#include "uart_mcu.h"
#include "xbee.h"

// A large struct to store data and settings. Since the GPS should only be initialized once
static GpsSettings *s_settings = NULL;

static bool s_gps_nmea_is_gga(const char *to_check) {
  if (to_check == NULL) {
    return false;
  }
  size_t len = strlen(to_check);
  if (len < 6) {
    return false;
  }
  if (to_check[3] == 'G' && to_check[4] == 'G' && to_check[5] == 'A') {
    return true;
  }
  return false;
}

// This method will be called every time the GPS sends data.
static void prv_gps_callback(const uint8_t *rx_arr, size_t len, void *context) {
  if (s_gps_nmea_is_gga((char *)rx_arr)) {
    snprintf(s_settings->gga_data, GPS_MAX_NMEA_LENGTH, "%s", (char *)rx_arr);
  }
  s_settings->gps_active = true;
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

// The following two functions are useful for callbacks since there's no need
// to pass more info into the context
static void prv_gps_set_power_state_on(SoftTimerID timer_id, void *context) {
  prv_gps_set_power_state(true);
}

static void prv_gps_set_power_state_off(SoftTimerID timer_id, void *context) {
  prv_gps_set_power_state(false);
}

// Stage 1 of initialization: pull high on ON/OFF line
static void prv_gps_init_stage_1(SoftTimerID timer_id, void *context) {
  gpio_set_state(s_settings->pin_on_off, GPIO_STATE_HIGH);
}

// Stage 2: set ON/OFF line to low
static void prv_gps_init_stage_2(SoftTimerID timer_id, void *context) {
  gpio_set_state(s_settings->pin_on_off, GPIO_STATE_LOW);
}

// This method runs every 5 seconds and clears the active flag. The UART
// callback for the GPS runs every second and sets the active flag.
// This is do that if the GPS crashes or something like that, it will be
// automatically rebooted
static void prv_gps_loop(SoftTimerID timer_id, void *context) {
  // Restarts the GPS if it is not "active" and the desired state is "on".
  if (!s_settings->gps_active && s_settings->gps_desired_state) {
    gps_clean_up();
    prv_gps_set_power_state(false);
    soft_timer_start_seconds(1, prv_gps_set_power_state_on, NULL, NULL);
    soft_timer_start_seconds(3, prv_gps_init_stage_1, NULL, NULL);
    soft_timer_start_millis(3100, prv_gps_init_stage_2, NULL, NULL);
    s_settings->gps_desired_state = true;
  }
  StatusCode ret = soft_timer_start_seconds(10, prv_gps_loop, NULL, NULL);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Could not start prv_gps_loop timer\n");
  }
  xbee_transmit(s_settings->gga_data, strlen(s_settings->gga_data));
  // Sets the GPS to the "inactive" state for book keeping purposes
  s_settings->gps_active = false;
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

  // Starts the GPS health monitor
  status_ok_or_return(soft_timer_start_seconds(5, prv_gps_loop, &s_settings, NULL));

  // Set the desired state to "on", in case of multiple initializations
  s_settings->gps_desired_state = true;
  return STATUS_CODE_OK;
}

StatusCode gps_clean_up() {
  // Makes sure that the health monitor doesn't keep restarting the GPS
  s_settings->gps_desired_state = false;

  // The shutdown message to be sent to the GPS
  char *message = "$PSRF117,16*0B\r\n";

  // Sends the message
  status_ok_or_return(uart_tx(s_settings->port, (uint8_t *)message, strlen(message)));
  return soft_timer_start_seconds(1, prv_gps_set_power_state_off, NULL, NULL);
}

// Are these two methods below thread safe? Or should I pass in a pointer,
// and use snprintf to copy into it?
bool gps_get_gga(char *gga_message) {
  if (gga_message != NULL) {
    gga_message = s_settings->gga_data;
  }
  return s_settings->gps_active;
}
