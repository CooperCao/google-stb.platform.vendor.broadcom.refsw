/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "list.h"
#include <pthread.h>
#include <semaphore.h>

/*
 * A simple, thread-safe queue.
 *
 * The queue uses list as it's internal storage. There are 2 operations
 * defined: enqueue and dequeue. Enqueue adds an item to the back of the list.
 * Dequeue removes an item from the front list.
 *
 * There's no size limit therefore the enqueue operation never blocks.
 * The dequeue can either be a blocking or a non-blocking call.
 *
 * A blocked dequeue operation can be interrupted by poisoning the queue.
 * Poisoning can be used to unblock a thread waiting on a queue and let it
 * finish before the queue is destroyed.
 */

struct queue
{
   pthread_mutex_t mutex;
   pthread_cond_t cond;
   bool poisoned;
   struct list items;
};

bool queue_init(struct queue *q);
void queue_destroy(struct queue *q);
void queue_enqueue(struct queue *q, struct list *item); /* non-blocking */
struct list *queue_try_dequeue(struct queue *q); /* non-blocking */
struct list *queue_dequeue(struct queue *q); /* blocking */
void queue_poison(struct queue *q);

#endif /* __QUEUE_H__ */
