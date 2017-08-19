/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "vcos_types.h"
#include "../demand.h"

#include <stddef.h>

EXTERN_C_BEGIN

/* These functions return indices into an external buffer which actually stores
 * the FIFO data. Therefore they operate on elements, not bytes. Note that the
 * size passed into these functions is the size of the external buffer, in
 * elements.
 *
 * To simplify the implementation there will always be a single element unused,
 * which means gfx_fifo_size will never return size.
 *
 * Things go on the FIFO at the tail and come off from the head.
 */

/* Returns UINT32_MAX when the FIFO is full, index otherwise */
static inline uint32_t gfx_fifo_try_push(uint32_t head, uint32_t* tail, uint32_t size)
{
   if(((*tail + 1) % size) == head)
   {
      return UINT32_MAX;
   }
   else
   {
      uint32_t idx = *tail;
      *tail = (*tail + 1) % size;
      return idx;
   }
}

static inline uint32_t gfx_fifo_push(uint32_t head, uint32_t* tail, uint32_t size)
{
   uint32_t idx = gfx_fifo_try_push(head, tail, size);
   demand(idx != UINT32_MAX);
   return idx;
}

/* Returns UINT32_MAX when the FIFO is empty, index otherwise */
static inline uint32_t gfx_fifo_try_pop(uint32_t* head, uint32_t tail, uint32_t size)
{
   if(*head == tail)
   {
      return UINT32_MAX;
   }
   else
   {
      uint32_t idx = *head;
      *head = (*head + 1) % size;
      return idx;
   }
}

static inline uint32_t gfx_fifo_pop(uint32_t* head, uint32_t tail, uint32_t size)
{
   uint32_t idx = gfx_fifo_try_pop(head, tail, size);
   demand(idx != UINT32_MAX);
   return idx;
}

/* Returns UINT32_MAX when the FIFO is empty, index of next pop otherwise */
static inline uint32_t gfx_fifo_peek(uint32_t head, uint32_t tail, uint32_t size)
{
   if(head == tail)
   {
      return UINT32_MAX;
   }
   else
   {
      return head;
   }
}

/* Returns UINT32_MAX when the FIFO is empty, index of last push otherwise */
static inline uint32_t gfx_fifo_peek_tail(uint32_t head, uint32_t tail, uint32_t size)
{
   if(head == tail)
   {
      return UINT32_MAX;
   }
   else
   {
      return tail == 0? size - 1 : tail - 1;
   }
}

/* The number of times pop will succeed with no intervening push [0, size) */
static inline uint32_t gfx_fifo_size(uint32_t head, uint32_t tail, uint32_t size)
{
   if(tail < head)
   {
      return size - (head - tail);
   }
   else
   {
      return tail - head;
   }
}

/* The number of times push will succeed with no intervening pop [0, size) */
static inline uint32_t gfx_fifo_space_left(uint32_t head, uint32_t tail, uint32_t size)
{
   return (size - 1) - gfx_fifo_size(head, tail, size);
}

EXTERN_C_END
