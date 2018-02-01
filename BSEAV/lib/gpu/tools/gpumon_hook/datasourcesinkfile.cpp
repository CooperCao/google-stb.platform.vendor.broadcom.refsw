/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "datasourcesinkfile.h"

#include "debuglog.h"

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

size_t DataSourceSinkFile::Read(void *data, size_t size)
{
   uint8_t *ptr = reinterpret_cast<uint8_t*>(data);
   size_t count = size;

   while(count)
   {
      ssize_t n = read(m_fd, ptr, count);
      if (n == -1 && errno != EAGAIN)
         break;
      ptr += n;
      count -= n;
   }

   if (count)
   {
      int save = errno;
      debug_log(DEBUG_ERROR, "ERROR reading from file descriptor %d: %s\n",
            errno, strerror(errno));
      errno = save;
   }
   return size - count;
}

size_t DataSourceSinkFile::Write(const void *data, size_t size)
{
   const uint8_t *ptr = reinterpret_cast<const uint8_t*>(data);
   size_t count = size;

   while(count)
   {
      ssize_t n = write(m_fd, ptr, count);
      if (n == -1 && errno != EAGAIN)
         break;
      ptr += n;
      count -= n;
   }

   if (count)
   {
      int save = errno;
      debug_log(DEBUG_ERROR, "ERROR writing to file descriptor: %s (%d)\n",
            strerror(errno), errno);
      errno = save;
   }
   return size - count;
}

bool DataSourceSinkFile::Flush()
{
   if (!m_fsync || fsync(m_fd) == 0)
   {
      return true;
   }
   else
   {
      int save = errno;
      debug_log(DEBUG_ERROR, "ERROR flushing file descriptor: %s (%d)\n",
            strerror(errno), errno);
      errno = save;
      return false;
   }
}
