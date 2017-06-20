#include "adc.h"
#include "stm32f0xx.h"  

#include <stdio.h>

static uint8_t prv_get_channel(GPIOAddress *address) {
  uint8_t adc_channel = address->pin;

  switch (address->port) {
     case GPIO_PORT_A:
       if (address->pin > 7) { return 11; }
       break;

     case GPIO_PORT_B:
       if (address->pin > 1) { return 11; }
       adc_channel += 8;
       break;

     case GPIO_PORT_C:
       if (address->pin > 5) { return 11; }
       adc_channel += 10;
       break;

     default:
       return 11;
  }

  return adc_channel;
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
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADCAL)) { }

  // Enable the ADC
  ADC_Cmd(ADC1, ENABLE);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)) { }

  // Continuous conversions must be initiated by setting the ADSTART 
  if ((ADC1->CFGR1 >> 13) & 1) {
    ADC_StartOfConversion(ADC1);
  }
}

StatusCode adc_init_pin(GPIOAddress *address, ADCSampleRate adc_sample_rate) {
  uint8_t adc_channel = prv_get_channel(address);

  // Returns invalid if the given address is not connected to an ADC channel 
  if (adc_channel == 11) {
    return STATUS_CODE_INVALID_ARGS;
  }

  ADC_ChannelConfig(ADC1, adc_channel, adc_sample_rate);
  return STATUS_CODE_OK;
}

uint16_t adc_read(GPIOAddress *address, uint16_t max) {
  bool continuous_mode = (ADC1->CFGR1 >> 13) & 1;

  uint8_t adc_channel = prv_get_channel(address);
  
  if (adc_channel == 11) {
    return 0;
  }

  // In single mode, the ADSTART bit must be explicitly set for each conversion
  if (!continuous_mode) {
    ADC_StartOfConversion(ADC1);
  }

  ADC1->CHSELR = 1 << adc_channel;
  uint16_t adc_reading = ADC_GetConversionValue(ADC1); 
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) { }                        

  uint16_t reading = (max * adc_reading)/4096;

  return reading;
}

void adc_disable() {
  ADC_StopOfConversion(ADC1);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADSTP)) { }

  ADC1->CR |= ADC_FLAG_ADDIS;
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN)) { }
}