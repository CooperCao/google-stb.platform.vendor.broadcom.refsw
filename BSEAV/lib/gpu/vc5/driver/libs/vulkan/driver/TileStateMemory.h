/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Common.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/platform/gmem.h"

#include <list>
#include <mutex>

#if !V3D_VER_AT_LEAST(4,1,34,0)

namespace bvk {

// This class manages tile-state memory for all GPU jobs.
// Since we never queue bin jobs in h/w, a single large piece of tile-state memory
// would be sufficient, but too large to be practical.
//
// This class implements a more useful management strategy.

class TileStateMemory
{
public:
   enum { AgeThreshold = 5 };

   ~TileStateMemory();

   // Get a tile state block large enough for minimumByteSize
   v3d_addr_t Acquire(size_t minimumByteSize);

   // Return a previously acquired block
   void Release(v3d_addr_t addr);

private:
   class TileStateBlock
   {
   public:
      TileStateBlock(size_t numBytes);
      ~TileStateBlock();

      void Age() { if (m_refCount == 0) m_age++; }
      void Reference() { m_refCount++; m_age = 0; }
      bool Dereference() { assert(m_refCount > 0);  m_refCount--; return m_refCount == 0; }

   public:
      gmem_handle_t  m_handle = GMEM_HANDLE_INVALID;
      v3d_addr_t     m_phys = 0;
      uint32_t       m_refCount = 0;
      size_t         m_numBytes = 0;
      uint32_t       m_age = 0;
   };

   // Free old unused blocks
   void RecycleOldBlocks();

   // Free any unused blocks
   void Recycle();

   std::mutex                  m_mutex;
   std::list<TileStateBlock>   m_blocks;
};

} // namespace bvk

#endif
