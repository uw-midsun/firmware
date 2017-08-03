#pragma once

// Driver for the TLV493D magnetic sensor

typedef enum {
  TLV493D_RESOLUTION_8_BIT = 0,
  TLV493D_RESOLUTION_12_BIT,
} TLV493DResolution;

typedef enum {
  TLV493D_HALL_PROBE_BX = 0,
  TLV493D_HALL_PROBE_BY,
  TLV493D_HALL_PROBE_BZ,
  NUM_TLV493D_HALL_PROBE
} TLV493DHallProbe;

StatusCode tlv493d_init(TLV493DResolution resolution);

StatusCode tlv493d_read_data(TLV493DHallProbe probe);