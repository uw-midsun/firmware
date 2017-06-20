#include "adc.h"
#include "stm32f0xx.h"  

#include <stdio.h>

uint32_t prv_get_channel(GPIOAddress* address) {
  volatile uint8_t adc_channel = address->pin;

  switch (address->port) {
     case GPIO_PORT_A:
       if (address->pin > 7) { return -1; }
       break;

     case GPIO_PORT_B:
       if (address->pin > 1) { return -1; }
       adc_channel += 8;
       break;

     case GPIO_PORT_C:
       if (address->pin > 5) { return -1; }
       adc_channel += 10;
       break;

     default:
       return -1;
  }

  return adc_channel;
}

static uint16_t prv_adc_read_single(GPIOAddress* address, uint8_t adc_channel) {
  // In single mode, the ADSTART bit must be explicitly set
  ADC1->CR ^= ADC_CR_ADSTART;
  ADC1->CHSELR = 1 << adc_channel;
  return ADC_GetConversionValue(ADC1);
}


static uint16_t prv_adc_read_continuous(GPIOAddress* address, uint8_t adc_channel) {
  ADC1->CHSELR = 1 << adc_channel;
  return ADC_GetConversionValue(ADC1);
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

  ADC_GetCalibrationFactor(ADC1);

  // Calculate the ADC calibration factor 
  ADC_ContinuousModeCmd(ADC1, adc_mode);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADCAL)) {}

  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)) {}
  
  ADC_StartOfConversion(ADC1);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {}
}

void adc_init_pin(GPIOAddress* address, ADCSampleRate adc_sample_rate) {
  uint8_t adc_channel = prv_get_channel(address);
  ADC_ChannelConfig(ADC1, adc_channel, adc_sample_rate);
  return 1;
}

uint16_t adc_read(GPIOAddress* address, uint16_t max) {
  bool continuous_mode = ADC1->CFGR1 >> 13;
  uint8_t adc_channel = prv_get_channel(address);
  uint16_t adc_reading = (continuous_mode) ?
                          prv_adc_read_continuous(address, adc_channel) :
                          prv_adc_read_single(address, adc_channel);

  uint16_t reading = (max * adc_reading)/4096;

  return reading;
}


