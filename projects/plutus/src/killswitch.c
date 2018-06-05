#include "killswitch.h"
#include "gpio_it.h"

static void prv_killswitch_handler(const GPIOAddress *address, void *context) {
  BpsHeartbeatStorage *storage = context;

  GPIOState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);

  if (state == GPIO_STATE_LOW) {
    // Falling edge - killswitch was hit
    bps_heartbeat_raise_fault(storage, BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  } else {
    // Rising edge - killswitch was released
    bps_heartbeat_clear_fault(storage, BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  }
}

StatusCode killswitch_init(const GPIOAddress *killswitch, BpsHeartbeatStorage *bps_heartbeat) {
  const GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
  };
  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  // Active-low interrupt
  gpio_init_pin(killswitch, &gpio_settings);

  // Force update
  prv_killswitch_handler(killswitch, bps_heartbeat);
  return gpio_it_register_interrupt(killswitch, &it_settings, INTERRUPT_EDGE_RISING_FALLING,
                                    prv_killswitch_handler, bps_heartbeat);
}

StatusCode killswitch_bypass(const GPIOAddress *killswitch) {
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };

  // Force high to bypass killswitch
  return gpio_init_pin(killswitch, &gpio_settings);
}
