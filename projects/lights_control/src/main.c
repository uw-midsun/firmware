#include <stdint.h>

#include "can.h"
#include "gpio.h"
#include "interrupt.h"
#include "status.h"
#include "wait.h"

#include "lights_can.h"

int main(void) {
  while (true) {
    // process raised events
  }
  return STATUS_CODE_OK;
}
