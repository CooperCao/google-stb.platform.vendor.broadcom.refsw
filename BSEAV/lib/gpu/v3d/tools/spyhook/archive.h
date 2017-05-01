/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include "remote.h"
#include "platform.h"

#include <stdio.h>

#if (defined(__GNUC__) || defined(__clang__))
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

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
