/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos_thread.h"
#include "interface/vcos/vcos_event.h"
#include "interface/vcos/vcos_mutex.h"

#include "rtos_abstract_mem.h"
#include "list.h"

#include <malloc.h>

#define BLOCK_SIZE (1 * 1024 * 1024)   /* MUST BE A MULTIPLE OF CPU PAGE LENGTH */
#define BLOCK_PAGE_SIZE (512)          /* MUST BE A MULTIPLE OF CPU PAGE LENGTH */
#define MAX_BLOCKS (16)
#define GUARD_TAG (0x3d78a53d)

typedef struct block {
   LIST_ENTRY(block) link;

   // allocation is BLOCK_SIZE divided into cache line pages
   MEM_HANDLE_T mem_handle;
   char *cpu_ptr;
   uintptr_t dev_offset;

   // usage
   bool usage[BLOCK_SIZE / BLOCK_PAGE_SIZE];

   // everything after this is free
   char *free_head;
   // everything before this is free
   char *free_tail;

   size_t free_bytes;

   int age;
} block;

typedef struct talloc {
   // simple linked list should suffice, as we only are going to have upper max of 16MB (16 blocks)
   LIST_HEAD(blocklist, block) list;

   int blocks;

   VCOS_THREAD_T thread;
   VCOS_EVENT_T done;
   VCOS_MUTEX_T guard;
} talloc;

typedef struct header {
   uint32_t guard;
   block *b;
   int num_of_pages;
} header;

static inline uintptr_t talloc_virt_to_v3d_addr(block *b, char *p)
{
   return b->dev_offset + (p - b->cpu_ptr);
}

static block *talloc_block(void)
{
   block *b = (block *)malloc(sizeof(block));
   if (b) {

      b->mem_handle = mem_alloc_ex(BLOCK_SIZE, BLOCK_PAGE_SIZE, MEM_FLAG_DIRECT, "talloc_block", MEM_COMPACT_DISCARD);

      if (b->mem_handle == MEM_HANDLE_INVALID)
         goto error;
      b->cpu_ptr = mem_lock(b->mem_handle, NULL);
      b->dev_offset = mem_lock_offset(b->mem_handle);

      memset(b->usage, 0, sizeof(b->usage));
      b->free_tail =
         b->free_head = b->cpu_ptr;
      b->free_bytes = BLOCK_SIZE;
      b->age = 0;
   }

   return b;

error:
   free(b);
   return NULL;
}

static void *talloc_thread_main(void *arg)
{
   talloc *handle = (talloc *)arg;

   // kicks every second
   while (vcos_event_timed_wait(&handle->done, 1000) == VCOS_ETIMEDOUT) {
      block *next = NULL;

      vcos_mutex_lock(&handle->guard);

#if 0
      for (block *b = LIST_FIRST(&handle->list); b; b = LIST_NEXT(b, link)) {
         selective_logd("block %p, free %d", gmem_get_addr(b->gmem_handle), b->free_bytes);
      }
#endif

      // don't want to clean up the last block
      for (block *b = LIST_FIRST(&handle->list); b && handle->blocks > 1; b = next) {
         next = LIST_NEXT(b, link);
         // block is in a reset state.
         if (b->cpu_ptr == b->free_head &&
            b->free_head == b->free_tail) {
            b->age--;
            if (b->age == 0) {

               mem_release(b->mem_handle);

               handle->blocks--;
               LIST_REMOVE(&handle->list, b, link);
               free(b);
            }
         }
      }

      vcos_mutex_unlock(&handle->guard);
   }
   return NULL;
}

void *talloc_initialize(void)
{
   talloc *handle = (talloc *)malloc(sizeof(talloc));
   if (!handle)
      return NULL;
   memset(handle, 0, sizeof(*handle));

   if (vcos_mutex_create(&handle->guard, "tallocGuard") != VCOS_SUCCESS)
   {
      free(handle);
      return NULL;
   }

   block *b = talloc_block();
   if (b != NULL)
   {
      LIST_INIT(&handle->list);
      LIST_INSERT_HEAD(&handle->list, b, link);
      handle->blocks = 1;

      if (vcos_event_create(&handle->done, "tallocDone") == VCOS_SUCCESS)
      {
         VCOS_THREAD_ATTR_T   attr;
         vcos_thread_attr_init(&attr);
         vcos_thread_attr_setpriority(&attr, VCOS_THREAD_PRI_NORMAL);

         if (vcos_thread_create(&handle->thread, "talloc", &attr, talloc_thread_main, handle) == VCOS_SUCCESS)
         {
            return handle;
         }
         vcos_event_delete(&handle->done);
      }

      mem_release(b->mem_handle);
      free(b);
   }
   free(handle);
   return NULL;
}

void talloc_term(void *h)
{
   talloc *handle = (talloc *)h;
   if (handle) {
      block *next = NULL;

      vcos_event_signal(&handle->done);
      vcos_thread_join(&handle->thread, NULL);
      vcos_event_delete(&handle->done);

      for (block *b = LIST_FIRST(&handle->list); b; b = next) {

         mem_release(b->mem_handle);

         next = LIST_NEXT(b, link);
         free(b);
      }

      vcos_mutex_delete(&handle->guard);

      free(handle);
   }
}

static bool talloc_block_alloc(block* b, size_t size, size_t align, void **cpu_ptr, uintptr_t *addr)
{
   UNUSED(align);

   // One extra block for header information
   int num_of_pages = ((size + BLOCK_PAGE_SIZE - 1) / BLOCK_PAGE_SIZE) + 1;
   size_t allocation_size = num_of_pages * BLOCK_PAGE_SIZE;

   // Shouldn't get this far if allocation won't fit in a new block.
   assert(allocation_size <= BLOCK_SIZE);

   if (allocation_size > b->free_bytes)
      return false;

   // fits, just allocate respective pages
   int page_offset = (b->free_head - b->cpu_ptr) / BLOCK_PAGE_SIZE;
   for (int i = page_offset; i < (page_offset + num_of_pages); i++)
   {
      assert(b->usage[i] == false);
      b->usage[i] = true;
   }

   // header goes at the front of the allocation
   header *alloc_header = (header *)b->free_head;
   alloc_header->guard = GUARD_TAG;
   alloc_header->b = b;
   alloc_header->num_of_pages = num_of_pages;

   // actual allocation goes to next block
   *cpu_ptr = (void *)(b->free_head + BLOCK_PAGE_SIZE);
   *addr = talloc_virt_to_v3d_addr(b, *cpu_ptr);

   b->free_head = b->free_head + allocation_size;
   b->free_bytes -= allocation_size;

   return true;
}

// Called from within the global gmem mutex.
// align is ignored to its maximum alignment which is BLOCK_PAGE_SIZE
bool talloc_alloc(void *h, size_t size, size_t align, void **cpu_ptr, uintptr_t *addr)
{
   talloc *handle = (talloc *)h;

   // basic checks
   if (align > BLOCK_PAGE_SIZE || size > (BLOCK_SIZE-BLOCK_PAGE_SIZE))
      return false;

   vcos_mutex_lock(&handle->guard);

   for (block *b = LIST_FIRST(&handle->list); b; b = LIST_NEXT(b, link))
   {
      if (talloc_block_alloc(b, size, align, cpu_ptr, addr))
      {
         vcos_mutex_unlock(&handle->guard);
         return true;
      }
   }

   // allocation failed, grab new block and try again
   if (handle->blocks < MAX_BLOCKS)
   {
      block *b = talloc_block();
      if (b)
      {
         LIST_INSERT_HEAD(&handle->list, b, link);
         handle->blocks++;
         vcos_demand(talloc_block_alloc(b, size, align, cpu_ptr, addr));
         vcos_mutex_unlock(&handle->guard);
         return true;
      }
   }

   vcos_mutex_unlock(&handle->guard);
   return false;
}

// Called from within the global gmem mutex.
void talloc_free(void *h, void *p)
{
   talloc *handle = (talloc *)h;

   char *allocation = (char *)p;

   // check the allocation looks valid (aligned to page size)
   if ((char *)((uintptr_t)(allocation + (BLOCK_PAGE_SIZE - 1)) & ~(BLOCK_PAGE_SIZE - 1)) == allocation) {

      vcos_mutex_lock(&handle->guard);

      // rewind to the block header
      header *alloc_header = (header *)((uintptr_t)p - BLOCK_PAGE_SIZE);
      if (alloc_header->guard == GUARD_TAG) {
         block *b = alloc_header->b;
         alloc_header->guard = 0;
         int num_of_pages = alloc_header->num_of_pages;

         int page_offset = ((uintptr_t)alloc_header - (uintptr_t)b->cpu_ptr) / BLOCK_PAGE_SIZE;
         int i;

         for (i = page_offset; i < (page_offset + num_of_pages); i++) {
            assert(b->usage[i] == true);
            b->usage[i] = false;
         }

         // freeing at the end of a block can just rewind
         if ((char *)alloc_header + (num_of_pages * BLOCK_PAGE_SIZE) == b->free_head) {
            b->free_head = (char *)alloc_header;
            b->free_bytes += (num_of_pages * BLOCK_PAGE_SIZE);
         }
         else if ((char *)alloc_header == b->free_tail) {
            int maxi = (b->free_head - b->cpu_ptr) / BLOCK_PAGE_SIZE;
            assert(b->usage[maxi] == false);
            // this loop will coalesce free block to the other blocks
            // CAREFUL! it carries over 'i' from above
            while (b->usage[i] == false && (i < maxi)) {
               num_of_pages++;
               i++;
            }
            b->free_tail = (char *)alloc_header + (num_of_pages * BLOCK_PAGE_SIZE);
         }

         if (b->free_head == b->free_tail) {
            // reset to front of the block
            b->free_head =
               b->free_tail = b->cpu_ptr;
            // reset the number of free bytes
            b->free_bytes = BLOCK_SIZE;
            // start the aging process - after b->age seconds of no use it will die
            b->age = 10;
         }
      }

      vcos_mutex_unlock(&handle->guard);
   }
}
