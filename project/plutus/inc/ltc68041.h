#pragma once

#define LTC6804_ADCOPT                          (1 << 0)

#define LTC6804_SWTRD                           (1 << 1)

#define LTC6804_GPIO1_PD_OFF                    (1 << 3)
#define LTC6804_GPIO1_PD_ON                     (0 << 3)
#define LTC6804_GPIO2_PD_OFF                    (1 << 4)
#define LTC6804_GPIO2_PD_ON                     (0 << 4)
#define LTC6804_GPIO3_PD_OFF                    (1 << 5)
#define LTC6804_GPIO3_PD_ON                     (0 << 5)
#define LTC6804_GPIO4_PD_OFF                    (1 << 6)
#define LTC6804_GPIO4_PD_ON                     (0 << 6)
#define LTC6804_GPIO5_PD_OFF                    (1 << 7)
#define LTC6804_GPIO5_PD_ON                     (0 << 7)

#define LTC6804_CNVT_CELL_ALL                   0
#define LTC6804_CNVT_CELL_1_7                   1
#define LTC6804_CNVT_CELL_2_8                   2
#define LTC6804_CNVT_CELL_3_9                   3
#define LTC6804_CNVT_CELL_4_10                  4
#define LTC6804_CNVT_CELL_5_11                  5
#define LTC6804_CNVT_CELL_6_12                  6

#define LTC6804_WRCFG_RESERVED                  (1 << 0)

#define LTC6804_RDCFG_RESERVED                  (1 << 1)

#define LTC6804_ADCV_RESERVED                   ((1 << 9) | (1 << 6) | (1 << 5))
#define LTC6804_ADCV_DISCHARGE_PERMITTED        (1 << 4)

#define LTC6804_RDAUX_RESERVED                  ((1 << 3) | (1 << 2))

#define LTC6804_CLRCELL_RESERVED                (1 << 0) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6804_CLRSTAT_RESERVED                (1 << 0) | (1 << 1) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6804_PLADC_RESERVED                  (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6804_DIAGNC_RESERVED                 (1 << 0) | (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6804_WRCOMM_RESERVED                 (1 << 0) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6804_RDCOMM_RESERVED                 (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)

#define LTC6804_STCOMM_RESERVED                 (1 << 0) | (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)
