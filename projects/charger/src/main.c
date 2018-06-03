#include <stdbool.h>
#include <stdlib.h>

#include "can.h"
#include "can_transmit.h"
#include "charger_cfg.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "charger_fsm.h"
#include "charger_pin.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "notify.h"
#include "soft_timer.h"
#include "uart.h"
#include "wait.h"

// GenericCanRxCb HACK
static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  CAN_TRANSMIT_CHARGER_SET_RELAY_STATE(EE_CHARGER_SET_RELAY_STATE_CLOSE);
  return STATUS_CODE_OK;
}

int main(void) {
  // Generic Libraries
  event_queue_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  can_interval_init();

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
  const ChargerSettings *charger_settings = charger_cfg_load_settings();
  ChargerCanStatus charger_status = { 0 };
  ChargerStorage charger_storage = { 0 };
  charger_controller_init(&charger_storage, charger_settings, &charger_status);

  // HACK
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CHARGER_CONN_STATE, prv_rx_handler, NULL);
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
        // TODO(ELEC-355): Validate nothing gets stuck here.
      }
    } while (status != STATUS_CODE_OK);
    LOG_DEBUG("Event id: %d data %d\n", e.id, e.data);

    fsm_process_event(&charger_fsm, &e);
    fsm_process_event(CAN_FSM, &e);
  }

  return EXIT_SUCCESS;
}
