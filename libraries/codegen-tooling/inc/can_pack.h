#pragma once

#include "can_msg_defs.h"
#include "can_pack_impl.h"

#define CAN_PACK_BPS_HEARTBEAT(msg_ptr, status_u8)                                             \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, 1,   \
                   (status_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,              \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BATTERY_RELAY(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_BATTERY_RELAY, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,              \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,           \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MAIN_RELAY(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_MAIN_RELAY, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,           \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,        \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_RELAY_REAR(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                 \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,              \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_RELAY_FRONT(msg_ptr, relay_state_u8)                                     \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT, 1, \
                   (relay_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,               \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWER_STATE(msg_ptr, power_state_u8)                                            \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS, SYSTEM_CAN_MESSAGE_POWER_STATE, \
                   1, (power_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWERTRAIN_HEARTBEAT(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT)

#define CAN_PACK_OVUV_DCDC_AUX(msg_ptr, dcdc_ov_flag_u8, dcdc_uv_flag_u8, aux_bat_ov_flag_u8, \
                               aux_bat_uv_flag_u8)                                            \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_OVUV_DCDC_AUX, 4,   \
                   (dcdc_ov_flag_u8), (dcdc_uv_flag_u8), (aux_bat_ov_flag_u8),                \
                   (aux_bat_uv_flag_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,            \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MC_ERROR_LIMITS(msg_ptr, error_id_u16, limits_u16)                      \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                       \
                    SYSTEM_CAN_MESSAGE_MC_ERROR_LIMITS, 4, (error_id_u16), (limits_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MOTOR_CONTROLS(msg_ptr, throttle_u16, direction_u16, cruise_control_u16,  \
                                mechanical_brake_state_u16)                                \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,                          \
                    SYSTEM_CAN_MESSAGE_MOTOR_CONTROLS, 8, (throttle_u16), (direction_u16), \
                    (cruise_control_u16), (mechanical_brake_state_u16))

#define CAN_PACK_LIGHTS_STATES(msg_ptr, light_id_u8, light_state_u8)                               \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS, SYSTEM_CAN_MESSAGE_LIGHTS_STATES, \
                   2, (light_id_u8), (light_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,   \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_HORN(msg_ptr, state_u8)                                                      \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS, SYSTEM_CAN_MESSAGE_HORN, 1,  \
                   (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_CHARGER_CONN_STATE(msg_ptr, is_connected_u8)                                      \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHARGER, SYSTEM_CAN_MESSAGE_CHARGER_CONN_STATE, 1, \
                   (is_connected_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                    \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_CHARGER_SET_RELAY_STATE(msg_ptr, state_u8)                                        \
  can_pack_impl_u8((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_CHARGER_SET_RELAY_STATE, \
                   1, (state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,   \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                  \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_STEERING_ANGLE(msg_ptr, steering_angle_u16)                    \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,               \
                    SYSTEM_CAN_MESSAGE_STEERING_ANGLE, 2, (steering_angle_u16), \
                    CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_BATTERY_SOC(msg_ptr) \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_BATTERY_SOC)

#define CAN_PACK_BATTERY_VCT(msg_ptr, module_id_u16, voltage_u16, current_u16, temperature_u16) \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_MESSAGE_BATTERY_VCT, 8,     \
                    (module_id_u16), (voltage_u16), (current_u16), (temperature_u16))

#define CAN_PACK_MOTOR_CONTROLLER_VC(msg_ptr, mc_voltage_1_u16, mc_current_1_u16,  \
                                     mc_voltage_2_u16, mc_current_2_u16)           \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                 \
                    SYSTEM_CAN_MESSAGE_MOTOR_CONTROLLER_VC, 8, (mc_voltage_1_u16), \
                    (mc_current_1_u16), (mc_voltage_2_u16), (mc_current_2_u16))

#define CAN_PACK_MOTOR_VELOCITY(msg_ptr, vehicle_velocity_left_u32, vehicle_velocity_right_u32) \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                              \
                    SYSTEM_CAN_MESSAGE_MOTOR_VELOCITY, 8, (vehicle_velocity_left_u32),          \
                    (vehicle_velocity_right_u32))

#define CAN_PACK_MOTOR_ANGULAR_FREQUENCY(msg_ptr, angular_freq_left_u32, angular_freq_right_u32) \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,                               \
                    SYSTEM_CAN_MESSAGE_MOTOR_ANGULAR_FREQUENCY, 8, (angular_freq_left_u32),      \
                    (angular_freq_right_u32))

#define CAN_PACK_MOTOR_TEMPS(msg_ptr, motor_temp_l_u32, motor_temp_r_u32)                          \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, SYSTEM_CAN_MESSAGE_MOTOR_TEMPS, \
                    8, (motor_temp_l_u32), (motor_temp_r_u32))

#define CAN_PACK_MOTOR_AMP_HR(msg_ptr, motor_amp_hr_l_u32, motor_amp_hr_r_u32) \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,             \
                    SYSTEM_CAN_MESSAGE_MOTOR_AMP_HR, 8, (motor_amp_hr_l_u32),  \
                    (motor_amp_hr_r_u32))

#define CAN_PACK_ODOMETER(msg_ptr, odometer_val_u32)                                               \
  can_pack_impl_u32((msg_ptr), SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER, SYSTEM_CAN_MESSAGE_ODOMETER, 4, \
                    (odometer_val_u32), CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_AUX_DCDC_VC(msg_ptr, aux_voltage_u16, aux_current_u16, dcdc_voltage_u16,  \
                             dcdc_current_u16)                                             \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_AUX_DCDC_VC, 8, \
                    (aux_voltage_u16), (aux_current_u16), (dcdc_voltage_u16), (dcdc_current_u16))

#define CAN_PACK_DCDC_TEMPS(msg_ptr, temp_1_u16, temp_2_u16)                              \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_MESSAGE_DCDC_TEMPS, 4, \
                    (temp_1_u16), (temp_2_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_DATA_FRONT(msg_ptr, module_id_u16, voltage_u16, current_u16,         \
                                  temperature_u16)                                          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT,                        \
                    SYSTEM_CAN_MESSAGE_SOLAR_DATA_FRONT, 8, (module_id_u16), (voltage_u16), \
                    (current_u16), (temperature_u16))

#define CAN_PACK_SOLAR_DATA_REAR(msg_ptr, module_id_u16, voltage_u16, current_u16,         \
                                 temperature_u16)                                          \
  can_pack_impl_u16((msg_ptr), SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR,                        \
                    SYSTEM_CAN_MESSAGE_SOLAR_DATA_REAR, 8, (module_id_u16), (voltage_u16), \
                    (current_u16), (temperature_u16))

#define CAN_PACK_LINEAR_ACCELERATION(msg_ptr)                    \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_SENSOR_BOARD, \
                      SYSTEM_CAN_MESSAGE_LINEAR_ACCELERATION)

#define CAN_PACK_ANGULAR_ROTATION(msg_ptr)                       \
  can_pack_impl_empty((msg_ptr), SYSTEM_CAN_DEVICE_SENSOR_BOARD, \
                      SYSTEM_CAN_MESSAGE_ANGULAR_ROTATION)
