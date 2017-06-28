#include <stdbool.h>

#include "adc.h"
#include "log.h"
#include "stm32f0xx.h"

#define TS_CAL1 0x1ffff7b8
#define TS_CAL2 0x1ffff7c2
#define VREFINT_CAL 0x1ffff7ba

typedef struct ADCInterrupt {
  ADCCallback callback;
  void *context;
  uint16_t reading;
} ADCInterrupt;

typedef struct ADCStatus {
  ADCChannel current_channel;
  bool continuous;
} ADCStatus;

static ADCInterrupt s_adc_interrupts[NUM_ADC_CHANNEL];
static ADCStatus s_adc_status;

// Get the first set bit left of the bit specified by the current channel 
static prv_get_ffs(uint32_t select, ADCChannel adc_channel) {
  select = select >> adc_channel + 1;
  uint8_t offset = 0;

  uint32_t ffs_max = NUM_ADC_CHANNEL - adc_channel;
  for (uint32_t i = 0; i < ffs_max; i++) {
    if (select & (1 << i)) {
      return (i + adc_channel + 1);
    }
  }
  return 0;
}

static int16_t prv_get_temp(int32_t reading) {
  uint16_t ts_cal1 = *(uint16_t*)TS_CAL1;
  uint16_t ts_cal2 = *(uint16_t*)TS_CAL2;

  reading = (110 - 30) * (reading - ts_cal1) / (ts_cal2 - ts_cal1) + 30;

  return reading;
}

static uint16_t prv_get_vdd(uint32_t reading) {
  uint16_t vrefint_cal = *(uint16_t*)VREFINT_CAL;
  reading = (3300 * vrefint_cal) / reading;
  return reading; 
}

void adc_init(ADCMode adc_mode) {
  // Stop ongoing conversions and disable the ADC
  ADC_StopOfConversion(ADC1);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADSTP)) { }

  ADC1->CR |= ADC_FLAG_ADDIS;
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)) { }

  // Once the ADC has been reset, enable it with the given settings
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_InitTypeDef adc_settings = {
    .ADC_Resolution = ADC_Resolution_12b,
    .ADC_ContinuousConvMode = adc_mode,
    .ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None,
    .ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO,
    .ADC_DataAlign = ADC_DataAlign_Right,
    .ADC_ScanDirection = ADC_ScanDirection_Upward,
  };

  ADC_Init(ADC1, &adc_settings);

  // Calculate the ADC calibration factor
  ADC_GetCalibrationFactor(ADC1);

  ADC_TempSensorCmd(ENABLE);
  ADC_VrefintCmd(ENABLE);

  ADC_ContinuousModeCmd(ADC1, adc_mode);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADCAL)) { }

  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)) { }

  // Set the ADC to wait mode
  ADC_WaitModeCmd(ADC1, ENABLE);
  ADC_AutoPowerOffCmd(ADC1, ENABLE);

  // Enable interrupts for the end of each conversion
  stm32f0xx_interrupt_nvic_enable(ADC1_COMP_IRQn, INTERRUPT_PRIORITY_NORMAL);
  ADC_ITConfig(ADC1, ADC_IER_EOCIE, ENABLE);

  // Initialize static varables
  s_adc_status.continuous = adc_mode;
  s_adc_status.current_channel = 0;

  // Start background conversions if in continuous
  if (adc_mode) {
    ADC_StartOfConversion(ADC1);
  }
}

StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state) {
  if (adc_channel >= NUM_ADC_CHANNEL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  if (new_state) {
    ADC_ChannelConfig(ADC1, (1 << adc_channel), ADC_SampleTime_239_5Cycles);
  } else {
    ADC1->CHSELR &= !(1 << adc_channel);
  }

  // The bridge divider is only enabled when needed to minimize consumption on the battery 
  if (adc_channel == ADC_CHANNEL_BAT) {
    ADC_VbatCmd(new_state);
  } 

  return STATUS_CODE_OK;
}

StatusCode adc_register_callback(ADCChannel adc_channel, ADCCallback callback, void *context) {
  // Returns invalid if the given address is not connected to an ADC channel
  if (adc_channel >= NUM_ADC_CHANNEL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_adc_interrupts[adc_channel].callback = callback;
  s_adc_interrupts[adc_channel].context = context;

  return STATUS_CODE_OK;
}

uint16_t adc_read_value(ADCChannel adc_channel) {
  if (!s_adc_status.continuous) {
    s_adc_status.current_channel = 0;
    ADC_StartOfConversion(ADC1);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOSEQ)) {}
  }

  return s_adc_interrupts[adc_channel].reading;
}

void ADC1_COMP_IRQHandler() {
  ADCChannel current_channel = s_adc_status.current_channel;
  uint32_t select = ADC1->CHSELR;
  uint16_t reading;

  if (select & (1 << current_channel)) {
    reading = ADC_GetConversionValue(ADC1);

    switch (current_channel) {
      case ADC_CHANNEL_TEMP:
        reading = prv_get_temp(reading);
        break;

      case ADC_CHANNEL_REF:
        reading = prv_get_vdd(reading);
        break;

      case ADC_CHANNEL_BAT:
        reading *= 2;
        break;
      
      default:
        break;
    }

    if (s_adc_interrupts[current_channel].callback != NULL) {
      s_adc_interrupts[current_channel].callback(current_channel, reading,
                                          s_adc_interrupts[current_channel].context);
    }
    s_adc_interrupts[current_channel].reading = reading;
  }

  current_channel = prv_get_ffs(select, current_channel);
  s_adc_status.current_channel = current_channel;
}
