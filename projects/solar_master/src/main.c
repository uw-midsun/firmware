#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ads1015.h"
#include "can.h"
#include "can_msg_defs.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mcp3427.h"
#include "soft_timer.h"
#include "wait.h"

#include "solar_master_can.h"
#include "solar_master_config.h"
#include "solar_master_current.h"
#include "solar_master_event.h"
#include "solar_master_relay.h"
#include "solar_master_slave.h"

static SolarMasterCanStorage s_solar_master_can_storage = { 0 };

static Ads1015Storage s_current_ads1015 = { 0 };
static SolarMasterCurrent s_current_storage = { 0 };

static Mcp3427Storage s_slave_mcp3427[SOLAR_MASTER_NUM_SOLAR_SLAVES] = { 0 };
static SolarMasterSlave s_slave_storage[SOLAR_MASTER_NUM_SOLAR_SLAVES] = { 0 };

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();
  event_queue_init();
  solar_master_config_init();

  SolarMasterConfig *config = solar_master_config_load();

  // Slave i2c bus handles voltage + temp measurements across each slave board,
  // current i2c bus communicates with adc measuring total current per master board
  i2c_init(config->slave_i2c_port, config->slave_i2c_settings);
  i2c_init(config->current_i2c_port, config->current_i2c_settings);

  // Initialize current sense ADC
  GpioAddress current_ready_pin = CURRENT_ADC_READY_PIN;
  ads1015_init(&s_current_ads1015, config->current_i2c_port, SOLAR_MASTER_CURRENT_ADC_ADDR,
               &current_ready_pin);
  StatusCode status = solar_master_current_init(&s_current_storage, &s_current_ads1015);
  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master Current.\n");
  }

  status = solar_master_slave_init(s_slave_storage, s_slave_mcp3427,
                                   config->slave_mcp3427_settings_base);

  status = solar_master_relay_init();
  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master Relay.\n");
  }

  // Initialize solar_master_can.
  s_solar_master_can_storage.current_storage = &s_current_storage;
  s_solar_master_can_storage.slave_storage = s_slave_storage;
  status = solar_master_can_init(&s_solar_master_can_storage, config->can_settings, config->board);

  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing Solar Master CAN.\n");
  }

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      solar_master_relay_process_event(&e);
      // process the FSM event for the appropriate slave ADC
      if (e.data < 8) {
        fsm_process_event(&(s_slave_mcp3427[SLAVE_ADDR_LOOKUP_REVERSE[e.data]].fsm), &e);
      }
    }
    wait();
  }
  return STATUS_CODE_OK;
}
