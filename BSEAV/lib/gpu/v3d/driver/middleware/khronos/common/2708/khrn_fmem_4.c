/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/2708/khrn_fmem_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#ifndef NO_OPENVG
#include "middleware/khronos/vg/vg_ramp.h"
#endif /* NO_OPENVG */
#include "interface/vcos/vcos.h"
#include <stddef.h> /* for offsetof */

#define GAP 5

static void tweak_init(KHRN_FMEM_T *fmem, KHRN_FMEM_TWEAK_LIST_T *list);
static KHRN_FMEM_TWEAK_T *tweak_new(KHRN_FMEM_T *fmem, MEM_LOCK_T *lbh);
static KHRN_FMEM_TWEAK_T *tweak_next(KHRN_FMEM_T *fmem, KHRN_FMEM_TWEAK_LIST_T *list);
static void tweak_close(KHRN_FMEM_TWEAK_LIST_T *list);
static bool alloc_next(KHRN_FMEM_T *fmem);
static bool fix(KHRN_FMEM_T *fmem, uint8_t *location, MEM_HANDLE_T handle, uint32_t offset);
static void do_fix_lock(KHRN_FMEM_T *fmem);
static void do_fix_unlock(KHRN_FMEM_T *fmem);
static void do_fix_release(KHRN_FMEM_T *fmem);
static void do_specials(KHRN_FMEM_T *fmem, const uint32_t *specials);
static void do_interlock_transfer(KHRN_FMEM_T *fmem);
static void do_interlock_release(KHRN_FMEM_T *fmem);
#ifndef NO_OPENVG
static void do_ramp_lock(KHRN_FMEM_T *fmem);
static void do_ramp_unlock(KHRN_FMEM_T *fmem);
#endif /* NO_OPENVG */
static uint32_t *alloc_junk(KHRN_FMEM_T *fmem, int size, int align, MEM_LOCK_T *lbh);

KHRN_FMEM_T *khrn_fmem_init(KHRN_INTERLOCK_USER_T interlock_user)
{
   uint32_t *block;
   KHRN_NMEM_GROUP_T temp_nmem_group;
   KHRN_FMEM_T *fmem;
   MEM_LOCK_T lbh;

   khrn_nmem_group_init(&temp_nmem_group, true);

   block = (uint32_t *)khrn_nmem_group_alloc_master(&temp_nmem_group, &lbh);
   if (!block) return NULL;

   fmem = (KHRN_FMEM_T *)block;
   fmem->lbh = lbh;
   fmem->nmem_group = temp_nmem_group;
   block += sizeof(KHRN_FMEM_T)/4;
   fmem->nmem_entered = false;

   fmem->bin_begin = NULL;
   fmem->bin_end = NULL;
   fmem->render_begin = NULL;
   fmem->cle_pos = (uint8_t *)block;
   fmem->last_cle_pos = NULL;
   fmem->junk_pos = block + ((KHRN_NMEM_GROUP_BLOCK_SIZE-sizeof(KHRN_FMEM_T))/4);

   fmem->fix_start = (KHRN_FMEM_FIX_T *)alloc_junk(fmem, sizeof(KHRN_FMEM_FIX_T), 4, &lbh);
   vcos_assert(fmem->fix_start);  /* Should be enough room in initial block that this didn't fail */
   fmem->fix_end = fmem->fix_start;
   fmem->fix_end->count = 0;
   fmem->fix_end->next = NULL;

   tweak_init(fmem, &fmem->special);
   tweak_init(fmem, &fmem->interlock);
#ifndef NO_OPENVG
   tweak_init(fmem, &fmem->ramp);
#endif /* NO_OPENVG */

   fmem->interlock_user = interlock_user;

   return fmem;
}

void khrn_fmem_discard(KHRN_FMEM_T *fmem)
{
   vcos_assert(!fmem->nmem_entered);
   tweak_close(&fmem->interlock);
   do_fix_release(fmem);
   do_interlock_release(fmem);
   khrn_nmem_group_term(&fmem->nmem_group);
}

static uint32_t *alloc_junk(KHRN_FMEM_T *fmem, int size, int align, MEM_LOCK_T *lbh)
{
   size_t new_junk;

   vcos_assert(!((size_t)fmem->junk_pos & 3) && !(size & 3) && align >= 4 && align <= 4096);
   vcos_assert(fmem->cle_pos + GAP <= (uint8_t *)fmem->junk_pos);

   new_junk = ((size_t)fmem->junk_pos - size) & ~(align - 1);
   if ((size_t)fmem->cle_pos + GAP > new_junk)
   {
      if (!alloc_next(fmem)) return NULL;
      new_junk = ((size_t)fmem->junk_pos - size) & ~(align - 1);
   }

   vcos_assert((size_t)fmem->cle_pos + GAP <= new_junk);

   fmem->junk_pos = (uint32_t *)new_junk;

   /* mem handle where the block came from */
   *lbh = fmem->lbh;
   return (uint32_t *)new_junk;
}

uint32_t *khrn_fmem_junk(KHRN_FMEM_T *fmem, int size, int align, MEM_LOCK_T *lbh)
{
   uint32_t *result;
   result = alloc_junk(fmem, size, align, lbh);
   return result;
}

uint8_t *khrn_fmem_cle(KHRN_FMEM_T *fmem, int size)
{
   uint8_t *result;

   vcos_assert(fmem->cle_pos + GAP <= (uint8_t *)fmem->junk_pos);

   if (fmem->cle_pos + size + GAP > (uint8_t *)fmem->junk_pos)
   {
      if (!alloc_next(fmem)) return NULL;
   }

   result = fmem->cle_pos;
   fmem->cle_pos += size;

   fmem->last_cle_pos = fmem->cle_pos;

   return result;
}

bool khrn_fmem_fix(KHRN_FMEM_T *fmem, uint32_t *location, MEM_HANDLE_T handle, uint32_t offset)
{
   bool result;
   result = fix(fmem, (uint8_t *)location, handle, offset);
   return result;
}

bool khrn_fmem_add_fix(KHRN_FMEM_T *fmem, uint8_t **p, MEM_HANDLE_T handle, uint32_t offset)
{
   if (fix(fmem, *p, handle, offset))
   {
      *p += 4;
      return true;
   }
   /* vcos_assert(0);      This is a valid out-of-memory condition */
   return false;
}

static bool fix(KHRN_FMEM_T *fmem, uint8_t *location, MEM_HANDLE_T handle, uint32_t offset)
{
   uint32_t count;

   count = fmem->fix_end->count;
   vcos_assert(count <= TWEAK_COUNT);
   if (count == TWEAK_COUNT)
   {
      MEM_LOCK_T lbh;
      KHRN_FMEM_FIX_T *f;

      f = (KHRN_FMEM_FIX_T *)alloc_junk(fmem, sizeof(KHRN_FMEM_FIX_T), 4, &lbh);
      if (!f) return false;

      f->lbh = lbh;
      f->count = 0;
      f->next = NULL;
      count = 0;
      fmem->fix_end->next = f;
      fmem->fix_end = f;
   }

   vcos_assert(count < TWEAK_COUNT);

   fmem->fix_end->handles[count].mh_handle = handle;
   fmem->fix_end->handles[count].offset = offset;
   fmem->fix_end->locations[count] = location;
   fmem->fix_end->count = count + 1;

   mem_acquire_retain(handle);
   return true;
}

bool khrn_fmem_add_special(KHRN_FMEM_T *fmem, uint8_t **p, uint32_t special_i, uint32_t offset)
{
   KHRN_FMEM_TWEAK_T *tw;

   tw = tweak_next(fmem, &fmem->special);
   if (!tw) return false;

   tw->special_location = *p;
   tw->special_i = special_i;
   add_word(p, offset);

   return true;
}

bool khrn_fmem_interlock(KHRN_FMEM_T *fmem, MEM_HANDLE_T handle, uint32_t offset)
{
   KHRN_FMEM_TWEAK_T *tw;

   tw = tweak_next(fmem, &fmem->interlock);
   if (!tw) return false;

   tw->interlock.mh_handle = handle;
   tw->interlock.offset = offset;

   mem_acquire(handle);

   return true;
}

void khrn_fmem_start_bin(KHRN_FMEM_T *fmem)
{
   vcos_assert(fmem->bin_begin == NULL);

   fmem->bin_begin = fmem->cle_pos;
   fmem->bin_begin_lbh = fmem->lbh;
}

bool khrn_fmem_start_render(KHRN_FMEM_T *fmem)
{
   vcos_assert(fmem->bin_end == NULL && fmem->render_begin == NULL);

   fmem->bin_end = fmem->cle_pos;
   fmem->render_begin = fmem->cle_pos;
   fmem->render_begin_lbh = fmem->lbh;
   return true;
}

void khrn_fmem_prep_for_job(KHRN_FMEM_T *fmem, uint32_t bin_mem, uint32_t bin_mem_size)
{
   uint32_t     specials[4];

   tweak_close(&fmem->special);
   tweak_close(&fmem->interlock);
#ifndef NO_OPENVG
   tweak_close(&fmem->ramp);
#endif /* NO_OPENVG */

   do_interlock_transfer(fmem);

   fmem->nmem_entered = true;
   fmem->nmem_pos = khrn_nmem_enter();

   specials[0] = 0;
   specials[1] = bin_mem;                 /* khrn_hw_addr((void *)bin_mem); */
   specials[2] = bin_mem + bin_mem_size;  /* khrn_hw_addr((void *)bin_mem) + bin_mem_size; */
   specials[3] = bin_mem_size;

   do_fix_lock(fmem);
#ifndef NO_OPENVG
   do_ramp_lock(fmem);
#endif /* NO_OPENVG */
   do_specials(fmem, specials);  /* This patches up the bin memory addresses */
}

void khrn_fmem_prep_for_render_only_job(KHRN_FMEM_T *fmem)
{
   tweak_close(&fmem->special);
   tweak_close(&fmem->interlock);
#ifndef NO_OPENVG
   tweak_close(&fmem->ramp);
#endif /* NO_OPENVG */

   do_interlock_transfer(fmem);

   fmem->nmem_entered = true;
   fmem->nmem_pos = khrn_nmem_enter();

   do_fix_lock(fmem);
#ifndef NO_OPENVG
   do_ramp_lock(fmem);
#endif /* NO_OPENVG */
}

void khrn_job_done_fmem(KHRN_FMEM_T *fmem)
{
   do_fix_unlock(fmem);
#ifndef NO_OPENVG
   do_ramp_unlock(fmem);
#endif /* NO_OPENVG */
   vcos_assert(fmem->nmem_entered);
   khrn_nmem_group_term_and_exit(&fmem->nmem_group, fmem->nmem_pos);
}

static bool alloc_next(KHRN_FMEM_T *fmem)
{
   uint32_t *block;
   MEM_LOCK_T lbh;

   //vcos_assert(fmem->cle_pos + GAP <= (uint8_t *)fmem->junk_pos);

   block = (uint32_t *)khrn_nmem_group_alloc_master(&fmem->nmem_group, &lbh);
   if (!block) return false;

   fmem->lbh = lbh;
   add_byte(&fmem->cle_pos, KHRN_HW_INSTR_BRANCH);
   add_pointer(&fmem->cle_pos, block, &fmem->lbh);

   fmem->cle_pos = (uint8_t *)block;
   fmem->junk_pos = block + (KHRN_NMEM_GROUP_BLOCK_SIZE/4);
   return true;
}

static void tweak_init(KHRN_FMEM_T *fmem, KHRN_FMEM_TWEAK_LIST_T *list)
{
   list->start = tweak_new(fmem, &list->lbh);
   list->header = list->start;
   list->i = 0;
   vcos_assert(list->start);   /* Should be enough room in initial block that this didn't fail */
}

static void tweak_close(KHRN_FMEM_TWEAK_LIST_T *list)
{
   list->header->count = list->i;
}

static KHRN_FMEM_TWEAK_T *tweak_new(KHRN_FMEM_T *fmem, MEM_LOCK_T *lbh)
{
   KHRN_FMEM_TWEAK_T *result;

   result = (KHRN_FMEM_TWEAK_T *)alloc_junk(fmem, sizeof(KHRN_FMEM_TWEAK_T) * TWEAK_COUNT, 4, lbh);
   if (!result) return NULL;
   memset(result, 0, sizeof(KHRN_FMEM_TWEAK_T) * TWEAK_COUNT);

   result->next = NULL;
   result->count = TWEAK_COUNT-1;

   return result;
}

static KHRN_FMEM_TWEAK_T *tweak_next(KHRN_FMEM_T *fmem, KHRN_FMEM_TWEAK_LIST_T *list)
{
   if (list->i >= list->header->count)
   {
      MEM_LOCK_T lbh;
      KHRN_FMEM_TWEAK_T *tw = tweak_new(fmem, &lbh);
      if (!tw) return NULL;

      vcos_assert(list->header->count == list->i);
      list->header->next = tw;
      list->header = tw;
      list->i = 0;
      list->lbh = lbh;
   }

   list->i++;
   return &list->header[list->i];
}

static void do_fix_lock(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_FIX_T *f;
   uint32_t i;

   for (f = fmem->fix_start; f != NULL; f = f->next)
   {
      for (i = 0; i < f->count; i++)
      {
         uint32_t offset = mem_lock_offset(f->handles[i].mh_handle);
         put_word(f->locations[i], offset + f->handles[i].offset);    //PTR
      }
   }
}

static void do_fix_unlock(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_FIX_T *f;
   uint32_t i;

   for (f = fmem->fix_start; f != NULL; f = f->next)
   {
      for (i = 0; i < f->count; i++)
      {
         mem_unlock(f->handles[i].mh_handle);
         mem_release(f->handles[i].mh_handle);
      }
   }
}

static void do_fix_release(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_FIX_T *f;
   uint32_t i;

   for (f = fmem->fix_start; f != NULL; f = f->next)
   {
      for (i = 0; i < f->count; i++)
         mem_release(f->handles[i].mh_handle);
   }
}

static void do_specials(KHRN_FMEM_T *fmem, const uint32_t *specials)
{
   KHRN_FMEM_TWEAK_T *h;
   uint32_t i;
   uint32_t w;

   for (h = fmem->special.start; h != NULL; h = h->next)
   {
      for (i = 1; i <= h->count; i++)
      {
         w = get_word(h[i].special_location);
         w += specials[h[i].special_i];
         put_word(h[i].special_location, w);
      }
   }
}

static void do_interlock_transfer(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_TWEAK_T *h;
   uint32_t i;
   KHRN_INTERLOCK_T *interlock;

   for (h = fmem->interlock.start; h != NULL; h = h->next)
   {
      for (i = 1; i <= h->count; i++)
      {
         interlock = (KHRN_INTERLOCK_T *)((uint8_t *)mem_lock(h[i].interlock.mh_handle, NULL) + h[i].interlock.offset);
         khrn_interlock_transfer(interlock, fmem->interlock_user, KHRN_INTERLOCK_FIFO_HW_RENDER);
         mem_unlock(h[i].interlock.mh_handle);
         mem_release(h[i].interlock.mh_handle);
      }
   }
}

static void do_interlock_release(KHRN_FMEM_T *fmem) {
   KHRN_FMEM_TWEAK_T *h;
   uint32_t i;
   KHRN_INTERLOCK_T *interlock;

   for (h = fmem->interlock.start; h != NULL; h = h->next)
   {
      for (i = 1; i <= h->count; i++)
      {
         interlock = (KHRN_INTERLOCK_T *)((uint8_t *)mem_lock(h[i].interlock.mh_handle, NULL) + h[i].interlock.offset);
         khrn_interlock_release(interlock, fmem->interlock_user);
         mem_unlock(h[i].interlock.mh_handle);
         mem_release(h[i].interlock.mh_handle);
      }
   }
}

#ifndef NO_OPENVG

static void do_ramp_lock(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_TWEAK_T *h;
   uint32_t i;
   VG_RAMP_T *ramp;

   for (h = fmem->ramp.start; h != NULL; h = h->next)
   {
      for (i = 1; i <= h->count; i++)
      {
         ramp = (VG_RAMP_T *)mem_lock(h[i].ramp_handle, NULL);
         MEM_LOCK_T lbh;
         void *ramp_location = mem_lock(ramp->data, &lbh);
         *h[i].ramp_location += khrn_hw_addr(ramp_location, &lbh);
         mem_unlock(h[i].ramp_handle);
      }
   }
}

static void do_ramp_unlock(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_TWEAK_T *h;
   uint32_t i;
   VG_RAMP_T *ramp;

   for (h = fmem->ramp.start; h != NULL; h = h->next)
   {
      for (i = 1; i <= h->count; i++)
      {
         ramp = (VG_RAMP_T *)mem_lock(h[i].ramp_handle, NULL);
         mem_unlock(ramp->data);
         mem_unlock(h[i].ramp_handle);
         vg_ramp_unretain(h[i].ramp_handle); /* todo: this locks internally */
         vg_ramp_release(h[i].ramp_handle, h[i].ramp_i);
      }
   }
}

#endif /* NO_OPENVG */

bool khrn_fmem_special(KHRN_FMEM_T *fmem, uint32_t *location, uint32_t special_i, uint32_t offset)
{
   uint8_t *p = (uint8_t *)location;
   return khrn_fmem_add_special(fmem, &p, special_i, offset);
}

bool khrn_fmem_is_here(KHRN_FMEM_T *fmem, uint8_t *p)
{
   return fmem->last_cle_pos == p;
}

#ifndef NO_OPENVG

bool khrn_fmem_ramp(KHRN_FMEM_T *fmem, uint32_t *location, MEM_HANDLE_T handle, uint32_t offset, uint32_t ramp_i)
{
   KHRN_FMEM_TWEAK_T *tw;

   tw = tweak_next(fmem, &fmem->ramp);
   if (!tw) return false;

   // Should this use add_word to be endian correct??
   *location = offset;

   tw->ramp_location = location;
   tw->ramp_handle = handle;
   tw->ramp_i = ramp_i;

   vg_ramp_acquire(handle, ramp_i);

   return true;
}

#endif /* NO_OPENVG */

bool khrn_fmem_fix_special_0(KHRN_FMEM_T *fmem, uint32_t *location, MEM_HANDLE_T handle, uint32_t offset)
{
   uint8_t *p = (uint8_t *)location;
   return khrn_fmem_add_fix_special_0(fmem, &p, handle, offset);
}

bool khrn_fmem_add_fix_special_0(KHRN_FMEM_T *fmem, uint8_t **p, MEM_HANDLE_T handle, uint32_t offset)
{
   uint8_t *p2 = *p;
   if (!khrn_fmem_add_special(fmem, &p2, KHRN_FMEM_SPECIAL_0, 0)) return false;

   if (handle == MEM_INVALID_HANDLE)
   {
      add_word(p, offset);
      return true;
   }
   else
   {
      return khrn_fmem_add_fix(fmem, p, handle, offset);
   }
}

bool khrn_fmem_add_fix_image(KHRN_FMEM_T *fmem, uint8_t **p, MEM_HANDLE_T image_handle, uint32_t offset)
{
   bool result;
   KHRN_IMAGE_T *image;
   if (!khrn_fmem_interlock(fmem, image_handle, offsetof(KHRN_IMAGE_T, interlock)))
   {
      return false;
   }
   image = (KHRN_IMAGE_T *)mem_lock(image_handle, NULL);

   result = khrn_fmem_add_fix(fmem, p, image->mh_storage, image->offset + offset);
   mem_unlock(image_handle);
   return result;
}
