#pragma once
#include "can.h"
#include "solar_master_config.h"

typedef struct SolarMasterCanStorage {
  SolarMasterConfigBoard board;
  CANStorage can_storage;
} SolarMasterCanStorage;

StatusCode solar_master_can_init(SolarMasterCanStorage *storage, const CANSettings *can_settings,
                                SolarMasterConfigBoard board);
