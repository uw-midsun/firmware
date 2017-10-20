#pragma once

#include "can_msg_def.h"
#include "can_unpack_impl.h"

#define CAN_UNPACK_BPS_FAULT(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_BATTERY_RELAY(msg_ptr, relay_state_u8_ptr)                             \
  can_unpack_impl_u8((msg_ptr), 1, (relay_state_u8_ptr), CAN_UNPACK_IMPL_EMPTY,           \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_MAIN_RELAY(msg_ptr, relay_state_u8_ptr)                                \
  can_unpack_impl_u8((msg_ptr), 1, (relay_state_u8_ptr), CAN_UNPACK_IMPL_EMPTY,           \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SOLAR_RELAY(msg_ptr, relay_state_u8_ptr)                               \
  can_unpack_impl_u8((msg_ptr), 1, (relay_state_u8_ptr), CAN_UNPACK_IMPL_EMPTY,           \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_POWER_STATE(msg_ptr, power_state_u8_ptr)                               \
  can_unpack_impl_u8((msg_ptr), 1, (power_state_u8_ptr), CAN_UNPACK_IMPL_EMPTY,           \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_OVUV_DCDC_AUX(msg_ptr, dcdc_ov_flag_u8_ptr, dcdc_uv_flag_u8_ptr,             \
                                 aux_bat_ov_flag_u8_ptr, aux_bat_uv_flag_u8_ptr)                \
  can_unpack_impl_u8((msg_ptr), 4, (dcdc_ov_flag_u8_ptr), (dcdc_uv_flag_u8_ptr),                \
                     (aux_bat_ov_flag_u8_ptr), (aux_bat_uv_flag_u8_ptr), CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_MC_ERROR_LIMITS(msg_ptr, error_id_u32_ptr, limits_u32_ptr) \
  can_unpack_impl_u32((msg_ptr), 8, (error_id_u32_ptr), (limits_u32_ptr))

#define CAN_UNPACK_THROTTLE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_MOTOR_CRUISE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_DIRECTION_SELECTOR(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_LIGHTS_STATES(msg_ptr, hazard_u8_ptr, left_turn_u8_ptr, right_turn_u8_ptr, \
                                 brakes_u8_ptr, headlights_u8_ptr)                            \
  can_unpack_impl_u8((msg_ptr), 5, (hazard_u8_ptr), (left_turn_u8_ptr), (right_turn_u8_ptr),  \
                     (brakes_u8_ptr), (headlights_u8_ptr), CAN_UNPACK_IMPL_EMPTY,             \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_HORN(msg_ptr, state_u8_ptr)                                                   \
  can_unpack_impl_u8((msg_ptr), 1, (state_u8_ptr), CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY,        \
                     CAN_UNPACK_IMPL_EMPTY, CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_MECHANICAL_BRAKE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_BATTERY_SOC(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_BATTERY_VCT(msg_ptr, module_id_u16_ptr, voltage_u16_ptr, current_u16_ptr,   \
                               temperature_u16_ptr)                                            \
  can_unpack_impl_u16((msg_ptr), 8, (module_id_u16_ptr), (voltage_u16_ptr), (current_u16_ptr), \
                      (temperature_u16_ptr))

#define CAN_UNPACK_MOTOR_CONTROLLER_VC(msg_ptr, mc_voltage_1_u16_ptr, mc_current_1_u16_ptr, \
                                       mc_voltage_2_u16_ptr, mc_current_2_u16_ptr)          \
  can_unpack_impl_u16((msg_ptr), 8, (mc_voltage_1_u16_ptr), (mc_current_1_u16_ptr),         \
                      (mc_voltage_2_u16_ptr), (mc_current_2_u16_ptr))

#define CAN_UNPACK_MOTOR_VELOCITY(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_MOTOR_CONTROLLER_TEMPS(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_MOTOR_AMP_HR(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_ODOMETER(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_AUX_DCDC_VC(msg_ptr, aux_voltage_u16_ptr, aux_current_u16_ptr, \
                               dcdc_voltage_u16_ptr, dcdc_current_u16_ptr)        \
  can_unpack_impl_u16((msg_ptr), 8, (aux_voltage_u16_ptr), (aux_current_u16_ptr), \
                      (dcdc_voltage_u16_ptr), (dcdc_current_u16_ptr))

#define CAN_UNPACK_DCDC_TEMPS(msg_ptr, temp_1_u16_ptr, temp_2_u16_ptr)                         \
  can_unpack_impl_u16((msg_ptr), 4, (temp_1_u16_ptr), (temp_2_u16_ptr), CAN_UNPACK_IMPL_EMPTY, \
                      CAN_UNPACK_IMPL_EMPTY)

#define CAN_UNPACK_SOLAR_DATA_FRONT(msg_ptr, module_id_u16_ptr, voltage_u16_ptr, current_u16_ptr, \
                                    temperature_u16_ptr)                                          \
  can_unpack_impl_u16((msg_ptr), 8, (module_id_u16_ptr), (voltage_u16_ptr), (current_u16_ptr),    \
                      (temperature_u16_ptr))

#define CAN_UNPACK_SOLAR_DATA_REAR(msg_ptr, module_id_u16_ptr, voltage_u16_ptr, current_u16_ptr, \
                                   temperature_u16_ptr)                                          \
  can_unpack_impl_u16((msg_ptr), 8, (module_id_u16_ptr), (voltage_u16_ptr), (current_u16_ptr),   \
                      (temperature_u16_ptr))

#define CAN_UNPACK_STEERING_ANGLE(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_LINEAR_ACCELERATION(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)

#define CAN_UNPACK_ANGULAR_ROTATION(msg_ptr) can_unpack_impl_empty((msg_ptr), 0)
