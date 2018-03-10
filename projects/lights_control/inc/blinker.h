#pragma once
#include "soft_timer.h"

typedef enum {
  BLINKER_STATE_OFF = 0,
  BLINKER_STATE_ON,
  NUM_BLINKER_STATES
} BlinkerState;

typedef void (*SyncCallback)(void);

typedef void (*BlinkerCallback)(BlinkerState state);

typedef uint32_t BlinkerDuration;

typedef struct Blinker {
  SoftTimerID timer_id;
  volatile BlinkerState state;
  SoftTimerCallback timer_callback;
  BlinkerCallback callback;
  SyncCallback sync_callback;
  BlinkerDuration duration_us;
  uint8_t blink_count;
  uint8_t sync_frequency;
} Blinker;

void blinker_init_sync(Blinker *, BlinkerCallback, SyncCallback, uint8_t sync_frequency);

StatusCode blinker_on_us(Blinker *, BlinkerDuration duration_us);

bool blinker_off(Blinker *);

StatusCode blinker_reset(Blinker *);

#define BLINKER_STATE_INVALID 2

#define blinker_init(blinker, blinker_callback) \
        blinker_init_sync(blinker, blinker_callback, NULL, 10)

#define blinker_on_millis(blinker, duration) blinker_on_us(blinker, duration * 1000)

#define blinker_on_seconds(blinker, duration) blinker_on_us(blinker, duration * 1000000)

#define blinker_on(blinker) blinker_on_millis(blinker, 500)

