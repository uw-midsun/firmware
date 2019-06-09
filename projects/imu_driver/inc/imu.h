#pragma once
// IMU module, written for IMU board
// This module is for communicating accelerometer and gyroscope data
// from the car to the telemetry server for strategic purposes.
#include "gpio.h"
#include "spi.h"

// WhoAmI register on IC used for sanity check
#define WHOAMI_REG 0x0f
#define WHOAMI_REG_VAL 0x69

// Registers for sensor output data rate setting (gyroscope and accelerometer)
#define GYRO_ODR_REG 0x11
#define XL_ODR_REG 0x10

// Sensor axes (x, y, and z) - gyroscope and accelerometer
typedef enum imu_sensor_axes {
  IMU_SENSOR_X = 0,
  IMU_SENSOR_Y,
  IMU_SENSOR_Z,
  NUM_IMU_SENSOR_AXES
} imu_sensor_axes;

// IMU settings structure
typedef struct ImuSettings {
  // Spi settings
  GpioAddress cs;
  GpioAddress mosi;
  GpioAddress miso;
  GpioAddress sclk;

  uint8_t gyro_head_register; // First register of gyroscope data
  uint8_t xl_head_register; // first register of accelerometer data

  SpiMode mode;
  uint32_t spi_baudrate;

  SpiPort port;
  uint32_t data_send_period;
} ImuSettings;

// IMU storage structure
typedef struct ImuStorage {
  SpiPort port;
  uint32_t read_rate_s;
  uint16_t gyro_data[NUM_IMU_SENSOR_AXES];
  uint16_t xl_data[NUM_IMU_SENSOR_AXES];
  uint8_t gyro_registers[NUM_IMU_SENSOR_AXES * 2];
  uint8_t xl_registers[NUM_IMU_SENSOR_AXES * 2];
} ImuStorage;

StatusCode imu_init(const ImuSettings *settings, ImuStorage *storage);
