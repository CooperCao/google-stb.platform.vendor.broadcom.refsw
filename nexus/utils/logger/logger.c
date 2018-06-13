/******************************************************************************
 *  Copyright (C) 2016-2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
#include "blst_squeue.h"
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
#include <pthread.h>
#include "nexus_driver_ioctl.h"

BDBG_FILE_MODULE(logger);

static bool sigusr1_fired=false;

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


static BDBG_Level get_log_message_level(const char *pMsgBuf, size_t msgLen)
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
#if 0
    if(*pMsgLen) {
        BDBG_MODULE_ERR(logger, ("%u %u", (unsigned)*pMsgLen, (unsigned)strlen(pBuf)));
    }
#endif
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
        BDBG_MODULE_MSG(logger, ("Can't read data from the driver"));
        *pMsgLen = 0;
        BERR_TRACE(urc);
        return BERR_UNKNOWN;
    }

    *pTimeout = dequeue.timeout;    
    *pMsgLen  = dequeue.buffer_size;

#if 0
    if(*pMsgLen) {
        BDBG_MODULE_ERR(logger, ("%u %u", (unsigned)*pMsgLen, (unsigned)strlen(pBuf)));
    }
#endif

    return BERR_SUCCESS;
}


#define MAX_PREFIX_LEN    16      /* Reserve this many bytes in front of message. */
#define MAX_MSG_LEN      256      /* Reserve this many bytes for message.         */
#define MAX_SUFFIX_LEN     8      /* Reserve this many bytes after message.       */

struct msg_buf {
    /* Allocate a buffer that can hold the message, but reserve
     * some space for a prefix and suffix.  */
    char buf[MAX_PREFIX_LEN + MAX_MSG_LEN + MAX_SUFFIX_LEN + 1]; /* + 1 for ending newline char */
    unsigned offset;
    size_t msgLen;
};

struct msg_buf_queue_entry {
    struct msg_buf buf;
    BLST_SQ_ENTRY(msg_buf_queue_entry) link;
};

struct msg_buf_queue {
    BLST_SQ_HEAD(msg_buf_queue_head, msg_buf_queue_entry) list;
    unsigned count; /* number of entries in queue */
};

struct stderr_thread_state {
    struct msg_buf_queue queue;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int stderr_fd;
    bool exit;
};

struct write_sink {
    int stderr_fd;
    FILE *file_fd;
    bool chopTimestamps;
    bool ansiColorsEnabled;
    struct stderr_thread_state thread;
};

static void msg_buf_queue_init(struct msg_buf_queue *queue)
{
    BLST_SQ_INIT(&queue->list);
    queue->count = 0;
    return;
}

static void msg_buf_queue_enqueue(struct msg_buf_queue *queue, const struct msg_buf *buf)
{
    struct msg_buf_queue_entry *e=BKNI_Malloc(sizeof(*e));
    BDBG_ASSERT(e);
    e->buf = *buf;
    BLST_SQ_INSERT_TAIL(&queue->list, e, link);
    queue->count++;
    return;
}

static bool msg_buf_queue_dequeue(struct msg_buf_queue *queue, struct msg_buf *buf)
{
    struct msg_buf_queue_entry *e=BLST_SQ_FIRST(&queue->list);
    if(e) {
        if(buf) {
            *buf = e->buf;
        }
        BLST_SQ_REMOVE_HEAD(&queue->list, link);
        queue->count--;
        BKNI_Free(e);
        return true;
    }
    return false;
}

static void msg_buf_queue_uninit(struct msg_buf_queue *queue)
{
    for(;;) {
        if(!msg_buf_queue_dequeue(queue, NULL)) {
            break;
        }
    }
    return;
}

static void *stderr_sink_thread(void *_t)
{
    struct stderr_thread_state *t = _t;
    int rc;

    rc = pthread_mutex_lock(&t->mutex);
    BDBG_ASSERT(rc==0);
    while(!t->exit) {
        struct msg_buf buf;
        if(!msg_buf_queue_dequeue(&t->queue, &buf)) {
            pthread_cond_wait(&t->cond, &t->mutex);
        } else {
            rc=pthread_mutex_unlock(&t->mutex);
            BDBG_ASSERT(rc==0);
            rc = write(t->stderr_fd, buf.buf+buf.offset, buf.msgLen);
            if (rc) BERR_TRACE(rc); /* keep going */
            rc=pthread_mutex_lock(&t->mutex);
            BDBG_ASSERT(rc==0);
        }
    }
    rc=pthread_mutex_unlock(&t->mutex);
    BDBG_ASSERT(rc==0);
    return NULL;
}

static void stderr_thread_start(struct stderr_thread_state *t, int stderr_fd)
{
    int rc;
    t->exit = false;
    t->stderr_fd = stderr_fd;
    msg_buf_queue_init(&t->queue);
    rc = pthread_mutex_init(&t->mutex, NULL);
    BDBG_ASSERT(rc==0);
    rc = pthread_cond_init(&t->cond, NULL);
    BDBG_ASSERT(rc==0);
    rc = pthread_create(&t->thread, NULL, stderr_sink_thread, t);
    BDBG_ASSERT(rc==0);
    return;
}

static void stderr_thread_stop(struct stderr_thread_state *t)
{
    int rc;
    unsigned stuck_count = 0;
    unsigned prev_count = 0;

    rc = pthread_mutex_lock(&t->mutex);
    BDBG_ASSERT(rc==0);
    while(t->queue.count) { /* wait for thread to write all data */
        rc=pthread_mutex_unlock(&t->mutex);
        BDBG_ASSERT(rc==0);
        BKNI_Sleep(100);
        rc = pthread_mutex_lock(&t->mutex);
        BDBG_ASSERT(rc==0);
        if(t->queue.count==prev_count) {
            stuck_count++;
            if(stuck_count>10) {
                /* thread stuck on I/O cancel it */
                rc = pthread_cancel(t->thread);
                BDBG_ASSERT(rc==0);
                break;
            }
        } else {
            prev_count = t->queue.count;
            stuck_count = 0;
        }
    }
    t->exit = true; /* signal thread to exit */
    rc = pthread_mutex_unlock(&t->mutex);
    BDBG_ASSERT(rc==0);
    rc = pthread_cond_signal(&t->cond);
    BDBG_ASSERT(rc==0);

    rc = pthread_join(t->thread, NULL);
    BDBG_ASSERT(rc==0);

    rc = pthread_mutex_destroy(&t->mutex);
    BDBG_ASSERT(rc==0);
    rc = pthread_cond_destroy(&t->cond);
    BDBG_ASSERT(rc==0);

    msg_buf_queue_uninit(&t->queue);
    return;
}

static void stderr_thread_enqueue(struct stderr_thread_state *t, const struct msg_buf *buf)
{
    int rc;

    rc = pthread_mutex_lock(&t->mutex);
    BDBG_ASSERT(rc==0);
    msg_buf_queue_enqueue(&t->queue, buf);
    rc=pthread_mutex_unlock(&t->mutex);
    BDBG_ASSERT(rc==0);
    rc = pthread_cond_signal(&t->cond);
    BDBG_ASSERT(rc==0);
    return;
}

static int write_sink_open(struct write_sink *sink)
{
    const char   *debug_log_file = getenv("debug_log_file");
    sink->stderr_fd = STDERR_FILENO;
    sink->file_fd = NULL;
    sink->chopTimestamps = false;
    sink->ansiColorsEnabled = false;
    if(debug_log_file) {
        sink->file_fd = fopen(debug_log_file, "w");
        if(sink->file_fd) {
            stderr_thread_start(&sink->thread, sink->stderr_fd);
        } else {
            perror(debug_log_file);
        }
    }

    {
        const char *s = getenv("BDBG_TIMESTAMPS");
        if (s && !strcmp(s, "n")) {
            sink->chopTimestamps = true;
        }
    }
    {
        const char    * envString = getenv("debug_log_colors");

        if (envString  &&  strcmp(envString,"ansi")==0  &&  isatty(sink->stderr_fd)) {
            sink->ansiColorsEnabled = true;
        }
    }
    return 0;
}

static void write_sink_close(struct write_sink *sink)
{
    if(sink->file_fd) {
        fflush(sink->file_fd);
        stderr_thread_stop(&sink->thread);
        fclose(sink->file_fd);
    }
    return;
}

static void write_sink_flush(struct write_sink *sink)
{
    if(sink->file_fd) {
        fflush(sink->file_fd);
    }
    return;
}

static void write_sink_msg_buf(struct write_sink *sink, struct msg_buf *buf)
{
    char  *pMsgBuf = buf->buf + buf->offset;
    size_t msgLen = buf->msgLen;

    if(sink->file_fd) {
        fwrite(pMsgBuf, msgLen, 1, sink->file_fd);
        fputc('\n', sink->file_fd);
    }
#ifdef  ENABLE_ANSI_COLORS
    if (sink->ansiColorsEnabled) {
        BDBG_Level msgLevel = get_log_message_level(pMsgBuf, msgLen);
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

    pMsgBuf[msgLen]  = '\n';
    msgLen++;

#define BDBG_TIMESTAMP_LEN 17
    if (sink->chopTimestamps && buf->msgLen >= BDBG_TIMESTAMP_LEN) {
        /* don't chop BERR_TRACE */
        if (strncmp(pMsgBuf, "!!!Error", 8)) {
            pMsgBuf += BDBG_TIMESTAMP_LEN;
            msgLen -= BDBG_TIMESTAMP_LEN;
        }
    }
    buf->offset = pMsgBuf - buf->buf;
    buf->msgLen = msgLen;

    if(sink->file_fd) {
        stderr_thread_enqueue(&sink->thread, buf);
    } else {
        int urc;
        urc = write(sink->stderr_fd, pMsgBuf, msgLen);
    }
    return;
}

static void write_sink_data(struct write_sink *sink, const void *data, size_t size)
{
    struct msg_buf buf;
    char *pMsgBuf;

    buf.offset = MAX_PREFIX_LEN;
    pMsgBuf = buf.buf + buf.offset;
    BDBG_ASSERT( (pMsgBuf - buf.buf) + size <= sizeof(buf.buf));
    buf.msgLen = size;
    BKNI_Memcpy(pMsgBuf, data, size);
    write_sink_msg_buf(sink, &buf);
    return;
}


static BERR_Code print_log_message(struct write_sink *sink, BDBG_FifoReader_Handle logReader, int deviceFd, PROXY_NEXUS_Log_Instance *pProxyInstance,  unsigned *pTimeout )
{
    BERR_Code   rc;
    struct msg_buf buf;
    char       *pMsgBuf;

    buf.offset = MAX_PREFIX_LEN;
    pMsgBuf = buf.buf + buf.offset;


    /* Retrieve the next log message as requested.  Depending on    
     * what arguments were passed, we'll either get the message  
     * directly from BDBG, or we'll have to go through the proxy  
     * to get it.  */
    BDBG_ASSERT(logReader || pProxyInstance);
    rc = BERR_SUCCESS;
    if (logReader) {
        rc = get_usermode_log_message(logReader, pTimeout, pMsgBuf, MAX_MSG_LEN, &buf.msgLen);
    }
    else if (pProxyInstance) {
        rc = get_driver_log_message(deviceFd, pProxyInstance, pTimeout, pMsgBuf, MAX_MSG_LEN, &buf.msgLen);
    }
    if (rc != BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }

    if (buf.msgLen > 0) {
        write_sink_msg_buf(sink, &buf);
    }

    return BERR_SUCCESS;
}


int main(int argc, char * const argv[])
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
    struct write_sink sink;

    if(argc<2) {
        usage(argv[0]);
    }
    BSTD_UNUSED(argv);

    rc = BKNI_Init();
    BDBG_ASSERT(rc==BERR_SUCCESS);
    rc = BDBG_Init();
    BDBG_ASSERT(rc==BERR_SUCCESS);


    /* coverity[var_assign_var] */
    fname = argv[1];
    BDBG_ASSERT(fname);
    /* coverity[tainted_string] */
    if ( strcmp(fname, "disabled") ) {
        fd = open(fname, O_RDWR);
        if(fd<0) {
            perror(fname);
            usage(argv[0]);
        }
    }
    write_sink_open(&sink);

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
        shared = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
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
                rc = print_log_message(&sink, logReader, -1, NULL, &timeout );
                BSTD_UNUSED(rc);
            } else {
                timeout = 5;
            }

            if(driver_ready) {
                rc = print_log_message(&sink, NULL, device_fd, &instance, &driverTimeout );

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
            static const char terminated[] = "_____ TERMINATED _____";
            write_sink_data(&sink, terminated, sizeof(terminated)-1);
            break;
        }
        write_sink_flush(&sink);
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
    write_sink_close(&sink);
    BDBG_Uninit();
    /* BKNI_Uninit(); Don't call it since this would print memory allocation stats */
    return 0;
}
