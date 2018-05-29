#pragma once

// number of devices in daisy chain (including master)
// #define PLUTUS_CFG_AFE_DEVICES_IN_CHAIN 4
// #define PLUTUS_CFG_TOTAL_CELLS 36

#define PLUTUS_CFG_AFE_DEVICES_IN_CHAIN 1
#define PLUTUS_CFG_TOTAL_CELLS 6

// Using all 12 cell inputs
#define PLUTUS_CFG_INPUT_BITSET_FULL 0xFFF
// Using 6 cell inputs split across the 2 muxes
#define PLUTUS_CFG_INPUT_BITSET_SPLIT (0x7 << 6 | 0x7)
