/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "archive.h"
#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <assert.h>

#include <mutex>
#include <chrono>
#include <thread>

#ifndef WIN32
#define THREADED_IO
#endif

#define TEST_ZIP

#ifdef ANDROID
#include <cutils/log.h>
#include <cutils/properties.h>
#define printf LOGD
#define Error0(s) LOGD(s)
#define Error1(s, a) LOGD(s, a)
#define Log1(s, a) LOGD(s, a)
#define Log2(s, a, b) LOGD(s, a, b)
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

#ifdef THREADED_IO

static uint32_t BUFFER_BYTES = 8 * 1024 * 1024;

static uint8_t *sWrite = 0;
static uint8_t *sRead = 0;
static uint32_t sFreeBytes = 0;
static uint8_t *sBuf = 0;
static uint8_t *sEnd = 0;
static bool     sDie = false;

std::mutex sIOMutex;

static uint32_t BytesToWrite(uint8_t **from)
{
   uint32_t bytesToWrite = 0;

   std::lock_guard<std::mutex> lock(sIOMutex);

   if (sFreeBytes != BUFFER_BYTES)
   {
      bytesToWrite = BUFFER_BYTES - sFreeBytes;

      if (sRead + bytesToWrite > sEnd)
         bytesToWrite = sEnd - sRead;

      *from = sRead;
   }

   return bytesToWrite;
}

static void BufferForWrite(uint8_t *data, uint32_t numBytes)
{
   while (numBytes > 0)
   {
      std::unique_lock<std::mutex> IOMutex(sIOMutex);

      // Can't buffer anything unless we have some space
      while (sFreeBytes == 0)
      {
         IOMutex.unlock();
         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
         IOMutex.lock();
      }

      assert(sWrite < sEnd);

      uint32_t bytes = numBytes;
      if (bytes > sFreeBytes)
         bytes = sFreeBytes;
      if (sWrite + bytes > sEnd)
         bytes = sEnd - sWrite;

      uint8_t *p = sWrite;

      // Unlock the buffer for the saving thread while we copy in our new data
      IOMutex.unlock();

      memcpy(p, data, bytes);
      data += bytes;
      numBytes -= bytes;

      IOMutex.lock();

      sWrite += bytes;
      sFreeBytes -= bytes;
      if (sWrite >= sEnd)
         sWrite = sBuf;
   }
}

// Separate file i/o thread to attempt to aid capture performance
void Archive::ioThreadMain()
{
   while (!sDie)
   {
      uint8_t *from;
      uint32_t bytesToWrite = BytesToWrite(&from);

      while (bytesToWrite > 0)
      {
         fwrite(from, bytesToWrite, 1, FilePointer());

         std::unique_lock<std::mutex> IOMutex(sIOMutex);

         sFreeBytes += bytesToWrite;

         sRead += bytesToWrite;
         assert(sRead <= sEnd);
         if (sRead == sEnd)
            sRead = sBuf;

         IOMutex.unlock();

         bytesToWrite = BytesToWrite(&from);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   }
}
#endif

Archive::Archive(const std::string &filename) :
   m_filename(filename),
   m_fp(NULL),
   m_buffer(NULL),
   m_curPtr(NULL),
   m_bytesWritten(0)
{
}

Archive::~Archive()
{
   Disconnect();
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

#ifdef THREADED_IO
      char *env = getenv("GPUMonitorCaptureBuffer");
      if (env)
         BUFFER_BYTES = atoi(env);

      // Make the circular i/o buffer
      m_buffer = new uint8_t[BUFFER_BYTES];
      sBuf = sRead = sWrite = m_buffer;
      sEnd = m_buffer + BUFFER_BYTES;
      sFreeBytes = BUFFER_BYTES;

      // Start the i/o thread
      m_ioThread = std::thread([=] { ioThreadMain(); });
#endif

      return true;
   }

   return false;
}

void Archive::Disconnect()
{
   if (m_fp != NULL)
   {
      Flush();
#ifdef THREADED_IO
      bool done = false;
      while (!done)
      {
         std::lock_guard<std::mutex> lock(sIOMutex);
         done = (sFreeBytes == BUFFER_BYTES);
      }

      sDie = true;
      m_ioThread.join();
#endif
      fclose(m_fp);
   }

#ifdef THREADED_IO
   delete [] m_buffer;
#endif

   m_fp = NULL;
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
#ifndef THREADED_IO
      if (m_queuedLen > 0)
         fwrite(m_queuedData, m_queuedLen, 1, m_fp);
#else
      BufferForWrite(m_queuedData, m_queuedLen);
#endif

      m_bytesWritten += m_queuedLen;

      m_queuedLen = 0;
   }
}
