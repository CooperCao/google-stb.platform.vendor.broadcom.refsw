/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include <utility>

#include "libs/util/log/log.h"
#include "HeapManager.h"

LOG_DEFAULT_CAT("bvk::HeapManager");

namespace bvk {

void HeapManager::Initialize(size_t heapBytes, size_t allocAlignment)
{
   Reset();

   assert(gfx_size_is_power_of_2(allocAlignment));

   heapBytes = gfx_zround_up(heapBytes, allocAlignment);

   m_bytesFree = heapBytes;
   m_bytesUsed = 0;
   m_alignment = allocAlignment;

   // Create a single free block for the whole pool
   m_allChunks.emplace_back(nullptr, nullptr, 0, m_bytesFree);
   m_sizeOrderFreeMap.emplace(std::make_pair(m_bytesFree, &m_allChunks.back()));

   log_trace("[%p] Initialize %zd bytes", this, heapBytes);
   Log();
}

size_t HeapManager::Allocate(size_t numBytes, size_t allocAlignment)
{
   log_trace("[%p] Allocate %zu bytes, align = %zu", this, numBytes, allocAlignment);

   // In future we may want to respect individual alloc alignment, not the heap min alignment.
   // For now, just check that the minAlignment is ok.
   assert(allocAlignment <= m_alignment);

   numBytes = gfx_zround_up(numBytes, m_alignment);
   if (numBytes > m_bytesFree)
   {
      log_warn("[%p] Allocate of %zu bytes failed", this, numBytes);
      return OUT_OF_MEMORY;
   }

   // Finds the first block where the size is >= numBytes
   auto bestFitIter = m_sizeOrderFreeMap.lower_bound(numBytes);
   if (bestFitIter == m_sizeOrderFreeMap.end())
   {
      log_warn("[%p] Fragmentation failure allocating %zd bytes", this, numBytes);
      return FRAGMENTED;
   }

   Chunk *chunk = bestFitIter->second;
   if (numBytes < chunk->size)
   {
      // Create and link a new free chunk
      m_allChunks.emplace_back(chunk, chunk->next, chunk->start + numBytes, chunk->size - numBytes);
      Chunk *newFreeChunk = &m_allChunks.back();
      if (chunk->next)
         chunk->next->prev = newFreeChunk;
      chunk->next = newFreeChunk;
      m_sizeOrderFreeMap.emplace(std::make_pair(newFreeChunk->size, newFreeChunk));
   }

   // Remove the original chunk from free list
   m_sizeOrderFreeMap.erase(bestFitIter);

   chunk->size = numBytes;
   chunk->free = false;

   m_bytesUsed += numBytes;
   m_bytesFree -= numBytes;

   size_t ret = chunk->start;

   m_allocMap.emplace(std::make_pair(ret, chunk));
   Log();

   return ret;
}

void HeapManager::RemoveFreeMapChunk(Chunk *c)
{
   auto range = m_sizeOrderFreeMap.equal_range(c->size);

   for (auto i = range.first; i != range.second; ++i)
   {
      if (i->second == c)
      {
         m_sizeOrderFreeMap.erase(i);
         return;
      }
   }
}

bool HeapManager::MergeChunksIfFree(Chunk *c1, Chunk *c2)
{
   if (c1 != nullptr && c2 != nullptr)
   {
      if (c1->free && c2->free)
      {
         // Merge into c1
         c1->size += c2->size;
         c1->start = std::min(c1->start, c2->start);

         if (c1->next == c2)
         {
            c1->next = c2->next;
            if (c1->next != nullptr)
               c1->next->prev = c1;
         }
         if (c1->prev == c2)
         {
            c1->prev = c2->prev;
            if (c1->prev != nullptr)
               c1->prev->next = c1;
         }

         RemoveFreeMapChunk(c2);
         m_allChunks.remove(*c2);

         return true;
      }
   }

   return false;
}

void HeapManager::Free(size_t offset)
{
   log_trace("[%p] Free at offset %zd", this, offset);

   Chunk *chunk = m_allocMap[offset];
   assert(chunk != nullptr);

   chunk->free = true;
   m_bytesFree += chunk->size;
   m_bytesUsed -= chunk->size;

   // Merge with earlier or later free block if possible
   Chunk *prev = chunk->prev;
   Chunk *next = chunk->next;

   MergeChunksIfFree(chunk, prev);
   MergeChunksIfFree(chunk, next);

   // Remove from alloc map
   m_allocMap.erase(offset);

   // Add to free list
   m_sizeOrderFreeMap.emplace(std::make_pair(chunk->size, chunk));

   Log();
}

void HeapManager::Reset()
{
   log_trace("[%p] Reset", this);

   m_allChunks.clear();
   m_allocMap.clear();
   m_sizeOrderFreeMap.clear();

   size_t totalBytes = m_bytesUsed + m_bytesFree;
   m_bytesUsed = 0;
   m_bytesFree = totalBytes;

   if (totalBytes > 0)
   {
      m_allChunks.emplace_back(nullptr, nullptr, 0, m_bytesFree);
      m_sizeOrderFreeMap.emplace(std::make_pair(m_bytesFree, &m_allChunks.back()));
   }

   Log();
}

void HeapManager::Log() const
{
   if (log_trace_enabled())
   {
      log_trace("==================================================");
      log_trace("[%p] MemoryHeap (%s)", this, m_debugName ? m_debugName : "unnamed");
      log_trace("==================================================");
      log_trace("Used bytes  = %zu", m_bytesUsed);
      log_trace("Free bytes  = %zu", m_bytesFree);

      if (m_allChunks.size() > 0)
      {
         const Chunk *c = &m_allChunks.front();
         while (c != nullptr)
         {
            log_trace("  Chunk %p : start=%zu, size=%zu, %s", c, c->start, c->size, c->free ? "FREE" : "USED");
            c = c->next;
         }
      }

      log_trace("Allocated chunks:");
      for (const auto &fc : m_allocMap)
      {
         Chunk *c = fc.second;
         log_trace("  Chunk %p : start=%zu, size=%zu, %s", c, c->start, c->size, c->free ? "FREE" : "USED");
      }

      log_trace("Free chunks:");
      for (const auto &fc : m_sizeOrderFreeMap)
      {
         Chunk *c = fc.second;
         log_trace("  Chunk %p : start=%zu, size=%zu, %s", c, c->start, c->size, c->free ? "FREE" : "USED");
      }
      log_trace(" ");
   }
}

} // namespace bvk
