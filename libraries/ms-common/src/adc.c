#include "adc.h"

//
#include <stdio.h>
//

static uint8_t prv_get_channel(GPIOAddress* address) {
  uint8_t adc_channel = ADC_Channel_0 + address->pin;

  switch (address->port) {
     case GPIO_PORT_A:
       if (address->pin > 7) { return 0; }
       break;

     case GPIO_PORT_B:
       if (address->pin > 1) { return 0; }
       adc_channel += 8;
       break;

     case GPIO_PORT_C:
       if (address->pin > 5) { return 0; }
       adc_channel += 10;
       break;

     default:
       return 0;
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
  ADC_Cmd(ADC1, ENABLE);

  ADC_ContinuousModeCmd(ADC1, adc_mode);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADCAL)) {}

  ADC_StartOfConversion(ADC1);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {}
  //ADC_StopOfConversion(ADC1);
}

bool adc_init_pin(GPIOAddress* address, ADCSampleRate adc_sample_rate) {
  uint8_t adc_channel = prv_get_channel(address);
  ADC_ChannelConfig(ADC1, adc_channel, adc_sample_rate);
  return 1;
}

uint16_t adc_read(GPIOAddress* address, uint16_t max_voltage) {

  // while(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADSTART)) {} 
  
  //uint16_t adc_channel = prv_get_channel(address);

  /*if (!(ADC1->CHSELR >> adc_channel)) {
    return 0;
  }*/

  //uint32_t temp = ADC1->CHSELR;

  printf("ADC_CHSELR: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(ADC1->CHSELR));  
  
  uint16_t adc_reading = ADC_GetConversionValue(ADC1);
  uint16_t voltage = (max_voltage * adc_reading)/4096;

  return voltage;
}

