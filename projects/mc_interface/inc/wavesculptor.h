#pragma once
// WaveSculptor 20 CAN definitions

#define WAVESCULPTOR_FORWARD_VELOCITY 100.0f
#define WAVESCULPTOR_REVERSE_VELOCITY -100.0f

typedef union WaveSculptorCanId {
  struct {
    uint16_t msg_id : 5;
    uint16_t device_id : 6;
  };
  uint16_t raw;
} WaveSculptorCanId;

// Send to WaveSculptor
typedef enum {
  WAVESCULPTOR_CMD_ID_DRIVE = 1,
  WAVESCULPTOR_CMD_ID_POWER = 2,
  WAVESCULPTOR_CMD_ID_RESET = 3,
} WaveSculptorCmdId;

// Driver Controls Base Addr + 1
typedef struct WaveSculptorDriveCmd {
  float motor_velocity_ms;
  float motor_current_percentage;
} WaveSculptorDriveCmd;

// Driver Controls Base Addr + 2
typedef struct WaveSculptorPowerCmd {
  uint32_t reserved;
  float bus_current_percentage;
} WaveSculptorPowerCmd;

// Driver Controls Base Addr + 3
typedef struct WaveSculptorResetCmd {
  uint64_t reserved;
} WaveSculptorResetCmd;

// Receive from WaveSculptor

// We only really care about the bus measurement and velocity measurements
typedef enum {
  WAVESCULPTOR_MEASUREMENT_ID_BUS = 2,
  WAVESCULPTOR_MEASUREMENT_ID_VELOCITY = 3,
} WaveSculptorMeasurementId;

// Driver Controls Base Addr + 2
typedef struct WaveSculptorBusMeasurement {
  float bus_voltage;  // in V
  float bus_current;  // in A
} WaveSculptorBusMeasurement;

// Driver Controls Base Addr + 3
typedef struct WaveSculptorVelocityMeasurement {
  float motor_velocity_rpm;
  float vehicle_velocity_ms;
} WaveSculptorVelocityMeasurement;

typedef union WaveSculptorCanData {
  uint64_t raw;
  WaveSculptorDriveCmd drive_cmd;
  WaveSculptorPowerCmd power_cmd;
  WaveSculptorResetCmd reset_cmd;
  WaveSculptorBusMeasurement bus_measurement;
  WaveSculptorVelocityMeasurement velocity_measurement;
} WaveSculptorCanData;
