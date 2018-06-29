#include <string.h>
#include "can_hw.h"
#include "delay.h"
#include "event_queue.h"
#include "generic_can_hw.h"
#include "interrupt.h"
#include "log.h"
#include "motor_controller.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "wavesculptor.h"

#define TEST_MOTOR_CONTROLLER_EVENT_CAN_FAULT 0
// arbitrary value of 100A to make the math easy
#define TEST_MOTOR_CONTROLLER_MAX_BUS_CURRENT 100.0f

typedef enum {
  TEST_MOTOR_CONTROLLER_CAN_ID_MC_LEFT = 0,
  TEST_MOTOR_CONTROLLER_CAN_ID_MC_RIGHT,
  TEST_MOTOR_CONTROLLER_CAN_ID_DC_LEFT,
  TEST_MOTOR_CONTROLLER_CAN_ID_DC_RIGHT,
} TestMotorControllerCanId;

static GenericCanHw s_can;
static MotorControllerStorage s_storage;
static WaveSculptorDriveCmd s_drive_cmds[NUM_MOTOR_CONTROLLERS];

static void prv_copy_drive_cmd(const GenericCanMsg *msg, void *context) {
  WaveSculptorDriveCmd *drive_cmd = context;
  WaveSculptorCanData can_data = { .raw = msg->data };
  *drive_cmd = can_data.drive_cmd;

  WaveSculptorCanId can_id = { .raw = msg->id };
  printf("RX drive command from %d (current %.4f velocity %.2f)\n", can_id.device_id,
         drive_cmd->motor_current_percentage, drive_cmd->motor_velocity_ms);
}

static void prv_fake_bus_measurement(float bus_current) {
  // Build fake bus measurement from motor controller
  WaveSculptorCanId can_id = {
    .device_id = TEST_MOTOR_CONTROLLER_CAN_ID_MC_LEFT,
    .msg_id = WAVESCULPTOR_MEASUREMENT_ID_BUS,
  };
  WaveSculptorCanData can_data = { 0 };
  can_data.bus_measurement.bus_current = bus_current;
  GenericCanMsg motor_bus_msg = {
    .id = can_id.raw,
    .data = can_data.raw,
    .dlc = 8,
    .extended = false,
  };

  generic_can_tx((GenericCan *)&s_can, &motor_bus_msg);
}

static void prv_fake_velocity_measurement(MotorControllerCanId id, float velocity_ms) {
  WaveSculptorCanId can_id = {
    .device_id = id,
    .msg_id = WAVESCULPTOR_MEASUREMENT_ID_VELOCITY,
  };
  WaveSculptorCanData can_data = { 0 };
  can_data.velocity_measurement.vehicle_velocity_ms = velocity_ms;
  GenericCanMsg motor_velocity_msg = {
    .id = can_id.raw,
    .data = can_data.raw,
    .dlc = 8,
    .extended = false,
  };

  generic_can_tx((GenericCan *)&s_can, &motor_velocity_msg);
}

static void prv_handle_speed(int16_t speed_cms[], size_t num_speeds, void *context) {
  int16_t *speed_arr = context;
  speed_arr[0] = speed_cms[0];
  speed_arr[1] = speed_cms[1];
}

void setup_test(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  event_queue_init();

  const CANHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  // Use CAN HW to mock CAN UART
  TEST_ASSERT_OK(
      generic_can_hw_init(&s_can, &can_hw_settings, TEST_MOTOR_CONTROLLER_EVENT_CAN_FAULT));

  // clang-format off
  const MotorControllerSettings mc_settings = {
    .motor_can = (GenericCan *)&s_can,
    .ids = {
      [MOTOR_CONTROLLER_LEFT] = {
          .motor_controller = TEST_MOTOR_CONTROLLER_CAN_ID_MC_LEFT,
          .interface = TEST_MOTOR_CONTROLLER_CAN_ID_DC_LEFT,
      },
      [MOTOR_CONTROLLER_RIGHT] = {
          .motor_controller = TEST_MOTOR_CONTROLLER_CAN_ID_MC_RIGHT,
          .interface = TEST_MOTOR_CONTROLLER_CAN_ID_DC_RIGHT,
      },
    },
    .max_bus_current = TEST_MOTOR_CONTROLLER_MAX_BUS_CURRENT,
  };
  // clang-format on
  motor_controller_init(&s_storage, &mc_settings);

  MotorControllerCanId dc_ids[NUM_MOTOR_CONTROLLERS] = {
    TEST_MOTOR_CONTROLLER_CAN_ID_DC_LEFT,
    TEST_MOTOR_CONTROLLER_CAN_ID_DC_RIGHT,
  };
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    WaveSculptorCanId can_id = {
      .device_id = dc_ids[i],
      .msg_id = WAVESCULPTOR_CMD_ID_DRIVE,
    };
    generic_can_register_rx((GenericCan *)&s_can, prv_copy_drive_cmd, GENERIC_CAN_EMPTY_MASK,
                            can_id.raw, false, &s_drive_cmds[i]);
  }

  memset(s_drive_cmds, 0, sizeof(s_drive_cmds));
}

void teardown_test(void) {}

void test_motor_controller_throttle_forward(void) {
  // Check accel/coast/brake in forward
  motor_controller_set_throttle(&s_storage, 100, EE_DRIVE_OUTPUT_DIRECTION_FORWARD);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT((float)100 / EE_DRIVE_OUTPUT_DENOMINATOR,
                            s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(WAVESCULPTOR_FORWARD_VELOCITY, s_drive_cmds[i].motor_velocity_ms);
  }

  motor_controller_set_throttle(&s_storage, 0, EE_DRIVE_OUTPUT_DIRECTION_FORWARD);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(WAVESCULPTOR_FORWARD_VELOCITY, s_drive_cmds[i].motor_velocity_ms);
  }

  motor_controller_set_throttle(&s_storage, -100, EE_DRIVE_OUTPUT_DIRECTION_FORWARD);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT((float)100 / EE_DRIVE_OUTPUT_DENOMINATOR,
                            s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_velocity_ms);
  }
}

void test_motor_controller_throttle_neutral(void) {
  // Check accel/coast/brake in netural
  motor_controller_set_throttle(&s_storage, 100, EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_velocity_ms);
  }

  motor_controller_set_throttle(&s_storage, 0, EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_velocity_ms);
  }

  motor_controller_set_throttle(&s_storage, -100, EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_velocity_ms);
  }
}

void test_motor_controller_throttle_reverse(void) {
  // Check accel/coast/brake in reverse
  motor_controller_set_throttle(&s_storage, 100, EE_DRIVE_OUTPUT_DIRECTION_REVERSE);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT((float)100 / EE_DRIVE_OUTPUT_DENOMINATOR,
                            s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(WAVESCULPTOR_REVERSE_VELOCITY, s_drive_cmds[i].motor_velocity_ms);
  }

  motor_controller_set_throttle(&s_storage, 0, EE_DRIVE_OUTPUT_DIRECTION_REVERSE);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(WAVESCULPTOR_REVERSE_VELOCITY, s_drive_cmds[i].motor_velocity_ms);
  }

  motor_controller_set_throttle(&s_storage, -100, EE_DRIVE_OUTPUT_DIRECTION_REVERSE);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT((float)100 / EE_DRIVE_OUTPUT_DENOMINATOR,
                            s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_velocity_ms);
  }
}

void test_motor_controller_watchdog(void) {
  // Make sure we're outputting some non-zero data
  motor_controller_set_throttle(&s_storage, 100, EE_DRIVE_OUTPUT_DIRECTION_FORWARD);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT((float)100 / EE_DRIVE_OUTPUT_DENOMINATOR,
                            s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(WAVESCULPTOR_FORWARD_VELOCITY, s_drive_cmds[i].motor_velocity_ms);
  }

  LOG_DEBUG("Delaying until timeout\n");
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * (MOTOR_CONTROLLER_WATCHDOG_COUNTER + 1));
  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_current_percentage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[i].motor_velocity_ms);
  }
}

void test_motor_controller_cruise(void) {
  motor_controller_set_cruise(&s_storage, 45);
  // Pretend we're accelerating - left motor controller sends bus measurement of 50A
  prv_fake_bus_measurement(50.0f);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);

  // Primary controller should use velocity setpoint
  TEST_ASSERT_EQUAL_FLOAT(1.0f, s_drive_cmds[0].motor_current_percentage);
  TEST_ASSERT_EQUAL_FLOAT(0.45f, s_drive_cmds[0].motor_velocity_ms);

  // Secondary controller should use current setpoint
  TEST_ASSERT_EQUAL_FLOAT(50.0f / TEST_MOTOR_CONTROLLER_MAX_BUS_CURRENT,
                          s_drive_cmds[1].motor_current_percentage);
  TEST_ASSERT_EQUAL_FLOAT(WAVESCULPTOR_FORWARD_VELOCITY, s_drive_cmds[1].motor_velocity_ms);

  // Pretend we're braking
  prv_fake_bus_measurement(-50.0f);
  delay_ms(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS * 2);

  // Primary controller should use velocity setpoint
  TEST_ASSERT_EQUAL_FLOAT(1.0f, s_drive_cmds[0].motor_current_percentage);
  TEST_ASSERT_EQUAL_FLOAT(0.45f, s_drive_cmds[0].motor_velocity_ms);

  // Secondary controller should use current setpoint
  TEST_ASSERT_EQUAL_FLOAT(50.0f / TEST_MOTOR_CONTROLLER_MAX_BUS_CURRENT,
                          s_drive_cmds[1].motor_current_percentage);
  TEST_ASSERT_EQUAL_FLOAT(0.0f, s_drive_cmds[1].motor_velocity_ms);
}

void test_motor_controller_speed(void) {
  int16_t speed_arr[NUM_MOTOR_CONTROLLERS] = { 0 };
  motor_controller_set_speed_cb(&s_storage, prv_handle_speed, speed_arr);

  prv_fake_velocity_measurement(TEST_MOTOR_CONTROLLER_CAN_ID_MC_LEFT, 10.0f);
  prv_fake_velocity_measurement(TEST_MOTOR_CONTROLLER_CAN_ID_MC_RIGHT, -10.0f);

  delay_ms(10);

  TEST_ASSERT_EQUAL(1000, speed_arr[0]);
  TEST_ASSERT_EQUAL(-1000, speed_arr[1]);
}
