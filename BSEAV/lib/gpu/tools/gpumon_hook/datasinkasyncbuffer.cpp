/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "datasinkasyncbuffer.h"

#include "debuglog.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

DataSinkAsyncBuffer::DataSinkAsyncBuffer(size_t size, DataSink &sink,
      size_t threshold, uint32_t timeout):
   m_sink(sink),
   m_ringBuffer(size),
   m_stop(false),
   m_flush(threshold, timeout)
{
   m_read.resize(size);
   m_thread = std::thread(&DataSinkAsyncBuffer::Run, this);
}

DataSinkAsyncBuffer::~DataSinkAsyncBuffer()
{
   WaitFlush();

   m_stop = true;
   m_cond.notify_one(); //wake up the thread
   m_thread.join();
}

size_t DataSinkAsyncBuffer::Write(const void *data, size_t size)
{
   size_t bytes = m_ringBuffer.Write(data, size, RingBuffer::WriteBlock);
   if (bytes)
      m_cond.notify_one();
   return bytes;
}

bool DataSinkAsyncBuffer::Flush()
{
   std::unique_lock<std::mutex> lock { m_mutex };
   if (!m_ringBuffer.Empty())
   {
      m_flush.m_now = true;
      lock.unlock();
      m_cond.notify_one();
   }
   return true;
}

bool DataSinkAsyncBuffer::WaitFlush()
{
   return m_ringBuffer.Flush();
}

void DataSinkAsyncBuffer::SetAutoFlush(size_t threshold, uint32_t timeout)
{
   {
      std::lock_guard<std::mutex> lock {m_mutex};
      m_flush.m_threshold = threshold;
      m_flush.m_timeout = timeout;
      //keep m_flush.now - we don't want to cancel a pending Flush() request
   }
   m_cond.notify_one(); //use new auto-flush settings now
}

void DataSinkAsyncBuffer::Run()
{
   do
   {
      std::unique_lock<std::mutex> lock { m_mutex };

      auto go = [this] { return m_flush.m_now || m_stop ||
               (m_flush.m_threshold ?
                     m_ringBuffer.Size() >= m_flush.m_threshold :
                     !m_ringBuffer.Empty()); };

      if (m_flush.m_timeout)
         m_cond.wait_for(lock, std::chrono::milliseconds(m_flush.m_timeout), go);
      else
         m_cond.wait(lock, go);
      if (m_ringBuffer.Empty())
         continue; //timed out or ending without any data

      // keep going even if we're about to m_stop

      //unlock before writing so that empty buffer space can be filled
      //while we're writing out the already occupied part
      lock.unlock();
      assert(m_ringBuffer.Size() <= m_read.size());
      size_t bytesRead = m_ringBuffer.Read(m_read.data(), m_ringBuffer.Size(),
            RingBuffer::ReadBlock);
      size_t bytesWritten = m_sink.Write(m_read.data(), bytesRead);
      lock.lock();

      if(m_flush.m_now)
      {
         m_flush.m_now = false;
         if (!m_sink.Flush())
            break;
      }

      if (bytesRead != bytesWritten) // oops! m_sink didn't write all data
         break;
   } while (!m_stop);

   if (!m_ringBuffer.Empty())
   {
      debug_log(DEBUG_ERROR, "ERROR writing buffered data: %s (%d)\n",
            strerror(errno), errno);
      m_stop = true;
   }
}
