#include <stdbool.h>

#include "adc.h"
#include "log.h"
#include "stm32f0xx.h"

typedef struct ADCInterrupt {
  adc_callback callback;
  void *context;
  uint16_t reading;
} ADCInterrupt;

typedef struct ADCStatus {
  ADCChannel current_channel;
  bool continuous;
} ADCStatus;

static ADCInterrupt s_adc_interrupts[NUM_ADC_CHANNEL];
static ADCStatus s_adc_status;

static void prv_adc_conversion_sequence() {
  s_adc_status.current_channel = 0;
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
}

StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state) {
  if (adc_channel >= NUM_ADC_CHANNEL) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint32_t select = prv_get_channel_select();

  if (new_state) {
    ADC_ChannelConfig(ADC1, (1 << adc_channel), ADC_SampleTime_239_5Cycles);
  } else {
    ADC1->CHSELR &= !(1 << adc_channel);
  }

  return STATUS_CODE_OK;
}

StatusCode adc_register_callback(ADCChannel adc_channel, adc_callback callback, void *context) {
  // Returns invalid if the given address is not connected to an ADC channel
  if (adc_channel >= 19) {
    return STATUS_CODE_INVALID_ARGS;
  }

  s_adc_interrupts[adc_channel].callback = callback;
  s_adc_interrupts[adc_channel].context = context;

  return STATUS_CODE_OK;
}

void adc_start_continuous() {
  ADC_StartOfConversion(ADC1);
}

uint16_t adc_read_value(ADCChannel adc_channel) {
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
  ADC_WaitModeCmd(ADC1, ENABLE);
  ADCChannel current_channel = s_adc_status.current_channel;
  uint32_t select = prv_get_channel_select();
  uint16_t reading;

  // Only search for the next channel if more than one are active
  if ((select != (select & -(select))) || (select != 1 << current_channel))  {
    // Set current channel equal to the next in the sequence
    for (ADCChannel i = 1; i < NUM_ADC_CHANNEL; i++) {
      if ((select >> (current_channel + i) % NUM_ADC_CHANNEL) & 1) {
        current_channel += i;
        current_channel %= NUM_ADC_CHANNEL;
        break;
      }
    }
  }

  reading = ADC_GetConversionValue(ADC1);
  if (s_adc_interrupts[current_channel].callback != NULL) {
    s_adc_interrupts[current_channel].callback(current_channel, reading,
                                          s_adc_interrupts[current_channel].context);
  }

  s_adc_interrupts[current_channel].reading = reading;
  s_adc_status.current_channel = current_channel;
  return;
}
