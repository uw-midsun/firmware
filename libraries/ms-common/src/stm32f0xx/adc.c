#include <stdbool.h>

#include "adc.h"
#include "log.h"
#include "stm32f0xx.h"

#define ADC_CHANNELS_EXTERNAL 16
#define ADC_CHANNELS_INTERNAL 3
#define ADC_CHANNELS_TOTAL 19

typedef struct ADCInterrupt {
  adc_callback callback;
  uint16_t reading;
  void *context;
} ADCInterrupt;

typedef struct ADCStatus {
  bool continuous;
  ADCChannel current_channel;
  adc_callback *callback;
  uint16_t *reading;
  void **context;
} ADCStatus;

static ADCInterrupt s_adc_interrupts[ADC_CHANNELS_TOTAL];
static ADCStatus s_adc_status;

static void prv_adc_conversion_sequence() {
  ADC_StartOfConversion(ADC1);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOSEQ)) {}
}

static uint32_t prv_get_channel_select() {
  return ADC1->CHSELR;
}

void adc_init(ADCMode adc_mode) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_InitTypeDef adc_settings = {
    ADC_Resolution_12b,
    adc_mode,
    ADC_ExternalTrigConvEdge_None,
    ADC_ExternalTrigConv_T1_TRGO,
    ADC_DataAlign_Right,
    ADC_ScanDirection_Upward,
  };

  ADC_Init(ADC1, &adc_settings);

  // Calculate the ADC calibration factor 
  ADC_GetCalibrationFactor(ADC1);

  ADC_ContinuousModeCmd(ADC1, adc_mode);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADCAL)) { }

  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)) { }

  // Enable interrupts for the end of each conversion
  stm32f0xx_interrupt_nvic_enable(ADC1_COMP_IRQn, INTERRUPT_PRIORITY_NORMAL);
  ADC_ITConfig(ADC1, ADC_IER_EOCIE, ENABLE);

  // Initialize static varables
  s_adc_status.continuous = (ADC1->CFGR1 >> 13) & 1;
  s_adc_status.current_channel = 0;

  for (ADCChannel i = 0; i < ADC_CHANNELS_TOTAL; i++) {
    ADC_ChannelConfig(ADC1, i, ADC_SampleTime_239_5Cycles);
  }

  // Continuous conversions must be initiated by setting the ADSTART 
  if (s_adc_status.continuous) {
    ADC_StartOfConversion(ADC1);
  }
}

void adc_set_channel(ADCChannel adc_channel, bool new_state) {
  if (new_state) {
    ADC1->CHSELR |= (1 << adc_channel);
  } else {
    ADC1->CHSELR &= !(1 << adc_channel);
  }
}

StatusCode adc_interrupt_callback(uint8_t adc_channel, adc_callback callback, void *context) {

  // Returns invalid if the given address is not connected to an ADC channel 
  if (adc_channel >= 19) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_adc_interrupts[adc_channel].callback = callback;
  s_adc_interrupts[adc_channel].context = context;

  return STATUS_CODE_OK;
}

uint16_t adc_read(uint8_t adc_channel) {
  if (!s_adc_status.continuous) {
    prv_adc_conversion_sequence();
  }

  return s_adc_interrupts[adc_channel].reading;
}

void adc_disable() {
  ADC_StopOfConversion(ADC1);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADSTP)) { }

  ADC1->CR |= ADC_FLAG_ADDIS;
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)) { }
}

void ADC1_COMP_IRQHandler() {
  //ADC_StopOfConversion(ADC1);
  volatile uint32_t select = prv_get_channel_select();

  // Distinguish channels via bitshifting the ADC1_DR register
  s_adc_interrupts[s_adc_status.current_channel].reading = ADC_GetConversionValue(ADC1);

  // Set current channel equal to the next in the sequence
  for (ADCChannel i = 1; i < ADC_CHANNELS_TOTAL; i++) {
    if ((select >> (s_adc_status.current_channel + i) % ADC_CHANNELS_TOTAL) & 1) {
      s_adc_status.current_channel += i;
      s_adc_status.current_channel %= ADC_CHANNELS_TOTAL;
      break;
    }
  }
  //printf("interrupts\n");
  //ADC_StartOfConversion(ADC1);
}
