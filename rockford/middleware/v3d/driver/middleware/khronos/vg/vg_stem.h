/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG stem

FILE DESCRIPTION
The client allocates a number of stem objects in advance. Where possible, the
client-side vgCreate* functions pass a stem object handle to the server (rather
than letting the server allocate a handle), which is then converted into the
appropriate object on the server. This allows the client to return the handle
immediately, rather than having to wait for the server to reply with the value
of the handle it allocated.
=============================================================================*/

#ifndef VG_STEM_H
#define VG_STEM_H

#include "middleware/khronos/common/khrn_mem.h"

#define VG_STEM_SIZE 64 /* this should allow stem objects to be allocated from the small object pool */
#define VG_STEM_ALIGN 4

static INLINE MEM_HANDLE_T vg_stem_alloc(void)
{
   return mem_alloc_ex(VG_STEM_SIZE, VG_STEM_ALIGN, MEM_FLAG_NONE, "vg_stem", MEM_COMPACT_DISCARD); /* MEM_FLAG_NONE to permit alloc from small object pool */
}

static INLINE bool vg_is_stem(MEM_HANDLE_T handle)
{
   return !mem_get_term(handle);
}

#endif
