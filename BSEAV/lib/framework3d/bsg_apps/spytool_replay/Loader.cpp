/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "Loader.h"
#include "spytool_replay.h"
#include "Command.h"
#include "packet.h"
#include "packetreader.h"
#include "bsg_task.h"
#include <mutex>

#ifdef WIN32
#include <windows.h>
#define sleep(x) Sleep(x)
#else
#include <unistd.h>
#endif

#include <memory.h>

#ifdef HAS_UNZIP
#include "unzip.h"
#endif

#define BUFFER_IO

///////////////////////////////////////////////////////////////////////////////

LoaderTask::LoaderTask(Loader &loader) :
   m_loader(loader)
{}

// This is the task that is run on the secondary thread.  Do not do BSG or GL operations here
void LoaderTask::OnThread()
{
   while (1)
   {
      bool filled = m_loader.FillBuffer(false);
      if (filled)
         sleep(1);
      else
      {
         std::lock_guard<std::mutex> guard(m_loader.m_queueMutex);
         m_loader.m_taskDone = true;
         return;
      }
   }
}

// These calls are executed in the main thread (so it is safe to do BSG and graphics operations)
void LoaderTask::OnCallback(bool finished)
{
}

///////////////////////////////////////////////////////////////////////////////

Loader::Loader(SpyToolReplay *replay) :
   m_replay(replay),
   m_fp(NULL),
   m_major(0),
   m_minor(0),
   m_bufferLen(0),
   m_buffer(NULL),
   m_readBytes(0),
   m_cmdQueueBytes(0),
   m_taskDone(false),
   m_insertAt(0),
   m_takeAt(0),
   m_numCmds(0),
   m_bufferBytes(64 * 1024 * 1024),
   m_ioBufferBytes(64 * 1024),
   m_maxCmds(200000),
   m_reprime(true),
   m_readPtr(NULL),
   m_loaderTask(NULL),
   m_tasker(NULL)
#ifdef HAS_UNZIP
   ,
   m_zipFile(false),
   m_zipFP(NULL)
#endif
{
}

Loader::~Loader()
{
   if (m_fp)
      fclose(m_fp);

#ifdef HAS_UNZIP
   if (m_zipFP)
      unzClose(m_zipFP);
#endif

   delete [] m_buffer;

   m_cmdQueue.clear();
   m_cmdQueue.resize(0);

   delete m_tasker;
}

size_t Loader::Read(void *buf, size_t count)
{
#ifndef BUFFER_IO
#ifdef HAS_UNZIP
   if (m_zipFile)
   {
      int bytesRead = unzReadCurrentFile(m_zipFP, buf, count);
      if (bytesRead > 0)
      {
         m_readBytes += bytesRead;
         return bytesRead;
      }
      else
         return 0;
   }
   else
#endif
   {
      size_t bytesRead = fread(buf, 1, count, m_fp);
      m_readBytes += bytesRead;
      return bytesRead;
   }
#else
   size_t bytesRead = 0;

   while ((uint32_t)bytesRead < count)
   {
      if (m_bufferLen == 0)
      {
         // Load a new buffer full
#ifdef HAS_UNZIP
         if (m_zipFile)
            m_bufferLen = unzReadCurrentFile(m_zipFP, m_buffer, m_ioBufferBytes);
         else
#endif
            m_bufferLen = fread(m_buffer, 1, m_ioBufferBytes, m_fp);

         if (m_bufferLen == 0)
            return bytesRead;
         m_readPtr = m_buffer;
      }
      else if (m_readPtr + count - bytesRead > m_buffer + m_bufferLen)
      {
         // Need more data, so first exhaust what we have
         memcpy((void*)((uint8_t*)buf + bytesRead), m_readPtr, m_buffer + m_bufferLen - m_readPtr);
         bytesRead += m_buffer + m_bufferLen - m_readPtr;
         m_readPtr += bytesRead;
         m_bufferLen = 0;
      }
      else
      {
         memcpy((void*)((uint8_t*)buf + bytesRead), m_readPtr, count - bytesRead);
         m_readPtr += count - bytesRead;
         bytesRead += count - bytesRead;
      }
   }
   m_readBytes += bytesRead;
   return bytesRead;
#endif
}

bool Loader::LoadCommand(Command **cmd)
{
   if (m_reprime)
   {
      // Re-prime when empty - no background loading
      if (m_numCmds == 0 && !m_taskDone)
         PrimeBuffer();

      if (m_taskDone && m_numCmds == 0)
         return false;
      else
      {
         *cmd = &m_cmdQueue[m_takeAt];
         m_takeAt++;
         if (m_takeAt >= m_maxCmds)
            m_takeAt = 0;

         m_numCmds--;
         m_cmdQueueBytes -= (*cmd)->ByteSize();
      }

      return true;
   }
   else
   {
      // Multi-threaded background loading
      std::unique_lock<std::mutex> q(m_queueMutex);

      while (m_numCmds == 0)
      {
         q.unlock();
         printf("Waiting for data - file reading too slow - FPS may be inaccurate\n");
         sleep(1);
         q.lock();

         if (m_taskDone && m_numCmds == 0)
            return false;
      }

      *cmd = &m_cmdQueue[m_takeAt];
      m_takeAt++;
      if (m_takeAt >= m_maxCmds)
         m_takeAt = 0;

      m_numCmds--;
      m_cmdQueueBytes -= (*cmd)->ByteSize();

/*
      if (abs(lastBytes - (int32_t)m_cmdQueueBytes) > 2 * 1024 * 1024)
      {
         printf("%d MB, %d cmds\n", m_cmdQueueBytes / (1024 * 1024), m_numCmds);
         lastBytes = m_cmdQueueBytes;
      }
*/
      return true;
   }
}

bool Loader::ReadCommand(Command *cmd)
{
   while (!cmd->Valid())
   {
      Packet packet;
      m_readBytes = 0;
      if (!PacketReader::Read(&packet, *this))
         return false;
      cmd->ByteSize() += m_readBytes;

      ePacketType type = packet.Type();

      if ((type == eAPI_FUNCTION && !cmd->HasRetCode()) || type == eREINIT || type == eTHREAD_CHANGE)
         cmd->GetPacket() = packet;
      else if (type == eRET_CODE && cmd->HasAPIFunc())
         cmd->GetRetPacket() = packet;
   }

   return true;
}

bool Loader::CaptureHasPointerSize()
{
   #define CAPTURE_WITH_PTR_SIZE_MAJOR_VER_MIN 1
   #define CAPTURE_WITH_PTR_SIZE_MINOR_VER_MIN 5

   return m_major > CAPTURE_WITH_PTR_SIZE_MAJOR_VER_MIN ||
         (m_major == CAPTURE_WITH_PTR_SIZE_MAJOR_VER_MIN &&
               m_minor >= CAPTURE_WITH_PTR_SIZE_MINOR_VER_MIN);
}

bool Loader::Open(const std::string &filename, uint32_t bufferBytes, uint32_t ioBufferBytes, uint32_t maxCmds, bool reprime)
{
   m_bufferBytes = bufferBytes;
   m_ioBufferBytes = ioBufferBytes;
   m_maxCmds = maxCmds;
   m_reprime = reprime;

   if (!m_reprime)
      m_tasker = new bsg::Tasker;

   printf("\nReplay settings\n");
   printf("---------------\n");
   printf("Command buffer size    = %d MB\n", m_bufferBytes / (1024 * 1024));
   printf("Max commands in buffer = %d\n", m_maxCmds);
   printf("Read buffer size       = %d KB\n", m_ioBufferBytes / 1024);
   if (reprime)
      printf("Re-prime buffer when empty\n");
   else
      printf("Background loading\n");
   printf("\n");

   // Create the buffer
   m_buffer = new uint8_t[m_ioBufferBytes];
   m_readPtr = m_buffer;
   m_cmdQueue.resize(m_maxCmds);

   if (m_fp)
   {
      fclose(m_fp);
      m_fp = NULL;
   }

#ifdef HAS_UNZIP
   if (m_zipFP)
   {
      unzClose(m_zipFP);
      m_zipFP = NULL;
   }

   std::string ext = filename.substr(filename.length() - 4, std::string::npos);
   if (ext == ".zip" || ext == ".ZIP" || ext == ".Zip")
      m_zipFile = true;

   if (m_zipFile)
   {
      m_zipFP = unzOpen(filename.c_str());
      if (m_zipFP)
      {
         // Find the capture file in the zip
         int nextRes;
         bool found = false;

         do
         {
            unz_file_info info;
            char          name[1024];

            if (unzGetCurrentFileInfo(m_zipFP, &info, name, 1024, NULL, 0, NULL, 0) == UNZ_OK)
            {
               if (info.uncompressed_size > 0)
                  found = true;
            }

            if (!found)
               nextRes = unzGoToNextFile(m_zipFP);

         } while (!found && nextRes == UNZ_OK);

         if (unzOpenCurrentFile(m_zipFP) != UNZ_OK)
         {
            unzClose(m_zipFP);
            m_zipFP = NULL;
         }
      }
   }
   else
#endif
   {
      m_fp = fopen(filename.c_str(), "rb");
   }

#ifdef HAS_UNZIP
   if (m_fp || m_zipFP)
#else
   if (m_fp)
#endif
   {
      int32_t ident = PacketReader::Read32(*this);
      if (ident != (int32_t)0xBCCA97DA)
      {
         printf("Error: Not a capture file\n");

         if (m_fp)
         {
            fclose(m_fp);
            m_fp = NULL;
         }
#ifdef HAS_UNZIP
         if (m_zipFP)
         {
            unzClose(m_zipFP);
            m_zipFP = NULL;
         }
#endif
      }

      m_major = PacketReader::Read32(*this);
      m_minor = PacketReader::Read32(*this);

      if (m_major > CAPTURE_MAJOR_VER || (m_major <= CAPTURE_MAJOR_VER && m_minor > CAPTURE_MINOR_VER))
      {
         printf("Error: Capture format %d.%d too new for this replayer %d.%d\n", m_major, m_minor,
            CAPTURE_MAJOR_VER, CAPTURE_MINOR_VER);
         if (m_fp)
         {
            fclose(m_fp);
            m_fp = NULL;
         }
#ifdef HAS_UNZIP
         if (m_zipFP)
         {
            unzClose(m_zipFP);
            m_zipFP = NULL;
         }
#endif
      }

      #define DEFAULT_PTR_SIZE 4 // Assume 32-bit platform if not specified
      unsigned capturePtrSize = CaptureHasPointerSize() ?
            PacketReader::Read32(*this) : DEFAULT_PTR_SIZE;
      if (!PacketItem::SetPointerSize(capturePtrSize))
      {
         unsigned ourPtrSize = sizeof(void*);
         printf("Error: Capture pointer size of %d-bit too big for this %d-bit "
               "replayer\n", capturePtrSize * 8, ourPtrSize * 8);
         if (m_fp)
         {
            fclose(m_fp);
            m_fp = NULL;
         }
#ifdef HAS_UNZIP
         if (m_zipFP)
         {
            unzClose(m_zipFP);
            m_zipFP = NULL;
         }
#endif
      }

      printf("Capture format version = %d.%d %zu-bit\n\n", m_major, m_minor,
            PacketItem::GetPointerSize() * 8);

      if (!m_reprime)
      {
         m_loaderTask = new LoaderTask(*this);
         m_tasker->Submit(m_loaderTask);
      }

      PrimeBuffer();
   }
   return m_fp != NULL
#ifdef HAS_UNZIP
      || m_zipFP != NULL
#endif
   ;
}

void Loader::PrimeBuffer()
{
   if (m_reprime)
   {
      m_replay->SetFPSTimer(false);

      if (!FillBuffer(true))
         m_taskDone = true;

      m_replay->SetFPSTimer(true);
   }
   else
   {
      int32_t last = 0;

      printf("Buffer empty - priming...\n");
      while (!m_taskDone && m_numCmds < m_maxCmds - 1 && m_cmdQueueBytes < m_bufferBytes)
      {
         if (abs(last - (int32_t)m_cmdQueueBytes) > 1 * 1024 * 1024)
         {
            printf("Buffered %d MB, %d commands               \r", m_cmdQueueBytes / (1024 * 1024), m_numCmds);
            fflush(stdout);
            last = m_cmdQueueBytes;
         }
         sleep(1);
      }

      printf("Buffered %d MB, %d commands               \n", m_cmdQueueBytes / (1024 * 1024), m_numCmds);
   }
}

bool Loader::FillBuffer(bool print)
{
   uint32_t size = m_cmdQueueBytes;
   uint32_t cmds = m_numCmds;
   int32_t  last = 0;

   // Use MAX_CMDS - 1 since we don't want to start overwriting a cmd that has been taken and
   // is currently executing.
   while (cmds < m_maxCmds - 1 && size < m_bufferBytes)
   {
      SpecializedCommand *cmd = &m_cmdQueue[m_insertAt];

      if (!ReadCommand(cmd))
         return false;

      m_insertAt++;
      if (m_insertAt >= m_maxCmds)
         m_insertAt = 0;

      {
         std::lock_guard<std::mutex> guard(m_queueMutex);
         m_cmdQueueBytes += cmd->ByteSize();
         m_numCmds++;

         size = m_cmdQueueBytes;
         cmds = m_numCmds;
      }

      if (print && abs(last - (int32_t)m_cmdQueueBytes) > 1 * 1024 * 1024)
      {
         printf("Buffered %d MB, %d commands               \r", m_cmdQueueBytes / (1024 * 1024), m_numCmds);
         fflush(stdout);
         last = m_cmdQueueBytes;
      }
   }

   if (print)
      printf("Buffered %d MB, %d commands               \n", m_cmdQueueBytes / (1024 * 1024), m_numCmds);

   return true;
}
