/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include "remote.h"
#include "platform.h"

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

class Archive : public Remote
{
public:
   Archive(const std::string &filename);
   virtual ~Archive();

   virtual bool Connect();
   virtual void Disconnect();

   virtual void Send(uint8_t *data, uint32_t size, bool isArray = false);
   virtual void Flush();

   uint64_t    BytesWritten() const { return m_bytesWritten; }

private:
   void BufferForWrite(uint8_t *data, uint32_t numBytes);
   void worker();
   std::string                         m_filename;
   FILE                                *m_fp;
   std::mutex                          m_mutex;
   std::condition_variable             m_condition;
   std::queue<std::vector<uint8_t>>    m_queue;
   std::vector<uint8_t>                m_buffer;
   uint64_t                            m_bytesWritten;
   std::thread                         m_thread;
   bool                                m_done;
};

#endif /* __ARCHIVE_H__ */
