
#include "dwire.h"

#define DWIRE_DEV_SIG 0x950f
#define DWIRE_DEV_IO_SIZE 224
#define DWIRE_DEV_SRAM_SIZE 2048
#define DWIRE_DEV_EEPROM_SIZE 1024
#define DWIRE_DEV_FLASH_SIZE 32768
#define DWIRE_DEV_DWDR 0x31
#define DWIRE_DEV_SPMCSR 0x37
#define DWIRE_DEV_PAGESIZE 128
#define DWIRE_DEV_BOOT 0x0000
#define DWIRE_DEV_BOOTFLAGS 0
#define DWIRE_DEV_EECR 0x1f
#define DWIRE_DEV_EEARH 0x22

#define BYTES(...) (uint8_t[]){__VA_ARGS__}, sizeof((uint8_t[]){__VA_ARGS__})
#define ADDR(a) ((a) >> 8) & 0xff, (a) & 0xff
#define WORD(a) ((a) >> 8) & 0xff, (a) & 0xff
#define WORD_LE(a) (a) & 0xff, ((a) >> 8) & 0xff

#define CACHED_REG(reg) ((reg) >= 28 && (reg) <= 31)

#define DWIRE_DISABLE 0x06
#define DWIRE_RESET 0x07
#define DWIRE_GO 0x20
#define DWIRE_SS 0x21
#define DWIRE_SS_INST 0x23
#define DWIRE_RESUME 0x30
#define DWIRE_RESUME_SS 0x31
#define DWIRE_FLAG_RUN 0x60
#define DWIRE_FLAG_RUN_TO_CURSOR 0x61
#define DWIRE_FLAG_STEP_OUT 0x63
#define DWIRE_FLAG_STEP_IN 0x79
#define DWIRE_FLAG_SINGLE_STEP 0x7a
#define DWIRE_FLAG_MEMORY 0x66
#define DWIRE_FLAG_INST 0x64
#define DWIRE_FLAG_FLASH_INST 0x44
#define DWIRE_BAUD_128 0x83
#define DWIRE_BAUD_64 0x82
#define DWIRE_BAUD_32 0x81
#define DWIRE_BAUD_16 0x80
#define DWIRE_SET_PC_LOW 0xc0
#define DWIRE_SET_PC 0xd0
#define DWIRE_SET_BP 0xd1
#define DWIRE_SET_BP_LOW 0xc1
#define DWIRE_SET_IR 0xd2
#define DWIRE_GET_PC 0xf0
#define DWIRE_GET_BP 0xf1
#define DWIRE_GET_IR 0xf2
#define DWIRE_GET_SIG 0xf3
#define DWIRE_RW_MODE 0xc2
#define DWIRE_MODE_READ_SRAM 0x00
#define DWIRE_MODE_READ_REGS 0x01
#define DWIRE_MODE_READ_FLASH 0x02
#define DWIRE_MODE_WRITE_SRAM 0x04
#define DWIRE_MODE_WRITE_REGS 0x05

void DWire_Send(DWire_HandleTypeDef *dwire, uint8_t *buf, size_t len)
{
  size_t i = 0;
  while (i < len) {
    size_t cnt = UARTHelper_TrySend1(dwire->huarth, &buf[i], len - i);
    for (size_t j = 0; j < cnt; j++) {
        uint8_t byte;
        uint8_t ret = UARTHelper_Recv(dwire->huarth, &byte, 1);
        if (ret != 1 || byte != buf[i+j]) longjmp(dwire->env, 1);
    }
    i += cnt;
  }
}

void DWire_Receive(DWire_HandleTypeDef *dwire, uint8_t *buf, size_t len)
{
  size_t ret = UARTHelper_Recv(dwire->huarth, buf, len);
  if (ret != len) longjmp(dwire->env, 1);
}

uint8_t DWire_ReceiveByte(DWire_HandleTypeDef *dwire)
{
  uint8_t byte;
  DWire_Receive(dwire, &byte, 1);
  return byte;
}

uint16_t DWire_ReceiveWord(DWire_HandleTypeDef *dwire)
{
  uint8_t bytes[2];
  DWire_Receive(dwire, bytes, 2);
  return (bytes[0] << 8) | bytes[1];
}

void DWire_SendByte(DWire_HandleTypeDef *dwire, uint8_t byte)
{
  DWire_Send(dwire, BYTES(byte));
}

void DWire_SetPC(DWire_HandleTypeDef *dwire, uint16_t pc)
{
  DWire_Send(dwire, BYTES(DWIRE_SET_PC, ADDR(pc)));
}

void DWire_SetBP(DWire_HandleTypeDef *dwire, uint16_t bp)
{
  DWire_Send(dwire, BYTES(DWIRE_SET_BP, ADDR(bp)));
}

void DWire_PreInst(DWire_HandleTypeDef *dwire)
{
  DWire_Send(dwire, BYTES(DWIRE_FLAG_INST));
}

void DWire_PreFlashInst(DWire_HandleTypeDef *dwire)
{
  DWire_Send(dwire, BYTES(DWIRE_FLAG_FLASH_INST));
}

void DWire_Inst(DWire_HandleTypeDef *dwire, uint16_t inst)
{
  DWire_Send(dwire, BYTES(DWIRE_SET_IR, WORD(inst), DWIRE_SS_INST));
}

void DWire_In(DWire_HandleTypeDef *dwire, uint8_t reg, uint16_t ioreg)
{
  DWire_Inst(dwire, 0xb000 | ((ioreg << 5) & 0x600) | ((reg << 4) & 0x01F0) | (ioreg & 0x000F));
}

void DWire_Out(DWire_HandleTypeDef *dwire, uint16_t ioreg, uint8_t reg)
{
  DWire_Inst(dwire, 0xb800 | ((ioreg << 5) & 0x600) | ((reg << 4) & 0x01F0) | (ioreg & 0x000F));
}

void DWire_In_DWDR(DWire_HandleTypeDef *dwire, uint8_t reg)
{
  DWire_In(dwire, reg, DWIRE_DEV_DWDR);
}

void DWire_Out_DWDR(DWire_HandleTypeDef *dwire, uint8_t reg)
{
  DWire_Out(dwire, DWIRE_DEV_DWDR, reg);
}

void DWire_Out_SPMCSR(DWire_HandleTypeDef *dwire, uint8_t reg)
{
  DWire_Out(dwire, DWIRE_DEV_SPMCSR, reg);
}

void DWire_LPM(DWire_HandleTypeDef *dwire, uint8_t reg)
{
  DWire_Inst(dwire, 0x9004 | (reg << 4));
}

void DWire_SPM(DWire_HandleTypeDef *dwire)
{
  DWire_Inst(dwire, 0x95e8);
}

void DWire_SPM_Z(DWire_HandleTypeDef *dwire)
{
  DWire_Inst(dwire, 0x95f8);
}

void DWire_LDI(DWire_HandleTypeDef *dwire, uint8_t reg, uint8_t val)
{
  DWire_Inst(dwire, 0xe000 | ((reg << 4) & 0xf0) | (val & 0xf) | ((val << 4) & 0xf00));
}

uint8_t DWire_GetReg(DWire_HandleTypeDef *dwire, size_t reg)
{
  DWire_PreInst(dwire);
  DWire_Out_DWDR(dwire, reg);
  return DWire_ReceiveByte(dwire);
}

void DWire_GetRegs(DWire_HandleTypeDef *dwire, size_t first, uint8_t *buf, size_t count)
{
  DWire_SetPC(dwire, first);
  DWire_SetBP(dwire, first + count);
  DWire_Send(dwire, BYTES(DWIRE_FLAG_MEMORY, DWIRE_RW_MODE, DWIRE_MODE_READ_REGS, DWIRE_GO));
  DWire_Receive(dwire, buf, count);
}

void DWire_SetReg(DWire_HandleTypeDef *dwire, size_t reg, uint8_t val)
{
  DWire_PreInst(dwire);
  DWire_In_DWDR(dwire, reg);
  DWire_SendByte(dwire, val);
}

void DWire_SetRegs(DWire_HandleTypeDef *dwire, size_t first, uint8_t *buf, size_t count)
{
  DWire_SetPC(dwire, first);
  DWire_SetBP(dwire, first + count);
  DWire_Send(dwire, BYTES(DWIRE_FLAG_MEMORY, DWIRE_RW_MODE, DWIRE_MODE_WRITE_REGS, DWIRE_GO));
  DWire_Send(dwire, buf, count);
}

void DWire_SetZ(DWire_HandleTypeDef *dwire, uint16_t addr)
{
  DWire_SetRegs(dwire, 30, BYTES(WORD_LE(addr)));
}

void DWire_CacheRegs(DWire_HandleTypeDef *dwire)
{
  dwire->pc = (DWire_ReceiveWord(dwire) * 2 - 1) % DWIRE_DEV_FLASH_SIZE;
  DWire_GetRegs(dwire, 28, &dwire->regs[28], 4);
}

void DWire_FlushCacheRegs(DWire_HandleTypeDef *dwire)
{
  DWire_SetRegs(dwire, 28, &dwire->regs[28], 4);
  DWire_SetPC(dwire, dwire->pc/2);
}

void DWire_ReadAddr(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count)
{
  DWire_SetZ(dwire, addr);
  DWire_SetBP(dwire, 2);
  DWire_Send(dwire, BYTES(DWIRE_FLAG_MEMORY, DWIRE_RW_MODE, DWIRE_MODE_READ_SRAM));
  for (int i = 0; i < count; i++) {
    if (CACHED_REG(addr) || addr == DWIRE_DEV_DWDR) {
      DWire_SetPC(dwire, 0);
      DWire_Send(dwire, BYTES(DWIRE_GO));
      buf[i] = DWire_ReceiveByte(dwire);
    } else {
      if (CACHED_REG(addr)) buf[i] = dwire->regs[addr];
      DWire_SetZ(dwire, addr + i + 1);
    }
  }
}

void DWire_WriteAddr(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count)
{
  DWire_SetZ(dwire, addr);
  DWire_SetBP(dwire, 3);
  DWire_Send(dwire, BYTES(DWIRE_FLAG_MEMORY, DWIRE_RW_MODE, DWIRE_MODE_WRITE_SRAM));
  for (int i = 0; i < count; i++) {
    if (CACHED_REG(addr) || addr == DWIRE_DEV_DWDR) {
      DWire_SetPC(dwire, 1);
      DWire_Send(dwire, BYTES(DWIRE_GO, buf[i]));
    } else {
      if (CACHED_REG(addr)) dwire->regs[addr] = buf[i];
      DWire_SetZ(dwire, addr + i + 1);
    }
  }
}

void DWire_Sync(DWire_HandleTypeDef *dwire)
{
  while(1) {
    uint8_t byte = DWire_ReceiveByte(dwire);
    if (byte == 0x55) break;
    if (byte != 0x00) longjmp(dwire->env, 1);
  }
  if (!UARTHelper_PollBreak(dwire->huarth)) longjmp(dwire->env, 1);
}

void DWire_BreakSync(DWire_HandleTypeDef *dwire)
{
  UARTHelper_PollBreak(dwire->huarth);
  UARTHelper_SendBreak(dwire->huarth);
  DWire_Sync(dwire);
}

void DWire_Reconnect(DWire_HandleTypeDef *dwire)
{
  DWire_Send(dwire, BYTES(DWIRE_GET_PC));
  DWire_CacheRegs(dwire);
}

void DWire_Reset(DWire_HandleTypeDef *dwire)
{
  DWire_Send(dwire, BYTES(DWIRE_RESET));
  DWire_Sync(dwire);
  DWire_Reconnect(dwire);
}

void DWire_Disable(DWire_HandleTypeDef *dwire)
{
  DWire_Send(dwire, BYTES(DWIRE_DISABLE));
}

void DWire_Break(DWire_HandleTypeDef *dwire)
{
  DWire_BreakSync(dwire);
  DWire_Reconnect(dwire);
}

void DWire_Start(DWire_HandleTypeDef *dwire)
{
  // clear the buffer
  while (1) {
    uint8_t buf;
    size_t res = UARTHelper_TryRecv1(dwire->huarth, &buf, 1);
    if (!res) break;
  }
  DWire_Break(dwire);
}

void DWire_Step(DWire_HandleTypeDef *dwire)
{
  DWire_FlushCacheRegs(dwire);
  DWire_Send(dwire, BYTES(DWIRE_FLAG_RUN, DWIRE_RESUME_SS));
  DWire_Sync(dwire);
  DWire_Reconnect(dwire);
}

void DWire_Continue(DWire_HandleTypeDef *dwire)
{
  DWire_FlushCacheRegs(dwire);
  if (dwire->breakpoint < 0) {
    DWire_Send(dwire, BYTES(DWIRE_FLAG_RUN));
  } else {
    DWire_SetBP(dwire, dwire->breakpoint / 2);
    DWire_Send(dwire, BYTES(DWIRE_FLAG_RUN_TO_CURSOR));
  }
  DWire_Send(dwire, BYTES(DWIRE_RESUME));
}

uint16_t DWire_Signature(DWire_HandleTypeDef *dwire)
{
  DWire_Send(dwire, BYTES(DWIRE_GET_SIG));
  return DWire_ReceiveWord(dwire);
}

void DWire_CheckSignature(DWire_HandleTypeDef *dwire)
{
  uint16_t sig = DWire_Signature(dwire);
  if (sig != DWIRE_DEV_SIG) longjmp(dwire->env, 2);
}

void DWire_ReadFlash(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count)
{
  DWire_SetZ(dwire, addr);
  DWire_SetBP(dwire, count * 2);
  DWire_SetPC(dwire, 0);
  DWire_Send(dwire, BYTES(DWIRE_FLAG_MEMORY, DWIRE_RW_MODE, DWIRE_MODE_READ_FLASH, DWIRE_GO));
  DWire_Receive(dwire, buf, count);
}

void DWire_WriteFlashPage(DWire_HandleTypeDef *dwire, uint16_t addr, uint8_t *buf, size_t count)
{
  DWire_SetZ(dwire, addr);
  DWire_SetPC(dwire, 0x3f00);
  DWire_PreInst(dwire);
//  DWire_Inst(dwire, 0x01cf);  // movw r24,r30
  DWire_LDI(dwire, 29, 0x03);
  DWire_Out_SPMCSR(dwire, 29);
  DWire_SPM(dwire);
  DWire_BreakSync(dwire);
  DWire_PreFlashInst(dwire);
  DWire_LDI(dwire, 29, 0x01);
  for (int i = 0; i < count; i += 2) {
    DWire_SetPC(dwire, 0x3f00);
    DWire_In_DWDR(dwire, 0);
    DWire_SendByte(dwire, buf[i]);
    DWire_In_DWDR(dwire, 1);
    DWire_SendByte(dwire, buf[i+1]);
    DWire_Out_SPMCSR(dwire, 29);
//    DWire_SPM_Z(dwire);
    DWire_SPM(dwire);
    DWire_Inst(dwire, 0x9632);
  }
  DWire_SetPC(dwire, 0x3f00);
  DWire_LDI(dwire, 31, (addr >> 8) & 0xff);
  DWire_LDI(dwire, 30, addr & 0xff);
  DWire_LDI(dwire, 29, 0x05);
  DWire_Out_SPMCSR(dwire, 29);
  DWire_SPM(dwire);
  osDelay(10); // TODO hack can we do without it?
  DWire_BreakSync(dwire);
  DWire_SetPC(dwire, 0x3f00);
  DWire_LDI(dwire, 29, 0x11);
  DWire_Out_SPMCSR(dwire, 29);
  DWire_SPM(dwire);
  DWire_BreakSync(dwire);
  // TODO: restore regs r0/r1
}

uint8_t DWire_ReadFuseBits(DWire_HandleTypeDef *dwire, uint16_t z)
{
  DWire_SetZ(dwire, z);
  //DWire_SetRegs(dwire, 29, BYTES(0x09, WORD_LE(z)));
  //DWire_SetPC(dwire, 0x3f00);
  DWire_PreInst(dwire);
  DWire_LDI(dwire, 29, 0x09);
  DWire_Out_SPMCSR(dwire, 29);
  DWire_LPM(dwire, 29);
  DWire_Out_DWDR(dwire, 29);
  return DWire_ReceiveByte(dwire);
}

uint8_t DWire_ReadFuseBitsLow(DWire_HandleTypeDef *dwire)
{
  return DWire_ReadFuseBits(dwire, 0);
}

uint8_t DWire_ReadFuseBitsHigh(DWire_HandleTypeDef *dwire)
{
  return DWire_ReadFuseBits(dwire, 3);
}

uint8_t DWire_ReadFuseBitsExt(DWire_HandleTypeDef *dwire)
{
  return DWire_ReadFuseBits(dwire, 2);
}

uint8_t DWire_ReadLockBits(DWire_HandleTypeDef *dwire)
{
  return DWire_ReadFuseBits(dwire, 1);
}


