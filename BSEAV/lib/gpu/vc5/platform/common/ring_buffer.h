/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <pthread.h>
#include <stdbool.h>

/*
 * A simple, thread-safe ring buffer.
 */

struct ring_buffer
{
   pthread_mutex_t mutex;
   pthread_cond_t cond;
   size_t count;
   size_t elsize;
   size_t read;
   size_t write;
   bool full;
   bool poisoned;
   void *buffer;
};

bool ring_buffer_init(struct ring_buffer *rb, size_t count, size_t elsize);
void ring_buffer_destroy(struct ring_buffer *rb);
bool ring_buffer_write(struct ring_buffer *rb, const void *element, bool block);
bool ring_buffer_read(struct ring_buffer *rb, void *element, bool block);

/* convenience macro to kill the ring_buffer */
#define ring_buffer_poison(rb) ring_buffer_write((rb), NULL, false)

#endif /* __RING_BUFFER_H__ */
