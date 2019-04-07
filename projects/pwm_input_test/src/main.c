#include "gpio.h"
#include "pwm.h"
#include "pwm_input.h"
#include "wait.h"
#include "interrupt.h"
#include "log.h"
#include "delay.h"
#include "soft_timer.h"
#include "debug_led.h"
#include "gpio.h"

void TIM1_CC_IRQHandler(void) {
  // LOG_DEBUG("lmao\n");
  pwm_input_handle_interrupt();
}

void TIM3_IRQHandler(void) {
  // LOG_DEBUG("lmao\n");
  // TODO: Filter out other interrupts
  pwm_input_handle_interrupt();
}

static void prv_test_callback(const Status *status) {
  printf("CODE: %d:%s:%s %s\n", status->code, status->source, status->caller, status->message);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  status_register_callback(prv_test_callback);
  gpio_init();

// Use port for Green LED
  GpioAddress output = {
    .port = GPIO_PORT_A,
    .pin = 8,
  };

  GpioSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = GPIO_ALTFN_2
  };

  gpio_init_pin(&output, &output_settings);

  // Set a PWM signal of 1000ms with a duty cycle of 50%
  // Should blink for half a second

  // The second parameter is in us
  PwmTimer output_timer = PWM_TIMER_1;
  pwm_init(output_timer, 65534);
  pwm_set_dc(output_timer, 50);

  GpioAddress input = {
    .port = GPIO_PORT_B,
    .pin = 4
  };

  GpioSettings input_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = GPIO_ALTFN_1
  };

  gpio_init_pin(&input, &input_settings);
  pwm_input_init();

  // Use TIM1_CH2 to use pin PA9
  debug_led_init(DEBUG_LED_RED);

  GpioState state = GPIO_STATE_HIGH;
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
      delay_ms(50);
      LOG_DEBUG("i: %d, DC: %d, Period: %d\n", i, (int) pwm_input_get_dc(), (int) pwm_input_get_period());
      
    }
  }
}
