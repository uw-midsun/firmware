#pragma once

#include <stdint.h>
#include "status.h"

typedef struct ThrottleStorage {
  Ads1015Storage *pedal_ads1015_storage;
  uint16_t callback_counter[2];
  bool reading_updated_flag;
  bool reading_ok_flag;
  Ads1015Channel channels[2];
} ThrottleStorage;