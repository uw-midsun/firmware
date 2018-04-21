#include "gpio_fsm.h"

#include "chaos_config.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "unity.h"

void setup_test(void) {
  const ChaosConfig *config = chaos_config_load();

  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_fsm_init(config);
}

void teardown_test(void) {}

void test_gpio_fsm(void) {
  Event e = {
    .id = CHAOS_EVENT_GPIO_CHARGE,
  };

  // Note: Don't bother with testing the GPIO setting component since gpio_seq and gpio libraries
  // will handle this.

  // Valid: idle -> charge
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: charge -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Valid: charge -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: idle -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: charge -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: emergency -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Invalid: emergency -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Valid: emergency -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: idle -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: drive -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Valid: drive -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: idle -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: drive -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));
}
