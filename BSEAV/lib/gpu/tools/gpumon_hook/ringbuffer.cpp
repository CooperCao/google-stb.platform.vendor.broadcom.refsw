/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "ringbuffer.h"

#include "debuglog.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

RingBuffer::RingBuffer(size_t size):
   m_buffer(new uint8_t[size]),
   m_full(false),
   m_abort(false)
{
   m_end = m_buffer + size;
   m_read = m_buffer;
   m_write = m_buffer;
}

RingBuffer::~RingBuffer()
{
   Abort();
   delete [] m_buffer;
   m_buffer = nullptr;
}

size_t RingBuffer::Size()
{
   std::lock_guard<std::mutex> lock {m_mutex};
   return SizeInternal();
}

bool RingBuffer::Empty()
{
   std::lock_guard<std::mutex> lock {m_mutex};
   return EmptyInternal();
}

bool RingBuffer::Full()
{
   std::lock_guard<std::mutex> lock {m_mutex};
   return FullInternal();
}

void RingBuffer::Abort()
{
   {
      std::lock_guard<std::mutex> lock {m_mutex};
      m_abort = true;
   }
   m_cond.notify_all(); //wake up read/write/flush
}

void RingBuffer::Reset()
{
   {
      std::lock_guard<std::mutex> lock {m_mutex};
      m_read = m_write = m_buffer;
      m_abort = false;
   }
   m_cond.notify_all(); //wake up read/write/flush
}

size_t RingBuffer::SizeInternal() const
{
   if (FullInternal())
      return Capacity();
   else if (m_write >= m_read)
      return m_write - m_read;
   else //wrapped
      return Capacity() - (m_read - m_write);
}

size_t RingBuffer::Write(const void *data, size_t size, WriteAction writeAction)
{
   const uint8_t *ptr = reinterpret_cast<const uint8_t*>(data);
   size_t count = size;

   std::unique_lock<std::mutex> lock { m_mutex };
   while(count && !m_abort)
   {
      if (FullInternal())
      {
         if (writeAction == WriteBlock)
         {
            m_cond.notify_one();
            m_cond.wait(lock, [this]{ return !FullInternal() || m_abort; });
            if (m_abort)
               break; //the while loop
         }
         else if (writeAction == WriteFill)
         {
            break; //the while loop
         }
         else if (writeAction == WriteOver)
         {
            //contintinue writing
         }
      }

      if (m_read <= m_write && !FullInternal())
      {
         //read pointer is not ahead so we can write up to the end of the buffer
         size_t available = m_end - m_write;
         size_t bytes = count < available ? count : available;
         memcpy(m_write, ptr, bytes);
         m_write += bytes;
         ptr += bytes;
         count -= bytes;

         if (m_write == m_end) //reached the end - wrap around and continue
            m_write = m_buffer;
      }
      //now either read pointer is ahead so we can fill up to the read pointer
      //or there are no more data bytes remaining to write
      assert(m_read >= m_write || !count);

      if (count)
      {
         size_t available = (writeAction == WriteOver) ?
               m_end - m_write : m_read - m_write;
         size_t bytes = count < available ? count : available;
         memcpy(m_write, ptr, bytes);
         m_write += bytes;
         ptr += bytes;
         count -= bytes;
         if (m_read < m_write) //destroyed some unread data
         {
            assert(writeAction == WriteOver);
            m_read = m_write;
         }
      }

      if (ptr != data) //wrote some data so this buffer is not empty
      {
         if (m_write == m_read)
            m_full = true; //it's actually full

         //full or not, unblock Read(), if waiting for more data
         lock.unlock();
         m_cond.notify_one();
         lock.lock();
      }
   }
   return size - count;
}

bool RingBuffer::Flush()
{
   std::unique_lock<std::mutex> lock { m_mutex };
   m_cond.wait(lock, [this]{ return EmptyInternal() || m_abort; });
   return EmptyInternal();
}

size_t RingBuffer::Read(void *data, size_t size, ReadAction readAction)
{
   uint8_t *ptr = reinterpret_cast<uint8_t*>(data);
   size_t count = size;

   std::unique_lock<std::mutex> lock { m_mutex };
   while(count && !m_abort)
   {
      if (EmptyInternal())
      {
         if (readAction == ReadBlock)
         {
            m_cond.wait(lock, [this]{ return !EmptyInternal() || m_abort; });
            if (m_abort)
               break;  //the while loop
         }
         else
            break; //the while loop
      }

      assert(!EmptyInternal());

      if (m_read > m_write || m_full)
      {
         //buffer is and not empty and runs at least to the end, may wrap
         //consume the end of the buffer (before possible wrap)

         size_t available = m_end - m_read;
         size_t bytes = count < available ? count : available;
         memcpy(ptr, m_read, bytes);
         m_read += bytes;
         ptr += bytes;
         count -= bytes;
         if (m_read == m_end) //reached the end - wrap and continue
            m_read = m_buffer;
      }

      //the buffer is not wrapped and not full now, but it can be empty
      assert(m_read <= m_write || !count);
      if (count)
      {
         size_t available = m_write - m_read;
         size_t bytes = count < available ? count : available;
         memcpy(ptr, m_read, bytes);
         m_read += bytes;
         ptr += bytes;
         count -= bytes;
      }

      if (ptr != data) //read some data so this buffer is not full
      {
         m_full = false;
         lock.unlock();
         m_cond.notify_one();
         lock.lock();
      }
   }
   return size - count;
}
