#include "potentiometer.h"
#include <stddef.h>
#include "log.h"
#include "soft_timer.h"

StatusCode potentiometer_init(const PotentiometerSettings *settings,
                              PotentiometerStorage *storage) {
  // TODO(ELEC-624): Implement this once potentiometer pin has been re-mapped on hardware
  return STATUS_CODE_UNIMPLEMENTED;
}
