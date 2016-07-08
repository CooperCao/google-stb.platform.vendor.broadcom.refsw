/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef __REMOTE_H__
#define __REMOTE_H__

#include <stdint.h>

#include <vector>
#include <string>

class Packet;

class Remote
{
public:
   enum
   {
     eBufferSize = 1024 * 128
   };

public:
   Remote();
   Remote(uint16_t port);
   virtual ~Remote();

   virtual bool Connect();
   virtual void Disconnect();

   virtual void Send(uint8_t *data, uint32_t size, bool isArray = false);
   virtual void Flush();

   virtual bool ReceivePacket(Packet *p);

private:
   uint32_t To32(uint8_t *ptr) { return *(uint32_t*)ptr; }
   int32_t Read32();
   int32_t ReadAll(int fd, void * buf, size_t count);

private:
   std::string m_server;
   uint16_t    m_port;
   int         m_socket;

protected:
   uint32_t    m_queuedLen;
   uint8_t     *m_queuedData;
};

#endif /* __REMOTE_H__ */
