#include <stddef.h>

#include "status.h"
#include "soft_timer.h"

#include "blinker.h"

static void prv_timer_callback(SoftTimerID timer_id, void *context) {
  Blinker *blinker = (Blinker *)context;
  blinker->state = !blinker->state;
  blinker->callback(blinker->state);
  if (blinker->state && blinker->sync_callback != NULL) {
    blinker->blink_count++;
    if (blinker->blink_count >= blinker->sync_frequency) {
      blinker->sync_callback();
    }
  }

  soft_timer_start(blinker->duration_us, prv_timer_callback,
                    (void *) blinker,
                    &blinker->timer_id);
}

void blinker_init_sync(Blinker *blinker, BlinkerCallback callback, 
                SyncCallback sync_callback, uint8_t sync_frequency) {
  blinker->callback = callback;
  blinker->state = BLINKER_STATE_ON;
  blinker->sync_callback = sync_callback;
  blinker->sync_frequency = sync_frequency;
  blinker->blink_count = 0;
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
}

StatusCode blinker_on_us(Blinker *blinker, BlinkerDuration duration_us) {
  blinker->duration_us = duration_us;
  blinker->state = BLINKER_STATE_ON;
  blinker->callback(blinker->state);
  blinker->blink_count = 0;
  if (blinker->timer_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(blinker->timer_id);
  }
  return soft_timer_start(blinker->duration_us, prv_timer_callback,
                    (void *) blinker,
                    &blinker->timer_id);
}

bool blinker_off(Blinker *blinker) {
  blinker->state = BLINKER_STATE_OFF;
  blinker->callback(blinker->state);
  blinker->blink_count = 0;
  bool ret = soft_timer_cancel(blinker->timer_id);
  blinker->timer_id = SOFT_TIMER_INVALID_TIMER;
  return ret;
}

StatusCode blinker_reset(Blinker *blinker) {
  return blinker_on_us(blinker, blinker->duration_us);
}

