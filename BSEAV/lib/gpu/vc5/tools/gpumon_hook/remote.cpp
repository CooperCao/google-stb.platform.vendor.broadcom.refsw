/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "remote.h"
#include "packet.h"
#include "packetreader.h"
#include "debuglog.h"

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
#include <assert.h>

#ifdef ANDROID
#include <cutils/properties.h>
#endif

#ifdef SPYPROXY
static int g_use_proxy    = 0;
static int g_ctrl_opened  = 0;
static int g_fd_fifo_ctl  = -1;
static int g_fd_fifo_ack  = -1;
static int g_fd_fifo_tx   = -1;
static int g_fd_fifo_rx   = -1;
#endif

Remote::Remote(uint16_t port) :
   m_socket(INVALID_SOCKET),
   m_tcp(m_socket),
   m_buffer(eBufferSize, m_tcp)
{

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
      debug_log(DEBUG_ERROR, "GPU Monitor disabled as no GPUMonitorIP environment variable or file found\n");
   else
      debug_log(DEBUG_WARN, "GPU Monitor initialized for target %s, port %d\n", m_server.c_str(), port);

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
      debug_log(DEBUG_ERROR, "WSAStartup failed: %d\n", iResult);
#endif
}

Remote::~Remote()
{
   Disconnect();
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
      m_tcp.SetFd(g_fd_fifo_tx);
      return true;
   }
#endif
#endif

   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket == INVALID_SOCKET)
   {
#ifndef WIN32
      debug_log(DEBUG_ERROR, "ERROR opening socket - error [%s]\n", hstrerror(h_errno));
#else
      debug_log(DEBUG_ERROR, "ERROR opening socket\n");
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
         debug_log(DEBUG_ERROR, "ERROR no such host - %s\n", m_server.c_str());
         debug_log(DEBUG_ERROR, " - error [%s]\n", hstrerror(h_errno));
         return false;
      }

      memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
   }
#endif

   if (connect(m_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
   {
      debug_log(DEBUG_ERROR, "ERROR connecting %s\n", strerror(errno));
      return false;
   }

   m_tcp.SetFd(m_socket);
   return true;
}

void Remote::Disconnect()
{
   m_buffer.Flush();
   m_tcp.SetFd(INVALID_SOCKET);

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
         debug_log(DEBUG_ERROR, "ERROR disconnecting\n");
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
   m_socket = INVALID_SOCKET;
}

static jmp_buf sJumpBuf;

static void sBadReadHandler(int signal)
{
   longjmp(sJumpBuf, 1);
}

size_t Remote::Write(const void *srcData, size_t srcSize)
{
   const uint8_t *data = reinterpret_cast<const uint8_t *>(srcData);
   uint32_t size = srcSize;
   void (*prevHandler)(int) = NULL;

   while (size > 0)
   {
      size_t sent;
#ifndef WIN32
      if (setjmp(sJumpBuf))
         goto abortCopy;
      prevHandler = signal(SIGSEGV, sBadReadHandler);
#endif

      sent = m_buffer.Write(data, size);
      data += sent;
      size -= sent;

#ifndef WIN32
   abortCopy:
      signal(SIGSEGV, prevHandler);
#endif
   }
   return srcSize - size;
}

bool Remote::Flush()
{
   return m_buffer.Flush();
}

bool Remote::ReceivePacket(Packet *p)
{
   return PacketReader::Read(p, m_tcp);
}
