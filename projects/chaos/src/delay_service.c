#include "delay_service.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "soft_timer.h"

static SoftTimerId s_timer_id = SOFT_TIMER_INVALID_TIMER;

static void prv_delay_end(SoftTimerId timer_id, void *context) {
  (void)timer_id;
  SoftTimerId *id = context;
  event_raise(CHAOS_EVENT_DELAY_DONE, 0);
  *id = SOFT_TIMER_INVALID_TIMER;
}

void delay_service_process_event(const Event *e) {
  if (e->id == CHAOS_EVENT_DELAY_MS) {
    if (s_timer_id != SOFT_TIMER_INVALID_TIMER) {
      soft_timer_cancel(s_timer_id);
      s_timer_id = SOFT_TIMER_INVALID_TIMER;
    }
    if (e->data) {
      soft_timer_start_millis(((uint32_t)e->data), prv_delay_end, &s_timer_id, &s_timer_id);
    }
  }
}

void delay_service_cancel(void) {
  if (s_timer_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(s_timer_id);
    s_timer_id = SOFT_TIMER_INVALID_TIMER;
    event_raise(CHAOS_EVENT_DELAY_DONE, 0);
  }
}
