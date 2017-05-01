/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "fence_queue.h"
#include <stdlib.h>

bool fence_queue_init(struct fence_queue *fq, size_t num_fences,
      const FenceInterface *fence_interface)
{
   fq->fence_interface = fence_interface;
   return ring_buffer_init(&fq->fences, num_fences, sizeof(int));
}

void fence_queue_destroy(struct fence_queue *fq)
{
   int fence;

   ring_buffer_poison(&fq->fences);
   while (ring_buffer_read(&fq->fences, &fence, false))
      FenceInterface_Signal(fq->fence_interface, fence);
   ring_buffer_destroy(&fq->fences);
}

void fence_queue_enqueue(struct fence_queue *fq, int fence)
{
   ring_buffer_write(&fq->fences, &fence, true);
}

bool fence_queue_dequeue(struct fence_queue *fq, int *fence, bool block)
{
   return ring_buffer_read(&fq->fences, fence, block);
}
