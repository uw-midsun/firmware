#include "charger_pin.h"

#include <stdbool.h>
#include <stddef.h>

#include "adc.h"
#include "charger_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "soft_timer.h"
#include "status.h"

static void prv_poll_value(SoftTimerId id, void *context) {
  GpioAddress *addr = context;

  AdcChannel chan = NUM_ADC_CHANNELS;
  adc_get_channel(*addr, &chan);
  uint16_t millivolts = UINT16_MAX;
  StatusCode status = adc_read_converted(chan, &millivolts);
  // If the status is bad something happened bad during conversion. Ignore the reading.
  // TODO(ELEC-497): Consider faulting if bad readings happen consistently?
  if (status_ok(status)) {
    if (millivolts < CHARGER_PIN_CONNECTED_THRESHOLD) {
      event_raise(CHARGER_EVENT_CONNECTED, 0);
    } else {
      event_raise(CHARGER_EVENT_DISCONNECTED, 0);
    }
  }

  soft_timer_start_millis(CHARGER_PIN_POLL_PERIOD_MS, prv_poll_value, addr, NULL);
}

static void prv_poll_pilot_pwm(SoftTimerId id, void *context) {
  // Add voltage and current measurements from pilot pin
  uint16_t pwm_voltage = 0;  // duty cycle % in decimal * 12V
  uint16_t pwm_max_current;  // from duty cycle calculation (as per datasheet)

  // Use pwm_max_current to either update max current setting or make comparison

  if (pwm_voltage > 2 && pwm_voltage < 7) {
    event_raise(CHARGER_EVENT_CONNECTED_CHARGING_ALLOWED, 0);
  } else if (pwm_voltage > 8 && pwm_voltage < 10) {
    event_raise(CHARGER_EVENT_CONNECTED_NO_CHARGING_ALLOWED, 0);
  } else if (pwm_voltage > 11 && pwm_voltage < 13) {
    event_raise(CHARGER_EVENT_DISCONNECTED, 0);
  } else {
    event_raise(CHARGER_EVENT_ERROR, 0);
  }
}

StatusCode charger_pin_init(const GpioAddress *address) {
  const GpioSettings settings = {
    .state = GPIO_STATE_LOW,
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  status_ok_or_return(gpio_init_pin(address, &settings));

  AdcChannel chan = NUM_ADC_CHANNELS;
  adc_get_channel(*address, &chan);
  adc_set_channel(chan, true);

  return soft_timer_start_millis(CHARGER_PIN_POLL_PERIOD_MS, prv_poll_value, address, NULL);
}

StatusCode pwm_pin_init(const GpioAddress *address) {
  const GpioSettings settings = {
    .state = GPIO_STATE_LOW,
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  status_ok_or_return(gpio_init_pin(address, &settings));

  AdcChannel chan = NUM_ADC_CHANNELS;
  adc_get_channel(*address, &chan);
  adc_set_channel(chan, true);

  return soft_timer_start_millis(CHARGER_PIN_POLL_PERIOD_MS, prv_poll_pilot_pwm, address, NULL);
}
