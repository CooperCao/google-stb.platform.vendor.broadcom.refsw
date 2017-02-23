/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bdbg_fifo.h"
#include "bdbg_log.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "nexus_driver_ioctl.h"

BDBG_MODULE(logger);

static bool sigusr1_fired=false;
static bool g_chopTimestamps = false;

static void sigusr1_handler(int unused)
{
    BSTD_UNUSED(unused);
    sigusr1_fired = true;
    return;
}

static void usage(const char *name)
{
    fprintf(stderr, "Usage:\n%s log_file [device_node]\n", name);
    exit(0);
}


static BDBG_Level get_log_message_level(char *pMsgBuf, size_t msgLen)
{
    BDBG_Level level = BDBG_P_eUnknown;
    char ch;

    /* We're going to look at the first three characters, so make   
     * sure we have that many.  */
    if (msgLen < 3) {
        return BDBG_P_eUnknown;
    }

    /* Now make sure the first three characters are all same. */
    ch = *pMsgBuf++;
    if (ch != *pMsgBuf++  || ch != *pMsgBuf) {
        return BDBG_P_eUnknown;
    }

    /* Now just convert the 3-char prefix into a BDBG_Level, just   
     * the reverse of what is done by the gDbgPrefix[] array in  
     * bdbg.c.  */
    switch (ch) {
        case '.':   level = BDBG_eTrace;    /* ... */   break;
        case '-':   level = BDBG_eMsg;      /* --- */   break;
        case '*':   level = BDBG_eWrn;      /* *** */   break;
        case '#':                           /* ### */
        case '!':   level = BDBG_eErr;      /* !!! */   break;
        case ' ':   level = BDBG_eLog;      /*     */   break;
        default:    level = BDBG_P_eUnknown;            break;
    }
    return level;
}

#define ENABLE_ANSI_COLORS
#ifdef  ENABLE_ANSI_COLORS


static const char  * get_color_suffix_for_level(BDBG_Level level)
{
    static char     colorSuffix[] = "\033[0m";

    BSTD_UNUSED(level);

    return colorSuffix;
}


static const char  * get_color_prefix_for_level(BDBG_Level level)
{
    static const char *colorPrefix[] =
    {
       "",                  /* unknown                      */
       "\033[1;32;40m" ,    /* trace        green on black  */
       "",                  /* msg          default colors  */
       "\033[1;33;40m" ,    /* warning      yellow on black */
       "\033[1;31;40m" ,    /* error        red on black    */
       "\033[1;36;40m" ,    /* log          cyan on black   */
    };

    BDBG_CASSERT(sizeof(colorPrefix)/sizeof(*colorPrefix) == BDBG_P_eLastEntry);

    if (level<BDBG_P_eLastEntry) {
       return colorPrefix[level];
    } else {
       return colorPrefix[0];
    }
}
#endif  /* ENABLE_ANSI_COLORS */


static BERR_Code get_usermode_log_message(BDBG_FifoReader_Handle logReader, unsigned *pTimeout, char *pBuf, size_t bufLen, size_t *pMsgLen)
{
    BERR_Code rc;

    rc = BDBG_Log_Dequeue(logReader, pTimeout, pBuf, bufLen, pMsgLen);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);return rc;}
/*  if(*pMsgLen) {                                          */
/*      BDBG_ERR(("%u %u", *pMsgLen, strlen(pBuf)));        */
/*  }                                                       */
    return BERR_SUCCESS;
}


static BERR_Code get_driver_log_message(int device_fd, PROXY_NEXUS_Log_Instance *pProxyInstance, unsigned *pTimeout, char *pBuf, size_t bufLen, size_t *pMsgLen)
{
    int         urc;
    PROXY_NEXUS_Log_Dequeue dequeue;

    dequeue.instance    = *pProxyInstance;
    dequeue.buffer_size = bufLen-1;
    dequeue.buffer      = (unsigned long)pBuf;
    dequeue.timeout     = 0;
    urc = ioctl(device_fd, IOCTL_PROXY_NEXUS_Log_Dequeue, &dequeue);
    if(urc!=0) {
        BDBG_MSG(("Can't read data from the driver"));
        *pMsgLen = 0;
        BERR_TRACE(urc);
        return BERR_UNKNOWN;
    }

    *pTimeout = dequeue.timeout;    
    *pMsgLen  = dequeue.buffer_size;

/*  if(*pMsgLen) {                                          */
/*      BDBG_ERR(("%u %u", *pMsgLen, strlen(pBuf)));        */
/*  }                                                       */

    return BERR_SUCCESS;
}


#define MAX_PREFIX_LEN    16      /* Reserve this many bytes in front of message. */
#define MAX_MSG_LEN      256      /* Reserve this many bytes for message.         */
#define MAX_SUFFIX_LEN     8      /* Reserve this many bytes after message.       */

static BERR_Code print_log_message(BDBG_FifoReader_Handle logReader, int deviceFd, PROXY_NEXUS_Log_Instance *pProxyInstance,  unsigned *pTimeout )
{
    BERR_Code   rc;
    int         urc;

    /* Allocate a buffer that can hold the message, but reserve     
     * some space for a prefix and suffix.  */
    char        buf[MAX_PREFIX_LEN + MAX_MSG_LEN + MAX_SUFFIX_LEN + 1]; /* + 1 for ending newline char */
    char       *pMsgBuf = buf + MAX_PREFIX_LEN;
    size_t      msgLen;

#ifdef  ENABLE_ANSI_COLORS
    static bool     ansiColorsEnabled = false;
    static bool     firstTime = true;

    /* Enable output color-coding if the environment has            
     * "debug_log_colors=ansi" but also make sure that stderr is  
     * going to a tty (and not redirected to a file).  */
    if (firstTime) {
        const char    * envString = getenv("debug_log_colors");

        firstTime = false;

        if (envString  &&  strcmp(envString,"ansi")==0  &&  isatty(STDERR_FILENO)) {
            ansiColorsEnabled = true;
        }
    }
#endif  /* ENABLE_ANSI_COLORS */

    /* Retrieve the next log message as requested.  Depending on    
     * what arguments were passed, we'll either get the message  
     * directly from BDBG, or we'll have to go through the proxy  
     * to get it.  */
    BDBG_ASSERT(logReader || pProxyInstance);
    rc = BERR_SUCCESS;
    if (logReader) {
        rc = get_usermode_log_message(logReader, pTimeout, pMsgBuf, MAX_MSG_LEN, &msgLen);
    }
    else if (pProxyInstance) {
        rc = get_driver_log_message(deviceFd, pProxyInstance, pTimeout, pMsgBuf, MAX_MSG_LEN, &msgLen);
    }
    if (rc != BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }

    if (msgLen > 0) {
        BDBG_Level msgLevel = get_log_message_level(pMsgBuf, msgLen);
#ifdef  ENABLE_ANSI_COLORS
        if (ansiColorsEnabled) {
            const char * prefixString = get_color_prefix_for_level(msgLevel);
            size_t prefixLen = strlen(prefixString);

            const char * suffixString =  get_color_suffix_for_level(msgLevel);
            size_t suffixLen = strlen(suffixString);

            /* Make sure the prefix and suffix can fit into the space       
             * that's been reserved for them.  */
            BDBG_ASSERT(prefixLen <= MAX_PREFIX_LEN);
            BDBG_ASSERT(suffixLen <= MAX_SUFFIX_LEN);

            /* Insert the prefix in front of the message. */
            pMsgBuf -= prefixLen;
            msgLen  += prefixLen;
            BKNI_Memcpy(pMsgBuf, prefixString, prefixLen);

            /* Insert the suffix after the message. */
            BKNI_Memcpy(pMsgBuf+msgLen, suffixString, suffixLen);
            msgLen += suffixLen;
        }
#endif  /* ENABLE_ANSI_COLORS */

        /* BDBG_ERR(("%u %u", dbgStrLen, strlen(dbgStr))); */
        pMsgBuf[msgLen]  = '\n';
        msgLen++;

#define BDBG_TIMESTAMP_LEN 17
        if (g_chopTimestamps && msgLen >= BDBG_TIMESTAMP_LEN) {
            /* don't chop BERR_TRACE */
            if (strncmp(pMsgBuf, "!!!Error", 8)) {
                pMsgBuf += BDBG_TIMESTAMP_LEN;
                msgLen -= BDBG_TIMESTAMP_LEN;
            }
        }

        urc = write(STDERR_FILENO, pMsgBuf, msgLen); 

        /* BDBG_ERR(("%d %d %d", urc, msgLen, strlen(buf)));*/
        /* BDBG_ASSERT(urc==(int)dbgStrLen); */
    }

    return BERR_SUCCESS;
}


int main(int argc, const char *argv[])
{
    BERR_Code rc;
    BDBG_FifoReader_Handle logReader=NULL;
    BDBG_Fifo_Handle logWriter=NULL;
    int fd=-1;
    int device_fd=-1;
    bool driver_ready = false;
    const char *fname;
    size_t size=0;
    void *shared;
    struct stat st;
    int urc;
    pid_t parent;
    PROXY_NEXUS_Log_Instance instance;

    if(argc<2) {
        usage(argv[0]);
    }
    BSTD_UNUSED(argv);

    rc = BKNI_Init();
    BDBG_ASSERT(rc==BERR_SUCCESS);
    rc = BDBG_Init();
    BDBG_ASSERT(rc==BERR_SUCCESS);

    {
        const char *s = getenv("BDBG_TIMESTAMPS");
        if (s && !strcmp(s, "n")) {
            g_chopTimestamps = true;
        }
    }

    /* coverity[var_assign_var] */
    fname = argv[1];
    BDBG_ASSERT(fname);
    /* coverity[tainted_string] */
    if ( strcmp(fname, "disabled") ) {
        fd = open(fname, O_RDONLY);
        if(fd<0) {
            perror(fname);
            usage(argv[0]);
        }
    }

    if(argc>2 && argv[2][0]!='\0') {
        int ready;
        /*( device_fd = open(argv[2],O_RDWR); */
        device_fd = atoi(argv[2]);
        if(device_fd<0) {
            perror(argv[2]);
            usage(argv[0]);
        }
        urc = ioctl(device_fd, IOCTL_PROXY_NEXUS_Log_Test, &ready);
        if(urc!=0) {
            perror(argv[2]);
            usage(argv[0]);
        }
        if(ready) {
            urc = ioctl(device_fd, IOCTL_PROXY_NEXUS_Log_Create, &instance);
            if(urc==0) {
                driver_ready = true;
            }
        }
    }
    /* unlink(fname); don't remove file, allow multiple copies of logger */
    if ( fd >= 0 ) {
        urc = fstat(fd, &st);
        if(urc<0) {
            perror("stat");
            usage(argv[0]);
        }
    }
    parent = getppid();
    signal(SIGUSR1,sigusr1_handler);
    if ( fd >= 0 ) {
        size = st.st_size;
        shared = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
        if(shared==MAP_FAILED) {
            perror("mmap");
            usage(argv[0]);
        }
        logWriter = (BDBG_Fifo_Handle)shared;
        rc = BDBG_FifoReader_Create(&logReader, logWriter);
        BDBG_ASSERT(rc==BERR_SUCCESS);
    }
    if(argc>3) {
        int ready_fd = atoi(argv[3]);
        char data[1]={'\0'};
        int rc;
        rc = write(ready_fd,data,1); /* signal parent that we've started */
        close(ready_fd);
    }
    for(;;) {
        unsigned timeout;
        unsigned driverTimeout;

        for(;;) {
            if ( logReader ) {
                timeout = 0;
                rc = print_log_message(logReader, -1, NULL, &timeout );
            } else {
                timeout = 5;
            }

            if(driver_ready) {
                rc = print_log_message(NULL, device_fd, &instance, &driverTimeout );

                if (rc == BERR_SUCCESS && timeout>driverTimeout) {
                    timeout = driverTimeout;
                }
            }
            if(timeout!=0) {
                break;
            }
        }
        BDBG_ASSERT(timeout);
        if(sigusr1_fired) {
            goto done;
        }
        if(parent == 1 || kill(parent, 0)) {
            static const char terminated[] = "_____ TERMINATED _____\n";
            write(STDERR_FILENO, terminated, sizeof(terminated)-1);
            break;
        }
        BKNI_Sleep(timeout);
        if(device_fd>=0 && !driver_ready) {
            int ready;
            /* if parent was dealyed keep trying to attach */
            urc = ioctl(device_fd, IOCTL_PROXY_NEXUS_Log_Test, &ready);
            if(urc!=0) {
                goto done;
            }
            if(ready) {
                urc = ioctl(device_fd, IOCTL_PROXY_NEXUS_Log_Create, &instance);
                if(urc!=0) {
                    goto done;
                }
                driver_ready = true;
            }
        }
    }
done:
    if(device_fd>=0) {
        close(device_fd); 
    }
    if ( fd >= 0 ) {
        close(fd);
        if (logReader)
            BDBG_FifoReader_Destroy(logReader);
    }
    BDBG_Uninit();
    /* BKNI_Uninit(); Don't call it since this would print memory allocation stats */
    return 0;
}
