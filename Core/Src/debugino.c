
#include "debugino.h"
#include "dwire.h"
#include "main.h"
#include "queue_io.h"
#include "stk500v2.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>

#define BUFFER_SIZE 512

#define CONFIG_PARAM_BUILD_NUMBER_LOW 0
#define CONFIG_PARAM_BUILD_NUMBER_HIGH 0
#define CONFIG_PARAM_HW_VER 0x0F
#define CONFIG_PARAM_SW_MAJOR 2
#define CONFIG_PARAM_SW_MINOR 0x0A

static uint8_t buffer[BUFFER_SIZE];
static char * const cbuffer = (char*)buffer;
static const char debugger_id[] = "debugino";
static uint8_t txseq = 1;
static DWire_HandleTypeDef hdw;
static uint32_t address;
static bool reply_on_break = false;

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
  if (setjmp(hdw.env)) {
    // there was a DWIRE error, send a failure
    buffer[1] = STK500v2_STATUS_CMD_FAILED;
    SendSTK500v2(2);
    return;
  }
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
      DWire_Reset(&hdw);
      DWire_Run(&hdw);
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      txseq = 1;
      break;
    }
    case STK500v2_CMD_ENTER_PROGMODE_ISP: {
      DWire_Break(&hdw);
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      break;
    }
    case STK500v2_CMD_READ_SIGNATURE_ISP: {
      uint8_t signatureIndex = buffer[4];
      uint8_t sigbyte;
      uint16_t signature = DWire_Signature(&hdw);
      if (signatureIndex == 0) {
        sigbyte = 0x1e;
      } else if (signatureIndex == 1)
        sigbyte = signature >> 8;
      else 
        sigbyte = signature & 0xff;
      buffer[1] = STK500v2_STATUS_CMD_OK;
      buffer[2] = sigbyte;
      buffer[3] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(4);
      break;
    }
    case STK500v2_CMD_READ_FUSE_ISP: {
      uint8_t fuseBits;
      if (buffer[2] == 0x50) {
        if (buffer[3] == 0x08) fuseBits = DWire_ReadFuseBitsExt(&hdw);
        else fuseBits = DWire_ReadFuseBitsLow(&hdw);
      } else fuseBits = DWire_ReadFuseBitsHigh(&hdw);
      buffer[1] = STK500v2_STATUS_CMD_OK;
      buffer[2] = fuseBits;
      buffer[3] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(4);
      break;
    }
    case STK500v2_CMD_READ_LOCK_ISP: {
      buffer[1] = STK500v2_STATUS_CMD_OK;
      buffer[2] = DWire_ReadLockBits(&hdw);
      buffer[3] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(4);
      break;
    }
    case STK500v2_CMD_LOAD_ADDRESS: {
      address = ((uint32_t)(buffer[1])<<24) | ((uint32_t)(buffer[2])<<16) | ((uint32_t)(buffer[3])<<8) | buffer[4];
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      break;
    }
    case STK500v2_CMD_READ_FLASH_ISP: {
      uint16_t count = (buffer[1]<<8) | buffer[2];
      if (count > BUFFER_SIZE - 3) {
        buffer[1] = STK500v2_STATUS_CMD_FAILED;
        SendSTK500v2(2);
        break;
      }
      DWire_ReadFlash(&hdw, address * 2, &buffer[2], count);
      address += count/2;
      buffer[1] = STK500v2_STATUS_CMD_OK;
      buffer[count+2] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(count + 3);
      break;
    }
    case STK500v2_CMD_CHIP_ERASE_ISP: {
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      break;
    }
    case STK500v2_CMD_PROGRAM_FLASH_ISP: {
      // TODO: handle situations different than single page writes
      uint16_t count = (buffer[1]<<8) | buffer[2];
      if (count > BUFFER_SIZE - 3) {
        buffer[1] = STK500v2_STATUS_CMD_FAILED;
        SendSTK500v2(2);
        break;
      }
      DWire_WriteFlashPage(&hdw, address * 2, &buffer[10], count);
      address += count/2;
      buffer[1] = STK500v2_STATUS_CMD_OK;
      SendSTK500v2(2);
      break;
    }
    case STK500v2_CMD_PROGRAM_EEPROM_ISP:
    case STK500v2_CMD_READ_EEPROM_ISP:
    default: {
      buffer[1] = STK500v2_STATUS_CMD_FAILED;
      SendSTK500v2(2);
      break;
    }
  }
}

static int hex2val(int ch)
{
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  return -1;
}

static int val2hex(int val)
{
  if (val >= 0 && val <= 9) return val + '0';
  if (val >= 10 && val <= 15) return val + 'a';
  return '?';
}

static int hex2buf(uint8_t *buf, const char *data, int count)
{
  int n;
  for (n = 0; n < count && data[2*n] && data[2*n+1]; n++) {
    int v = (hex2val(data[2*n]) << 8) | hex2val(data[2*n+1]);
    if (v < 0) break;
    buf[n] = v;
  }
  return n;
}

static void buf2hex(const uint8_t *buf, char *data, int count)
{
  for (int n = 0; n < count; n++) {
    data[2*n] = val2hex(buf[n] >> 8);
    data[2*n+1] = val2hex(buf[n] & 0xf);
  }
}

int ReceiveGDB()
{
  uint8_t cksum = 0;
  int sz;
  for (sz = 0; sz < BUFFER_SIZE-1; sz++) {
    int ch = Queue_RecvChar(queueUSBtoDEBUGHandle);
    cksum += ch;
    if (ch == '#') break;
    if (ch == '}') {
      ch = Queue_RecvChar(queueUSBtoDEBUGHandle);
      cksum += ch;
      ch ^= 0x20;
    }
    buffer[sz] = ch;
  }
  buffer[sz] = 0;
  int ch1 = hex2val(Queue_RecvChar(queueUSBtoDEBUGHandle));
  int ch2 = hex2val(Queue_RecvChar(queueUSBtoDEBUGHandle));
  if (ch1 < 0 || ch2 < 0 || cksum != ((ch1 << 8) | ch2)) {
    Queue_SendChar(queueDEBUGtoUSBHandle, '-');
    return -1;
  } else {
    Queue_SendChar(queueDEBUGtoUSBHandle, '+');
    return sz;
  }
}

void SendGDB(int sz)
{
  uint8_t cksum = 0;
  Queue_SendChar(queueDEBUGtoUSBHandle, '$');
  for (int i = 0; i < sz; i++) {
    int ch = buffer[i];
    if (ch == '#' || ch == '}' || ch == '$') {
      Queue_SendChar(queueDEBUGtoUSBHandle, '}');
      cksum += '}';
      ch ^= 0x20;
    }
    Queue_SendChar(queueDEBUGtoUSBHandle, ch);
    cksum += ch;
  }
  Queue_SendChar(queueDEBUGtoUSBHandle, '#');
  Queue_SendChar(queueDEBUGtoUSBHandle, val2hex(cksum >> 8));
  Queue_SendChar(queueDEBUGtoUSBHandle, val2hex(cksum & 0xf));

  Queue_RecvChar(queueUSBtoDEBUGHandle);
}

void gdbprintf(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  int sz = vsprintf((char*)buffer, fmt, va);
  va_end(va);
  SendGDB(sz);
}

void SendGDBError(int no)
{
  gdbprintf("E%.2x", no);
}

void SendGDBStatus(int sig)
{
  gdbprintf("S%.2x", sig);
}

void SendGDBOK()
{
  gdbprintf("OK");
}

void HandleGDB()
{
  if (!hdw.stopped) {
    DWire_Break(&hdw);
  }
  int sz = ReceiveGDB();
  if (sz <= 0) return;
  switch (buffer[0]) {
    case 'm': {
      int addr, len;
      if (sscanf(cbuffer, "m%x,%x", &addr, &len) < 2) {
        SendGDBError(1);
        break;
      }
      if (len > BUFFER_SIZE/2) {
        SendGDBError(1);
        break;
      } 
      int space = addr & SWD_ADDR_SPACE_MASK;
      if (space < SWD_DATA_SPACE_OFFSET) {
        DWire_ReadFlash(&hdw, addr - SWD_DATA_SPACE_OFFSET, buffer + BUFFER_SIZE - len, len);
        buf2hex(buffer + BUFFER_SIZE - len, cbuffer, len);
        SendGDB(len*2);
      } else switch(space) {
        case SWD_DATA_SPACE_OFFSET: {
          DWire_ReadAddr(&hdw, addr - SWD_DATA_SPACE_OFFSET, buffer + BUFFER_SIZE - len, len);
          buf2hex(buffer + BUFFER_SIZE - len, cbuffer, len);
          SendGDB(len*2);
          break;
        }
        default:
          // TODO: support more address spaces
          SendGDBError(1);
          break;
      }
      break;
    }
    case 'M': {
      int addr, len, pos;
      if (sscanf(cbuffer, "M%x,%x:%n", &addr, &len, &pos) < 2) {
        SendGDBError(1);
        break;
      }
      if (len > (BUFFER_SIZE - pos)/2) {
        SendGDBError(1);
        break;
      }
      int space = addr & SWD_ADDR_SPACE_MASK;
      if (space < SWD_DATA_SPACE_OFFSET) {
        // TODO: flash writing by gdb
        SendGDBError(1);
        break;
      } else switch(space) {
        case SWD_DATA_SPACE_OFFSET: {
          hex2buf(buffer, cbuffer + pos, len);
          DWire_WriteAddr(&hdw, addr - SWD_DATA_SPACE_OFFSET, buffer, len);
          SendGDBOK();
        }
        default:
          // TODO: support more address spaces
          SendGDBError(1);
          break;
      }
      break;
    }
    case 'X': {
      int addr, len, pos;
      if (sscanf(cbuffer, "X%x,%x:%n", &addr, &len, &pos) < 2) {
        SendGDBError(1);
        break;
      }
      if (len > (BUFFER_SIZE - pos)) {
        SendGDBError(1);
        break;
      }
      int space = addr & SWD_ADDR_SPACE_MASK;
      if (space < SWD_DATA_SPACE_OFFSET) {
        // TODO: flash writing by gdb
        SendGDBError(1);
        break;
      } else switch(space) {
        case SWD_DATA_SPACE_OFFSET: {
          DWire_WriteAddr(&hdw, addr - SWD_DATA_SPACE_OFFSET, buffer + pos, len);
          SendGDBOK();
        }
        default:
          // TODO: support more address spaces
          SendGDBError(1);
          break;
      }
      SendGDBOK();
      break;
    }
    case 'v': {
      // TODO: support advanced features
      //char *epos = strpbrk(cbuffer + 1, ":;?");
      //if (!epos) epos = cbuffer + strlen(cbuffer);
      //int n = epos - cbuffer;
      SendGDB(0);
      break;
    }
    case 'q': {
      char *epos = strchr(cbuffer + 1, ':');
      if (!epos) epos = cbuffer + strlen(cbuffer);
      int n = epos - cbuffer;
      if (strncmp(cbuffer, "qSupported", n) == 0) {
        gdbprintf("PacketSize=%d", BUFFER_SIZE - 32);
      } else if (strncmp(cbuffer, "qOffsets", n) == 0) {
        gdbprintf("Text=0;Data=0;Bss=0");
      } else SendGDB(0);
      break;
    }
    case 'g': {
      DWire_CacheAllRegs(&hdw);
      buf2hex(hdw.regs, cbuffer, 32);
      uint8_t extraregs[8];
      DWire_ReadAddr(&hdw, 0x5d, extraregs + 1, 3); // read SPL SPH SREG
      extraregs[0] = extraregs[3]; // SREG at index 0
      extraregs[3] = hdw.pc & 0xff; // then PC little endian
      extraregs[4] = (hdw.pc >> 8) & 0xff; 
      extraregs[5] = extraregs[6] = 0;
      buf2hex(extraregs, cbuffer + 64, 7);
      SendGDB(78);
    }
    case 'G': {
      // possibly not needed, registers set using 'P'
      SendGDBError(1);
      break;
    }
    case '?': {
      SendGDBStatus(SIGTRAP);
      break;
    }
    case 'p': {
      // possibly not needed, registers read all at once using 'g'
      SendGDBError(1);
      break;
    }
    case 'P': {
      int reg, pos;
      if (sscanf(cbuffer, "P%x=%n", &reg, &pos) < 1) {
        SendGDBError(1);
        break;
      }
      if (reg >= 0 && reg < 32) {
        DWire_CacheAllRegs(&hdw);
        hex2buf(hdw.regs + reg, cbuffer + pos, 1);
        SendGDBOK();
      } else if (reg == 32) {
        uint8_t val;
        hex2buf(&val, cbuffer + pos, 1);
        DWire_WriteAddr(&hdw, 0x5f, &val, 1);
        SendGDBOK();
      } else if (reg == 33) {
        uint8_t spbuf[2];
        hex2buf(spbuf, cbuffer + pos, 2);
        DWire_WriteAddr(&hdw, 0x5d, spbuf, 2);
        SendGDBOK();
      } else if (reg == 35) {
        uint8_t pcbuf[4];
        hex2buf(pcbuf, cbuffer + pos, 4);
        hdw.pc = pcbuf[0] | (pcbuf[1] << 8);
        SendGDBOK();
      } else SendGDBError(1);
      break;
    }
    case 'k': {
      DWire_Continue(&hdw);
      break;
    }
    case 'c': {
      int new_pc;
      if (sscanf(cbuffer, "c%x", &new_pc) > 0)
        hdw.pc = new_pc;
      DWire_Continue(&hdw);
      reply_on_break = true;
      break;
    }
    case 's': {
      int new_pc;
      if (sscanf(cbuffer, "c%x", &new_pc) > 0)
        hdw.pc = new_pc;
      DWire_Step(&hdw);
      SendGDBStatus(SIGTRAP);
      break;
    }
    case 'D':
    case 'H':
    case 'r':
    case 'R':
    case 'z': {
      int type, addr, kind;
      if (sscanf(cbuffer, "z%d,%x,%x", &type, &addr, &kind) < 3) {
        SendGDBError(1);
        break;
      }
      switch (type) {
        case 1: {
          hdw.breakpoint = -1;
          break;
        }
        default:
          SendGDB(0);
          break;
      }
    }
    case 'Z': {
      int type, addr, kind;
      if (sscanf(cbuffer, "z%d,%x,%x", &type, &addr, &kind) < 3) {
        SendGDBError(1);
        break;
      }
      switch (type) {
        case 1: {
          hdw.breakpoint = addr;
          break;
        }
        default:
          SendGDB(0);
          break;
      }
    }
    default:
      SendGDB(0);
      break;
  }
}

void HandleBreak()
{
  if (reply_on_break) {
    SendGDBStatus(SIGTRAP);
    reply_on_break = false;
  }
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
    } else if (ch == 0x03) {
      DWire_Break(&hdw);
      HandleBreak();
    }
  } else if (UARTHelper_PollRecv(&huarth3)) {
    // unexpected data on dwire
    // throw it away
    uint8_t buf;
    while (UARTHelper_TryRecv(&huarth3, &buf, 1));
  } else if (UARTHelper_PollBreak(&huarth3)) {
    // break received from MCU
    hdw.stopped = true;
    DWire_Reconnect(&hdw);
    HandleBreak();
  } else {
    osDelay(1);
  }
}

void DebugInit(UARTHelper_HandleTypeDef *huarth)
{
  DWire_Init(&hdw, huarth);
}

