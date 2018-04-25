#pragma once

// Motor controller addresses (for telemetry values)
#define MOTOR_CONTROLLER_LEFT_ADDR 0x040
#define MOTOR_CONTROLLER_RIGHT_ADDR 0x060

// Motor controller interface addresses (for drive commands)
// TODO(ELEC-388): Program these with the config tool
#define MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR 0x020
#define MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR 0x000

#define MOTOR_CONTROLLER_INTERFACE_CAN_RX_NUM_HANDLERS 5
