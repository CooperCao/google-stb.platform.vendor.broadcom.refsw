/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__

#define INFINITE (~0u)
#define IMMEDIATE (0)

void *CreateQueue(unsigned int count, size_t element_size);
void DeleteQueue(void *q);
void *GetMessage(void *q, unsigned int t);
void SendMessage(void *q, void * m);
void *ReceiveMessage(void *q, unsigned int t);
void ReleaseMessage(void *q, void *m);

#endif /* __MESSAGE_QUEUE_H__ */