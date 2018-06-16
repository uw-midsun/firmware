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

  // Idle: All states to/from

  // Invalid: idle -> charge
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Invalid: idle -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Valid: idle -> charge_preconfig
  e.id = CHAOS_EVENT_GPIO_CHARGE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: charge_preconfig -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: idle -> drive_preconfig
  e.id = CHAOS_EVENT_GPIO_CHARGE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: drive_preconfig -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: idle -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: emergency -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: idle -> charge_preconfig
  e.id = CHAOS_EVENT_GPIO_CHARGE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: charge_preconfig -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: charge -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: idle -> drive_preconfig
  e.id = CHAOS_EVENT_GPIO_DRIVE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: drive_preconfig -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: drive -> idle
  e.id = CHAOS_EVENT_GPIO_IDLE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Emergency: All state to/from except idle (already tested)

  // Valid: idle -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: idle -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Invalid: emergency -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Valid: emergency -> charge_preconfig
  e.id = CHAOS_EVENT_GPIO_CHARGE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: charge_preconfig -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: emergency -> drive_preconfig
  e.id = CHAOS_EVENT_GPIO_DRIVE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: drive_preconfig -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: emergency -> charge_preconfig
  e.id = CHAOS_EVENT_GPIO_CHARGE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: charge_preconfig -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: charge -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: emergency -> drive_preconfig
  e.id = CHAOS_EVENT_GPIO_DRIVE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: drive_preconfig -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Valid: drive -> emergency
  e.id = CHAOS_EVENT_GPIO_EMERGENCY;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Charge: All states/to/from except idle/emergency

  // Valid: emergency -> charge_preconfig
  e.id = CHAOS_EVENT_GPIO_CHARGE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: charge_preconfig -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Valid: charge_preconfig -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: charge -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Drive: All states/to/from except idle/emergency/charge

  // Valid: charge -> drive_preconfig
  e.id = CHAOS_EVENT_GPIO_DRIVE_PRECONFIG;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: drive_preconfig -> charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));

  // Valid: drive_preconfig -> drive
  e.id = CHAOS_EVENT_GPIO_DRIVE;
  TEST_ASSERT_TRUE(gpio_fsm_process_event(&e));

  // Invalid: drive->charge
  e.id = CHAOS_EVENT_GPIO_CHARGE;
  TEST_ASSERT_FALSE(gpio_fsm_process_event(&e));
}
