// Functional test for control stalks - switches debug LEDs as follows:
// - Left turn signal: Blue LED (A)
// - Right turn signal: Blue LED (B)
// - CC Distance +/-: Yellow LED
// - Lane Assist: Red LED
#include "control_stalk.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"

#define TEST_CONTROL_STALK_I2C_PORT I2C_PORT_2

static Ads1015Storage s_ads1015;
static GpioExpanderStorage s_expander;
static ControlStalk s_stalk;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                    //
    .scl = { .port = GPIO_PORT_B, .pin = 10 },  //
    .sda = { .port = GPIO_PORT_B, .pin = 11 },  //
  };
  i2c_init(TEST_CONTROL_STALK_I2C_PORT, &i2c_settings);

  GpioAddress ready_pin = {
    .port = GPIO_PORT_A,  //
    .pin = 9,             //
  };
  ads1015_init(&s_ads1015, TEST_CONTROL_STALK_I2C_PORT, ADS1015_ADDRESS_GND, &ready_pin);

  GpioAddress int_pin = {
    .port = GPIO_PORT_A,  //
    .pin = 8,             //
  };
  gpio_expander_init(&s_expander, TEST_CONTROL_STALK_I2C_PORT, GPIO_EXPANDER_ADDRESS_0, &int_pin);

  TEST_ASSERT_OK(control_stalk_init(&s_stalk, &s_ads1015, &s_expander));
}

void teardown_test(void) {}

void test_control_stalks_readback(void) {
  Event e;

  const GpioAddress leds[] = {
    { .port = GPIO_PORT_B, .pin = 5 },   //
    { .port = GPIO_PORT_B, .pin = 4 },   //
    { .port = GPIO_PORT_B, .pin = 3 },   //
    { .port = GPIO_PORT_A, .pin = 15 },  //
  };

  GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };

  // Init all of the LED pins
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  while (true) {
    wait();
    while (status_ok(event_process(&e))) {
      LOG_DEBUG("Processing event %d, data %d\n", e.id, e.data);

      switch (e.id) {
        case INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE:
          gpio_set_state(&leds[0], GPIO_STATE_HIGH);
          gpio_set_state(&leds[1], GPIO_STATE_HIGH);
          break;
        case INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT:
          gpio_set_state(&leds[0], GPIO_STATE_LOW);
          gpio_set_state(&leds[1], GPIO_STATE_HIGH);
          break;
        case INPUT_EVENT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT:
          gpio_set_state(&leds[0], GPIO_STATE_HIGH);
          gpio_set_state(&leds[1], GPIO_STATE_LOW);
          break;
        case INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL:
          gpio_set_state(&leds[2], GPIO_STATE_HIGH);
          break;
        case INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_MINUS:
        case INPUT_EVENT_CONTROL_STALK_ANALOG_DISTANCE_PLUS:
          gpio_set_state(&leds[2], GPIO_STATE_LOW);
          break;
        case INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_RELEASED:
          gpio_set_state(&leds[3], GPIO_STATE_HIGH);
          break;
        case INPUT_EVENT_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED:
          gpio_set_state(&leds[3], GPIO_STATE_LOW);
          break;
        default:
          break;
      }
    }
  }
}
