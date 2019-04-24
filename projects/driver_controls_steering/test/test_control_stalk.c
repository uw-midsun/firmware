// Functional test for control stalks - switches debug LEDs as follows:
// - Left turn signal: Blue LED (A)
// - Right turn signal: Blue LED (B)
// - CC Distance +/-: Yellow LED
// - Lane Assist: Red LED
#include "adc.h"
#include "control_stalk.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "sc_input_event.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"
#include "steering_output.h"
#include "exported_enums.h"
#include "can_unpack.h"
#include "can_msg.h"

static ControlStalk s_stalk;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  adc_init(ADC_MODE_CONTINUOUS);
  soft_timer_init();
  event_queue_init();

  TEST_ASSERT_OK(control_stalk_init(&s_stalk));
}

void teardown_test(void) {}

void test_control_stalks_readback(void) {

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

    uint16_t control_stalk_analog_state = 0;
    uint16_t control_stalk_digital_state = 0;

    CanMessage msg = {};

    CAN_UNPACK_STEERING_OUTPUT(&msg, &control_stalk_analog_state, &control_stalk_digital_state);
    wait();
      // Digital pins
      switch (control_stalk_digital_state) {
        // CC_SET
        case EE_CONTROL_STALK_DIGITAL_CC_SET_PRESSED:
          LOG_DEBUG("DIGITAL_CC_SET_PRESSED\n");
          gpio_set_state(&leds[0], GPIO_STATE_LOW);
          break;
        case EE_CONTROL_STALK_DIGITAL_CC_SET_RELEASED:
          LOG_DEBUG("DIGITAL_CC_SET_RELEASED\n");
          gpio_set_state(&leds[0], GPIO_STATE_HIGH);
          break;
        // CC_ON/OFF
        case EE_CONTROL_STALK_DIGITAL_CC_ON:
          LOG_DEBUG("DIGITAL_CC_ON\n");
          gpio_set_state(&leds[1], GPIO_STATE_LOW);
          break;
        case EE_CONTROL_STALK_DIGITAL_CC_OFF:
          LOG_DEBUG("DIGITAL_CC_OFF\n");
          gpio_set_state(&leds[1], GPIO_STATE_HIGH);
          break;
        // CC_LANE_ASSIST
        case EE_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_PRESSED:
          LOG_DEBUG("DIGITAL_CC_LANE_ASSIST_PRESSED\n");
          gpio_set_state(&leds[2], GPIO_STATE_LOW);
          break;
        case EE_CONTROL_STALK_DIGITAL_CC_LANE_ASSIST_RELEASED:
          LOG_DEBUG("DIGITAL_CC_LANE_ASSIST_RELEASED\n");
          gpio_set_state(&leds[2], GPIO_STATE_HIGH);
          break;
        // HIGH_BEAM_FWD
        case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_FWD_PRESSED:
          LOG_DEBUG("DIGITAL_HIGH_BEAM_FWD_PRESSED\n");
          gpio_set_state(&leds[3], GPIO_STATE_LOW);
          break;
        case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_FWD_RELEASED:
          LOG_DEBUG("DIGITAL_HIGH_BEAM_FWD_RELEASED\n");
          gpio_set_state(&leds[3], GPIO_STATE_HIGH);
          break;
        // HIGH_BEAM_BACK
        case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_BACK_PRESSED:
          LOG_DEBUG("DIGITAL_HIGH_BEAM_BACK_PRESSED\n");
          break;
        case EE_CONTROL_STALK_DIGITAL_HIGH_BEAM_BACK_RELEASED:
          LOG_DEBUG("DIGITAL_HIGH_BEAM_BACK_RELEASED\n");
          break;
      }

       // Analog Pins
      switch (control_stalk_analog_state) {
        // TURN_SIGNAL
        case EE_CONTROL_STALK_ANALOG_CC_TURN_SIGNAL_NONE:
          LOG_DEBUG("ANALOG_TURN_SIGNAL_NONE\n");
          gpio_set_state(&leds[0], GPIO_STATE_HIGH);
          gpio_set_state(&leds[1], GPIO_STATE_HIGH);
          break;
        case EE_CONTROL_STALK_ANALOG_CC_TURN_SIGNAL_RIGHT:
          LOG_DEBUG("ANALOG_TURN_SIGNAL_RIGHT\n");
          gpio_set_state(&leds[0], GPIO_STATE_LOW);
          gpio_set_state(&leds[1], GPIO_STATE_HIGH);
          break;
        case EE_CONTROL_STALK_ANALOG_CC_TURN_SIGNAL_LEFT:
          LOG_DEBUG("ANALOG_TURN_SIGNAL_LEFT\n");
          gpio_set_state(&leds[0], GPIO_STATE_HIGH);
          gpio_set_state(&leds[1], GPIO_STATE_LOW);
          break;

        // CC_DISTANCE
        case EE_CONTROL_STALK_ANALOG_DISTANCE_NEUTRAL:
          LOG_DEBUG("ANALOG_DISTANCE_NEUTRAL\n");
          gpio_set_state(&leds[2], GPIO_STATE_HIGH);
          break;
        case EE_CONTROL_STALK_ANALOG_DISTANCE_MINUS:
          LOG_DEBUG("ANALOG_DISTANCE_MINUS\n");
          gpio_set_state(&leds[2], GPIO_STATE_LOW);
          break;
        case EE_CONTROL_STALK_ANALOG_DISTANCE_PLUS:
          LOG_DEBUG("ANALOG_DISTANCE_MINUS\n");
          gpio_set_state(&leds[2], GPIO_STATE_LOW);
          break;

        // CC_SPEED
        case EE_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL:
          LOG_DEBUG("ANALOG_CC_SPEED_NEUTRAL\n");
          break;
        case EE_CONTROL_STALK_ANALOG_CC_SPEED_MINUS:
          LOG_DEBUG("ANALOG_CC_SPEED_MINUS\n");
          break;
        case EE_CONTROL_STALK_ANALOG_CC_SPEED_PLUS:
          LOG_DEBUG("ANALOG_CC_SPEED_PLUS\n");
          break;

        // CC_CANCEL/RESUME
        case EE_CONTROL_STALK_ANALOG_CC_DIGITAL:
          LOG_DEBUG("ANALOG_CC_DIGITAL\n");
          break;
        case EE_CONTROL_STALK_ANALOG_CC_CANCEL:
          LOG_DEBUG("ANALOG_CC_CANCEL\n");
          break;
        case EE_CONTROL_STALK_ANALOG_CC_RESUME:
          LOG_DEBUG("ANALOG_CC_RESUME\n");
          break;

        default:
          break;
      }
  }
}
