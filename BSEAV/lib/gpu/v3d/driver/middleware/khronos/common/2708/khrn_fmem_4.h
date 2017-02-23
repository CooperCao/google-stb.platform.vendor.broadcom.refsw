/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Control per-frame memory allocator

FILE DESCRIPTION
Handles allocation of memory for control lists and associated data that will be
generated each frame as HW input.
=============================================================================*/

#ifndef KHRN_FMEM_4_H
#define KHRN_FMEM_4_H

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/2708/khrn_nmem_4.h"

typedef struct _KHRN_FMEM_TWEAK_T
{
   struct _KHRN_FMEM_TWEAK_T *next;
   uint32_t count;

   uint8_t *special_location;
   uint32_t special_i;

   MEM_HANDLE_OFFSET_T interlock;

   uint32_t *ramp_location;
   MEM_HANDLE_T ramp_handle;
   uint32_t ramp_i;

} KHRN_FMEM_TWEAK_T;

typedef struct
{
   KHRN_FMEM_TWEAK_T *start;
   KHRN_FMEM_TWEAK_T *header;
   uint32_t i;
   MEM_LOCK_T lbh;
} KHRN_FMEM_TWEAK_LIST_T;

#define TWEAK_COUNT 32

typedef struct _KHRN_FMEM_FIX_T
{
   struct _KHRN_FMEM_FIX_T *next;
   uint32_t count;
   MEM_HANDLE_OFFSET_T handles[TWEAK_COUNT];
   uint8_t *locations[TWEAK_COUNT];
   MEM_LOCK_T lbh;
} KHRN_FMEM_FIX_T;

typedef struct KHRN_FMEM
{
   KHRN_NMEM_GROUP_T nmem_group;
   bool nmem_entered;
   uint32_t nmem_pos;

   uint8_t *bin_begin;
   uint8_t *bin_end;
   uint8_t *render_begin;
   uint8_t *cle_pos;

   MEM_LOCK_T bin_begin_lbh;
   MEM_LOCK_T render_begin_lbh;

   /* cle_pos after the last cle alloc. if we've allocated another block since
    * then, this won't match cle_pos. this is to support khrn_fmem_is_here() */
   uint8_t *last_cle_pos;
   uint32_t *junk_pos;
   MEM_LOCK_T lbh;

   KHRN_FMEM_FIX_T *fix_start;
   KHRN_FMEM_FIX_T *fix_end;
   KHRN_FMEM_TWEAK_LIST_T special;
   KHRN_FMEM_TWEAK_LIST_T interlock;
#ifndef NO_OPENVG
   KHRN_FMEM_TWEAK_LIST_T ramp;
#endif /* NO_OPENVG */

   KHRN_INTERLOCK_USER_T interlock_user;
} KHRN_FMEM_T;

#define KHRN_FMEM_SPECIAL_0            0
#define KHRN_FMEM_SPECIAL_BIN_MEM      1
#define KHRN_FMEM_SPECIAL_BIN_MEM_END  2
#define KHRN_FMEM_SPECIAL_BIN_MEM_SIZE 3

extern KHRN_FMEM_T *khrn_fmem_init(KHRN_INTERLOCK_USER_T interlock_user);
extern void khrn_fmem_discard(KHRN_FMEM_T *fmem);
extern uint32_t *khrn_fmem_junk(KHRN_FMEM_T *fmem, int size, int align, MEM_LOCK_T *lbh);
extern uint8_t *khrn_fmem_cle(KHRN_FMEM_T *fmem, int size);
extern bool khrn_fmem_fix(KHRN_FMEM_T *fmem, uint32_t *location, MEM_HANDLE_T handle, uint32_t offset);
extern bool khrn_fmem_add_fix(KHRN_FMEM_T *fmem, uint8_t **p, MEM_HANDLE_T handle, uint32_t offset);
extern bool khrn_fmem_add_special(KHRN_FMEM_T *fmem, uint8_t **p, uint32_t special_i, uint32_t offset);
extern bool khrn_fmem_interlock(KHRN_FMEM_T *fmem, MEM_HANDLE_T handle, uint32_t offset);
extern void khrn_fmem_start_bin(KHRN_FMEM_T *fmem);
extern bool khrn_fmem_start_render(KHRN_FMEM_T *fmem);

extern bool khrn_fmem_special(KHRN_FMEM_T *fmem, uint32_t *location, uint32_t special_i, uint32_t offset);
extern bool khrn_fmem_is_here(KHRN_FMEM_T *fmem, uint8_t *p);
extern bool khrn_fmem_ramp(KHRN_FMEM_T *fmem, uint32_t *location, MEM_HANDLE_T handle, uint32_t offset, uint32_t ramp_i);
extern bool khrn_fmem_fix_special_0(KHRN_FMEM_T *fmem, uint32_t *location, MEM_HANDLE_T handle, uint32_t offset);
extern bool khrn_fmem_add_fix_special_0(KHRN_FMEM_T *fmem, uint8_t **p, MEM_HANDLE_T handle, uint32_t offset);
extern bool khrn_fmem_add_fix_image(KHRN_FMEM_T *fmem, uint8_t **p, MEM_HANDLE_T image_handle, uint32_t offset);

extern void khrn_fmem_prep_for_job(KHRN_FMEM_T *fmem, uint32_t bin_mem, uint32_t bin_mem_size);
extern void khrn_fmem_prep_for_render_only_job(KHRN_FMEM_T *fmem);
extern void khrn_job_done_fmem(KHRN_FMEM_T *fmem);

static INLINE uint32_t khrn_fmem_get_nmem_n(KHRN_FMEM_T *fmem)
{
   return fmem->nmem_group.n;
}

#endif
