#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "can.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "status.h"
#include "wait.h"

#include "can_settings.h"
#include "process_event.h"
#include "init_periph.h"
#include "gpio_addresses.h"

const GPIOAddress board_type_address = {
  .port = SOMEPORT,
  .pin = SOMEPIN
};

// figure out whether it's the front board or the back board
StatusCode get_board_type(BoardType * type) {
  GPIOState state;
  StatusCode state_status = gpio_get_state(&board_type_address, &state);
  if ( state_status != STATUS_CODE_OK ) {
    return state_status;
  }
  if (state == GPIO_STATE_LOW) {
    *type = LIGHTS_BOARD_FRONT;
  } else if (state == GPIO_STATE_HIGH) {
    *type = LIGHTS_BOARD_REAR;
  }
  return STATUS_CODE_OK;
}

int main(void) {
  Event e;

  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  // get the Board type (front or back)
  const BoardType boardtype;
  get_board_type(&boardtype);
  initialize_can_settings(boardtype);
  initialize_peripherals(boardtype);

  while (1) {
    StatusCode status = event_process(&e);
    if (status == STATUS_CODE_OK) {
      process_event(e);
    } else if (status == STATUS_CODE_EMPTY) {
      wait();
    }
  }

  return STATUS_CODE_OK;
}
