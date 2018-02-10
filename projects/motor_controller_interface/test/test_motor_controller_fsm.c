#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "generic_can_hw.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "can_interval.h"
#include "can_transmit.h"
#include "config.h"
#include "exported_enums.h"
#include "motor_controller.h"
#include "motor_controller_fsm.h"
#include "motor_controller_interface_events.h"

#include "delay.h"
#include "fifo.h"
#include "log.h"

#define TEST_MOTOR_CONTROLLER_INTERFACE_NUM_RX_HANDLERS 10

#define TEST_REVERSE_THROTTLE_VALUE ((float)1234)
#define TEST_FORWARD_THROTTLE_VALUE ((float)2345)

#define TEST_CRUISE_CONTROL_VALUE 500
#define TEST_CRUISE_CONTROL_CURRENT 4321
#define TEST_CRUISE_CONTROL_VOLTAGE 9876

#define TEST_CRUISE_CONTROL_REVERSE_VALUE 100

#define TEST_NUM_MESSAGES_RECEIVED 20

// CAN storage
static CANStorage s_storage;
static CANRxHandler s_rx_handlers[TEST_MOTOR_CONTROLLER_INTERFACE_NUM_RX_HANDLERS];

static GenericCanHw s_can = { 0 };

// FIFO storage
static Fifo s_fifo;
static DriverControlsData s_buffer[20];

// Motor Controller FSM storage
static MotorControllerFsmStorage s_mc_fsm_storage;
MotorControllerMeasurement s_mc_measurements = { 0 };

// Test metadata
union {
  MotorControllerDriveCmd mc_drive_cmd;
  MotorControllerPowerCmd mc_power_cmd;
  uint64_t data_u64;
} MotorControllerFsmTestUnion;

static volatile size_t num_braking_messages[2] = { 0 };
static volatile size_t num_cruise_control_forward_messages[2] = { 0 };
static volatile size_t num_forward_messages[2] = { 0 };
static volatile size_t num_reverse_messages[2] = { 0 };

// Assertion functions
static void prv_assert_braking_state(const GenericCanMsg *msg, void *context) {
  volatile size_t *value = (size_t *)context;
  TEST_ASSERT_TRUE(
      MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR) == msg->id ||
      MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR) == msg->id);

  MotorControllerFsmTestUnion.data_u64 = msg->data;
  // We should have 0 torque
  TEST_ASSERT_EQUAL_FLOAT(MOTOR_CONTROLLER_FORWARD_VELOCITY_MPS,
                          MotorControllerFsmTestUnion.mc_drive_cmd.velocity);
  TEST_ASSERT_EQUAL_FLOAT(0, MotorControllerFsmTestUnion.mc_drive_cmd.current_percentage);
}

static void prv_assert_reverse_state(const GenericCanMsg *msg, void *context) {
  volatile size_t *value = (size_t *)context;
  if (value[0] + value[1] < TEST_NUM_MESSAGES_RECEIVED) {
    // We should only receive Drive commands in Reverse
    TEST_ASSERT_TRUE(
        MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR) == msg->id ||
        MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR) == msg->id);

    MotorControllerFsmTestUnion.data_u64 = msg->data;
    // We set an impossible velocity in drive mode
    TEST_ASSERT_EQUAL_FLOAT(MOTOR_CONTROLLER_REVERSE_VELOCITY_MPS,
                            MotorControllerFsmTestUnion.mc_drive_cmd.velocity);
    TEST_ASSERT_EQUAL_FLOAT(TEST_REVERSE_THROTTLE_VALUE / DRIVER_CONTROLS_PEDAL_DENOMINATOR,
                            MotorControllerFsmTestUnion.mc_drive_cmd.current_percentage);

    if (MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR) == msg->id) {
      value[0]++;
    } else if (MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR) ==
               msg->id) {
      value[1]++;
    }
  }
}

static void prv_assert_forward_state(const GenericCanMsg *msg, void *context) {
  volatile size_t *value = (size_t *)context;
  if (value[0] + value[1] < TEST_NUM_MESSAGES_RECEIVED) {
    // We should only receive Drive commands in Forward
    TEST_ASSERT_TRUE(
        MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR) == msg->id ||
        MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR) == msg->id);

    MotorControllerFsmTestUnion.data_u64 = msg->data;

    TEST_ASSERT_EQUAL_FLOAT(MOTOR_CONTROLLER_FORWARD_VELOCITY_MPS,
                            MotorControllerFsmTestUnion.mc_drive_cmd.velocity);
    TEST_ASSERT_EQUAL_FLOAT(TEST_FORWARD_THROTTLE_VALUE / DRIVER_CONTROLS_PEDAL_DENOMINATOR,
                            MotorControllerFsmTestUnion.mc_drive_cmd.current_percentage);

    if (MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR) == msg->id) {
      value[0]++;
    } else if (MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR) ==
               msg->id) {
      value[1]++;
    }
  }
}

static void prv_assert_cruise_forward_state(const GenericCanMsg *msg, void *context) {
  volatile size_t *value = (size_t *)context;
  if (value[0] + value[1] < TEST_NUM_MESSAGES_RECEIVED) {
    // In Cruise Control mode, we send Drive commands from the left motor
    // controller and Power commands from the right
    TEST_ASSERT_TRUE(
        MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR) == msg->id ||
        MOTOR_CONTROLLER_POWER_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR) == msg->id);

    MotorControllerFsmTestUnion.data_u64 = msg->data;
    if (msg->id == MOTOR_CONTROLLER_POWER_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR)) {
    } else if (msg->id == MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR)) {
      TEST_ASSERT_EQUAL_FLOAT(MOTOR_CONTOLLER_VELOCITY_CMPS_TO_MPS(TEST_CRUISE_CONTROL_VALUE),
                              MotorControllerFsmTestUnion.mc_drive_cmd.velocity);
      TEST_ASSERT_EQUAL_FLOAT(1.0f, MotorControllerFsmTestUnion.mc_drive_cmd.current_percentage);
    }

    if (MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_LEFT_ADDR) == msg->id) {
      value[0]++;
    } else if (MOTOR_CONTROLLER_DRIVE_COMMAND_ID(MOTOR_CONTROLLER_INTERFACE_RIGHT_ADDR) ==
               msg->id) {
      value[1]++;
    }
  }
}

static void prv_transition_to_cruise_control(void) {
  // Set the left Motor Controller telemetry values to a known value
  s_mc_measurements.bus_meas_left.current = 100;
  s_mc_measurements.bus_meas_left.voltage = 1;

  // Send a Driver Controls CAN message going forward
  DriverControlsData data = {
    .throttle = 0,
    .direction = DRIVER_CONTROLS_FORWARD,
    .cruise_control = TEST_CRUISE_CONTROL_VALUE,
    .brake_state = DRIVER_CONTROLS_BRAKE_DISENGAGED,
  };
  fifo_push(&s_fifo, &data);
  event_raise(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, 0);

  // Run the FSM
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  do {
    status = event_process(&e);
    if (e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_TX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_RX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    } else {
      TEST_ASSERT_EQUAL(true, motor_controller_fsm_process_event(&e));
    }
  } while (status != STATUS_CODE_OK);

  delay_ms(100);

  num_cruise_control_forward_messages[0] = 0;
  num_cruise_control_forward_messages[1] = 0;
  TEST_ASSERT_OK(generic_can_register_rx((GenericCan *)&s_can, prv_assert_cruise_forward_state,
                                         0x00, 0x00, false,
                                         (void *)&num_cruise_control_forward_messages));
  while (num_cruise_control_forward_messages[0] + num_cruise_control_forward_messages[1] <
         TEST_NUM_MESSAGES_RECEIVED) {
  }
}

static void prv_transition_to_forward(void) {
  static int calls = 0;
  calls++;
  // Send a Driver Controls CAN message going forward
  DriverControlsData data = {
    .throttle = TEST_FORWARD_THROTTLE_VALUE,
    .direction = DRIVER_CONTROLS_FORWARD,
    .cruise_control = 0,
    .brake_state = DRIVER_CONTROLS_BRAKE_DISENGAGED,
  };
  fifo_push(&s_fifo, &data);
  event_raise(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, 0);

  // Run the FSM
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  do {
    status = event_process(&e);
    if (e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_TX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_RX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    } else {
      TEST_ASSERT_EQUAL(true, motor_controller_fsm_process_event(&e));
    }
  } while (status != STATUS_CODE_OK);

  delay_ms(100);

  // Assert that we're still receiving torque mode messages
  num_forward_messages[0] = 0;
  num_forward_messages[1] = 0;
  TEST_ASSERT_OK(generic_can_register_rx((GenericCan *)&s_can, prv_assert_forward_state, 0x00, 0x00,
                                         false, (void *)&num_forward_messages));
  while (num_forward_messages[0] + num_forward_messages[1] < TEST_NUM_MESSAGES_RECEIVED) {
  }
}

static void prv_transition_to_braking(void) {
  const CANMessage msg = { 0 };

  // Send a Driver Controls CAN message to enter braking state
  DriverControlsData data = {
    .throttle = 0,
    .direction = DRIVER_CONTROLS_FORWARD,
    .cruise_control = 0,
    .brake_state = DRIVER_CONTROLS_BRAKE_ENGAGED,
  };
  fifo_push(&s_fifo, &data);
  event_raise(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, 0);

  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  do {
    status = event_process(&e);
    if (e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_TX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_RX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    } else {
      TEST_ASSERT_EQUAL(true, motor_controller_fsm_process_event(&e));
    }
  } while (status != STATUS_CODE_OK);

  delay_ms(50);
  TEST_ASSERT_OK(generic_can_register_rx((GenericCan *)&s_can, prv_assert_braking_state, 0x00, 0x00,
                                         false, (void *)&num_braking_messages));
}

static void prv_transition_to_reverse(void) {
  static int calls = 0;
  calls++;

  DriverControlsData data = {
    .throttle = TEST_REVERSE_THROTTLE_VALUE,
    .direction = DRIVER_CONTROLS_REVERSE,
    .cruise_control = 0,
    .brake_state = DRIVER_CONTROLS_BRAKE_DISENGAGED,
  };
  fifo_push(&s_fifo, &data);
  event_raise(MOTOR_CONTROLLER_INTERFACE_EVENT_FIFO, 0);

  // Run the FSM
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  do {
    status = event_process(&e);
    if (e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_TX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_RX ||
        e.id == MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &e));
    } else {
      TEST_ASSERT_EQUAL(true, motor_controller_fsm_process_event(&e));
    }
  } while (status != STATUS_CODE_OK);

  delay_ms(100);

  // Assert that the values we are reading are from the initial state still
  // (because we didn't transition)
  num_reverse_messages[0] = 0;
  num_reverse_messages[1] = 0;
  TEST_ASSERT_OK(generic_can_register_rx((GenericCan *)&s_can, prv_assert_reverse_state, 0x00, 0x00,
                                         false, (void *)&num_reverse_messages));
  while (num_reverse_messages[0] + num_reverse_messages[1] < TEST_NUM_MESSAGES_RECEIVED) {
  }
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  fifo_init(&s_fifo, s_buffer);

  CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_RX,
    .tx_event = MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_TX,
    .fault_event = MOTOR_CONTROLLER_INTERFACE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  can_init(&settings, &s_storage, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));

  const CANHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_interval_init();

  memset(&s_can, 0x00, sizeof(s_can));
  TEST_ASSERT_OK(generic_can_hw_init(&s_can, &can_hw_settings, 0));

  s_mc_fsm_storage.generic_can = (GenericCan *)&s_can;
  s_mc_fsm_storage.measurement = &s_mc_measurements;
  s_mc_fsm_storage.fifo = &s_fifo;

  TEST_ASSERT_OK(motor_controller_fsm_init(&s_mc_fsm_storage));

  return;
}

void teardown_test(void) {
  return;
}

void test_motor_controller_fsm_initial_state_to_forward(void) {
  prv_transition_to_forward();
}

void test_motor_controller_fsm_initial_state_to_reverse(void) {
  // We start in the initial state

  // Send a Driver Controls CAN message reversing the direction
  prv_transition_to_reverse();
}

void test_motor_controller_fsm_initial_state_to_cruise_control_forward(void) {
  prv_transition_to_forward();

  prv_transition_to_cruise_control();
}

void test_motor_controller_fsm_initial_state_to_braking(void) {
  // Send a Driver Controls CAN message activating the mechanical brake
  prv_transition_to_braking();
}

void test_motor_controller_fsm_cruise_control_to_forward(void) {
  prv_transition_to_cruise_control();

  prv_transition_to_forward();
}

void test_motor_controller_fsm_cruise_control_to_reverse(void) {
  prv_transition_to_forward();

  prv_transition_to_cruise_control();

  prv_transition_to_reverse();
}

void test_motor_controller_fsm_cruise_control_to_cruise_control_forward(void) {
  prv_transition_to_cruise_control();

  prv_transition_to_cruise_control();
}

void test_motor_controller_fsm_cruise_control_to_braking(void) {
  prv_transition_to_forward();

  prv_transition_to_cruise_control();

  prv_transition_to_braking();
}

void test_motor_controller_fsm_reverse_to_forward(void) {
  prv_transition_to_reverse();

  prv_transition_to_forward();
}

void test_motor_controller_fsm_reverse_to_reverse(void) {
  prv_transition_to_reverse();

  prv_transition_to_reverse();
}

void test_motor_controller_fsm_reverse_to_cruise_control_forward(void) {
  prv_transition_to_reverse();

  prv_transition_to_forward();
}

void test_motor_controller_fsm_reverse_to_braking(void) {
  prv_transition_to_reverse();

  prv_transition_to_braking();
}
