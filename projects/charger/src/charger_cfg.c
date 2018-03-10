#include "charger_cfg.h"

#include <stddef.h>

#include "can.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "generic_can_network.h"
#include "generic_can_uart.h"
#include "gpio.h"
#include "status.h"
#include "uart.h"

// TODO(ELEC-355): Fill in the pinouts.

static CANSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CHARGER,
  .bitrate = CAN_HW_BITRATE_250KBPS,
  .tx = { 0, 0 },
  .rx = { 0, 0 },
  .rx_event = CHARGER_EVENT_CAN_RX,
  .tx_event = CHARGER_EVENT_CAN_TX,
  .fault_event = CHARGER_EVENT_CAN_FAULT,
  .loopback = false,
};

CANSettings *charger_cfg_load_can_settings(void) {
  return &s_can_settings;
}

static UARTSettings s_uart_settings = {
  .baudrate = 12500,
  .rx_handler = NULL,
  .context = NULL,
  .tx = { 0, 0 },
  .rx = { 0, 0 },
  .alt_fn = NUM_GPIO_ALTFNS,
};

UARTSettings *charger_cfg_load_uart_settings(void) {
  return &s_uart_settings;
}

UARTPort charger_cfg_load_uart_port(void) {
  return 0;
}

GPIOAddress charger_cfg_load_charger_pin(void) {
  return ((GPIOAddress){ 0, 0 });
}

static GenericCanNetwork s_can_storage;
static GenericCanUart s_can_uart_storage;

static ChargerSettings s_charger_settings = {
  .max_voltage = 0,
  .max_current = 0,
  .can = (GenericCan *)&s_can_storage,
  .can_uart = (GenericCan *)&s_can_uart_storage,
};

StatusCode charger_cfg_init_settings(void) {
  status_ok_or_return(generic_can_network_init(&s_can_storage));
  return generic_can_uart_init(charger_cfg_load_uart_port(), &s_can_uart_storage);
}

ChargerSettings *charger_cfg_load_settings(void) {
  return &s_charger_settings;
}
