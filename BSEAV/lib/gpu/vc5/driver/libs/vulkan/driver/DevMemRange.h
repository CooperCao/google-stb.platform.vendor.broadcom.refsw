/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "libs/platform/gmem.h"
#include "libs/core/v3d/v3d_addr.h"

namespace bvk {

// Represents a sub-range of an allocated gmem_handle
class DevMemRange
{
public:
   DevMemRange() = default;

   DevMemRange(gmem_handle_t handle, v3d_addr_t phys, void *ptr, size_t size) :
      m_handle(handle), m_phys(phys), m_ptr(ptr), m_size(size)
   {}

   void Set(gmem_handle_t handle, v3d_addr_t phys, void *ptr, size_t size)
   {
      m_handle = handle;
      m_phys   = phys;
      m_ptr    = ptr;
      m_size   = size;
   }

   void Set(gmem_handle_t handle, size_t offset, size_t size)
   {
      m_handle = handle;
      m_phys   = gmem_get_addr(m_handle) + offset;
      m_ptr    = (void*)((uintptr_t)gmem_get_ptr(m_handle) + offset);
      m_size   = size;
   }

   gmem_handle_t Handle()  const { return m_handle; }
   v3d_addr_t    Phys()    const { return m_phys;   }
   void         *Ptr()     const { return m_ptr;    }
   size_t        Size()    const { return m_size;   }
   size_t        Offset()  const { return m_phys - gmem_get_addr(m_handle); }
   bool          IsValid() const { return m_handle != GMEM_HANDLE_INVALID;  }

   // Flush after writing by CPU
   void SyncMemory() const
   {
      if (m_handle != GMEM_HANDLE_INVALID)
         gmem_flush_mapped_range(m_handle, Offset(), m_size);
   }

   // Invalidate after writing by V3D
   void InvalidateMemory() const
   {
      if (m_handle != GMEM_HANDLE_INVALID)
         gmem_invalidate_mapped_range(m_handle, Offset(), m_size);
   }

private:
   gmem_handle_t  m_handle = GMEM_HANDLE_INVALID;
   v3d_addr_t     m_phys   = 0;
   void          *m_ptr    = nullptr;
   size_t         m_size   = 0;
};

} // namespace bvk
