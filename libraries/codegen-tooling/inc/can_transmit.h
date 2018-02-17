#pragma once

#include <stddef.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_pack.h"

#define CAN_TRANSMIT_BPS_FAULT(ack_ptr)                \
  ({                                                   \
    CANMessage msg = { 0 };                            \
    CAN_PACK_BPS_FAULT(&msg);                          \
    StatusCode status = can_transmit(&msg, (ack_ptr)); \
    status;                                            \
  })

#define CAN_TRANSMIT_BATTERY_RELAY(ack_ptr, relay_state_u8) \
  ({                                                        \
    CANMessage msg = { 0 };                                 \
    CAN_PACK_BATTERY_RELAY(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));      \
    status;                                                 \
  })

#define CAN_TRANSMIT_MAIN_RELAY(ack_ptr, relay_state_u8) \
  ({                                                     \
    CANMessage msg = { 0 };                              \
    CAN_PACK_MAIN_RELAY(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));   \
    status;                                              \
  })

#define CAN_TRANSMIT_SOLAR_RELAY_REAR(ack_ptr, relay_state_u8) \
  ({                                                           \
    CANMessage msg = { 0 };                                    \
    CAN_PACK_SOLAR_RELAY_REAR(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));         \
    status;                                                    \
  })

#define CAN_TRANSMIT_SOLAR_RELAY_FRONT(ack_ptr, relay_state_u8) \
  ({                                                            \
    CANMessage msg = { 0 };                                     \
    CAN_PACK_SOLAR_RELAY_FRONT(&msg, (relay_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));          \
    status;                                                     \
  })

#define CAN_TRANSMIT_POWER_STATE(ack_ptr, power_state_u8) \
  ({                                                      \
    CANMessage msg = { 0 };                               \
    CAN_PACK_POWER_STATE(&msg, (power_state_u8));         \
    StatusCode status = can_transmit(&msg, (ack_ptr));    \
    status;                                               \
  })

#define CAN_TRANSMIT_OVUV_DCDC_AUX(dcdc_ov_flag_u8, dcdc_uv_flag_u8, aux_bat_ov_flag_u8,     \
                                   aux_bat_uv_flag_u8)                                       \
  ({                                                                                         \
    CANMessage msg = { 0 };                                                                  \
    CAN_PACK_OVUV_DCDC_AUX(&msg, (dcdc_ov_flag_u8), (dcdc_uv_flag_u8), (aux_bat_ov_flag_u8), \
                           (aux_bat_uv_flag_u8));                                            \
    StatusCode status = can_transmit(&msg, NULL);                                            \
    status;                                                                                  \
  })

#define CAN_TRANSMIT_MC_ERROR_LIMITS(error_id_u16, limits_u16)    \
  ({                                                              \
    CANMessage msg = { 0 };                                       \
    CAN_PACK_MC_ERROR_LIMITS(&msg, (error_id_u16), (limits_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                 \
    status;                                                       \
  })

#define CAN_TRANSMIT_MOTOR_CONTROLS(throttle_u16, steering_angle_u16)    \
  ({                                                                     \
    CANMessage msg = { 0 };                                              \
    CAN_PACK_MOTOR_CONTROLS(&msg, (throttle_u16), (steering_angle_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                        \
    status;                                                              \
  })

#define CAN_TRANSMIT_LIGHTS_STATES(light_id_u8, light_state_u8)    \
  ({                                                               \
    CANMessage msg = { 0 };                                        \
    CAN_PACK_LIGHTS_STATES(&msg, (light_id_u8), (light_state_u8)); \
    StatusCode status = can_transmit(&msg, NULL);                  \
    status;                                                        \
  })

#define CAN_TRANSMIT_HORN(state_u8)               \
  ({                                              \
    CANMessage msg = { 0 };                       \
    CAN_PACK_HORN(&msg, (state_u8));              \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_MECHANICAL_BRAKE(state_u8)   \
  ({                                              \
    CANMessage msg = { 0 };                       \
    CAN_PACK_MECHANICAL_BRAKE(&msg, (state_u8));  \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_CHARGING_REQ()               \
  ({                                              \
    CANMessage msg = { 0 };                       \
    CAN_PACK_CHARGING_REQ(&msg);                  \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_CHARGING_PERMISSION(allowed_u8)  \
  ({                                                  \
    CANMessage msg = { 0 };                           \
    CAN_PACK_CHARGING_PERMISSION(&msg, (allowed_u8)); \
    StatusCode status = can_transmit(&msg, NULL);     \
    status;                                           \
  })

#define CAN_TRANSMIT_BATTERY_SOC()                \
  ({                                              \
    CANMessage msg = { 0 };                       \
    CAN_PACK_BATTERY_SOC(&msg);                   \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_BATTERY_VCT(module_id_u16, voltage_u16, current_u16, temperature_u16)        \
  ({                                                                                              \
    CANMessage msg = { 0 };                                                                       \
    CAN_PACK_BATTERY_VCT(&msg, (module_id_u16), (voltage_u16), (current_u16), (temperature_u16)); \
    StatusCode status = can_transmit(&msg, NULL);                                                 \
    status;                                                                                       \
  })

#define CAN_TRANSMIT_MOTOR_CONTROLLER_VC(mc_voltage_1_u16, mc_current_1_u16, mc_voltage_2_u16,     \
                                         mc_current_2_u16)                                         \
  ({                                                                                               \
    CANMessage msg = { 0 };                                                                        \
    CAN_PACK_MOTOR_CONTROLLER_VC(&msg, (mc_voltage_1_u16), (mc_current_1_u16), (mc_voltage_2_u16), \
                                 (mc_current_2_u16));                                              \
    StatusCode status = can_transmit(&msg, NULL);                                                  \
    status;                                                                                        \
  })

#define CAN_TRANSMIT_MOTOR_VELOCITY_L(vehicle_velocity_u32, angular_freq_u32)    \
  ({                                                                             \
    CANMessage msg = { 0 };                                                      \
    CAN_PACK_MOTOR_VELOCITY_L(&msg, (vehicle_velocity_u32), (angular_freq_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                                \
    status;                                                                      \
  })

#define CAN_TRANSMIT_MOTOR_VELOCITY_R(vehicle_velocity_u32, angular_freq_u32)    \
  ({                                                                             \
    CANMessage msg = { 0 };                                                      \
    CAN_PACK_MOTOR_VELOCITY_R(&msg, (vehicle_velocity_u32), (angular_freq_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                                \
    status;                                                                      \
  })

#define CAN_TRANSMIT_MOTOR_TEMPS(motor_temp_l_u32, motor_temp_r_u32)    \
  ({                                                                    \
    CANMessage msg = { 0 };                                             \
    CAN_PACK_MOTOR_TEMPS(&msg, (motor_temp_l_u32), (motor_temp_r_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                       \
    status;                                                             \
  })

#define CAN_TRANSMIT_MOTOR_AMP_HR(motor_amp_hr_l_u32, motor_amp_hr_r_u32)    \
  ({                                                                         \
    CANMessage msg = { 0 };                                                  \
    CAN_PACK_MOTOR_AMP_HR(&msg, (motor_amp_hr_l_u32), (motor_amp_hr_r_u32)); \
    StatusCode status = can_transmit(&msg, NULL);                            \
    status;                                                                  \
  })

#define CAN_TRANSMIT_ODOMETER(odometer_val_u32)   \
  ({                                              \
    CANMessage msg = { 0 };                       \
    CAN_PACK_ODOMETER(&msg, (odometer_val_u32));  \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_AUX_DCDC_VC(aux_voltage_u16, aux_current_u16, dcdc_voltage_u16,     \
                                 dcdc_current_u16)                                       \
  ({                                                                                     \
    CANMessage msg = { 0 };                                                              \
    CAN_PACK_AUX_DCDC_VC(&msg, (aux_voltage_u16), (aux_current_u16), (dcdc_voltage_u16), \
                         (dcdc_current_u16));                                            \
    StatusCode status = can_transmit(&msg, NULL);                                        \
    status;                                                                              \
  })

#define CAN_TRANSMIT_DCDC_TEMPS(temp_1_u16, temp_2_u16)    \
  ({                                                       \
    CANMessage msg = { 0 };                                \
    CAN_PACK_DCDC_TEMPS(&msg, (temp_1_u16), (temp_2_u16)); \
    StatusCode status = can_transmit(&msg, NULL);          \
    status;                                                \
  })

#define CAN_TRANSMIT_SOLAR_DATA_FRONT(module_id_u16, voltage_u16, current_u16, temperature_u16) \
  ({                                                                                            \
    CANMessage msg = { 0 };                                                                     \
    CAN_PACK_SOLAR_DATA_FRONT(&msg, (module_id_u16), (voltage_u16), (current_u16),              \
                              (temperature_u16));                                               \
    StatusCode status = can_transmit(&msg, NULL);                                               \
    status;                                                                                     \
  })

#define CAN_TRANSMIT_SOLAR_DATA_REAR(module_id_u16, voltage_u16, current_u16, temperature_u16) \
  ({                                                                                           \
    CANMessage msg = { 0 };                                                                    \
    CAN_PACK_SOLAR_DATA_REAR(&msg, (module_id_u16), (voltage_u16), (current_u16),              \
                             (temperature_u16));                                               \
    StatusCode status = can_transmit(&msg, NULL);                                              \
    status;                                                                                    \
  })

#define CAN_TRANSMIT_LINEAR_ACCELERATION()        \
  ({                                              \
    CANMessage msg = { 0 };                       \
    CAN_PACK_LINEAR_ACCELERATION(&msg);           \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })

#define CAN_TRANSMIT_ANGULAR_ROTATION()           \
  ({                                              \
    CANMessage msg = { 0 };                       \
    CAN_PACK_ANGULAR_ROTATION(&msg);              \
    StatusCode status = can_transmit(&msg, NULL); \
    status;                                       \
  })
