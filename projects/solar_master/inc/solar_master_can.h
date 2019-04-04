#pragma once
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "i2c.h"
#include "solar_master_config.h"
#include "solar_master_current.h"
#include "solar_master_event.h"
#include "solar_master_slave.h"

#define SOLAR_MASTER_TELEMETRY_PERIOD_MS 3000

typedef struct SolarMasterCanStorage {
  SolarMasterConfigBoard board;
  SolarMasterCurrent *current_storage;
  SolarMasterSlave *slave_storage;
  CanStorage can_storage;
} SolarMasterCanStorage;

StatusCode solar_master_can_init(SolarMasterCanStorage *storage, const CanSettings *can_settings,
                                 SolarMasterConfigBoard board);
