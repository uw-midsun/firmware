#pragma once
#include <stdio.h>

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_WARN,
  LOG_LEVEL_FAULT,
  LOG_LEVEL_NONE // Invalid log level
} LogLevel;

#ifndef LOG_LEVEL_VERBOSITY
#define LOG_LEVEL_VERBOSITY LOG_LEVEL_DEBUG
#endif

#define LOG(level, fmt, ...) \
do { \
  if ((level) >= LOG_LEVEL_VERBOSITY) { \
    printf("%s:%d:%s():" (fmt), __FILE__, __LINE__, __func__, __VA_ARGS__); \
  } \
} while (0);
