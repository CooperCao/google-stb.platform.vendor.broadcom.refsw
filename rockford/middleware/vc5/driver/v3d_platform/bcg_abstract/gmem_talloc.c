/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :
Module   :

FILE DESCRIPTION
Small module for transient allocations used in the BCG gmem module.
This ulimately saves us the trip through the platform layer and out to NEXUS
for lock & map.
=============================================================================*/

#include "vcos.h"
#include "gmem_abstract.h"
#include "list.h"

// not part of the public gmem API
// as talloc is called from within gmem, it need to be able to recursively call itself

extern gmem_handle_t gmem_alloc_internal(size_t size, size_t align, uint32_t usage_flags, const char *desc);
extern void gmem_free_internal(gmem_handle_t handle);

extern v3d_addr_t gmem_lock_internal(struct gmem_lock_list *lock_list, gmem_handle_t handle,
                                     bool take_mutex);
extern void gmem_lock_list_unlock_and_destroy_internal(struct gmem_lock_list *lock_list, bool take_mutex);

extern void *gmem_map_internal(gmem_handle_t handle, size_t offset, size_t length,
                               uint32_t usage_flags /* Some combination of GMEM_USAGE_CPU_READ/GMEM_USAGE_CPU_WRITE */, bool take_mutex);
extern void gmem_unmap_internal(gmem_handle_t handle);

#define BLOCK_SIZE (1 * 1024 * 1024)   /* MUST BE A MULTIPLE OF CPU PAGE LENGTH */
#define BLOCK_PAGE_SIZE (512)          /* MUST BE A MULTIPLE OF CPU PAGE LENGTH */
#define MAX_BLOCKS (16)
#define GUARD_TAG (0x3d78a53d)

typedef struct block {
   LIST_ENTRY(block) link;

   // allocation is BLOCK_SIZE divided into cache line pages
   gmem_handle_t           memory_handle;
   struct gmem_lock_list   lock_list;
   v3d_addr_t              cur_lock_phys;
   char                    *cur_map_ptr;

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

   VCOS_MUTEX_T mutex;
   VCOS_THREAD_T thread;
   VCOS_EVENT_T done;
} talloc;

typedef struct header {
   uint32_t guard;
   block *b;
   int num_of_pages;
} header;

static v3d_addr_t talloc_virt_to_offset(block *b, char *p)
{
   return (v3d_addr_t)b->cur_lock_phys + (p - b->cur_map_ptr);
}

static block *talloc_block(void)
{
   block *b = (block *)malloc(sizeof(block));
   if (b) {

      gmem_lock_list_init(&b->lock_list);

      b->memory_handle = gmem_alloc_internal(BLOCK_SIZE, BLOCK_PAGE_SIZE, GMEM_USAGE_ALL, "talloc_block");
      if (b->memory_handle == GMEM_HANDLE_INVALID)
         goto error0;

      b->cur_lock_phys = gmem_lock_internal(&b->lock_list, b->memory_handle, false);
      if (b->cur_lock_phys == 0)
         goto error1;

      b->cur_map_ptr = gmem_map_internal(b->memory_handle, 0, BLOCK_SIZE, GMEM_USAGE_CPU_WRITE, false);
      if (b->cur_map_ptr == NULL)
         goto error2;

      memset(b->usage, 0, sizeof(b->usage));
      b->free_tail =
         b->free_head = b->cur_map_ptr;
      b->free_bytes = BLOCK_SIZE;
      b->age = 0;
   }

   return b;

error2:
   gmem_lock_list_unlock_and_destroy_internal(&b->lock_list, false);

error1:
   gmem_free_internal(b->memory_handle);

error0:
   free(b);
   return NULL;
}

static void* talloc_thread_main(void *arg)
{
   talloc *handle = (talloc *)arg;
   block  *cleanup_list[MAX_BLOCKS];
   int    num_cleanups = 0;

   // kicks every second
   while (vcos_event_timed_wait(&handle->done, 1000) == VCOS_ETIMEDOUT) {
      block *next = NULL;

      vcos_mutex_lock(&handle->mutex);

#if 0
      for (block *b = LIST_FIRST(&handle->list); b; b = LIST_NEXT(b, link)) {
         selective_logd("block %p, free %d", b->cur_lock_phys, b->free_bytes);
      }
#endif

      if (handle->blocks > 1) {
         for (block *b = LIST_FIRST(&handle->list); b; b = next) {
            next = LIST_NEXT(b, link);
            // block is in a reset state.
            if (b->cur_map_ptr == b->free_head &&
               b->free_head == b->free_tail) {
               b->age--;
               if (b->age == 0) {

                  // Store the blocks we are going to remove
                  cleanup_list[num_cleanups] = b;
                  num_cleanups++;

                  handle->blocks--;
                  LIST_REMOVE(&handle->list, b, link);

                  // don't want to clean up the last block
                  if (handle->blocks == 1)
                     break;
               }
            }
         }
      }

      vcos_mutex_unlock(&handle->mutex);

      // Now we no longer have the mutex cleanup the blocks we don't want
      // (gmem functions might call back into talloc, so we must not have the mutex)
      for (int i = 0; i < num_cleanups; i++)
      {
         block *b = cleanup_list[i];

         gmem_unmap(b->memory_handle);
         gmem_lock_list_unlock_and_destroy(&b->lock_list);
         gmem_free(b->memory_handle);
         free(b);
      }

      num_cleanups = 0;
   }

   return NULL;
}

void *talloc_initialize(void)
{
   talloc *handle = (talloc *)malloc(sizeof(talloc));
   if (handle) {
      block *b;

      if (vcos_mutex_create(&handle->mutex, "tallocMutex") != VCOS_SUCCESS)
         goto exit0;

      if (vcos_event_create(&handle->done, "tallocDone") != VCOS_SUCCESS)
         goto exit1;

      if (vcos_thread_create(&handle->thread, "talloc", NULL, talloc_thread_main, handle) != VCOS_SUCCESS)
         goto exit2;

      vcos_mutex_lock(&handle->mutex);

      LIST_INIT(&handle->list);
      b = talloc_block();
      LIST_INSERT_HEAD(&handle->list, b, link);
      handle->blocks = 1;

      vcos_mutex_unlock(&handle->mutex);
   }

   return handle;

exit2:
   vcos_event_delete(&handle->done);

exit1:
   vcos_mutex_delete(&handle->mutex);

exit0:
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

      vcos_mutex_lock(&handle->mutex);

      for (block *b = LIST_FIRST(&handle->list); b; b = next) {

         gmem_unmap_internal(b->memory_handle);
         gmem_lock_list_unlock_and_destroy_internal(&b->lock_list, false);
         gmem_free_internal(b->memory_handle);

         next = LIST_NEXT(b, link);
         free(b);
      }

      vcos_mutex_unlock(&handle->mutex);
      vcos_mutex_delete(&handle->mutex);
   }
}

static bool talloc_alloc_internal(talloc *handle, size_t size, uint32_t align, void **cur_map_ptr, v3d_addr_t *cur_lock_phys)
{
   bool res = false;

   for (block *b = LIST_FIRST(&handle->list); b; b = LIST_NEXT(b, link))
   {
      // One extra block for header information
      int num_of_pages = ((size + BLOCK_PAGE_SIZE - 1) / BLOCK_PAGE_SIZE) + 1;
      size_t allocation_size = num_of_pages * BLOCK_PAGE_SIZE;

      if (allocation_size < b->free_bytes)
      {
         // fits, just allocate respective pages
         int page_offset = (b->free_head - b->cur_map_ptr) / BLOCK_PAGE_SIZE;
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
         *cur_map_ptr = (void *)(b->free_head + BLOCK_PAGE_SIZE);
         *cur_lock_phys = talloc_virt_to_offset(b, *cur_map_ptr);

         b->free_head = b->free_head + allocation_size;
         b->free_bytes -= allocation_size;

         res = true;

         break;
      }
   }

   if (*cur_map_ptr == NULL)
   {
      // allocation failed, grab new block and try again
      if (handle->blocks < (MAX_BLOCKS - 1))
      {
         block *b = talloc_block();
         if (b)
         {
            LIST_INSERT_HEAD(&handle->list, b, link);
            handle->blocks++;
            res = talloc_alloc_internal(handle, size, align, cur_map_ptr, cur_lock_phys);
         }
      }
   }

   return res;
}

// align is ignored to its maximum alignment which is BLOCK_PAGE_SIZE
bool talloc_alloc(void *h, size_t size, uint32_t align, void **cur_map_ptr, v3d_addr_t *cur_lock_phys)
{
   talloc *handle = (talloc *)h;
   bool res = false;

   // basic checks
   if (align > BLOCK_PAGE_SIZE ||
       size > BLOCK_SIZE)
      return false;

   if (handle) {
      vcos_mutex_lock(&handle->mutex);

      res = talloc_alloc_internal(handle, size, align, cur_map_ptr, cur_lock_phys);

      vcos_mutex_unlock(&handle->mutex);
   }

   return res;
}

void talloc_free(void *h, void *p)
{
   talloc *handle = (talloc *)h;

   if (handle) {
      char *allocation = (char *)p;

      vcos_mutex_lock(&handle->mutex);

      // check the allocation looks valid (aligned to page size)
      if ((char *)((uintptr_t)(allocation + (BLOCK_PAGE_SIZE - 1)) & ~(BLOCK_PAGE_SIZE - 1)) == allocation) {
         // rewind to the block header
         header *alloc_header = (header *)((uintptr_t)p - BLOCK_PAGE_SIZE);
         if (alloc_header->guard == GUARD_TAG) {
            block *b = alloc_header->b;
            alloc_header->guard = 0;
            int num_of_pages = alloc_header->num_of_pages;

            int page_offset = ((uintptr_t)alloc_header - (uintptr_t)b->cur_map_ptr) / BLOCK_PAGE_SIZE;
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
               int maxi = (b->free_head - b->cur_map_ptr) / BLOCK_PAGE_SIZE;
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
                  b->free_tail = b->cur_map_ptr;
               // reset the number of free bytes
               b->free_bytes = BLOCK_SIZE;
               // start the aging process - after b->age seconds of no use it will die
               b->age = 10;
            }
         }
      }

      vcos_mutex_unlock(&handle->mutex);
   }
}
