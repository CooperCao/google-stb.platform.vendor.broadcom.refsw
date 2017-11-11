/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Allocator.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include <algorithm>
#include <cstdlib>

namespace bvk {

#ifndef NDEBUG
std::atomic_uint_fast32_t APIScoper::m_apiEntries {};
#endif

static void *VKAPI_PTR s_defaultAlloc(
   void  *pUserData,
   size_t size,
   size_t alignment,
   VkSystemAllocationScope allocationScope)
{
#ifdef WIN32
   return _aligned_malloc(size, alignment);
#else
   // Alignment will be a power of two, but posix_memalign also needs it to be
   // at least sizeof(void*)
   void *ptr = nullptr;
   int ret = posix_memalign(&ptr, std::max(alignment, sizeof(void*)), size);
   return ret == 0 ? ptr : nullptr;
#endif
}

static void *VKAPI_PTR s_defaultRealloc(
   void  *pUserData,
   void  *pOriginal,
   size_t size,
   size_t alignment,
   VkSystemAllocationScope allocationScope)
{
#ifdef WIN32
   return _aligned_realloc(pOriginal, size, alignment);
#else
   // We never actually use realloc from the driver, so this is just a dumb
   // alloc-copy-free implementation.
   void *ptr = nullptr;

   if (size > 0)
   {
      int ret = posix_memalign(&ptr, std::max(alignment, sizeof(void*)), size);
      if (ret != 0)
         return nullptr;

      if (pOriginal != nullptr)
      {
         size_t oldSize = malloc_usable_size(pOriginal);
         memcpy(ptr, pOriginal, oldSize);
      }
   }

   free(pOriginal);
   return ptr;
#endif
}

static void VKAPI_PTR s_defaultFree(
   void* pUserData,
   void* pMemory)
{
#ifdef WIN32
   _aligned_free(pMemory);
#else
   free(pMemory);
#endif
}

VkAllocationCallbacks g_defaultAllocCallbacks =
{
   nullptr, // pUserData
   s_defaultAlloc,
   s_defaultRealloc,
   s_defaultFree,
   nullptr,
   nullptr
};

} // namespace bvk
