/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include "remote.h"
#include "platform.h"
#include "datasink.h"
#include "datasourcesinkfile.h"
#include "datasinkasyncbuffer.h"

#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <streambuf>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

#include <stdio.h>

class Archive : public DataSink
{
public:
   Archive();
   virtual ~Archive();

   bool Open(const char *filename);
   void Close();

   uint64_t    BytesWritten() const { return m_bytesWritten; }

public: //DataSink interface
   virtual size_t Write(const void *data, size_t size) override;
   virtual bool Flush() override;

private:
   void BufferForWrite(uint8_t *data, uint32_t numBytes);
   void worker();
   int                                 m_fd;
   DataSourceSinkFile                  m_file;
   DataSinkAsyncBuffer                 m_buffer;
   uint64_t                            m_bytesWritten;
};
