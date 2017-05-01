/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "datasinkbuffer.h"

#include "debuglog.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

DataSinkBuffer::DataSinkBuffer(size_t size, DataSink &sink):
   m_sink(sink),
   m_buffer(new uint8_t[size])
{
   m_end = m_buffer + size;
   m_write = m_buffer;
}

DataSinkBuffer::~DataSinkBuffer()
{
   Flush();

   delete [] m_buffer;
   m_buffer = nullptr;
}

size_t DataSinkBuffer::Write(const void *data, size_t size)
{
   const uint8_t *ptr = reinterpret_cast<const uint8_t*>(data);
   size_t count = size;

   while(count)
   {
      size_t bytesLeft = Capacity() - Size();
      size_t bytes = count < bytesLeft ? count : bytesLeft;
      memcpy(m_write, ptr, bytes);
      m_write += bytes;
      ptr += bytes;
      count -= bytes;

      if (count && !Flush())
         break;
   }
   return size - count;
}

bool DataSinkBuffer::Flush()
{
   bool ok = true;
   size_t bytes = m_sink.Write(m_buffer, Size());
   if (bytes != Size())
      ok = false;
   if (!m_sink.Flush())
      ok = false;

   if (!ok)
      debug_log(DEBUG_ERROR, "ERROR writing buffered data: %s (%d)\n",
            strerror(errno), errno);

   m_write = m_buffer;
   return ok;
}
