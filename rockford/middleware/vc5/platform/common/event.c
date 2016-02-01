/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include <pthread.h>
#include <stdbool.h>
#include <malloc.h>

typedef struct
{
   pthread_mutex_t      mutex;
   pthread_cond_t       cond;
   bool                 signal;
} Event;

void *CreateEvent(void)
{
   void *p = malloc(sizeof(Event));
   if (p)
   {
      Event *ev = (Event *)p;
      pthread_mutex_init(&ev->mutex, NULL);
      pthread_cond_init(&ev->cond, NULL);
      ev->signal = false;
   }
   return p;
}

bool DestroyEvent(void *p)
{
   if (p)
   {
      Event *ev = (Event *)p;
      pthread_cond_destroy(&ev->cond);
      pthread_mutex_destroy(&ev->mutex);
      free(p);
      return true;
   }

   return false;
}

bool SetEvent(void *p)
{
   if (p)
   {
      Event *ev = (Event *)p;
      pthread_mutex_lock(&ev->mutex);
      ev->signal = true;
      pthread_mutex_unlock(&ev->mutex);
      pthread_cond_signal(&ev->cond);
      return true;
   }

   return false;
}

bool WaitEvent(void *p)
{
   if (p)
   {
      Event *ev = (Event *)p;
      pthread_mutex_lock(&ev->mutex);
      while (!ev->signal)
         pthread_cond_wait(&ev->cond, &ev->mutex);
      ev->signal = false;
      pthread_mutex_unlock(&ev->mutex);
      return true;
   }

   return false;
}

bool ResetEvent(void *p)
{
   if (p)
   {
      Event *ev = (Event *)p;
      pthread_mutex_lock(&ev->mutex);
      ev->signal = false;
      pthread_mutex_unlock(&ev->mutex);
      return true;
   }

   return false;
}