
#include "debugino.h"
#include "main.h"
#include "queue_io.h"
#include "stk500v2.h"
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 512

#define CONFIG_PARAM_BUILD_NUMBER_LOW 0
#define CONFIG_PARAM_BUILD_NUMBER_HIGH 0
#define CONFIG_PARAM_HW_VER 0x0F
#define CONFIG_PARAM_SW_MAJOR 2
#define CONFIG_PARAM_SW_MINOR 0x0A

static uint8_t buffer[BUFFER_SIZE];
static const char debugger_id[] = "debugino";
static uint8_t txseq = 1;

static inline int RecvCharCksum(uint8_t *cksum)
{
  int ret = Queue_RecvChar(queueUSBtoDEBUGHandle);
  *cksum ^= ret;
  return ret;
}

static inline void SendCharCksum(uint8_t *cksum, int ch)
{
  *cksum ^= ch;
  Queue_SendChar(queueDEBUGtoUSBHandle, ch);
}

int ReceiveSTK500v2()
{
  uint8_t cksum = STK500v2_MESSAGE_START;
  RecvCharCksum(&cksum);
  int sz = RecvCharCksum(&cksum) << 8;
  sz |= RecvCharCksum(&cksum);
  if (sz > BUFFER_SIZE || sz < 0) return -1;
  int tok = RecvCharCksum(&cksum);
  if (tok != STK500v2_TOKEN) return -1;
  int rsz = Queue_Recv(queueUSBtoDEBUGHandle, buffer, sz);
  if (rsz != sz) return -1;
  RecvCharCksum(&cksum);
  for (int i = 0; i < sz; i++) cksum ^= buffer[i];
  if (cksum != 0) return -1;
  return sz;
}

void SendSTK500v2(int sz)
{
  uint8_t cksum = 0;
  SendCharCksum(&cksum, STK500v2_MESSAGE_START);
  SendCharCksum(&cksum, txseq++);
  SendCharCksum(&cksum, (sz >> 8) & 0xff);
  SendCharCksum(&cksum, sz & 0xff);
  SendCharCksum(&cksum, STK500v2_TOKEN);
  Queue_Send(queueDEBUGtoUSBHandle, buffer, sz);
  for (int i = 0; i < sz; i++) cksum ^= buffer[i];
  Queue_SendChar(queueDEBUGtoUSBHandle, cksum);
}

void HandleSTK500v2()
{
  int sz = ReceiveSTK500v2();
  if (sz <= 0) return;
  switch (buffer[0]) {
    case STK500v2_CMD_SIGN_ON: {
      buffer[1] = STK500v2_STATUS_CMD_OK;
      buffer[2] = sizeof(debugger_id) - 1;
      strcpy((char*)&buffer[3], debugger_id);
      SendSTK500v2(sizeof(debugger_id) + 2);
      break;
    }
    case STK500v2_CMD_GET_PARAMETER: {
      uint8_t value;
      switch (buffer[1]) {
        case STK500v2_PARAM_BUILD_NUMBER_LOW: value = CONFIG_PARAM_BUILD_NUMBER_LOW; break;
        case STK500v2_PARAM_BUILD_NUMBER_HIGH: value = CONFIG_PARAM_BUILD_NUMBER_HIGH; break;
        case STK500v2_PARAM_HW_VER: value = CONFIG_PARAM_HW_VER; break;
        case STK500v2_PARAM_SW_MAJOR: value = CONFIG_PARAM_SW_MAJOR; break;
        case STK500v2_PARAM_SW_MINOR: value = CONFIG_PARAM_SW_MINOR; break;
        default: value = 0; break;
      }
      buffer[1] = STK500v2_STATUS_CMD_OK;
      buffer[2] = value;
      SendSTK500v2(3);
      break;
    }
    case STK500v2_CMD_SET_PARAMETER: {
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      break;
    }
    case STK500v2_CMD_LEAVE_PROGMODE_ISP: {
      // TODO: DWIRE reset and start
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      txseq = 1;
      break;
    }
    case STK500v2_CMD_ENTER_PROGMODE_ISP: {
      // TODO: DWIRE break
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      break;
    }
    case STK500v2_CMD_READ_SIGNATURE_ISP: {
      uint8_t signatureIndex = buffer[4];
      uint8_t signature;
      if (signatureIndex == 0) {
        signature = 0;
      } else if (signatureIndex == 1) {
        signature = 1;
      } else signature = 2;
      buffer[1] = STK500v2_STATUS_CMD_OK;
      buffer[2] = signature;
      buffer[3] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(4);
      break;
    }
    case STK500v2_CMD_LOAD_ADDRESS:
    case STK500v2_CMD_PROGRAM_FLASH_ISP:
    case STK500v2_CMD_PROGRAM_EEPROM_ISP:
    case STK500v2_CMD_READ_FLASH_ISP:
    case STK500v2_CMD_READ_EEPROM_ISP:
    default: {
      buffer[1] = STK500v2_STATUS_CMD_FAILED;
      SendSTK500v2(2);
      break;
    }
  }
}

void HandleGDB()
{
}

void DebugLoop()
{
  if (Queue_PollRecv(queueUSBtoDEBUGHandle)) {
    // debugger command available
    int ch = Queue_RecvChar(queueUSBtoDEBUGHandle);
    if (ch == STK500v2_MESSAGE_START) {
      HandleSTK500v2();
    } else if (ch == '$') {
      HandleGDB();
    }
  } else if (UARTHelper_PollRecv(&huarth3)) {
    // unexpected data on dwire
    // throw it away
    uint8_t buf;
    while (UARTHelper_TryRecv(&huarth3, &buf, 1));
  } else if (UARTHelper_PollBreak(&huarth3)) {
    // break received from MCU
  } else {
    fflush(stdout);
    osDelay(1);
  }
}

