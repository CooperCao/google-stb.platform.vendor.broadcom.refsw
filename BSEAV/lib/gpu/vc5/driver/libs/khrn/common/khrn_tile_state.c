/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "libs/core/v3d/v3d_ver.h"

#if !V3D_VER_AT_LEAST(4,1,34,0)

#include "khrn_tile_state.h"
#include "khrn_process.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/util/demand.h"
#include "libs/platform/v3d_scheduler.h"
#include "vcos_atomic.h"

typedef struct khrn_tile_state_allocator
{
   khrn_shared_tile_state* mem[2];  // weak references
   uint32_t size[2];
   uint32_t size_uses[2];
   uint32_t history_head;
   struct
   {
      uint32_t size : 31;
      uint32_t secure : 1;
   } history[64];

} khrn_tile_state_allocator;

static khrn_tile_state_allocator tsa;

static khrn_shared_tile_state* shared_tile_state_alloc(size_t size, bool secure)
{
   gmem_handle_t handle = khrn_tile_state_alloc_gmem(size, secure);
   if (!handle)
      return NULL;

   khrn_shared_tile_state* mem = (khrn_shared_tile_state*)malloc(sizeof(khrn_shared_tile_state));
   if (!mem)
   {
      gmem_free(handle);
      return NULL;
   }

   mem->handle = handle;
   vcos_atomic_store_uint32(&mem->refs, 0x80000001, VCOS_MEMORY_ORDER_RELAXED);
   return mem;
}

static bool tile_state_strong_acquire(khrn_shared_tile_state* mem)
{
   // Acquire a strong reference, but only if there is at least one existing.
   uint32_t refs = vcos_atomic_load_uint32(&mem->refs, VCOS_MEMORY_ORDER_RELAXED);
   while (refs & 0x7fffffff)
   {
      bool success = vcos_atomic_compare_exchange_weak_uint32(
         &mem->refs,
         &refs,
         refs + 1,
         VCOS_MEMORY_ORDER_RELAXED,
         VCOS_MEMORY_ORDER_RELAXED);
      if (success)
         return true;
   }
   return false;
}

static void tile_state_strong_release(khrn_shared_tile_state* mem)
{
   // Load the gmem handle before dropping our reference.
   gmem_handle_t handle = mem->handle;

   // Release a strong reference, and assert there was one.
   uint32_t prev = vcos_atomic_fetch_sub_uint32(&mem->refs, 1, VCOS_MEMORY_ORDER_ACQ_REL);
   assert(prev & 0x7fffffff);

   // Free gmem when no strong references remain.
   if ((prev & 0x7fffffff) == 1)
      gmem_free(handle);

   // Free header when no strong or weak references remain.
   if (prev == 1)
      free(mem);
}

static void tile_state_weak_release(khrn_shared_tile_state* mem)
{
   // Release a weak reference, and assert there was one.
   uint32_t prev = vcos_atomic_fetch_sub_uint32(&mem->refs, 0x80000000, VCOS_MEMORY_ORDER_ACQ_REL);
   assert(prev & 0x80000000);

   // Free header when no strong or weaks references remain.
   if (prev == 0x80000000)
      free(mem);
}

void khrn_tile_state_init(void)
{
   memset(&tsa, 0, sizeof(tsa));
}

void khrn_tile_state_deinit(void)
{
   for (unsigned i = 0; i != 2; ++i)
   {
      if (tsa.mem[i])
         tile_state_weak_release(tsa.mem[i]);
   }
}

gmem_handle_t khrn_tile_state_alloc_gmem(size_t size, bool secure)
{
   gmem_usage_flags_t flags = GMEM_USAGE_V3D_RW | GMEM_USAGE_HINT_DYNAMIC | (secure ? GMEM_USAGE_SECURE : 0);
   gmem_handle_t handle;
   while (!(handle = gmem_alloc(size, V3D_TILE_STATE_ALIGN, flags, "khrn_tile_state")))
   {
      if (!v3d_scheduler_wait_any())
         return 0;
   }
   return handle;
}

khrn_shared_tile_state* khrn_tile_state_alloc_shared(size_t size, bool secure)
{
   assert(size <= 0x7fffffff);
   assert(khrn_get_num_cores() == 1);
   unsigned index = (unsigned)secure;

   // If not big enough, or we can't acquire a strong reference.
   if (  size > tsa.size[index]
      || !tile_state_strong_acquire(tsa.mem[index])   )
   {
      // Release old tile-state memory.
      if (tsa.mem[index])
         tile_state_weak_release(tsa.mem[index]);

      // Find largest size from history and number of occurances.
      size_t new_size = size;
      unsigned num_of_size = 0;
      for (unsigned i = 0; i != countof(tsa.history); ++i)
      {
         if (tsa.history[i].secure == index)
         {
            size_t historical_size = tsa.history[i].size;
            if (historical_size > new_size)
            {
               new_size = historical_size;
               num_of_size = 1;
            }
            else if (historical_size == new_size)
            {
               num_of_size += 1;
            }
         }
      }

      // Allocate new reference counted tile-state memory (starts with strong and weak ref).
      khrn_shared_tile_state* mem = shared_tile_state_alloc(new_size, secure);
      if (!mem)
         return NULL;
      tsa.mem[index] = mem;
      tsa.size[index] = new_size;
      tsa.size_uses[index] = num_of_size;
   }

   // Increment count of uses of this size of tile-state.
   if (tsa.size[index] == size)
      tsa.size_uses[index] += 1;

   // Using this tile-state memory this time...
   khrn_shared_tile_state* ret = tsa.mem[index];

   // Pop head item from deque,
   // Force realloc of memory if no occurances of this size.
   unsigned head = tsa.history_head;
   uint32_t head_size = tsa.history[head].size;
   uint32_t head_index = tsa.history[head].secure;
   if (head_size != 0 && head_size == tsa.size[head_index])
   {
      assert(tsa.size_uses[head_index] > 0);
      if (!--tsa.size_uses[head_index])
      {
         tile_state_weak_release(tsa.mem[head_index]);
         tsa.mem[head_index] = NULL;
         tsa.size[head_index] = 0;
      }
   }

   // Record size of this tile-state in history.
   tsa.history[head].size = size;
   tsa.history[head].secure = index;
   tsa.history_head = (head + 1) % countof(tsa.history);

   return ret;
}

void khrn_tile_state_release_shared(khrn_shared_tile_state* mem)
{
   tile_state_strong_release(mem);
}

#endif
