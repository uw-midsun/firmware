#include "drive_output.h"
#include <string.h>
#include "can_transmit.h"
#include "debug_led.h"
#include "exported_enums.h"
#include "log.h"
#include "misc.h"

#define DRIVE_OUTPUT_VALID_WATCHDOG ((1 << NUM_DRIVE_OUTPUT_SOURCES) - 1)

static DriveOutputStorage s_storage;

static void prv_watchdog_cb(SoftTimerID timer_id, void *context) {
  DriveOutputStorage *storage = context;

  // // We're missing at least one updated response
  // if (storage->watchdog != DRIVE_OUTPUT_VALID_WATCHDOG) {
  //   // Error - raise a warning, clear stored data
  //   LOG_DEBUG("Drive output watchdog: 0x%x\n", storage->watchdog);
  //   memset(storage->data, 0, sizeof(storage->data));
  //   event_raise_priority(EVENT_PRIORITY_HIGHEST, storage->fault_event, 0);
  // }

  // Reset watchdog
  storage->watchdog = 0;

  soft_timer_start_millis(DRIVE_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, context,
                          &storage->watchdog_timer);
}

static void prv_broadcast_cb(SoftTimerID timer_id, void *context) {
  DriveOutputStorage *storage = context;

  // Note that this will usually output stale data from the previous update request
  event_raise(storage->update_req_event, 0);

  // Scale regen brake
  if (storage->data[DRIVE_OUTPUT_SOURCE_THROTTLE] < 0) {
    storage->data[DRIVE_OUTPUT_SOURCE_THROTTLE] = 0;
  }

  CAN_TRANSMIT_DRIVE_OUTPUT((uint16_t)storage->data[DRIVE_OUTPUT_SOURCE_THROTTLE],
                            (uint16_t)storage->data[DRIVE_OUTPUT_SOURCE_DIRECTION],
                            (uint16_t)storage->data[DRIVE_OUTPUT_SOURCE_CRUISE],
                            (uint16_t)storage->data[DRIVE_OUTPUT_SOURCE_MECH_BRAKE]);

  debug_led_toggle_state(DEBUG_LED_BLUE_A);

  soft_timer_start_millis(DRIVE_OUTPUT_BROADCAST_MS, prv_broadcast_cb, context,
                          &storage->output_timer);
}

StatusCode drive_output_init(DriveOutputStorage *storage, EventID fault_event,
                             EventID update_req_event) {
  memset(storage, 0, sizeof(*storage));
  storage->fault_event = fault_event;
  storage->update_req_event = update_req_event;

  storage->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
  storage->output_timer = SOFT_TIMER_INVALID_TIMER;

  debug_led_init(DEBUG_LED_BLUE_A);

  return STATUS_CODE_OK;
}

StatusCode drive_output_set_enabled(DriveOutputStorage *storage, bool enabled) {
  soft_timer_cancel(storage->watchdog_timer);
  soft_timer_cancel(storage->output_timer);

  if (enabled) {
    // Reset watchdog
    storage->watchdog = 0;

    StatusCode ret = soft_timer_start_millis(DRIVE_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, storage,
                                             &storage->watchdog_timer);
    status_ok_or_return(ret);

    return soft_timer_start_millis(DRIVE_OUTPUT_BROADCAST_MS, prv_broadcast_cb, storage,
                                   &storage->output_timer);
  } else {
    storage->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
    storage->output_timer = SOFT_TIMER_INVALID_TIMER;
  }

  return STATUS_CODE_OK;
}

StatusCode drive_output_update(DriveOutputStorage *storage, DriveOutputSource source,
                               int16_t data) {
  if (source >= NUM_DRIVE_OUTPUT_SOURCES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  storage->data[source] = data;
  storage->watchdog |= 1 << source;

  return STATUS_CODE_OK;
}

DriveOutputStorage *drive_output_global(void) {
  return &s_storage;
}
