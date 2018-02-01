/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "archive.h"
#include "packet.h"
#include "debuglog.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#define CHUNK_SIZE (8 * 1024 * 1024)
#define FLUSH_THRESHOLD (CHUNK_SIZE/2)
#define FLUSH_TIMEOUT 5 //ms

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

   debug_log(DEBUG_WARN, "Flushing capture archive on exit\n");

   if (deleteMe != NULL)
   {
      // Flush the archive and clean up
      delete deleteMe;
   }
}

Archive::Archive() :
   m_fd(-1),
   m_file(),
   m_buffer(CHUNK_SIZE, m_file, FLUSH_THRESHOLD, FLUSH_TIMEOUT),
   m_bytesWritten(0)
{
   s_archive = this;

   // Ensure this is flushed on program termination
   atexit(exitHandler);
}

Archive::~Archive()
{
   s_archive = NULL;

   Close();
}

bool Archive::Open(const char *filename)
{
   if (filename && *filename)
   {
      m_fd = creat(filename, 0644); // rw-r--r--
      if (m_fd < 0)
      {
         debug_log(DEBUG_ERROR, "Could not open %s for writing\n", filename);
         return false;
      }

      m_file.SetFd(m_fd);
      return true;
   }

   return false;
}

void Archive::Close()
{
   m_buffer.WaitFlush();
   m_file.SetFd(-1);

   if (m_fd >= 0)
      close(m_fd);
   m_fd = -1;
}

size_t Archive::Write(const void *srcData, size_t srcSize)
{
   size_t size = 0;
   if (m_fd >= 0)
   {
      size = m_buffer.Write(srcData, srcSize);
      m_bytesWritten += size;
   }
   return size;
}

bool Archive::Flush()
{
   return m_buffer.Flush();
}
