#pragma once

#include "imu.h"

// IMU polling rate in seconds
#define IMU_POLL_PERIOD_S 1

// IMU baudrate in nanoseconds
#define IMU_BAUDRATE_NS 1000

// Loads imu settings
ImuSettings *config_load_imu(void);
