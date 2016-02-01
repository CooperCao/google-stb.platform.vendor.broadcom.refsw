/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>
#include <stdint.h>
#include <errno.h>

#include "message_queue.h"

typedef struct list_s
{
   struct list_s *nextp;
   unsigned char data[0];
} list_t;

typedef struct mq_s
{
   /* semaphores to control signalling */
   sem_t rec_sem;
   sem_t send_sem;

   /* critical section to control access to the lists, just share, although you could use two */
   pthread_mutex_t cs;

   /* the two lists */
   list_t *rec_q;
   list_t *send_q;
} mq_t;

static void addtime(struct timespec *ts, unsigned int t)
{
   // round up
   // convert to milliseconds
   int64_t mseconds = (ts->tv_sec * 1000LL) + (ts->tv_nsec / 1000000LL) + ((ts->tv_nsec % 1000000LL) != 0);
   // add timeout
   mseconds += t;
   // convert to timespec
   int64_t remainder = mseconds % 1000LL;
   ts->tv_sec = (time_t)mseconds / 1000LL;
   ts->tv_nsec = (long)remainder * 1000000LL;
}

void *CreateQueue(unsigned int count, size_t element_size)
{
   /* allocate enough contiguous space for everything */
   mq_t *q = (mq_t *)malloc(sizeof(mq_t) + (count * (sizeof(list_t) + element_size)));

   if (q)
   {
      list_t *p = (list_t *)((uintptr_t)q + sizeof(mq_t));

      /* create the semaphores */
      sem_init(&q->send_sem, 0, count);
      sem_init(&q->rec_sem, 0, 0);

      /* initialize the critical section */
      pthread_mutex_init(&(q->cs), NULL);

      /* one NULL queue and one with all the mesages in */
      q->send_q = p;
      q->rec_q = NULL;

      /* populate the queue, last one is NULL terminated */
      for (unsigned int i = 0; i < (count - 1); i++, p = p->nextp)
         p->nextp = (list_t *)((uintptr_t)p + sizeof(list_t) + element_size);
      /* terminate the list */
      p->nextp = NULL;
   }

   /* will be NULL if failure */
   return q;
}

void DeleteQueue(void *q)
{
   mq_t *mq = (mq_t *)q;

   if (mq)
   {
      sem_destroy(&mq->rec_sem);
      sem_destroy(&mq->send_sem);

      pthread_mutex_destroy(&(mq->cs));

      free(mq);
   }
}

void *GetMessage(void *q, unsigned int t)
{
   mq_t *mq = (mq_t *)q;
   void *m = NULL;

   if (mq)
   {
      int s;
      struct timespec ts;

      if (t == IMMEDIATE)
      {
         /* immediate */
         s = sem_trywait(&(mq->send_sem));
      }
      else if (t != INFINITE)
      {
         clock_gettime(CLOCK_REALTIME, &ts);
         addtime(&ts, t);

         while ((s = sem_timedwait(&(mq->send_sem), &ts)) == -1 && errno == EINTR)
            continue;       /* Restart if interrupted by handler */
      }
      else
         s = sem_wait(&(mq->send_sem));

      /* this will pull a semaphore from the list, if non available then it will block */
      if (!s)
      {
         pthread_mutex_lock(&(mq->cs));

         /* we always guarentee a valid pointer back from here */
         m = mq->send_q->data;
         /* advance to next free item */
         mq->send_q = mq->send_q->nextp;

         pthread_mutex_unlock(&(mq->cs));
      }
   }

   return m;
}

void SendMessage(void *q, void *m)
{
   mq_t *mq = (mq_t *)q;

   if (mq)
   {
      pthread_mutex_lock(&(mq->cs));

      list_t *l = mq->rec_q;
      list_t *p = (list_t *)((uintptr_t)m - sizeof(list_t));

      /* configure the new block to point to the current head */
      p->nextp = NULL;
      /* place the block on the back of the receive queue */
      if (l)
      {
         while (l->nextp != NULL)
            l = l->nextp;
         l->nextp = p;
      }
      else
         mq->rec_q = p;

      pthread_mutex_unlock(&(mq->cs));

      sem_post(&(mq->rec_sem));
   }
}

void *ReceiveMessage(void *q, unsigned int t)
{
   mq_t *mq = (mq_t *)q;
   void *m = NULL;

   if (q)
   {
      int s;
      struct timespec ts;

      if (t == IMMEDIATE)
      {
         /* immediate */
         s = sem_trywait(&(mq->rec_sem));
      }
      else if (t != INFINITE)
      {
         clock_gettime(CLOCK_REALTIME, &ts);
         addtime(&ts, t);

         while ((s = sem_timedwait(&(mq->rec_sem), &ts)) == -1 && errno == EINTR)
            continue;       /* Restart if interrupted by handler */
      }
      else
         s = sem_wait(&(mq->rec_sem));

      /* this will pull a semaphore from the list, if non available then it will block */
      if (!s)
      {
         pthread_mutex_lock(&(mq->cs));

         /* we always guarentee a valid pointer back from here */
         m = mq->rec_q->data;
         /* move on to the next free block */
         mq->rec_q = mq->rec_q->nextp;

         pthread_mutex_unlock(&(mq->cs));
      }
   }

   return m;
}

void ReleaseMessage(void *q, void *m)
{
   mq_t *mq = (mq_t *)q;

   if (mq)
   {
      pthread_mutex_lock(&(mq->cs));

      list_t *l = mq->send_q;
      list_t *p = (list_t *)((uintptr_t)m - sizeof(list_t));

      p->nextp = NULL;
      if (l)
      {
         while (l->nextp != NULL)
            l = l->nextp;
         l->nextp = p;
      }
      else
         mq->send_q = p;

      pthread_mutex_unlock(&(mq->cs));

      sem_post(&(mq->send_sem));
   }
}
