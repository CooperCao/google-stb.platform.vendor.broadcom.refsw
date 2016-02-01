/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
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

#include <stdio.h>

#ifndef WIN32
#include <pthread.h>
#endif

class Archive : public Remote
{
public:
   Archive(const std::string &filename);
   virtual ~Archive();

   virtual bool Connect();
   virtual void Disconnect();

   virtual void Send(uint8_t *data, uint32_t size, bool isArray = false);
   virtual void Flush();

   FILE        *FilePointer() { return m_fp; }
   uint64_t    BytesWritten() const { return m_bytesWritten; }

private:
   std::string m_filename;
   FILE        *m_fp;
   uint8_t     *m_buffer;
   uint8_t     *m_curPtr;
   uint64_t    m_bytesWritten;

#ifndef WIN32
   pthread_t   m_ioThread;
#endif
};

#endif /* __ARCHIVE_H__ */
