/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  PPP
Module   :  MMM

FILE DESCRIPTION
DESC
=============================================================================*/

#include "remote.h"
#include "packet.h"

#include <fcntl.h>
#include <sys/types.h>
#include <setjmp.h>
#include <signal.h>

#ifdef WIN32
#include <WinSock2.h>
#define SHUT_RDWR SD_BOTH
#define write(fd, ptr, count) send((fd), (const char*)(ptr), (count), 0)
#define read(fd, ptr, count)  recv((fd), (char*)(ptr), (count), 0)
#define ssize_t int
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include "gpumon_proxy.h"

#ifdef ANDROID
#include <cutils/log.h>
#include <cutils/properties.h>
#define printf ALOGD
#define Error0(s) ALOGD(s)
#define Error1(s, a) ALOGD(s, a)
#define Log0(s) ALOGD(s)
#define Log1(s, a) ALOGD(s, a)
#define Log2(s, a, b) ALOGD(s, a, b)
#define Log3(s, a, b, c) ALOGD(s, a, b, c)
#else
#define Error0(s) fprintf(stderr, s)
#define Error1(s, a) fprintf(stderr, s, a)
#define Log0(s) printf(s)
#define Log1(s, a) printf(s, a)
#define Log2(s, a, b) printf(s, a, b)
#define Log3(s, a, b, c) printf(s, a, b, c)
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

#ifdef SPYPROXY
static int g_use_proxy    = 0;
static int g_ctrl_opened  = 0;
static int g_fd_fifo_ctl  = -1;
static int g_fd_fifo_ack  = -1;
static int g_fd_fifo_tx   = -1;
static int g_fd_fifo_rx   = -1;
#endif

Remote::Remote() :
   m_queuedLen(0),
   m_queuedData(NULL)
{
   m_queuedData = new uint8_t[eBufferSize];
}

Remote::Remote(uint16_t port) :
   m_queuedLen(0),
   m_queuedData(NULL)
{
   m_queuedData = new uint8_t[eBufferSize];

   char *serv = getenv("GPUMonitorIP");
   if (serv == NULL)
   {
      // If we can't find the env var, look for a file
      FILE *fp = fopen("GPUMonitorIP", "r");
      if (fp != NULL)
      {
         char buf[1024];
         if (fgets(buf, 1024, fp) != NULL)
            m_server = buf;

         fclose(fp);
      }
#ifdef ANDROID
      else
      {
         FILE *fp = fopen("/system/bin/GPUMonitorIP", "r");
         if (fp != NULL)
         {
            char buf[1024];
            if (fgets(buf, 1024, fp) != NULL)
               m_server = buf;

            fclose(fp);
         }
         else
         {
            char value[PROPERTY_VALUE_MAX];
            property_get( "debug.egl.hw.gpumon.ip", value, "" );
            m_server = value;

#ifdef SPYPROXY
            property_get("debug.egl.hw.gpumon.proxy", value, "0");
            g_use_proxy = atoi(value);
            m_server = "<gpumon proxy>";   // fake it
#endif
         }
      }
#endif
   }
   else
      m_server = std::string(serv);

   if (m_server == "")
      Error0("GPU Monitor disabled as no GPUMonitorIP environment variable or file found\n");
   else
      Log2("GPU Monitor initialized for target %s, port %d\n", m_server.c_str(), port);

   m_port = port;

#ifndef WIN32
#ifdef SPYPROXY
   if (g_use_proxy && !g_ctrl_opened)
   {
      g_fd_fifo_ctl = open(FIFO_CTL, O_WRONLY);
      if (g_fd_fifo_ctl != -1)
      {
         g_fd_fifo_ack = open(FIFO_ACK, O_RDONLY);
         if (g_fd_fifo_ack != -1)
         {
            g_ctrl_opened = 1;
         }
         else
         {
            close(g_fd_fifo_ctl);
         }
      }
      if (!g_ctrl_opened)
      {
         Error0("GPU Monitor disabled as could not connect to Proxy server FIFOs\n");
         m_server = "";
         g_use_proxy = 0;
      }
   }
#endif
#else
   // Initialize winsock
   WSADATA wsaData = {0};
   int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
   if (iResult != 0)
      printf("WSAStartup failed: %d\n", iResult);
#endif
}

Remote::~Remote()
{
   Disconnect();

   delete [] m_queuedData;
}

bool Remote::Connect()
{
   struct sockaddr_in serv_addr;

   if (m_server == "")
      return false;

#ifndef WIN32
#ifdef SPYPROXY
   if (g_use_proxy)
   {
      char buf[64];
      int n = snprintf(buf, sizeof(buf), "C %5hu", m_port) + 1;

      if (write(g_fd_fifo_ctl, buf, n) != n ||
          read(g_fd_fifo_ack, buf, 1) != 1 ||
          buf[0] != '1')
      {
         return false;
      }

      snprintf(buf, sizeof(buf), FIFO_TX_FMT, m_port);
      g_fd_fifo_tx = open(buf, O_WRONLY /*| O_NONBLOCK | O_SYNC*/);
      snprintf(buf, sizeof(buf), FIFO_RX_FMT, m_port);
      g_fd_fifo_rx = open(buf, O_RDONLY /*| O_SYNC*/);
      return true;
   }
#endif
#endif

   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket == INVALID_SOCKET)
   {
#ifndef WIN32
      Error1("ERROR opening socket - error [%s]\n", hstrerror(h_errno));
#else
      Error0("ERROR opening socket\n");
#endif
      return false;
   }

   memset((char *)&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_port = htons(m_port);

#ifdef WIN32
   serv_addr.sin_addr.s_addr = inet_addr(m_server.c_str());
#else

   if (inet_aton(m_server.c_str(), &serv_addr.sin_addr) == 1)
   {
   }
   else
   {
      struct hostent *server = gethostbyname(m_server.c_str());
      if (server == NULL)
      {
         Error1("ERROR no such host - %s\n", m_server.c_str());
         Error1(" - error [%s]\n", hstrerror(h_errno));
         return false;
      }

      memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
   }
#endif

   if (connect(m_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
   {
      Error1("ERROR connecting %s\n", strerror(errno));
      return false;
   }

   return true;
}

void Remote::Disconnect()
{
#ifndef WIN32
#ifdef SPYPROXY
   if (g_use_proxy)
   {
      char buf[64];
      int n = snprintf(buf, sizeof(buf), "D %5hu", m_port) + 1;

      if (write(g_fd_fifo_ctl, buf, n) != n ||
          read(g_fd_fifo_ack, buf, 1) != 1 ||
          buf[0] != '1')
      {
         Error0("ERROR disconnecting\n");
      }

      close(g_fd_fifo_tx);
      close(g_fd_fifo_rx);
      return;
   }
#endif
#endif
   shutdown(m_socket, SHUT_RDWR);
#ifdef WIN32
   closesocket(m_socket);
#else
   close(m_socket);
#endif
   m_socket = 0;
}

static jmp_buf sJumpBuf;

static void sBadReadHandler(int signal)
{
   longjmp(sJumpBuf, 1);
}

void Remote::Send(uint8_t *srcData, uint32_t size, bool isArray)
{
   uint8_t *data = srcData;
   void (*prevHandler)(int) = NULL;

#ifdef BIG_ENDIAN_CPU
   uint8_t copy[4];
   if (size == 4 && !isArray)
   {
      memcpy(copy, srcData, 4);
      TO_LE_W(copy);
      data = copy;
   }
#endif

   while (size > 0)
   {
      uint32_t bytes = eBufferSize - m_queuedLen;
      if (size < bytes)
         bytes = size;

#ifndef WIN32
      if (setjmp(sJumpBuf))
         goto abortCopy;
      prevHandler = signal(SIGSEGV, sBadReadHandler);
#endif

      memcpy(m_queuedData + m_queuedLen, data, bytes);

#ifndef WIN32
   abortCopy:
      signal(SIGSEGV, prevHandler);
#endif
      m_queuedLen += bytes;
      if (m_queuedLen >= eBufferSize)
         Flush();
      data += bytes;
      size -= bytes;
   }

/*
   uint32_t cnt = 0;
   while (cnt < size)
   {
      while (m_queuedLen < eBufferSize && cnt++ < size)
         m_queuedData[m_queuedLen++] = *data++;

      if (m_queuedLen >= eBufferSize)
         Flush();
   }
*/

}

void Remote::Flush()
{
   int fd = m_socket;

#ifndef WIN32
#ifdef SPYPROXY
   if (g_use_proxy)
      fd = g_fd_fifo_tx;
#endif
#endif

   uint8_t * ptr  = m_queuedData;
   uint32_t count = m_queuedLen;

   do {
      ssize_t n = write(fd, ptr, count);
      if (n == -1)
      {
         if (errno == EAGAIN)
            continue;

         Error0("ERROR writing to socket\n");
         break;
      }
      ptr   += n;
      count -= n;
   } while (count);

   m_queuedLen = 0;
}

int32_t Remote::Read32()
{
   uint8_t  buffer[4];

   // read a response back, everything is sync'd
   int n = ReadAll(m_socket, buffer, sizeof(buffer));
   if (n != sizeof(buffer))
      return -1;

   TO_LE_W(buffer);

   return To32(buffer);
}

int32_t Remote::ReadAll(int fd, void * buf, size_t count)
{
   int32_t pos = 0;
   int32_t res = count;

#ifndef WIN32
#ifdef SPYPROXY
   if (g_use_proxy)
      fd = g_fd_fifo_rx;
#endif
#endif

   do {
      ssize_t rc = read(fd, &((char *)buf)[pos], count);
      if (rc == -1)
      {
         if (errno == EAGAIN)
            continue;

         Error0("ERROR reading from socket\n");
         return rc;
      }
      count -= rc;
      pos += rc;
   }
   while (count != 0);

   return res;
}

bool Remote::ReceivePacket(Packet *p)
{
   int32_t type = Read32();
   if (type == -1)
      return false;

   p->SetType((ePacketType)type);

   int32_t numItems = Read32();
   if (numItems == -1)
      return false;

   for (int32_t i = 0; i < numItems; i++)
   {
      int32_t itemType = Read32();
      if (itemType == -1)
         return false;

      int32_t numBytes = Read32();
      if (numBytes == -1)
         return false;

      if (numBytes > 4)
      {
         uint8_t  *buffer = new uint8_t[numBytes];

         int n = ReadAll(m_socket, buffer, numBytes);
         if (n == (int)numBytes)
         {
            p->AddItem(PacketItem((eDataType)itemType, (uintptr_t)buffer, numBytes));
         }
         else
         {
            delete [] buffer;
            return false;
         }
      }
      else
      {
         uint32_t data = Read32();
         p->AddItem(PacketItem((eDataType)itemType, data, 4));
      }
   }

   return true;
}
