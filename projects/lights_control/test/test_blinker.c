#include <stdlib.h>

#include "soft_timer.h"
#include "interrupt.h"
#include "test_helpers.h"
#include "unity.h"
#include "status.h"

#include "blinker.h"

#define BLINKER_SYNC_FREQ_DEFAULT 10
#define BLINKER_CB_ARG_INVALID 2

static uint8_t s_blinker_cb_arg = 0;

static volatile bool s_sync_called = false;

void prv_blinker_callback(BlinkerState state) {
  s_blinker_cb_arg = state;
}

void prv_wait_blink(Blinker *blinker) {
  while (blinker->state == BLINKER_STATE_ON) {}
  while (blinker->state == BLINKER_STATE_OFF) {}
}

void prv_blinker_sync_callback(void) {
  s_sync_called = true;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_blinker_init(void) {
  Blinker * blinker = malloc(sizeof(Blinker));
  blinker_init(blinker, prv_blinker_callback);
 
  TEST_ASSERT_EQUAL(blinker->state, BLINKER_STATE_ON);
  TEST_ASSERT_EQUAL(blinker->blink_count, 0);
  TEST_ASSERT_EQUAL(blinker->sync_frequency, BLINKER_SYNC_FREQ_DEFAULT);
  TEST_ASSERT_EQUAL(blinker->callback, prv_blinker_callback);
  TEST_ASSERT_NULL(blinker->sync_callback);

  free(blinker);

  blinker = malloc(sizeof(Blinker));
  uint8_t sync_freq = 20;
  blinker_init_sync(blinker, prv_blinker_callback,
                  prv_blinker_sync_callback,
                  sync_freq);
  TEST_ASSERT_EQUAL(blinker->state, BLINKER_STATE_ON);
  TEST_ASSERT_EQUAL(blinker->blink_count, 0);
  TEST_ASSERT_EQUAL(blinker->sync_frequency, sync_freq);
  TEST_ASSERT_EQUAL(blinker->callback, prv_blinker_callback);
  TEST_ASSERT_EQUAL(blinker->sync_callback, prv_blinker_sync_callback);

  free(blinker);
}

void test_blinker_on(void) {
  Blinker blinker_us;
  Blinker blinker_millis;
  Blinker blinker_seconds;

  blinker_init(&blinker_us, prv_blinker_callback);
  blinker_init(&blinker_millis, prv_blinker_callback);
  blinker_init(&blinker_seconds, prv_blinker_callback);

  uint32_t duration_us = 300000;
  uint32_t duration_millis = 500;
  uint32_t duration_seconds = 1;

  blinker_us.state = BLINKER_STATE_INVALID;
  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;

  TEST_ASSERT_OK(blinker_on_us(&blinker_us, duration_us));
  TEST_ASSERT_EQUAL(blinker_us.duration_us, duration_us);
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, blinker_us.state);
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, s_blinker_cb_arg);
  TEST_ASSERT_EQUAL(blinker_us.blink_count, 0);
  TEST_ASSERT_NOT_EQUAL(blinker_us.timer_id, SOFT_TIMER_INVALID_TIMER);

  blinker_millis.state = BLINKER_STATE_INVALID;
  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;
  TEST_ASSERT_OK(blinker_on_millis(&blinker_millis, duration_millis));
  TEST_ASSERT_EQUAL(blinker_millis.duration_us, duration_millis * 1000);
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, blinker_millis.state);
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, s_blinker_cb_arg);
  TEST_ASSERT_EQUAL(blinker_millis.blink_count, 0);
  TEST_ASSERT_NOT_EQUAL(blinker_millis.timer_id, SOFT_TIMER_INVALID_TIMER);
  
  blinker_seconds.state = BLINKER_STATE_INVALID;
  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;
  TEST_ASSERT_OK(blinker_on_seconds(&blinker_seconds, duration_seconds));
  TEST_ASSERT_EQUAL(blinker_seconds.duration_us, duration_seconds * 1000000);
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, blinker_seconds.state);
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, s_blinker_cb_arg);
  TEST_ASSERT_EQUAL(blinker_seconds.blink_count, 0);
  TEST_ASSERT_NOT_EQUAL(blinker_seconds.timer_id, SOFT_TIMER_INVALID_TIMER);
  
  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;
  // wait for the timer to fire
  while (blinker_us.state == BLINKER_STATE_ON) {}
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, blinker_us.state);
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, s_blinker_cb_arg);

  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;
  // wait for the timer to fire
  while (blinker_millis.state == BLINKER_STATE_ON) {}
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, blinker_millis.state);
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, s_blinker_cb_arg);

  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;
  // wait for the timer to fire
  while (blinker_seconds.state == BLINKER_STATE_ON) {}
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, blinker_seconds.state);
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, s_blinker_cb_arg);
}

void test_blinker_sync(void) {
  Blinker blinker;
  uint8_t sync_frequency = 6;
  blinker_init_sync(&blinker, prv_blinker_callback,
                  prv_blinker_sync_callback, sync_frequency);
  blinker_on_millis(&blinker, 100);
  s_sync_called = false;
  for (uint8_t i = 0; i < sync_frequency; i++ ) {
    prv_wait_blink(&blinker);
  }
  TEST_ASSERT_TRUE(s_sync_called);
}

void test_blinker_off(void) {
  Blinker blinker;
  blinker_init(&blinker, prv_blinker_callback);
  TEST_ASSERT_OK(blinker_on(&blinker));
  TEST_ASSERT_NOT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
  TEST_ASSERT_TRUE(soft_timer_inuse());

  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;
  TEST_ASSERT_TRUE(blinker_off(&blinker));
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, blinker.state);
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, s_blinker_cb_arg);
  TEST_ASSERT_EQUAL(0, blinker.blink_count);
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_blinker_reset(void) {
  Blinker blinker;
  blinker_init(&blinker, prv_blinker_callback);
  TEST_ASSERT_OK(blinker_on(&blinker));
  TEST_ASSERT_NOT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
  SoftTimerID old_id = blinker.timer_id;

  s_blinker_cb_arg = BLINKER_CB_ARG_INVALID;
  TEST_ASSERT_OK(blinker_reset(&blinker));
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, s_blinker_cb_arg);
}

