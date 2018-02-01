/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/ext/egl_khr_sync_client.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/2708/khrn_fmem_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/egl/egl_server.h"
#include "interface/vcos/vcos.h"
#include "middleware/khronos/common/khrn_mem.h"
#include <stddef.h> /* for offsetof */

#define GAP 5

static void tweak_init(KHRN_FMEM_TWEAK_LIST_T *list);
static KHRN_FMEM_TWEAK_T *tweak_new(void);
static KHRN_FMEM_TWEAK_T *tweak_next(KHRN_FMEM_TWEAK_LIST_T *list);
static void tweak_close(KHRN_FMEM_TWEAK_LIST_T *list);
static bool alloc_next(KHRN_FMEM_T *fmem);
static bool fix(KHRN_FMEM_T *fmem, uint8_t *location, MEM_HANDLE_T mh_handle, uint32_t offset);
static void do_fix_lock(KHRN_FMEM_T *fmem);
static void do_fix_unlock(KHRN_FMEM_T *fmem);
static void do_fix_release(KHRN_FMEM_T *fmem);
static void do_specials(KHRN_FMEM_T *fmem, const uint32_t *specials);
static void do_specials_discard(KHRN_FMEM_T *fmem);
static void do_interlock_transfer(KHRN_FMEM_T *fmem);
static void do_interlock_release(KHRN_FMEM_T *fmem);
static void do_sync_release(KHRN_FMEM_T *fmem);
static uint32_t *alloc_junk(KHRN_FMEM_T *fmem, int size, int align, MEM_LOCK_T *lbh);

KHRN_FMEM_T *khrn_fmem_init(KHRN_INTERLOCK_USER_T interlock_user)
{
   KHRN_NMEM_GROUP_T temp_nmem_group;
   khrn_nmem_group_init(&temp_nmem_group, true);

   MEM_LOCK_T lbh;
   uint32_t *block = (uint32_t *)khrn_nmem_group_alloc_master(&temp_nmem_group, &lbh);
   if (!block) return NULL;

   KHRN_FMEM_T *fmem = (KHRN_FMEM_T *)block;
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

   fmem->fix_start = (KHRN_FMEM_FIX_T *)malloc(sizeof(KHRN_FMEM_FIX_T));
   assert(fmem->fix_start);  /* Should be enough room in initial block that this didn't fail */

   fmem->fix_end = fmem->fix_start;
   fmem->fix_end->count = 0;
   fmem->fix_end->next = NULL;

   tweak_init(&fmem->special);
   tweak_init(&fmem->interlock);
   tweak_init(&fmem->sync);

   fmem->interlock_user = interlock_user;

   return fmem;
}

void khrn_fmem_discard(KHRN_FMEM_T *fmem)
{
   assert(!fmem->nmem_entered);

   tweak_close(&fmem->special);
   tweak_close(&fmem->interlock);
   tweak_close(&fmem->sync);

   do_fix_release(fmem);
   do_interlock_release(fmem);
   do_sync_release(fmem);

   do_specials_discard(fmem);
   khrn_nmem_group_term(&fmem->nmem_group);
}

void khrn_fmem_flush(KHRN_FMEM_T *fmem)
{
   khrn_nmem_group_flush(&fmem->nmem_group);
}

static uint32_t *alloc_junk(KHRN_FMEM_T *fmem, int size, int align, MEM_LOCK_T *lbh)
{
   assert(!((size_t)fmem->junk_pos & 3) && !(size & 3) && align >= 4 && align <= 4096);
   assert(fmem->cle_pos + GAP <= (uint8_t *)fmem->junk_pos);

   size_t new_junk = ((size_t)fmem->junk_pos - size) & ~(align - 1);
   if ((size_t)fmem->cle_pos + GAP > new_junk)
   {
      if (!alloc_next(fmem)) return NULL;
      new_junk = ((size_t)fmem->junk_pos - size) & ~(align - 1);
   }

   assert((size_t)fmem->cle_pos + GAP <= new_junk);

   fmem->junk_pos = (uint32_t *)new_junk;

   /* mem handle where the block came from */
   *lbh = fmem->lbh;
   return (uint32_t *)new_junk;
}

uint32_t *khrn_fmem_junk(KHRN_FMEM_T *fmem, int size, int align, MEM_LOCK_T *lbh)
{
   uint32_t *result = alloc_junk(fmem, size, align, lbh);
   return result;
}

uint8_t *khrn_fmem_cle(KHRN_FMEM_T *fmem, int size)
{
   assert(fmem->cle_pos + GAP <= (uint8_t *)fmem->junk_pos);

   if (fmem->cle_pos + size + GAP > (uint8_t *)fmem->junk_pos)
   {
      if (!alloc_next(fmem)) return NULL;
   }

   uint8_t *result = fmem->cle_pos;
   fmem->cle_pos += size;

   fmem->last_cle_pos = fmem->cle_pos;

   return result;
}

bool khrn_fmem_fix(KHRN_FMEM_T *fmem, uint32_t *location, MEM_HANDLE_T handle, uint32_t offset)
{
   bool result = fix(fmem, (uint8_t *)location, handle, offset);
   return result;
}

bool khrn_fmem_add_fix(KHRN_FMEM_T *fmem, uint8_t **p, MEM_HANDLE_T handle, uint32_t offset)
{
   if (fix(fmem, *p, handle, offset))
   {
      *p += 4;
      return true;
   }
   /* assert(0);      This is a valid out-of-memory condition */
   return false;
}

static bool fix(KHRN_FMEM_T *fmem, uint8_t *location, MEM_HANDLE_T mh_handle, uint32_t offset)
{
   uint32_t count = fmem->fix_end->count;
   assert(count <= TWEAK_COUNT);
   if (count == TWEAK_COUNT)
   {
      KHRN_FMEM_FIX_T *f = (KHRN_FMEM_FIX_T *)malloc(sizeof(KHRN_FMEM_FIX_T));
      if (!f) return false;

      f->count = 0;
      f->next = NULL;
      count = 0;
      fmem->fix_end->next = f;
      fmem->fix_end = f;
   }

   assert(count < TWEAK_COUNT);

   fmem->fix_end->mh_handle[count] = mh_handle;
   fmem->fix_end->offset[count] = offset;
   fmem->fix_end->location[count] = location;

   fmem->fix_end->count = count + 1;

   mem_acquire_retain(mh_handle);

   return true;
}

bool khrn_fmem_add_special(KHRN_FMEM_T *fmem, uint8_t **p, uint32_t special_i, uint32_t offset)
{
   KHRN_FMEM_TWEAK_T *tw = tweak_next(&fmem->special);
   if (!tw) return false;

   tw->special_location = *p;
   tw->special_i = special_i;
   add_word(p, offset);

   return true;
}

bool khrn_fmem_interlock(KHRN_FMEM_T *fmem, void *p, uint32_t offset)
{
   KHRN_FMEM_TWEAK_T *tw = tweak_next(&fmem->interlock);
   if (!tw) return false;

   tw->interlock.p = p;
   tw->interlock.offset = offset;

   khrn_mem_acquire(p);

   return true;
}

void khrn_fmem_start_bin(KHRN_FMEM_T *fmem)
{
   assert(fmem->bin_begin == NULL);

   fmem->bin_begin = fmem->cle_pos;
   fmem->bin_begin_lbh = fmem->lbh;
}

bool khrn_fmem_start_render(KHRN_FMEM_T *fmem)
{
   assert(fmem->bin_end == NULL && fmem->render_begin == NULL);

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
   tweak_close(&fmem->sync);

   do_interlock_transfer(fmem);

   fmem->nmem_entered = true;
   fmem->nmem_pos = khrn_nmem_enter();

   specials[KHRN_FMEM_SPECIAL_0] = 0;
   specials[KHRN_FMEM_SPECIAL_BIN_MEM] = bin_mem;                 /* khrn_hw_addr((void *)bin_mem); */
   specials[KHRN_FMEM_SPECIAL_BIN_MEM_END] = bin_mem + bin_mem_size;  /* khrn_hw_addr((void *)bin_mem) + bin_mem_size; */
   specials[KHRN_FMEM_SPECIAL_BIN_MEM_SIZE] = bin_mem_size;

   do_specials(fmem, specials);  /* This patches up the bin memory addresses */

   do_fix_lock(fmem);
}

void khrn_fmem_prep_for_render_only_job(KHRN_FMEM_T *fmem)
{
   tweak_close(&fmem->special);
   tweak_close(&fmem->interlock);
   tweak_close(&fmem->sync);

   do_interlock_transfer(fmem);
   do_specials_discard(fmem);

   fmem->nmem_entered = true;
   fmem->nmem_pos = khrn_nmem_enter();

   do_fix_lock(fmem);
}

void khrn_job_done_fmem(KHRN_FMEM_T *fmem)
{
   do_fix_unlock(fmem);
   do_sync_release(fmem);
   assert(fmem->nmem_entered);
   khrn_nmem_group_term_and_exit(&fmem->nmem_group, fmem->nmem_pos);
}

static bool alloc_next(KHRN_FMEM_T *fmem)
{
   MEM_LOCK_T lbh;
   //assert(fmem->cle_pos + GAP <= (uint8_t *)fmem->junk_pos);
   uint32_t *block = (uint32_t *)khrn_nmem_group_alloc_master(&fmem->nmem_group, &lbh);
   if (!block) return false;

   fmem->lbh = lbh;
   add_byte(&fmem->cle_pos, KHRN_HW_INSTR_BRANCH);
   add_pointer(&fmem->cle_pos, block, &fmem->lbh);

   fmem->cle_pos = (uint8_t *)block;
   fmem->junk_pos = block + (KHRN_NMEM_GROUP_BLOCK_SIZE/4);
   return true;
}

static void tweak_init(KHRN_FMEM_TWEAK_LIST_T *list)
{
   list->start = tweak_new();
   list->header = list->start;
   list->i = 0;
   assert(list->start);   /* Should be enough room in initial block that this didn't fail */
}

static void tweak_close(KHRN_FMEM_TWEAK_LIST_T *list)
{
   list->header->count = list->i;
}

static KHRN_FMEM_TWEAK_T *tweak_new(void)
{
   KHRN_FMEM_TWEAK_T *result = (KHRN_FMEM_TWEAK_T *)malloc(sizeof(KHRN_FMEM_TWEAK_T) * TWEAK_COUNT);
   if (!result) return NULL;
   memset(result, 0, sizeof(KHRN_FMEM_TWEAK_T) * TWEAK_COUNT);

   result->next = NULL;
   result->count = TWEAK_COUNT;

   return result;
}

static KHRN_FMEM_TWEAK_T *tweak_next(KHRN_FMEM_TWEAK_LIST_T *list)
{
   if (list->i == list->header->count)
   {
      KHRN_FMEM_TWEAK_T *tw = tweak_new();
      if (!tw) return NULL;

      assert(list->header->count == list->i);
      list->header->next = tw;
      list->header = tw;
      list->i = 0;
   }

   return &list->header[list->i++];
}

static void do_fix_lock(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_FIX_T *f;
   for (f = fmem->fix_start; f != NULL; f = f->next)
   {
      for (uint32_t i = 0; i < f->count; i++)
      {
         uint32_t offset = mem_lock_offset(f->mh_handle[i]);
         put_word(f->location[i], offset + f->offset[i]);    //PTR
      }
   }
}

static void do_fix_unlock(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_FIX_T *next = NULL;
   for (KHRN_FMEM_FIX_T *f = fmem->fix_start; f != NULL; f = next)
   {
      next = f->next;
      for (uint32_t i = 0; i < f->count; i++)
      {
         mem_unlock(f->mh_handle[i]);
         mem_release(f->mh_handle[i]);
      }
      free(f);
   }
}

static void do_fix_release(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_FIX_T *next = NULL;
   for (KHRN_FMEM_FIX_T *f = fmem->fix_start; f != NULL; f = next)
   {
      next = f->next;
      for (uint32_t i = 0; i < f->count; i++)
         mem_release(f->mh_handle[i]);
      free(f);
   }
}

static void do_specials(KHRN_FMEM_T *fmem, const uint32_t *specials)
{
   KHRN_FMEM_TWEAK_T *next = NULL;
   for (KHRN_FMEM_TWEAK_T *h = fmem->special.start; h != NULL; h = next)
   {
      next = h->next;
      for (uint32_t i = 0; i < h->count; i++)
      {
         uint32_t w = get_word(h[i].special_location);
         w += specials[h[i].special_i];
         put_word(h[i].special_location, w);
      }
      free(h);
   }
}

static void do_specials_discard(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_TWEAK_T *next = NULL;
   for (KHRN_FMEM_TWEAK_T *h = fmem->special.start; h != NULL; h = next)
   {
      next = h->next;
      free(h);
   }
}

static void do_interlock_transfer(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_TWEAK_T *next = NULL;
   for (KHRN_FMEM_TWEAK_T *h = fmem->interlock.start; h != NULL; h = next)
   {
      next = h->next;
      for (uint32_t i = 0; i < h->count; i++)
      {
         KHRN_INTERLOCK_T *interlock = (KHRN_INTERLOCK_T *)((uint8_t *)h[i].interlock.p + h[i].interlock.offset);
         khrn_interlock_transfer(interlock, fmem->interlock_user, KHRN_INTERLOCK_FIFO_HW_RENDER);
         KHRN_MEM_ASSIGN(h[i].interlock.p, NULL);
      }
      free(h);
   }
}

static void do_interlock_release(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_TWEAK_T *next = NULL;
   for (KHRN_FMEM_TWEAK_T *h = fmem->interlock.start; h != NULL; h = next)
   {
      next = h->next;
      for (uint32_t i = 0; i < h->count; i++)
      {
         KHRN_INTERLOCK_T *interlock = (KHRN_INTERLOCK_T *)((uint8_t *)h[i].interlock.p + h[i].interlock.offset);
         khrn_interlock_release(interlock, fmem->interlock_user);
         KHRN_MEM_ASSIGN(h[i].interlock.p, NULL);
      }
      free(h);
   }
}

bool khrn_fmem_special(KHRN_FMEM_T *fmem, uint32_t *location, uint32_t special_i, uint32_t offset)
{
   uint8_t *p = (uint8_t *)location;
   return khrn_fmem_add_special(fmem, &p, special_i, offset);
}

bool khrn_fmem_is_here(KHRN_FMEM_T *fmem, uint8_t *p)
{
   return fmem->last_cle_pos == p;
}

bool khrn_fmem_sync(KHRN_FMEM_T *fmem, void *sync)
{
   KHRN_FMEM_TWEAK_T *tw = tweak_next(&fmem->sync);
   if (!tw) return false;

   khrn_mem_acquire(sync);

   tw->sync = sync;

   return true;
}

void do_sync_release(KHRN_FMEM_T *fmem)
{
   KHRN_FMEM_TWEAK_T *next = NULL;
   for (KHRN_FMEM_TWEAK_T *h = fmem->sync.start; h != NULL; h = next)
   {
      next = h->next;
      for (uint32_t i = 0; i < h->count; i++)
      {
         EGL_SYNC_T *sync = h->sync;
         vcos_semaphore_post(&sync->sem);
         sync->status = EGL_SIGNALED_KHR;
         KHRN_MEM_ASSIGN(sync, NULL);
      }
      free(h);
   }
}
