#include "unity.h"
#include "log.h"
#include "test_helpers.h"

#include "interrupt.h"
#include "soft_timer.h"
#include "i2c.h"
#include "magnetic_sensor.h"

void setup_test(void) {
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
  MagneticSensorReading reading = {
    .x = 2048,
    .y = 2048,
    .z = 2048
  };

  TEST_ASSERT_OK(magnetic_sensor_read_data(I2C_PORT_1, &reading));

  TEST_ASSERT_TRUE(reading.x > -2048 || reading.x < 2047);
  TEST_ASSERT_TRUE(reading.y > -2048 || reading.y < 2047);
  TEST_ASSERT_TRUE(reading.z > -2048 || reading.z < 2047);
}
