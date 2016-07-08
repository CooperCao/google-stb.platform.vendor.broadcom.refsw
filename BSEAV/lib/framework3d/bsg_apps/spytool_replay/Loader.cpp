/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#include "Loader.h"
#include "spytool_replay.h"
#include "Command.h"
#include "packet.h"
#include "bsg_task.h"


#include <memory.h>

#ifdef HAS_UNZIP
#include "unzip.h"
#endif

#ifdef WIN32
#include <windows.h>
#define usleep(x) Sleep((x) / 1000)
#else
#include <unistd.h>
#endif

#define BUFFER_IO

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

static uint32_t To32(uint8_t *ptr)
{
   return *(uint32_t*)ptr;
}


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
         usleep(1000);
      else
      {
         m_loader.m_queueMutex.Lock();
         m_loader.m_taskDone = true;
         m_loader.m_queueMutex.Unlock();
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
   m_cmdQueueBytes(0),
   m_taskDone(false),
   m_insertAt(0),
   m_takeAt(0),
   m_numCmds(0),
   m_bufferBytes(64 * 1024 * 1024),
   m_ioBufferBytes(64 * 1024),
   m_maxCmds(200000),
   m_reprime(true),
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

int32_t Loader::Read(void *buf, size_t count)
{
#ifndef BUFFER_IO
#ifdef HAS_UNZIP
   if (m_zipFile)
      return unzReadCurrentFile(m_zipFP, buf, count);
   else
#endif
      return fread(buf, 1, count, m_fp);
#else
   int32_t bytesRead = 0;

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
   return bytesRead;
#endif
}

int32_t Loader::Read32()
{
   uint8_t  buffer[4];

   int n = Read(buffer, sizeof(buffer));
   if (n != sizeof(buffer))
      return -1;

   TO_LE_W(buffer);

   return To32(buffer);
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
      m_queueMutex.Lock();

      while (m_numCmds == 0)
      {
         m_queueMutex.Unlock();

         printf("Waiting for data - file reading too slow - FPS may be inaccurate\n");
         usleep(1000);

         m_queueMutex.Lock();

         if (m_taskDone && m_numCmds == 0)
         {
            m_queueMutex.Unlock();
            return false;
         }
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

      m_queueMutex.Unlock();

      return true;
   }
}

bool Loader::ReadCommand(Command *cmd)
{
   while (!cmd->Valid())
   {
      int32_t type = Read32();

      if (type == -1 || type >= eLAST_PACKET_TYPE)
         return false;

      Packet *p = NULL;
      Packet skipPacket;

      if ((type == eAPI_FUNCTION && !cmd->HasRetCode()) || type == eREINIT || type == eTHREAD_CHANGE)
         p = &cmd->GetPacket();
      else if (type == eRET_CODE && cmd->HasAPIFunc())
         p = &cmd->GetRetPacket();
      else
         p = &skipPacket;

      if (p != NULL)
      {
         p->SetType((ePacketType)type);

         int32_t numItems = Read32();
         if (numItems == -1 || numItems > 10000)
            return false;

         cmd->ByteSize() += 8;

         for (int32_t i = 0; i < numItems; i++)
         {
            int32_t itemType = Read32();
            if (itemType == -1)
               return false;

            int32_t numBytes = Read32();
            if (numBytes == -1 || numBytes > 64 * 1024 * 1024)
               return false;

            cmd->ByteSize() += 8;

            if (numBytes > 4 || itemType == eBYTE_ARRAY || itemType == eCHAR_PTR)
            {
               uint8_t  *buffer = new uint8_t[numBytes];

               int n = Read(buffer, numBytes);
               cmd->ByteSize() += numBytes;
               if (n == (int)numBytes)
               {
                  p->AddItem(PacketItem((eDataType)itemType, (uintptr_t)buffer, numBytes));
                  cmd->AddDeleteItem(buffer);
               }
               else
               {
                  delete [] buffer;
                  return false;
               }
            }
            else if (numBytes > 0)
            {
               uint32_t data = Read32();
               p->AddItem(PacketItem((eDataType)itemType, data, 4));
               cmd->ByteSize() += 4;
            }
            else
            {
               p->AddItem(PacketItem((eDataType)itemType, 0, 0));
            }
         }
      }
   }

   return true;
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
      int32_t ident = Read32();
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

      m_major = Read32();
      m_minor = Read32();

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
         usleep(1000);
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

      m_queueMutex.Lock();

      m_cmdQueueBytes += cmd->ByteSize();
      m_numCmds++;

      size = m_cmdQueueBytes;
      cmds = m_numCmds;
      m_queueMutex.Unlock();

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
