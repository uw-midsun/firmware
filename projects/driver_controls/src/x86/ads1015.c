#include "ads1015.h"
#include <status.h>
#include <stdio.h>
#include "ads1015_def.h"
#include "gpio_it.h"

// initiates ads1015 by setting up registers and enabling ALRT/RDY Pin
StatusCode ads1015_init(ADS1015Data *data, I2CPort i2c_port, ADS1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  return STATUS_CODE_UNIMPLEMENTED;
}

// This function enable/disables channels, and registers callbacks for each channel.
// It also registers the interrupt handler on ALRT/RDY pin.
StatusCode ads1015_configure_channel(ADS1015Data *data, ADS1015Channel channel, bool enable,
                                     ADS1015Callback callback, void *context) {
  return STATUS_CODE_UNIMPLEMENTED;
}

// reads raw 12 bit conversion results which are expressed in two's complement format
StatusCode ads1015_read_raw(ADS1015Data *data, ADS1015Channel channel, uint16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}

// reads conversion value in mVolt
StatusCode ads1015_read_converted(ADS1015Data *data, ADS1015Channel channel, int16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}
