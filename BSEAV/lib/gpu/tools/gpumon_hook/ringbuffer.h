/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include <stdint.h>

#include <condition_variable>
#include <mutex>
#include <vector>

/* Ring buffer that can serve both as data source and data sink.
 *
 * This class is thread safe.
 */
class RingBuffer
{
public:
   typedef enum
   {
      ReadBlock,    //block until all requested data is read
      ReadAvailable //don't block, read only data available now
   } ReadAction;

   typedef enum
   {
      WriteBlock, //block until all data is written
      WriteFill,  //don't block, only fill available space
      WriteOver   //don't block, overwrite unread data if necessary
   } WriteAction;

   RingBuffer(size_t size);
   ~RingBuffer();

   RingBuffer(const RingBuffer &) = delete;
   RingBuffer &operator=(const RingBuffer &) = delete;

   size_t Capacity() const { return m_end - m_buffer; }
   size_t Size();
   bool Empty();
   bool Full();

   void Abort(); //abort current and future Read/Write/Flush
   void Reset(); //reset buffer to initial empty state

   size_t Read(void *data, size_t size, ReadAction readAction);
   size_t Write(const void *data, size_t size, WriteAction writeAction);
   bool Flush(); //block until all data is read

private:
   bool EmptyInternal() const { return m_write == m_read && !m_full; }
   bool FullInternal() const { return m_write == m_read && m_full; }
   size_t SizeInternal() const;

private:
   uint8_t *m_buffer;
   const uint8_t *m_end;
   const uint8_t *m_read;
   uint8_t *m_write;
   bool m_full;
   bool m_abort;
   std::mutex m_mutex;
   std::condition_variable m_cond;
};
