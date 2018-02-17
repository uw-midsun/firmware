#include "chaos_config.h"

static ChaosConfig s_config;

ChaosConfig *chaos_config_load(void) {
  return &s_config;
}
