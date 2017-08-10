#pragma once
// driver for LTC6804-1 AFE chip
// requires GPIO, Interrupts and Soft Timers to be initialized

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"

// static_assert shim macro since we aren't using C11
// taken from: http://www.pixelbeat.org/programming/gcc/static_assert.html
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
/* These can't be used after statements in c89. */
#ifdef __COUNTER__
  #define STATIC_ASSERT(e, m) \
    ; enum { ASSERT_CONCAT(static_assert_, __COUNTER__) = 1/(int)(!!(e)) }
#else
  /* This can't be used twice on the same line so ensure if using in headers
   * that the headers are not included twice (by wrapping in #ifndef...#endif)
   * Note it doesn't cause an issue when used on same line of separate modules
   * compiled with gcc -combine -fwhole-program.  */
  #define STATIC_ASSERT(e, m) \
    ; enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(int)(!!(e)) }
#endif

#define SWAP_UINT16(x) (uint16_t)(((uint16_t)(x) >> 8) | ((uint16_t)(x) << 8))

#if defined(__GNUC__)
#  define _PACKED __attribute__((packed))
#else
#  define _PACKED
#endif

// select the ADC mode (trade-off between speed or minimizing noise)
// see p.50 for conversion times and p.23 for noise
typedef enum {
  LTC_AFE_ADC_MODE_27KHZ = 0,
  LTC_AFE_ADC_MODE_7KHZ,
  LTC_AFE_ADC_MODE_26HZ,
  LTC_AFE_ADC_MODE_14KHZ,
  LTC_AFE_ADC_MODE_3KHZ,
  LTC_AFE_ADC_MODE_2KHZ,
  NUM_LTC_AFE_ADC_MODE
} LTCAFEADCMode;

typedef struct LTCAFESettings {
  GPIOAddress cs;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;

  const SPIPort spi_port;
  uint32_t spi_baudrate;

  LTCAFEADCMode adc_mode;
} LTCAFESettings;

typedef struct {
  unsigned int adcopt : 1;
  unsigned int swtrd : 1;
  unsigned int refon : 1;

  unsigned int gpio1 : 1;             // GPIO1 Pin Control
  unsigned int gpio2 : 1;             // GPIO2 Pin Control
  unsigned int gpio3 : 1;             // GPIO3 Pin Control
  unsigned int gpio4 : 1;             // GPIO4 Pin Control
  unsigned int gpio5 : 1;             // GPIO5 Pin Control

  unsigned int undervoltage : 12;     // Undervoltage Comparison Voltage
  unsigned int overvoltage : 12;      // Overvoltage Comparison Voltage

  unsigned int discharge_c1 : 1;
  unsigned int discharge_c2 : 1;
  unsigned int discharge_c3 : 1;
  unsigned int discharge_c4 : 1;
  unsigned int discharge_c5 : 1;
  unsigned int discharge_c6 : 1;
  unsigned int discharge_c7 : 1;
  unsigned int discharge_c8 : 1;
  unsigned int discharge_c9 : 1;
  unsigned int discharge_c10 : 1;
  unsigned int discharge_c11 : 1;
  unsigned int discharge_c12 : 1;

  unsigned int discharge_timeout : 4;
} _PACKED LTCAFEConfigRegisterData;
STATIC_ASSERT(sizeof(LTCAFEConfigRegisterData) == 6,
              "LTCAFEConfigRegisterData must be 6 bytes");

// initialize the LTC6804
StatusCode ltc_afe_init(const LTCAFESettings *afe);

// read all voltages
// result is an array of size LTC6804_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN
// len should be SIZEOF_ARRAY(result)
StatusCode ltc_afe_read_all_voltage(const LTCAFESettings *afe, uint16_t *result, size_t len);

// read all auxiliary registers
// result should be an array of size LTC6804_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN
// len should be SIZEOF_ARRAY(result)
StatusCode ltc_afe_read_all_aux(const LTCAFESettings *afe, uint16_t *result, size_t len);

// mark cells for discharging (takes effect after config is re-written)
StatusCode ltc_afe_toggle_discharge_cells(const LTCAFESettings *afe, uint16_t cell, bool discharge);
