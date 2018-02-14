#include <stdint.h>

#include "adc.h"
#include "can.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "lights_periph.h"
#include "status.h"
#include "wait.h"

#include "can_setup.h"
#include "process_event.h"

#define SOMEPORT 0
#define SOMEPIN 0

const GPIOAddress s_board_type_address = { .port = SOMEPORT, .pin = SOMEPIN };

// figure out whether it's the front board or the back board
StatusCode prv_get_board_type(BoardType* type) {
  GPIOState state;
  StatusCode state_status = gpio_get_state(&s_board_type_address, &state);
  if (state_status != STATUS_CODE_OK) {
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
  prv_get_board_type(&boardtype);
  can_setup_init(boardtype);
  lights_periph_init(boardtype);

  while (true) {
    StatusCode status = event_process(&e);
    if (status == STATUS_CODE_OK) {
      process_event(e);
    } else if (status == STATUS_CODE_EMPTY) {
      wait();
    }
  }

  return STATUS_CODE_OK;
}

