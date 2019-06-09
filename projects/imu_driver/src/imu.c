#include "imu.h"
#include "gpio.h"
#include "log.h"
#include "soft_timer.h"
#include "spi.h"

// Sensor data stored as a 16 bit number in IC, so need to join bytes
uint16_t prv_join_bytes(uint8_t lsb, uint8_t msb) {
  uint8_t bytes[] = { msb, lsb }; // Most Significant Byte, Least Significant Byte
  uint16_t *val = (uint16_t *)&bytes[0];
  return *val;
}

// Use spi to get both bytes of a sensor value and combine them, then store them
StatusCode prv_read_sensor(ImuStorage *storage, uint8_t *registers, uint16_t *reads) {
  // read x, y, and z axes
  for (uint32_t i = 0; i < NUM_IMU_SENSOR_AXES; ++i) {
    uint8_t read_lsb = 0;
    uint8_t read_msb = 0;
    // set read/write bit to read
    uint8_t get_lsb_bits = registers[2 * i] | (1 << 7);
    uint8_t get_msb_bits = registers[2 * i + 1] | (1 << 7);
    spi_exchange(storage->port, &get_lsb_bits, 1, &read_lsb, 1);
    spi_exchange(storage->port, &get_msb_bits, 1, &read_msb, 1);
    reads[i] = prv_join_bytes(read_lsb, read_msb);
  }
  return STATUS_CODE_OK;
}

// Periodically read data from IC
static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  ImuStorage *storage = context;

  prv_read_sensor(storage, &storage->gyro_registers[0], &storage->gyro_data[0]);
  prv_read_sensor(storage, &storage->xl_registers[0], &storage->xl_data[0]);

  // ...Implement CAN transmission for telemetry here...

  LOG_DEBUG("Gyroscope x: %hd, y: %hd, z: %hd\n", storage->gyro_data[IMU_SENSOR_X],
            storage->gyro_data[IMU_SENSOR_Y], storage->gyro_data[IMU_SENSOR_Z]);
  LOG_DEBUG("Accelerometer x: %hd, y: %hd, z: %hd\n", storage->xl_data[IMU_SENSOR_X],
            storage->xl_data[IMU_SENSOR_Y], storage->xl_data[IMU_SENSOR_Z]);

  soft_timer_start_seconds(storage->read_rate_s, prv_timer_callback, storage, NULL);
}

// Reads the WhoAmI register, used as a sanity check
StatusCode prv_read_whoAmI(ImuStorage *storage) {
  uint8_t read_value = 0;
  uint8_t address = WHOAMI_REG | (1 << 7);  // read/write bit set for spi
  uint8_t expected = WHOAMI_REG_VAL;

  // Read the value with spi
  status_ok_or_return(spi_exchange(storage->port, &address, 1, &read_value, 1));

  // Check if the value is right
  if (read_value == expected) {
    return STATUS_CODE_OK;
  } else {
    return STATUS_CODE_INTERNAL_ERROR;
  }
}

StatusCode imu_init(const ImuSettings *settings, ImuStorage *storage) {
  // Initializing the storage pieces necessary
  storage->port = settings->port;
  storage->read_rate_s = settings->data_send_period;

  // Set the registers for gyro and accelerometer in storage
  for (uint32_t i = 0; i < 6; ++i) {
    storage->gyro_registers[i] = settings->gyro_head_register + i;
    storage->xl_registers[i] = settings->xl_head_register + i;
  }

  // Spi settings
  SpiSettings spi_config = { .baudrate = settings->spi_baudrate,
                             .mode = settings->mode,
                             .cs = settings->cs,
                             .mosi = settings->mosi,
                             .miso = settings->miso,
                             .sclk = settings->sclk };

  spi_init(storage->port, &spi_config);

  // sanity check before finishing init
  status_ok_or_return(prv_read_whoAmI(storage));

  // Set accelerometer output data rate to 12.5hz (datasheet page 56)
  uint8_t xl_setup_bytes[] = { XL_ODR_REG, 0x10 };
  spi_cs_set_state(storage->port, GPIO_STATE_LOW);
  spi_tx(storage->port, xl_setup_bytes, 2);
  spi_cs_set_state(storage->port, GPIO_STATE_HIGH);

  // Set gyroscope output data rate to 12.5hz (datasheet page 57)
  uint8_t gyro_setup_bytes[] = { GYRO_ODR_REG, 0x10 };
  spi_cs_set_state(storage->port, GPIO_STATE_LOW);
  spi_tx(storage->port, gyro_setup_bytes, 2);
  spi_cs_set_state(storage->port, GPIO_STATE_HIGH);

  return soft_timer_start_seconds(storage->read_rate_s, prv_timer_callback, storage, NULL);
}
