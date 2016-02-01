/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Copy buffer

FILE DESCRIPTION
This is to support EGL's buffer preservation functionality. The idea is that we
can avoid doing the actual memcpy in most cases as we can just get the hardware
to load from the old buffer instead of the new one. The way this is implemented
is:
- Instead of just queueing a memcpy when we are asked to copy one buffer to
  another, we associate a KHRN_COPY_BUFFER_RENDER_STATE_T with the source and
  destination images (using the normal interlock/render state system). This will
  just sit around in the system until someone wants to do something with the
  source or destination.
- If VG or GL want to start rendering into the destination image, they notice
  the copy buffer render state and usurp it instead of flushing it.
- If the source or destination need to be modified or read for any other reason,
  the normal interlock/render state flush mechanism is used to actually perform
  the memcpy.
That was a bit long-winded, but hopefully it made sense.
=============================================================================*/

#ifndef KHRN_COPY_BUFFER_4_H
#define KHRN_COPY_BUFFER_4_H

#include "middleware/khronos/common/khrn_mem.h"

typedef struct {
   MEM_HANDLE_T dst, src;
} KHRN_COPY_BUFFER_RENDER_STATE_T;

extern bool khrn_delayed_copy_buffer(MEM_HANDLE_T dst_handle, MEM_HANDLE_T src_handle);
extern void khrn_do_copy_buffer(MEM_HANDLE_T dst_handle, MEM_HANDLE_T src_handle);
extern void khrn_copy_buffer_render_state_flush(KHRN_COPY_BUFFER_RENDER_STATE_T *render_state);

#endif
