#include "cobs.h"

StatusCode cobs_encode(uint8_t *data, size_t data_len, uint8_t *encoded, size_t *encoded_len) {
  if (data == NULL || encoded_len == NULL || data_len == 0 || *encoded_len < COBS_MAX_ENCODED_LEN(data_len)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *encoded_len = 0;
  uint8_t *start = encoded, *end = data + data_len;
  uint8_t code = 1, *code_ptr = encoded++;

  while (data < end) {
    if (code != 0xFF) {
      uint8_t c = *data++;
      if (c != 0) {
        *encoded++ = c;
        code++;
        continue;
      }
    }
    *code_ptr = code;
    code_ptr = encoded++;
    code = 1;
  }
  *code_ptr = code;

  *encoded_len = (size_t)(encoded - start);

  return STATUS_CODE_OK;
}
