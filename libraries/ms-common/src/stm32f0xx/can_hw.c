#include "can_hw.h"
#include <string.h>
#include "interrupt_def.h"
#include "stm32f0xx.h"

#define CAN_HW_PRESCALER 12

static CANHwConfig *s_can = NULL;

StatusCode can_hw_init(CANHwConfig *can_hw, uint16_t bus_speed, bool loopback) {
  memset(can_hw, 0, sizeof(*can_hw));

  can_hw->base = CAN;
  can_hw->bus_speed = bus_speed;
  can_hw->loopback = loopback;

  s_can = can_hw;

  stm32f0xx_interrupt_nvic_enable(CEC_CAN_IRQn, INTERRUPT_PRIORITY_HIGH);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, ENABLE);

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);
  // time quanta = (APB1 / Prescaler) / baudrate, -1 for start bit
  uint16_t tq = clocks.PCLK_Frequency / 1000 / CAN_HW_PRESCALER / can_hw->bus_speed - 1;
  uint16_t bs1 = tq * 7 / 8;  // 87.5% sample point

  CAN_DeInit(can_hw->base);

  CAN_InitTypeDef can_cfg;
  CAN_StructInit(&can_cfg);

  can_cfg.CAN_Mode = can_hw->loopback ? CAN_Mode_Silent_LoopBack : CAN_Mode_Normal;
  can_cfg.CAN_SJW = CAN_SJW_1tq;
  can_cfg.CAN_BS1 = bs1;
  can_cfg.CAN_BS2 = tq - bs1;
  can_cfg.CAN_Prescaler = CAN_HW_PRESCALER;
  CAN_Init(can_hw->base, &can_cfg);

  CAN_ITConfig(can_hw->base, CAN_IT_TME, ENABLE);
  CAN_ITConfig(can_hw->base, CAN_IT_FMP0, ENABLE);
  CAN_ITConfig(can_hw->base, CAN_IT_FMP1, ENABLE);
  CAN_ITConfig(can_hw->base, CAN_IT_BOF, ENABLE);

  can_hw_add_filter(can_hw, 0, 0);
  can_hw->num_filters = 0;

  return STATUS_CODE_OK;
}

StatusCode can_hw_register_callback(CANHwConfig *can_hw, CANHwEvent event,
                                    CANHwEventHandlerCb callback, void *context) {
  if (event >= NUM_CAN_HW_EVENTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  can_hw->handlers[event] = (CANHwEventHandler){
    .callback = callback,  //
    .context = context,    //
  };

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter(CANHwConfig *can_hw, uint16_t mask, uint16_t filter) {
  if (can_hw->num_filters >= CAN_HW_MCU_NUM_FILTER_BANKS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filter banks.");
  }

  // We currently use 32-bit filters. We could use 16-bit filters instead since we're only using
  // standard 11-bit IDs.
  CAN_FilterInitTypeDef filter_cfg = {
    .CAN_FilterNumber = can_hw->num_filters,
    .CAN_FilterMode = CAN_FilterMode_IdMask,
    .CAN_FilterScale = CAN_FilterScale_32bit,
    .CAN_FilterIdHigh = filter << 5,
    .CAN_FilterIdLow = 0x0000,
    .CAN_FilterMaskIdHigh = mask << 5,
    .CAN_FilterMaskIdLow = 0x0000,
    .CAN_FilterFIFOAssignment = (can_hw->num_filters % 2),
    .CAN_FilterActivation = ENABLE,
  };
  CAN_FilterInit(&filter_cfg);

  can_hw->num_filters++;
  return STATUS_CODE_OK;
}

CANHwBusStatus can_hw_bus_status(const CANHwConfig *can_hw) {
  if (CAN_GetFlagStatus(can_hw->base, CAN_FLAG_BOF) == SET) {
    return CAN_HW_BUS_STATUS_OFF;
  } else if (CAN_GetFlagStatus(can_hw->base, CAN_FLAG_EWG) == SET ||
             CAN_GetFlagStatus(can_hw->base, CAN_FLAG_EPV) == SET) {
    return CAN_HW_BUS_STATUS_ERROR;
  }

  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(const CANHwConfig *can_hw, uint16_t id, const uint8_t *data,
                           size_t len) {
  CanTxMsg tx_msg = {
    .StdId = id,        //
    .IDE = CAN_ID_STD,  //
    .DLC = len,         //
  };

  memcpy(tx_msg.Data, data, sizeof(*data) * len);

  // TX returns failed on loopback
  uint8_t tx_status = CAN_Transmit(can_hw->base, &tx_msg);
  switch (tx_status) {
    case CAN_TxStatus_NoMailBox:
      // case CAN_TxStatus_Failed:
      return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW TX failed");
      break;
    default:
      break;
  }

  return STATUS_CODE_OK;
}

bool can_hw_receive(const CANHwConfig *can_hw, uint16_t *id, uint64_t *data, size_t *len) {
  // 0: No messages available
  // 1: FIFO0 has received a message
  // 2: FIFO1 has received a message
  // 3: Both have received messages: arbitrarily pick FIFO0
  uint8_t fifo_status = (CAN_GetITStatus(can_hw->base, CAN_IT_FMP0) == SET) |
                        (CAN_GetITStatus(can_hw->base, CAN_IT_FMP1) == SET) << 1;
  uint8_t fifo = (fifo_status == 2);

  if (fifo_status == 0) {
    // No messages available
    return false;
  }

  CanRxMsg rx_msg = { 0 };
  CAN_Receive(can_hw->base, fifo, &rx_msg);

  *id = rx_msg.StdId;
  *len = rx_msg.DLC;
  memcpy(data, rx_msg.Data, sizeof(*rx_msg.Data) * rx_msg.DLC);

  return true;
}

void CEC_CAN_IRQHandler(void) {
  if (s_can == NULL) {
    return;
  }

  bool run_cb[NUM_CAN_HW_EVENTS] = {[CAN_HW_EVENT_TX_READY] =
                                        CAN_GetITStatus(s_can->base, CAN_IT_TME) == SET,
                                    [CAN_HW_EVENT_MSG_RX] =
                                        CAN_GetITStatus(s_can->base, CAN_IT_FMP0) == SET ||
                                        CAN_GetITStatus(s_can->base, CAN_IT_FMP1) == SET,
                                    [CAN_HW_EVENT_BUS_ERROR] =
                                        CAN_GetITStatus(s_can->base, CAN_IT_BOF) == SET };

  for (int event = 0; event < NUM_CAN_HW_EVENTS; event++) {
    CANHwEventHandler *handler = &s_can->handlers[event];
    if (handler->callback != NULL && run_cb[event]) {
      handler->callback(handler->context);
    }
  }

  CAN_ClearITPendingBit(s_can->base, CAN_IT_TME);
}
