/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef LINUX
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET_ERROR             -1
static int g_sockfd;

#else /* Win32 */

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

static SOCKET g_sockfd = INVALID_SOCKET;
static LPHOSTENT hp;
static SOCKADDR_IN ServerAddr;

/* Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib */
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#endif


#define SOCKET_NAK                 0
#define SOCKET_ACK                 1

#define SOCKET_STOP_SERVER         0
#define SOCKET_STOP_CLIENT         1
#define SOCKET_REG_READ            2
#define SOCKET_REG_WRITE           4
#define SOCKET_MEM_READ            6
#define SOCKET_MEM_WRITE           8
#define SOCKET_TIMEOUT            10
#define SOCKET_INIT_CAPTURE       12
#define SOCKET_START_CAPTURE      14
#define SOCKET_STOP_CAPTURE       16
#define SOCKET_COMMAND            18
#define SOCKET_REG_READ64         20
#define SOCKET_REG_WRITE64        22
#define SOCKET_MEM_READ40         32
#define SOCKET_MEM_WRITE40        34

#define SOCKET_PORT               9999

#define pout(string, integer) { fprintf(stderr, "ERROR: %s %d: %s: %d\n", string, integer, __FILE__, __LINE__ ); }

/* private helpers */
static int _openSocket( const char * hostname, const unsigned short portnum );
static int _closeSocket( bool shutdown );


#if LINUX

struct BcmSocketMutexObj
{
    pthread_mutex_t mutex;
};

typedef struct BcmSocketMutexObj *BcmSocketMutexHandle;
static BcmSocketMutexHandle g_hSocketMutex = 0;

static int bcmCreateMutex(BcmSocketMutexHandle *handle)
{
    *handle = (BcmSocketMutexHandle)malloc(sizeof(**handle));
    if (!*handle)
    {
        return 1;
    }

    if (pthread_mutex_init(&(*handle)->mutex, NULL))
    {
        printf("Failed pthread_mutex_init\n");
        goto error;
    }

    return 0;

error:
    free(*handle);
    return 1;

}

static int bcmTryAcquireMutex(BcmSocketMutexHandle handle)
{
    int rc = 0;
    rc = pthread_mutex_trylock(&handle->mutex);
    if (rc==0) {
        return 0;
    } else if (rc==EBUSY) {
        return 1;
    } else {
        return 2;
    }
}

static int bcmAcquireMutex(BcmSocketMutexHandle handle)
{
    if (pthread_mutex_lock(&handle->mutex))
        return 1;
    else
        return 0;
}

static void bcmReleaseMutex(BcmSocketMutexHandle handle)
{
    if (pthread_mutex_unlock(&handle->mutex))
    {
        fprintf(stderr, "pthread_mutex_unlock failed");
        assert(false);
    }
    return;
}

static void bcmDestroyMutex(BcmSocketMutexHandle handle)
{
    pthread_mutex_destroy(&handle->mutex);
    free(handle);
    return ;
}

#endif /* LINUX */


static int my_recv
(
    int s,
    void *buf,
    int len,
    int flags
)
{
    int result;
    int original_len = len;

    for( ;; )
    {
        result = recv( s, buf, len, flags );
        if( result <= 0 )
        {
            pout("recv failed, result", result);
            break;
        }

        buf = (char *) buf + result;
        len -= result;

        if( !len )
        {
            result = original_len;
            break;
        }
    }

    return result;
}

static int my_send
(
    int s,
    void *buf,
    int len,
    int flag
)
{
    int total = 0;        /* how many bytes we've sent */
    int bytesleft = len; /* how many we have left to send*/
    int n;

    while(total < len) {
        n = send(s, (char *)buf+total, bytesleft, flag);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    return total; /* return -1 on failure, 0 on success*/
}



/*******************************************************func*
 *  bcmUseSocket
 */
int bcmOpenSocket( const char * hostname, const unsigned short portnum )
{
    int iError = _openSocket(hostname, portnum);

    return iError;
}

int bcmCloseSocket( bool shutdown )
{
    int iError = _closeSocket(shutdown);
    return iError;
}

int bcmSocketReadRegister
    ( uint32_t  reg_addr,
      void     *pData,
      bool      is64Bit )
{
    int iReturn = SOCKET_ERROR;
    int iResult;
    unsigned char out_buffer[ 5 ];
    unsigned char in_buffer[ 9 ];

    /* NOTE: page wb will call client memory write which acquires mutex; so
       this function should be outside of AcquireMutex! */
#ifdef EMULATION
    bcmPageWriteBack();
    reg_addr |= BCHP_PHYSICAL_OFFSET;  /* 0x10000000 or 0xf0000000 for 28nm */
#else
    reg_addr |= 0x10000000;
#endif

    out_buffer[ 0 ] = (is64Bit) ? SOCKET_REG_READ64 : SOCKET_REG_READ;

    out_buffer[ 1 ] = (unsigned char) (reg_addr >> 24);
    out_buffer[ 2 ] = (unsigned char) (reg_addr >> 16);
    out_buffer[ 3 ] = (unsigned char) (reg_addr >>  8);
    out_buffer[ 4 ] = (unsigned char) (reg_addr >>  0);

#if LINUX
    /* This mutex is to protect the shared socket used by threads of the client! */
    if (0 != bcmAcquireMutex(g_hSocketMutex))
    {
        fprintf(stderr, "Failed to acquire mutex in bcmReadRegister%s\n", is64Bit?"64":"");
        return 0;
    }
#endif

    iResult = my_send( g_sockfd, out_buffer, sizeof out_buffer, 0 );
    if( iResult < 0 )
    {
        pout( "SOCKET_REG_READ send()", iResult);
        goto done;
    }
    else if( iResult < (int)sizeof(out_buffer) )
    {
        fprintf(stderr, "SOCKET_REG_READ%s send() only sent %d\n", is64Bit?"64":"", iResult);
        goto done;
    }

    iResult = my_recv( g_sockfd, (char *)in_buffer, is64Bit? sizeof(uint64_t) + 1 : sizeof(uint32_t) + 1, 0 );
    if( iResult < 0 )
    {
        fprintf(stderr, "SOCKET_REG_READ%s recv(): %d\n", is64Bit?"64":"", iResult);
        goto done;
    }
    else if( (is64Bit && (iResult != sizeof(uint64_t) + 1)) ||
             (!is64Bit && (iResult != sizeof(uint32_t) + 1)) )
    {
        fprintf(stderr, "SOCKET_REG_READ%s unexpected ACK size %d\n", is64Bit?"64":"", iResult);
        goto done;
    }
    else if( in_buffer[ 0 ] != (out_buffer[ 0 ] | SOCKET_ACK) )
    {
        fprintf(stderr, "SOCKET_REG_READ%s wrong ACK %d , %d\n", is64Bit?"64":"", in_buffer[ 0 ], out_buffer[ 0 ]);
        goto done;
    }

    /* assume network order of the byte stream in big-endian */
    if(is64Bit)
    {
        *(uint64_t*)pData =
            ((uint64_t) in_buffer[ 1 ] << 56) |
            ((uint64_t) in_buffer[ 2 ] << 48) |
            ((uint64_t) in_buffer[ 3 ] << 40) |
            ((uint64_t) in_buffer[ 4 ] << 32) |
            ((uint64_t) in_buffer[ 5 ] << 24) |
            ((uint64_t) in_buffer[ 6 ] << 16) |
            ((uint64_t) in_buffer[ 7 ] <<  8) |
            ((uint64_t) in_buffer[ 8 ] <<  0);
    }
    else
    {
        *(uint32_t*)pData =
            ((uint32_t) in_buffer[ 1 ] << 24) |
            ((uint32_t) in_buffer[ 2 ] << 16) |
            ((uint32_t) in_buffer[ 3 ] <<  8) |
            ((uint32_t) in_buffer[ 4 ] <<  0);
    }

    iReturn = 0;

done:
#if LINUX
    bcmReleaseMutex(g_hSocketMutex);
#endif
    return iReturn;
}

int bcmSocketWriteRegister
    ( uint32_t reg_addr,
      uint64_t data,
      bool     is64Bit )
{
    int iReturn = SOCKET_ERROR;
    int iResult;
    unsigned char out_buffer[ 13 ];
    unsigned char in_buffer[ 1 ];

#ifdef EMULATION
    bcmPageWriteBack();
    reg_addr |= BCHP_PHYSICAL_OFFSET;  /* 0x10000000 or 0xf0000000 for 28nm */
#else
    reg_addr |= 0x10000000;
#endif

    out_buffer[ 0 ] = (is64Bit) ? SOCKET_REG_WRITE64 : SOCKET_REG_WRITE;

    out_buffer[ 1 ] = (unsigned char) (reg_addr >> 24);
    out_buffer[ 2 ] = (unsigned char) (reg_addr >> 16);
    out_buffer[ 3 ] = (unsigned char) (reg_addr >>  8);
    out_buffer[ 4 ] = (unsigned char) (reg_addr >>  0);


    if(is64Bit) { /* assumed big endian */
        out_buffer[  5 ] = (unsigned char) (data >> 56);
        out_buffer[  6 ] = (unsigned char) (data >> 48);
        out_buffer[  7 ] = (unsigned char) (data >> 40);
        out_buffer[  8 ] = (unsigned char) (data >> 32);
        out_buffer[  9 ] = (unsigned char) (data >> 24);
        out_buffer[ 10 ] = (unsigned char) (data >> 16);
        out_buffer[ 11 ] = (unsigned char) (data >>  8);
        out_buffer[ 12 ] = (unsigned char) (data >>  0);
    } else {
        out_buffer[ 5 ] = (unsigned char) (data >> 24);
        out_buffer[ 6 ] = (unsigned char) (data >> 16);
        out_buffer[ 7 ] = (unsigned char) (data >>  8);
        out_buffer[ 8 ] = (unsigned char) (data >>  0);
    }

#if DEBUG
#define UINT64_ARG(x) (unsigned)((x)>>32), (unsigned)(x)
    fprintf(stderr, "REG write%s: addr = %x, data = 0x%x%08x\n", is64Bit?"64":"", reg_addr, UINT64_ARG(data));
#endif

#if LINUX
    /* This mutex is to protect the shared socket used by threads of the client! */
    if (0 != bcmAcquireMutex(g_hSocketMutex))
    {
        fprintf(stderr, "Failed to acquire mutex in bcmWriteRegister%s\n", is64Bit?"64":"");
        return iReturn;
    }
#endif

    iResult = my_send( g_sockfd, out_buffer, is64Bit? sizeof(uint64_t) + 5 : sizeof(uint32_t) + 5, 0 );
    if( iResult < 0 )
    {
        fprintf(stderr, "SOCKET_REG_WRITE%s send() %d\n", is64Bit?"64":"", iResult);
        goto done;
    }
    else if( (is64Bit && (iResult != sizeof(uint64_t) + 5)) ||
             (!is64Bit && (iResult != sizeof(uint32_t) + 5)) )
    {
        fprintf(stderr, "SOCKET_REG_WRITE%s send() only sent %d\n", is64Bit?"64":"", iResult);
        goto done;
    }

    iResult = my_recv( g_sockfd, in_buffer, sizeof in_buffer, 0 );
    if( iResult < 0 )
    {
        fprintf(stderr, "SOCKET_REG_WRITE%s recv(): %d\n", is64Bit?"64":"", iResult);
        goto done;
    }
    else if( iResult != sizeof in_buffer )
    {
        fprintf(stderr, "SOCKET_REG_WRITE%s unexpected ACK size %d\n", is64Bit?"64":"", iResult);
        goto done;
    }
    else if( in_buffer[ 0 ] != (out_buffer[ 0 ] | SOCKET_ACK) )
    {
        fprintf(stderr, "SOCKET_REG_WRITE%s wrong ACK %d\n", is64Bit?"64":"", in_buffer[ 0 ]);
        fprintf(stderr, "RegAddr = 0x%x, data = 0x%llx\n", reg_addr, data);
        goto done;
    }

    iReturn = 0;

done:
#if LINUX
    bcmReleaseMutex(g_hSocketMutex);
#endif

    return iReturn;
}

int bcmSocketReadMemory
    ( uint64_t         mem_addr,
      void            *data,
      size_t           size )
{
    int iReturn = SOCKET_ERROR;
    int iResult;
    char out_buffer[ 10 ];
    unsigned char in_buffer[ 4097 ];
    size_t ss, offset = 0;

    if( size & 3 )
    {
        pout("SOCKET_MEM_READ non-dword size: ", (unsigned int) size);
        return iReturn;
    }

    if (mem_addr >= (uint32_t)-1)
    {
        out_buffer[ offset++ ] = SOCKET_MEM_READ40;
        out_buffer[ offset++ ] = (uint8_t) (mem_addr >> 32);
    }
    else
    {
        out_buffer[ offset++ ] = SOCKET_MEM_READ;
    }

    out_buffer[ offset++ ] = (uint8_t) (mem_addr >> 24);
    out_buffer[ offset++ ] = (uint8_t) (mem_addr >> 16);
    out_buffer[ offset++ ] = (uint8_t) (mem_addr >>  8);
    out_buffer[ offset++ ] = (uint8_t) (mem_addr >>  0);

    out_buffer[ offset++ ] = (uint8_t) (size >> 24);
    out_buffer[ offset++ ] = (uint8_t) (size >> 16);
    out_buffer[ offset++ ] = (uint8_t) (size >>  8);
    out_buffer[ offset++ ] = (uint8_t) (size >>  0);

#if LINUX
    /* This mutex is to protect the shared socket used by threads of the client! */
    if (0 != bcmAcquireMutex(g_hSocketMutex))
    {
        fprintf(stderr, "Failed to acquire mutex in bcmReadMemory\n");
        return iReturn;
    }
#endif

    iResult = my_send( g_sockfd, out_buffer, offset, 0 );
    if( iResult < 0 )
    {
        pout("SOCKET_MEM_READ send(): ", iResult);
        goto done;
    }
    else if( iResult != offset )
    {
        pout("SOCKET_MEM_READ send() only sent ", iResult);
        goto done;
    }

    iResult = my_recv( g_sockfd, (char *)in_buffer, (int) size + 1, 0 );
    if( iResult < 0 )
    {
        pout("SOCKET_MEM_READ my_recv()", iResult);
        goto done;
    }
    else if( iResult != (int)(size + 1) )
    {
        pout("SOCKET_MEM_READ unexpected ACK size ", iResult);
        goto done;
    }
    else if( in_buffer[ 0 ] != (out_buffer[ 0 ] | SOCKET_ACK) )
    {
        pout("SOCKET_MEM_READ wrong ACK", in_buffer[ 0 ]);
#if DEBUG
        fprintf(stderr, "mem_addr = 0x%x, size = 0x%x", mem_addr, size);
#endif
        goto done;
    }

    for( ss = 0; ss < size; ss += sizeof (uint32_t) )
    {
        *(uint32_t *) ((uintptr_t) data + ss) =
            (in_buffer[ ss + 1 ] << 24) | (in_buffer[ ss + 2 ] << 16) |
            (in_buffer[ ss + 3 ] <<  8) | (in_buffer[ ss + 4 ] <<  0);
    }

#if DEBUG
    fprintf(stderr, "MEM read: addr = %x, data[0] = %x\n", mem_addr, *(uint32_t *)data);
#endif
    iReturn = 0;

done:
#if LINUX
    bcmReleaseMutex(g_hSocketMutex);
#endif
    return iReturn;
}

int bcmSocketWriteMemory
    ( uint64_t         mem_addr,
      void            *data,
      size_t           size )
{
    int iReturn = SOCKET_ERROR;
    int iResult;
    char out_buffer[ 4107 ];
    char in_buffer[ 1 ];
    size_t ss, offset = 0;
    uint32_t uu;

    if( size & 3 )
    {
        pout("SOCKET_MEM_WRITE non-dword size: ", (unsigned int) size);
        return iReturn;
    }


    if (mem_addr >= (uint32_t)-1)
    {
        out_buffer[ offset++ ] = SOCKET_MEM_WRITE40;
        out_buffer[ offset++ ] = (uint8_t) (mem_addr >> 32);
    }
    else
    {
        out_buffer[ offset++ ] = SOCKET_MEM_WRITE;
    }

    out_buffer[ offset++ ] = (uint8_t) (mem_addr >> 24);
    out_buffer[ offset++ ] = (uint8_t) (mem_addr >> 16);
    out_buffer[ offset++ ] = (uint8_t) (mem_addr >>  8);
    out_buffer[ offset++ ] = (uint8_t) (mem_addr >>  0);

    out_buffer[ offset++ ] = (uint8_t) (size >> 24);
    out_buffer[ offset++ ] = (uint8_t) (size >> 16);
    out_buffer[ offset++ ] = (uint8_t) (size >>  8);
    out_buffer[ offset++ ] = (uint8_t) (size >>  0);

    for( ss = 0; ss < size; ss += sizeof (uint32_t) )
    {
        uu = *(uint32_t *) ((uintptr_t) data + ss);

        out_buffer[ ss + offset + 0 ] = (uint8_t) (uu >> 24);
        out_buffer[ ss + offset + 1 ] = (uint8_t) (uu >> 16);
        out_buffer[ ss + offset + 2 ] = (uint8_t) (uu >>  8);
        out_buffer[ ss + offset + 3 ] = (uint8_t) (uu >>  0);
    }

#if DEBUG
    fprintf(stderr, "MEM write: addr = %x, data[0] = %x", mem_addr, *(uint32_t *)data);
#endif

#if LINUX
    /* This mutex is to protect the shared socket used by threads of the client! */
    if (0 != bcmAcquireMutex(g_hSocketMutex))
    {
        fprintf(stderr, "Failed to acquire mutex in bcmWriteMemory\n");
        return iReturn;
    }
#endif

    iResult = my_send( g_sockfd, out_buffer, size + offset, 0 );
    if( iResult < 0 )
    {
        pout("SOCKET_MEM_WRITE send(): ", iResult);
        goto done;
    }
    else if( iResult != ((int)(size + offset)) )
    {
        pout("SOCKET_MEM_WRITE send() only sent ", iResult);
        goto done;
    }

    iResult = my_recv( g_sockfd, in_buffer, sizeof in_buffer, 0 );
    if( iResult < 0 )
    {
        pout("SOCKET_MEM_WRITE my_recv()", iResult);
        goto done;
    }
    else if( iResult != sizeof in_buffer )
    {
        pout("SOCKET_MEM_WRITE unexpected ACK size", iResult);
        goto done;
    }
    else if( in_buffer[ 0 ] != (out_buffer[ 0 ] | SOCKET_ACK) )
    {
        pout("SOCKET_MEM_WRITE wrong ACK ", in_buffer[ 0 ]);
#if DEBUG
        fprintf(stderr, "mem_addr = 0x%x, size = 0x%x", mem_addr, size);
#endif
        goto done;
    }

    iReturn = 0;

done:
#if LINUX

    bcmReleaseMutex(g_hSocketMutex);
#endif
    return iReturn;
}

int bcmSocketCommand
    ( uint32_t  ulCmd,
      int32_t   ilSize,
      int32_t  *pilData )
{
    int iReturn = SOCKET_ERROR;
    int iResult;
    char out_buffer[ 4106 ];
    char in_buffer[ 1 ];
    int32_t ilVal;
    size_t size = ilSize * sizeof(int32_t);
    size_t i;

    out_buffer[ 0 ] = SOCKET_COMMAND;

    out_buffer[ 1 ] = (uint8_t) (ulCmd >> 24);
    out_buffer[ 2 ] = (uint8_t) (ulCmd >> 16);
    out_buffer[ 3 ] = (uint8_t) (ulCmd >>  8);
    out_buffer[ 4 ] = (uint8_t) (ulCmd >>  0);

    out_buffer[ 5 ] = (uint8_t) (size >> 24);
    out_buffer[ 6 ] = (uint8_t) (size >> 16);
    out_buffer[ 7 ] = (uint8_t) (size >>  8);
    out_buffer[ 8 ] = (uint8_t) (size >>  0);

    for( i=0; i<size; i+=sizeof(uint32_t))
    {
        ilVal = *(pilData++);

        out_buffer[ i +  9 ] = (uint8_t) (ilVal >> 24);
        out_buffer[ i + 10 ] = (uint8_t) (ilVal >> 16);
        out_buffer[ i + 11 ] = (uint8_t) (ilVal >>  8);
        out_buffer[ i + 12 ] = (uint8_t) (ilVal >>  0);
    }

#if LINUX
    /* This mutex is to protect the shared socket used by threads of the client! */
    if (0 != bcmAcquireMutex(g_hSocketMutex))
    {
        fprintf(stderr, "Failed to acquire mutex in bcmWriteMemory\n");
        return iReturn;
    }
#endif

    /* send out as 8-bit values instead of 32-bit values */
    iResult = my_send( g_sockfd, out_buffer, (int) size + 9, 0 );
    if( iResult < 0 )
    {
        pout("SOCKET_COMMAND send():", iResult);
        goto done;
    }
    else if( iResult != ((int) size + 9) )
    {
        pout("SOCKET_COMMAND send() only sent", iResult);
        goto done;
    }

    iResult = my_recv( g_sockfd, in_buffer, sizeof in_buffer, 0 );
    if( iResult < 0 )
    {
        pout("SOCKET_COMMAND my_recv()", iResult);
        goto done;
    }
    else if( iResult != sizeof in_buffer )
    {
        pout("SOCKET_COMMAND unexpected ACK size", iResult);
        goto done;
    }
    else if( in_buffer[ 0 ] != (out_buffer[ 0 ] | SOCKET_ACK) )
    {
        pout("SOCKET_COMMAND wrong ACK", in_buffer[ 0 ]);
        goto done;
    }

    iReturn = 0;

done:
#if LINUX
    bcmReleaseMutex(g_hSocketMutex);
#endif
    return iReturn;
}


#if LINUX

static int _openSocket
    ( const char * hostname,
      const unsigned short portnum )
{
    int iError = 0;
    unsigned char in_buffer[ 5 ];
    char *ptr = strchr(hostname, ':');

    struct sockaddr_in dest_addr;   /* will hold the destination addr */
    g_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if( g_sockfd == SOCKET_ERROR )
    {
        perror("socket() error");
        goto done;
    }

    if(ptr) /* with user specified port number */
    {
        *ptr++ = '\0';
    }

    dest_addr.sin_family = AF_INET;          /* host byte order */
    dest_addr.sin_port = htons(ptr? atoi(ptr) : portnum);   /* short, network byte order */
    dest_addr.sin_addr.s_addr = inet_addr(hostname);
    memset(dest_addr.sin_zero, '\0', sizeof dest_addr.sin_zero);

    /* don't forget to error check the connect()! */
    iError = connect(g_sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
    if( iError == SOCKET_ERROR )
    {
        perror("connect() error");
        goto done;
    }

    /* get Ack */
    iError = recv( g_sockfd, in_buffer, 1, 0 );
    if( iError <= 0 )
    {
        fprintf(stderr, "Connection is rejected by server. Another host may be connecting with it\n" );
        return 1;
    }
    else
    {
        fprintf(stderr, "Connected with the server @ %s\n", hostname);
    }

    if ( g_hSocketMutex == 0 )
    {
         iError = bcmCreateMutex(&g_hSocketMutex);
        if (iError)
        {
            fprintf(stderr, "Failed to create mutex.\n");
        }
    }
done:
    return iError;
}

static int _closeSocket
    ( bool shutdown )
{
    int iError = 0;
    char c;

    /* This flag is use to shutdown the server.  When done
     * with the emulator set this c=STOP_SERVER; set it to STOP_CLIENT
     * if only to stop client. */

    c = shutdown ? SOCKET_STOP_SERVER : SOCKET_STOP_CLIENT;
    fprintf(stderr, "Sending terminate messages to regscope  %s...\n", shutdown ? "server" : "client");

    /* This mutex is to protect the shared socket used by threads of the client! */
    if (0 != bcmAcquireMutex(g_hSocketMutex))
    {
        fprintf(stderr, "Failed to acquire mutex in bcmCloseSocket\n");
        return SOCKET_ERROR;
    }

    iError = my_send( g_sockfd, &c, 1, 0 );

    if( iError != 1 )
    {
        perror("send() error");
        goto done ;
    }

    close(g_sockfd);

    iError = 0;
done:

    if ( g_hSocketMutex )
    {
        bcmReleaseMutex(g_hSocketMutex);
        bcmDestroyMutex(g_hSocketMutex);
        g_hSocketMutex = 0;
    }

    return iError;
}
#else /* WIN32 */

#define WIN32_LEAN_AND_MEAN

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "9999"

int __cdecl _openSocket
    ( const char * hostname,
      const unsigned short portnum )
{
    WSADATA wsaData;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    int iResult;
	char in_buffer[2];
    char port[DEFAULT_BUFLEN];


    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        goto error;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = _itoa_s(portnum, port, DEFAULT_BUFLEN, 10);
    if (iResult)
    {
        goto error;
    }

    /* Resolve the server address and port */
    iResult = getaddrinfo(hostname, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        goto error;
    }

    /* Attempt to connect to an address until one succeeds */
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next)
    {
        /* Create a SOCKET for connecting to server */
        g_sockfd = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (g_sockfd == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            goto error;
        }

        /* Connect to server. */
        iResult = connect( g_sockfd, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(g_sockfd);
            g_sockfd = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    iResult = recv( g_sockfd, in_buffer, 1, 0 );
    if( iResult <= 0 )
    {
        printf( "\nConnection is rejected by server. Another host may be connecting with it.\n\n" );
        goto error;
    }
    else
    {
        printf( "\nConnected with the server.\n" );
        goto done;
    }

error:
    return 1;
done:
    return 0;
}

int __cdecl _closeSocket
    ( bool shutdown )
{
    char c;
    int iError;

    c = shutdown ? SOCKET_STOP_SERVER : SOCKET_STOP_CLIENT;

    iError = send( g_sockfd, &c, 1, 0 );
    if( iError < 1 )
    {
        pout( "send()", WSAGetLastError() );
    }

#if 0
    /* shutdown the connection since no more data will be sent */
    iResult = shutdown(g_sockfd, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
    }
#endif
    /* cleanup */
    iError = closesocket(g_sockfd);
    if( iError == SOCKET_ERROR )
    {
        pout( "closesocket()", WSAGetLastError() );
    }

    iError = WSACleanup();
    if( iError )
    {
        pout( "WSACleanup()", WSAGetLastError() );
    }

    return iError;
}

#endif
