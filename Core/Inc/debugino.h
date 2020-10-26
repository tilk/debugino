#ifndef __DEBUGINO_H
#define __DEBUGINO_H

#include "uart_helper.h"

void DebugInit(UARTHelper_HandleTypeDef *huarth);
void DebugLoop();

#endif
