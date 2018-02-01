/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include "datasink.h"

#include <stdint.h>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include "ringbuffer.h"

/* Asynchronous data buffer sink that has to be daisy-chained before
 * another sink in order to created a buffered sink.
 *
 * Flushing the buffered data to the attached sink is done from a background
 * thread, possibly in parallel to writes to this buffer, i.e. flushing
 * half-full buffer allows for filling up the other half in parallel.
 *
 * The Write() will block if the write size exceeds the remaining capacity.
 * As soon as the background Flush() frees up some space, the Write() will
 * resume.
 *
 * The background flush can be triggered manually by calling Flush() or
 * automatically when either of the auto-flush criteria is met.
 *
 * This class is thread safe.
 */
class DataSinkAsyncBuffer: public DataSink
{
public:
   DataSinkAsyncBuffer(size_t size, DataSink &sink,
         size_t threshold, uint32_t timeout);
   virtual ~DataSinkAsyncBuffer();
   DataSinkAsyncBuffer(const DataSinkAsyncBuffer &) = delete;
   DataSinkAsyncBuffer &operator=(const DataSinkAsyncBuffer &) = delete;

   size_t Capacity() const { return m_ringBuffer.Capacity(); }
   size_t Size() { return m_ringBuffer.Size(); }
   void SetAutoFlush(size_t threshold, uint32_t timeout);
   bool WaitFlush();

public: //DataSink interface
   virtual size_t Write(const void *data, size_t size) override;
   virtual bool Flush() override;

private:
   void Run();

private:
   DataSink &m_sink;
   RingBuffer m_ringBuffer;
   std::vector<uint8_t> m_read;
   bool m_stop;

   struct AutoFlush
   {
      size_t m_threshold;
      uint32_t m_timeout;
      bool m_now;
      AutoFlush(size_t threshold = 0, uint32_t timeout = 0):
         m_threshold(threshold), m_timeout(timeout), m_now(false) {}
   } m_flush;

   std::thread m_thread;
   std::mutex m_mutex;
   std::condition_variable m_cond;
};
