#include "solar_master_relay.h"
#include "exported_enums.h"
#include "gpio.h"
#include "solar_master_event.h"
#include "status.h"

// Relay address is the same across all boards.
static const GpioAddress s_relay_address = {
  .port = GPIO_PORT_A,  //
  .pin = 8,             //
};

StatusCode solar_master_relay_init(void) {
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_LOW,         //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };

  event_raise(SOLAR_MASTER_EVENT_RELAY_STATE, EE_CHARGER_SET_RELAY_STATE_CLOSE);
  return gpio_init_pin(&s_relay_address, &settings);
}

StatusCode solar_master_relay_process_event(const Event *e) {
  if (e->id != SOLAR_MASTER_EVENT_RELAY_STATE) {
    return STATUS_CODE_OK;
  }
  return gpio_set_state(&s_relay_address,
                        (e->data == EE_RELAY_STATE_OPEN) ? GPIO_STATE_LOW : GPIO_STATE_HIGH);
}
