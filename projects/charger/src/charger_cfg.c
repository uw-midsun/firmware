#include "charger_cfg.h"

#include <stddef.h>

#include "can.h"
#include "can_msg_defs.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "generic_can_mcp2515.h"
#include "generic_can_network.h"
#include "gpio.h"
#include "mcp2515.h"
#include "spi_mcu.h"
#include "status.h"

static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_CHARGER,
  .bitrate = CAN_HW_BITRATE_250KBPS,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .rx_event = CHARGER_EVENT_CAN_RX,
  .tx_event = CHARGER_EVENT_CAN_TX,
  .fault_event = CHARGER_EVENT_CAN_FAULT,
  .loopback = false,
};

CanSettings *charger_cfg_load_can_settings(void) {
  return &s_can_settings;
}

static Mcp2515Settings s_mcp2515_settings = {
  .spi_port = SPI_PORT_2,
  .baudrate = 750000,
  .mosi = { .port = GPIO_PORT_B, 15 },
  .miso = { .port = GPIO_PORT_B, 14 },
  .sclk = { .port = GPIO_PORT_B, 13 },
  .cs = { .port = GPIO_PORT_B, 12 },
  .int_pin = { .port = GPIO_PORT_A, 8 },
  .loopback = false,
};

Mcp2515Settings *charger_cfg_load_mcp2515_settings(void) {
  return &s_mcp2515_settings;
}

GpioAddress charger_cfg_load_charger_pin(void) {
  return ((GpioAddress){ GPIO_PORT_A, 7 });
}

GpioAddress charger_cfg_load_pilot_pin(void) {
  return ((GpioAddress){ GPIO_PORT_A, 5 });
}

static GenericCanNetwork s_can_storage;
static GenericCanMcp2515 s_can_mcp2515_storage;

static ChargerSettings s_charger_settings = {
  .max_voltage = 1512,  // 151.2 V
  .max_current = 1224,  // 122.4 A
  .can = (GenericCan *)&s_can_storage,
  .can_mcp2515 = (GenericCan *)&s_can_mcp2515_storage,
  .relay_control_pin = { GPIO_PORT_A, 9 },
  .relay_control_pin_secondary = { GPIO_PORT_B, 9 },
  .pilot_select_pin = { GPIO_PORT_A, 2 },
};

StatusCode charger_cfg_init_settings(void) {
  status_ok_or_return(generic_can_network_init(&s_can_storage));
  return generic_can_mcp2515_init(&s_can_mcp2515_storage, charger_cfg_load_mcp2515_settings());
}

ChargerSettings *charger_cfg_load_settings(void) {
  return &s_charger_settings;
}
