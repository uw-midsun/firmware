#include "pwm.h"

#include <stdint.h>

#include "status.h"

static uint16_t s_period_ms = 0;

StatusCode pwm_init(uint16_t period_ms) {
  if (period_ms == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Period must be greater than 0");
  }

  s_period_ms = period_ms;

  return STATUS_CODE_OK;
}

uint16_t pwm_get_period(void) {
  return s_period_ms;
}

StatusCode pwm_set_pulse(uint16_t pulse_width) {
  if (s_period_ms == 0) {
    return status_msg(STATUS_CODE_UNINITIALIZED, "Pwm must be initialized.");
  } else if (pulse_width > s_period_ms) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Pulse width must be leq period.");
  }
  return STATUS_CODE_OK;
}

StatusCode pwm_set_dc(uint16_t dc) {
  if (dc > 100) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Duty Cycle must be leq 100%.");
  }
  return STATUS_CODE_OK;
}
