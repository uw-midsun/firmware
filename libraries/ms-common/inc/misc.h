#pragma once
// Common helper macros

#define SIZEOF_ARRAY(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
