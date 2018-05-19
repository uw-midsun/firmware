#include <stdbool.h>
#include <stdlib.h>

#include "can.h"
#include "charger_cfg.h"
#include "charger_controller.h"
#include "charger_fsm.h"
#include "charger_pin.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "notify.h"
#include "soft_timer.h"
#include "uart.h"
#include "wait.h"

int main(void) {
  // Generic Libraries
  event_queue_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  // CAN
  const CANSettings *can_settings = charger_cfg_load_can_settings();
  CANStorage can_storage;
  CANRxHandler can_rx_handlers[CHARGER_CFG_NUM_CAN_RX_HANDLERS];
  can_init(can_settings, &can_storage, can_rx_handlers, CHARGER_CFG_NUM_CAN_RX_HANDLERS);

  // UART
  UARTSettings *uart_settings = charger_cfg_load_uart_settings();
  UARTStorage uart_storage;
  uart_init(charger_cfg_load_uart_port(), uart_settings, &uart_storage);

  // Charger Cfg
  charger_cfg_init_settings();

  // Charger Pin
  const GPIOAddress pin_addr = charger_cfg_load_charger_pin();
  charger_pin_init(&pin_addr);

  // Charger Controller
  ChargerSettings *charger_settings = charger_cfg_load_settings();
  ChargerCanStatus charger_status;
  charger_controller_init(charger_settings, &charger_status);

  // Notify/Command
  notify_init(charger_settings->can, CHARGER_CFG_SEND_PERIOD_S, CHARGER_CFG_WATCHDOG_PERIOD_S);

  // FSM
  FSM charger_fsm;
  charger_fsm_init(&charger_fsm);

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    do {
      status = event_process(&e);

      // All events are interrupt driven so it is safe to wait while the event_queue is empty.
      // TODO(ELEC-355): This may change based on what happens with the charger pin work as it may
      // be on an ADC/PWM Signal.
      if (status == STATUS_CODE_EMPTY) {
        wait();
        // TODO(ELEC-355): Validate nothing gets stuck here.
      }
    } while (status != STATUS_CODE_OK);

    fsm_process_event(&charger_fsm, &e);
    fsm_process_event(CAN_FSM, &e);
  }

  return EXIT_SUCCESS;
}
