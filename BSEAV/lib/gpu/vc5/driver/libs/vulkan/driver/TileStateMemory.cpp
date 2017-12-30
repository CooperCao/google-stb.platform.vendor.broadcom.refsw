/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "TileStateMemory.h"
#include "libs/util/log/log.h"
#include "libs/platform/v3d_scheduler.h"

#if !V3D_VER_AT_LEAST(4,1,34,0)

LOG_DEFAULT_CAT("bvk::TileStateMemory");

namespace bvk {

// Inner helper
TileStateMemory::TileStateBlock::TileStateBlock(size_t numBytes) :
   m_numBytes(numBytes)
{
   m_handle = gmem_alloc(numBytes, V3D_TILE_STATE_ALIGN, GMEM_USAGE_V3D_RW, "tileState");
   if (m_handle == GMEM_HANDLE_INVALID)
      throw bad_device_alloc();

   m_phys = gmem_get_addr(m_handle);
   assert(m_phys != 0);

   log_trace("New block (addr=0x%08x, size=%zu)", m_phys, m_numBytes);
}

TileStateMemory::TileStateBlock::~TileStateBlock()
{
   if (m_handle != GMEM_HANDLE_INVALID)
   {
      gmem_free(m_handle);

      log_trace("Free block (addr=0x%08x, size=%zu)", m_phys, m_numBytes);
   }
}

// TileStateMemory
//
// This is mutex protected as it's effectively a singleton by being owned by
// physicalDevice. Multiple logical devices could potentially access it
// simultaneously.
TileStateMemory::~TileStateMemory()
{
   std::lock_guard<std::mutex> lock(m_mutex);

   Recycle();
   assert(m_blocks.size() == 0);
}

v3d_addr_t TileStateMemory::Acquire(size_t minimumByteSize)
{
   TileStateBlock *best = nullptr;

   std::lock_guard<std::mutex> lock(m_mutex);

   if (v3d_scheduler_get_hub_identity()->num_cores > 1 ||
       m_blocks.size() == 0 || minimumByteSize > m_blocks.back().m_numBytes)
   {
      // No blocks large enough, make a new one.
      // New (larger) blocks are always put at the end, so the list is always in
      // size order.
      m_blocks.emplace_back(minimumByteSize);
      best = &m_blocks.back();

      for (TileStateBlock &tsb : m_blocks)
         tsb.Age();
   }
   else
   {
      // Find smallest block in which we fit.
      // The list is ordered from smallest to largest, so the first we find is best.
      for (TileStateBlock &tsb : m_blocks)
      {
         tsb.Age();

         if (tsb.m_numBytes >= minimumByteSize)
            best = &tsb;
      }
   }

   assert(best != nullptr);

   best->Reference();

   log_trace("Acquired block (addr=0x%08x, size=%zu, refCount=%u)",
                  best->m_phys, best->m_numBytes, best->m_refCount);

   return best->m_phys;
}

void TileStateMemory::Release(v3d_addr_t addr)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   for (auto tsb = m_blocks.begin(); tsb != m_blocks.end(); ++tsb)
   {
      if (tsb->m_phys == addr)
      {
         tsb->Dereference();

         log_trace("Released block (addr=0x%08x, size=%zu, refCount=%u)",
                           tsb->m_phys, tsb->m_numBytes, tsb->m_refCount);
      }
   }

   RecycleOldBlocks();
}

// Private
void TileStateMemory::RecycleOldBlocks()
{
   // Remove all unused blocks older than threshold
   for (auto tsb = m_blocks.begin(); tsb != m_blocks.end(); )
   {
      log_trace("List: Active block (addr=0x%08x, size=%zu, refCnt=%u, age=%u)",
                 tsb->m_phys, tsb->m_numBytes, tsb->m_refCount, tsb->m_age);

      if (tsb->m_refCount == 0 && tsb->m_age > AgeThreshold)
         tsb = m_blocks.erase(tsb);
      else
         ++tsb;
   }
}

// Private
void TileStateMemory::Recycle()
{
   // Remove all blocks not in use
   for (auto tsb = m_blocks.begin(); tsb != m_blocks.end(); )
   {
      if (tsb->m_refCount == 0)
         tsb = m_blocks.erase(tsb);
      else
         ++tsb;
   }
}

} // namespace bvk

#endif
