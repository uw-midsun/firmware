#include "unity.h"
#include "log.h"
#include "test_helpers.h"

#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "i2c.h"
#include "magnetic_sensor.h"

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .sda = { GPIO_PORT_B, 9 },
    .scl = { GPIO_PORT_B, 8 }
  };

  i2c_init(I2C_PORT_1, &settings);

  TEST_ASSERT_OK(magnetic_sensor_init(I2C_PORT_1));
}

void teardown_test(void) { }

void test_magnetic_sensor_read_data(void) {
  magnetic_sensor_init(I2C_PORT_1);

  int16_t readings[3];

  TEST_ASSERT_OK(magnetic_sensor_read_data(I2C_PORT_1, readings));

  for (uint8_t i = 0; i < NUM_MAGNETIC_SENSOR_PROBES; i++) {
    TEST_ASSERT_TRUE(readings[i] > -2048 || readings[i] < 2047);
  }
}
