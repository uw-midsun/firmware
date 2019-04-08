#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "pwm_input.h"
#include "pwm.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_test_callback(const Status *status) {
  printf("CODE: %d:%s:%s %s\n", status->code, status->source, status->caller, status->message);
}

volatile uint32_t dc = 0;
volatile uint32_t period = 0;

static void prv_dc_callback(uint32_t local_dc, uint32_t local_period, void *context) {
  dc = local_dc;
  period = local_period;
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  status_register_callback(prv_test_callback);

  PwmTimer output_timer = PWM_TIMER_3;
  PwmTimer input_timer = PWM_TIMER_1;

  PwmInputStorage storage = { 0 };

  PwmInputSettings settings = { .channel = PWM_CHANNEL_1, .callback = prv_dc_callback };

  // Pwm output
  GpioAddress output = {
    .port = GPIO_PORT_B,
    .pin = 4,
  };

  GpioSettings output_settings = { .direction = GPIO_DIR_OUT, .alt_function = GPIO_ALTFN_1 };

  GpioAddress input = { .port = GPIO_PORT_A, .pin = 8 };

  GpioSettings input_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_2 };

  gpio_init_pin(&output, &output_settings);

  pwm_init(output_timer, 65534);
  pwm_set_dc(output_timer, 0);

  gpio_init_pin(&input, &input_settings);
  pwm_input_init(input_timer, &settings, &storage);

  while (1) {
    for (int i = 0; i < 100; i++) {
      pwm_set_dc(output_timer, i);
      delay_ms(500);
      LOG_DEBUG("i: %d DC: %d, ldc %d, Period: %d\n", i, (int)pwm_input_get_dc(input_timer),
                (int)dc, (int)pwm_input_get_period(input_timer));
    }
  }
}
