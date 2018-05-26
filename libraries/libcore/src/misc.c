#include "misc.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

bool strtok_d(const char *rx_arr, char delimiter, char **new_str, bool *do_nothing) {
  if (do_nothing != NULL && *do_nothing == true) {
    *do_nothing = false;
    return false;
  }
  if (rx_arr == NULL || new_str == NULL) {
    return false;
  }
  *new_str = strchr(rx_arr, delimiter);
  if (new_str == NULL || *new_str == NULL) {
    return false;
  }

  // This is because the first character will be the delimiter
  if (strlen(*new_str) > 1) {
    // We want to remove the delimiter
    *new_str = *new_str + 1;
  } else {
    return false;
  }

  // Return true if the second character is not the delimiter, therefore it is not
  // a NULL field
  if (strlen(*new_str) > 1) {
    if ((*new_str)[0] == delimiter) {
      // Get rid of delimiter
      *new_str = *new_str + 1;
      return true;
    }
  }
  return false;
}
