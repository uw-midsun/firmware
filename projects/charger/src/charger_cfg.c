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

static CANSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CHARGER,
  .bitrate = CAN_HW_BITRATE_250KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .rx_event = CHARGER_EVENT_CAN_RX,
  .tx_event = CHARGER_EVENT_CAN_TX,
  .fault_event = CHARGER_EVENT_CAN_FAULT,
  .loopback = false,
};

CANSettings *charger_cfg_load_can_settings(void) {
  return &s_can_settings;
}

static UartSettings s_uart_settings = {
  .baudrate = 115200,
  .rx_handler = NULL,
  .context = NULL,
  .tx = { GPIO_PORT_B, 10 },
  .rx = { GPIO_PORT_B, 11 },
  .alt_fn = GPIO_ALTFN_4,
};

UartSettings *charger_cfg_load_uart_settings(void) {
  return &s_uart_settings;
}

UartPort charger_cfg_load_uart_port(void) {
  return UART_PORT_3;
}

GpioAddress charger_cfg_load_charger_pin(void) {
  return ((GpioAddress){ GPIO_PORT_A, 15 });
}

static GenericCanNetwork s_can_storage;
static GenericCanUart s_can_uart_storage;

static ChargerSettings s_charger_settings = {
  .max_voltage = 1512,  // 151.2 V
  .max_current = 1224,  // 122.4 A
  .can = (GenericCan *)&s_can_storage,
  .can_uart = (GenericCan *)&s_can_uart_storage,
};

StatusCode charger_cfg_init_settings(void) {
  status_ok_or_return(generic_can_network_init(&s_can_storage));
  return generic_can_uart_init(&s_can_uart_storage, charger_cfg_load_uart_port());
}

ChargerSettings *charger_cfg_load_settings(void) {
  return &s_charger_settings;
}
