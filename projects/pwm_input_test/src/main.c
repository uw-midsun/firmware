#include "gpio.h"
#include "pwm.h"
#include "pwm_input.h"
#include "wait.h"
#include "interrupt.h"
#include "log.h"
#include "delay.h"
#include "soft_timer.h"

void TIM1_CC_IRQHandler(void) {
  // LOG_DEBUG("lmao\n");
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
    .port = GPIO_PORT_B,
    .pin = 4,
  };

  GpioSettings output_settings = {
    .direction = GPIO_DIR_OUT,
    .alt_function = GPIO_ALTFN_1
  };

  gpio_init_pin(&output, &output_settings);

  // Set a PWM signal of 1000ms with a duty cycle of 50%
  // Should blink for half a second

  // The second parameter is in us
  pwm_init(PWM_TIMER_3, 10000);
  pwm_set_dc(PWM_TIMER_3, 95);

  GpioAddress input = {
    .port = GPIO_PORT_A,
    .pin = 9
  };

  GpioSettings input_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = GPIO_ALTFN_2
  };

  gpio_init_pin(&input, &input_settings);
  pwm_input_init();

  // Use TIM1_CH2 to use pin PA9

  // Pray this works
  // status_msg(STATUS_CODE_EMPTY, "Test\n");

  GpioState state = GPIO_STATE_LOW;
  for (;;) {
    // For testing that PWM is actually working
    // gpio_get_state(&input, &state);
    // if (state == GPIO_STATE_HIGH) {
    //   LOG_DEBUG("High\n");
    // } else {
    //   LOG_DEBUG("Low\n");
    // }
    LOG_DEBUG("DC: %d, Period: %d\n", (int) pwm_input_get_dc(), (int) pwm_input_get_period());
    delay_ms(500);
  }

}
