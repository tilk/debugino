
#include "debugino.h"
#include "main.h"
#include "queue_io.h"
#include <stdio.h>

void DebugLoop()
{
  if (Queue_PollRecv(queueUSBtoDEBUGHandle)) {
    // debugger command available
    Queue_SendChar(queueDEBUGtoUSBHandle, Queue_RecvChar(queueUSBtoDEBUGHandle));
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

