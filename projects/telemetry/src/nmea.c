#include "nmea.h"
#include <stdbool.h>
#include <string.h>

// Just checks if the first couple of characters are similar to $GPGGA
bool nmea_is_gga(const char *to_check) {
  if (to_check == NULL) {
    return false;
  }
  size_t len = strlen(to_check);
  if (len < 6) {
    return false;
  }
  if (to_check[3] == 'G' && to_check[4] == 'G' && to_check[5] == 'A') {
    return true;
  }
  return false;
}

// Just checks if the first couple of characters are similar to $GPVTG
bool nmea_is_vtg(const char *to_check) {
  if (to_check == NULL) {
    return false;
  }
  size_t len = strlen(to_check);
  if (len < 6) {
    return false;
  }
  if (to_check[3] == 'V' && to_check[4] == 'T' && to_check[5] == 'G') {
    return true;
  }
  return false;
}
