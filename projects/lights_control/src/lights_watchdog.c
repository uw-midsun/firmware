#include "lights_watchdog.h"
#include "exported_enums.h"
#include "lights_events.h"

static void prv_watchdog_callback(SoftTimerID timer_id, void *context) {
  // If this callback fires, we turn strobe ON.
  uint8_t dummy_data = 0;
  event_raise(LIGHTS_EVENT_STROBE_ON, dummy_data);
}

StatusCode lights_watchdog_init(LightsWatchDogStorage *storage, LightsWatchDogTimeout timeout_ms) {
  storage->timeout_ms = timeout_ms;
  return soft_timer_start_millis(timeout_ms, prv_watchdog_callback, NULL, &storage->timer_id);
}

StatusCode lights_watchdog_process_event(LightsWatchDogStorage *storage, const Event *e) {
  // Ignore the event if its not a bps heartbeat.
  if (e->id != LIGHTS_EVENT_BPS_HEARTBEAT) {
    return STATUS_CODE_OK;
  }
  // Cancel the timer to prevent the interrupt from triggering while a bps heartbeat event is
  // being processed.
  soft_timer_cancel(storage->timer_id);
  if (e->data == EE_BPS_HEARTBEAT_STATE_ERROR) {
    prv_watchdog_callback(storage->timer_id, NULL);
    return STATUS_CODE_OK;
  }
  return lights_watchdog_init(storage, storage->timeout_ms);
}
