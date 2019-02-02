#include "cc_cfg.h"
#include "delay.h"
#include "event_queue.h"
#include "i2c.h"
#include "input_event.h"
#include "gpio_expander.h"
#include "led_output.h"
#include "log.h"

GpioExpanderStorage s_expander;

void setup_test(void) {

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = CC_CFG_I2C_BUS_SDA,  //
    .scl = CC_CFG_I2C_BUS_SCL,  //
  };

  i2c_init(CC_CFG_I2C_BUS_PORT, &i2c_settings);

  gpio_expander_init(&s_expander, CC_CFG_I2C_BUS_PORT, CC_CFG_CONSOLE_IO_ADDR,
                    NULL);

  led_output_init(&s_expander);
}

void teardown_test(void) {}

void test_led_output_power(void) {
  Event e = { .id = INPUT_EVENT_CENTER_CONSOLE_POWER };
  GpioState currentState;

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_PWR_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

  LOG_DEBUG("The center console power LED should now be on.\n");
  delay_s(5);

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_PWR_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

  LOG_DEBUG("The center console power LED should now be off.\n");
  delay_s(5);
}

void test_led_output_direction(void) {

}

void test_led_output_drl(void) {
  Event e = { .id = INPUT_EVENT_CENTER_CONSOLE_DRL };
  GpioState currentState;

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_DRL_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

  LOG_DEBUG("The center console DRL LED should now be on.\n");
  delay_s(5);

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_DRL_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

  LOG_DEBUG("The center console DRL LED should now be off.\n");
  delay_s(5);
}

void test_led_output_lowbeams(void) {
  Event e = { .id = INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS };
  GpioState currentState;

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_LOW_BEAM_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

  LOG_DEBUG("The center console lowbeams LED should now be on.\n");
  delay_s(5);

  led_output_process_event(&e);
  gpio_expander_get_state(&s_expander, CC_CFG_LOW_BEAM_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

  LOG_DEBUG("The center console lowbeams LED should now be off.\n");
  delay_s(5);
}

void test_led_output_hazards(void) {
  Event e1 = { .id = INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED };
  GpioState currentState;

  led_output_process_event(&e1);
  gpio_expander_get_state(&s_expander, CC_CFG_HAZARD_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_HIGH);

  LOG_DEBUG("The center console hazards LED should now be on.\n");
  delay_s(5);

  Event e2 = { .id = INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED };
  led_output_process_event(&e2);
  gpio_expander_get_state(&s_expander, CC_CFG_HAZARD_LED, &currentState);
  TEST_ASSERT_EQUAL(currentState, GPIO_STATE_LOW);

  LOG_DEBUG("The center console lowbeams LED should now be off.\n");
  delay_s(5);
}
