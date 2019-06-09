#pragma once

#include "imu.h"

// IMU polling rate in seconds
#define IMU_POLL_PERIOD_S 1

// IMU baudrate in nanoseconds
#define IMU_BAUDRATE_NS 1000

// Gyroscope output head register, data is stored in subsequent 5 registers
#define GYRO_OUTPUT_HEAD_REG 0x22
// Accelerometer output head register, data is stored in subsequent 5 registers
#define XL_OUTPUT_HEAD_REG 0x28

// Loads imu settings
ImuSettings *config_load_imu(void);
