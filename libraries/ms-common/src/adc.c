#include "adc.h"

static uint8_t prv_get_channel(GPIOAddress* address) {
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

void adc_init(ADCMode adc_mode) {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  ADC_InitTypeDef adc_settings = {
    ADC_Resolution_10b,
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
}

bool adc_init_pin(GPIOAddress* address, ADCSampleRate adc_sample_rate) {
  uint8_t adc_channel = prv_get_channel(address);
  ADC_ChannelConfig(ADC1, adc_channel, adc_sample_rate);
  return 1;
}

uint16_t adc_read(GPIOAddress* address, uint16_t max) {
	bool continuous_mode = ADC1->CFGR1 >> 13;
	
	if (!continuous_mode) {
		ADC_StartOfConversion(ADC1);
	}

	ADC1->CFGR1 ^= (ADC1->CFGR1 >> 13) ? ADC_CR_ADSTART : 0;
  ADC1->CHSELR = 1 << prv_get_channel(address);
	ADC1->CFGR1 ^= (ADC1->CFGR1 >> 13) ? ADC_CR_ADSTART : 0;
  
  uint16_t adc_reading = ADC_GetConversionValue(ADC1);
  uint16_t reading = (max * adc_reading)/1024;

  return reading;
}

uint16_t adc_average(GPIOAddress* address, uint16_t period, uint16_t max) {
  uint16_t average = 0;
  for (uint16_t i = 0; i < period; i++) {
      average += adc_read(address, max);
  }

  average /= period;
  return average;
}
