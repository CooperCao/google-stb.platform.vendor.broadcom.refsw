/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/2708/khrn_nmem_4.h"
#include "interface/vcos/vcos.h"
#include "middleware/khronos/common/khrn_mem.h"

#ifndef MEM_LEGACY_BLOCK_SIZE
#define MEM_LEGACY_BLOCK_SIZE    0x00200000
#endif

#ifndef MEM_LEGACY_BLOCK_ALIGN
#define MEM_LEGACY_BLOCK_ALIGN   4096
#endif

#define CALLBACKS_N 4
#define LEGACY_BLOCKS_N_MAX 32

/* todo: what should these values be? */
#define LIMIT 96 /* 6MB. todo: decide this dynamically? */
#define THROTTLE_LIMIT_MIN 1
#define THROTTLE_LIMIT_MAX 60
#define INITIAL_THROTTLE_LIMIT 16

union KHRN_NMEM_BLOCK_U;
typedef union KHRN_NMEM_BLOCK_U KHRN_NMEM_BLOCK_T;
union KHRN_NMEM_BLOCK_U {
   KHRN_NMEM_BLOCK_T *free_next;
   uint8_t data[KHRN_NMEM_BLOCK_SIZE];
};

vcos_static_assert(sizeof(KHRN_NMEM_BLOCK_T) == KHRN_NMEM_BLOCK_SIZE);

static VCOS_MUTEX_T mutex;

static struct {
   KHRN_NMEM_CALLBACK_T callback;
   void *p;
} callbacks[CALLBACKS_N];

/* enter_pos only modified by the master task */
static uint32_t enter_pos, exit_pos;

/* reserved_n only modified by the master task */
static uint32_t reserved_n, alloced_n, alloced_n_max, throttle_alloced_n, throttle_limit;
static bool nonthrottle_denied;

static uint32_t blocks_per_legacy_block;
static struct {
   void *p;
   MEM_HANDLE_T b; /* backing for p */
   MEM_LOCK_T lbh;
   KHRN_NMEM_BLOCK_T *free_head;
   uint32_t free_n;
} legacy_blocks[LEGACY_BLOCKS_N_MAX];

/******************************************************************************
actual alloc/free (call with mutex acquired)
******************************************************************************/

static void *alloc_block(MEM_LOCK_T *lbh)
{
   uint32_t i, j;
   void *p;
   MEM_HANDLE_T b;

   /*
      first look for free blocks within the currently allocated legacy blocks
   */

   for (i = 0; (i != LEGACY_BLOCKS_N_MAX) && legacy_blocks[i].p; ++i) {
      KHRN_NMEM_BLOCK_T *block = legacy_blocks[i].free_head;
      if (block) {
         *lbh = legacy_blocks[i].lbh;
         legacy_blocks[i].free_head = block->free_next;
         --legacy_blocks[i].free_n;
         return block;
      }
   }

   /*
      no free blocks... allocate a new legacy block
   */

   if (!vcos_verify(!legacy_blocks[LEGACY_BLOCKS_N_MAX - 1].p)) {
      /* increase LEGACY_BLOCKS_N_MAX? */
      return NULL;
   }

   b = mem_alloc(MEM_LEGACY_BLOCK_SIZE, MEM_LEGACY_BLOCK_ALIGN, (MEM_FLAG_T)(MEM_FLAG_DIRECT | MEM_FLAG_NO_INIT), "legacy_block");
   if (b == MEM_HANDLE_INVALID)
      return NULL;

   p = mem_lock(b, lbh);

   /*
      setup the legacy block
   */

   for (i = 1; i != blocks_per_legacy_block; ++i) { /* skip the first block (we're going to return it) */
      ((KHRN_NMEM_BLOCK_T *)p)[i].free_next = (i == (blocks_per_legacy_block - 1)) ? NULL : ((KHRN_NMEM_BLOCK_T *)p + i + 1);
   }

   /*
      insert the new legacy block into our list

      we keep the list sorted by memory address (highest first) under the
      assumption that it would be preferable to release legacy blocks with lower
      memory addresses back to the system (ie we allocate from the high ones
      first, so the low ones will get freed up)
   */

   for (i = 0; (uintptr_t)p < (uintptr_t)legacy_blocks[i].p; ++i) ;

   for (j = (LEGACY_BLOCKS_N_MAX - 1); j != i; --j) {
      legacy_blocks[j] = legacy_blocks[j - 1];
   }

   legacy_blocks[i].p = p;
   legacy_blocks[i].b = b;
   legacy_blocks[i].lbh = *lbh;
   /* we're going to return the first block */
   legacy_blocks[i].free_head = (KHRN_NMEM_BLOCK_T *)p + 1;
   legacy_blocks[i].free_n = blocks_per_legacy_block - 1;

   /*
      return the first block
   */

   return p;
}

static bool p_in(void *p, void *q, uint32_t size)
{
   return ((uintptr_t)p - (uintptr_t)q) < size;
}

static void free_block(void *p)
{
   uint32_t i;

   /*
      find the legacy block it's in
   */

   for (i = 0; !p_in(p, legacy_blocks[i].p, blocks_per_legacy_block * KHRN_NMEM_BLOCK_SIZE); ++i) {
      assert(i != LEGACY_BLOCKS_N_MAX); /* p not found in any legacy blocks! */
   }

   /*
      add the block to the free list
   */

   ((KHRN_NMEM_BLOCK_T *)p)->free_next = legacy_blocks[i].free_head;
   legacy_blocks[i].free_head = (KHRN_NMEM_BLOCK_T *)p;
   if (++legacy_blocks[i].free_n != blocks_per_legacy_block) {
      return; /* still some stuff in the legacy block */
   }

   if (i > 0)
   {
      /*
      the entire legacy block is free, release it
      todo: should we refuse to free a legacy block if we're holding another
      block with a lower address?
      */

      mem_unlock(legacy_blocks[i].b);
      mem_release(legacy_blocks[i].b);

      /*
      and remove the gap from the list
      */

      for (; i != (LEGACY_BLOCKS_N_MAX - 1); ++i) {
         legacy_blocks[i] = legacy_blocks[i + 1];
      }

      legacy_blocks[LEGACY_BLOCKS_N_MAX - 1].p = NULL;
   }
}

/******************************************************************************
helpers
******************************************************************************/

/* call with mutex acquired */
static void *alloc_count(bool throttle, MEM_LOCK_T *lbh)
{
   void *p = alloc_block(lbh);
   if (p) {
      ++alloced_n;
      alloced_n_max = _max(alloced_n_max, alloced_n);
      if (throttle) { ++throttle_alloced_n; }
   }
   return p;
}

/* call with mutex acquired */
static void free_count(void *p, bool throttle)
{
   if (throttle) { --throttle_alloced_n; }
   --alloced_n;
   free_block(p);
}

/* call with mutex acquired */
static void free_group_count(KHRN_NMEM_GROUP_BLOCK_T *block, bool throttle,
   /* just for checking */
   uint32_t n)
{
   KHRN_NMEM_GROUP_BLOCK_T *next_block;
   for (; block; block = next_block) {
      next_block = block->next;
      free_count(block, throttle);
      --n;
   }
   assert(n == 0);
}

/* call with mutex acquired */
static void do_callbacks(void)
{
   uint32_t i;
   for (i = 0; i != CALLBACKS_N; ++i) {
      if (callbacks[i].callback) {
         callbacks[i].callback(callbacks[i].p);
      }
   }
}

/* call with mutex acquired */
static void update_throttle_limit(void)
{
   if (nonthrottle_denied) {
      /* divide by 2 to quickly bring throttle_limit down when there is an
       * extreme change in memory usage. todo: this might result in an
       * unpleasant saw oscillation... */
      throttle_limit = _max(throttle_limit / 2, THROTTLE_LIMIT_MIN);
   } else if (alloced_n_max < (LIMIT - reserved_n)) {
      throttle_limit = _min(throttle_limit + 1, THROTTLE_LIMIT_MAX);
   }
   alloced_n_max = alloced_n;
   nonthrottle_denied = false;
}

static void advance_exit_pos(uint32_t pos)
{
   UNUSED(pos);
   assert(exit_pos == pos);
   ++exit_pos;
}

static bool alloc_ok(bool throttle)
{
   return (alloced_n < (LIMIT - reserved_n)) &&
      (!throttle || (throttle_alloced_n < throttle_limit));
}

static bool wait_ok(uint32_t pos)
{
   /* if there's something ahead of pos in the pipeline, ok to wait */
   return (int32_t)(pos - exit_pos) > 0;
}

/******************************************************************************
interface
******************************************************************************/

bool khrn_nmem_init(void)
{
   uint32_t i;

   if (!vcos_verify(vcos_mutex_create(&mutex, "khrn_nmem") == VCOS_SUCCESS)) {
      return false;
   }

   for (i = 0; i != CALLBACKS_N; ++i) {
      callbacks[i].callback = NULL;
   }

   enter_pos = 0;
   exit_pos = 0;

   reserved_n = 0;
   alloced_n = 0;
   alloced_n_max = 0;
   throttle_alloced_n = 0;
   throttle_limit = INITIAL_THROTTLE_LIMIT;
   nonthrottle_denied = false;

   blocks_per_legacy_block = MEM_LEGACY_BLOCK_SIZE / KHRN_NMEM_BLOCK_SIZE;
   for (i = 0; i != LEGACY_BLOCKS_N_MAX; ++i) {
      legacy_blocks[i].p = NULL;
   }

   return true;
}

void khrn_nmem_term(void)
{
#ifndef NDEBUG
   uint32_t i;
#endif

   if (legacy_blocks[0].b) {
      mem_unlock(legacy_blocks[0].b);
      mem_release(legacy_blocks[0].b);
      legacy_blocks[0].p = NULL;
   }

   assert(!legacy_blocks[0].p);

   assert((throttle_limit >= THROTTLE_LIMIT_MIN) && (throttle_limit <= THROTTLE_LIMIT_MAX));
   assert(throttle_alloced_n == 0);
   assert(alloced_n == 0);
   assert(reserved_n == 0);

   assert(enter_pos == exit_pos);

#ifndef NDEBUG
   for (i = 0; i != CALLBACKS_N; ++i) {
      assert(!callbacks[i].callback);
   }
#endif

   vcos_mutex_delete(&mutex);
}

void khrn_nmem_register(KHRN_NMEM_CALLBACK_T callback, void *p)
{
   uint32_t i;
   vcos_mutex_lock(&mutex);
   for (i = 0; i != CALLBACKS_N; ++i) {
      if (!callbacks[i].callback) {
         callbacks[i].callback = callback;
         callbacks[i].p = p;
         vcos_mutex_unlock(&mutex);
         return;
      }
   }
   UNREACHABLE(); /* todo: increase CALLBACKS_N? */
}

void khrn_nmem_unregister(KHRN_NMEM_CALLBACK_T callback, void *p)
{
   uint32_t i;
   vcos_mutex_lock(&mutex);
   for (i = 0; i != CALLBACKS_N; ++i) {
      if ((callbacks[i].callback == callback) &&
         (callbacks[i].p == p)) {
         callbacks[i].callback = NULL;
         vcos_mutex_unlock(&mutex);
         return;
      }
   }
   UNREACHABLE(); /* callback not found! */
}

void khrn_nmem_reserve(int32_t adjustment)
{
   reserved_n += adjustment;
   assert((int32_t)reserved_n >= 0);
   assert(reserved_n < LIMIT);
}

uint32_t khrn_nmem_enter(void)
{
   uint32_t pos;
   vcos_mutex_lock(&mutex);
   pos = enter_pos;
   enter_pos++;
   khrn_barrier();
   vcos_mutex_unlock(&mutex);

   return pos;
}

uint32_t khrn_nmem_get_enter_pos(void)
{
   uint32_t pos;

   vcos_mutex_lock(&mutex);
   pos = enter_pos;
   khrn_barrier();
   vcos_mutex_unlock(&mutex);
   return pos;
}

void khrn_nmem_exit(uint32_t pos)
{
   vcos_mutex_lock(&mutex);
   update_throttle_limit();
   advance_exit_pos(pos);
   do_callbacks(); /* notify potential waiters */
   vcos_mutex_unlock(&mutex);
}

bool khrn_nmem_should_wait(bool throttle, uint32_t pos)
{
   bool ret = false;
   vcos_mutex_lock(&mutex);
   if (alloc_ok(throttle)) {
      vcos_mutex_unlock(&mutex);
      return ret;
   }
   if (!throttle) {
      nonthrottle_denied = true;
   }
   ret = wait_ok(pos);
   vcos_mutex_unlock(&mutex);
   return ret;
}

void *khrn_nmem_alloc(bool throttle, bool *wait, uint32_t pos, MEM_LOCK_T *lbh)
{
   void *p;

   vcos_mutex_lock(&mutex);

   /* reserved blocks are for allocations that can't wait. we currently set
    * nonthrottle_denied if we allocate into the reserved space, even if !wait.
    * todo: should we? */
   if (!alloc_ok(throttle)) {
      if (!throttle) {
         nonthrottle_denied = true;
      }
      if (wait && wait_ok(pos)) {
         vcos_mutex_unlock(&mutex);
         *wait = true;
         return NULL;
      }
      /* can't wait, have to alloc now... */
   }

   p = alloc_count(throttle, lbh);
   if (!p) {
      if (!throttle) {
         nonthrottle_denied = true;
      }
      if (wait) {
         *wait = wait_ok(pos);
      }
      vcos_mutex_unlock(&mutex);
      return NULL;
   }

   vcos_mutex_unlock(&mutex);

   return p;
}

void khrn_nmem_free(void *p, bool throttle)
{
   vcos_mutex_lock(&mutex);
   free_count(p, throttle);
   do_callbacks(); /* notify potential waiters */
   vcos_mutex_unlock(&mutex);
}

void khrn_nmem_group_init(KHRN_NMEM_GROUP_T *group, bool throttle)
{
   group->head = NULL;
   group->throttle = throttle;
   group->n = 0;
}

void *khrn_nmem_group_alloc(KHRN_NMEM_GROUP_T *group, bool *wait, uint32_t pos, MEM_LOCK_T *lbh)
{
   KHRN_NMEM_GROUP_BLOCK_T *block = (KHRN_NMEM_GROUP_BLOCK_T *)khrn_nmem_alloc(
      group->throttle, wait, pos, lbh);
   if (!block) { return NULL; }
   block->next = group->head;
   group->head = block;
   ++group->n;
   return block->data;
}

void khrn_nmem_group_term(KHRN_NMEM_GROUP_T *group)
{
   vcos_mutex_lock(&mutex);
   free_group_count(group->head, group->throttle, group->n);
   /* we don't dereference group after this point. this allows group to be
    * located inside an allocated block */
   do_callbacks(); /* notify potential waiters */
   vcos_mutex_unlock(&mutex);
}

void khrn_nmem_group_term_and_exit(KHRN_NMEM_GROUP_T *group, uint32_t pos)
{
   vcos_mutex_lock(&mutex);
   free_group_count(group->head, group->throttle, group->n);
   update_throttle_limit();
   advance_exit_pos(pos);
   do_callbacks(); /* notify potential waiters */
   vcos_mutex_unlock(&mutex);
}
