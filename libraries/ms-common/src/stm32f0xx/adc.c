#include <stdbool.h>

#include "adc.h"
#include "log.h"
#include "stm32f0xx.h"

// TS_CAL addresses obtained from section 3.10.1 of the specific device
// datasheet
#define TS_CAL1 0x1FFFF7b8
#define TS_CAL2 0x1FFFF7c2

// VREFINT_CAL address obtained from section 3.10.2 of the specific device
// datasheet
#define VREFINT_CAL 0x1FFFF7ba

typedef struct ADCInterrupt {
  ADCCallback callback;
  void *context;
  uint16_t reading;
} ADCInterrupt;

typedef struct ADCStatus {
  uint32_t sequence;
  bool continuous;
} ADCStatus;

static ADCInterrupt s_adc_interrupts[NUM_ADC_CHANNELS];
static ADCStatus s_adc_status;

// Formula obtained from section 13.9 of the reference manual. Returns reading
// in kelvin
static uint16_t prv_get_temp(uint16_t reading) {
  uint16_t ts_cal1 = *(uint16_t *)TS_CAL1;
  uint16_t ts_cal2 = *(uint16_t *)TS_CAL2;

  reading = ((110 - 30) * (reading - ts_cal1)) / (ts_cal2 - ts_cal1) + 30;

  return reading + 273;
}

// Formula obtained from section 13.9 of the reference manual. Returns Vdda in
// mV
static uint16_t prv_get_vdda(uint16_t reading) {
  uint16_t vrefint_cal = *(uint16_t *)VREFINT_CAL;
  reading = (3300 * vrefint_cal) / reading;
  return reading;
}

void adc_init(ADCMode adc_mode) {
  ADC_DeInit(ADC1);

  // Once the ADC has been reset, enable it with the given settings
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, true);

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

  ADC_ContinuousModeCmd(ADC1, adc_mode);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADCAL)) {
  }

  // Enable the ADC
  ADC_Cmd(ADC1, true);
  while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY)) {
  }

  ADC_WaitModeCmd(ADC1, true);
  ADC_AutoPowerOffCmd(ADC1, !adc_mode);

  // Enable interrupts for the end of each conversion
  stm32f0xx_interrupt_nvic_enable(ADC1_COMP_IRQn, INTERRUPT_PRIORITY_NORMAL);
  ADC_ITConfig(ADC1, ADC_IER_EOCIE, true);
  ADC_ITConfig(ADC1, ADC_IER_EOSEQIE, true);

  // Initialize static variables
  s_adc_status.continuous = adc_mode;
  s_adc_status.sequence = 0;

  if (adc_mode) {
    ADC_StartOfConversion(ADC1);
  }

  // Configure internal reference channel to run by default for voltage
  // conversions
  adc_set_channel(ADC_CHANNEL_REF, true);
}

StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (new_state) {
    ADC_ChannelConfig(ADC1, ((uint32_t)1 << adc_channel),
                      ADC_SampleTime_239_5Cycles);
  } else {
    ADC1->CHSELR &= ~((uint32_t)1 << adc_channel);
  }

  // Keep internal channels enabled only when set
  switch (adc_channel) {
  case ADC_CHANNEL_BAT:
    ADC_VbatCmd(new_state);
    break;

  case ADC_CHANNEL_REF:
    ADC_VrefintCmd(new_state);
    break;

  case ADC_CHANNEL_TEMP:
    ADC_TempSensorCmd(new_state);
    break;

  default:
    break;
  }

  s_adc_status.sequence = ADC1->CHSELR;
  return STATUS_CODE_OK;
}

// ADC Channel to GPIO Address mapping found in table 13 of the specific device
// datasheet.
// Channels 0 to 7 are occupied by port A, 8 to 9 by prt B, and 10 to 15 by port
// C
StatusCode adc_get_channel(GPIOAddress address, ADCChannel *adc_channel) {
  *adc_channel = address.pin;

  switch (address.port) {
  case GPIO_PORT_A:
    if (address.pin > 7) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
    break;
  case GPIO_PORT_B:
    if (address.pin > 1) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
    *adc_channel += 8;
    break;
  case GPIO_PORT_C:
    if (address.pin > 5) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
    *adc_channel += 10;
    break;
  }

  if (*adc_channel > ADC_CHANNEL_15) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  return STATUS_CODE_OK;
}

StatusCode adc_register_callback(ADCChannel adc_channel, ADCCallback callback,
                                 void *context) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (!(ADC1->CHSELR & ((uint32_t)1 << adc_channel))) {
    return status_code(STATUS_CODE_EMPTY);
  }

  s_adc_interrupts[adc_channel].callback = callback;
  s_adc_interrupts[adc_channel].context = context;

  return STATUS_CODE_OK;
}

StatusCode adc_read_raw(ADCChannel adc_channel, uint16_t *reading) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (!(ADC1->CHSELR & ((uint32_t)1 << adc_channel))) {
    return status_code(STATUS_CODE_EMPTY);
  }

  if (!s_adc_status.continuous) {
    ADC_StartOfConversion(ADC1);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOSEQ)) {
    }
  }

  *reading = s_adc_interrupts[adc_channel].reading;

  return STATUS_CODE_OK;
}

StatusCode adc_read_converted(ADCChannel adc_channel, uint16_t *reading) {
  if (adc_channel >= NUM_ADC_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (!(ADC1->CHSELR & ((uint32_t)1 << adc_channel))) {
    return status_code(STATUS_CODE_EMPTY);
  }

  uint16_t adc_reading;
  adc_read_raw(adc_channel, &adc_reading);

  switch (adc_channel) {
  case ADC_CHANNEL_TEMP:
    *reading = prv_get_temp(adc_reading);
    return STATUS_CODE_OK;

  case ADC_CHANNEL_REF:
    *reading = prv_get_vdda(adc_reading);
    return STATUS_CODE_OK;

  case ADC_CHANNEL_BAT:
    adc_reading *= 2;
    break;

  default:
    break;
  }

  uint16_t vdda;
  adc_read_converted(ADC_CHANNEL_REF, &vdda);
  *reading = (adc_reading * vdda) / 4095;

  return STATUS_CODE_OK;
}

void ADC1_COMP_IRQHandler() {
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC)) {
    ADCChannel current_channel = __builtin_ctz(s_adc_status.sequence);

    uint16_t reading = ADC_GetConversionValue(ADC1);

    if (s_adc_interrupts[current_channel].callback != NULL) {
      s_adc_interrupts[current_channel].callback(
          current_channel, s_adc_interrupts[current_channel].context);
    }

    s_adc_interrupts[current_channel].reading = reading;
    s_adc_status.sequence &= ~((uint32_t)1 << current_channel);
  }

  if (ADC_GetITStatus(ADC1, ADC_IT_EOSEQ)) {
    s_adc_status.sequence = ADC1->CHSELR;
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOSEQ);
  }
}
