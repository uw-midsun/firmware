#include "console_output.h"
#include <string.h>
#include "can_transmit.h"
#include "debug_led.h"
#include "exported_enums.h"
#include "log.h"
#include "misc.h"

#define CONSOLE_OUTPUT_VALID_WATCHDOG ((1 << NUM_CONSOLE_OUTPUT_SOURCES) - 1)

static ConsoleOutputStorage s_storage;

static void prv_watchdog_cb(SoftTimerId timer_id, void *context) {
  ConsoleOutputStorage *storage = context;

  // We're missing at least one updated response
  if (storage->watchdog != CONSOLE_OUTPUT_VALID_WATCHDOG) {
    // Error - raise a warning, clear stored data
    LOG_DEBUG("Console output watchdog: 0x%x\n", storage->watchdog);
    memset(storage->data, 0, sizeof(storage->data));
    event_raise_priority(EVENT_PRIORITY_HIGHEST, storage->fault_event, 0);
  }

  // Reset watchdog
  storage->watchdog = 0;

  soft_timer_start_millis(CONSOLE_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, context,
                          &storage->watchdog_timer);
}

static void prv_broadcast_cb(SoftTimerId timer_id, void *context) {
  ConsoleOutputStorage *storage = context;

  // Note that this will usually output stale data from the previous update request
  event_raise(storage->update_req_event, 0);

  CAN_TRANSMIT_CONSOLE_OUTPUT((uint16_t)storage->data[CONSOLE_OUTPUT_SOURCE_DIRECTION]);

  debug_led_toggle_state(DEBUG_LED_BLUE_A);

  soft_timer_start_millis(CONSOLE_OUTPUT_BROADCAST_MS, prv_broadcast_cb, context,
                          &storage->output_timer);
}

StatusCode console_output_init(ConsoleOutputStorage *storage, EventId fault_event,
                               EventId update_req_event) {
  memset(storage, 0, sizeof(*storage));
  storage->fault_event = fault_event;
  storage->update_req_event = update_req_event;

  storage->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
  storage->output_timer = SOFT_TIMER_INVALID_TIMER;

  debug_led_init(DEBUG_LED_BLUE_A);

  return STATUS_CODE_OK;
}

StatusCode console_output_set_enabled(ConsoelOutputStorage *storage, bool enabled) {
  soft_timer_cancel(storage->watchdog_timer);
  soft_timer_cancel(storage->output_timer);

  if (enabled) {
    // Reset watchdog
    storage->watchdog = 0;

    StatusCode ret = soft_timer_start_millis(CONSOLE_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, storage,
                                             &storage->watchdog_timer);
    status_ok_or_return(ret);

    return soft_timer_start_millis(CONSOLE_OUTPUT_BROADCAST_MS, prv_broadcast_cb, storage,
                                   &storage->output_timer);
  } else {
    storage->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
    storage->output_timer = SOFT_TIMER_INVALID_TIMER;
  }

  return STATUS_CODE_OK;
}

StatusCode console_output_update(ConsoelOutputStorage *storage, ConsoleOutputSource source,
                                 int16_t data) {
  if (source >= NUM_CONSOLE_OUTPUT_SOURCES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  storage->data[source] = data;
  storage->watchdog |= 1 << source;

  return STATUS_CODE_OK;
}

ConsoleOutputStorage *console_output_global(void) {
  return &s_storage;
}
