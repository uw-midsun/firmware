#pragma once


#include <stdio.h>
#include <stddef.h>
#include "log.h"
#include "mcp3427.h"

#define SOLAR_MASTER_NUM_SOLAR_SLAVES 1

#define SOLAR_MASTER_MCP3427_SAMPLE_SIZE 5 

#define SOLAR_MASTER_VOLTAGE_SCALING_FACTOR 19
#define SOLAR_MASTER_TEMP_SCALING_FACTOR 1
typedef struct SolarMasterSlave {
  Mcp3427Storage *mcp3427;
  int32_t averaging_voltage[SOLAR_MASTER_MCP3427_SAMPLE_SIZE];
  int32_t averaging_temp[SOLAR_MASTER_MCP3427_SAMPLE_SIZE];
  int32_t sliding_sum_voltage;
  int32_t sliding_sum_temp;
  uint32_t counter;
} SolarMasterSlave;

StatusCode solar_master_slave_init(SolarMasterSlave *slave, Mcp3427Storage *mcp3427);
