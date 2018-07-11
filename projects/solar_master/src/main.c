#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#include "solar_master_can.h"
#include "solar_master_config.h"
#include "solar_master_event.h"
#include "solar_master_relay.h"

#define SOLAR_MASTER_I2C_BUS_SDA \
  { GPIO_PORT_B, 11 }

#define SOLAR_MASTER_I2C_BUS_SCL \
  { GPIO_PORT_B, 10 }

static SolarMasterCanStorage s_solar_master_can_storage = { 0 };

int main(void) {
  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_STANDARD,      //
    .sda = SOLAR_MASTER_I2C_BUS_SDA,  //
    .scl = SOLAR_MASTER_I2C_BUS_SCL,  //
  };

  CANSettings can_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .tx = { .port = GPIO_PORT_A, .pin = 12 },
    .rx = { .port = GPIO_PORT_A, .pin = 11 },
    .rx_event = SOLAR_MASTER_EVENT_CAN_RX,
    .tx_event = SOLAR_MASTER_EVENT_CAN_TX,
    .fault_event = SOLAR_MASTER_EVENT_CAN_FAULT,
    .loopback = false,
  };

  uint16_t device_id_lookup[NUM_SOLAR_MASTER_CONFIG_BOARDS] = {
    [SOLAR_MASTER_CONFIG_BOARD_FRONT] = SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT,
    [SOLAR_MASTER_CONFIG_BOARD_REAR] = SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR,
  };

  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();
  event_queue_init();
  solar_master_config_init();

  SolarMasterConfig *config = solar_master_config_load();

  i2c_init(config->i2c_port, &i2c_settings);

  StatusCode status = solar_master_relay_init();

  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master Relay.\n");
  }

  // Initialize lights_can.
  can_settings.device_id = device_id_lookup[config->board];

  status = solar_master_can_init(&s_solar_master_can_storage, &can_settings, config->board);

  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master CAN.\n");
  }

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      solar_master_relay_process_event(&e);
      // TODO(ELEC-502): Add ADC drivers' FSM process event here.
      // TODO(ELEC-502): Add can process event here.
    }
    wait();
  }
  return STATUS_CODE_OK;
}
