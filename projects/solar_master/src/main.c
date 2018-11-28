#include "can.h"
#include "gpio.h"
#include "event_queue.h"
#include "interrupt.h"
#include "can_unpack.h"
#include "wait.h"

#define SOLAR_MASTER_CFG_ADDR_PIN { .port = GPIO_PORT_A, .pin = 9 }
#define SOLAR_MASTER_CFG_RELAY_GPIO { .port = GPIO_PORT_B, .pin = 8 }
#define SOLAR_MASTER_CFG_CAN_BITRATE CAN_HW_BITRATE_500KBPS

typedef enum {
  SOLAR_MASTER_EVENT_CAN_TX,
  SOLAR_MASTER_EVENT_CAN_RX,
  SOLAR_MASTER_EVENT_CAN_FAULT,
  NUM_SOLAR_MASTER_EVENTS,
} SolarMasterEvent;

typedef struct SolarMasterConfig {
  SystemCanDevice device_id;
  SystemCanMessage relay_msg;
} SolarMasterConfig;

typedef enum {
  SOLAR_MASTER_BOARD_FRONT,
  SOLAR_MASTER_BOARD_REAR,
  NUM_SOLAR_MASTER_BOARDS,
} SolarMasterBoard;

static const SolarMasterConfig s_config[NUM_SOLAR_MASTER_BOARDS] = {
  [SOLAR_MASTER_BOARD_FRONT] = { .device_id = SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT, .relay_msg = SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT, },
  [SOLAR_MASTER_BOARD_REAR] = { .device_id = SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR, .relay_msg = SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR, },
};

static CANStorage s_can_storage;

static StatusCode prv_handle_relay_msg(const CanMessage *msg, void *context, CANAckStatus *ack_reply) {
  GpioAddress *relay_addr = context;

  uint8_t relay_state = 0;
  CAN_UNPACK_SOLAR_RELAY_FRONT(msg, &relay_state);
  gpio_set_state(relay_addr, relay_state);

  return STATUS_CODE_OK;
}

int main(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  const GpioSettings addr_settings = { .direction = GPIO_DIR_IN };
  GpioAddress addr_pin = SOLAR_MASTER_CFG_ADDR_PIN;
  GpioState addr_state = NUM_GPIO_STATES;
  gpio_init_pin(&addr_pin, &addr_settings);
  gpio_get_state(&addr_pin, &addr_state);

  LOG_DEBUG("Solar master: %s\n", addr_state ? "Rear" : "Front");

  CANSettings can_settings = {
    .device_id = s_config[addr_state].device_id,
    .bitrate = SOLAR_MASTER_CFG_CAN_BITRATE,
    .rx_event = SOLAR_MASTER_EVENT_CAN_RX,
    .tx_event = SOLAR_MASTER_EVENT_CAN_TX,
    .fault_event = SOLAR_MASTER_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);
  can_add_filter(s_config[addr_state].relay_msg);
  can_register_rx_handler(s_config[addr_state].relay_msg, prv_handle_relay_msg, &addr_pin);

  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
    wait();
  }

  return 0;
}