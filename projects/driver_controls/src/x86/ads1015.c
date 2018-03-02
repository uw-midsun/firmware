#include "ads1015.h"
#include <status.h>
#include <stdio.h>
#include "ads1015_def.h"
#include "gpio_it.h"

// Initiates ads1015 by setting up registers and enabling ALRT/RDY Pin.
StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

// This function enable/disables channels, and registers callbacks for each channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

// Reads raw 12 bit conversion results which are expressed in two's complement format.
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, int16_t *reading) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}

// Reads conversion value in mVolt.
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading) {
  return status_code(STATUS_CODE_UNIMPLEMENTED);
}
