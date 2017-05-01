/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "queue.h"
#include <assert.h>

bool queue_init(struct queue *q)
{
   bool done = false;
   if (pthread_mutex_init(&q->mutex, NULL) == 0)
   {
      if (pthread_cond_init(&q->cond, NULL) == 0)
      {
         list_init(&q->items);
         q->poisoned = false;
         done = true;
      }
      else
      {
         pthread_mutex_destroy(&q->mutex);
      }
   }
   return done;
}

void queue_destroy(struct queue *q)
{
   if (q)
   {
      assert(list_empty(&q->items));

      pthread_cond_destroy(&q->cond);
      pthread_mutex_destroy(&q->mutex);
   }
}

void queue_enqueue(struct queue *q, struct list *item)
{
   assert(item != NULL);
   pthread_mutex_lock(&q->mutex);
   assert(!q->poisoned);
   list_push_back(&q->items, item);
   pthread_cond_broadcast(&q->cond);
   pthread_mutex_unlock(&q->mutex);
}

void queue_poison(struct queue *q)
{
   pthread_mutex_lock(&q->mutex);
   q->poisoned = true; /* poison the queue - queue will never block now */
   pthread_cond_broadcast(&q->cond);
   pthread_mutex_unlock(&q->mutex);
}


struct list *queue_try_dequeue(struct queue *q)
{
   pthread_mutex_lock(&q->mutex);
   struct list *item = list_pop_front(&q->items); /* NULL if queue is empty */
   pthread_mutex_unlock(&q->mutex);
   return item;
}

struct list *queue_dequeue(struct queue *q)
{
   struct list *item;
   pthread_mutex_lock(&q->mutex);

   /* wait for an item as long as this queue is alive */
   while (list_empty(&q->items) && !q->poisoned
         && pthread_cond_wait(&q->cond, &q->mutex) == 0)
   {
   }

   item = list_pop_front(&q->items); /* NULL only if queue was poisoned */

   pthread_mutex_unlock(&q->mutex);
   return item;
}
