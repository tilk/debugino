
#ifndef __DWIRE_H
#define __DWIRE_H

#include <stdbool.h>
#include <setjmp.h>
#include "uart_helper.h"

typedef struct {
  UARTHelper_HandleTypeDef *huarth;
  jmp_buf env;
  uint8_t regs[32];
  int pc;
  int breakpoint;
} DWire_HandleTypeDef;

void DWire_Send(DWire_HandleTypeDef *huarth, uint8_t *buf, size_t len);
void DWire_Receive(DWire_HandleTypeDef *dwire, uint8_t *buf, size_t len);
uint8_t DWire_ReceiveByte(DWire_HandleTypeDef *dwire);
uint16_t DWire_ReceiveWord(DWire_HandleTypeDef *dwire);
void DWire_SendByte(DWire_HandleTypeDef *dwire, uint8_t byte);
void DWire_ReadAddr(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count);
void DWire_WriteAddr(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count);
void DWire_Start(DWire_HandleTypeDef *dwire);
void DWire_Break(DWire_HandleTypeDef *dwire);
void DWire_Reconnect(DWire_HandleTypeDef *dwire);
void DWire_Reset(DWire_HandleTypeDef *dwire);
void DWire_Disable(DWire_HandleTypeDef *dwire);
void DWire_Step(DWire_HandleTypeDef *dwire);
void DWire_Run(DWire_HandleTypeDef *dwire);
void DWire_Continue(DWire_HandleTypeDef *dwire);
uint16_t DWire_Signature(DWire_HandleTypeDef *dwire);
void DWire_CheckSignature(DWire_HandleTypeDef *dwire);
uint8_t DWire_ReadFuseBitsLow(DWire_HandleTypeDef *dwire);
uint8_t DWire_ReadFuseBitsHigh(DWire_HandleTypeDef *dwire);
uint8_t DWire_ReadFuseBitsExt(DWire_HandleTypeDef *dwire);
uint8_t DWire_ReadLockBits(DWire_HandleTypeDef *dwire);
void DWire_ReadFlash(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count);
void DWire_WriteFlashPage(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count);

#endif

