#include "throttle.h"

#define GET_PEDAL_ROM_PERCENTAGE(converted_pedal_reading) ((converted_pedal_reading - 670) / 21) 


// questions: 
// Should the function take a reading or do the reading itself?
// How to know if readings are up to date? should set a timer on gpio pin?
// There are two channels for the pedal.

StatusCode throttle_get_position(uint8_t *position_percentage, int16_t *converted_pedal_reading) {
  *position_percentage = GET_PEDAL_ROM_PERCENTAGE(*converted_pedal_reading);
 
  return STATUS_CODE_OK;
}