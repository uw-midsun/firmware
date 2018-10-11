#pragma once


#include <stdio.h>
#include <stddef.h>
#include "log.h"
#include "mcp3427.h"

#define SOLAR_MASTER_VOLTAGE_SAMPLE_SIZE 50 

typedef struct SolarMasterSlave {
  Mcp3427Storage *voltage_mcp3427;
  // Mcp3427Storage *temp_mcp3427;
  uint32_t averaging_voltage[SOLAR_MASTER_VOLTAGE_SAMPLE_SIZE];
  uint32_t counter_voltage;
} SolarMasterSlave;

StatusCode solar_master_slave_init(SolarMasterSlave *slave, Mcp3427Storage *mcp3427);
