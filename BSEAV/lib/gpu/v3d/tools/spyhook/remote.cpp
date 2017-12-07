/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <fcntl.h>
#include <sys/types.h>
#include <setjmp.h>
#include <signal.h>

#ifdef WIN32
#include <WinSock2.h>
#define SHUT_RDWR SD_BOTH
#define spy_write(fd, ptr, count) send((fd), (const char*)(ptr), (count), 0)
#define spy_read(fd, ptr, count)  recv((fd), (char*)(ptr), (count), 0)
#define ssize_t int
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define spy_write write
#define spy_read read
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include "remote.h"
#include "packet.h"
#include "spyproxy.h"
#include "platform.h"

#ifdef ANDROID
#include <cutils/log.h>
#include <cutils/properties.h>
#define printf LOGD
#define Error0(s) LOGD(s)
#define Error1(s, a) LOGD(s, a)
#define Log1(s, a) LOGD(s, a)
#define Log2(s, a, b) LOGD(s, a, b)
#else
#define Error0(s) fprintf(stderr, s)
#define Error1(s, a) fprintf(stderr, s, a)
#define Log1(s, a) printf(s, a)
#define Log2(s, a, b) printf(s, a, b)
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

#define SEND_CTRL(m)        do { fputs((m), g_fp_fifo_ctrl); fflush(g_fp_fifo_ctrl); } while(0);

#define GET_ACK(r)          do { char buf[4];                                   \
                                 char *s=fgets(buf,sizeof(buf),g_fp_fifo_ack);  \
                                 if(!s)continue;                                \
                                 r=buf[0]=='1';  break;                         \
                            } while(1);

#ifndef WIN32
static int g_use_proxy = 0;
static int g_ctrl_opened     = 0;
static FILE * g_fp_fifo_ctrl = NULL;
static FILE * g_fp_fifo_ack = NULL;
static int    g_fd_fifo_tx;
static int    g_fd_fifo_rx;
#endif

Remote::Remote() :
   m_queuedData(NULL),
   m_queuedLen(0)
{
   m_queuedData = new uint8_t[eBufferSize];
}

Remote::Remote(uint32_t port) :
   m_queuedData(NULL),
   m_queuedLen(0)
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
         char value[PROPERTY_VALUE_MAX];
         property_get( "debug.egl.hw.gpumonitor", value, "0" );
         if ( atoi( value ) )
         {
            int enabled = true;
            property_get("debug.egl.hw.gpumonitor_appname", value, "");

            if (value[0] != '\0')
            {
               char buf[4096];
               snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());

               FILE *fp = fopen(buf, "r");
               if (fp != NULL)
               {
                  buf[0] = '\0';
                  fgets(buf, sizeof(buf), fp);
                  fclose(fp);

                  if (buf[strlen(buf) - 1] == '\n')
                     buf[strlen(buf) -1] = '\0';

                  if (buf[0] != '\0' && strcmp(buf, value))
                     enabled = false;
               }
               else
                  enabled = false;
            }

            if (enabled == true)
            {
               property_get( "debug.egl.hw.gpumonitor_remip", value, "" );
               m_server = value;

               property_get( "debug.egl.hw.gpumonitor_proxy", value, "0" );
               g_use_proxy = atoi( value );
            }
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
   if ( g_use_proxy && !g_ctrl_opened )
   {
      g_fp_fifo_ctrl = fopen( FIFO_CTRL, "w" );
      g_fp_fifo_ack  = fopen( FIFO_ACK,  "r" );
      if ( g_fp_fifo_ctrl != NULL && g_fp_fifo_ack != NULL )
      {
         g_ctrl_opened = 1;
      }
      else
      {
         Error0("GPU Monitor disabled as could not connect to Proxy server FIFOs\n");
         m_server = "";
         g_use_proxy = 0;
      }
   }
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
   if ( g_use_proxy )
   {
      char buf[256];
      int r;
      snprintf( buf, sizeof(buf), "open %s %d\n", m_server.c_str(), m_port );
      SEND_CTRL(buf);
      GET_ACK(r);
      if ( !r )
         return false;

      g_fd_fifo_tx = open( FIFO_TX, O_WRONLY | O_NONBLOCK | O_SYNC );
      g_fd_fifo_rx = open( FIFO_RX, O_RDONLY | O_NONBLOCK | O_SYNC );
      return true;
   }
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

void Remote::Disconnect(void)
{
#ifndef WIN32
   if ( g_use_proxy )
   {
      int __attribute__((unused)) r;
      SEND_CTRL( "close\n" );
      GET_ACK(r);
      close(g_fd_fifo_tx);
      close(g_fd_fifo_rx);
      return;
   }
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

static void sBadReadHandler(int)
{
   longjmp(sJumpBuf, 1);
}

void Remote::Send(uint8_t *srcData, uint32_t size, bool SPY_UNUSED isArray)
{
   uint8_t *data = srcData;
   void (*prevHandler)(int);

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

      if (setjmp(sJumpBuf))
         goto abortCopy;
      prevHandler = signal(SIGSEGV, sBadReadHandler);

      memcpy(m_queuedData + m_queuedLen, data, bytes);

abortCopy:
      signal(SIGSEGV, prevHandler);
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
   if ( g_use_proxy )
      fd = g_fd_fifo_tx;
#endif

   uint8_t * ptr  = m_queuedData;
   uint32_t count = m_queuedLen;

   do {
      ssize_t n = spy_write(fd, ptr, count);
      if ( n == -1 )
      {
         if ( errno == EAGAIN )
            continue;

         Error0("ERROR writing to socket\n");
         break;
      }
      ptr   += n;
      count -= n;
   } while ( count );

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
   if ( g_use_proxy )
      fd = g_fd_fifo_rx;
#endif

   do {
      ssize_t rc = spy_read(fd, &((char *)buf)[pos], count);
      if (rc == -1)
      {
         if ( errno == EAGAIN )
            continue;

         Error0("ERROR reading from socket\n");
         return rc;
      }
      else if (rc == 0)
      {
         Error0("ERROR connection closed\n");
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
            p->AddItem(PacketItem((eDataType)itemType, (uint32_t)buffer, numBytes));
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
