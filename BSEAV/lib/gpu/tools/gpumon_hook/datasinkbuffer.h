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

/* A data buffer sink that has to be daisy-chained before another sink
 * in order to created a buffered sink.
 *
 * The Flush() writes all buffered data to a lower-level sink and flushes that
 * sink. The Flush() can be triggered internally by Write() when amount of data
 * written exceeds the remaining buffer capacity().
 *
 * This class is not thread safe.
 */
class DataSinkBuffer: public DataSink
{
public:
   DataSinkBuffer(size_t size, DataSink &sink);
   virtual ~DataSinkBuffer();
   DataSinkBuffer(const DataSinkBuffer &) = delete;
   DataSinkBuffer &operator=(const DataSinkBuffer &) = delete;

   size_t Capacity() const { return m_end - m_buffer; }
   size_t Size() const { return m_write - m_buffer; }
   bool Empty() const { return m_write == m_buffer; }
   bool Full() const { return m_write == m_end; }

public: //DataSink interface
   virtual size_t Write(const void *data, size_t size) override;
   virtual bool Flush() override;

private:
   DataSink &m_sink;
   uint8_t *m_buffer;
   const uint8_t *m_end;
   uint8_t *m_write;
};
