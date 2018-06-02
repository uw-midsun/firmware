#include <stdint.h>

#include "can.h"
#include "gpio.h"
#include "interrupt.h"
#include "status.h"
#include "wait.h"

#include "lights_blinker.h"
#include "lights_can.h"
#include "lights_can_config.h"
#include "lights_gpio.h"
#include "lights_gpio_config.h"
#include "lights_config.h"
#include "lights_signal_fsm.h"

int main(void) {
  // Initialize the libraries.
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  // Get board's configuration.
  lights_config_init();
  LightsConfig *config = lights_config_load();

  // Initialize lights_gpio.
  lights_gpio_init(config->lights_gpio);

  // Initialize lights_can.
  lights_can_init(config->lights_can_storage, config->lights_can_settings, config->can_settings);

  // Initialize lights_signal_fsm.
  lights_signal_fsm_init(config->signal_fsm, config->signal_blinker_duration, config->sync_count);


  while (event_process(&e) != STATUS_CODE_OK) {
    const Event raised_event = {
      .id = e.id,
      .data = e.data
    };
    lights_can_process_event(&raised_event);
    lights_gpio_process_event(lights_gpio_config, &raised_event);
    lights_signal_fsm_process_event(&signal_fsm, &raised_event);

  }
  return STATUS_CODE_OK;
}

