/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  GPU Monitor Hook
Module   :  Circular buffer

FILE DESCRIPTION
Classes implementing a circular buffer with separation of data and meta data
=============================================================================*/

#ifndef __CIRCULAR_BUFFER_H__
#define __CIRCULAR_BUFFER_H__

#include <stdlib.h>
#include <stdint.h>
#include <list>
#include <vector>


// Class containing information to locate a block of data in the circular buffer
class DataBlockLocation
{
public:
   uint32_t       offset;
   uint8_t        size;
};

// This circular buffer has a vector and a queue:
// * the queue keeps track of the position of the data
// * the vector keeps the data itself
// This class provides the functionality of a circular buffer that grows
// up to a max size and contains blocks of data of different sizes
class CircularEventBuffer
{
public:
   CircularEventBuffer(uint32_t max_api_buffer_size   /*in bytes*/,
                       uint32_t alloc_chunk_size      /*in bytes*/);

   void reset();
   uint32_t *getPointerToWriteData(uint32_t requestedBytes);
   uint32_t size() const;

   // The beginning of the circular buffer might be somewhere in the middle of the buffer
   // so a function returns the data between the read index and the end of the buffer
   // and another function returns the data between the beginning of the vector and the
   // write index
   void *getFirstBufferPart(uint32_t *size);
   void *getSecondBufferPart(uint32_t *size);

   bool dataHasBeenLost() const
   {
      return m_lostData;
   }

   void setMaxSize(uint32_t max_size)
   {
      m_maxBufferSize = max_size * 1024 * 1024;
   }

   uint32_t getMaxSize() const
   {
      return m_maxBufferSize;
   }
private:
   uint32_t                            m_maxBufferSize;
   uint32_t                            m_allocChunkSize;
   std::vector<uint8_t>                m_dataBuffer;
   std::list<DataBlockLocation>        m_dataBlockReadLocations;
   uint32_t                            m_writeOffset;
   bool                                m_wrapped;
   bool                                m_lostData;
   short                               m_instance;          // Used for debug
   static short                        m_bufferInstance;
};

#endif // __CIRCULAR_BUFFER_H__
