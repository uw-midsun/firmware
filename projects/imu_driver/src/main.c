// ms-common includes
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"
#include "spi.h"

// Module specific includes
#include "config.h"
#include "imu.h"

// Persistent IMU storage
static ImuStorage imu_storage;

int main(void) {
  // Initializations
  gpio_init();
  soft_timer_init();

  // Initialize imu including starting periodic reads
  imu_init(config_load_imu(), &imu_storage);

  // Superloop
  while (true) {
  }
}
