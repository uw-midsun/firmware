#pragma once
// Common helper macros

#include <stdbool.h>

#define SIZEOF_ARRAY(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define SIZEOF_FIELD(type, field) (sizeof(((type *)0)->field))
// Casts void * to uint8_t *
#define VOID_PTR_UINT8(x) (_Generic((x), void * : (uint8_t *)(x), default : (x)))

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })

#define MAX(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

// Returns true (if and only if) the next delimiter occurred twice in a row
// rx_arr is the input array, new_str will be overwritten to contain a pointer
// to the first character after the next delimiter. new_str can be null if the
// delimiter is not found (ie. end of string). if do_nothing is set to true, this function
// will have no effect except for flipping it to false. If do_nothing is NULL, the function
// will proceed as normal.
bool strtok_d(const char *rx_arr, char delimiter, char **new_str, bool *do_nothing);
