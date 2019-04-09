#include "pwm_input.h"

#include <stdint.h>

StatusCode pwm_input_init(PwmTimer timer, PwmInputSettings *settings, PwmInputStorage *storage) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

uint32_t pwm_input_get_period(PwmTimer timer) {
  status_code(STATUS_CODE_UNIMPLEMENTED);
  return 0;
}

uint32_t pwm_input_get_dc(PwmTimer timer) {
  status_code(STATUS_CODE_UNIMPLEMENTED);
  return 0;
}
