/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#include <stdbool.h>

#define INFINITE true
#define IMMEDIATE false

void *CreateQueue(unsigned int count, size_t element_size);
void DeleteQueue(void *q);
void *GetMessage(void *q, bool block);
void SendMessage(void *q, void * m);
void *ReceiveMessage(void *q, bool block);
void ReleaseMessage(void *q, void *m);

#endif /* __MESSAGE_QUEUE_H__ */
