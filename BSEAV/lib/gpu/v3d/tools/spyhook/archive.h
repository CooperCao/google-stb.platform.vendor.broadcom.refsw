/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "remote.h"
#include "platform.h"

#include <stdio.h>

#include <thread>

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

   void ioThreadMain();

private:
   std::string m_filename;
   FILE        *m_fp;
   uint8_t     *m_buffer;
   uint8_t     *m_curPtr;
   uint64_t    m_bytesWritten;

   std::thread    m_ioThread;
};
