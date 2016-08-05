/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __FENCE_QUEUE_H__
#define __FENCE_QUEUE_H__

#include "fence_interface.h"
#include "ring_buffer.h"
#include <stdbool.h>

struct fence_queue
{
   const struct FenceInterface *fence_interface;
   struct ring_buffer fences;
};

bool fence_queue_init(struct fence_queue *fq, size_t num_fences,
      const struct FenceInterface *fence_interface);
void fence_queue_destroy(struct fence_queue *fq);
void fence_queue_enqueue(struct fence_queue *fq, int fence);
bool fence_queue_dequeue(struct fence_queue *fq, int *fence, bool block);

#endif /* __FENCE_QUEUE_H__ */
