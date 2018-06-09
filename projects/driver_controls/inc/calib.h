#pragma once
// Calibration persistence - uses the flash persist layer
// Any changes must be explicitly committed.
#include "persist.h"
#include "throttle.h"

// Use the last flash page for calibration persistence
#define CALIB_FLASH_PAGE NUM_FLASH_PAGES - 1

typedef struct CalibBlob {
  ThrottleCalibrationData throttle_calib;
} CalibBlob;

typedef struct CalibStorage {
  PersistStorage persist;
  CalibBlob blob;
} CalibStorage;

StatusCode calib_init(CalibStorage *calib);

StatusCode calib_commit(CalibStorage *calib);

CalibBlob *calib_blob(CalibStorage *calib);
