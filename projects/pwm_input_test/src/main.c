#include "gpio.h"
#include "pwm_output.h"
#include "pwm_input.h"
#include "wait.h"
#include "interrupt.h"
#include "log.h"
#include "delay.h"
#include "soft_timer.h"

static void prv_test_callback(const Status *status) {
  printf("CODE: %d:%s:%s %s\n", status->code, status->source, status->caller, status->message);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  status_register_callback(prv_test_callback);

  PwmTimer output_timer = PWM_TIMER_3;
  PwmTimer input_timer = PWM_TIMER_1;

  // Pwm output
  GpioAddress output = {
    .port = GPIO_PORT_B,
    .pin = 4,
  };

  GpioSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = GPIO_ALTFN_1
  };

  gpio_init_pin(&output, &output_settings);

  pwm_init(output_timer, 65534);
  pwm_set_dc(output_timer, 0);

  GpioAddress input = {
    .port = GPIO_PORT_A,
    .pin = 9
  };

  GpioSettings input_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = GPIO_ALTFN_2
  };

  gpio_init_pin(&input, &input_settings);
  pwm_input_init(input_timer);

  while(1) {
    for (int i = 0; i < 100; i ++) {
      // For testing that PWM is actually working
      // gpio_get_state(&input, &state);
      // if (state == GPIO_STATE_HIGH) {
      //   debug_led_set_state(DEBUG_LED_RED, false);
      // } else {
      //   debug_led_set_state(DEBUG_LED_RED, true);
      // }
      pwm_set_dc(output_timer, i);
      delay_ms(500);
      LOG_DEBUG("i: %d DC: %d, Period: %d\n", i, (int) pwm_input_get_dc(), (int) pwm_input_get_period());
    }
  }
}
