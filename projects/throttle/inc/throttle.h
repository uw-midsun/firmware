#pragma once

#include <stdint.h>
#include "status.h"

typedef struct ThrottleStorage {
  Ads1015Storage *pedal_ads1015_storage;
  bool reading_updated_flag;
  bool reading_ok_flag;
  Ads1015Channel channels[2];
  SoftTimerID raise_event_timer_id;
} ThrottleStorage;