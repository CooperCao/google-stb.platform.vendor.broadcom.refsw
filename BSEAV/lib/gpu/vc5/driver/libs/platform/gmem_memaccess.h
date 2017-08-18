/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifdef __cplusplus

#include "gmem.h"
#include "libs/sim/simcom/simcom_memaccess.h"
#include "libs/util/demand.h"
#include <map>

class gmem_memaccess_ro : public simcom_memaccess
{
public:
   gmem_memaccess_ro()
      :  simcom_memaccess()
   {
      this->p = this;
      this->read = read_fn;
   }

private:
   gmem_memaccess_ro(gmem_memaccess_ro const&) = delete;
   gmem_memaccess_ro& operator=(gmem_memaccess_ro const&) = delete;

   void read_fn(void *dst, v3d_addr_t read_addr, size_t read_size)
   {
      // Find gmem buffer with largest addr <= read_addr.
      auto upper = m_buffers.upper_bound(read_addr);
      gmem_handle_t handle = upper != m_buffers.begin() ? std::prev(upper)->second : NULL;

      // Check read_addr actually lies within this buffer.
      if (!handle || read_addr >= gmem_get_addr(handle)+gmem_get_size(handle))
      {
         // Obtain handle from gmem.
         handle = gmem_find_handle_by_addr(read_addr);
         demand(handle);

         // Record the buffer.
         m_buffers[gmem_get_addr(handle)] = handle;

         // Map and sync.
         demand(gmem_map_and_get_ptr(handle));
         gmem_invalidate_mapped_buffer(handle);
      }

      v3d_addr_t buf_addr = gmem_get_addr(handle);

      // Accesses spanning buffers are not allowed.
    #ifndef NDEBUG
      size_t read_end = read_addr + read_size;
      size_t buf_end = buf_addr + gmem_get_size(handle);
      assert(read_addr >= buf_addr);
      assert(read_end <= buf_end);
    #endif

      // Get CPU ptr for address and copy data.
      v3d_size_t offset = read_addr - buf_addr;
      void* ptr = gmem_get_ptr(handle);
      memcpy(dst, (char*)ptr + offset, read_size);
   }

   static void read_fn(void *dst, v3d_addr_t read_addr, size_t read_size,
         const char *file, uint32_t line, const char *func, void *p)
   {
      ((gmem_memaccess_ro*)p)->read_fn(dst, read_addr, read_size);
   }

private:
   std::map<v3d_addr_t, gmem_handle_t> m_buffers;
};

#endif
