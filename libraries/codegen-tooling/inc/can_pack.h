#pragma once

#include "can_msg_def.h"
#include "can_pack_impl.h"

#define CAN_PACK_BPS_FAULT(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_PLUTUS, CAN_MESSAGE_BPS_FAULT, 0)

#define CAN_PACK_BATTERY_RELAY(msg_ptr, relay_state_u8)                                         \
  can_pack_impl_u8((msg_ptr), CAN_DEVICE_CHAOS, CAN_MESSAGE_BATTERY_RELAY, 1, (relay_state_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,               \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,               \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MAIN_RELAY(msg_ptr, relay_state_u8)                                         \
  can_pack_impl_u8((msg_ptr), CAN_DEVICE_CHAOS, CAN_MESSAGE_MAIN_RELAY, 1, (relay_state_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,            \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,            \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_RELAY(msg_ptr, relay_state_u8)                                         \
  can_pack_impl_u8((msg_ptr), CAN_DEVICE_CHAOS, CAN_MESSAGE_SOLAR_RELAY, 1, (relay_state_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,             \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_POWER_STATE(msg_ptr, power_state_u8)                                 \
  can_pack_impl_u8((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_POWER_STATE, 1, \
                   (power_state_u8), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,        \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,     \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_OVUV_DCDC_AUX(msg_ptr, dcdc_ov_flag_u8, dcdc_uv_flag_u8, aux_bat_ov_flag_u8,    \
                               aux_bat_uv_flag_u8)                                               \
  can_pack_impl_u8((msg_ptr), CAN_DEVICE_CHAOS, CAN_MESSAGE_OVUV_DCDC_AUX, 4, (dcdc_ov_flag_u8), \
                   (dcdc_uv_flag_u8), (aux_bat_ov_flag_u8), (aux_bat_uv_flag_u8),                \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,                \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MC_ERROR_LIMITS(msg_ptr, error_id_u32, limits_u32)                         \
  can_pack_impl_u32((msg_ptr), CAN_DEVICE_MOTOR_CONTROLLER, CAN_MESSAGE_MC_ERROR_LIMITS, 8, \
                    (error_id_u32), (limits_u32))

#define CAN_PACK_THROTTLE(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_THROTTLE, 0)

#define CAN_PACK_MOTOR_CRUISE(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_MOTOR_CRUISE, 0)

#define CAN_PACK_DIRECTION_SELECTOR(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_DIRECTION_SELECTOR, 0)

#define CAN_PACK_LIGHTS_STATES(msg_ptr, hazard_u8, left_turn_u8, right_turn_u8, brakes_u8,     \
                               headlights_u8)                                                  \
  can_pack_impl_u8((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_LIGHTS_STATES, 5,        \
                   (hazard_u8), (left_turn_u8), (right_turn_u8), (brakes_u8), (headlights_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_HORN(msg_ptr, state_u8)                                                   \
  can_pack_impl_u8((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_HORN, 1, (state_u8), \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,          \
                   CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY,          \
                   CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_MECHANICAL_BRAKE(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_MECHANICAL_BRAKE, 0)

#define CAN_PACK_BATTERY_SOC(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_PLUTUS, CAN_MESSAGE_BATTERY_SOC, 0)

#define CAN_PACK_BATTERY_VCT(msg_ptr, module_id_u16, voltage_u16, current_u16, temperature_u16) \
  can_pack_impl_u16((msg_ptr), CAN_DEVICE_PLUTUS, CAN_MESSAGE_BATTERY_VCT, 8, (module_id_u16),  \
                    (voltage_u16), (current_u16), (temperature_u16))

#define CAN_PACK_MOTOR_CONTROLLER_VC(msg_ptr, mc_voltage_1_u16, mc_current_1_u16,               \
                                     mc_voltage_2_u16, mc_current_2_u16)                        \
  can_pack_impl_u16((msg_ptr), CAN_DEVICE_MOTOR_CONTROLLER, CAN_MESSAGE_MOTOR_CONTROLLER_VC, 8, \
                    (mc_voltage_1_u16), (mc_current_1_u16), (mc_voltage_2_u16),                 \
                    (mc_current_2_u16))

#define CAN_PACK_MOTOR_VELOCITY(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_MOTOR_CONTROLLER, CAN_MESSAGE_MOTOR_VELOCITY, 0)

#define CAN_PACK_MOTOR_CONTROLLER_TEMPS(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_MOTOR_CONTROLLER, CAN_MESSAGE_MOTOR_CONTROLLER_TEMPS, 0)

#define CAN_PACK_MOTOR_AMP_HR(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_MOTOR_CONTROLLER, CAN_MESSAGE_MOTOR_AMP_HR, 0)

#define CAN_PACK_ODOMETER(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_MOTOR_CONTROLLER, CAN_MESSAGE_ODOMETER, 0)

#define CAN_PACK_AUX_DCDC_VC(msg_ptr, aux_voltage_u16, aux_current_u16, dcdc_voltage_u16,       \
                             dcdc_current_u16)                                                  \
  can_pack_impl_u16((msg_ptr), CAN_DEVICE_CHAOS, CAN_MESSAGE_AUX_DCDC_VC, 8, (aux_voltage_u16), \
                    (aux_current_u16), (dcdc_voltage_u16), (dcdc_current_u16))

#define CAN_PACK_DCDC_TEMPS(msg_ptr, temp_1_u16, temp_2_u16)                              \
  can_pack_impl_u16((msg_ptr), CAN_DEVICE_CHAOS, CAN_MESSAGE_DCDC_TEMPS, 4, (temp_1_u16), \
                    (temp_2_u16), CAN_PACK_IMPL_EMPTY, CAN_PACK_IMPL_EMPTY)

#define CAN_PACK_SOLAR_DATA_FRONT(msg_ptr, module_id_u16, voltage_u16, current_u16,            \
                                  temperature_u16)                                             \
  can_pack_impl_u16((msg_ptr), CAN_DEVICE_SOLAR_MASTER_FRONT, CAN_MESSAGE_SOLAR_DATA_FRONT, 8, \
                    (module_id_u16), (voltage_u16), (current_u16), (temperature_u16))

#define CAN_PACK_SOLAR_DATA_REAR(msg_ptr, module_id_u16, voltage_u16, current_u16,           \
                                 temperature_u16)                                            \
  can_pack_impl_u16((msg_ptr), CAN_DEVICE_SOLAR_MASTER_REAR, CAN_MESSAGE_SOLAR_DATA_REAR, 8, \
                    (module_id_u16), (voltage_u16), (current_u16), (temperature_u16))

#define CAN_PACK_STEERING_ANGLE(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_DRIVER_CONTROLS, CAN_MESSAGE_STEERING_ANGLE, 0)

#define CAN_PACK_LINEAR_ACCELERATION(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_SENSOR_BOARD, CAN_MESSAGE_LINEAR_ACCELERATION, 0)

#define CAN_PACK_ANGULAR_ROTATION(msg_ptr) \
  can_pack_impl_empty((msg_ptr), CAN_DEVICE_SENSOR_BOARD, CAN_MESSAGE_ANGULAR_ROTATION, 0)
