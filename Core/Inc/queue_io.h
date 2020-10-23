#ifndef __QUEUE_IO_H
#define __QUEUE_IO_H

#include <stdbool.h>

size_t Queue_Send(osMessageQueueId_t queue, uint8_t *buf, size_t len);
size_t Queue_TrySend(osMessageQueueId_t queue, uint8_t *buf, size_t len);
size_t Queue_TrySend1(osMessageQueueId_t queue, uint8_t *buf, size_t len);
size_t Queue_Recv(osMessageQueueId_t queue, uint8_t *buf, size_t len);
size_t Queue_TryRecv(osMessageQueueId_t queue, uint8_t *buf, size_t len);
size_t Queue_TryRecv1(osMessageQueueId_t queue, uint8_t *buf, size_t len);
bool Queue_PollRecv(osMessageQueueId_t queue);
void Queue_SendChar(osMessageQueueId_t queue, int c);
int Queue_RecvChar(osMessageQueueId_t queue);

#endif

