#include "pedal_output.h"
#include <string.h>
#include "can_transmit.h"
#include "debug_led.h"
#include "exported_enums.h"
#include "log.h"
#include "misc.h"

#define PEDAL_OUTPUT_VALID_WATCHDOG ((1 << NUM_PEDAL_OUTPUT_SOURCES) - 1)

static PedalOutputStorage s_storage;

static void prv_watchdog_cb(SoftTimerId timer_id, void *context) {
  PedalOutputStorage *storage = context;

  // We're missing at least one updated response
  if (storage->watchdog != PEDAL_OUTPUT_VALID_WATCHDOG) {
    // Error - raise a warning, clear stored data
    LOG_DEBUG("Pedal output watchdog: 0x%x\n", storage->watchdog);
    memset(storage->data, 0, sizeof(storage->data));
    event_raise_priority(EVENT_PRIORITY_HIGHEST, storage->fault_event, 0);
  }

  // Reset watchdog
  storage->watchdog = 0;

  soft_timer_start_millis(PEDAL_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, context,
                          &storage->watchdog_timer);
}

static void prv_broadcast_cb(SoftTimerId timer_id, void *context) {
  PedalOutputStorage *storage = context;

  // Note that this will usually output stale data from the previous update request
  event_raise(storage->update_req_event, 0);

  CAN_TRANSMIT_PEDAL_OUTPUT((uint16_t)storage->data[PEDAL_OUTPUT_SOURCE_THROTTLE],
                            (uint16_t)storage->data[PEDAL_OUTPUT_SOURCE_THROTTLE_STATE],
                            (uint16_t)storage->data[PEDAL_OUTPUT_SOURCE_MECH_BRAKE]);

  debug_led_toggle_state(DEBUG_LED_BLUE_A);

  soft_timer_start_millis(PEDAL_OUTPUT_BROADCAST_MS, prv_broadcast_cb, context,
                          &storage->output_timer);
}

StatusCode pedal_output_init(PedalOutputStorage *storage, EventId fault_event,
                             EventId update_req_event) {
  memset(storage, 0, sizeof(*storage));
  storage->fault_event = fault_event;
  storage->update_req_event = update_req_event;

  storage->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
  storage->output_timer = SOFT_TIMER_INVALID_TIMER;

  debug_led_init(DEBUG_LED_BLUE_A);

  return STATUS_CODE_OK;
}

StatusCode pedal_output_set_enabled(PedalOutputStorage *storage, bool enabled) {
  soft_timer_cancel(storage->watchdog_timer);
  soft_timer_cancel(storage->output_timer);

  if (enabled) {
    // Reset watchdog
    storage->watchdog = 0;

    StatusCode ret = soft_timer_start_millis(PEDAL_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, storage,
                                             &storage->watchdog_timer);
    status_ok_or_return(ret);

    return soft_timer_start_millis(PEDAL_OUTPUT_BROADCAST_MS, prv_broadcast_cb, storage,
                                   &storage->output_timer);
  } else {
    storage->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
    storage->output_timer = SOFT_TIMER_INVALID_TIMER;
  }

  return STATUS_CODE_OK;
}

StatusCode pedal_output_update(PedalOutputStorage *storage, PedalOutputSource source,
                               int16_t data) {
  if (source >= NUM_PEDAL_OUTPUT_SOURCES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  if (source == PEDAL_OUTPUT_SOURCE_THROTTLE) {
    int16_t prev_data = storage->data[source];
    bool data_negative = data < 0, prev_negative = prev_data < 0;

    int16_t abs_diff = abs(data - prev_data);
    int16_t torque_diff = (data - prev_data) * (data_negative ? -1 : 1);

    if (data == 0) {
      // Attempting to be in coast, so don't worry about it
    } else if ((data_negative != prev_negative && abs_diff > 1000) ||
               (data_negative == prev_negative && torque_diff > 1000)) {
      // Sign changed rapidly or torque increased rapidly - limit change
      data = storage->data[source] + ((data > 0) ? 100 : -100);

      data = MIN(data, EE_PEDAL_OUTPUT_DENOMINATOR);
      data = MAX(data, -EE_PEDAL_OUTPUT_DENOMINATOR);
    }
  }

  storage->data[source] = data;
  storage->watchdog |= 1 << source;

  return STATUS_CODE_OK;
}

PedalOutputStorage *pedal_output_global(void) {
  return &s_storage;
}
