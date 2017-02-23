/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "ring_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

bool ring_buffer_init(struct ring_buffer *rb, size_t count, size_t elsize)
{
   rb->buffer = calloc(count, elsize);
   if (!rb->buffer)
      goto end;

   if (pthread_mutex_init(&rb->mutex, NULL) != 0)
      goto no_mutex;

   if (pthread_cond_init(&rb->cond, NULL) != 0)
      goto no_cond;

   rb->count = count;
   rb->elsize = elsize;
   rb->read = rb->write = 0;
   rb->full = rb->poisoned = false;
   goto end;

no_cond:
   pthread_mutex_destroy(&rb->mutex);
no_mutex:
   free(rb->buffer);
   rb->buffer = NULL;
end:
   return rb->buffer != NULL;
}

void ring_buffer_destroy(struct ring_buffer *rb)
{
   if (rb)
   {
      pthread_cond_destroy(&rb->cond);
      pthread_mutex_destroy(&rb->mutex);
      free(rb->buffer);
   }
}

static inline size_t next(size_t index, size_t count)
{
   return ++index < count ? index : 0;
}

static inline bool empty(const struct ring_buffer *rb)
{
   return rb->write == rb->read && !rb->full;
}

static inline bool full(const struct ring_buffer *rb)
{
   return rb->write == rb->read && rb->full;
}

static inline void *address(const struct ring_buffer *rb, size_t index)
{
   return ((uint8_t*)rb->buffer) + index * rb->elsize;
}

bool ring_buffer_write(struct ring_buffer *rb, const void *element, bool block)
{
   void *internal;
   pthread_mutex_lock(&rb->mutex);
   if (element)
   {
      if (block)
      {
         while(full(rb) && !rb->poisoned &&
               pthread_cond_wait(&rb->cond, &rb->mutex) == 0)
         {}
      }
      if (full(rb))
      {
         /* ignore the write request - there's nothing else we can do */
         internal = NULL;
      }
      else
      {
         internal = address(rb, rb->write);
         memcpy(internal, element, rb->elsize);
         rb->write = next(rb->write, rb->count);
         rb->full = (rb->write == rb->read);
      }
   }
   else
   {
      rb->poisoned = true; /* poison the ring_buffer - it will never block now */
      internal = NULL;
   }
   pthread_cond_broadcast(&rb->cond);
   pthread_mutex_unlock(&rb->mutex);
   return internal != NULL;
}

bool ring_buffer_read(struct ring_buffer *rb, void *element, bool block)
{
   void *internal;
   pthread_mutex_lock(&rb->mutex);
   if (block)
   {
      while(empty(rb) && !rb->poisoned &&
         pthread_cond_wait(&rb->cond, &rb->mutex) == 0)
      {}
   }
   if (empty(rb))
   {
      internal = NULL;
   }
   else
   {
      internal = address(rb, rb->read);
      memcpy(element, internal, rb->elsize);
      rb->read = next(rb->read, rb->count);
      rb->full = false;
   }
   pthread_cond_broadcast(&rb->cond);
   pthread_mutex_unlock(&rb->mutex);
   return internal != NULL;
}
