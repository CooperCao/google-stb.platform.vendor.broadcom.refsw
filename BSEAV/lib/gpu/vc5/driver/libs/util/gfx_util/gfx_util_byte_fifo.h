/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "vcos_types.h"
#include "gfx_util.h"
#include "gfx_util_fifo.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

VCOS_EXTERN_C_BEGIN

struct gfx_byte_fifo
{
   uint32_t head, tail, size;
   uint8_t* buffer;
};

static inline void gfx_byte_fifo_init(struct gfx_byte_fifo* fifo, uint8_t* buffer, uint32_t buffer_bytes)
{
   fifo->head = 0;
   fifo->tail = 0;
   fifo->buffer = buffer;
   fifo->size = buffer_bytes;
}

static inline bool gfx_byte_fifo_try_push(struct gfx_byte_fifo* fifo, uint8_t const* data, uint32_t data_bytes)
{
   if(gfx_fifo_space_left(fifo->head, fifo->tail, fifo->size) < data_bytes)
   {
      return false;
   }
   else
   {
      uint32_t const to_tail = gfx_umin(data_bytes, fifo->size - fifo->tail);
      memcpy(&fifo->buffer[fifo->tail], data, to_tail);
      memcpy(fifo->buffer, &data[to_tail], data_bytes - to_tail);

      fifo->tail = (fifo->tail + data_bytes) % fifo->size;

      return true;
   }
}

static inline uint32_t gfx_byte_fifo_peek(struct gfx_byte_fifo const* fifo, uint8_t* data, uint32_t data_bytes)
{
   uint32_t to_read = gfx_umin(data_bytes, gfx_fifo_size(fifo->head, fifo->tail, fifo->size));

   if(data)
   {
      uint32_t const from_head = gfx_umin(to_read, fifo->size - fifo->head);
      memcpy(data, &fifo->buffer[fifo->head], from_head);
      memcpy(&data[from_head], fifo->buffer, to_read - from_head);
   }

   return to_read;
}

static inline bool gfx_byte_fifo_try_pop(struct gfx_byte_fifo* fifo, uint8_t* data, uint32_t data_bytes)
{
   uint32_t read = gfx_byte_fifo_peek(fifo, data, data_bytes);
   if(read == data_bytes)
   {
      fifo->head = (fifo->head + read) % fifo->size;
      return true;
   }
   else
   {
      return false;
   }
}

static inline uint32_t gfx_byte_fifo_size(struct gfx_byte_fifo const* fifo)
{
   return gfx_fifo_size(fifo->head, fifo->tail, fifo->size);
}

static inline uint32_t gfx_byte_fifo_space_left(struct gfx_byte_fifo const* fifo)
{
   return gfx_fifo_space_left(fifo->head, fifo->tail, fifo->size);
}

VCOS_EXTERN_C_END
