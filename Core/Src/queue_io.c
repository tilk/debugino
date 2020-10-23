
#include <stdio.h>
#include <sys/stat.h>
#include "cmsis_os.h"
#include "queue_io.h"

bool Queue_PollRecv(osMessageQueueId_t queue)
{
  return osMessageQueueGetCount(queue) > 0;
}

size_t Queue_Send(osMessageQueueId_t queue, uint8_t *buf, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    osStatus_t status = osMessageQueuePut(queue, &buf[i], 0, osWaitForever);
    if (status != osOK) return i;
  }
  return len;
}

size_t Queue_TrySend(osMessageQueueId_t queue, uint8_t *buf, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    osStatus_t status = osMessageQueuePut(queue, &buf[i], 0, 0);
    if (status != osOK) return i;
  }
  return len;
}

size_t Queue_TrySend1(osMessageQueueId_t queue, uint8_t *buf, size_t len)
{
  size_t i = 0;
  osStatus_t status = osMessageQueuePut(queue, &buf[i], 0, osWaitForever);
  while (status == osOK) {
    i++;
    if (i >= len) break;
    status = osMessageQueuePut(queue, &buf[i], 0, 0);
  }
  return i;
}

size_t Queue_Recv(osMessageQueueId_t queue, uint8_t *buf, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    osStatus_t status = osMessageQueueGet(queue, &buf[i], NULL, osWaitForever);
    if (status != osOK) return i;
  }
  return len;
}

size_t Queue_TryRecv(osMessageQueueId_t queue, uint8_t *buf, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    osStatus_t status = osMessageQueueGet(queue, &buf[i], NULL, 0);
    if (status != osOK) return i;
  }
  return len;

}

size_t Queue_TryRecv1(osMessageQueueId_t queue, uint8_t *buf, size_t len)
{
  size_t i = 0;
  osStatus_t status = osMessageQueueGet(queue, &buf[i], NULL, osWaitForever);
  while (status == osOK) {
    i++;
    if (i >= len) break;
    status = osMessageQueueGet(queue, &buf[i], NULL, 0);
  }
  return i;
}

void Queue_SendChar(osMessageQueueId_t queue, int c)
{
  char buf = c;
  Queue_Send(queue, &buf, 1);
}

int Queue_RecvChar(osMessageQueueId_t queue)
{
  char buf;
  size_t s = Queue_Recv(queue, &buf, 1);
  if (s == 1) return buf;
  else return -1;
}


