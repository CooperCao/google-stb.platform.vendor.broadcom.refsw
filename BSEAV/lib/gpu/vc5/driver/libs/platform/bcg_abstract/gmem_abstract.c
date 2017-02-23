/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.
=============================================================================*/

#include "gmem_abstract.h"
#include "gmem_talloc.h"
#include "gmem_analyzer.h"
#include "vcos.h"
#include "vcos_mutex.h"
#include "libs/util/demand.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/v3d/v3d_limits.h"
#include <stdlib.h>

LOG_DEFAULT_CAT("gmem")

static struct
{
   VCOS_MUTEX_T         api_mutex;
   gmem_alloc_item      alloc_list;
   BEGL_MemoryInterface mem_iface;
   void*                talloc;
   v3d_size_t           non_coherent_atom_size;
} s_context;


/* Memory interface wrappers */

static inline BEGL_MemHandle mem_interface_alloc(size_t size, size_t alignment, uint32_t flags, const char *desc)
{
   assert(s_context.mem_iface.Alloc != NULL);
   return s_context.mem_iface.Alloc(s_context.mem_iface.context, size, alignment, flags, desc);
}

static inline void mem_interface_free(BEGL_MemHandle h)
{
   assert(s_context.mem_iface.Free != NULL);
   s_context.mem_iface.Free(s_context.mem_iface.context, h);
}

static inline void* mem_interface_map(BEGL_MemHandle h, size_t offset, size_t length, uint32_t usage_flags)
{
   assert(s_context.mem_iface.Map != NULL);
   return s_context.mem_iface.Map(s_context.mem_iface.context, h, offset, length, usage_flags);
}

static inline void mem_interface_unmap(BEGL_MemHandle h, void *cpu_ptr, size_t length)
{
   assert(s_context.mem_iface.Unmap != NULL);
   s_context.mem_iface.Unmap(s_context.mem_iface.context, h, cpu_ptr, length);
}

static inline v3d_addr_t mem_interface_lock(BEGL_MemHandle h)
{
   assert(s_context.mem_iface.Lock != NULL);
   return (v3d_addr_t)s_context.mem_iface.Lock(s_context.mem_iface.context, h);
}

static inline void mem_interface_unlock(BEGL_MemHandle h)
{
   assert(s_context.mem_iface.Unlock != NULL);
   return s_context.mem_iface.Unlock(s_context.mem_iface.context, h);
}

static inline void mem_interface_flush_cache(BEGL_MemHandle h, void *cpu_ptr, size_t length)
{
   assert(s_context.mem_iface.FlushCache != NULL);
   s_context.mem_iface.FlushCache(s_context.mem_iface.context, h, cpu_ptr, length);
}

static inline bool mem_interface_has_get_info(void)
{
   return s_context.mem_iface.GetInfo != NULL;
}

static inline uint64_t mem_interface_get_info(BEGL_MemInfoType type)
{
   assert(s_context.mem_iface.GetInfo != NULL);
   return s_context.mem_iface.GetInfo(s_context.mem_iface.context, type);
}

static inline bool mem_interface_has_wrap_external(void)
{
   return s_context.mem_iface.WrapExternal != NULL;
}

static inline BEGL_MemHandle mem_interface_wrap_external(uint64_t physaddr, size_t length, const char *desc)
{
   assert(s_context.mem_iface.WrapExternal != NULL);
   return s_context.mem_iface.WrapExternal(s_context.mem_iface.context, physaddr, length, desc);
}

__attribute__((visibility("default")))
void BEGL_RegisterMemoryInterface(BEGL_MemoryInterface *iface)
{
   log_trace(VCOS_FUNCTION);

   if (iface != NULL)
   {
      s_context.mem_iface = *iface;                                    /* Register   */

      uint32_t cpuCacheLine = 512;  /* Safe value */

      /* Get the real value if possible */
      if (mem_interface_has_get_info())
         cpuCacheLine = (uint32_t)mem_interface_get_info(BEGL_MemCacheLineSize);

      s_context.non_coherent_atom_size = vcos_max(V3D_MAX_CACHE_LINE_SIZE, cpuCacheLine);

      demand(s_context.mem_iface.Alloc != NULL);
      demand(s_context.mem_iface.Free != NULL);
      demand(s_context.mem_iface.Map != NULL);
      demand(s_context.mem_iface.Unmap != NULL);
      demand(s_context.mem_iface.Lock != NULL);
      demand(s_context.mem_iface.Unlock != NULL);
      demand(s_context.mem_iface.FlushCache != NULL);
   }
   else
   {
      memset(&s_context.mem_iface, 0, sizeof(BEGL_MemoryInterface));   /* Unregister */
      s_context.non_coherent_atom_size = 0;
   }
}

bool gmem_init(void)
{
   log_trace(VCOS_FUNCTION);

   demand_msg(s_context.non_coherent_atom_size != 0, "Memory interface not registered");
   demand(vcos_mutex_create(&s_context.api_mutex, "gmem mutex") == VCOS_SUCCESS);
   vcos_mutex_lock(&s_context.api_mutex);

   gmem_analyzer_init();

   s_context.talloc = talloc_initialize();

   vcos_mutex_unlock(&s_context.api_mutex);
   return true;
}

void gmem_destroy(void)
{
   log_trace(VCOS_FUNCTION);

   // Talloc thread shares the gmem lock and is still running.
   vcos_mutex_lock(&s_context.api_mutex);

   bool leaked = false;
   for (unsigned pass2 = 0; pass2 <= 1; ++pass2)
   {
      /* initial block is NULL block, everything else is inserted after this.  Check for
         additional linked blocks */
      for (gmem_alloc_item *item = s_context.alloc_list.next; item != NULL; )
      {
         /* save next ptr before its deleted */
         gmem_alloc_item *next = item->next;

         /* Process talloc allocations on first pass. */
         if (pass2 || item->type == GMEM_ALLOC_TALLOC)
         {
            leaked = true;

            log_error("** LEAKAGE: desc='%s', handle=%p, addr=0x%08x, size=%u\n",
                        item->desc ? item->desc : "", item, item->v3d_addr, item->size);

            /* free this handle */
            gmem_free_internal(item);
         }

         item = next;
      }

      /* Now safe to free talloc. */
      if (s_context.talloc)
      {
         talloc_term(s_context.talloc);
         s_context.talloc = NULL; /* Not a valid pointer after talloc_term */
      }
   }

   gmem_analyzer_term();

   vcos_mutex_unlock(&s_context.api_mutex);
   vcos_mutex_delete(&s_context.api_mutex);

   assert(!leaked);
}

void gmem_mutex_lock_internal(void)
{
   vcos_mutex_lock(&s_context.api_mutex);
}

void gmem_mutex_unlock_internal(void)
{
   vcos_mutex_unlock(&s_context.api_mutex);
}

gmem_alloc_item* gmem_alloc_internal(size_t size, v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc)
{
   assert(gfx_size_is_power_of_2(align));

   log_trace("%s size=%zd, align=%zd, usage=0x%x, desc='%s'", VCOS_FUNCTION, size, (size_t)align, usage_flags, desc ? desc : "<null>");

   gmem_alloc_item* item = (gmem_alloc_item *)calloc(1, sizeof(gmem_alloc_item));
   if (item == NULL)
      goto error;

#ifndef NDEBUG
   item->magic = GMEM_HANDLE_MAGIC;
#endif
   item->size = size;
   item->desc = desc;
   item->usage_flags = usage_flags;
   item->type = GMEM_ALLOC_REGULAR;

   /* Round align/size up to be a multiple of sync-block size */
   align = gfx_zround_up_p2(align, s_context.non_coherent_atom_size);
   size = gfx_zround_up_p2(size, s_context.non_coherent_atom_size);

   if (s_context.talloc && (usage_flags & GMEM_USAGE_HINT_DYNAMIC) && !(usage_flags & GMEM_USAGE_SECURE))
   {
      if (talloc_alloc(s_context.talloc, size, align, &item->cpu_ptr, &item->v3d_addr))
      {
         // Talloc pollutes the CPU cache with lines from these allocations so
         // ensure they are flushed from the cache before any V3D writes.
         // Ideally we'd use a non-intrusive heap so this isn't necessary.
         if (usage_flags & GMEM_USAGE_V3D_WRITE)
            mem_interface_flush_cache(NULL, item->cpu_ptr, size);

         item->type = GMEM_ALLOC_TALLOC;
      }
   }

   if (item->type == GMEM_ALLOC_REGULAR)
   {
      /* Fill in item */
      item->memory_handle = mem_interface_alloc(size, align, usage_flags, desc);
      if (item->memory_handle == NULL)
         goto error;

      item->v3d_addr = mem_interface_lock(item->memory_handle);
      if (!item->v3d_addr)
         goto error;

      log_trace("%s addr=0x%08x", VCOS_FUNCTION, item->v3d_addr);
   }

   log_trace("%s item=%p", VCOS_FUNCTION, item);

   /* Insert after (dummy) head of alloc-list */
   if (s_context.alloc_list.next != NULL)
      s_context.alloc_list.next->prev = item;
   item->prev = &s_context.alloc_list;
   item->next = s_context.alloc_list.next;
   s_context.alloc_list.next = item;

   gmem_analyzer_alloc(item);

   return (gmem_handle_t)item;

error:
   if (item->memory_handle != NULL)
      mem_interface_free(item->memory_handle);

   if (item != NULL)
      free(item);

   return GMEM_HANDLE_INVALID;
}

gmem_alloc_item* gmem_alloc_and_map_internal(size_t size, v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc)
{
   gmem_alloc_item* item = gmem_alloc_internal(size, align, usage_flags, desc);
   if (!item)
      return NULL;

   if (!item->cpu_ptr)
   {
      assert(item->type == GMEM_ALLOC_REGULAR);
      item->cpu_ptr = mem_interface_map(item->memory_handle, 0, item->size, item->usage_flags);
      if (!item->cpu_ptr)
      {
         gmem_free_internal(item);
         return NULL;
      }
   }

   return item;
}


gmem_handle_t gmem_alloc(size_t size, v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc)
{
   vcos_mutex_lock(&s_context.api_mutex);

   gmem_alloc_item* item = gmem_alloc_internal(size, align, usage_flags, desc);

   vcos_mutex_unlock(&s_context.api_mutex);

   return item;
}

gmem_handle_t gmem_alloc_and_map(size_t size, v3d_size_t align, gmem_usage_flags_t usage_flags, const char *desc)
{
   vcos_mutex_lock(&s_context.api_mutex);

   gmem_alloc_item* item = gmem_alloc_and_map_internal(size, align, usage_flags, desc);

   vcos_mutex_unlock(&s_context.api_mutex);

   return item;
}

/* Wrap a device offset and cached ptr in a gmem_handle */
gmem_handle_t gmem_from_external_memory(GMEM_TERM_T external_term, void *external_context,
                                        uint64_t physOffset, void *cpu_ptr,
                                        size_t length, const char *desc)
{
   gmem_alloc_item   *item = NULL;

   vcos_mutex_lock(&s_context.api_mutex);

   item = (gmem_alloc_item *)calloc(1, sizeof(gmem_alloc_item));
   if (item == NULL)
      goto error;

#ifndef NDEBUG
   item->magic             = GMEM_HANDLE_MAGIC;
#endif
   item->size              = length;
   item->desc              = desc;
   item->external_context  = external_context;
   item->external_term     = external_term;
   item->cpu_ptr           = cpu_ptr;

   if (mem_interface_has_wrap_external())
   {
      item->memory_handle = mem_interface_wrap_external(physOffset, length, desc);
      if (!item->memory_handle)
         goto error;

      item->v3d_addr = mem_interface_lock(item->memory_handle);
      if (!item->v3d_addr)
         goto error;
   }
   else
   {
      item->v3d_addr = physOffset;
      item->memory_handle = 0;
   }

   item->type = GMEM_ALLOC_EXTERNAL;

   /* Test for a secure buffer */
   item->usage_flags = GMEM_USAGE_V3D_RW | (cpu_ptr == NULL ? GMEM_USAGE_SECURE : 0);

   log_trace("%s physOffset=0x%08X%08X, cpu_ptr=%p, length=%zd, desc='%s', item=%p, usage=0x%x", VCOS_FUNCTION,
                  (uint32_t)(physOffset >> 32), (uint32_t)(physOffset & 0xFFFFFFFF), cpu_ptr,
                   length, desc ? desc : "<null>", item, item->usage_flags);

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
   if (item->memory_handle)
      mem_interface_free(item->memory_handle);
   free(item);

   vcos_mutex_unlock(&s_context.api_mutex);

   return GMEM_HANDLE_INVALID;
}

void gmem_free_internal(gmem_handle_t handle)
{
   if (handle == GMEM_HANDLE_INVALID)
      return;

   gmem_alloc_item *item = gmem_validate_handle(handle);

   log_trace("%s handle=%p, item=%p, desc='%s', type=%d, memory_handle=%p",
                  VCOS_FUNCTION, handle, item, item->desc ? item->desc : "<null>",
                  item->type, item->memory_handle);

   switch (item->type)
   {
   case GMEM_ALLOC_REGULAR:
      if (item->cpu_ptr)
         mem_interface_unmap(item->memory_handle, item->cpu_ptr, item->size);
      mem_interface_unlock(item->memory_handle);
      mem_interface_free(item->memory_handle);
      break;

   case GMEM_ALLOC_TALLOC:
      talloc_free(s_context.talloc, item->cpu_ptr);
      break;

   case GMEM_ALLOC_EXTERNAL:
      if (item->memory_handle != NULL)
      {
         /*
          * This is a wrapped external memory object, so we still need to free it.
          *
          * We locked it when we wrapped the memory to get the physical,
          * so unlock it, although this will probably always be a nop.
          */
         mem_interface_unlock(item->memory_handle);
         mem_interface_free(item->memory_handle);
      }
      break;
   default:
      unreachable();
   }

   gmem_analyzer_free(item);

   /* Unlink from allocation list */
   item->prev->next = item->next;

   if (item->next != NULL)
      item->next->prev = item->prev;

   /* Just in case this memory gets reused as an item */
#ifndef NDEBUG
   item->magic = 0xFFEEFFEE;
#endif

   if (item->external_term)
      item->external_term(item->external_context);

   free(item);
}

void gmem_free(gmem_handle_t handle)
{
   vcos_mutex_lock(&s_context.api_mutex);

   gmem_free_internal(handle);

   vcos_mutex_unlock(&s_context.api_mutex);
}

void* gmem_map_and_get_ptr_internal(gmem_alloc_item* item)
{
   vcos_mutex_lock(&s_context.api_mutex);

   // Only regular allocations should need mapping.
   switch (item->type)
   {
   case GMEM_ALLOC_REGULAR:
      break;
   case GMEM_ALLOC_TALLOC:
   case GMEM_ALLOC_EXTERNAL:
      unreachable();
   }

   // Only map the pointer for real now we have the gmem mutex.
   void* cpu_ptr = item->cpu_ptr;
   if (!cpu_ptr)
   {
      cpu_ptr = mem_interface_map(item->memory_handle, 0, item->size, item->usage_flags);

      // Use atomic store so that another thread calling gmem_map_and_get_ptr
      // will not see partial write to item->cpu_ptr.
      vcos_atomic_store_ptr(&item->cpu_ptr, cpu_ptr, VCOS_MEMORY_ORDER_RELEASE);
   }

   vcos_mutex_unlock(&s_context.api_mutex);

   return cpu_ptr;
}

v3d_size_t gmem_non_coherent_atom_size(void)
{
   assert(s_context.non_coherent_atom_size > 0);
   return s_context.non_coherent_atom_size;
}

static inline void gmem_validate_cpu_access_range(gmem_handle_t handle, v3d_size_t offset, v3d_size_t length)
{
#ifndef NDEBUG
   gmem_alloc_item *item = gmem_validate_handle(handle);
   v3d_size_t end = offset + length;
   assert(end >= offset);
   assert(end <= item->size);
#endif
}

/* CPU access functions */

void gmem_invalidate_mapped_buffer(gmem_handle_t handle)
{
   gmem_alloc_item *item = gmem_validate_handle(handle);
   gmem_invalidate_mapped_range(handle, 0, item->size);
}

void gmem_flush_mapped_buffer(gmem_handle_t handle)
{
   gmem_alloc_item *item = gmem_validate_handle(handle);
   gmem_flush_mapped_range(handle, 0, item->size);
}

void gmem_invalidate_mapped_range(
   gmem_handle_t handle,
   v3d_size_t offset,
   v3d_size_t length)
{
   gmem_validate_cpu_access_range(handle, offset, length);

   // Only sync mapped buffers written by V3D.
   gmem_alloc_item *item = handle;
   assert(item->cpu_ptr != NULL);

   if (item->usage_flags & GMEM_USAGE_V3D_WRITE)
   {
      mem_interface_flush_cache(
         item->memory_handle,
         (uint8_t *)item->cpu_ptr + offset,
         length);
   }
}

void gmem_flush_mapped_range(
   gmem_handle_t handle,
   v3d_size_t offset,
   v3d_size_t length)
{
   gmem_validate_cpu_access_range(handle, offset, length);

   // Only flush mapped buffers written by CPU.
   gmem_alloc_item *item = (gmem_alloc_item *)handle;
   assert(item->cpu_ptr != NULL);

   mem_interface_flush_cache(
      item->memory_handle,
      (uint8_t *)item->cpu_ptr + offset,
      length);
}

gmem_handle_t gmem_find_handle_by_addr(v3d_addr_t addr)
{
   vcos_mutex_lock(&s_context.api_mutex);

   gmem_alloc_item* item;
   for (item = s_context.alloc_list.next; item != NULL; item = item->next)
   {
      if (addr >= item->v3d_addr && addr < item->v3d_addr + item->size)
         break;
   }

   vcos_mutex_unlock(&s_context.api_mutex);

   return item;
}

void gmem_print_level(log_level_t level)
{
   if (!log_enabled(level))
      return;

   log_msg(level, "GMEM SUMMARY (note: buffers currently on display may not be included):");

   vcos_mutex_lock(&s_context.api_mutex);

   size_t used   = 0;
   size_t mapped = 0;

   for (gmem_alloc_item *item = s_context.alloc_list.next; item != NULL; item = item->next)
   {
      switch (item->type)
      {
      case GMEM_ALLOC_REGULAR:
      case GMEM_ALLOC_EXTERNAL:
         used += item->size;
         if (item->cpu_ptr != NULL)
            mapped += item->size;
         break;
      case GMEM_ALLOC_TALLOC:
         // Don't include duplicates!
         break;
      default:
         unreachable();
      }

      log_msg(level, "handle=%p, addr=0x%08x, mapped=%u, type=%d, size=%u, desc='%s'%s",
                  item, item->v3d_addr, item->cpu_ptr ? 1u : 0u, item->type, item->size,
                  item->desc ? item->desc : "<null>", item->next ? "" : "\n");
   }

   log_msg(level, "Memory in use : %zu, Memory Mapped %zu\n", used, mapped);

   if (mem_interface_has_get_info())
      mem_interface_get_info(BEGL_MemPrintHeapData);

   vcos_mutex_unlock(&s_context.api_mutex);
}

void gmem_print(void)
{
   gmem_print_level(LOG_ERROR);
}

uint64_t gmem_get_pagetable_physical_addr(void)
{
   if (!mem_interface_has_get_info())
      return 0;
   return mem_interface_get_info(BEGL_MemPagetablePhysAddr);
}

v3d_addr_t gmem_get_mmu_max_virtual_addr(void)
{
   if (!mem_interface_has_get_info())
      return 0;
   return (v3d_addr_t)mem_interface_get_info(BEGL_MemMmuMaxVirtAddr);
}

int64_t gmem_get_mmu_unsecure_bin_translation(void)
{
   if (!mem_interface_has_get_info())
      return 0;
   return (int64_t)mem_interface_get_info(BEGL_MemMmuUnsecureBinTranslation);
}

int64_t gmem_get_mmu_secure_bin_translation(void)
{
   if (!mem_interface_has_get_info())
      return 0;
   return (int64_t)mem_interface_get_info(BEGL_MemMmuSecureBinTranslation);
}

uint64_t gmem_get_platform_token(void)
{
   if (!mem_interface_has_get_info())
      return 0;
   return mem_interface_get_info(BEGL_MemPlatformToken);
}
