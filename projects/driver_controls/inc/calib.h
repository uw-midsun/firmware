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

// Load any stored data into the calib blob.
StatusCode calib_init(CalibStorage *calib);

// Store any changes to the calib blob.
StatusCode calib_commit(CalibStorage *calib);

// Retrieve persisted calibration data.
CalibBlob *calib_blob(CalibStorage *calib);
