#include <stdbool.h>
#include <stdlib.h>

#include "can.h"
#include "can_interval.h"
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

static CANStorage s_can_storage;
static UartStorage s_uart_storage;
static ChargerCanStatus s_charger_status;
static ChargerStorage s_charger_storage;
static FSM s_charger_fsm;

// TODO(ELEC-355): Add support for polling the ADC/PWM signal from the charging station.

int main(void) {
  // Generic Libraries
  event_queue_init();
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  can_interval_init();

  // CAN
  const CanSettings *can_settings = charger_cfg_load_can_settings();
  can_init(&s_can_storage, can_settings);

  // UART
  UartSettings *uart_settings = charger_cfg_load_uart_settings();
  uart_init(charger_cfg_load_uart_port(), uart_settings, &s_uart_storage);

  // Charger Cfg
  charger_cfg_init_settings();

  // Charger Pin
  const GpioAddress pin_addr = charger_cfg_load_charger_pin();
  charger_pin_init(&pin_addr);

  // Charger Controller
  const ChargerSettings *charger_settings = charger_cfg_load_settings();
  charger_controller_init(&s_charger_storage, charger_settings, &s_charger_status);

  // Notify/Command
  notify_init(charger_settings->can, CHARGER_CFG_SEND_PERIOD_S, CHARGER_CFG_WATCHDOG_PERIOD_S);

  // FSM
  charger_fsm_init(&s_charger_fsm);

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    do {
      status = event_process(&e);

      // All events are interrupt driven so it is safe to wait while the event_queue is empty.
      if (status == STATUS_CODE_EMPTY) {
        wait();
      }
    } while (status != STATUS_CODE_OK);

    fsm_process_event(&s_charger_fsm, &e);
    can_process_event(&e);
  }

  return EXIT_SUCCESS;
}
