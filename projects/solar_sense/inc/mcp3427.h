#pragma once

#include "i2c.h"

typedef enum {
  MCP3427_PIN_STATE_LOW = 0,
  MCP3427_PIN_STATE_FLOAT,
  MCP3427_PIN_STATE_HIGH,
  NUM_MCP3427_PIN_STATES
} Mcp3427PinState;

typedef enum {
  MCP3427_SAMPLE_RATE_12_BIT = 0,
  MCP3427_SAMPLE_RATE_14_BIT,
  MCP3427_SAMPLE_RATE_16_BIT,
  NUM_MCP3427_SAMPLE_RATES
} Mcp3427SampleRate;

typedef enum {
  MCP3427_CHANNEL_1 = 0,
  MCP3427_CHANNEL_2,
  NUM_MCP3427_CHANNELS
} Mcp3427Channel;

typedef enum {
  MCP3427_AMP_GAIN_1 = 0,
  MCP3427_AMP_GAIN_2,
  MCP3427_AMP_GAIN_4,
  MCP3427_AMP_GAIN_8,
  NUM_MCP3427_AMP_GAINS
} Mcp3427AmpGain;

typedef enum {
  MCP3427_CONVERSION_MODE_ONE_SHOT = 0,
  MCP3427_CONVERSION_MODE_CONTINUOUS,
  NUM_MCP3427_CONVERSION_MODES
} Mcp3427ConversionMode;

typedef struct Mcp3427Setting {
  Mcp3427SampleRate sample_rate;
  Mcp3427PinState Adr0;
  Mcp3427PinState Adr1;
  Mcp3427AmpGain amplifier_gain;
  Mcp3427ConversionMode conversion_mode;
  I2CPort port;
} Mcp3427Setting;

typedef struct Mcp3427Storage {
  I2CPort port;
  I2CAddress addr;
  uint8_t config;
} Mcp3427Storage;

// Initializes the ADC.
StatusCode mcp3427_init(Mcp3427Storage *storage, Mcp3427Setting *setting);

// Read value from a specific channel on the ADC.
StatusCode mcp3427_read(Mcp3427Storage *storage, Mcp3427Channel, uint16_t *data);

