/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#include "archive.h"
#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <assert.h>

#ifdef ANDROID
#include <cutils/log.h>
#include <cutils/properties.h>
#define printf ALOGD
#define Error0(s) ALOGD(s)
#define Error1(s, a) ALOGD(s, a)
#define Log1(s, a) ALOGD(s, a)
#define Log2(s, a, b) ALOGD(s, a, b)
#else
#define Error0(s) fprintf(stderr, s)
#define Error1(s, a) fprintf(stderr, s, a)
#define Log1(s, a) printf(s, a)
#define Log2(s, a, b) printf(s, a, b)
#endif

#ifdef BIG_ENDIAN_CPU
#define TO_LE_W(w) \
{ \
   uint32_t tmp = *((uint32_t*)&w);\
   uint8_t *t = (uint8_t*)&tmp;\
   uint8_t *p = (uint8_t*)&w;\
   p[0] = t[3];\
   p[1] = t[2];\
   p[2] = t[1];\
   p[3] = t[0];\
}
#else
#define TO_LE_W(w)
#endif

#define CHUNK_SIZE (8 * 1024 * 1024)

#define USE_MEMCPY 1

// This is all a bit nasty.
//
// Since we are buffering capture data in a separate thread, anything that terminates
// uncleanly (without deleting the archive object) won't write anything if it's data
// hasn't reach 8MB yet, and in general will lose the end of the capture data.
// We store the last archive to be made (there should only ever be one) and ensure it
// cleans up properly and flushes the archive in an onexit handler.
//
static Archive *s_archive = NULL;

static void exitHandler()
{
   Archive *deleteMe = s_archive;
   s_archive = NULL;

   printf("Flushing capture archive on exit\n");

   if (deleteMe != NULL)
   {
      // Flush the archive and clean up
      delete deleteMe;
   }
}

void Archive::BufferForWrite(uint8_t *data, uint32_t numBytes)
{
   while (numBytes)
   {
      uint32_t remaining = m_buffer.capacity() - m_buffer.size();

      uint32_t copySize = std::min(numBytes, remaining);

#if USE_MEMCPY
      m_buffer.resize(m_buffer.size() + copySize);
      memcpy(&m_buffer[m_buffer.size() - copySize], data, copySize);
#else
      m_buffer.insert(m_buffer.end(), data, &data[copySize]);
#endif

      if (m_buffer.capacity() - m_buffer.size() == 0)
      {
         // scope for the lock
         {
            std::unique_lock<std::mutex> guard(m_mutex);
            m_queue.push(std::move(m_buffer));
         }
         m_condition.notify_one();
         m_buffer = std::vector<uint8_t>(0);
         m_buffer.reserve(CHUNK_SIZE);
      }

      numBytes -= copySize;
      data += copySize;
   }
}

void Archive::worker()
{
   bool local_done(false);
   std::vector<uint8_t> buf;
   while (!m_queue.empty() || !local_done)
   {
      // scope for the lock
      {
         std::unique_lock<std::mutex> guard(m_mutex);
         m_condition.wait(guard,
            [this](){ return !this->m_queue.empty()
            || this->m_done; });

         while (m_queue.empty() && !m_done)
            m_condition.wait(guard);

         if (!m_queue.empty())
         {
            buf.swap(m_queue.front());
            m_queue.pop();
         }
         local_done = m_done;
      }

      if (!buf.empty())
      {
         fwrite(buf.data(), buf.size(), 1, m_fp);
         buf.clear();
      }
   }

   // flush prior to close
   if (!buf.empty())
   {
      fwrite(buf.data(), buf.size(), 1, m_fp);
      buf.clear();
   }

   fclose(m_fp);
   m_fp = NULL;
}

Archive::Archive(const std::string &filename) :
   m_filename(filename),
   m_fp(NULL),
   m_buffer(0),
   m_bytesWritten(0),
   m_thread(std::bind(&Archive::worker, this))
{
   m_buffer.reserve(CHUNK_SIZE);

   s_archive = this;

   // Ensure this is flushed on program termination
   atexit(exitHandler);
}

Archive::~Archive()
{
   s_archive = NULL;

   Disconnect();

   // scope for the lock
   {
      std::unique_lock<std::mutex> guard(m_mutex);
      if (!m_buffer.empty())
         m_queue.push(std::move(m_buffer));
      m_done = true;
   }
   m_condition.notify_one();
   m_thread.join();
}

bool Archive::Connect()
{
   if (m_filename != "")
   {
      m_fp = fopen(m_filename.c_str(), "wb");
      if (m_fp == NULL)
      {
         Error1("Could not open %s for writing\n", m_filename.c_str());
         return false;
      }

      return true;
   }

   return false;
}

void Archive::Disconnect()
{
   if (m_fp != NULL)
      Flush();
}

void Archive::Send(uint8_t *srcData, uint32_t size, bool isArray)
{
   if (m_fp != NULL)
      Remote::Send(srcData, size, isArray);
}

void Archive::Flush()
{
   if (m_fp != NULL)
   {
      BufferForWrite(m_queuedData, m_queuedLen);
      m_bytesWritten += m_queuedLen;
      m_queuedLen = 0;
   }
}
