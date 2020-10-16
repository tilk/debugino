
#include <stdio.h>
#include <sys/stat.h>
#include "cmsis_os.h"
#include "main.h"

int _read(int file, char *data, int len)
{
  if (len <= 0) return 0;
  osStatus_t status = osMessageQueueGet(queueUSBtoDEBUGHandle, data, NULL, osWaitForever);
  if (status != osOK) return -1;
  int i;
  for (i = 1; i < len; i++) {
    status = osMessageQueueGet(queueUSBtoDEBUGHandle, data + i, NULL, 0);
    if (status != osOK) break;
  }
  return i;
}

int _write(int file, char *data, int len)
{
  for (int i = 0; i < len; i++)
    osMessageQueuePut(queueDEBUGtoUSBHandle, data + i, 0, osWaitForever);
  return len;
}

int _close(int file)
{
    return -1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _fstat(int file, struct stat *st)
{
    return 0;
}

int _isatty(int file)
{
    return 1;
}

