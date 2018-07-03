#include "charger_pin.h"

#include <stddef.h>
#include <stdbool.h

#include "adc.h"
#include "charger_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "soft_timer.h"
#include "status.h"

static void prv_poll_value(SoftTimerID id, void *context) {
  GPIOAddress *addr = context;

  ADCChannel chan = NUM_ADC_CHANNELS;
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

StatusCode charger_pin_init(const GPIOAddress *address) {
  const GPIOSettings settings = {
    .state = GPIO_STATE_LOW,
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  status_ok_or_return(gpio_init_pin(address, &settings));

  ADCChannel chan = NUM_ADC_CHANNELS;
  adc_get_channel(*address, &chan);
  adc_set_channel(chan, true);

  return soft_timer_start_millis(CHARGER_PIN_POLL_PERIOD_MS, prv_poll_value, address, NULL);
}
