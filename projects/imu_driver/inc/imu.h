#pragma once
// Imu module written for IMU board
// This module is for communicating accelerometer and gyroscope data
// from the car to the telemetry server for strategic purposes.
#include "gpio.h"
#include "spi.h"

typedef enum gyro_read_axes { GYRO_X = 0, GYRO_Y, GYRO_Z } gyro_read_axes;

typedef enum xl_read_axes { XL_X = 0, XL_Y, XL_Z } xl_read_axes;

// IMU settings structure
typedef struct ImuSettings {
  // Spi settings
  GpioAddress cs;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;

  uint8_t gyro_first_register;
  uint8_t xl_first_register;

  SpiMode mode;
  uint32_t spi_baudrate;

  SpiPort port;
  uint32_t data_send_period;
} ImuSettings;

// IMU storage structure
typedef struct ImuStorage {
  SpiPort port;
  uint32_t read_rate_s;
  uint16_t gyro_reads[3];
  uint16_t xl_reads[3];
  uint8_t gyro_registers[6];
  uint8_t xl_registers[6];
} ImuStorage;

StatusCode imu_init(const ImuSettings *settings, ImuStorage *storage);
