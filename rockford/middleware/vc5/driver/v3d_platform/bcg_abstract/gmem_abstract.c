/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "gmem_abstract.h"
#include "gmem_talloc.h"
#include "gmem_analyzer.h"
#include "vcos.h"
#include "helpers/demand.h"
#include "helpers/v3d/v3d_limits.h"
#include <stdlib.h>

VCOS_LOG_CAT_T gmem_abstract_log;
#define VCOS_LOG_CATEGORY (&gmem_abstract_log)

typedef struct
{
   VCOS_MUTEX_T         api_mutex;
   gmem_alloc_item      alloc_list;
   BEGL_MemoryInterface mem_iface;
   size_t               sync_block_size;
   bool                 disable_map_lock_opt;

   void                 *talloc;
} gmem_context;

static gmem_context s_context;
volatile bool gmem_sync_cpu_write;

/* Forwards declare */
void gmem_unmap_internal(gmem_handle_t handle);
void gmem_free_internal(gmem_handle_t handle);

__attribute__((visibility("default")))
void BEGL_RegisterMemoryInterface(BEGL_MemoryInterface *iface)
{
   if (iface != NULL)
   {
      s_context.mem_iface = *iface;                                    /* Register   */

      uint32_t cpuCacheLine = 512;  /* Safe value */

      /* Get the real value if possible */
      if (s_context.mem_iface.GetInfo != NULL)
         cpuCacheLine = s_context.mem_iface.GetInfo(s_context.mem_iface.context, BEGL_MemCacheLineSize);

      s_context.sync_block_size = vcos_max(V3D_MAX_CACHE_LINE_SIZE, cpuCacheLine);
   }
   else
   {
      memset(&s_context.mem_iface, 0, sizeof(BEGL_MemoryInterface));   /* Unregister */
      s_context.sync_block_size = 0;
   }
}

static size_t allocRoundingSize(void)
{
   /* Use double the max cache line to allow for pre-fetcher */
   return s_context.sync_block_size * 2;
}

static VCOS_LOG_LEVEL_T get_logging_level(void)
{
   char value[VCOS_PROPERTY_VALUE_MAX];
   vcos_property_get("debug.gmem.log_level", value, sizeof(value), "warn");

   if (value != NULL)
   {
      if (!vcos_strcasecmp(value, "trace"))
         return VCOS_LOG_TRACE;
      else if (!vcos_strcasecmp(value, "info"))
         return VCOS_LOG_INFO;
      else if (!vcos_strcasecmp(value, "warn"))
         return VCOS_LOG_WARN;
      else if (!vcos_strcasecmp(value, "error"))
         return VCOS_LOG_ERROR;
      else if (!vcos_strcasecmp(value, "never"))
         return VCOS_LOG_NEVER;
   }
   return VCOS_LOG_WARN;
}

bool gmem_init(void)
{
   /* vcos_mutex_lock(&s_context.api_mutex);
   vcos_mutex_unlock(&s_context.api_mutex); */

   /* We can't use khrn_options here - we might be being used solely by OpenCL for example */
   char value[VCOS_PROPERTY_VALUE_MAX];
   vcos_property_get("ro.nx.v3d.no_map_lock", value, sizeof(value), "0");
   s_context.disable_map_lock_opt = value != NULL ? (atoi(value) == 0 ? false : true) : false;

   vcos_log_register("gmem_abstract", &gmem_abstract_log);
   vcos_log_set_level(&gmem_abstract_log, get_logging_level());

   gmem_analyzer_init();

   s_context.talloc = talloc_initialize();

   vcos_log_trace(VCOS_FUNCTION);

   return true;
}

void gmem_destroy(void)
{
   bool             leaked = false;
   gmem_alloc_item *item;

   vcos_mutex_lock(&s_context.api_mutex);

   vcos_log_trace(VCOS_FUNCTION);

   talloc_term(s_context.talloc);

   /* initial block is NULL block, everything else is inserted after this.  Check for
      additional linked blocks */
   for (item = s_context.alloc_list.next; item != NULL; )
   {
      /* save next ptr before its deleted */
      gmem_alloc_item *next = item->next;

      if (!item->externalAlloc)
      {
         leaked = true;

         vcos_log_error("** LEAKAGE: desc = '%s', handle = %p, lazy_freed = %d, cur_lock_phys = 0x%08x, map_count = %u, size = %u\n",
                item->desc ? item->desc : "", item, item->freed ? 1 : 0, item->cur_lock_phys, item->driver_map_count, item->size
                );

         /* unmap before freeing this handle */
         while (item->driver_map_count)
            gmem_unmap_internal(item);

         /* free this handle */
         gmem_free_internal(item);
      }

      item = next;
   }

   gmem_analyzer_term();

   vcos_mutex_unlock(&s_context.api_mutex);

   assert(!leaked);
}

gmem_handle_t gmem_alloc_internal(size_t size, size_t align, gmem_usage_flags_t usage_flags, const char *desc)
{
   gmem_alloc_item   *item  = NULL;
   size_t             allocSize = allocRoundingSize();

   demand(s_context.mem_iface.Alloc != NULL);

   /* Round align up to be a multiple of cache line size */
   align = (align + (s_context.sync_block_size - 1)) & ~(s_context.sync_block_size - 1);

   /* Round size to a multiple of alloc rounding size */
   size = (size + (allocSize - 1)) & ~(allocSize - 1);

   vcos_log_trace("%s size=%d, align = %d, usage=%d, desc='%s'", VCOS_FUNCTION, size, align, usage_flags, desc ? desc : "<null>");

   item = (gmem_alloc_item *)calloc(1, sizeof(gmem_alloc_item));
   if (item == NULL)
      goto error;

   item->magic             = GMEM_HANDLE_MAGIC;
   item->size              = size;
   item->align             = align;
   item->desc              = desc;
   item->driver_map_count  = 0;
   item->cur_map_ptr       = NULL;
   item->driver_lock_count = 0;
   item->cur_lock_phys     = 0;
   item->externalAlloc     = false;
   item->freed             = false;

   if (usage_flags & GMEM_USAGE_HINT_DYNAMIC)
   {
      item->memory_handle = NULL;

      if (talloc_alloc(s_context.talloc, size, align, &item->cur_map_ptr, &item->cur_lock_phys))
         item->externalAlloc = true;
      else
         /* remove marking, so free calls the correct variant */
         usage_flags &= ~GMEM_USAGE_HINT_DYNAMIC;
   }
   item->usage_flags = usage_flags;
   item->v3d_writes = 0;

   if (item->externalAlloc == false)
   {
      /* Fill in item */
      item->memory_handle = s_context.mem_iface.Alloc(s_context.mem_iface.context, size, align, usage_flags, desc);
      if (item->memory_handle == NULL)
         goto error;
   }

   vcos_log_trace("%s item=%p", VCOS_FUNCTION, item);

   /* Insert after (dummy) head of alloc-list */
   if (s_context.alloc_list.next != NULL)
      s_context.alloc_list.next->prev = item;
   item->prev = &s_context.alloc_list;
   item->next = s_context.alloc_list.next;
   s_context.alloc_list.next = item;

   gmem_analyzer_alloc(item);

   return (gmem_handle_t)item;

error:
   if (item != NULL)
      free(item);

   return GMEM_HANDLE_INVALID;
}


gmem_handle_t gmem_alloc(size_t size, size_t align, gmem_usage_flags_t usage_flags, const char *desc)
{
   gmem_handle_t item;

   vcos_mutex_lock(&s_context.api_mutex);

   item = gmem_alloc_internal(size, align, usage_flags, desc);

   vcos_mutex_unlock(&s_context.api_mutex);

   return item;
}

/* Wrap a device offset and cached ptr in a gmem_handle */
gmem_handle_t gmem_from_external_memory(GMEM_TERM_T term, void *nativeSurface,
                                        uint64_t physOffset, void *cachedPtr,
                                        size_t length, const char *desc)
{
   gmem_alloc_item   *item = NULL;

   vcos_mutex_lock(&s_context.api_mutex);

   item = (gmem_alloc_item *)calloc(1, sizeof(gmem_alloc_item));
   if (item == NULL)
      goto error;

   vcos_log_trace("%s physOffset=0x%08X%08X, cachedPtr=%p, length=%d, desc='%s', item=%p", VCOS_FUNCTION,
                  (uint32_t)(physOffset >> 32), (uint32_t)(physOffset & 0xFFFFFFFF), cachedPtr,
                   length, desc ? desc : "<null>", item);

   item->memory_handle = 0;

   item->magic          = GMEM_HANDLE_MAGIC;
   item->size           = length;
   item->align          = 1;
   item->usage_flags    = GMEM_USAGE_ALL; /* Conservatively assume we need CPU and V3D access. */
   item->desc           = desc;
   item->nativeSurface  = nativeSurface;
    item->term          = term;

   item->driver_map_count     = 0;           /* This is pre-mapped, but not by the driver */
   item->cur_map_ptr          = cachedPtr;
   item->driver_lock_count    = 0;           /* This is pre-locked, but not by the driver */
   item->cur_lock_phys        = physOffset;
   item->externalAlloc        = true;
   item->freed                = false;

   /* Insert after (dummy) head of alloc-list */
   if (s_context.alloc_list.next != NULL)
      s_context.alloc_list.next->prev = item;
   item->prev = &s_context.alloc_list;
   item->next = s_context.alloc_list.next;
   s_context.alloc_list.next = item;

   gmem_analyzer_alloc(item);

   vcos_mutex_unlock(&s_context.api_mutex);

   return (gmem_handle_t)item;

error:
   vcos_mutex_unlock(&s_context.api_mutex);

   return GMEM_HANDLE_INVALID;
}

/* Free the item and its associated device memory only when its lock and map counts
 * have reached zero. */
static void lazy_free_item(gmem_alloc_item *item)
{
   if (!item->freed || item->driver_lock_count > 0 || item->driver_map_count > 0)
      return;

   if (item->usage_flags & GMEM_USAGE_HINT_DYNAMIC)
   {
      talloc_free(s_context.talloc, item->cur_map_ptr);
   }
   else if (!item->externalAlloc)
   {
      demand(s_context.mem_iface.Free != NULL);
      demand(s_context.mem_iface.Unmap != NULL);
      demand(s_context.mem_iface.Unlock != NULL);

      if (!s_context.disable_map_lock_opt && item->cur_map_ptr != NULL)
      {
         // Really unmap now
         s_context.mem_iface.Unmap(s_context.mem_iface.context, item->memory_handle, item->cur_map_ptr, item->size);
         item->cur_map_ptr = NULL;
      }

      if (!s_context.disable_map_lock_opt && item->cur_lock_phys != 0)
      {
         // Really unlock now
         s_context.mem_iface.Unlock(s_context.mem_iface.context, item->memory_handle);
         item->cur_lock_phys = 0;
      }

      // Really free now
      s_context.mem_iface.Free(s_context.mem_iface.context, item->memory_handle);
   }

   gmem_analyzer_free(item);

   /* Unlink from allocation list */
   item->prev->next = item->next;

   if (item->next != NULL)
      item->next->prev = item->prev;

   /* Just in case this memory gets reused as an item */
   item->magic = 0xFFEEFFEE;

   if (item->term)
      item->term(item->nativeSurface);

   free(item);
}

void gmem_free_internal(gmem_handle_t handle)
{
   gmem_alloc_item   *item  = (gmem_alloc_item *)handle;

   if (handle == GMEM_HANDLE_INVALID)
      return;

   demand(item->magic == GMEM_HANDLE_MAGIC);

   vcos_log_trace("%s handle=%p, item=%p, desc='%s', extAlloc=%d, memory_handle=%p",
                  VCOS_FUNCTION, handle, item, item->desc ? item->desc : "<null>",
                  item->externalAlloc, item->memory_handle);

   if (item->driver_lock_count > 0)
   {
      vcos_log_warn("********************* Still locked when freed (fix me!!!): %s, %p", item->desc, item);
      assert(item->driver_lock_count == 0);
   }

   if (item->driver_map_count > 0)
   {
      vcos_log_warn("********************* Still mapped when freed (fix me!!!): %s, %p", item->desc, item);
      assert(item->driver_map_count == 0);
   }

   if (item->freed)
      vcos_log_error("********************* gmem item freed more than once (fix me!!!): %s, %p", item->desc, item);

   item->freed = true;
   lazy_free_item(item);
}

void gmem_free(gmem_handle_t handle)
{
   vcos_mutex_lock(&s_context.api_mutex);

   gmem_free_internal(handle);

   vcos_mutex_unlock(&s_context.api_mutex);
}

void gmem_lock_list_init(struct gmem_lock_list *lock_list)
{
   assert(lock_list != NULL);

   lock_list->items = NULL;
   lock_list->size = 0;
   lock_list->capacity = 0;
   lock_list->bad = false;
}

#define LOCK_ITEM_INITIAL_ALLOC_SIZE 8

static bool gmem_grow_lock_list(struct gmem_lock_list *lock_list)
{
   unsigned int cap;

   if (lock_list->capacity == 0)
      cap = LOCK_ITEM_INITIAL_ALLOC_SIZE;
   else
      cap = lock_list->capacity * 2;

   lock_list->items = (gmem_lock_item*)realloc(lock_list->items, cap * sizeof(gmem_lock_item));
   if (lock_list->items == NULL)
      return false;

   lock_list->capacity = cap;

   return true;
}

v3d_addr_t gmem_lock_internal(struct gmem_lock_list *lock_list, gmem_handle_t handle,
                              bool take_mutex)
{
   gmem_alloc_item   *item;
   gmem_lock_item    *lock_item;
   v3d_addr_t        device_addr = 0;
   bool              need_real_lock = false;

   assert(lock_list != NULL);
   demand(s_context.mem_iface.Lock != NULL);

   if (take_mutex)
      vcos_mutex_lock(&s_context.api_mutex);

   /* Convert handle to item */
   item = (gmem_alloc_item *)handle;

   if (lock_list->size == lock_list->capacity)
   {
      if (!gmem_grow_lock_list(lock_list))
      {
         lock_list->bad = true;
         if (take_mutex)
            vcos_mutex_unlock(&s_context.api_mutex);
         return 0;
      }
   }

   lock_item = &lock_list->items[lock_list->size];
   lock_list->size++;

   /* Add to list */
   lock_item->handle = handle;

   if (s_context.disable_map_lock_opt)
   {
      // If the optimisation is disabled, lock and unlock on zero ref count
      if (item->driver_lock_count == 0)
         need_real_lock = true;
   }
   else
   {
      // Only lock if this is the first time we've been asked
      if (item->driver_lock_count == 0 && item->cur_lock_phys == 0)
         need_real_lock = true;
   }

   /* Get the device address */
   if (need_real_lock && !item->externalAlloc)
   {
      device_addr = (v3d_addr_t)s_context.mem_iface.Lock(s_context.mem_iface.context, item->memory_handle);
      item->cur_lock_phys = device_addr;
   }
   else
      device_addr = item->cur_lock_phys;

   item->driver_lock_count++;

   if (take_mutex)
      vcos_mutex_unlock(&s_context.api_mutex);

   return device_addr;
}

v3d_addr_t gmem_lock(struct gmem_lock_list *lock_list, gmem_handle_t handle)
{
   return gmem_lock_internal(lock_list, handle, true);
}

bool gmem_lock_list_is_bad(const struct gmem_lock_list *lock_list)
{
   return lock_list->bad;
}

void gmem_lock_list_unlock_and_destroy_internal(struct gmem_lock_list *lock_list, bool take_mutex)
{
   gmem_lock_item *lock_item      = NULL;

   demand(s_context.mem_iface.Unlock != NULL);

   if (take_mutex)
      vcos_mutex_lock(&s_context.api_mutex);

   for (unsigned int i = 0; i < lock_list->size; i++)
   {
      lock_item = &lock_list->items[i];
      gmem_alloc_item   *item = (gmem_alloc_item *)lock_item->handle;

      assert(item->driver_lock_count > 0);
      item->driver_lock_count--;

      if (item->driver_lock_count == 0 && s_context.disable_map_lock_opt && !item->externalAlloc)
      {
         s_context.mem_iface.Unlock(s_context.mem_iface.context, item->memory_handle);
         item->cur_lock_phys = 0;
      }

      if (item->driver_lock_count == 0)
         lazy_free_item(item);
   }

   /* Reset the list just in case someone decides to use it again */
   free(lock_list->items);
   lock_list->capacity = 0;
   lock_list->size = 0;
   lock_list->bad         = false;


   if (take_mutex)
      vcos_mutex_unlock(&s_context.api_mutex);
}

void gmem_lock_list_unlock_and_destroy(struct gmem_lock_list *lock_list)
{
   gmem_lock_list_unlock_and_destroy_internal(lock_list, true);
}

void *gmem_map_internal(gmem_handle_t handle, size_t offset, size_t length,
                        gmem_usage_flags_t usage_flags /* Some combination of GMEM_USAGE_CPU_READ/GMEM_USAGE_CPU_WRITE */, bool take_mutex)
{
   gmem_alloc_item   *item = gmem_validate_handle(handle);
   void              *addr = NULL;
   bool              need_real_map = false;

   demand(s_context.mem_iface.Map != NULL);

   if (take_mutex)
      vcos_mutex_lock(&s_context.api_mutex);

   if (s_context.disable_map_lock_opt)
   {
      // With the optimisation disabled, we map and unmap when ref count is zero
      if (item->driver_map_count == 0)
         need_real_map = true;
   }
   else
   {
      // Normally, we only map on first use, and then use the cached result forever
      if (item->cur_map_ptr == NULL && item->driver_map_count == 0)
         need_real_map = true;
   }

   if (need_real_map && !item->externalAlloc)
   {
      /* Always map the entire buffer so we can share the results among multiple map calls */
      addr = s_context.mem_iface.Map(s_context.mem_iface.context, item->memory_handle, 0, item->size, usage_flags);
      item->cur_map_ptr = addr;
   }
   else
      addr = item->cur_map_ptr;

   if (addr != NULL)
   {
      addr += offset;
      item->driver_map_count++;
   }

   if (take_mutex)
      vcos_mutex_unlock(&s_context.api_mutex);

   return addr;
}

void *gmem_map(gmem_handle_t handle)
{
   gmem_alloc_item *item = gmem_validate_handle(handle);
   return gmem_map_internal(handle, 0, item->size, item->usage_flags, true);
}

void gmem_unmap_internal(gmem_handle_t handle)
{
   gmem_alloc_item *item = gmem_validate_handle(handle);

   demand(s_context.mem_iface.Unmap != NULL);

   if (item->cur_map_ptr != NULL)
   {
      demand(item->driver_map_count > 0);

      item->driver_map_count--;

      if (s_context.disable_map_lock_opt && item->driver_map_count == 0 && !item->externalAlloc)
      {
         s_context.mem_iface.Unmap(s_context.mem_iface.context, item->memory_handle, item->cur_map_ptr, item->size);
         item->cur_map_ptr = NULL;
      }
   }

   if (item->driver_map_count == 0)
      lazy_free_item(item);
}

void gmem_unmap(gmem_handle_t handle)
{
   vcos_mutex_lock(&s_context.api_mutex);

   gmem_unmap_internal(handle);

   vcos_mutex_unlock(&s_context.api_mutex);
}

size_t gmem_get_sync_block_size(void)
{
   assert(s_context.sync_block_size > 0);
   return s_context.sync_block_size;
}

#ifndef NDEBUG
static inline size_t sync_block_start(size_t addr)
{
   return addr & ~(s_context.sync_block_size - 1u);
}
static inline size_t sync_block_end(size_t addr)
{
   return (addr + s_context.sync_block_size - 1u) & ~(s_context.sync_block_size - 1u);
}
#endif

static inline void gmem_validate_sync_range(gmem_handle_t handle, size_t start, size_t end, uint32_t sync_flags)
{
#ifndef NDEBUG
   gmem_alloc_item *item = gmem_validate_handle(handle);
#endif

   assert(start < end && end <= item->size);
   assert((sync_flags & GMEM_SYNC_RELAXED) || start == sync_block_start(start));
}

void gmem_validate_cpu_access(gmem_handle_t handle, uint32_t sync_flags)
{
#ifndef NDEBUG
   gmem_alloc_item *item = gmem_validate_handle(handle);
#endif

   // Must specify some sync-flags.
   assert((sync_flags & GMEM_SYNC_CPU) != 0);

   // V3D sync-flags not allowed.
   assert((sync_flags & GMEM_SYNC_V3D) == 0);

   // Discard only valid for write access.
   assert((sync_flags & (GMEM_SYNC_CPU_WRITE | GMEM_SYNC_DISCARD)) != GMEM_SYNC_DISCARD);

   // Check usage flags of handle.
   assert(!(sync_flags & GMEM_SYNC_CPU_READ) || (item->usage_flags & GMEM_USAGE_CPU_READ));
   assert(!(sync_flags & GMEM_SYNC_CPU_WRITE) || (item->usage_flags & GMEM_USAGE_CPU_WRITE));
}

void gmem_validate_cpu_access_range(gmem_handle_t handle, size_t offset, size_t length, uint32_t sync_flags)
{
   gmem_validate_cpu_access(handle, sync_flags);
   gmem_validate_sync_range(handle, offset, offset + length, sync_flags);
}

void gmem_validate_v3d_access(gmem_handle_t handle, uint32_t sync_flags)
{
#ifndef NDEBUG
   gmem_alloc_item *item = gmem_validate_handle(handle);
#endif

   // Must specify some sync flags.
   assert((sync_flags & GMEM_SYNC_V3D) != 0);

   // CPU sync-flags not allowed.
   assert((sync_flags & GMEM_SYNC_CPU) == 0);

   // Discard only valid for write access.
   assert((sync_flags & (GMEM_SYNC_V3D_WRITE | GMEM_SYNC_DISCARD)) != GMEM_SYNC_DISCARD);

   // Check V3D usage flags.
   assert(!(sync_flags & GMEM_SYNC_V3D_READ) || (item->usage_flags & GMEM_USAGE_V3D_READ));
   assert(!(sync_flags & GMEM_SYNC_V3D_WRITE) || (item->usage_flags & GMEM_USAGE_V3D_WRITE));
}

void gmem_validate_v3d_access_range(gmem_handle_t handle, size_t offset, size_t length, uint32_t sync_flags)
{
   gmem_validate_v3d_access(handle, sync_flags);
   gmem_validate_sync_range(handle, offset, offset + length, sync_flags);
}

/* CPU access functions */

void gmem_cpu_sync_list_init(struct gmem_cpu_sync_list *sync_list)
{
   LIST_INIT(&sync_list->list);
   sync_list->flags = 0;
}

void gmem_cpu_sync_list_add(struct gmem_cpu_sync_list *sync_list,
   gmem_handle_t handle, gmem_sync_flags_t sync_flags)
{
   gmem_alloc_item *item = gmem_validate_handle(handle);
   gmem_cpu_sync_list_add_range(sync_list, handle, 0, item->size, sync_flags);
}

void gmem_cpu_sync_list_add_range(struct gmem_cpu_sync_list *sync_list,
   gmem_handle_t handle, size_t offset, size_t length, gmem_sync_flags_t sync_flags)
{
   debug_only(gmem_validate_cpu_access_range(handle, offset, length, sync_flags));

   // accumulate flags in sync list
   sync_list->flags |= sync_flags;

   // No need to sync buffers that are not written by V3D.
   gmem_alloc_item *item = (gmem_alloc_item *)handle;
   if ((item->usage_flags & GMEM_USAGE_V3D_WRITE) && !(sync_flags & GMEM_SYNC_DISCARD))
   {
      gmem_abstract_cpu_sync_block_t *b =
         (gmem_abstract_cpu_sync_block_t *)malloc(sizeof(gmem_abstract_cpu_sync_block_t));

      b->handle = handle;
      b->sync_flags = sync_flags;
      b->offset = offset;
      b->length = length;

      LIST_INSERT_HEAD(&sync_list->list, b, link);
   }
}

void gmem_cpu_sync_list_destroy(struct gmem_cpu_sync_list *sync_list)
{
   gmem_abstract_cpu_sync_block_t *next = NULL;

   for (gmem_abstract_cpu_sync_block_t *b = LIST_FIRST(&sync_list->list); b; b = next) {
      next = LIST_NEXT(b, link);
      free(b);
   }
}

void gmem_sync_pre_cpu_access_list(struct gmem_cpu_sync_list *sync_list)
{
   assert(s_context.mem_iface.FlushCache != NULL);

   size_t sync_total = 0;
   for (gmem_abstract_cpu_sync_block_t *b = LIST_FIRST(&sync_list->list); b; b = LIST_NEXT(b, link))
   {
      gmem_alloc_item* item = b->handle;
      if (__atomic_load_n(&item->v3d_writes, __ATOMIC_RELAXED))
      {
         sync_total += b->length;

         if (  (sync_total > 3 * 1024 * 1024)
            || (item->driver_map_count == 0 && !item->externalAlloc)) // unmapped
         {
            s_context.mem_iface.FlushCache(s_context.mem_iface.context, NULL, NULL, ~0);
            return;
         }
      }
   }

   if (sync_total > 0)
   {
      for (gmem_abstract_cpu_sync_block_t *b = LIST_FIRST(&sync_list->list); b; b = LIST_NEXT(b, link))
      {
         gmem_alloc_item* item = b->handle;
         if (__atomic_load_n(&item->v3d_writes, __ATOMIC_RELAXED))
         {
            s_context.mem_iface.FlushCache(s_context.mem_iface.context, item->memory_handle, item->cur_map_ptr + b->offset, b->length);
         }
      }
   }
}

void gmem_sync_pre_cpu_access(gmem_handle_t handle, gmem_sync_flags_t sync_flags)
{
   gmem_alloc_item *item = gmem_validate_handle(handle);
   gmem_sync_pre_cpu_access_range(handle, 0, item->size, sync_flags);
}

void gmem_sync_pre_cpu_access_range(
   gmem_handle_t handle,
   size_t offset,
   size_t length,
   gmem_sync_flags_t sync_flags
)
{
   debug_only(gmem_validate_cpu_access_range(handle, offset, length, sync_flags));

   // No need to sync buffers that are not written by V3D.
   gmem_alloc_item *item = handle;
   if (__atomic_load_n(&item->v3d_writes, __ATOMIC_RELAXED) && !(sync_flags & GMEM_SYNC_DISCARD))
   {
      assert(s_context.mem_iface.FlushCache != NULL);

      bool unmapped_buffers = item->driver_map_count == 0 && !item->externalAlloc;
      if (length > 3 * 1024 * 1024 || unmapped_buffers)
      {
         s_context.mem_iface.FlushCache(s_context.mem_iface.context, NULL, NULL, ~0);
      }
      else
      {
         s_context.mem_iface.FlushCache(s_context.mem_iface.context, item->memory_handle, item->cur_map_ptr + offset, length);
      }
   }
}

void gmem_print_level(VCOS_LOG_LEVEL_T level)
{
   uint32_t used   = 0;
   uint32_t mapped = 0;
   uint32_t locked = 0;

   if (!vcos_is_log_enabled(VCOS_LOG_CATEGORY, level))
      return;

   _VCOS_LOG_X(VCOS_LOG_CATEGORY, level, "GMEM SUMMARY (note: buffers currently on display may not be included):");

   gmem_alloc_item *item = s_context.alloc_list.next;
   while (item != NULL)
   {
      used += item->size;
      if (item->driver_map_count > 0)
         mapped += item->size;
      if (item->driver_lock_count > 0)
         locked += item->size;

      _VCOS_LOG_X(VCOS_LOG_CATEGORY, level, "handle = %p, cur_lock_phys = 0x%08x, driver_map_count = %u, extAlloc=%d, size = %u, desc = '%s'%s",
                  item, item->cur_lock_phys, item->driver_map_count, item->externalAlloc, item->size,
                  item->desc ? item->desc : "<null>", item->next ? "" : "\n");

      item = item->next;
   }

   _VCOS_LOG_X(VCOS_LOG_CATEGORY, level, "Memory in use : %d, Memory locked : %d, Memory Mapped %d\n", used, locked, mapped);

   if (s_context.mem_iface.GetInfo)
      s_context.mem_iface.GetInfo(s_context.mem_iface.context, BEGL_MemPrintHeapData);
}

void gmem_print(void)
{
   gmem_print_level(VCOS_LOG_ERROR);
}

size_t gmem_get_size(gmem_handle_t handle)
{
   gmem_alloc_item   *item  = (gmem_alloc_item *)handle;

   assert(item != NULL);

   return item->size;
}

gmem_usage_flags_t gmem_get_usage(gmem_handle_t handle)
{
   gmem_alloc_item   *item  = (gmem_alloc_item *)handle;

   assert(item != NULL);

   return item->usage_flags;
}

void *gmem_addr_to_ptr(v3d_addr_t addr)
{
   assert(0);
   return NULL;
}

v3d_addr_t gmem_ptr_to_addr(void *p)
{
   assert(0);
   return 0;
}
