#include "tlv493d.h"
#include "i2c.h"

// Array to hold 
static TLV493DHallProbe s_hall_probe[NUM_TLV493D_HALL_PROBE];

// Maybe start by broadcasting to address 0x0 to reset the device
StatusCode tlv493d_init(TLV493DResolution resolution) { }