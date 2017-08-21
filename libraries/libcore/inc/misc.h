#pragma once
// Common helper macros

#define SIZEOF_ARRAY(arr) (sizeof((arr)) / sizeof((arr)[0]))
// Casts void * to uint8_t *
#define VOID_PTR_UINT8(x) (_Generic((x), void *: (uint8_t *)(x), default: (x)))

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
