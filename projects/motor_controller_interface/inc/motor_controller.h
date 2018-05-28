#pragma once

#include <assert.h>
#include <stdint.h>

// Messages should be sent at least 50ms
#define MOTOR_CONTROLLER_MESSAGE_INTERVAL_US (40 * 1000)

// Note: All CAN messages that are sent to/from the Motor Controllers have 8
// bytes of payload.

// 1 cm/s = (1 cm/s) * (1 m/100 cm)
#define MOTOR_CONTOLLER_VELOCITY_CMPS_TO_MPS(cmps) ((float)(cmps) * ((float)1 / (float)100))
// 1 m/s = (1 m/s) * (100 cm / 1 m)
#define MOTOR_CONTROLLER_VELOCITY_MPS_TO_CMPS(mps) ((float)(mps) * ((float)100 / (float)1))

// 1km/h = (1 km/h) * (1 h/60 min) * (1 min/60 s) * (1000 m/1 km) * (100 cm / 1 m)
#define MOTOR_CONTROLLER_VELOCITY_KMPH_TO_CMPS(kmph)                       \
  ((kmph) * ((float)1) * ((float)1 / (float)60) * ((float)1 / (float)60) * \
   ((float)1000 / (float)1) * ((float)100 / (float)1))

#define MOTOR_CONTROLLER_FORWARD_VELOCITY_MPS ((float)100)
#define MOTOR_CONTROLLER_REVERSE_VELOCITY_MPS ((float)-100)

// TODO(ELEC-388): change this denominator current
#define MOTOR_CONTROLLER_CURRENT_LIMIT 100.0f

#define MOTOR_CONTROLLER_CURRENT_TO_PERCENT(current) ((current) / MOTOR_CONTROLLER_CURRENT_LIMIT)

// 3.1.1 Motor Drive Command
#define MOTOR_CONTROLLER_DRIVE_COMMAND_ID(base_addr) ((base_addr) + 1)
typedef struct {
  float velocity;
  float current_percentage;
} MotorControllerDriveCmd;
static_assert(sizeof(MotorControllerDriveCmd) == 8, "MotorControllerDriveCmd was not 8 bytes");

// 3.1.2 Motor Power Command
#define MOTOR_CONTROLLER_POWER_COMMAND_ID(base_addr) ((base_addr) + 2)
typedef struct {
  uint32_t reserved;
  float current_percentage;
} MotorControllerPowerCmd;
static_assert(sizeof(MotorControllerPowerCmd) == 8, "MotorControllerPowerCmd was not 8 bytes");

// 3.1.3 Reset Command
#define MOTOR_CONTROLLER_RESET_COMMAND_ID(base_addr) ((base_addr) + 3)
typedef struct {
  uint32_t unused[2];
} MotorControllerResetCmd;
static_assert(sizeof(MotorControllerResetCmd) == 8, "MotorControllerResetCmd was not 8 bytes");

// Motor Controller Broadcast Messages
// 3.3.1 Identification Information
#define MOTOR_CONTROLLER_ID_INFO(base_addr) ((base_addr) + 0)
typedef struct {
  char id[4];
  uint32_t serial_num;
} MotorControllerIdInfo;
static_assert(sizeof(MotorControllerIdInfo) == 8, "MotorControllerIdInfo was not 8 bytes");

// 3.3.2 Status Information
#define MOTOR_CONTROLLER_STATUS_ID(base_addr) ((base_addr) + 1)
#define MOTOR_CONTROLLER_STATUS_INTERVAL_MS 200
// bitfield flag options
typedef struct {
  uint16_t limit_flags_bridge_pwm : 1;
  uint16_t limit_flags_motor_current : 1;
  uint16_t limit_flags_velocity : 1;
  uint16_t limit_flags_bus_current : 1;
  uint16_t limit_flags_bus_voltage_upper_limit : 1;
  uint16_t limit_flags_bus_voltage_lower_limit : 1;
  uint16_t limit_flags_heatsink_temperature : 1;
  uint16_t limit_flags_reserved : 9;

  uint16_t error_flags_hw_overcurrent : 1;
  uint16_t error_flags_sw_overcurrent : 1;
  uint16_t error_flags_dc_bus_overvoltage : 1;
  uint16_t error_flags_bad_motor_position_hall_sequence : 1;
  uint16_t error_flags_watchdog_last_reset : 1;
  uint16_t error_flags_config_read_error : 1;
  uint16_t error_flags_15v_undervoltage_lock : 1;
  uint16_t error_flags_reserved : 9;

  uint16_t active_motor;

  uint16_t reserved;
} MotorControllerStatusInfo;
static_assert(sizeof(MotorControllerStatusInfo) == 8, "MotorControllerStatusInfo is not 8 bytes");

// 3.3.3 Bus Measurement
#define MOTOR_CONTROLLER_BUS_MEASUREMENT(base_addr) ((base_addr) + 2)
typedef struct {
  uint32_t voltage;
  uint32_t current;
} MotorControllerBusMeasurement;
static_assert(sizeof(MotorControllerBusMeasurement) == 8,
              "MotorControllerBusMeasurement is not 8 bytes");

// 3.3.4 Velocity Measurement
#define MOTOR_CONTROLLER_VELOCITY_MEASUREMENT(base_addr) ((base_addr) + 3)
typedef struct {
  uint32_t angular_frequency;
  uint32_t vehicle_velocity;
} MotorControllerVelocityMeasurement;
static_assert(sizeof(MotorControllerVelocityMeasurement) == 8,
              "MotorControllerVelocityMeasurement is not 8 bytes");

// 3.3.5 Phase Current Measurement
// Unused
#define MOTOR_CONTROLLER_PHASE_CURRENT_MEASUREMENT(base_addr) ((base_addr) + 4)

// 3.3.6 Motor Voltage Vector Measurement
// Unused
#define MOTOR_CONTROLLER_VOLTAGE_VECTOR_MEASUREMENT(base_addr) ((base_addr) + 5)

// 3.3.7 Motor Current Vector Measurement
// Unused
#define MOTOR_CONTROLLER_CURRENT_VECTOR_MEASUREMENT(base_addr) ((base_addr) + 6)

// 3.3.8 Motor BackEMF Measurement / Prediction
// Unused
#define MOTOR_CONTROLLER_BACK_EMF_MEASUREMENT(base_addr) ((base_addr) + 7)

// 3.3.9 15 & 1.65 Voltage Rail Measurement
// Unused
#define MOTOR_CONTROLLER_15V_1_65V_RAIL_MEASUREMENT(base_addr) ((base_addr) + 8)

// 3.3.10 2.5V & 1.2V Voltage Rail Measurement
// Unused
#define MOTOR_CONTROLLER_2_5V_1_2V_RAIL_MEASUREMENT(base_addr) ((base_addr) + 9)

// 3.3.11 Fan Speed Measurement
// Unused
#define MOTOR_CONTROLLER_FAN_SPEED_MEASUREMENT(base_addr) ((base_addr) + 10)

// 3.3.12 Sink & Motor Temperature Measurement
#define MOTOR_CONTROLLER_SINK_MOTOR_TEMPERATURE_MEASUREMENT(base_addr) ((base_addr) + 11)
typedef struct {
  uint32_t motor_temp;
  uint32_t heatsink_temp;
} MotorControllerSinkMotorTempMeasurement;
static_assert(sizeof(MotorControllerSinkMotorTempMeasurement) == 8,
              "MotorControllerSinkMotorTempMeasurement is not 8 bytes");

// 3.3.13 Air In & CPU Temperature Measurement
// Unused
#define MOTOR_CONTROLLER_AIR_IN_CPU_TEMPERATURE_MEASUREMENT(base_addr) ((base_addr) + 12)

// 3.3.14 Air Out & Cap Temperature Measurement
// Unused
#define MOTOR_CONTROLLER_AIR_OUT_CAP_TEMPERATURE_MEASUREMENT(base_addr) ((base_addr) + 13)

// 3.3.15 Odometer & Bus AmpHours Measurement
#define MOTOR_CONTROLLER_ODOMETER_BUS_AMP_HOURS_MEASUREMENT(base_addr) ((base_addr) + 14)
typedef struct {
  uint32_t odometer;
  uint32_t dc_bus_amp_hours;
} MotorControllerOdometerBusAmpHoursMeasurement;
static_assert(sizeof(MotorControllerOdometerBusAmpHoursMeasurement) == 8,
              "MotorControllerOdometerBusAmpHoursMeasurement is not 8 bytes");
