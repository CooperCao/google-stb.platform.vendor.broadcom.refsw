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
      buffer *buf = upper != m_buffers.begin() ? &std::prev(upper)->second : NULL;

      // Check read_addr actually lies within this buffer.
      if (!buf || read_addr >= gmem_get_addr(buf->handle)+gmem_get_size(buf->handle))
      {
         // Obtain handle from gmem.
         gmem_handle_t handle = gmem_find_handle_by_addr(read_addr);
         demand(handle);

         // Record the buffer.
         buf = &m_buffers[gmem_get_addr(handle)];
         buf->handle = handle;

         if (gmem_has_bidi_sync())
         {
            // Map and sync.
            demand(gmem_map_and_get_ptr(handle));
            gmem_bidi_sync_mapped_buffer(handle);
         }
         else
         {
            // No bidi sync. We want the data that V3D sees, but we can't just
            // sync this to the host as doing so could lose unflushed host
            // writes. Instead we use a special gmem API that allows us to see
            // memory as V3D does without disturbing anything. Note that we
            // need to malloc a buffer to store this data.
            buf->data = std::unique_ptr<void, deleter>(
               ::operator new(gmem_get_size(handle)));
            gmem_read_buffer_as_v3d(buf->data.get(), handle);
         }
      }

      v3d_addr_t buf_addr = gmem_get_addr(buf->handle);

      // Accesses spanning buffers are not allowed.
    #ifndef NDEBUG
      size_t read_end = read_addr + read_size;
      size_t buf_end = buf_addr + gmem_get_size(buf->handle);
      assert(read_addr >= buf_addr);
      assert(read_end <= buf_end);
    #endif

      // Get CPU ptr for address and copy data.
      v3d_size_t offset = read_addr - buf_addr;
      void* ptr = buf->data ? buf->data.get() : gmem_get_ptr(buf->handle);
      memcpy(dst, (char*)ptr + offset, read_size);
   }

   static void read_fn(void *dst, v3d_addr_t read_addr, size_t read_size,
         const char *file, uint32_t line, const char *func, void *p)
   {
      try
      {
         ((gmem_memaccess_ro*)p)->read_fn(dst, read_addr, read_size);
      }
      catch (const std::exception &e)
      {
         demand_msg(0, "%s", e.what());
      }
   }

private:
   struct deleter
   {
      void operator()(void *p)
      {
         ::operator delete(p);
      }
   };

   struct buffer
   {
      gmem_handle_t handle;
      std::unique_ptr<void, deleter> data;
   };

   std::map<v3d_addr_t, buffer> m_buffers;
};

#endif
