/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  GPU monitor hook
Module   :  Circular buffer

FILE DESCRIPTION
Classes implementing a circular buffer with separation of data and meta data
Helper class to store events before sending them to GPUMonitor App
=============================================================================*/

#include "circularbuffer.h"

short CircularEventBuffer::m_bufferInstance = 0;

CircularEventBuffer::CircularEventBuffer(uint32_t max_api_buffer_size   /*in bytes*/,
                                         uint32_t alloc_chunk_size      /*in bytes*/):
   m_maxBufferSize(max_api_buffer_size),
   m_allocChunkSize(alloc_chunk_size),
   m_dataBuffer(alloc_chunk_size),
   m_dataBlockReadLocations(alloc_chunk_size),
   m_writeOffset(0),
   m_wrapped(false),
   m_lostData(false),
   m_instance(++m_bufferInstance)
{
}

void CircularEventBuffer::reset()
{
   m_writeOffset = 0;
   m_dataBuffer.resize(0);
   m_wrapped = false;
   m_lostData = false;
   m_dataBlockReadLocations.resize(0);
}

uint32_t *CircularEventBuffer::getPointerToWriteData(uint32_t requestedBytes)
{
   if (requestedBytes > m_maxBufferSize)
      return NULL;

   uint32_t nextWriteOffset = 0;
   // Resize if the vector too small
   if ((m_writeOffset + requestedBytes) > m_dataBuffer.size())
   {
      if ((m_dataBuffer.size() + requestedBytes) > m_dataBuffer.capacity())
      {
         if (m_dataBuffer.size() + requestedBytes + m_allocChunkSize < m_maxBufferSize)
         {
            // Vector too small but it can grow
            m_dataBuffer.reserve(m_dataBuffer.size() + requestedBytes + m_allocChunkSize);
            m_dataBuffer.resize(m_dataBuffer.size() + requestedBytes);  // Resize the vector to have the space for bytes
         }
         else
         {
            // Vector too small but it can't grow anymore so wrapping around
            m_wrapped = true;

            // Anything after the last offset (m_writeOffset) becomes invalid
            // Remove any read after the last write offset
            while(!m_dataBlockReadLocations.empty() &&
                  (m_writeOffset <= m_dataBlockReadLocations.front().offset))
            {
               m_dataBlockReadLocations.pop_front();
            }

            // If the front wrapped around too carry on moving it further
            // to leave space for the new data
            if (m_dataBlockReadLocations.front().offset == 0)
            {
               while(requestedBytes > m_dataBlockReadLocations.front().offset)
                  m_dataBlockReadLocations.pop_front();
            }

            // If the buffer is big enough for all the requested data
            if (requestedBytes < m_writeOffset)
               // Resize the buffer to the last write
               m_dataBuffer.resize(m_writeOffset);
            else
            {
               // otherwise make it big enough
               if (requestedBytes > m_dataBuffer.capacity())
                  m_dataBuffer.reserve(requestedBytes);

               m_dataBuffer.resize(requestedBytes);
            }

            m_writeOffset = 0;
            m_lostData = true;
         }
      }
      else
      {
         m_dataBuffer.resize(m_dataBuffer.size() + requestedBytes);  // Resize the vector to have the space for bytes
      }
   }

   nextWriteOffset = m_writeOffset + requestedBytes;

   // Add this write as a new read at the back
   DataBlockLocation tmp;
   tmp.offset = m_writeOffset;
   tmp.size = requestedBytes;
   m_dataBlockReadLocations.push_back(tmp);

   if (m_wrapped)
   {
      // Remove invalid read from the front
      while(nextWriteOffset > m_dataBlockReadLocations.front().offset)
      {
         m_dataBlockReadLocations.pop_front();

         // If the buffer is wrapped the offset 0 always points to valid data
         if (m_dataBlockReadLocations.front().offset == 0)
         {
            m_wrapped = false;
            break;
         }
      }
   }

   uint32_t *pointerToWrite = reinterpret_cast<uint32_t *> (&m_dataBuffer[m_writeOffset]);
   m_writeOffset = nextWriteOffset;

   return pointerToWrite;
}

uint32_t CircularEventBuffer::size() const
{
   if (!m_wrapped)
      return m_dataBuffer.size();
   else
   {
      uint32_t readOffset = m_dataBlockReadLocations.front().offset;
      if (m_writeOffset <= readOffset)
         return (m_dataBuffer.size() - (readOffset - m_writeOffset));
      else
         return m_writeOffset - readOffset;
   }
}

void *CircularEventBuffer::getFirstBufferPart(uint32_t *size)
{
   uint32_t readOffset = m_dataBlockReadLocations.front().offset;
   *size = m_dataBuffer.size() - readOffset;
   return &m_dataBuffer[readOffset];
}

void *CircularEventBuffer::getSecondBufferPart(uint32_t *size)
{
   uint32_t readOffset = m_dataBlockReadLocations.front().offset;
   if (readOffset >= m_writeOffset)
      *size = m_writeOffset;
   else
      *size = 0;
   return &m_dataBuffer[0];
}
