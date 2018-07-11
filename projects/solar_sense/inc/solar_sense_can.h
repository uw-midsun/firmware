#pragma once
#include "solar_sense_config.h"
#include "can.h"

typedef struct SolarSenseCanStorage {
  SolarSenseConfigBoard board;
  CANStorage can_storage;
} SolarSenseCanStorage;

StatusCode solar_sense_can_init(SolarSenseCanStorage *storage, const CANSettings *can_settings,
                SolarSenseConfigBoard board);

