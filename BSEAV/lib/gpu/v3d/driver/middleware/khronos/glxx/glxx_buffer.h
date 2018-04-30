/******************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************/
#pragma once

#include <GLES/gl.h>

#include "interface/khronos/common/khrn_int_common.h"
#include "vcfw/rtos/abstract/rtos_abstract_mem.h"
#include "middleware/khronos/common/khrn_interlock.h"

/*
   Invariant:

   Before performing _any_ operation on a GLXX_BUFFER_T, the underlying memory
   block will have had glxx_buffer_term() installed as its terminator



   We have a conceptual boolean variable conceptual_buffers_owned_by_master which
   represents our ability to modify buffer contents from within the master process.
   (It is not necessary in order to modify the buffer object itself or to alter the
   "shared" object by adding/removing buffers).

               glxx_hw_draw_triangles
                ------------------->
           true                     false
                <-------------------
                  khrn_fifo_finish
*/

/*
   To reduce flushes to the hardware, we keep a small pool of buffers.
   If an attempt is made to change the contents of the buffer while
   there is still a pending render from that buffer, one of the other
   members of the pool is used instead.
*/
typedef struct {
   /*
      storage associated with the buffer

      Invariants:

      mh_storage != MEM_HANDLE_INVALID
      mh_storage is MEM_ZERO_SIZE_HANDLE or we have the only reference to it
   */

   MEM_HANDLE_T mh_storage;


   KHRN_INTERLOCK_T interlock;

} GLXX_BUFFER_INNER_T;

#define GLXX_BUFFER_POOL_SIZE 16

typedef struct {
   /*
      name of the buffer

      Implementation Note:

      This is here solely to let us answer glGet<blah>() queries. Note that due
      to the shared context semantics it's not necessarily a no-op to get the name
      of the bound buffer and then call glBindBuffer() with that name

      Invariant:

      name != 0
   */

   uint32_t name;

   /*
      usage of the buffer

      Invariants:

      usage in {STATIC_DRAW, DYNAMIC_DRAW}
   */

   GLenum usage;

   int current_item;
   GLXX_BUFFER_INNER_T pool[GLXX_BUFFER_POOL_SIZE];

} GLXX_BUFFER_T;

extern void glxx_buffer_init(GLXX_BUFFER_T *buffer, uint32_t name);
extern void glxx_buffer_term(void *p);

extern bool glxx_buffer_data(GLXX_BUFFER_T *buffer, int32_t size, const void *data, GLenum usage, bool transient);
extern void glxx_buffer_subdata(GLXX_BUFFER_T *buffer, int32_t offset, int32_t size, const void *data);

extern int glxx_buffer_get_size(GLXX_BUFFER_T *buffer);

extern MEM_HANDLE_T glxx_buffer_get_storage_handle(GLXX_BUFFER_T *buffer);
extern uint32_t glxx_buffer_get_interlock_offset(GLXX_BUFFER_T *buffer);
