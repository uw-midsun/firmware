#pragma once
// Consistent Overhead Byte Stuffing (COBS)
// Used for packet framing over serial
//
// See https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing
#include "status.h"
#include "misc.h"
#include <stdint.h>
#include <stddef.h>

#define COBS_MAX_ENCODED_LEN(data_len) \
  ({ size_t _len = (data_len); \
     CEIL(_len, 254) + _len; })

// Takes in an array of input bytes |data| of size |data_len| and an output array |encoded|
// where |encoded_len| points to storage initially set to the maximum size of |encoded|.
// After encoding, |encoded_len| is set to the size of the encoded output.
StatusCode cobs_encode(uint8_t *data, size_t data_len, uint8_t *encoded, size_t *encoded_len);
