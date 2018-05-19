#include "control_stalk.h"
#include "log.h"
#include <string.h>

#define CONTROL_STALK_THRESHOLD(ohms) ((1 << 12) / 2 * (ohms) / ((CONTROL_STALK_RESISTOR) + (ohms)))

// actual recorded: 604
// 2k181 +10% resistor = ~2k4, -10% = 1k963
#define CONTROL_STALK_2181_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(2400)
// 681 +10% resistor = ~750, -10% = 613
// actual recorded: 254
#define CONTROL_STALK_681_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(750)

static void prv_analog_cb(Ads1015Channel channel, void *context) {
  ControlStalk *stalk = context;
  int16_t reading = 0;
  ads1015_read_raw(stalk->ads1015, channel, &reading);

  ControlStalkState state = CONTROL_STALK_STATE_FLOATING;
  if (reading <= CONTROL_STALK_681_OHMS_THRESHOLD) {
    state = CONTROL_STALK_STATE_681_OHMS;
  } else if (reading <= CONTROL_STALK_2181_OHMS_THRESHOLD) {
    state = CONTROL_STALK_STATE_2181_OHMS;
  }

  if (stalk->states[channel] != state) {
    const char *states[] = { "floating", "681 ohms", "2182 ohms" };
    stalk->states[channel] = state;
    printf("C%d changed state: %s (reading %d)\n", channel, states[state], reading);
  }
}

void prv_digital_cb(GpioExpanderPin pin, GPIOState state, void *context) {
  ControlStalk *stalk = context;
  LOG_DEBUG("Pin %d changed state: %d\n", pin, state);
}

StatusCode control_stalk_init(ControlStalk *stalk, Ads1015Storage *ads1015, GpioExpanderStorage *expander) {
  memset(stalk, 0, sizeof(*stalk));
  stalk->ads1015 = ads1015;
  stalk->expander = expander;

  printf("681 thresh: %d\n2181 thresh: %d\n", CONTROL_STALK_681_OHMS_THRESHOLD, CONTROL_STALK_2181_OHMS_THRESHOLD);

  for (Ads1015Channel channel = 0; channel < CONTROL_STALK_ANALOG_INPUTS; channel++) {
    ads1015_configure_channel(stalk->ads1015, channel, true, prv_analog_cb, stalk);
  }

  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_PULLUP,
  };

  for (GpioExpanderPin pin = 0; pin < CONTROL_STALK_DIGITAL_INPUTS; pin++) {
    status_ok_or_return(gpio_expander_init_pin(expander, pin, &gpio_settings));
    status_ok_or_return(gpio_expander_register_callback(expander, pin, prv_digital_cb, stalk));
  }

  return STATUS_CODE_OK;
}
