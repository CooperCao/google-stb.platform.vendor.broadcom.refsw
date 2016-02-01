/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Fixed-size block allocator for small Vertex Buffers

FILE DESCRIPTION
To avoid round trip cost incurred while allocating new UMEMs
This API provides per context storage for small Vextex Buffer Objects
within a single UMEM
=============================================================================*/
#define VCOS_LOG_CATEGORY (&log_cat)

#include "middleware/khronos/glxx/glxx_small_buffer_pool.h"
#include "vcos.h"

static VCOS_LOG_CAT_T log_cat = VCOS_LOG_INIT("glxx_small_buffers", VCOS_LOG_WARN);

static void initialise(GLXX_SMALL_BUFFER_POOL_T *pool)
{
   uint32_t i;
   pool->initialised = true;
   for (i = 0; i < GLXX_SMALL_BUFFER_COUNT-1; i++)
      pool->list[i] = i + 1;

   pool->list[GLXX_SMALL_BUFFER_COUNT-1] = GLXX_SMALL_BUFFER_HANDLE_INVALID;
   pool->first = 0;
   pool->last = GLXX_SMALL_BUFFER_COUNT-1;

   pool->ustorage = khrn_umem_alloc(GLXX_SMALL_BUFFER_COUNT * GLXX_SMALL_BUFFER_SIZE, KHRN_UMEM_TYPE_VERTEX);
   vcos_log_trace("[%s]", __FUNCTION__);
}

void glxx_small_buffer_pool_term(void *v, uint32_t size)
{
   GLXX_SMALL_BUFFER_POOL_T *pool = (GLXX_SMALL_BUFFER_POOL_T *)v;
   if (pool->ustorage != KHRN_UMEM_HANDLE_INVALID)
      khrn_umem_wait_release(pool->ustorage);

   UNUSED(size);
}

GLXX_SMALL_BUFFER_HANDLE_T glxx_small_buffer_pool_alloc(GLXX_SMALL_BUFFER_POOL_T *pool, uint32_t size)
{
   GLXX_SMALL_BUFFER_HANDLE_T handle = GLXX_SMALL_BUFFER_HANDLE_INVALID;
   assert(size < GLXX_SMALL_BUFFER_SIZE);

   if (pool != NULL)
   {
      if (!pool->initialised)
         initialise(pool);

      if (pool->ustorage != KHRN_UMEM_HANDLE_INVALID && pool->first != GLXX_SMALL_BUFFER_HANDLE_INVALID)
      {
         handle = pool->first;
         pool->first = pool->list[handle];
         if (pool->first == GLXX_SMALL_BUFFER_HANDLE_INVALID)
            pool->last = GLXX_SMALL_BUFFER_HANDLE_INVALID;
         pool->list[handle] = GLXX_SMALL_BUFFER_HANDLE_INVALID;
         vcos_log_trace("[%s] %d size %d next is %d", __FUNCTION__, handle, size, pool->first);
      }
      else
         vcos_log_trace("[%s] size %d pool is full", __FUNCTION__, size);
   }

   return handle;
}

void glxx_small_buffer_pool_free(GLXX_SMALL_BUFFER_POOL_T *pool, GLXX_SMALL_BUFFER_HANDLE_T handle)
{
   assert(pool != NULL);
   assert(pool->initialised);
   assert(pool->ustorage != KHRN_UMEM_HANDLE_INVALID);
   assert(handle != GLXX_SMALL_BUFFER_HANDLE_INVALID && handle < GLXX_SMALL_BUFFER_COUNT);

   vcos_log_trace("[%s] %d next is %d", __FUNCTION__, handle, pool->first);

   if (pool->last == GLXX_SMALL_BUFFER_HANDLE_INVALID)
   {
      assert(pool->first == GLXX_SMALL_BUFFER_HANDLE_INVALID);
      pool->first = handle;
   }
   else
   {
      assert(pool->list[pool->last] == GLXX_SMALL_BUFFER_HANDLE_INVALID);
      pool->list[pool->last] = handle;
   }
   pool->last = handle;
   pool->list[handle] = GLXX_SMALL_BUFFER_HANDLE_INVALID;
}
