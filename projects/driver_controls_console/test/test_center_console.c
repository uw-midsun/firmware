#include "cc_input_event.h"
#include "center_console.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "wait.h"

static CenterConsoleStorage s_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  event_queue_init();
  soft_timer_init();

  TEST_ASSERT_OK(center_console_init(&s_storage));
}

void teardown_test(void) {}

void test_center_console_readback(void) {
  Event e;

  while (true) {
    wait();
    while (status_ok(event_process(&e))) {
      switch (e.id) {
        case INPUT_EVENT_CENTER_CONSOLE_POWER:
          LOG_DEBUG("POWER\n");
          break;
        case INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE:
          LOG_DEBUG("DIRECTION_DRIVE\n");
          break;
        case INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL:
          LOG_DEBUG("DIRECTION_NEUTRAL\n");
          break;
        case INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE:
          LOG_DEBUG("DIRECTION_REVERSE\n");
          break;
        case INPUT_EVENT_CENTER_CONSOLE_DRL:
          LOG_DEBUG("DRL\n");
          break;
        case INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS:
          LOG_DEBUG("LOWBEAMS\n");
          break;
        case INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED:
          LOG_DEBUG("HAZARDS_RELEASED\n");
          break;

        default:
          break;
      }
    }
  }
}
