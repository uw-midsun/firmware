#include "cc_cfg.h"
#include "cc_input_event.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_expander.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "led_output.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"

#define TEST_LED_OUTPUT_PRINT_AND_DELAY

GpioExpanderStorage s_expander;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = CC_CFG_I2C_BUS_SDA,  //
    .scl = CC_CFG_I2C_BUS_SCL,  //
  };

  i2c_init(CC_CFG_I2C_BUS_PORT, &i2c_settings);

  gpio_expander_init(&s_expander, CC_CFG_I2C_BUS_PORT, CC_CFG_CONSOLE_IO_ADDR, NULL);

  led_output_init(&s_expander);
}

void teardown_test(void) {}

void test_led_output_power(void) {
  Event e = { .id = INPUT_EVENT_CENTER_CONSOLE_POWER };
  GpioState currentState;

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_PWR_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console power LED should now be on.\n");
  delay_s(5);
#endif

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_PWR_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console power LED should now be off.\n");
  delay_s(5);
#endif
}

void test_led_output_direction(void) {
  GpioState driveState;
  GpioState neutralState;
  GpioState reverseState;

  // Drive
  Event e1 = { .id = INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE };
  led_output_process_event(&e1);

  gpio_expander_get_state(&s_expander, CC_CFG_DRIVE_LED, &driveState);
  gpio_expander_get_state(&s_expander, CC_CFG_NEUTRAL_LED, &neutralState);
  gpio_expander_get_state(&s_expander, CC_CFG_REVERSE_LED, &reverseState);

  TEST_ASSERT_EQUAL(driveState, GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(neutralState, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(reverseState, GPIO_STATE_LOW);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The drive LED should now be on.\n The neutral and reverse LEDs should be off.\n");
  delay_s(5);
#endif

  // Neutral
  Event e2 = { .id = INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL };
  led_output_process_event(&e2);

  gpio_expander_get_state(&s_expander, CC_CFG_DRIVE_LED, &driveState);
  gpio_expander_get_state(&s_expander, CC_CFG_NEUTRAL_LED, &neutralState);
  gpio_expander_get_state(&s_expander, CC_CFG_REVERSE_LED, &reverseState);

  TEST_ASSERT_EQUAL(driveState, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(neutralState, GPIO_STATE_HIGH);
  TEST_ASSERT_EQUAL(reverseState, GPIO_STATE_LOW);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The neutral LED should now be on.\n The drive and reverse LEDs should be off.\n");
  delay_s(5);
#endif

  // Reverse
  Event e3 = { .id = INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE };
  led_output_process_event(&e3);

  gpio_expander_get_state(&s_expander, CC_CFG_DRIVE_LED, &driveState);
  gpio_expander_get_state(&s_expander, CC_CFG_NEUTRAL_LED, &neutralState);
  gpio_expander_get_state(&s_expander, CC_CFG_REVERSE_LED, &reverseState);

  TEST_ASSERT_EQUAL(driveState, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(neutralState, GPIO_STATE_LOW);
  TEST_ASSERT_EQUAL(reverseState, GPIO_STATE_HIGH);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The reverse LED should now be on.\n The drive and neutral LEDs should be off.\n");
  delay_s(5);
#endif
}

void test_led_output_drl(void) {
  Event e = { .id = INPUT_EVENT_CENTER_CONSOLE_DRL };
  GpioState currentState;

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_DRL_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console DRL LED should now be on.\n");
  delay_s(5);
#endif

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_DRL_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console DRL LED should now be off.\n");
  delay_s(5);
#endif
}

void test_led_output_lowbeams(void) {
  Event e = { .id = INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS };
  GpioState currentState;

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_LOW_BEAM_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console lowbeams LED should now be on.\n");
  delay_s(5);
#endif

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_LOW_BEAM_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console lowbeams LED should now be off.\n");
  delay_s(5);
#endif
}

void test_led_output_hazards(void) {
  Event e1 = { .id = INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED };
  GpioState currentState;

  led_output_process_event(&e1);
  gpio_expander_get_state(&s_expander, CC_CFG_HAZARD_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console hazards LED should now be on.\n");
  delay_s(5);
#endif

  Event e2 = { .id = INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED };
  led_output_process_event(&e2);
  gpio_expander_get_state(&s_expander, CC_CFG_HAZARD_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

#ifdef TEST_LED_OUTPUT_PRINT_AND_DELAY
  LOG_DEBUG("The center console lowbeams LED should now be off.\n");
  delay_s(5);
#endif
}
