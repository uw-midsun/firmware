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
static volatile bool s_gps_active = false; // Keeps track of whether the GPS is sending data or not
static volatile bool s_gps_desired_state = true; // Keeps track of whether we want the
                                                  // GPS to be active or not

static bool s_evm_initialize_by_gpio = false;

// Just some constants so that the max length of raw data can be set.
// A GGA message will be around a hundred characters. VTG is less.
#define s_max_nmea_length 128

// Stores raw NMEA messages sent by the chip
static volatile char s_gga_data[s_max_nmea_length];
static volatile char s_vtg_data[s_max_nmea_length];

static void prv_gps_callback(const uint8_t *rx_arr, size_t len, void *context) {
  if (nmea_is_gga((char *)rx_arr)) {
    snprintf(s_gga_data, s_max_nmea_length, "%s", (char *)rx_arr);
    s_gps_active = true;
  } else if (nmea_is_vtg((char *)rx_arr)) {
    snprintf(s_vtg_data, s_max_nmea_length, "%s", (char *)rx_arr);
    s_gps_active = true;
  }
}

static void prv_gps_set_power_state(gps_settings *settings, bool powered) {
  gps_settings *settings = context;
  if (settings->pin_power == NULL) {
    return;
  }
  if (powered) {
    gpio_set_state(settings->pin_power, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(settings->pin_power, GPIO_STATE_LOW);
  }
}

// The following two functions are useful for callbacks since there's no need
// to pass more info into the context
static void prv_gps_set_power_state_on(SoftTimerID timer_id, void *context) {
  if (context != NULL) {
    prv_gps_set_power_state((gps_settings *) context, true);
  }
}

static void prv_gps_set_power_state_off(SoftTimerID timer_id, void *context) {
  if (context != NULL) {
    prv_gps_set_power_state((gps_settings *) context, false);
  }
}

// Stage 1 of initialization: pull high on ON/OFF line
static void prv_gps_init_stage_1(SoftTimerID timer_id, void *context) {
  if (context != NULL) {
    gps_settings *settings = context;
    gpio_set_state(settings->pin_on_off, GPIO_STATE_HIGH);
  }
}

// Stage 2: set ON/OFF line to low
static void prv_gps_init_stage_2(SoftTimerID timer_id, void *context) {
  if (context != NULL) {
    gps_settings *settings = context;
    gpio_set_state(settings->pin_on_off, GPIO_STATE_LOW);
  }
}

StatusCode gps_validate_settings(gps_settings *settings) {
  // Making sure all settings as passed in
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

static void prv_gps_health_monitor(SoftTimerID timer_id, void *context) {
  if (context == NULL) {
    LOG_CRITICAL("Context to prv_gps_health_monitor is NULL, cannot monitor GPS health\n");
    return;
  }
  if (!s_gps_active && s_gps_desired_state) {
    prv_gps_set_power_state(settings, false);
    soft_timer_start_seconds(1, prv_gps_set_power_state_on, settings, NULL);
    soft_timer_start_seconds(3, prv_gps_init_stage_1, settings, NULL);
    soft_timer_start_millis(3100, prv_gps_init_stage_2, settings, NULL);
    s_gps_active = false;
  }
  StatusCode ret = soft_timer_start_seconds(5, prv_gps_health_monitor, settings, NULL);
  if (!status_ok(ret)) {
    LOG_CRITICAL("Could not start prv_gps_health_monitor timer\n");
  }
}

// Initialization of this chip is described on page 10 of:
// https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
StatusCode gps_init(gps_settings *settings) {
  status_ok_or_return(gps_validate_settings(settings));

  // Initializes UART
  StatusCode ret = uart_init(settings->port, settings->uart_settings, &s_storage);
  uart_set_rx_handler(settings->port, prv_gps_callback, NULL);

  // Initializes the pins
  status_ok_or_return(gpio_init_pin(settings->pin_power, settings->settings_power));
  status_ok_or_return(gpio_init_pin(settings->pin_on_off, settings->settings_on_off));

  // Initializes the GPS by pulling the ON OFF pin to high for 100 ms
  status_ok_or_return(soft_timer_start_seconds(5, prv_gps_health_monitor, settings, NULL));
  s_gps_desired_state = true;
  return STATUS_CODE_OK;
}

// Implementing shut down here:
// From page 25 of:
// https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
StatusCode gps_clean_up(gps_settings *settings) {
  // The char array below should read $PSRF117,16*0B\r\n
  s_gps_desired_state = false;
  char *message = "$PSRF117,16*0B\r\n";

  status_ok_or_return(uart_tx(settings->port, message, strlen(message)));
  status_ok_or_return(soft_timer_start_seconds(1, prv_gps_set_power_state_off, settings, NULL));
  return STATUS_CODE_OK;
}