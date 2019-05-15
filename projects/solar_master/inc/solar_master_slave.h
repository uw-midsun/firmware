#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "log.h"
#include "mcp3427.h"
#include "mcp3427defs.h"
#include "soft_timer.h"

#define SOLAR_MASTER_NUM_SOLAR_SLAVES 6
#define SOLAR_MASTER_SLAVE_REINIT_PERIOD_MS 5000

#define SOLAR_MASTER_MCP3427_SAMPLE_SIZE 5

#define SOLAR_MASTER_VOLTAGE_SCALING_FACTOR 19

// Constants defining voltage dividor for temperature sense
// 5V --> thermistor --> Measurement (mv) --> 10k Ohms --> GND
// R = (5 V) * (10000 Ohms) * 1000 / (Avg Measurement mV)
#define SOLAR_MASTER_TEMP_VOLTAGE 5
#define SOLAR_MASTER_TEMP_RESISTOR 10000 * 1000

// The switches on the ith slave board must be set up to set the Adr0 and Adr1 pins to match
// ADC_ADDRESS_MAP[i]
// See the MCP3427 datasheet section 5.3 for details
static const Mcp3427PinState ADC_ADDRESS_MAP[SOLAR_MASTER_NUM_SOLAR_SLAVES][2] = {
  { MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_LOW },     // I2C Address 0x68 i.e. 0x68 ^ 0x00
  { MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_FLOAT },   // I2C Address 0x69 i.e. 0x68 ^ 0x01
  { MCP3427_PIN_STATE_LOW, MCP3427_PIN_STATE_HIGH },    // I2C Address 0x70 i.e. 0x68 ^ 0x02
  { MCP3427_PIN_STATE_FLOAT, MCP3427_PIN_STATE_LOW },   // I2C Address 0x71 i.e. 0x68 ^ 0x03
  { MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_LOW },    // I2C Address 0x72 i.e. 0x68 ^ 0x04
  { MCP3427_PIN_STATE_HIGH, MCP3427_PIN_STATE_FLOAT },  // I2C Address 0x73 i.e. 0x68 ^ 0x05
};

// Initialize voltage/temp (slave) reading adcs
// Some information must be hardcoded
// Need to coordinate identifying slave modules with driver controls and telemetry
static const uint8_t SLAVE_ADDR_LOOKUP_REVERSE[8] = { 0, 1, 2, 3, 4, 5, 0, 0 };

typedef struct SolarMasterSlave {
  Mcp3427Storage *mcp3427;
  Mcp3427Setting *mcp3427_settings;
  int32_t averaging_voltage[SOLAR_MASTER_MCP3427_SAMPLE_SIZE];
  int32_t averaging_temp[SOLAR_MASTER_MCP3427_SAMPLE_SIZE];
  int32_t sliding_sum_voltage_mv;
  int32_t sliding_sum_temp_mv;
  uint32_t counter;
  bool is_stale;
  bool is_init;
} SolarMasterSlave;

StatusCode solar_master_slave_init(SolarMasterSlave *slave_storage, Mcp3427Storage *mcp3427_storage,
                                   Mcp3427Setting *mcp3427_settings_base);
