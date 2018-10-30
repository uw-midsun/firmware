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
#include "ads1015.h"
#include "mcp3427.h"

#include "solar_master_can.h"
#include "solar_master_config.h"
#include "solar_master_event.h"
#include "solar_master_relay.h"
#include "solar_master_current.h"
#include "solar_master_slave.h"

#define SOLAR_MASTER_I2C_BUS_SDA \
  { GPIO_PORT_B, 11 }

#define SOLAR_MASTER_I2C_BUS_SCL \
  { GPIO_PORT_B, 10 }

#define SOLAR_CURRENT_I2C_BUS_SDA \
  { GPIO_PORT_B, 9 }

#define SOLAR_CURRENT_I2C_BUS_SCL \
  { GPIO_PORT_B, 8 }

static SolarMasterCanStorage s_solar_master_can_storage = { 0 };

static Ads1015Storage s_current_ads1015 = { 0 };
static SolarMasterCurrent s_current_storage = { 0 };

static Mcp3427Storage s_slave_mcp3427 = { 0 };
static SolarMasterSlave s_slave_storage = { 0 };

int main(void) {
  // TODO(ELEC-502): Add I2C high speed support to the driver.
  const I2CSettings slave_i2c_settings = {
    .speed = I2C_SPEED_STANDARD,      //
    .sda = SOLAR_MASTER_I2C_BUS_SDA,  //
    .scl = SOLAR_MASTER_I2C_BUS_SCL,  //
  };

  const I2CSettings current_i2c_settings = {
    .speed = I2C_SPEED_STANDARD,       //
    .sda = SOLAR_CURRENT_I2C_BUS_SDA,  //
    .scl = SOLAR_CURRENT_I2C_BUS_SCL,  //
  };

  const Mcp3427Setting slave_mcp3427_settings = {
    .sample_rate = MCP3427_SAMPLE_RATE_12_BIT,
    .Adr0 = MCP3427_PIN_STATE_FLOAT,
    .Adr1 = MCP3427_PIN_STATE_FLOAT, 
    .amplifier_gain = MCP3427_AMP_GAIN_1,
    .conversion_mode = MCP3427_CONVERSION_MODE_CONTINUOUS,
    .port = I2C_PORT_2,
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

  // Slave i2c bus handles voltage+temp measurements across each slave board,
  // current i2c bus communicates with adc measuring total current per master board
  i2c_init(config->slave_i2c_port, &slave_i2c_settings);
  i2c_init(config->current_i2c_port, &current_i2c_settings);

  // TODO(ELEC-502): unify constants
  // Initialize current sense ADC
  GPIOAddress current_ready_pin = CURRENT_ADC_READY_PIN;
  ads1015_init(&s_current_ads1015, SOLAR_MASTER_CURRENT_I2C_BUS_PORT, SOLAR_MASTER_CURRENT_ADC_ADDR, &current_ready_pin);
  StatusCode status = solar_master_current_init(&s_current_storage, &s_current_ads1015);
  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master Current.\n");
  }

  status = solar_master_relay_init();

  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master Relay.\n");
  }

  // Initialize lights_can.
  can_settings.device_id = device_id_lookup[config->board];

  status = solar_master_can_init(&s_solar_master_can_storage, &can_settings, config->board);

  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master CAN.\n");
  }

  

  // Initialize voltage/temp (slave) reading adc
  //status = mcp3427_init(&s_slave_mcp3427, &slave_mcp3427_settings);
  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master Slave adc.\n");
  }
  //status = solar_master_slave_init(&s_slave_storage, &s_slave_mcp3427);
  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master Slave.\n");
  }

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      solar_master_relay_process_event(&e);
      fsm_process_event(&(s_slave_mcp3427.fsm), &e);
      // TODO(ELEC-502): Add can process event here.
    }
    wait();
  }
  return STATUS_CODE_OK;
}
