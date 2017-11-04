#include <string.h>
#include "util.h"

uint8_t compute_checksum(char* message) {
  uint8_t sum = 0;
  uint8_t message_len = strlen(message);
  for (uint8_t i = 1; message[i] != '*' && i < message_len; i++) {
    sum ^= message[i];
  }
  return sum;
}
