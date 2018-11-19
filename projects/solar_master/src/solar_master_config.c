#include "solar_master_config.h"

#define SOLAR_MASTER_I2C_BUS_SDA \
  { GPIO_PORT_B, 11 }

#define SOLAR_MASTER_I2C_BUS_SCL \
  { GPIO_PORT_B, 10 }

#define SOLAR_CURRENT_I2C_BUS_SDA \
  { GPIO_PORT_B, 9 }

#define SOLAR_CURRENT_I2C_BUS_SCL \
  { GPIO_PORT_B, 8 }

// TODO(ELEC-502): Add I2C high speed support to the driver.
const I2CSettings slave_i2c_settings = {
  .speed = I2C_SPEED_STANDARD,      //
  .sda = SOLAR_MASTER_I2C_BUS_SDA,  //
  .scl = SOLAR_MASTER_I2C_BUS_SCL,  //
};

const I2CSettings current_i2c_settings = {
  .speed = I2C_SPEED_STANDARD,       //
  .sda = SOLAR_CURRENT_I2C_BUS_SDA,  //
  .scl = SOLAR_CURRENT_I2C_BUS_SCL,  //
};

const Mcp3427Setting slave_mcp3427_settings = {
  .sample_rate = MCP3427_SAMPLE_RATE_12_BIT,
  .Adr0 = MCP3427_PIN_STATE_FLOAT,
  .Adr1 = MCP3427_PIN_STATE_FLOAT,
  .amplifier_gain = MCP3427_AMP_GAIN_1,
  .conversion_mode = MCP3427_CONVERSION_MODE_CONTINUOUS,
  .port = I2C_PORT_2,
};

CanSettings can_settings = {
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .tx = { .port = GPIO_PORT_A, .pin = 12 },
  .rx = { .port = GPIO_PORT_A, .pin = 11 },
  .rx_event = SOLAR_MASTER_EVENT_CAN_RX,
  .tx_event = SOLAR_MASTER_EVENT_CAN_TX,
  .fault_event = SOLAR_MASTER_EVENT_CAN_FAULT,
  .loopback = false,
};

static SolarMasterConfig s_config = {
  .slave_i2c_port = I2C_PORT_2,
  .current_i2c_port = I2C_PORT_1,
  .slave_i2c_settings = &slave_i2c_settings,
  .current_i2c_settings = &current_i2c_settings,
  .slave_mcp3427_settings_base = &slave_mcp3427_settings,
  .can_settings = &can_settings,
};

const uint16_t device_id_lookup[NUM_SOLAR_MASTER_CONFIG_BOARDS] = {
  [SOLAR_MASTER_CONFIG_BOARD_FRONT] = SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT,
  [SOLAR_MASTER_CONFIG_BOARD_REAR] = SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR,
};

// We'll ignore mode_sel_1 because there's only two solar masters.
static const GpioAddress s_mode_sel_0 = { .port = GPIO_PORT_A, .pin = 9 };
static const GpioAddress s_mode_sel_1 = { .port = GPIO_PORT_A, .pin = 10 };

StatusCode solar_master_config_init(void) {
  // Getting board type.
  GpioState state = NUM_GPIO_STATES;
  StatusCode status = gpio_get_state(&s_mode_sel_0, &state);
  if (!status_ok(status)) {
    LOG_DEBUG("Error getting board type.\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }
  s_config.board =
      (state == GPIO_STATE_HIGH) ? SOLAR_MASTER_CONFIG_BOARD_FRONT : SOLAR_MASTER_CONFIG_BOARD_REAR;

  s_config.can_settings->device_id = device_id_lookup[s_config.board];
  return STATUS_CODE_OK;
}

SolarMasterConfig *solar_master_config_load() {
  return &s_config;
}
