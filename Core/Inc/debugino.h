#ifndef __DEBUGINO_H
#define __DEBUGINO_H

#include "uart_helper.h"
#include <stdbool.h>

#define SWD_FLASH_SPACE_OFFSET 0x000000
#define SWD_DATA_SPACE_OFFSET 0x800000
#define SWD_EEPROM_SPACE_OFFSET 0x810000
#define SWD_FUSE_SPACE_OFFSET 0x820000
#define SWD_LOCK_SPACE_OFFSET 0x830000
#define SWD_SIG_SPACE_OFFSET 0x840000
#define SWD_REG_SPACE_OFFSET 0x850000
#define SWD_BP_SPACE_OFFSET 0x900000
#define SWD_ADDR_SPACE_MASK 0xFF0000

void DebugInit(UARTHelper_HandleTypeDef *huarth);
void DebugLoop();
bool DebugIsStopped();

#endif
