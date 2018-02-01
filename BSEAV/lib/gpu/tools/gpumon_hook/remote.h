/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include "datasink.h"
#include "datasinkbuffer.h"
#include "datasourcesinkfile.h"

#include <stdint.h>

#include <vector>
#include <string>

class Packet;

class Remote: public DataSink
{
public:
   enum
   {
     eBufferSize = 1024 * 128
   };

public:
   Remote(uint16_t port);
   virtual ~Remote();

   bool Connect();
   void Disconnect();

   bool ReceivePacket(Packet *p);

public: //DataSink interface
   virtual size_t Write(const void *data, size_t size) override;
   virtual bool Flush() override;

private:
   std::string m_server;
   uint16_t    m_port;
   int         m_socket;

   DataSourceSinkFile m_tcp;
   DataSinkBuffer m_buffer;
};
