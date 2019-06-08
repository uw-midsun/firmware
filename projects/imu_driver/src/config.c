#include "config.h"
#include "imu.h"

// IMU settings
static ImuSettings imu_settings = { .cs = { .port = GPIO_PORT_A, .pin = 4 },
                                    .mosi = { .port = GPIO_PORT_A, .pin = 7 },
                                    .miso = { .port = GPIO_PORT_A, .pin = 6 },
                                    .sclk = { .port = GPIO_PORT_A, .pin = 5 },

                                    .gyro_first_register = 0x22,
                                    .xl_first_register = 0x28,

                                    .mode = SPI_MODE_3,
                                    .spi_baudrate = IMU_BAUDRATE_NS,

                                    .port = SPI_PORT_1,
                                    .data_send_period = IMU_POLL_PERIOD_S };

ImuSettings *config_load_imu(void) {
  return &imu_settings;
}
