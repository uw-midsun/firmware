#include <stdlib.h>

#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "lights_blinker.h"

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_lights_blinker_init(void) {
  LightsBlinker blinker;
  lights_blinker_init(&blinker);
  TEST_ASSERT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
}

void test_lights_blinker_on(void) {
  LightsBlinker blinker_us;
  LightsBlinker blinker_millis;
  LightsBlinkerDuration duration_us = 300000;
  LightsBlinkerDuration duration_millis = 500;

  EventID EVENT_1 = 1;
  EventID EVENT_2 = 2;

  TEST_ASSERT_OK(lights_blinker_init(&blinker_us));
  TEST_ASSERT_OK(lights_blinker_init(&blinker_millis));

  TEST_ASSERT_OK(lights_blinker_on_us(&blinker_us, duration_us, EVENT_1));
  TEST_ASSERT_EQUAL(blinker_us.duration_us, duration_us);
  TEST_ASSERT_EQUAL(LIGHTS_BLINKER_STATE_ON, blinker_us.state);
  TEST_ASSERT_NOT_EQUAL(blinker_us.timer_id, SOFT_TIMER_INVALID_TIMER);

  Event e;
  while (event_process(&e) != STATUS_CODE_OK) {
  }

  TEST_ASSERT_EQUAL(e.id, EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  TEST_ASSERT_OK(lights_blinker_on_millis(&blinker_millis, duration_millis, EVENT_2));
  TEST_ASSERT_EQUAL(blinker_millis.duration_us, duration_millis * 1000);
  TEST_ASSERT_EQUAL(LIGHTS_BLINKER_STATE_ON, blinker_millis.state);
  TEST_ASSERT_NOT_EQUAL(blinker_millis.timer_id, SOFT_TIMER_INVALID_TIMER);

  while (event_process(&e) != STATUS_CODE_OK) {
  }

  TEST_ASSERT_EQUAL(e.id, EVENT_2);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  // wait for the microseconds timer to fire
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_OFF);

  // wait for the milliseconds timer to fire
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, EVENT_2);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_OFF);
}

void test_lights_blinker_on_uninitialized(void) {
  LightsBlinker blinker;
  LightsBlinkerDuration duration = 300;
  EventID EVENT_1 = 1;
  TEST_ASSERT_NOT_OK(lights_blinker_on_millis(&blinker, duration, EVENT_1));
}

void test_lights_blinker_on_already_active(void) {
  LightsBlinker blinker;
  LightsBlinkerDuration duration = 300;
  EventID EVENT_1 = 1;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on_millis(&blinker, duration, EVENT_1));
  TEST_ASSERT_NOT_OK(lights_blinker_on_millis(&blinker, duration, EVENT_1));
}

void test_lights_blinker_off(void) {
  LightsBlinker blinker;
  LightsBlinkerDuration duration = 300;
  EventID EVENT_1 = 1;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on_millis(&blinker, duration, EVENT_1));
  TEST_ASSERT_OK(lights_blinker_off(&blinker));
  TEST_ASSERT_EQUAL(blinker.timer_id, SOFT_TIMER_INVALID_TIMER);
  TEST_ASSERT_FALSE(soft_timer_inuse());
}

void test_lights_blinker_off_already_inactive(void) {
  LightsBlinker blinker;
  LightsBlinkerDuration duration = 300;
  EventID EVENT_1 = 1;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on_millis(&blinker, duration, EVENT_1));
  TEST_ASSERT_OK(lights_blinker_off(&blinker));
  TEST_ASSERT_NOT_OK(lights_blinker_off(&blinker));
}

void test_lights_blinker_reset(void) {
  LightsBlinker blinker;
  LightsBlinkerDuration duration = 300;
  EventID EVENT_1 = 1;
  TEST_ASSERT_OK(lights_blinker_init(&blinker));
  TEST_ASSERT_OK(lights_blinker_on_millis(&blinker, duration, EVENT_1));

  Event e;
  while (event_process(&e) != STATUS_CODE_OK) {
  }

  TEST_ASSERT_EQUAL(e.id, EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);

  TEST_ASSERT_OK(lights_blinker_reset(&blinker));

  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(e.id, EVENT_1);
  TEST_ASSERT_EQUAL(e.data, LIGHTS_BLINKER_STATE_ON);
}
