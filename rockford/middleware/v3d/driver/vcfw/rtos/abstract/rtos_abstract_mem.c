/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Memory management
File     :  $RCSfile$
Revision :  $Revision$

FILE DESCRIPTION
Implementation of memory management API.
=============================================================================*/

#include <sys/types.h>
#include <stdarg.h>

#include "interface/vcos/vcos.h"

#include "rtos_abstract_mem.h"

#include "interface/khronos/include/EGL/begl_memplatform.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_hw.h"           /* For stats recording */
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#ifdef BCG_VC4_DEFRAG
#include "middleware/khronos/glxx/glxx_server.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/******************************************************************************
helpers
******************************************************************************/

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

/******************************************************************************
manager
******************************************************************************/

#if defined(BCG_VC4_DEFRAG) || !defined(NDEBUG)

/* Used to record allocations */
typedef struct MEM_HEADER_LIST_S
{
   MEM_HEADER_T               *header;
   struct MEM_HEADER_LIST_S   *next;
} MEM_HEADER_LIST_T;

#endif

/* The manager itself */
typedef struct {

   BEGL_MemoryInterface *memInterface;
   VCOS_MUTEX_T          mh;

#if defined(BCG_VC4_DEFRAG) || !defined(NDEBUG)
   MEM_HEADER_LIST_T     *deviceAllocations;
#endif

} MEM_MANAGER_T;

/* C standard says this is auto initialized to zero */
static MEM_MANAGER_T g_mgr;

MEM_HANDLE_T MEM_ZERO_SIZE_HANDLE;
MEM_HANDLE_T MEM_EMPTY_STRING_HANDLE;

/******************************************************************************
pool management api
******************************************************************************/

/*
Initialize the memory subsystem, allocating a pool of a given size and
with space for the given number of handles.
*/

int mem_init(void * memInterface)
{
   /* should this take a copy? */
   g_mgr.memInterface = memInterface;

   vcos_mutex_create((VCOS_MUTEX_T *)&g_mgr.mh, "mem_init");

   /* allocate special handles */
   MEM_ZERO_SIZE_HANDLE = mem_alloc(0, 4, MEM_FLAG_NONE, "zero size");
   MEM_EMPTY_STRING_HANDLE = mem_strdup("");

#if defined(BCG_VC4_DEFRAG) || !defined(NDEBUG)
   g_mgr.deviceAllocations = NULL;
#endif

   return 1;
}

uint32_t mem_map_cached_to_physical(void *virt)
{
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->ConvertCachedToPhysical != NULL)
      return g_mgr.memInterface->ConvertCachedToPhysical(g_mgr.memInterface->context, virt);
   else
      return 0;
}

void *mem_map_physical_to_cached(uint32_t phys)
{
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->ConvertPhysicalToCached)
      return g_mgr.memInterface->ConvertPhysicalToCached(g_mgr.memInterface->context, phys);
   else
      return NULL;
}

void mem_flush_cache(void)
{
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->FlushCache != NULL)
      g_mgr.memInterface->FlushCache(g_mgr.memInterface->context, 0, 0xFFFFFFFF);
}

void mem_flush_cache_range(void * p, size_t numBytes)
{
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->FlushCache != NULL)
      g_mgr.memInterface->FlushCache(g_mgr.memInterface->context, p, numBytes);
}

#ifndef NDEBUG
static void mem_hdr_list_dump(MEM_HEADER_LIST_T *list)
{
   MEM_HEADER_LIST_T *ptr = list;

   for (; ptr != NULL; ptr = ptr->next)
   {
      MEM_HEADER_T   *h = ptr->header;

      printf("====================\n");

      if (h != NULL)
      {
         printf("handle    : %p\n", h);
         printf("ptr       : %p\n", h->ptr);
         printf("alloc size: %d\n", h->allocedSize);
         printf("size      : %d\n", h->size);
         printf("align     : %d\n", h->align);
         printf("term      : %p\n", h->term);
         printf("desc      : %s\n", h->desc);
         printf("ref count : %d\n", h->ref_count);
#ifdef BCG_VC4_DEFRAG
         printf("lock count: %d\n", h->lock_count);
#endif
         printf("flags     : %x\n", h->flags);
      }
      else
      {
         printf("NULL\n");
      }
   }
}
#endif /* NDEBUG */

/*
Terminate the memory subsystem, releasing the pool.
*/

void mem_term(void)
{
#ifndef NDEBUG
   if (g_mgr.deviceAllocations != NULL)
   {
      /* Looks like we might have some memory leaks */
      printf("MEMORY LEAKS DETECTED:\n");
      mem_hdr_list_dump(g_mgr.deviceAllocations);
   }
#endif

   vcos_mutex_delete((VCOS_MUTEX_T *)&g_mgr.mh);

   mem_release(MEM_ZERO_SIZE_HANDLE);
   mem_release(MEM_EMPTY_STRING_HANDLE);
}

/*
The heap is compacted to the maximum possible extent. If (mode & MEM_COMPACT_DISCARD)
is non-zero, all discardable, unlocked, and unretained MEM_HANDLE_Ts are resized to size 0.
If (mode & MEM_COMPACT_AGGRESSIVE) is non-zero, all long-term block owners (which are
obliged to have registered a callback) are asked to unlock their blocks for the duration
of the compaction.
*/

void mem_compact(mem_compact_mode_t mode)
{
   UNUSED(mode);
   assert(0);
}

/******************************************************************************
core api
******************************************************************************/

/* Zepher has a 256kb L2 with 128byte line size - all allocations should therefore be a multiple of 128bytes and aligned to 128byte boundary (minimum) */
#define CACHELINE_SIZE 128

uint32_t mem_cacheline_size(void)
{
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->GetInfo != NULL)
      return g_mgr.memInterface->GetInfo(g_mgr.memInterface->context, BEGL_MemCacheLineSize);
   else
      return CACHELINE_SIZE;
}

#ifdef BCG_VC4_DEFRAG
/* MEM_HEADER_LIST_T functions
*
* The mem-header-list is used to record memory allocations so that they can be
* defragmented if necessary when an allocation fails
*/

/* mem_hdr_list_length
*
* Calculate the length of a header list
*
* Use only within a mutex.
*/
static uint32_t mem_hdr_list_length(MEM_HEADER_LIST_T *list)
{
   uint32_t          l = 0;
   MEM_HEADER_LIST_T *ptr = list;

   for (; ptr != NULL; ptr = ptr->next)
      l++;

   return l;
}

/* mem_hdr_list_free
*
* Frees all the elements in the list and NULLs the list
*
* Use only within a mutex.
*/
static void mem_hdr_list_free(MEM_HEADER_LIST_T **list)
{
   MEM_HEADER_LIST_T *ptr = *list;
   MEM_HEADER_LIST_T *next = NULL;

   for (; ptr != NULL; ptr = next)
   {
      next = ptr->next;
      free(ptr);
   }

   *list = NULL;
}

/* mem_hdr_list_insert
*
* Inserts a header in front of the first list item with a greater pointer value
* Can be used to implement an insert-sort.
* If the malloc for the new item fails, no insertion takes place.
*
* Use only within a mutex.
*/
static void mem_hdr_list_insert(MEM_HEADER_LIST_T **list, MEM_HEADER_T *newHdr)
{
   MEM_HEADER_LIST_T *newNode = NULL;

   if (newHdr == NULL)
      return;

   newNode = (MEM_HEADER_LIST_T *)malloc(sizeof(MEM_HEADER_LIST_T));
   if (newNode == NULL)
      return;
   newNode->header = newHdr;

   if (*list != NULL)
   {
      /* lower address is inserted prior to match in list */
      MEM_HEADER_LIST_T *prev, *ptr;
      bool              found = false;

      for (ptr = *list, prev = NULL; !found && ptr != NULL; prev = ptr, ptr = ptr->next)
      {
         MEM_HEADER_T   *hdr = ptr->header;

         if (hdr && hdr->blockOffset > newHdr->blockOffset)
         {
            found = true;
            break;
         }
      }

      if (found)
      {
         /* insert after a valid node */
         if (prev)
         {
            newNode->next = ptr;
            prev->next = newNode;
         }
         else
         {
            /* insert at the head */
            newNode->next = ptr;
            *list = newNode;
         }
      }
      else
      {
         /* insert at the end */
         newNode->next = NULL;
         prev->next = newNode;
      }
   }
   else
   {
      /* special case for empty list */
      newNode->next = NULL;
      *list = newNode;
   }
}

/* record_allocation
*
* Record an allocation by adding the header to the head of the g_mgr allocation list.
*/
static void record_allocation(MEM_HEADER_T *hdr)
{
   vcos_mutex_lock((VCOS_MUTEX_T *)&g_mgr.mh);

   {
      MEM_HEADER_LIST_T *alloc = (MEM_HEADER_LIST_T *)malloc(sizeof(MEM_HEADER_LIST_T));

      /* If we can't record it, forget it, it will just not be defragged - we're probably doomed anyway */
      if (alloc != NULL)
      {
         MEM_HEADER_LIST_T *head = g_mgr.deviceAllocations;

         g_mgr.deviceAllocations = alloc;

         alloc->next = head;
         alloc->header = hdr;
      }
   }

   vcos_mutex_unlock((VCOS_MUTEX_T *)&g_mgr.mh);
}

/* remove_allocation
*
* Remove an allocation by removing it from the g_mgr allocation list.
*/
static void remove_allocation(MEM_HEADER_T *hdr)
{
   vcos_mutex_lock((VCOS_MUTEX_T *)&g_mgr.mh);

   {
      MEM_HEADER_LIST_T *ptr = g_mgr.deviceAllocations;
      MEM_HEADER_LIST_T **ptrPtr = &g_mgr.deviceAllocations;

      while (ptr != NULL)
      {
         MEM_HEADER_LIST_T *next = ptr->next;

         if (ptr->header == hdr)
         {
            free(ptr);

            *ptrPtr = next;
            ptr = NULL;
         }
         else
         {
            ptrPtr = &ptr->next;
            ptr = next;
         }
      }

      vcos_mutex_unlock((VCOS_MUTEX_T *)&g_mgr.mh);
   }
}

#else
static void record_allocation(MEM_HEADER_T *hdr) { UNUSED(hdr); }
static void remove_allocation(MEM_HEADER_T *hdr) { UNUSED(hdr); }
#endif

#ifdef BCG_VC4_DEFRAG

/* mem_defrag
*
* Attempts to defragment device memory.
*
* For each group of movable memory allocations
*    Copy to system memory
*    Delete device memory
* Then when there is no more system memory or when the end of a block is reached:
*    Allocate device memory
*    Copy back from system memory
*/
extern void glxx_hw_finish_context(GLXX_SERVER_STATE_T *state, bool wait);
extern void khrn_hw_wait(void);

static uint32_t find_biggest_alloc(MEM_HEADER_T **allocs, uint32_t howMany)
{
   uint32_t i;
   uint32_t biggestSize = 0;
   int32_t  biggestIndex = -1;

   for (i = 0; i < howMany; ++i)
   {
      if (allocs[i] != NULL && allocs[i]->allocedSize > biggestSize)
      {
         biggestSize = allocs[i]->allocedSize;
         biggestIndex = i;
      }
   }

   return biggestIndex;
}

/* mem_defrag()
*
* Attempts to defragment the heap by copying data into system memory and back.  When copying back, biggest
* items are copied first to maximise the chance that they will fit.
*
* Returns false if heap memory was not available during the copy back.  This is unlikely to happen, but could
* occur if the heap is shared with other threads/processes.  This is a bad case because the memory will be in
* an undefined state, so GL should report out-of-memory and applications should restart with a fresh context.
*/
static bool mem_defrag()
{
   static int  entries = 0;

   MEM_HEADER_LIST_T *sorted = NULL;
   MEM_HEADER_LIST_T *ptr = NULL;
   bool              ok = false;
   MEM_HEADER_T      **sysAllocs = NULL;
   uint32_t          sysAllocTop = 0;

   /* Can't defrag if we don't have a working mem interface */
   if (g_mgr.memInterface == NULL || g_mgr.memInterface->Free == NULL || g_mgr.memInterface->Alloc == NULL)
      return false;

   vcos_atomic_increment(&entries);

   /* Can't defrag re-entrantly */
   if (entries > 1)
      goto exit_atomic;

   INCR_DRIVER_COUNTER(defrags);

   /* Don't want anyone changing stuff behind our backs */
   vcos_mutex_lock((VCOS_MUTEX_T *)&g_mgr.mh);

   /* Allocate enough room to record the allocations */
   sysAllocs = (MEM_HEADER_T **)malloc(sizeof(MEM_HEADER_T *)* mem_hdr_list_length(g_mgr.deviceAllocations));
   sysAllocTop = 0;

   /* If we have no system memory then we can't defrag */
   if (sysAllocs == NULL)
      goto exit_mutex;

   /* Create a sorted version of the header list */
   for (ptr = g_mgr.deviceAllocations; ptr != NULL; ptr = ptr->next)
   {
      /* only defrag targets which have been locked, so have a valid device address */
      if ((ptr->header != NULL) &&
          (ptr->header->blockOffset != 0))
         mem_hdr_list_insert(&sorted, ptr->header);
   }

   /*
   * Note that the loop will keep running so long as there are still blocks to handle or there
   * are blocks to copy back from system memory.
   *
   * On each iteration, if ptr has not reached the end of the list then it will be advanced to the next
   * entry unless there is not enough system memory available.  In this case (and when the ptr has reached
   * the end of the list), sysPtr will be NULL and the second part of the loop will kick in.
   *
   * The second part of the loop will reset sysAllocTop to zero (regardless of whether all the memory
   * could be copied back -- it will copy as much as it can).
   *
   * There is a tricky case if the malloc fails, and we have no allocations in the system allocation list.  In
   * this case, we give up on defragging.  The defrag will return success because it may (unlikely) have done
   * some work.
   */
   for (ptr = sorted, ok = true; ptr != NULL || sysAllocTop != 0;)
   {
      void  *sysPtr = NULL;

      /* Deal with the next allocation, if there is one */
      if (ptr != NULL)
      {
         MEM_HEADER_T   *hdr = ptr->header;

         /* If we have an unlocked allocation, then copy to system memory and delete from device memory
         * and add it to the sysAlloc list.
         */
         if (hdr != NULL && hdr->lock_count == 0)
         {
            sysPtr = malloc(hdr->allocedSize);

            if (sysPtr != NULL)
            {
               memcpy(sysPtr, hdr->ptr, hdr->allocedSize);
               g_mgr.memInterface->Free(g_mgr.memInterface->context, hdr->ptr);
               hdr->ptr = sysPtr;

               /* Remember this one -- move on to next */
               sysAllocs[sysAllocTop++] = hdr;
               ptr = ptr->next;
            }
            else
            {
               /* sysAllocTop == 0: This would indicate that we have no system memory of our own to work with.
               * We've freed up all our system allocations and the malloc has still failed.  Give up on defragging.
               */
               if (sysAllocTop == 0)
                  ptr = NULL;
            }
         }
         else
         {
            /* Skip this one */
            ptr = ptr->next;
         }
      }

      /* If the allocaction was locked/wrapped, or the system memory allocation failed, then sysPtr will
      * still be NULL, so copy back all the stacked allocations (if any) to device memory and patch up the memory headers.
      */
      if (sysPtr == NULL)
      {
         uint32_t i;

         for (i = 0; i < sysAllocTop; ++i)
         {
            /* Work from the biggest down.  It is easier to fit small things round a big thing */
            uint32_t       biggest = find_biggest_alloc(sysAllocs, sysAllocTop);
            MEM_HEADER_T   *hdr = sysAllocs[biggest];
            void           *nptr;

            assert(hdr != NULL);

            /* We don't want to use this one again! */
            sysAllocs[biggest] = NULL;

            bool secure = !!(hdr->flags & MEM_FLAG_SECURE);
            nptr = g_mgr.memInterface->Alloc(g_mgr.memInterface->context, hdr->allocedSize, hdr->align, secure);

            /* Put the data back into device memory
            * This should not fail, but we need to do something sensible if it does
            */
            if (nptr != NULL)
               memcpy(nptr, hdr->ptr, hdr->allocedSize);
            else
               ok = false;

            free(hdr->ptr);
            hdr->ptr = nptr;
            hdr->blockOffset = 0;
            hdr->blockPtr = NULL;
         }

         sysAllocTop = 0;
      }
   }

   mem_hdr_list_free(&sorted);
   free(sysAllocs);

   /* Make sure that the device memory is coherent */
   mem_flush_cache();

exit_mutex:
   vcos_mutex_unlock((VCOS_MUTEX_T *)&g_mgr.mh);

exit_atomic:
   vcos_atomic_decrement(&entries);

   return ok;
}

#else
static bool mem_defrag(void)
{
   return true;
}
#endif

/* allocate_direct
*
* Helper function for allocating device memory
* Attempts to allocate memory, if this fails, then trigger a defragmentation and try again.
* If second allocation fails, then give up.
*/
static void *allocate_direct(MEM_HEADER_T *h)
{
   void  *ptr = NULL;

   if (g_mgr.memInterface != NULL && g_mgr.memInterface->Alloc != NULL)
   {
      bool secure = !!(h->flags & MEM_FLAG_SECURE);

      /* Always try to allocate */
      ptr = g_mgr.memInterface->Alloc(g_mgr.memInterface->context, h->allocedSize, h->align, secure);

      /* If the allocation failed, then defrag and try again */
      if (ptr == NULL)
      {
         if (mem_defrag())
         {
            ptr = g_mgr.memInterface->Alloc(g_mgr.memInterface->context, h->allocedSize, h->align, secure);
         }
      }
   }

   return ptr;
}

static void init_header(MEM_HEADER_T *h, uint32_t size, uint32_t align, MEM_FLAG_T flags, const char *desc)
{
   /* needs to be max of CPU cache line and the GCACHE on the v3d */
   uint32_t cacheline_size = max(mem_cacheline_size(), BCG_GCACHE_LINE_SIZE);

   if (flags & MEM_FLAG_DIRECT)
   {
      h->allocedSize = (size + (cacheline_size - 1)) & ~(cacheline_size - 1);
      if (flags & MEM_FLAG_256BIT_PAD)
      {
         /* round to 32bytes */
         h->allocedSize = ((h->allocedSize + 31) & ~0x1F);
         /* add 32bytes, for HW to prefetch */
         h->allocedSize += 32;
      }
   }
   else
      h->allocedSize = size;

   h->size = size;
   h->align = (flags & MEM_FLAG_DIRECT) ? ((align > cacheline_size) ? align : cacheline_size) : align;
   h->term = NULL;
   h->desc = desc;
   h->ref_count = 1;
#ifdef BCG_VC4_DEFRAG
   h->lock_count = 0;
#endif
   h->magic = MAGIC;
   flags = (MEM_FLAG_T)((flags & ~MEM_FLAG_RETAINED) | ((flags & MEM_FLAG_NO_INIT) ? MEM_FLAG_ABANDONED : 0));
   h->flags = flags;
   h->blockOffset = 0;
   h->blockPtr = NULL;

   if (h->size > 0)
   {
      /* in the case of non HW allocated blocks, just use a standard allocation */
      if (flags & MEM_FLAG_DIRECT)
      {
         h->ptr = allocate_direct(h);

         if (h->ptr)
         {
            /* initializ(s)e the memory */
            if ((flags & MEM_FLAG_NO_INIT) == 0)
            {
               void * p = mem_lock((MEM_HANDLE_T)h, NULL);
               vcos_demand(p != NULL);
               memset(p, (flags & MEM_FLAG_ZERO) ? 0 : MEM_HANDLE_INVALID, h->allocedSize);
               mem_unlock((MEM_HANDLE_T)h);
            }

            /* Record this allocation for defrag purposes */
            record_allocation(h);
         }
#ifdef ASSERT_ON_ALLOC_FAIL
         else
         {
            printf("BEGL_memoryInterface->Alloc() FAILED, size = %d, align = %d, desc = %s\n", h->size, h->align, h->desc);
            assert(0);
         }
#endif
      }
      else
      {
         h->ptr = malloc(h->allocedSize);

         if (h->ptr)
         {
            /* initializ(s)e the memory */
            if ((flags & MEM_FLAG_NO_INIT) == 0)
               memset(h->ptr, (flags & MEM_FLAG_ZERO) ? 0 : MEM_HANDLE_INVALID, h->size);
            else
            {
               /*  in a debug build, set uninitialised memory to rubbish */
#if !defined(NDEBUG)
               /*  don't do this for large allocations (eg frame buffers) */
               if (h->size < (64 * 1024))
                  memset(h->ptr, 0xdddddddd, h->size);
#endif
            }
         }
#ifdef ASSERT_ON_ALLOC_FAIL
         else
            assert(0);
#endif
      }
   }
   else
      h->ptr = NULL;
}

/*
Attempts to allocate a movable block of memory of the specified size and
alignment. size may be 0. A MEM_HANDLE_T with size 0 is special: see
mem_lock/mem_unlock. mode specifies the types of compaction permitted,
including MEM_COMPACT_NONE.

Preconditions:

- align is a power of 2.
- flags only has set bits within the range specified by MEM_FLAG_ALL.
- desc is NULL or a pointer to a null-terminated string.

Postconditions:

If the attempt succeeds:
- A fresh MEM_HANDLE_T referring to the allocated block of memory is
returned.
- The MEM_HANDLE_T is unlocked, without a terminator, and has a reference
count of 1.
- If MEM_FLAG_RETAINED was specified, the MEM_HANDLE_T has a retain count of
1, otherwise it is unretained.
- If the MEM_FLAG_ZERO flag was specified, the block of memory is set to 0.
Otherwise, each aligned word is set to MEM_HANDLE_INVALID.

If the attempt fails:
- MEM_HANDLE_INVALID is returned.
*/

MEM_HANDLE_T mem_alloc_ex(
   uint32_t size,
   uint32_t align,
   MEM_FLAG_T flags,
   const char *desc,
   mem_compact_mode_t mode)
{
   MEM_HEADER_T * h;

   UNUSED(mode);

   assert(is_power_of_2(align));
   assert((flags & MEM_FLAG_RESIZEABLE) || !(flags & MEM_FLAG_HINT_GROW));

#ifdef DETECT_LEAKS
   /* BMEM allocator has a leak detector.  Force this always to check for leaks (for free!) */
   flags |= MEM_FLAG_DIRECT;
#endif

   h = (MEM_HEADER_T *)malloc(sizeof(MEM_HEADER_T));
   if (h == NULL)
   {
#ifdef ASSERT_ON_ALLOC_FAIL
      assert(0);
#endif
      return MEM_HANDLE_INVALID;
   }

   init_header(h, size, align, flags, desc);
   if ((h->size > 0) && (h->ptr == NULL))
   {
      free(h);
      return MEM_HANDLE_INVALID;
   }
   else
      return (MEM_HANDLE_T)(uintptr_t)h;
}

/* IN THE CASE OF MMA, THE BLOCK MUST BE PRE-LOCKED BY THE PLATFORM LAYER */
MEM_HANDLE_T mem_wrap(void *p, uint32_t offset, uint32_t size, uint32_t align, MEM_FLAG_T flags, const char *desc)
{
   MEM_HEADER_T *h;

   assert(is_power_of_2(align));
   assert(!(flags & MEM_FLAG_RESIZEABLE));

   h = (MEM_HEADER_T *)malloc(sizeof(MEM_HEADER_T));

   if (h == NULL)
   {
#ifdef ASSERT_ON_ALLOC_FAIL
      assert(0);
#endif
      return MEM_HANDLE_INVALID;
   }

   h->size = size;
   h->allocedSize = size;
   h->align = align;
   h->term = NULL;
   h->desc = desc;
   h->ref_count = 1;
#ifdef BCG_VC4_DEFRAG
   h->lock_count = 0;
#endif
   h->magic = MAGIC;
   flags = (MEM_FLAG_T)((flags & ~MEM_FLAG_RETAINED) | ((flags & MEM_FLAG_NO_INIT) ? MEM_FLAG_ABANDONED : 0));
   flags |= MEM_FLAG_WRAPPED;
   h->flags = flags;
   h->ptr = p;
   h->blockOffset = offset;
   h->blockPtr = p;

   return (MEM_HANDLE_T)(uintptr_t)h;
}

/*
If the reference count of the MEM_HANDLE_T is 1 and it has a terminator, the
terminator is called with a pointer to and the size of the MEM_HANDLE_T's
block of memory (or NULL/0 if the size of the MEM_HANDLE_T is 0). The
MEM_HANDLE_T may not be used during the call.

Preconditions:

- handle is a valid MEM_HANDLE_T.
- If its reference count is 1, it must not be locked or retained.

Postconditions:

If the reference count of the MEM_HANDLE_T was 1:
- The MEM_HANDLE_T is now invalid. The associated block of memory has been
freed.

Otherwise:
- The reference count of the MEM_HANDLE_T is decremented.
*/

void mem_release_inner(MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;

   /* put me back in if things start to go wrong */
   /*assert((handle != MEM_ZERO_SIZE_HANDLE) && (handle != MEM_EMPTY_STRING_HANDLE));*/

   if (h->term)
      h->term(handle);

   if (h->ptr != NULL)
   {
      if (!(h->flags & MEM_FLAG_WRAPPED))
      {
         if (h->flags & MEM_FLAG_DIRECT)
         {
            if (g_mgr.memInterface != NULL && g_mgr.memInterface->Free != NULL)
               g_mgr.memInterface->Free(g_mgr.memInterface->context, h->ptr);

            remove_allocation(h);
         }
         else
         {
            if (((h->blockOffset != 0) || (h->blockPtr != NULL)) &&
                 (g_mgr.memInterface != NULL && g_mgr.memInterface->MemUnlock != NULL))
               g_mgr.memInterface->MemUnlock(g_mgr.memInterface->context, h->ptr);
            free(h->ptr);
         }
      }
   }
   free(h);
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.

Postconditions:

If the reference count of the MEM_HANDLE_T was 1:
- 0 is returned.
- The reference count of the MEM_HANDLE_T is still 1.

Otherwise:
- 1 is returned.
- The reference count of the MEM_HANDLE_T is decremented.
*/

int mem_try_release(
   MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   unsigned int ref_count;

   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);

   /* <= in case another thread jumps in and multiple InterlockedDecrement occur */
   ref_count = vcos_atomic_decrement(&h->ref_count);

   if (ref_count <= 0)
   {
      vcos_atomic_increment(&h->ref_count);
      return 0;
   }
   else
      return 1;
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.

Postconditions:

- The MEM_HANDLE_T's reference count is returned.
*/

uint32_t mem_get_ref_count(
   MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);
   return h->ref_count;
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.

Postconditions:

- The MEM_HANDLE_T's lock count is returned.
*/

uint32_t mem_get_lock_count(
   MEM_HANDLE_T handle)
{
   MEM_HEADER_T   *h = (MEM_HEADER_T *)handle;
   assert(h->magic == MAGIC);
#ifdef BCG_VC4_DEFRAG
   return h->lock_count;
#else
   return 0;
#endif
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.

Postconditions:

- The MEM_HANDLE_T's description string is returned.
*/

const char *mem_get_desc(
   MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   assert(h->magic == MAGIC);
   return h->desc;
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.
- desc is NULL or a pointer to a null-terminated string.

Postconditions:

- The MEM_HANDLE_T's description is set to desc.
*/

void mem_set_desc(
   MEM_HANDLE_T handle,
   const char *desc)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   assert(h->magic == MAGIC);
   h->desc = desc;
}

/*
The MEM_HANDLE_T's terminator is called just before the MEM_HANDLE_T becomes
invalid: see mem_release.

Preconditions:

- handle is a valid MEM_HANDLE_T.
- term must be NULL or a pointer to a function compatible with the MEM_TERM_T
type.

Postconditions:

- The MEM_HANDLE_T's terminator is set to term (if term was NULL, the
MEM_HANDLE_T no longer has a terminator).
*/

void mem_set_term(
   MEM_HANDLE_T handle,
   MEM_TERM_T term,
   void *param)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);
   h->term = term;
   h->param = param;
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.

Postconditions:

- The MEM_HANDLE_T's terminator is returned, or NULL if there is none.
*/

MEM_TERM_T mem_get_term(
   MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);
   return h->term;
}

/*
void mem_set_user_flag(MEM_HANDLE_T handle, int flag)

Preconditions:

- handle is a valid MEM_HANDLE_T.

Postconditions:

- The MEM_HANDLE_T's user flag is set to 0 if flag is 0, or to 1 otherwise.
*/

void mem_set_user_flag(
   MEM_HANDLE_T handle, int flag)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);

   vcos_mutex_lock((VCOS_MUTEX_T *)&g_mgr.mh);

   if (flag)
      h->flags |= MEM_FLAG_USER;
   else
      h->flags &= ~MEM_FLAG_USER;

   vcos_mutex_unlock((VCOS_MUTEX_T *)&g_mgr.mh);
}

/*
Attempts to resize the MEM_HANDLE_T's block of memory. The attempt is
guaranteed to succeed if the new size is less than or equal to the old size.
size may be 0. A MEM_HANDLE_T with size 0 is special: see
mem_lock/mem_unlock. mode specifies the types of compaction permitted,
including MEM_COMPACT_NONE.

Preconditions:

- handle is a valid MEM_HANDLE_T.
- It must not be locked.
- It must have been created with the MEM_FLAG_RESIZEABLE flag.

Postconditions:

If the attempt succeeds:
- 1 is returned.
- The MEM_HANDLE_T's block of memory has been resized.
- The contents in the region [0, min(old size, new size)) are unchanged. If
the MEM_HANDLE_T is zero'd, the region [min(old size, new size), new size)
is set to 0. Otherwise, each aligned word in the region
[min(old size, new size), new size) is set to MEM_HANDLE_INVALID.

If the attempt fails:
- 0 is returned.
- The MEM_HANDLE_T is still valid.
- Its block of memory is unchanged.
*/

int mem_resize_ex(
   MEM_HANDLE_T handle,
   uint32_t size,
   mem_compact_mode_t mode)
{
   int res;
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   void * nptr;

   UNUSED(mode);

   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);
#ifdef BCG_VC4_DEFRAG
   assert(h->lock_count == 0);
#endif
   assert(h->flags & MEM_FLAG_RESIZEABLE);

   /* dont allow resize of secure blocks */
   vcos_demand(!(h->flags & MEM_FLAG_SECURE));

   if (h->flags & MEM_FLAG_DIRECT)
   {
      /* needs to be max of CPU cache line and the GCACHE on the v3d */
      uint32_t cacheline_size = max(mem_cacheline_size(), BCG_GCACHE_LINE_SIZE);
      h->allocedSize = (size + (cacheline_size - 1)) & ~(cacheline_size - 1);
      if (h->flags & MEM_FLAG_256BIT_PAD)
      {
         /* round to 32bytes */
         h->allocedSize = ((h->allocedSize + 31) & ~0x1F);
         /* add 32bytes, for HW to prefetch */
         h->allocedSize += 32;
      }

      nptr = allocate_direct(h);

      vcos_mutex_lock((VCOS_MUTEX_T *)&g_mgr.mh);

      if (nptr)
      {
         void * dst;
         MEM_HEADER_T new;

         new = *h;
         new.size = size;
         new.ptr = nptr;
         new.blockOffset = 0;
         new.blockPtr = NULL;

         dst = mem_lock((MEM_HANDLE_T)&new, NULL);

         if (dst != NULL )
         {
            void * src;
            MEM_HEADER_T old = *h;
            src = mem_lock((MEM_HANDLE_T)&old, NULL);

            if (src)
            {
               memcpy(dst, src, min(h->size, h->allocedSize));

               mem_unlock((MEM_HANDLE_T)&old);

               if (g_mgr.memInterface != NULL && g_mgr.memInterface->Free != NULL)
                  g_mgr.memInterface->Free(g_mgr.memInterface->context, old.ptr);
            }
            else
               mem_unlock((MEM_HANDLE_T)&old);
         }

         h->ptr = nptr;
         h->blockOffset = new.blockOffset;
         h->blockPtr = new.blockPtr;

         if (h->ptr)
         {
            if ((size > h->size) && (h->flags & MEM_FLAG_ZERO))
               memset((void *)((uintptr_t)dst + h->size), 0, size - h->size);
            h->size = size;
            res = 1;
         }
         else
            res = 0;

         mem_unlock((MEM_HANDLE_T)&new);
      }
      else
         res = 0;
   }
   else
   {
      vcos_mutex_lock((VCOS_MUTEX_T *)&g_mgr.mh);

      /* realloc() is defined to ping-pong.  If ptr = NULL and size == 0, then it returns a ptr to a zero size array.
      If you then resize a zero size array to zero, its the equivalent of calling free() and the return in NULL.
      This test will call the free instead and allow the ptr = NULL and size == 0 to work.
      */
      if (h->ptr && size == 0)
      {
         free(h->ptr);
         h->ptr = NULL;
      }

      h->ptr = realloc(h->ptr, size);

      if (h->ptr)
      {
         if ((size > h->size) && (h->flags & MEM_FLAG_ZERO))
            memset((void *)((uintptr_t)h->ptr + h->size), 0, size - h->size);
         h->size = size;
         h->allocedSize = size;
         res = 1;
      }
      else
         res = 0;
   }

   vcos_mutex_unlock((VCOS_MUTEX_T *)&g_mgr.mh);

   return res;
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.
- If its size is not 0, it must be locked.

Postconditions:

If the MEM_HANDLE_T's size is 0:
- The MEM_HANDLE_T is completely unchanged.

Otherwise:
- The MEM_HANDLE_T's lock count is decremented.
*/

void mem_unlock(MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;

   if (h->size == 0)
      return;

#if 0
   /* in the MMA solution we leave everything mapped/locked in the cached until Free */
   if (h->flags & MEM_FLAG_DIRECT)
   {
      if (g_mgr.memInterface != NULL && g_mgr.memInterface->MemUnlock != NULL)
         g_mgr.memInterface->MemUnlock(g_mgr.memInterface->context, h->ptr);
      else
         /* MemUnlock is mandatory */
         vcos_assert(0);
   }
#endif

#ifdef BCG_VC4_DEFRAG
   assert(h->lock_count != 0);
   vcos_atomic_decrement(&h->lock_count);
#endif
}


uintptr_t mem_lock_offset(MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   uint32_t res;

   assert(handle != MEM_HANDLE_INVALID);
   assert(h->magic == MAGIC);

   if (h->size == 0)
      return 0;

#ifdef BCG_VC4_DEFRAG
   vcos_atomic_increment(&h->lock_count);
#endif

   if (h->flags & MEM_FLAG_DIRECT)
   {
      if (g_mgr.memInterface != NULL && g_mgr.memInterface->MemLockOffset != NULL)
      {
         /* populate the block cache if required */
         if (h->blockOffset == 0)
            h->blockOffset = g_mgr.memInterface->MemLockOffset(g_mgr.memInterface->context, h->ptr);

         res = h->blockOffset;
      }
      else
         /* MemLock is mandatory */
         UNREACHABLE();
   }
   else
   {
      /* illegal to get an offset of linux memory */
      UNREACHABLE();
   }

   return res;
}

/*
A MEM_HANDLE_T with a lock count greater than 0 is considered to be locked
and may not be moved by the memory manager.

Preconditions:

- handle is a valid MEM_HANDLE_T.
If it is locked, the previous lock must have been by mem_lock.

Postconditions:

If the MEM_HANDLE_T's size is 0:
NULL is returned.
- The MEM_HANDLE_T is completely unchanged.

Otherwise:
- A pointer to the MEM_HANDLE_T's block of memory is returned. The pointer is
valid until the MEM_HANDLE_T's lock count reaches 0.
- The MEM_HANDLE_T's lock count is incremented.
- Clears MEM_FLAG_ABANDONED.
*/

void * mem_lock(MEM_HANDLE_T handle, MEM_LOCK_T *lbh)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   void *res;

   assert(handle != MEM_HANDLE_INVALID);
   assert(h->magic == MAGIC);

   if (h->size == 0)
      return NULL;

#ifdef BCG_VC4_DEFRAG
   vcos_atomic_increment(&h->lock_count);
#endif

   if (h->flags & MEM_FLAG_DIRECT)
   {
      if (g_mgr.memInterface != NULL && g_mgr.memInterface->MemLock != NULL)
      {
         bool secure = !!(h->flags & MEM_FLAG_SECURE);

         /* populate the block cache if required, wrapped already filled out */
         if (!(h->flags & MEM_FLAG_WRAPPED))
         {
            /* don't map secure buffers */
            if (!secure && h->blockPtr == NULL)
               h->blockPtr = g_mgr.memInterface->MemLock(g_mgr.memInterface->context, h->ptr);

            if (h->blockOffset == 0)
               h->blockOffset = g_mgr.memInterface->MemLockOffset(g_mgr.memInterface->context, h->ptr);
         }

         if (lbh)
         {
            res = lbh->p = h->blockPtr;
            lbh->offset = h->blockOffset;
            lbh->size = h->size;
            lbh->desc = h->desc;
         }
         else
            res = h->blockPtr;
      }
      else
         /* MemLock is mandatory */
         UNREACHABLE();
   }
   else
   {
      res = h->ptr;
      vcos_assert(lbh == NULL);
   }

   return res;
}

/*
A discardable MEM_HANDLE_T with a retain count greater than 0 is
considered retained and may not be discarded by the memory manager.

Preconditions:

- handle is a valid MEM_HANDLE_T.
- It must be discardable.

Postconditions:

- 0 is returned if the size of the MEM_HANDLE_T's block of memory is 0,
otherwise 1 is returned.
- The retain count of the MEM_HANDLE_T is incremented.
*/

int mem_retain(
   MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;

   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);
   assert(h->flags & MEM_FLAG_DISCARDABLE);

   return !(h->size == 0);
}

/*
Preconditions:

- handle is a valid MEM_HANDLE_T.
- It must be retained.

Postconditions:

- The retain count of the MEM_HANDLE_T is decremented.
*/

void mem_unretain(
   MEM_HANDLE_T handle)
{
   UNUSED(handle);
}

void mem_copy2d(KHRN_IMAGE_FORMAT_T format, MEM_HANDLE_T hDst, MEM_HANDLE_T hSrc,
                uint16_t width, uint16_t height, int32_t stride, int32_t dstStride, bool secure)
{
   MEM_HEADER_T *src = (MEM_HEADER_T *)hSrc;
   MEM_HEADER_T *dst = (MEM_HEADER_T *)hDst;
   void *srcCached, *dstCached;
   BEGL_BufferFormat conv_format = BEGL_BufferFormat_INVALID;

   switch (format)
   {
   case ABGR_8888_RSO:
   case XBGR_8888_RSO:
   case RGB_565_RSO:
      /* this just informs the platform to do a packet blit operation.  As we pass the
         stride, it works for 16 bit as well */
      conv_format = BEGL_BufferFormat_eA8B8G8R8;
      break;
   case YV12_RSO:
      conv_format = BEGL_BufferFormat_eYV12_Texture;
      break;
   default:
      break;
   }

   if (conv_format != BEGL_BufferFormat_INVALID)
   {
      MEM_LOCK_T spLbh, dpLbh;
      uint32_t srcOffset, dstOffset;

      /* dst physical offset */
      dstCached = mem_lock(hDst, &dpLbh);
      dstOffset = khrn_hw_addr(dstCached, &dpLbh);

      /* src physical offset */
      srcCached = mem_lock(hSrc, &spLbh);
      srcOffset = khrn_hw_addr(srcCached, &spLbh);

      if ((src->flags & MEM_FLAG_WRAPPED) && (dst->flags & MEM_FLAG_DIRECT))
      {
         if (g_mgr.memInterface != NULL && g_mgr.memInterface->MemCopy2d != NULL)
         {
            BEGL_MemCopy2d params;
            params.dstOffset = dstOffset;
            params.srcOffset = srcOffset;
            params.dstCached = dstCached;
            params.srcCached = srcCached;
            params.width = width;
            params.height = height;
            params.stride = stride;
            params.dstStride = dstStride;
            params.format = conv_format;
            params.secure = secure;

            g_mgr.memInterface->MemCopy2d(g_mgr.memInterface->context, &params);
         }
         else
         {
            if (conv_format == BEGL_BufferFormat_eA8B8G8R8)
            {
               memcpy(dstCached, srcCached, height * stride);
               mem_flush_cache();
            }
            else
               vcos_assert(0);   /* NO SW PATH FOR YUV */
         }
      }
      else
      {
         if (conv_format == BEGL_BufferFormat_eA8B8G8R8)
            memcpy(dstCached, srcCached, height * stride);
         else
            vcos_assert(0);   /* NO SW PATH FOR YUV */
      }

      mem_unlock(hSrc);
      mem_unlock(hDst);
   }
}

/******************************************************************************
Movable memory helpers
******************************************************************************/

MEM_HANDLE_T mem_strdup_ex(
   const char *str,
   mem_compact_mode_t mode)
{
   MEM_HANDLE_T handle = mem_alloc_ex((uint32_t)strlen(str) + 1, 1, MEM_FLAG_NONE, "mem_strdup", mode);
   if (handle == MEM_HANDLE_INVALID) {
      return MEM_HANDLE_INVALID;
   }
   strcpy((char *)mem_lock(handle, NULL), str);
   mem_unlock(handle);
   return handle;
}

/******************************************************************************
special interface only for KHRN_AUTOCLIF
******************************************************************************/
void *autoclif_addr_to_ptr(uint32_t addr)
{
   /* non optional interface if autoclif is on.  Crash if not present */
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->DebugAutoclifAddrToPtr != NULL)
      return g_mgr.memInterface->DebugAutoclifAddrToPtr(g_mgr.memInterface->context, addr);
   else
   {
      UNREACHABLE();
      return NULL;
   }
}

uint32_t autoclif_ptr_to_addr(void *p)
{
   /* non optional interface if autoclif is on.  Crash if not present */
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->DebugAutoclifPtrToAddr != NULL)
      return g_mgr.memInterface->DebugAutoclifPtrToAddr(g_mgr.memInterface->context, p);
   else
   {
      UNREACHABLE();
      return 0;
   }
}

const char *autoclif_get_clif_filename(void)
{
   /* non optional interface if autoclif is on.  Crash if not present */
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->DebugAutoclifGetClifFilename != NULL)
      return g_mgr.memInterface->DebugAutoclifGetClifFilename(g_mgr.memInterface->context);
   else
   {
      UNREACHABLE();
      return NULL;
   }
}

void autoclif_reset(void)
{
   /* optional interface depending on the platform */
   if (g_mgr.memInterface != NULL && g_mgr.memInterface->DebugAutoclifReset != NULL)
      g_mgr.memInterface->DebugAutoclifReset(g_mgr.memInterface->context);
   else
      return;
}