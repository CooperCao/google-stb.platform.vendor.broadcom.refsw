/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* Description: Live Streaming of Data to a Remote Client
*
***************************************************************************/

#if defined(LINUX) || defined(__vxworks)

#include "b_playback_ip_lib.h"
#include "b_playback_ip_priv.h"
#include "b_playback_ip_utils.h"

BDBG_MODULE(b_playback_ip_live_streaming);
BDBG_FILE_MODULE(b_playback_ip_live_streaming_summary);
#define PRINTMSG_SUMMARY(bdbg_args)         BDBG_MODULE_MSG(b_playback_ip_live_streaming_summary, bdbg_args);

#if defined(LINUX)
#include <sys/syscall.h>
#ifdef __mips__
#include <asm/cachectl.h>
#endif
#include <errno.h>
/* needed for the tcpconnect */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#endif

static bool enableRecording = false; /* gets set via env variable if we need to record a live channel for testing purpose */
#define LIVE_STREAMING_DATA_READY_TIMEOUT_INTERVAL 50
#define LIVE_STREAMING_DATA_READY_MAX_TIMEOUT 50000
#define HLS_RESUME_STREAMING_TIMEOUT 3000
#define BRCM_TPIT_ENTRY_SIZE 24 /* Each TPIT entry is 6 dwords: 24 bytes */
#define GOPS_IN_A_SEGMENT 1 /* pass it from app: TODO: RT mode may have mute frames in start, so keep GOP count higher */
#define MAX_TRACKED_EVENTS 2
/* ******************************************************************************************************
*                                                                                                       *
*   B_PlaybackIp_UtilsDumpBuffer - dumps the specified buffer in hex and ascii                          *
*                                                                                                       *
********************************************************************************************************/
int
B_PlaybackIp_UtilsDumpBuffer ( const char * funccaller, const unsigned char * buffer, unsigned long int buflen )
{
    char line[128];
    char nibble[3];
    unsigned int idx=0;
    unsigned char * pos = (unsigned char*) buffer;


    memset(line, 0, sizeof line);
    for (idx=0; idx<buflen;idx++) {
        snprintf(nibble, sizeof(nibble)-1, "%02x", pos[idx]);
        nibble[2] = '\0';
        if ( (strlen(line) + strlen(nibble)  + 1) < sizeof(line) ) {
            strncat(line, nibble, sizeof(line)-1 );
        } else {
            BDBG_WRN(("%s: line buffer len (%zu) exceeded", funccaller, sizeof(line) ));
        }
        /* add a space after every two bytes */
        if (idx%2==1) {
            strncat(line, " ", sizeof(line)-1 );
        }
    }
    BDBG_WRN(("%s: (%s)", funccaller, line ));

    return 0;
}

static off_t
B_PlaybackIp_StreamNetDataTrim(bfile_io_write_t self, off_t trim_pos)
{
    BSTD_UNUSED(self);
    BSTD_UNUSED(trim_pos);
    return 0;
}

static off_t
B_PlaybackIp_StreamNetIndexTrim(bfile_io_write_t self, off_t trim_pos)
{
   BSTD_UNUSED(self);
   BSTD_UNUSED(trim_pos);
   return 0;
}


static const struct bfile_io_write net_io_data_write = {
    B_PlaybackIp_UtilsStreamingCtxWrite,
    B_PlaybackIp_StreamNetDataTrim,
    BIO_DEFAULT_PRIORITY
};

static const struct bfile_io_write net_io_index_write = {
    B_PlaybackIp_UtilsStreamingCtxWrite,
    B_PlaybackIp_StreamNetIndexTrim,
    BIO_DEFAULT_PRIORITY
};

void
B_PlaybackIp_NetworkRecordingStop(struct NEXUS_FileRecord *file)
{
    struct bfile_out_net *fileRecord = (struct bfile_out_net *)file;

    BDBG_ASSERT(file);
    BDBG_MSG(("%s: %p", BSTD_FUNCTION, (void *)file));
    if(file->index) {
        if(fileRecord->index.fd>0) {
            BDBG_MSG(("closing index fd %d\n", fileRecord->index.fd));
            close(fileRecord->index.fd);
            fileRecord->index.fd = -1;
        }
    }

    if(fileRecord->data.fd>0) {
        BDBG_MSG(("closing data fd %d\n", fileRecord->data.fd));
        close(fileRecord->data.fd);
        fileRecord->data.fd = -1;
    }

    BKNI_Free(file);
    return;
}

/* NETRECORD This Function will start the  socket Connection  to stream out the data */
B_PlaybackIpError
B_PlaybackIp_NetworkRecordingStart(
    B_PlaybackIpSocketOpenSettingsHandle socketOpenSettings,
    const char *fname,
    const char *indexname,
    B_PlaybackIpPsiInfoHandle psi,
    /*B_PlaybackIpSocketStateHandle socketState,*/
    NEXUS_FileRecordHandle *fileRecordPtr
    )
{
    int rc = -1;
    char *wbuf = NULL;
    struct bfile_out_net *fileRecord = NULL;

    /*BSTD_UNUSED(socketState);*/
    BDBG_MSG(("%s:\n", BSTD_FUNCTION));

    wbuf = (char *)BKNI_Malloc(TMP_BUF_SIZE);
    fileRecord = BKNI_Malloc(sizeof(*fileRecord));
    if (!wbuf || !fileRecord) {
        BDBG_ERR(("%s: memory allocation failure\n", BSTD_FUNCTION));
        rc = B_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    memset(wbuf, 0, TMP_BUF_SIZE);
    memset(fileRecord, 0, sizeof(*fileRecord));

    switch (socketOpenSettings->protocol) {
        /* Build a Post Request then Connect */
    case B_PlaybackIpProtocol_eHttp: {
        if (!psi || !fname ) {
           BDBG_ERR(("%s: Incorrect Args for Network Recording over HTTP, psi %p, fname %p\n",
                       BSTD_FUNCTION, (void *)psi, (void *)fname));
           rc = B_ERROR_INVALID_PARAMETER;
           goto error;
        }

        /* TODO: may need to convert the codecs from Nexus to Settop Type */
        /* as these are used to build Info file which expect values in Settop & not Nexus types */
        snprintf(wbuf, TMP_BUF_SIZE, "POST /%s HTTP/1.1\r\n"
                                 "Stream-Info: yes\r\nPcr-Pid: %x\r\nAudio-Pid: %x\r\n"
                                 "Video-Pid: %x\r\nVideo-Type: %x\r\nAudio-Type: %x\r\n"
                                 "TTS: %d\r\n"
                                 "\r\n"
                                  ,fname,psi->pcrPid,psi->audioPid,psi->videoPid,
                                  psi->videoCodec, psi->audioCodec, psi->transportTimeStampEnabled);

        if (B_PlaybackIp_UtilsTcpSocketConnect(NULL, socketOpenSettings->ipAddr, socketOpenSettings->port, 0 /* non-blocking mode */, 10000, &fileRecord->data.fd) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to Connect to the Server, errno %d\n", BSTD_FUNCTION, errno));
            rc = B_ERROR_OS_ERROR;
            goto error;
        }
        else {
            BDBG_WRN(("Connected filename %s", fname));
            /* now send the Post request */
            rc = write(fileRecord->data.fd,wbuf,strlen(wbuf));
            if (rc <= 0 || (unsigned)rc != strlen(wbuf)) {
                BDBG_ERR(("%s: Failed to Send the Post request for writing AV content to Server, errno %d\n",
                            BSTD_FUNCTION, errno));
                rc = B_ERROR_OS_ERROR;
                goto error;
            }
            fsync(fileRecord->data.fd);
            BKNI_Sleep(200);
        }

        fileRecord->data.self = net_io_data_write;
        fileRecord->self.data = &(fileRecord->data.self);

        /* Check if we also need to record the index data */
        if (indexname) {
            memset(wbuf, 0, TMP_BUF_SIZE);
            BDBG_WRN(("Connected filename %s\n", indexname));
            snprintf(wbuf,TMP_BUF_SIZE, "POST /%s HTTP/1.1\r\n\r\n", indexname);

            if (B_PlaybackIp_UtilsTcpSocketConnect(NULL, socketOpenSettings->ipAddr, socketOpenSettings->port+1, 0 /* non-blocking mode */, 10000, &fileRecord->index.fd) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to Connect to the Server, errno %d\n", BSTD_FUNCTION, errno));
                rc = B_ERROR_OS_ERROR;
                goto error;
            } else {
                BDBG_WRN(("Connected for sending Index data\n"));
                /* send the post request */
                rc = write(fileRecord->index.fd, wbuf, strlen(wbuf));
                if (rc <= 0 || (unsigned)rc != strlen(wbuf)) {
                    BDBG_ERR(("%s: Failed to Send the Post request for writing Index data to Server, errno %d\n",
                                BSTD_FUNCTION, errno));
                    rc = B_ERROR_OS_ERROR;
                    goto error;
                }
                BKNI_Sleep(100);
            }

            fileRecord->index.self = net_io_index_write;
            fileRecord->self.index = &(fileRecord->index.self);
        }

        fileRecord->self.close = B_PlaybackIp_NetworkRecordingStop;
        rc = B_ERROR_SUCCESS;
        break;
        }
    default:
        BDBG_ERR((" non-supported socket type"));
        break;
    }

error:
    if (wbuf) BKNI_Free(wbuf);
    if (rc != B_ERROR_SUCCESS) {
        if (fileRecord) {
            if (fileRecord->index.fd)
                close(fileRecord->index.fd);
            if (fileRecord->data.fd)
                close(fileRecord->data.fd);
            BKNI_Free(fileRecord);
        }
    }
    else {
        if (fileRecord)
            *fileRecordPtr = &fileRecord->self;
    }
    return rc;
}

extern int createInfoFile( char *fileName, char *infoFileName, B_PlaybackIpPsiInfoHandle psi);
extern void replaceFileExtension(char *newName, int newNameSize, char *curName, char *newExtension);
static int gFileNameSuffix = 0;
typedef enum {
    B_PlaybackIpLiveStreamingState_eIdle,                    /* initial state */
    B_PlaybackIpLiveStreamingState_eStarting,                 /* LiveStreamingStart is starting */
    B_PlaybackIpLiveStreamingState_eStreaming,               /* Streaming is started */
    B_PlaybackIpLiveStreamingState_eWaitingToResumeStreaming, /* Waiting to resume streaming: app will indicate via SetSettings */
    B_PlaybackIpLiveStreamingState_eStopping,                 /* Streaming is being stopped */
    B_PlaybackIpLiveStreamingState_eStopped,                 /* Streaming is stopped */
    B_PlaybackIpLiveStreamingState_eMax
} B_PlaybackIpLiveStreamingState;

typedef struct B_PlaybackIpLiveStreaming
{
    B_PlaybackIpLiveStreamingOpenSettings settings;
    NEXUS_FilePlayHandle filePlayHandle;
    NEXUS_FifoRecordHandle fifoFileHandle;
    NEXUS_RecpumpHandle recpumpHandle;
    int fd;
    FILE *fp;
    int indexFileFd;
    FILE *indexFileFp;
    struct bfile_io_write_net data;
    B_ThreadHandle streamingThread;
    bool stop;  /* set when app wants to stop the streaming session */
    bool threadRunning; /* set when file streaming thread is running */
    bool threadReStarted; /* set when file streaming thread is running */
    BKNI_EventHandle startStreamingEvent;
    BKNI_EventHandle stopStreamingEvent;
    BKNI_EventHandle resumeStreamingEvent;
    BKNI_EventHandle dataReadyEvent;
    BKNI_EventHandle hDataReadyEventLocal;
    off_t totalBytesStreamed;
    B_PlaybackIpConnectionState connectionState;    /* state of the socket: active, timeouts, error, eof */
    B_PlaybackIpPsiInfo psi;
    unsigned char *streamingBuf;
    unsigned char *streamingBufOrig;
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    unsigned char *clearBuf;
#endif
    unsigned streamingBufSize;
    FILE *fclear;
    int fileNameSuffix;
    unsigned totalHlsSegmentsSent;
    B_EventGroupHandle eventGroup;
    unsigned maxTriggeredEvents;
    B_PlaybackIpLiveStreamingState state;
    bool resetStreaming;
    bool abortStreaming; /* flag set during the back to back seek events to indicate streaming thread to abort its current streaming operation as new request has come in */
#define HLS_PAT_PMT_BUFFER_SIZE 188
    uint8_t patBuffer[HLS_PAT_PMT_BUFFER_SIZE];
    uint8_t pmtBuffer[HLS_PAT_PMT_BUFFER_SIZE];
    bool sendPatPmt;
    bool ipVerboseLog;
}B_PlaybackIpLiveStreaming;

/* Functions to stream live content from a QAM/VSB/IP Source to a local client */
void
B_PlaybackIp_LiveStreamingClose(B_PlaybackIpLiveStreamingHandle liveStreamingHandle)
{

    if (liveStreamingHandle) {
        BDBG_MSG(("%s: liveStreamingHandle %p, filePlayHandle %p", BSTD_FUNCTION, (void *)liveStreamingHandle, (void *)liveStreamingHandle->filePlayHandle));
#ifdef B_HAS_DTCP_IP
        B_PlaybackIp_UtilsDtcpServerCtxClose(&liveStreamingHandle->data);
#endif
        B_PlaybackIp_UtilsStreamingCtxClose(&liveStreamingHandle->data);
        if (liveStreamingHandle->filePlayHandle) {
            NEXUS_FilePlay_Close(liveStreamingHandle->filePlayHandle);
        }
        if (liveStreamingHandle->hDataReadyEventLocal) {
            BKNI_DestroyEvent( liveStreamingHandle->hDataReadyEventLocal);
            liveStreamingHandle->hDataReadyEventLocal = NULL;
        }
        BKNI_Free(liveStreamingHandle);
    }
    return;
}

static void dataReadyCallbackFromRecpump(
    void *context,
    int   param
    )
{
    BKNI_EventHandle hEvent = context;

    BSTD_UNUSED( param );
    BDBG_ASSERT(hEvent);

    BKNI_SetEvent(hEvent);
} /* dataReadyCallbackFromRecpump */

/* This Function will start the  socket Connection  to stream out the data */
B_PlaybackIpLiveStreamingHandle
B_PlaybackIp_LiveStreamingOpen(
    B_PlaybackIpLiveStreamingOpenSettings *liveStreamingSettings
    )
{
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle = NULL;
    char *infoFileName;
    char *pValue = NULL;

    if (!liveStreamingSettings) {
        BDBG_ERR(("%s: Invalid param: Need to pass-in live streaming open Settings\n", BSTD_FUNCTION));
        goto error;
    }

    liveStreamingHandle = BKNI_Malloc(sizeof(B_PlaybackIpLiveStreaming));
    if (!liveStreamingHandle) {
        BDBG_ERR(("%s: memory allocation failure\n", BSTD_FUNCTION));
        goto error;
    }
    memset(liveStreamingHandle, 0, sizeof(B_PlaybackIpLiveStreaming));

    liveStreamingHandle->settings = *liveStreamingSettings;
    liveStreamingHandle->fifoFileHandle = liveStreamingSettings->fifoFileHandle;
    liveStreamingHandle->recpumpHandle = liveStreamingSettings->recpumpHandle;
    if (liveStreamingSettings->dataReadyEvent)
    {
        liveStreamingHandle->dataReadyEvent = liveStreamingSettings->dataReadyEvent;
    }
    else
    {
        NEXUS_Error nrc = NEXUS_SUCCESS;
        NEXUS_RecpumpSettings recpumpSettings;

        if (BKNI_CreateEvent(&liveStreamingHandle->dataReadyEvent)) {
            BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
            goto error;
        }

        /* Check streamijng mode. */
        NEXUS_Recpump_GetSettings( liveStreamingHandle->recpumpHandle, &recpumpSettings );
        if(liveStreamingHandle->settings.streamingMethod == B_PlaybackIpStreamingMethod_eSystemTimerBased)
        {
            recpumpSettings.data.dataReady.callback = NULL;
            recpumpSettings.data.dataReady.context  = NULL;
        }
        else     /* B_PlaybackIpStreamingMethod_eRaveInterruptBased */
        {
            liveStreamingHandle->hDataReadyEventLocal = liveStreamingHandle->dataReadyEvent;
            recpumpSettings.data.dataReady.callback = dataReadyCallbackFromRecpump;
            recpumpSettings.data.dataReady.context  = liveStreamingHandle->dataReadyEvent;
        }
        nrc = NEXUS_Recpump_SetSettings( liveStreamingHandle->recpumpHandle, &recpumpSettings );
        if (nrc != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Recpump_SetSettings failed to set the dataReadyCallback", BSTD_FUNCTION));
            goto error;
        }
    }

    /* Create Info file */
    if (liveStreamingSettings->psi) {
        infoFileName = liveStreamingSettings->fileName;
        replaceFileExtension(infoFileName, strlen(liveStreamingSettings->fileName), liveStreamingSettings->fileName, B_PLAYBACK_IP_INFO_FILE_EXTENSION);
        if (createInfoFile(liveStreamingSettings->fileName, infoFileName, liveStreamingSettings->psi)) {
            BDBG_ERR(("%s: Failed to create info file %s", BSTD_FUNCTION, liveStreamingSettings->fileName));
            goto error;
        }
    }

    pValue = getenv("enableRecording");
    if (pValue) {
        enableRecording = true;
        BDBG_WRN(("%s: recording of live sessions is enabled...", BSTD_FUNCTION));
    }

    {
        char *pValue = NULL;
        pValue = getenv("ipVerboseLog");
        if (pValue)
            liveStreamingHandle->ipVerboseLog = true;
        else
            liveStreamingHandle->ipVerboseLog = false;
    }
    liveStreamingHandle->fileNameSuffix = gFileNameSuffix;
    gFileNameSuffix++; /* increment the global counter */
    liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eIdle;
    BDBG_MSG(("%s: Done", BSTD_FUNCTION));
    return liveStreamingHandle;

error:
    if (liveStreamingHandle) {
        if (liveStreamingHandle->hDataReadyEventLocal) {
            BKNI_DestroyEvent( liveStreamingHandle->hDataReadyEventLocal);
            liveStreamingHandle->hDataReadyEventLocal = NULL;
        }
        BKNI_Free(liveStreamingHandle);
    }
    return NULL;
}

static void
liveStreamingThreadFromFifo(
    void *data
    )
{
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle = (B_PlaybackIpLiveStreamingHandle)data;
    B_PlaybackIpLiveStreamingOpenSettings *liveStreamingSettings;
    unsigned char *xbuf, *readBuf;
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    unsigned char *clearBuf;
    int cryptoUnAlignedBytes;
    int cryptoAlignedSize;
#endif
    int streamingFd;
    NEXUS_FilePlayHandle filePlayHandle;
    int bytesWritten, bytesToWrite =0, bytesRead, bytesToRead;
    unsigned int loopCount = 0;
    char indexFileName[128];
    int readTimeouts = 0;
    int bufSize;
    bool firstTime = true;
    bool streamingDisabled = false;
    NEXUS_MemoryAllocationSettings allocSettings;

    BDBG_ASSERT(liveStreamingHandle);
    liveStreamingSettings = &liveStreamingHandle->settings;
    streamingFd = liveStreamingSettings->streamingFd;

    /* setup bytes to read: account for transport timestamp & aes encryption */
    if (liveStreamingSettings->transportTimestampEnabled)
        bufSize = (TS_PKT_SIZE+4) * HTTP_AES_BLOCK_SIZE * STREAMING_BUF_MULTIPLE;
    else
        bufSize = TS_PKT_SIZE * HTTP_AES_BLOCK_SIZE * STREAMING_BUF_MULTIPLE;

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    if (liveStreamingSettings->heapHandle)
        allocSettings.heap = liveStreamingSettings->heapHandle;
    if (NEXUS_Memory_Allocate(bufSize + 2*DIO_BLK_SIZE, &allocSettings, (void *)(&liveStreamingHandle->streamingBufOrig))) {
        BDBG_ERR(("%s: memory allocation failure at %d\n", BSTD_FUNCTION, __LINE__));
        goto error;
    }
    xbuf = liveStreamingHandle->streamingBufOrig + DIO_BLK_SIZE;
    liveStreamingHandle->streamingBuf = readBuf = DIO_ALIGN(xbuf);
    bytesToRead = bufSize;
    liveStreamingHandle->streamingBufSize = bufSize;

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (liveStreamingHandle->data.pvrDecKeyHandle) {
        if (NEXUS_Memory_Allocate(bufSize, &allocSettings, (void *)(&clearBuf))) {
            BDBG_ERR(("%s: memory allocation failure at %d\n", BSTD_FUNCTION, __LINE__));
            goto error;
        }
        liveStreamingHandle->clearBuf = clearBuf;
    }
#endif

    replaceFileExtension(indexFileName, strlen(liveStreamingSettings->fileName), liveStreamingSettings->fileName, B_PLAYBACK_IP_INDEX_FILE_EXTENSION);
    filePlayHandle = NEXUS_FifoPlay_Open(liveStreamingSettings->fileName, indexFileName, liveStreamingSettings->fifoFileHandle);
    if (!filePlayHandle) {
        BDBG_ERR(("%s: NEXUS_FifoPlay_Open() failed to open circular file (%s) handle for live streaming, fifoFileHandle %p",
                    BSTD_FUNCTION, liveStreamingSettings->fileName, (void *)liveStreamingSettings->fifoFileHandle));
        goto error;
    }
    liveStreamingHandle->filePlayHandle = filePlayHandle;

    BDBG_WRN(("%s: handle %p, streamingFd %d, streaming buf %p", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, (void *)liveStreamingHandle->streamingBuf));
    /* seek to begining of file */
    while (loopCount++ < 100) {
        if (filePlayHandle->file.data->seek(filePlayHandle->file.data, 0, SEEK_SET) != 0) {
            if (loopCount == 100) {
                BDBG_ERR(("%s: underflow, no data coming in, return failure, fd %p, loopCount %d", BSTD_FUNCTION, (void *)filePlayHandle->file.data, loopCount));
                goto error;
            }
            else {
                BDBG_MSG(("%s: underflow: no data yet in the FIFO, retrying after small sleep, offset %d for fd %p, loopCount %d", BSTD_FUNCTION, 0, (void *)filePlayHandle->file.data, loopCount));
            }
            BKNI_Sleep(130);
            continue;
        }
        break;
    }

    loopCount = 0;
    liveStreamingHandle->stop = false;
    liveStreamingHandle->threadRunning = true;
    liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eActive;
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    cryptoAlignedSize = bufSize;
#endif

    if (liveStreamingHandle->startStreamingEvent)
        BKNI_SetEvent(liveStreamingHandle->startStreamingEvent);
    BDBG_MSG(("%s: Ready for streaming: handle %p, streamingFd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd));
    while (!liveStreamingHandle->stop) {
        readBuf = liveStreamingHandle->streamingBuf;
        if (readTimeouts > 200) {
            liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
            BDBG_WRN(("%s: too many timeouts (cnt %d, time %d) while reading the live fifo, breaking out of streaming loop", BSTD_FUNCTION, readTimeouts, (readTimeouts*100)/1000));
            break;
        }
        if (liveStreamingHandle->data.fd <= 0) {
            off_t curOffset;
            BDBG_MSG(("%s: streaming session is not yet enabled, skipping %d bytes", BSTD_FUNCTION, bytesToWrite));
            streamingDisabled = true;
            curOffset = filePlayHandle->file.data->seek(filePlayHandle->file.data, 0, SEEK_CUR);
            BDBG_MSG(("cur offset %"PRId64 ", ts aligned mod %" PRId64 ", aligned %"PRId64, curOffset, curOffset%188, curOffset - curOffset%188));
        }
        else {
            if (streamingDisabled) {
                off_t curOffset;
                int offset;
                /* streaming was disabled, but is now being enabled */
                streamingDisabled = false;
                /* reset the file offset to TS friendly point and resume streaming from there */
                curOffset = filePlayHandle->file.data->seek(filePlayHandle->file.data, 0, SEEK_CUR);
                BDBG_WRN(("enable streaming: cur offset %"PRId64 ", ts aligned mod %d, aligned %"PRId64, curOffset, (int)curOffset%188, curOffset - curOffset%188));
                /* calculate offset to go back */
                offset = curOffset%(512*188); /* to make the offset DIO & TS aligned */
#if 1
                offset += (512*188*10*4); /* goback additional few seconds so as to allow quick TSM lock at the client */
#endif
                if (filePlayHandle->file.data->seek(filePlayHandle->file.data, -curOffset, SEEK_CUR) < 0) {
                    BDBG_ERR(("%s: failed to shift back the current ffset by %d amount, errno %d", BSTD_FUNCTION, (int)curOffset, errno));
                    break;
                }
                curOffset = filePlayHandle->file.data->seek(filePlayHandle->file.data, 0, SEEK_CUR);
                BDBG_WRN(("enable streaming: aligned cur offset %"PRId64 " by %d bytes ", curOffset, offset));
            }
        }
        BDBG_MSG(("readBUf %p, bytesToRead %d\n", (void *)readBuf, bytesToRead));
        if ((bytesRead = (int)filePlayHandle->file.data->read(filePlayHandle->file.data, (void *)readBuf, (size_t)bytesToRead)) <= 0) {
            BDBG_MSG(("%s: read returned %d errno %d\n", BSTD_FUNCTION, bytesRead, errno));
            if (errno == EINTR) {
                BDBG_WRN(("%s: read Interrupted, retrying errno %d\n", BSTD_FUNCTION, errno));
                BKNI_Sleep(100);
                continue;
            }
            if (bytesRead == 0) {
                /* reached file end */
                BDBG_MSG(("%s: reached EOF for circular file, sleep for 1 second and then retry reading", BSTD_FUNCTION));
                readTimeouts++;
                BKNI_Sleep(100);
                continue;
            }
            else {
                BDBG_WRN(("%s: read error for circular file fd %p, rewinding to begining", BSTD_FUNCTION, (void *)filePlayHandle->file.data));
                readTimeouts++;
                BKNI_Sleep(100);
                filePlayHandle->file.index->seek(filePlayHandle->file.index, 0, SEEK_SET);
                filePlayHandle->file.data->seek(filePlayHandle->file.data, 0, SEEK_SET);
                continue;
            }
        }
        if (firstTime) {
            BDBG_WRN(("%s: bytesRead %d bytesToRead %d, sd %d", BSTD_FUNCTION, bytesRead, bytesToRead, streamingFd));
            firstTime = false;
        }
        readTimeouts = 0;
        if (bytesRead != bytesToRead)
            BDBG_MSG(("%s: bytesRead %d bytesToRead %d, sd %d, aligned %d", BSTD_FUNCTION, bytesRead, bytesToRead, streamingFd, bytesRead % 188 ));
        bytesToWrite = bytesRead;
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
        if (liveStreamingHandle->data.pvrDecKeyHandle) {
            /* make sure that buffer size is MPEG2 TS, DIO, & AES Block size aligned */
            /* move past the bytes that were re-read to be dio compliant */
            cryptoUnAlignedBytes = bytesRead % cryptoAlignedSize;
            if (cryptoUnAlignedBytes) {
                /* read amount is not crypto aligned, so trim the read and move the file position back by that much */
                BDBG_MSG(("%s: bytesRead %d are not crypto size %d aligned, trim by %d bytes", BSTD_FUNCTION, bytesRead, cryptoAlignedSize, cryptoUnAlignedBytes));
                bytesRead -= cryptoUnAlignedBytes;
                if (filePlayHandle->file.data->seek(filePlayHandle->file.data, -cryptoUnAlignedBytes, SEEK_CUR) < 0) {
                    BDBG_ERR(("%s: failed to shift back the current cryptoUnAlignedBytes by %d amount, errno %d", BSTD_FUNCTION, cryptoUnAlignedBytes, errno));
                    break;
                }
                else
                    BDBG_MSG(("%s: shift back the file position by  %d offsets, crypto aligned bytesRead %d", BSTD_FUNCTION, cryptoUnAlignedBytes, bytesRead));
                if (bytesRead == 0) {
                    BKNI_Sleep(20);
                    continue;
                }
            }
            /* decrypt the buffer */
            if (B_PlaybackIp_UtilsPvrDecryptBuffer(&liveStreamingHandle->data, readBuf, clearBuf, bytesRead, &bytesToWrite) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: PVR Decryption Failed", BSTD_FUNCTION));
                break;
            }
            readBuf = clearBuf;
            BDBG_MSG(("%s: decrypting of %d bytes buffer done", BSTD_FUNCTION, bytesRead));
        }
#endif
        bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&liveStreamingHandle->data, readBuf, bytesToWrite);
        if (bytesWritten <= 0) {
            /* this happens if client closed the connection or client connection is dead */
            BDBG_MSG(("%s: failed to write %d bytes for streaming fd %d, wrote %d bytes, errno %d", BSTD_FUNCTION, bytesToWrite, streamingFd, bytesWritten, errno));
            liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
            break;
        }
        BDBG_MSG(("%s: wrote %d bytes for streaming fd %d, bytesToRead %d, bytesRead %d, sync %x",
                    BSTD_FUNCTION, bytesWritten, streamingFd, bytesToRead, bytesRead, *(readBuf)));
        liveStreamingHandle->totalBytesStreamed += bytesWritten;
        loopCount++;
        readBuf = liveStreamingHandle->streamingBuf;
    }

error:
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (liveStreamingHandle->clearBuf)
        NEXUS_Memory_Free(liveStreamingHandle->clearBuf);
#endif
    if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
        /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
        BDBG_WRN(("%s: invoking End of Streaming callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
        liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, B_PlaybackIpEvent_eServerEndofStreamReached);
    }
    BDBG_WRN(("%s: Done: handle %p, streamed %"PRId64 " bytes for streaming fd %d in %d send calls", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->totalBytesStreamed, streamingFd, loopCount));
    if (liveStreamingHandle->streamingBufOrig)
        NEXUS_Memory_Free(liveStreamingHandle->streamingBufOrig);
    if (liveStreamingHandle->stopStreamingEvent)
        BKNI_SetEvent(liveStreamingHandle->stopStreamingEvent);
    liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
    liveStreamingHandle->threadRunning = false;
    return;
}

/* Non timehisft case */
static void
liveStreamingThreadFromRaveBuffer(
    void *data
    )
{
    int rc;
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle = (B_PlaybackIpLiveStreamingHandle)data;
    B_PlaybackIpLiveStreamingOpenSettings *liveStreamingSettings;
    void *readBuf = NULL;
    size_t clearBufferSize;
    size_t * indexBuf = NULL;
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    unsigned char *clearBuf;
#endif
    size_t bytesRead = 0, bytesToRead = 0, indexBytesRead, totalBytesRead;
    int streamingFd;
    int bytesWritten = 0, bytesToWrite = 0;
    unsigned int loopCount = 0;
    int readTimeouts = 0;
    bool firstTime = true;
    NEXUS_RecpumpStatus status;
    unsigned long long bytesRecordedTillPreviousGop = 0;
    uint64_t bytesRecordedTillCurrentGop = 0;
    int gopsSentInHlsSegment = 0;
    FILE *fclear = NULL;
    int fileNameSuffix = 0;
    char recordFileName[32];
    bool gotErrorInStreamingLoop = false;
    B_PlaybackIpEventIds eventId = B_PlaybackIpEvent_eServerEndofStreamReached;
#if 0
#define ENABLE_SW_PACING
#endif
#ifdef ENABLE_SW_PACING
    double rate;
    struct timeval start, stop;
    double dt=0, maxrate;
#endif
    bool streamingNull=false; /* Used in SATIP to track when NullPackets streaming out*/

    BDBG_ASSERT(liveStreamingHandle);
    loopCount = 0;
    liveStreamingSettings = &liveStreamingHandle->settings;
    streamingFd = liveStreamingSettings->streamingFd;
    liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eActive;
    fileNameSuffix = liveStreamingHandle->fileNameSuffix;

    if (enableRecording) {
        memset(recordFileName, 0, sizeof(recordFileName));
        snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/rec%d_0.ts", fileNameSuffix);
        liveStreamingHandle->fclear = fclear = fopen(recordFileName, "w+b");
    }
    /* setup bytes to read: account for transport timestamp & aes encryption */
    /* Increase clearBuffer size to 60Kbytes based on latest http/udp optimization analysis.*/
    if (liveStreamingSettings->transportTimestampEnabled)
        clearBufferSize = (TS_PKT_SIZE+4) * HTTP_AES_BLOCK_SIZE * 5 * 4 ;
    else
        clearBufferSize = TS_PKT_SIZE * HTTP_AES_BLOCK_SIZE * 5 * 4;

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (liveStreamingHandle->data.pvrDecKeyHandle) {
        NEXUS_MemoryAllocationSettings allocSettings;
        NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
        if (liveStreamingSettings->heapHandle)
            allocSettings.heap = liveStreamingSettings->heapHandle;
        if (NEXUS_Memory_Allocate(clearBufferSize, &allocSettings, (void *)(&clearBuf))) {
            BDBG_ERR(("%s: memory allocation failure at %d\n", BSTD_FUNCTION, __LINE__));
            goto error;
        }
        liveStreamingHandle->clearBuf = clearBuf;
    }
#endif

    /* let the main thread know that we have started */
    liveStreamingHandle->threadRunning = true;
    liveStreamingHandle->stop = false;
    if (liveStreamingHandle->startStreamingEvent)
        BKNI_SetEvent(liveStreamingHandle->startStreamingEvent);
#ifdef BDBG_DEBUG_BUILD
    if (liveStreamingHandle->ipVerboseLog)
    BDBG_ERR(("%s: Ready for streaming: handle %p, streamingFd %d: clearBufferSize %zu", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, clearBufferSize));
#endif

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (liveStreamingSettings->appHeader.valid) {
        BDBG_MSG(("%s: appHeader Enabled %d, length %d", BSTD_FUNCTION, liveStreamingSettings->appHeader.valid, liveStreamingSettings->appHeader.length));
        BDBG_MSG(("appHeader data: %x data: %x data: %x data: %x ", liveStreamingSettings->appHeader.data[0], liveStreamingSettings->appHeader.data[1], liveStreamingSettings->appHeader.data[2], liveStreamingSettings->appHeader.data[3]));
        bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&liveStreamingHandle->data, liveStreamingSettings->appHeader.data, liveStreamingSettings->appHeader.length);
        if (bytesWritten != (int)liveStreamingSettings->appHeader.length) {
            /* this happens if client closed the connection or client connection is dead */
            BDBG_WRN(("%s: handle: %p, failed to write %d bytes of app header data of len %d for fd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, bytesWritten, liveStreamingSettings->appHeader.length, streamingFd));
            goto error;
        }
        BDBG_MSG(("%s: handle: %p, wrote %d bytes of app header data of len %d for fd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, bytesWritten, liveStreamingSettings->appHeader.length, streamingFd));
    }
#endif

#ifdef ENABLE_SW_PACING
    gettimeofday(&start, NULL);
    maxrate = 15.;
#endif

    while (!liveStreamingHandle->stop) {
        if(liveStreamingHandle->settings.dataReadyTimeoutInterval == 0)
        {
            liveStreamingHandle->settings.dataReadyTimeoutInterval = LIVE_STREAMING_DATA_READY_TIMEOUT_INTERVAL;
        }

        /* New timeOutInterval is present then overwrite the legacy one with the latest. Right now this is only supported for live cases.
           Later we can think or having only one of them.*/
        if(liveStreamingHandle->settings.timeOutIntervalInMs) {
            liveStreamingHandle->settings.dataReadyTimeoutInterval = liveStreamingHandle->settings.timeOutIntervalInMs;
        }


        if(liveStreamingHandle->settings.streamingMethod == B_PlaybackIpStreamingMethod_eSystemTimerBased)
        {
            usleep(liveStreamingHandle->settings.dataReadyTimeoutInterval*1000);
            rc = BERR_TIMEOUT;
        }
        else /* B_PlaybackIpStreamingMethod_eRaveInterruptBased */
        {
            rc = BKNI_WaitForEvent(liveStreamingHandle->dataReadyEvent, liveStreamingHandle->settings.dataReadyTimeoutInterval);
        }


        if (liveStreamingHandle->stop) {
            BDBG_MSG(("%s: app asked us to stop streaming (handle %p)", BSTD_FUNCTION, (void *)liveStreamingHandle));
            break;
        }

        if (rc != 0 && rc != BERR_TIMEOUT) {
            /* error case, we are done */
            BDBG_WRN(("%s: dataReadyEvent ERROR (%d), breaking out of streaming loop ", BSTD_FUNCTION, rc));
            break;
        }
        BKNI_ResetEvent(liveStreamingHandle->dataReadyEvent);

        /* check for read timeouts */
        if (readTimeouts * LIVE_STREAMING_DATA_READY_TIMEOUT_INTERVAL > LIVE_STREAMING_DATA_READY_MAX_TIMEOUT) {
            BDBG_ERR(("%s: handle %p, too many timeouts (cnt %d, time %d) while waiting for live data from recpump for streamingFd %d, breaking out of streaming loop",
                        BSTD_FUNCTION, (void *)liveStreamingHandle, readTimeouts, LIVE_STREAMING_DATA_READY_MAX_TIMEOUT, streamingFd));
            break;
        }
        /* determine how many bytes to read from the rave buffer */
        if(liveStreamingHandle->settings.sendNullRtpPktsOnTimeout && streamingNull == true)
        {
            BDBG_WRN(("%s: Stop Send Null RtspPacket  data found in Rave.", BSTD_FUNCTION ));
            streamingNull = false;
        }

        /* for HLS sessions, we read GOP at a time */
        if (liveStreamingSettings->hlsSession) {
            /* either we had a timeout or we actually got the data ready event: try reading */
            if (NEXUS_Recpump_GetIndexBuffer(liveStreamingHandle->recpumpHandle, (const void **)&indexBuf, (size_t *)&indexBytesRead) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_GetIndexBuffer failed, breaking out of streaming loop", BSTD_FUNCTION));
                break;
            }
            if (indexBuf && indexBytesRead) {
                unsigned long long highByte;
                /* we got some data, reset the readTimeout counter */
                readTimeouts = 0;
                if (firstTime) {
                    if (indexBytesRead <= BRCM_TPIT_ENTRY_SIZE) {
                        /* wait until first two full GOPs are available in rave */
                        BDBG_MSG(("%s: waiting until first full GOP is received, indexBytesRead %zu", BSTD_FUNCTION, indexBytesRead));
                        continue;
                    }
                    else {
                        /* got atleast 1st 2 GOPs, so get the bytesRecordedTill last gop */
                        BDBG_ASSERT(indexBytesRead%BRCM_TPIT_ENTRY_SIZE==0);
                        BDBG_MSG(("%s: received 1st GOP (indexBytesRead %zu), last RAP entry %p", BSTD_FUNCTION, indexBytesRead, (void *)(indexBuf + (indexBytesRead - BRCM_TPIT_ENTRY_SIZE))));
                        indexBuf = (size_t *) ((size_t)indexBuf + (indexBytesRead - BRCM_TPIT_ENTRY_SIZE));
                    }
                }
                /* accumulated upto 2nd Random Access Indicator, i.e. one full GOP */
                firstTime = false;
                /* calculate the GOP size, so that only that many bytes are read from rave and streamed to the client */
                highByte = ((unsigned long long)*(indexBuf+2) >> 24);
                bytesRecordedTillCurrentGop = highByte << 32;
                BDBG_MSG(("bytesRecordedTillCurrentGop %"PRIu64 , bytesRecordedTillCurrentGop));
                bytesRecordedTillCurrentGop |= (unsigned long long)*(indexBuf+3);
                BDBG_MSG(("final bytesRecordedTillCurrentGop %"PRIu64, bytesRecordedTillCurrentGop));
                bytesToRead = bytesRecordedTillCurrentGop  - bytesRecordedTillPreviousGop;
                bytesRecordedTillPreviousGop = bytesRecordedTillCurrentGop;
                BDBG_MSG(("%s: CTX %p: Got full GOP: index bytesRead %zu, bytesToRead %zu, streaming fd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, indexBytesRead, bytesToRead, streamingFd));

#ifdef DEBUG
                BDBG_MSG(("%s: GOT full GOP: index bytesRead %zu, tipt[0] 0x%x, tpit[2] 0x%x, tpit[3] 0x%x, bytesToRead %zu", BSTD_FUNCTION, indexBytesRead, *indexBuf, *(indexBuf+2), *(indexBuf+3), bytesToRead));
                NEXUS_Recpump_GetStatus(liveStreamingHandle->recpumpHandle, &status);
                BDBG_WRN(("%s: streamingFd %d: Recpump status: FIFO: depth %d, size %d", BSTD_FUNCTION, streamingFd, status.data.fifoDepth, status.data.fifoSize));
#endif
            }
            else {
                readTimeouts++;
                BDBG_MSG(("%s: handle %p, for streamingFd %d, readTimeouts %d, wait until 1 full GOP is received", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, readTimeouts));
                continue;
            }
        } else {
            /* non-HLS session: try to read upto one clearBufferSize */
            bytesToRead = clearBufferSize;
        }

        totalBytesRead = 0;
        while (totalBytesRead < bytesToRead) {
            if (liveStreamingHandle->stop) {
                BDBG_MSG(("%s: app asked us to stop streaming (handle %p)", BSTD_FUNCTION, (void *)liveStreamingHandle));
                gotErrorInStreamingLoop = true;
                break;
            }

            /* read the remaining bytes in this GOP and stream them out */
            rc = NEXUS_Recpump_GetDataBuffer(liveStreamingHandle->recpumpHandle, (const void **)&readBuf, (size_t *)&bytesRead);

           /* Send Null Packets for SES SATIP spec when no signal is being received,
                       the signal is being lost, or no PID is available (also e.g. when pids=none) */
            if (rc != 0  && liveStreamingHandle->settings.sendNullRtpPktsOnTimeout) {
                if(streamingNull == false)
                {
                    BDBG_WRN(("%s: Send Null RtspPacket(timeout Interval %d).No data in Rave.\n Most likely signal lost(sat coax unplugged) or invalid pids or zero pids specified",
                          BSTD_FUNCTION, liveStreamingHandle->settings.dataReadyTimeoutInterval ));
                    streamingNull = true;
                }
                B_PlaybackIp_UtilsSendNullRtpPacket((struct bfile_io_write_net *)&liveStreamingHandle->data);
                break;
            }
            else if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_GetDataBuffer failed, breaking out of streaming loop rc %d", BSTD_FUNCTION,rc));
                gotErrorInStreamingLoop = true;
                break;
            }

            if (readBuf == NULL || bytesRead <= 0) {
                readTimeouts++;
                /* break out of this inner loop, but check after this loop will again try to read more data */
                break;
            }
            readTimeouts = 0;

            if (bytesRead > (bytesToRead-totalBytesRead))
                /* read more than the GOP size, so trim it */
                bytesRead = bytesToRead-totalBytesRead;
            if (bytesRead > clearBufferSize)
                /* read more than the buffer size, so trim it (as we try to write only a small size buffer to keep low latency) */
                bytesRead = clearBufferSize;
#if 0
            /* check buffer for missing sync bytes */
            {
                size_t idx;
                for (idx=0; idx<bytesRead; idx+=188) {
                    if (readBuf[idx] != 0x47) {
                        BDBG_ERR(("%s: sync error ... bytesToRead %d, bytesRead %d, totalBytesRead %d", BSTD_FUNCTION, bytesToRead, bytesRead, totalBytesRead));
                        B_PlaybackIp_UtilsDumpBuffer ( BSTD_FUNCTION, &readBuf[idx], 48 );
                    }
                }
            }
#endif

            /* if NOT hlsSession */
            if (!liveStreamingSettings->hlsSession) {
                /* only send AES block size aligned bytes, its is fine for continous live streaming */
                if (liveStreamingSettings->protocol == B_PlaybackIpProtocol_eHttp) {
                    bytesRead -= bytesRead%HTTP_AES_BLOCK_SIZE;
                }
#if 0
                /* let it send whatever amount of data we get for now and not worry about any size alignment */
                else {
                    bytesRead -= bytesRead%1316;
                }
#endif

                bytesToRead = bytesRead;
                BDBG_MSG(("%s: bytesToRead %zu, adjusted bytesRead %zu, totalBytesRead %zu", BSTD_FUNCTION, bytesToRead, bytesRead, totalBytesRead));
                if (bytesToRead <= 0) {
                    BDBG_MSG(("%s: recpump underflow (bytesRead %zu), sleeping for 100msec...", BSTD_FUNCTION, bytesRead));
                    continue;
                }
            }
            /* NOTE: else for HLS if security is enabled, need to handle the case where data read is not AES block aligned */
            /* the issue is that last few bytes of a GOP may not be AES block size aligned and thus we will need to insert padding */

            /* send bytes just read */
            bytesToWrite = bytesRead;

            BDBG_MSG(("%s:%p: bytesToWrite (%lu); mod 188 (%lu); thread (%lx)", BSTD_FUNCTION, (void *)liveStreamingHandle->recpumpHandle, (long unsigned int)bytesToWrite,(long unsigned int)bytesToWrite%188, pthread_self() ));

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
            if (liveStreamingHandle->data.pvrDecKeyHandle) {
                /* decrypt the buffer first */
                if (B_PlaybackIp_UtilsPvrDecryptBuffer((struct bfile_io_write_net *)&data, readBuf, clearBuf, bytesRead, &bytesToWrite) != B_ERROR_SUCCESS) {
                    BDBG_ERR(("%s: handle %p, PVR Decryption Failed", BSTD_FUNCTION, (void *)liveStreamingHandle));
                    gotErrorInStreamingLoop = true;
                    break;
                }
                readBuf = clearBuf;
                BDBG_MSG(("%s: decrypting of %zu bytes buffer done, bytesToWrite %d", BSTD_FUNCTION, bytesRead, bytesToWrite));
            }
#endif
            bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&liveStreamingHandle->data, readBuf, bytesToWrite);
            if (bytesWritten <= 0) {
                /* this happens if client closed the connection or client connection is dead */
                if ( liveStreamingHandle->ipVerboseLog )
                    BDBG_WRN(("%s: failed to write %d bytes for handle %p, streaming fd %d, wrote %d bytes, errno %d, streamed %"PRId64 " bytes in %d calls", BSTD_FUNCTION, bytesToWrite, (void *)liveStreamingHandle, streamingFd, bytesWritten, errno, liveStreamingHandle->totalBytesStreamed, loopCount));
                gotErrorInStreamingLoop = true;
                liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
                if (NEXUS_Recpump_DataReadComplete(liveStreamingHandle->recpumpHandle, 0) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: NEXUS_Recpump_DataReadComplete failed, breaking out of streaming loop", BSTD_FUNCTION));
                    break;
                }
                break;
            }
            /* write data to file */
            if (enableRecording && fclear) {
                fwrite(readBuf, 1, bytesToWrite, fclear);
            }

            /* tell rave that we have consumed bytesWritten amount of data */
            if (NEXUS_Recpump_DataReadComplete(liveStreamingHandle->recpumpHandle, bytesWritten) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_DataReadComplete failed, breaking out of streaming loop", BSTD_FUNCTION));
                gotErrorInStreamingLoop = true;
                break;
            }
            totalBytesRead += bytesRead;
            liveStreamingHandle->totalBytesStreamed += bytesWritten;
            loopCount++;
        }

        if (readTimeouts) {
            /* didn't get any data, retry */
            BDBG_MSG(("%s: handle %p, for streamingFd %d, readTimeouts %d, wait until more data is received", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, readTimeouts));
            continue;
        }
        if (gotErrorInStreamingLoop == true) {
            BDBG_MSG(("%s: gotErrorInStreamingLoop: breaking out of streaming loop", BSTD_FUNCTION));
            break;
        }

        BDBG_MSG(("%s: bytesToRead %zu bytes for streaming fd %d, wrote %zu bytes", BSTD_FUNCTION, bytesToRead, streamingFd, totalBytesRead));
        if (liveStreamingSettings->hlsSession) {
            gopsSentInHlsSegment++;
            if (NEXUS_Recpump_IndexReadComplete(liveStreamingHandle->recpumpHandle, indexBytesRead) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_IndexReadComplete failed, breaking out of streaming loop", BSTD_FUNCTION));
                break;
            }
            BDBG_MSG(("%s: gopsSentInHlsSegment %d", BSTD_FUNCTION, gopsSentInHlsSegment));
            if (gopsSentInHlsSegment % GOPS_IN_A_SEGMENT == 0/*liveStreamingSettings->hlsSegmentSize*/) {
                /* 3 GOPs in a segment */
                BDBG_MSG(("%s: finished writing one segment, # of GOPs sent %d, closing current socket fd %d and waiting for client to request new segment", BSTD_FUNCTION, gopsSentInHlsSegment, streamingFd));
                /* we have to close the socket here as that is the only way client knows to request the next segment */
                shutdown(streamingFd, 2);
                close(streamingFd);
                if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
                    /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
                    BDBG_MSG(("%s: invoking End of segment callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
                    liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, B_PlaybackIpEvent_eServerEndofSegmentReached);
                }
                if (fclear) {
                    fflush(fclear);
                    fclose(fclear);
                    snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/rec%d_%d.ts", fileNameSuffix, gopsSentInHlsSegment/GOPS_IN_A_SEGMENT);
                    liveStreamingHandle->fclear = fclear = fopen(recordFileName, "w+b");
                }
                /* wait for app to resume steaming, otherwise timeout and close */
                BKNI_ResetEvent(liveStreamingHandle->resumeStreamingEvent);
                rc = BKNI_WaitForEvent(liveStreamingHandle->resumeStreamingEvent, HLS_RESUME_STREAMING_TIMEOUT);
                if (rc == BERR_TIMEOUT || rc != 0) {
                    BDBG_WRN(("%s: resume streaming event timedout or error on event, handle %p, client is done w/ HLS session", BSTD_FUNCTION, (void *)liveStreamingHandle));
                    eventId = B_PlaybackIpEvent_eServerStartStreamingTimedout;
                    break;
                }
                streamingFd = liveStreamingHandle->data.fd; /* updated by the SetSettings function */
                BDBG_MSG(("%s: CTX %p: resume streaming on fd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd));
            }
        }

        /* look for recpump overflow before returning to top of the loop */
        NEXUS_Recpump_GetStatus(liveStreamingHandle->recpumpHandle, &status);
        if (loopCount % 50 == 0) {
            BDBG_MSG(("%s: streamingFd %d: Recpump status: FIFO: depth %zu, size %zu, read %zu, written %d, totolBytesStreamd %lu", BSTD_FUNCTION, streamingFd, status.data.fifoDepth, status.data.fifoSize, bytesRead, bytesWritten, (long unsigned int)liveStreamingHandle->totalBytesStreamed));
        }
        if (status.data.fifoDepth >= .95 * status.data.fifoSize) {
            if (NEXUS_Recpump_GetDataBuffer(liveStreamingHandle->recpumpHandle, (const void **)&readBuf, (size_t *)&bytesRead) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_GetDataBuffer failed, breaking out of streaming loop", BSTD_FUNCTION));
                break;
            }
            BDBG_WRN(("%s: streamingFd %d: Recpump overflow, flushing the FIFO: depth %zu, size %zu, read %zu", BSTD_FUNCTION, streamingFd, status.data.fifoDepth, status.data.fifoSize, bytesRead));
            if (NEXUS_Recpump_DataReadComplete(liveStreamingHandle->recpumpHandle, bytesRead) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_DataReadComplete failed, breaking out of streaming loop", BSTD_FUNCTION));
                break;
            }
            BKNI_Sleep(50); /* wait a bit for new data to accumulate in recpump FIFO */
        }
#ifdef ENABLE_SW_PACING
        gettimeofday(&stop, NULL);
        dt = difftime1(&start, &stop);
        rate = 8.*liveStreamingHandle->totalBytesStreamed/dt;
        if ((loopCount % 50) == 0) {
            BDBG_MSG(("[%6lu] Wrote %10lu Bytes in dt = %12.2fusec at rate=%2.1f/%2.1f\n", loopCount, liveStreamingHandle->totalBytesStreamed, dt, rate, maxrate));
        }
        while (rate > maxrate) {
            usleep(10000);
            gettimeofday(&stop, NULL);
            dt = difftime1(&start,&stop);
            rate = 8.*liveStreamingHandle->totalBytesStreamed/dt;
        }
#endif
    }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
error:
    if (liveStreamingHandle->clearBuf) NEXUS_Memory_Free(liveStreamingHandle->clearBuf);
#endif
    if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
        /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
        BDBG_MSG(("%s: invoking End of Streaming callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
        liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, eventId);
    }
    if (enableRecording && fclear) {
        fflush(fclear);
        fclose(fclear);
    }
    BDBG_MSG(("%s: Done: handle %p, streamed %"PRId64 " bytes for streaming fd %d in %d send calls ", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->totalBytesStreamed, streamingFd, loopCount));
    if (liveStreamingHandle->stopStreamingEvent)
        BKNI_SetEvent(liveStreamingHandle->stopStreamingEvent);
    liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
    liveStreamingHandle->threadRunning = false;
    return;
}

#if 1
#define HLS_LOW_LATENCY_STREAMING
#endif
#ifdef HLS_LOW_LATENCY_STREAMING
int sendPatPmt(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle
    )
{
    int bytesWritten = 0;
    char recordFileName[32];

    /* we only manually send PAT/PMT for HLS sessions and not use Stream Mux for inserting System Data for HLS. */
    /* This is because stream mux can't ensure that PAT/PMT will be inserted before 1st frame. HLS requires each */
    /* new segment to begin with PAT/PMT. */

    /* send Pat */
    bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&liveStreamingHandle->data, liveStreamingHandle->patBuffer, HLS_PAT_PMT_BUFFER_SIZE);
    if (bytesWritten != HLS_PAT_PMT_BUFFER_SIZE) {
        BDBG_ERR(("%s: CTX %p: Failed to write PAT on socket %u", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->data.fd));
        return -1;
    }
    BDBG_MSG(("%s: ctx:streamingFd %p:%d: sent PAT", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->data.fd));

    /* send Pmt */
    bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&liveStreamingHandle->data, liveStreamingHandle->pmtBuffer, HLS_PAT_PMT_BUFFER_SIZE);
    if (bytesWritten != HLS_PAT_PMT_BUFFER_SIZE) {
        BDBG_ERR(("%s: CTX %p: Failed to write PMT on socket %u", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->data.fd));
        return -1;
    }
    BDBG_MSG(("%s: ctx:streamingFd %p:%d: sent PMT", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->data.fd));

    /* optionally: if local recording is enabled for debug purposes, we also write the PAT & PMT to that file */
    if (enableRecording) {
        if (liveStreamingHandle->fclear == NULL) {
            memset(recordFileName, 0, sizeof(recordFileName));
            snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/rec%d_%d.ts", liveStreamingHandle->fileNameSuffix, liveStreamingHandle->totalHlsSegmentsSent);
            BDBG_MSG(("%s: Opening recording %s", BSTD_FUNCTION, recordFileName));
            liveStreamingHandle->fclear = fopen(recordFileName, "w+b");
            if (liveStreamingHandle->fclear == NULL) {
                BDBG_ERR(("%s: CTX %p: local recording of HLS Segments is enabled, but fopen failed for file name %s, errno %d", BSTD_FUNCTION, (void *)liveStreamingHandle, recordFileName, errno));
                return 0; /* we ignore the recording failure of the PAT/PMT as this is just for debug purposes */
            }
        }
        /* save PAT */
        fwrite(liveStreamingHandle->patBuffer, 1, HLS_PAT_PMT_BUFFER_SIZE, liveStreamingHandle->fclear);
        /* save PMT */
        fwrite(liveStreamingHandle->pmtBuffer, 1, HLS_PAT_PMT_BUFFER_SIZE, liveStreamingHandle->fclear);
        BDBG_MSG(("%s: recorded %d bytes of PAT & %d bytes of PMT", BSTD_FUNCTION, HLS_PAT_PMT_BUFFER_SIZE, HLS_PAT_PMT_BUFFER_SIZE));
    }
    return 0;
}

static int waitUntilAvDataAvailableOrTimeout(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle,
    const void **readBuf,
    size_t *bytesRead
    )
{
    unsigned i;
    int readTimeouts = 0;
    B_Error rc = B_ERROR_UNKNOWN;
    unsigned numTriggeredEvents = 0, waitingToResumeStreaming = 0;
    B_EventHandle triggeredEvents[MAX_TRACKED_EVENTS];

    *readBuf = NULL;
    *bytesRead = 0;
    while (true)
    {
        rc = B_ERROR_UNKNOWN;
        if (liveStreamingHandle->stop) {
            BDBG_WRN(("%s: ctx:fd %p:%d: app asked us to stop streaming", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->settings.streamingFd));
            break;
        }

        /* we wait for either recpump to send us an event when data upto our threshold is available, resumeStreamingEvent from app (via SetSettings) or timeout */
        numTriggeredEvents = 0;
        B_EventGroup_Wait( liveStreamingHandle->eventGroup, LIVE_STREAMING_DATA_READY_TIMEOUT_INTERVAL, triggeredEvents, liveStreamingHandle->maxTriggeredEvents, &numTriggeredEvents );

        /* check again for stop streaming event */
        if (liveStreamingHandle->stop) {
            BDBG_WRN(("%s: ctx:fd %p:%d: app asked us to stop streaming", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->settings.streamingFd));
            break;
        }
        if (liveStreamingHandle->abortStreaming) {
            liveStreamingHandle->abortStreaming = false;
            BDBG_WRN(("%s: ctx:fd %p:%d: app asked us to abort streaming", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->settings.streamingFd));
            break;
        }
        for (i = 0; i < numTriggeredEvents; i++)
        {
            if (triggeredEvents[i] == liveStreamingHandle->resumeStreamingEvent) {
                liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eStreaming;
                liveStreamingHandle->settings.streamingFd = liveStreamingHandle->data.fd; /* updated by the SetSettings function */
                BDBG_MSG(("%s: CTX %p: resume streaming on fd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->settings.streamingFd));
                BKNI_ResetEvent(liveStreamingHandle->resumeStreamingEvent);
                continue; /* to check for the next event */
            }
            if (triggeredEvents[i] == liveStreamingHandle->dataReadyEvent) {
                /* HLS streaming requires low latency. That means we dont wait for rave threshold amount of data to show up in the recpump. */
                /* Instead, we will stream what ever is available in the recpump either via the event or via the timeout */
                BKNI_ResetEvent(liveStreamingHandle->dataReadyEvent);
                continue; /* to check for the next event */
            }
        }
        if (liveStreamingHandle->state == B_PlaybackIpLiveStreamingState_eWaitingToResumeStreaming) {
            /* if we are not in the streaming state, we continue waiting for app to tell us to resume streaming */
            waitingToResumeStreaming++;
            BDBG_MSG(("%s: ctx:fd %p:%d: waiting for app to resume streaming (waitingToResumeStreaming %d)", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->settings.streamingFd, waitingToResumeStreaming));
            continue;
        }

        BDBG_ASSERT(liveStreamingHandle->state == B_PlaybackIpLiveStreamingState_eStreaming);
        /* we are in the streaming state: check how many av bytes are available for streaming  */
        if (NEXUS_Recpump_GetDataBuffer(liveStreamingHandle->recpumpHandle, readBuf, bytesRead) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Recpump_GetDataBuffer failed, breaking out!", BSTD_FUNCTION));
            break;
        }

        if (*readBuf == NULL || *bytesRead <= 0)
        {
            readTimeouts++;
            /* nothing available at this time, keep trying until we exceed the max timeout */
            BDBG_MSG(("%s: ctx:fd %p:%d: no data available yet: readTimeouts %d, readBuf %p, bytesRead %zu", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->settings.streamingFd, readTimeouts, *readBuf, *bytesRead));
            /* check if we have gotten enough readTimeout events to give up on this streaming session */
            if (readTimeouts * LIVE_STREAMING_DATA_READY_TIMEOUT_INTERVAL > LIVE_STREAMING_DATA_READY_MAX_TIMEOUT) {
                BDBG_ERR(("%s: handle %p, too many timeouts (cnt %d, timeout %d) while waiting for live data for streamingFd %d, breaking out",
                            BSTD_FUNCTION, (void *)liveStreamingHandle, readTimeouts, LIVE_STREAMING_DATA_READY_MAX_TIMEOUT, liveStreamingHandle->settings.streamingFd));
                break;
            }
        }
        else {
            BDBG_MSG(("%s: readBuf %p, bytesRead %zu, readTimeouts %d, waitingToResumeStreaming %d", BSTD_FUNCTION, *readBuf, *bytesRead, readTimeouts, waitingToResumeStreaming));
            rc = B_ERROR_SUCCESS;
            break;
        }
    }
    return rc;
}

/* HLS Streaming Thread */
static void
hlsStreamingThreadFromRaveBuffer(
    void *data
    )
{
    B_Error rc;
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle = (B_PlaybackIpLiveStreamingHandle)data;
    B_PlaybackIpLiveStreamingOpenSettings *liveStreamingSettings;
    const void *readBuf = NULL;
    const size_t * indexBuf = NULL;
    size_t bytesRead = 0, indexBytesRead, bytesSentInCurrentGop = 0, bytesToWrite = 0, totalBytesWritten = 0;
    int streamingFd;
    int bytesWritten = 0;
    unsigned int loopCount = 0;
    bool firstRai = true;
    unsigned long long bytesRecordedTillPreviousGop = 0;
    unsigned long long bytesRecordedTillCurrentGop = 0;
    unsigned gopsSentInHlsSegment = 0;
    bool gotErrorInStreamingLoop = false;
    B_PlaybackIpEventIds eventId = B_PlaybackIpEvent_eServerEndofStreamReached;
    int gopsReadSoFar = 0;
    unsigned maxGopsInHlsSegment = GOPS_IN_A_SEGMENT;

    BDBG_ASSERT(liveStreamingHandle);
    liveStreamingSettings = &liveStreamingHandle->settings;
    liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eActive;

    /* let the main thread know that we have started */
    liveStreamingHandle->threadRunning = true;
    liveStreamingHandle->stop = false;
    liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eStreaming;
    if (liveStreamingHandle->startStreamingEvent)
        BKNI_SetEvent(liveStreamingHandle->startStreamingEvent);
    liveStreamingHandle->totalBytesStreamed = 0;

    /* app may start the live streaming context but may have set the streamingEnabled flag */
    /* This may be done to prefill the xcode pipe before the actual 1st segment request comes in */
    /* we need to wait until app sets the streamingEnabled flag */
    while (!liveStreamingHandle->stop) {
        if (!liveStreamingHandle->settings.streamingDisabled) {
            BDBG_MSG(("%s: ctx: streaming is no longer distabled: streamingFd %p : %d", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingSettings->streamingFd));
            break;
        }
        rc = BKNI_WaitForEvent(liveStreamingHandle->resumeStreamingEvent, LIVE_STREAMING_DATA_READY_TIMEOUT_INTERVAL);
        if (rc != 0 && rc != BERR_TIMEOUT) {
            /* error case, we are done */
            BDBG_WRN(("%s: dataReadyEvent ERROR (%d), breaking out of initial loop ", BSTD_FUNCTION, rc));
            break;
        }
        else if (rc == BERR_TIMEOUT)
            continue;
        BKNI_ResetEvent(liveStreamingHandle->resumeStreamingEvent);
    }

#ifdef BDBG_DEBUG_BUILD
    if (liveStreamingHandle->ipVerboseLog)
    BDBG_WRN(("%s: Ready for streaming: handle %p, streamingFd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingSettings->streamingFd));
#endif
    while (!liveStreamingHandle->stop)
    {
        if (enableRecording) {
            char recordFileName[32];
            if (liveStreamingHandle->fclear == NULL) {
                memset(recordFileName, 0, sizeof(recordFileName));
                snprintf(recordFileName, sizeof(recordFileName)-1, "./videos/rec%d_%d.ts", liveStreamingHandle->fileNameSuffix, liveStreamingHandle->totalHlsSegmentsSent);
                BDBG_MSG(("%s: Opening recording %s", BSTD_FUNCTION, recordFileName));
                liveStreamingHandle->fclear = fopen(recordFileName, "w+b");
            }
        }
        /* wait for dataReadyEvent from the recpump to indicate that some data is available unless timeout happens, in which case we stream out what we have to keep the latency low */
        rc = waitUntilAvDataAvailableOrTimeout(liveStreamingHandle, &readBuf, &bytesRead);
        if (rc != B_ERROR_SUCCESS) {
            /* error case, we are done */
            BDBG_ERR(("%s: failed to read data from recpump, not breaking out of streaming loop, but continuing for app to stop the streaming ", BSTD_FUNCTION));
            /* we change the state here and go back to the top. waitUntilAvDataAvailableOrTimeout() waits for app to resume or stop streaming */
            liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eWaitingToResumeStreaming;
            if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
                /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
                BDBG_MSG(("%s: invoking ErrorStreaming callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
                liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, B_PlaybackIpEvent_eServerErrorStreaming);
            }
            continue;
        }
        streamingFd = liveStreamingSettings->streamingFd;

        /* so we read bytesRead worth bytes, read corresponding index data */
        if (NEXUS_Recpump_GetIndexBuffer(liveStreamingHandle->recpumpHandle, (const void **)&indexBuf, &indexBytesRead) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Recpump_GetIndexBuffer failed, breaking out of streaming loop", BSTD_FUNCTION));
            break;
        }
        BDBG_MSG(("%s: TOP: indexBuf %p, indexBytesRead %zu, bytesRead %zu", BSTD_FUNCTION, (void *)indexBuf, indexBytesRead, bytesRead));

        if (liveStreamingHandle->resetStreaming)
        {
            firstRai = true;
            bytesRecordedTillPreviousGop = 0;
            bytesRecordedTillCurrentGop = 0;
            bytesSentInCurrentGop = 0;
            liveStreamingHandle->resetStreaming = false;
            BDBG_MSG(("%s: ctx %p: Reset Streaming params!", BSTD_FUNCTION, (void *)liveStreamingHandle));
        }
        if (liveStreamingHandle->sendPatPmt)
        {
            rc = sendPatPmt(liveStreamingHandle);
            liveStreamingHandle->sendPatPmt = false;
            if (rc != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: ctx %p: failed to send PAT/PMT, issue errorcallback to app", BSTD_FUNCTION, (void *)liveStreamingHandle));
                /* we change the state here and go back to the top. waitUntilAvDataAvailableOrTimeout() waits for app to resume or stop streaming */
                liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eWaitingToResumeStreaming;
                if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
                    /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
                    BDBG_MSG(("%s: invoking ErrorStreaming callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
                    liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, B_PlaybackIpEvent_eServerErrorStreaming);
                }
                continue;
            }
        }
        /* Check if we have one full GOP worth data or not: index has TPIT entries corresponding to each I-frame. Two entries indicates 1 full GOP */
        if ( (indexBytesRead < BRCM_TPIT_ENTRY_SIZE) ||  /* no RAI entry yet, this must be due to the initial audio frame in rave. */
             (firstRai && indexBytesRead < 2*BRCM_TPIT_ENTRY_SIZE) /* or we got 1st RAI entry (1st I-frame), but not he full GOP yet as RAI worth entries make one full GOP. */
           )
        {
            /* We either have TS packets containing audio frames or not a complete video GOP. instead of waiting for full GOP, we now go ahead & send this partial data */
            /* iOS7 player otherwise times out if it doesn't receive initial data in ~1sec (which can happen during the start of xcode session) */
            bytesToWrite = bytesRead;
            bytesSentInCurrentGop += bytesToWrite;
            PRINTMSG_SUMMARY(("%s: ctx:fd %p:%d Current GOP is partially available, just send it: indexBytesRead %zu, bytesRead %zu, bytesSentInCurrentGop %zu", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, indexBytesRead, bytesRead, bytesSentInCurrentGop));
            /* we set this to 0 as we dont want to consume the index bytes below until we either get a full GOP worth or 1 complete GOP for first time case */
            indexBytesRead = 0;
        }
        else
        {
            /* got atleast 1 complete GOP worth data is available in recpump, we need to calculate the GOP size and only send that many bytes */
            unsigned long long highByte;
            unsigned currentGopSize;
            /* only read one GOP at a time */
            gopsReadSoFar += 1;
            if (firstRai) {
                /* for us to be here, we would have two RAI entries and thus indexBytesRead >= 2*BRCM_TPIT_ENTRY_SIZE */
                /* we will use the 2nd entry for calculating the size of the 1st GOP */
                BDBG_MSG(("%s: GOT 1st full GOP", BSTD_FUNCTION));
                BDBG_ASSERT((indexBytesRead >= (2*BRCM_TPIT_ENTRY_SIZE)));
                indexBuf = (size_t *) ((size_t)indexBuf + BRCM_TPIT_ENTRY_SIZE); /* index needs to point to the 2nd RAI entry, then we can get the size of the very 1st GOP in belows logic */
#ifdef DEBUG
                BDBG_MSG(("%s: 2nd index bytesRead %zu, tipt[0] 0x%x, tpit[2] 0x%x, tpit[3] 0x%x", BSTD_FUNCTION, indexBytesRead, *indexBuf, *(indexBuf+2), *(indexBuf+3)));
#endif
                indexBytesRead = 2*BRCM_TPIT_ENTRY_SIZE;  /* consume both index entries as they together make up the 1st GOP */
            }
            else {
                /* we have already sent one full GOP, so now we should only send 1 GOP worth. So we only use one RAI entry from indexBuf */
                indexBytesRead = BRCM_TPIT_ENTRY_SIZE;  /* limit to sending only 1 index worth entry at a time */
            }

            /* calculate the GOP size of 1 or more full GOPs that are present in the recpump. We dont want to send more than the GOP[s] available */
            highByte = ((unsigned long long)*(indexBuf+2) >> 24);
            bytesRecordedTillCurrentGop = highByte << 32;
            bytesRecordedTillCurrentGop |= (unsigned long long)*(indexBuf+3);
#ifdef DEBUG
            BDBG_MSG(("%s: 2nd index bytesRead %zu, tipt[0] 0x%x, tpit[2] 0x%x, tpit[3] 0x%x", BSTD_FUNCTION, indexBytesRead, *indexBuf, *(indexBuf+2), *(indexBuf+3)));
            BDBG_MSG(("%s: highByte %llx, bytesRecordedTillCurrentGop %llu, bytesRecordedTillPreviousGop %llu", BSTD_FUNCTION, highByte, bytesRecordedTillCurrentGop, bytesRecordedTillPreviousGop));
#endif
            currentGopSize = bytesRecordedTillCurrentGop  - bytesRecordedTillPreviousGop; /* TPIT entry contains the cumulative i-frame offsets, so we need to find the relative GOP sizes */
            /* take out any bytes that we have sent until this GOP was fully available */
#if 0
            /* TODO: need to understand this */
            BDBG_ASSERT(currentGopSize >= bytesSentInCurrentGop); /* make sure that current GOP size (bytesToWrite) is > what we have sent so far in that GOP! */
#endif
            bytesToWrite = currentGopSize - bytesSentInCurrentGop;
            bytesSentInCurrentGop = 0;
            PRINTMSG_SUMMARY(("%s: ctx:fd %p:%d: Got full GOP (# %d): currentGopSize %d, sent %zu, remain bytesToWrite %zu, bytesRead %zu, indexBytesRead %zu", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd,
                        gopsReadSoFar, currentGopSize, bytesSentInCurrentGop, bytesToWrite, bytesRead, indexBytesRead));
            if (bytesToWrite > bytesRead) {
                BDBG_MSG(("%s: FIFO wrap: index bytesRead %zu, bytesRead %zu, bytesToWrite %zu, streaming fd %d", BSTD_FUNCTION, indexBytesRead, bytesRead, bytesToWrite, streamingFd));
            }
        }

        /* either we have a some partial GOP data or a full GOP worth data, we read bytesToWrite amount and stream them out */
        totalBytesWritten = 0;
        gotErrorInStreamingLoop = false;
        while (bytesToWrite && totalBytesWritten < bytesToWrite)
        {
            /* Since RAVE uses a FIFO design, when it reaches near the end of FIFO, it only returns remaining # of bytes to the end of FIFO*/
            /* and doesn't return the wrapped bytes in the same Recpump_GetDataBuffer call. App has to call the _GetDataBuffer */
            /* in a loop to receive all needed bytes as determined by bytesToWrite above. Hence this while loop to read all data & send it out. */

            rc = waitUntilAvDataAvailableOrTimeout(liveStreamingHandle, &readBuf, &bytesRead);
            if (rc != B_ERROR_SUCCESS) {
                /* error case, we are done */
                BDBG_ERR(("%s: failed to read data from recpump, breaking out of streaming loop ", BSTD_FUNCTION));
                gotErrorInStreamingLoop = true;
                break;
            }
            if (bytesRead > (bytesToWrite - totalBytesWritten))
                /* read more than the current required size, so trim it */
                bytesRead = bytesToWrite - totalBytesWritten;

            BDBG_MSG(("%s: ctx:fd %p:%d: bytesToWrite %lu, bytesRead %zu, totalBytesWritten %zu ", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, (long unsigned int)bytesToWrite, bytesRead, totalBytesWritten));
            /* send it to the network client */
            bytesWritten = B_PlaybackIp_UtilsStreamingCtxWriteAll((struct bfile_io_write *)&liveStreamingHandle->data, readBuf, bytesRead);
            if (bytesWritten <= 0 || bytesWritten != (int)bytesRead) {
                /* this happens if client closed the connection or client connection is dead */
                BDBG_WRN(("%s: failed to write %zu bytes for handle %p, streaming fd %d, wrote %d bytes, errno %d, streamed %"PRIu64 " bytes in %u calls", BSTD_FUNCTION, bytesToWrite, (void *)liveStreamingHandle, streamingFd, bytesWritten, errno, liveStreamingHandle->totalBytesStreamed, loopCount));
                gotErrorInStreamingLoop = true;
                if (NEXUS_Recpump_DataReadComplete(liveStreamingHandle->recpumpHandle, 0) != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: NEXUS_Recpump_DataReadComplete failed, breaking out of streaming loop", BSTD_FUNCTION));
                    break;
                }
                break;
            }
            /* debug: write data to file */
            if (enableRecording && liveStreamingHandle->fclear) {
                fwrite(readBuf, 1, bytesWritten, liveStreamingHandle->fclear);
            }
            /* tell rave that we have consumed bytesWritten amount of data */
            if (NEXUS_Recpump_DataReadComplete(liveStreamingHandle->recpumpHandle, bytesWritten) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_DataReadComplete failed, breaking out of streaming loop", BSTD_FUNCTION));
                gotErrorInStreamingLoop = true;
                break;
            }
            loopCount+=1;
            totalBytesWritten += bytesWritten;
            liveStreamingHandle->totalBytesStreamed += bytesWritten;
        }
        if (gotErrorInStreamingLoop == true) {
            if (bytesWritten <= 0 || bytesWritten != (int)bytesRead) {
                BDBG_WRN(("%s: ctx:fd %p:%d: error while trying to write streaming data, wait for app to resume or stop streaming", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd));
                /* we change the state here and go back to the top. waitUntilAvDataAvailableOrTimeout() waits for app to resume or stop streaming */
                liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eWaitingToResumeStreaming;
                if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
                    /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
                    BDBG_MSG(("%s: invoking ErrorStreaming callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
                    liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, B_PlaybackIpEvent_eServerErrorStreaming);
                }
                continue;
            }
            BDBG_MSG(("%s: gotErrorInStreamingLoop: breaking out of streaming loop", BSTD_FUNCTION));
            break;
        }
        PRINTMSG_SUMMARY(("%s: ctx:fd %p:%d: bytesToWrite %d, bytesWritten %d bytes, wrote %d bytes so far", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, (int)bytesToWrite, (int)bytesWritten, (int)liveStreamingHandle->totalBytesStreamed));
        BDBG_ASSERT(totalBytesWritten == bytesToWrite);

        if (indexBytesRead)
        {
            /* we have consumed full GOP worth of data, so move the index fifo */
            if (NEXUS_Recpump_IndexReadComplete(liveStreamingHandle->recpumpHandle, indexBytesRead) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: NEXUS_Recpump_IndexReadComplete failed, breaking out of streaming loop", BSTD_FUNCTION));
                break;
            }
            BDBG_MSG(("%s: consumed %zu index bytes", BSTD_FUNCTION, indexBytesRead));
            gopsSentInHlsSegment +=1; /* we are only sending 1 GOP per indexRead */
            firstRai = false;
            bytesRecordedTillPreviousGop = bytesRecordedTillCurrentGop; /* Also update the previousGopOffset for the next GOP size calculations (above) */
        }
        BDBG_MSG(("%s: gopsSentInHlsSegment %d", BSTD_FUNCTION, gopsSentInHlsSegment));
        if (gopsSentInHlsSegment >= maxGopsInHlsSegment)
        {
            if (!liveStreamingSettings->dontCloseSocket) {
                /* we have to close the socket here as that is the only way client knows to request the next segment */
                shutdown(streamingFd, 2);
#ifndef B_USE_HTTP_KEEPALIVE
                /* we keep the connection open in the keep-alive case! Either client closes or server will timeout & close the socket */
                close(streamingFd);
#endif
                BDBG_MSG(("%s: closed streamingFd %d", BSTD_FUNCTION, streamingFd));
            }
            if (liveStreamingHandle->fclear) {
                fflush(liveStreamingHandle->fclear);
                fclose(liveStreamingHandle->fclear);
                liveStreamingHandle->fclear = NULL;
            }
            liveStreamingHandle->totalHlsSegmentsSent +=1;
            PRINTMSG_SUMMARY(("%s: ctx:fd %p:%d: finished writing one segment: sent %u GOPs, total Segments sent %u, closing current socket and waiting for client to request new segment", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd, gopsSentInHlsSegment, liveStreamingHandle->totalHlsSegmentsSent));
            gopsSentInHlsSegment = 0;
            /* we change the state here and go back to the top. waitUntilAvDataAvailableOrTimeout() waits for app to resume streaming or stop streaming */
            liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eWaitingToResumeStreaming;
            if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
                /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
                BDBG_MSG(("%s: invoking End of segment callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
                liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, B_PlaybackIpEvent_eServerEndofSegmentReached);
            }

#if 0
            /* wait for app to resume steaming, otherwise timeout and close */
            BKNI_ResetEvent(liveStreamingHandle->startStreamingEvent);
            rc = BKNI_WaitForEvent(liveStreamingHandle->startStreamingEvent, HLS_RESUME_STREAMING_TIMEOUT);
            if (rc == BERR_TIMEOUT || rc != 0) {
                BDBG_ERR(("%s: resume streaming event timedout or error on event, handle %p, client is done w/ HLS session", BSTD_FUNCTION, (void *)liveStreamingHandle));
                eventId = B_PlaybackIpEvent_eServerStartStreamingTimedout;
                break;
            }
            streamingFd = liveStreamingHandle->data.fd; /* updated by the SetSettings function */
            BDBG_MSG(("%s: CTX %p: resume streaming on fd %d", BSTD_FUNCTION, (void *)liveStreamingHandle, streamingFd));
#endif
        }
    } /* while (!liveStreaming->stop) loop */

    liveStreamingHandle->totalHlsSegmentsSent++; /* increment this incase the streaming loop is re-started, we dont overwrite the previously recorded segments */
    if (liveStreamingHandle->fclear) {
        fflush(liveStreamingHandle->fclear);
        fclose(liveStreamingHandle->fclear);
        liveStreamingHandle->fclear = NULL;
    }
    if (liveStreamingSettings->eventCallback && !liveStreamingHandle->stop) {
        /* app has defined eventCallback function & hasn't yet issued the stop, invoke the callback */
        BDBG_MSG(("%s: invoking End of Streaming callback for ctx %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
        liveStreamingHandle->connectionState = B_PlaybackIpConnectionState_eError;
        liveStreamingSettings->eventCallback(liveStreamingSettings->appCtx, eventId);
    }
    BDBG_MSG(("%s: Done: handle %p, streamed %"PRId64 " bytes for streaming fd %d in %d send calls ", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->totalBytesStreamed, liveStreamingSettings->streamingFd, loopCount));
    if (liveStreamingHandle->stopStreamingEvent)
        BKNI_SetEvent(liveStreamingHandle->stopStreamingEvent);
    liveStreamingHandle->threadRunning = false;
    return;
}
#endif

static int
startLiveStreamingThread(
    B_ThreadFunc liveStreamingThreadFunc,
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle
    )
{
    BERR_Code rc;
    liveStreamingHandle->streamingThread = B_Thread_Create("Live Streamer", (B_ThreadFunc)liveStreamingThreadFunc, (void *)liveStreamingHandle, NULL);
    if (liveStreamingHandle->streamingThread == NULL) {
        BDBG_ERR(("%s: Failed to create HTTP media file streaming thread \n", BSTD_FUNCTION));
        goto error;
    }
    rc = BKNI_WaitForEvent(liveStreamingHandle->startStreamingEvent, 3000);
    if (rc == BERR_TIMEOUT) {
        BDBG_WRN(("%s: startStreamingEvent timed out", BSTD_FUNCTION));
        goto error;
    }
    else if (rc != 0) {
        BDBG_ERR(("%s: failed to start the streaming thread", BSTD_FUNCTION));
        goto error;
    }
    BKNI_ResetEvent(liveStreamingHandle->startStreamingEvent);
    BDBG_MSG(("%s: CTX %p: complete, streaming socket %d", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingHandle->data.fd));
    return 0;
error:
    return -1;
}

/***************************************************************************
Summary:
This function starts streaming content from a file.
***************************************************************************/

B_PlaybackIpError
B_PlaybackIp_LiveStreamingSetSettings(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle,
    B_PlaybackIpLiveStreamingSettings *settings
    )
{
    B_ThreadFunc liveStreamingThreadFunc;
    B_PlaybackIpLiveStreamingOpenSettings *liveStreamingSettings = &liveStreamingHandle->settings;

    BDBG_MSG(("%s: protocol (%s); socket current %u, updated %u", BSTD_FUNCTION,  (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eHttp)?"HTTP":
              (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eRtp)?"RTP":
              (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eUdp)?"UDP":
              (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eRtsp)?"RTSP":"UNKNOWN", liveStreamingSettings->streamingFd, settings->streamingFd ));
    switch (liveStreamingSettings->protocol) {
    case B_PlaybackIpProtocol_eHttp:
    {
        if (settings->resetStreaming)
            liveStreamingHandle->resetStreaming = true;
        if (settings->abortStreaming)
            liveStreamingHandle->abortStreaming = true;
        else
            liveStreamingHandle->abortStreaming = false;
        if (settings->streamingEnabled) {
            liveStreamingHandle->data.fd = settings->streamingFd;
            liveStreamingSettings->streamingFd = settings->streamingFd;
            liveStreamingSettings->streamingDisabled = false;
            liveStreamingHandle->data.self = net_io_data_write;
            liveStreamingHandle->data.streamingProtocol = B_PlaybackIpProtocol_eHttp;
            if (B_PlaybackIp_UtilsStreamingCtxOpen(&liveStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("Failed to setup the streaming context\n"));
                return B_ERROR_UNKNOWN;
            }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
            if (B_PlaybackIp_UtilsPvrDecryptionCtxOpen(&liveStreamingSettings->securitySettings, &liveStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to setup pvr decryption", BSTD_FUNCTION));
                return B_ERROR_UNKNOWN;
            }
#endif
            if (liveStreamingSettings->hlsSession && liveStreamingSettings->psiTables.patBufferPtr && liveStreamingSettings->psiTables.pmtBufferPtr) {
                BKNI_Memcpy(liveStreamingHandle->patBuffer, liveStreamingSettings->psiTables.patBufferPtr, HLS_PAT_PMT_BUFFER_SIZE);
                BKNI_Memcpy(liveStreamingHandle->pmtBuffer, liveStreamingSettings->psiTables.pmtBufferPtr, HLS_PAT_PMT_BUFFER_SIZE);
                liveStreamingHandle->sendPatPmt = true;
            }
            BKNI_SetEvent(liveStreamingHandle->resumeStreamingEvent);
            BDBG_MSG(("%s: ctx:streamingFd %p:%d : complete, start streaming", BSTD_FUNCTION, (void *)liveStreamingHandle, liveStreamingSettings->streamingFd));
        }
        else if (settings->resumeStreaming && liveStreamingSettings->hlsSession) {
            liveStreamingHandle->data.fd = settings->streamingFd;
            liveStreamingSettings->streamingFd = settings->streamingFd;
            if (settings->hlsSegmentSize > 0)
                liveStreamingSettings->hlsSegmentSize = settings->hlsSegmentSize;

            if (liveStreamingSettings->psiTables.patBufferPtr && liveStreamingSettings->psiTables.pmtBufferPtr) {
                BKNI_Memcpy(liveStreamingHandle->patBuffer, liveStreamingSettings->psiTables.patBufferPtr, HLS_PAT_PMT_BUFFER_SIZE);
                BKNI_Memcpy(liveStreamingHandle->pmtBuffer, liveStreamingSettings->psiTables.pmtBufferPtr, HLS_PAT_PMT_BUFFER_SIZE);
                liveStreamingHandle->sendPatPmt = true;
            }
            if (liveStreamingHandle->threadRunning == false) {
                /* app is asking us to resume streaming, but streaming thread must have existed due to some error, restart it */
                B_Thread_Destroy(liveStreamingHandle->streamingThread);
                if (liveStreamingSettings->enableTimeshifting)
                    liveStreamingThreadFunc = liveStreamingThreadFromFifo;
#ifdef HLS_LOW_LATENCY_STREAMING
                else if (liveStreamingSettings->hlsSession)
                    liveStreamingThreadFunc = hlsStreamingThreadFromRaveBuffer;
#endif
                else
                    liveStreamingThreadFunc = liveStreamingThreadFromRaveBuffer;
                liveStreamingHandle->threadReStarted = true;
                if (startLiveStreamingThread(liveStreamingThreadFunc, liveStreamingHandle) < 0)
                    return B_ERROR_UNKNOWN;
                BDBG_MSG(("%s: CTX %p: Live streaming thread restarted, streaming on fd %d, hlsSegmentSize %d", BSTD_FUNCTION, (void *)liveStreamingHandle, settings->streamingFd, liveStreamingSettings->hlsSegmentSize));
            }
            else {
                if (liveStreamingHandle->resumeStreamingEvent)
                    BKNI_SetEvent(liveStreamingHandle->resumeStreamingEvent);
                else {
                    BDBG_ERR(("%s: CTX %p: resumeStreamingEvent is NULL for new fd %d, returning error", BSTD_FUNCTION, (void *)liveStreamingHandle, settings->streamingFd));
                    return B_ERROR_UNKNOWN;
                }
                BDBG_MSG(("%s: CTX %p: Generated event to resume streaming on fd %d, hlsSegmentSize %d", BSTD_FUNCTION, (void *)liveStreamingHandle, settings->streamingFd, liveStreamingSettings->hlsSegmentSize));
            }
        }
        break;
    }
    case B_PlaybackIpProtocol_eRtp:
    {
        if (settings->streamingEnabled) {
            liveStreamingHandle->data.fd = settings->streamingFd;
            liveStreamingHandle->data.self = net_io_data_write;
            if (B_PlaybackIp_UtilsStreamingCtxOpen(&liveStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("Failed to setup the streaming context\n"));
                return B_ERROR_UNKNOWN;
            }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
            if (B_PlaybackIp_UtilsPvrDecryptionCtxOpen(&liveStreamingSettings->securitySettings, &liveStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("%s: Failed to setup pvr decryption", BSTD_FUNCTION));
                return B_ERROR_UNKNOWN;
            }
#endif
            BDBG_MSG(("%s: complete, streaming socket %d", BSTD_FUNCTION, liveStreamingHandle->data.fd));
        }
        break;
    }

    default:
        BDBG_ERR((" non-supported socket type"));
        return B_ERROR_UNKNOWN;
    }

    return B_ERROR_SUCCESS;
}

B_PlaybackIpError
B_PlaybackIp_LiveStreamingStart(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle
    )
{
    B_Error rc;
    B_ThreadFunc liveStreamingThreadFunc;
    B_PlaybackIpLiveStreamingOpenSettings *liveStreamingSettings = &liveStreamingHandle->settings;

    BDBG_MSG(("%s: protocol (%s); socket %u", BSTD_FUNCTION,  (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eHttp)?"HTTP":
 (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eRtp)?"RTP":
              (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eUdp)?"UDP":
              (liveStreamingSettings->protocol==B_PlaybackIpProtocol_eRtsp)?"RTSP":"UNKNOWN", liveStreamingSettings->streamingFd ));
    liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eStarting;
    switch (liveStreamingSettings->protocol) {
    case B_PlaybackIpProtocol_eHttp: {
        if (liveStreamingSettings->streamingDisabled) {
            BDBG_MSG(("%s: streaming fd is not yet set for this session (%p), defer streaming setup", BSTD_FUNCTION, (void *)liveStreamingHandle));
            liveStreamingHandle->data.fd = -1;
            liveStreamingHandle->data.self = net_io_data_write;
            break;
        }
        liveStreamingHandle->data.fd = liveStreamingSettings->streamingFd;
        liveStreamingHandle->data.streamingProtocol = B_PlaybackIpProtocol_eHttp;
        liveStreamingHandle->data.self = net_io_data_write;
        liveStreamingHandle->data.liveStreaming = true;
        if (B_PlaybackIp_UtilsStreamingCtxOpen(&liveStreamingHandle->data) != B_ERROR_SUCCESS) {
            BDBG_ERR(("Failed to setup the streaming context\n"));
            return B_ERROR_UNKNOWN;
        }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
        if (B_PlaybackIp_UtilsPvrDecryptionCtxOpen(&liveStreamingSettings->securitySettings, &liveStreamingHandle->data) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: Failed to setup pvr decryption", BSTD_FUNCTION));
            return B_ERROR_UNKNOWN;
        }
#endif
#ifdef B_HAS_DTCP_IP
        if (B_PlaybackIp_UtilsDtcpServerCtxOpen(&liveStreamingSettings->securitySettings, &liveStreamingHandle->data) != B_ERROR_SUCCESS) {
            BDBG_ERR(("Failed to setup the streaming context\n"));
            return B_ERROR_UNKNOWN;
        }
#endif
        break;
        }
        case B_PlaybackIpProtocol_eRtp:
        case B_PlaybackIpProtocol_eRtsp:
        case B_PlaybackIpProtocol_eUdp: {
            liveStreamingHandle->data.interfaceName = liveStreamingSettings->rtpUdpSettings.interfaceName;
            liveStreamingHandle->data.streamingProtocol = liveStreamingSettings->protocol;
            BDBG_MSG(("%s: liveStreamingsettings->psi 1 (%p); streamingFd (%d)", BSTD_FUNCTION, (void *)liveStreamingSettings->psi, liveStreamingSettings->streamingFd ));fflush(stderr);fflush(stdout);
            if (liveStreamingSettings->psi) {
                if (liveStreamingSettings->psi->mpegType == 0 || liveStreamingSettings->psi->mpegType > 10 ) liveStreamingSettings->psi->mpegType  = NEXUS_TransportType_eTs;
            }
            liveStreamingHandle->data.fd = liveStreamingSettings->streamingFd;
            liveStreamingHandle->data.self = net_io_data_write;
            liveStreamingHandle->data.streamingSockAddr.sin_family = AF_INET;
            liveStreamingHandle->data.streamingSockAddr.sin_port = htons(liveStreamingSettings->rtpUdpSettings.streamingPort);
            liveStreamingHandle->data.ipTtl = liveStreamingSettings->ipTtl;
            if (inet_aton(liveStreamingSettings->rtpUdpSettings.streamingIpAddress, &liveStreamingHandle->data.streamingSockAddr.sin_addr) == 0) {
                BDBG_ERR(("%s: inet_aton() failed on %s", BSTD_FUNCTION, liveStreamingSettings->rtpUdpSettings.streamingIpAddress));
                goto error;
            }
            BDBG_MSG(("Streaming URL is %s%s:%d",
                      liveStreamingSettings->protocol == B_PlaybackIpProtocol_eRtp? "rtp://":
                      liveStreamingSettings->protocol == B_PlaybackIpProtocol_eRtsp? "rtsp://":"udp://",
                        inet_ntoa(liveStreamingHandle->data.streamingSockAddr.sin_addr), ntohs(liveStreamingHandle->data.streamingSockAddr.sin_port)));

            if (B_PlaybackIp_UtilsRtpUdpStreamingCtxOpen(&liveStreamingHandle->settings.securitySettings, &liveStreamingHandle->data) != B_ERROR_SUCCESS) {
                BDBG_ERR(("Failed to setup the streaming context\n"));
                goto error;
            }
            if ( (liveStreamingSettings->protocol == B_PlaybackIpProtocol_eRtp) || (liveStreamingSettings->protocol == B_PlaybackIpProtocol_eUdp) ) {
                BDBG_MSG(("%s: liveStreamingsettings->psi 2 (%p)", BSTD_FUNCTION, (void *)liveStreamingSettings->psi ));fflush(stderr);fflush(stdout);
                if (liveStreamingSettings->psi) {
                    B_PlaybackIp_UtilsSetRtpPayloadType(liveStreamingSettings->psi->mpegType, &liveStreamingHandle->data.rtpPayloadType);
                    BDBG_MSG(("%s: complete for RTP/UDP streaming, socket %d", BSTD_FUNCTION, liveStreamingHandle->data.fd));
                } else /* some servers (satip live streaming) provide pids; there is no psi in this case */ {
                    B_PlaybackIp_UtilsSetRtpPayloadType(NEXUS_TransportType_eTs, &liveStreamingHandle->data.rtpPayloadType);
                    BDBG_MSG(("%s: complete for RTP/UDP streaming, socket %d", BSTD_FUNCTION, liveStreamingHandle->data.fd));
                }
            }
            break;
        }
    default:
        BDBG_ERR(("%s: non-supported socket type", BSTD_FUNCTION ));
        break;
    }

    if (BKNI_CreateEvent(&liveStreamingHandle->startStreamingEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }
    if (BKNI_CreateEvent(&liveStreamingHandle->resumeStreamingEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }
    if (BKNI_CreateEvent(&liveStreamingHandle->stopStreamingEvent)) {
        BDBG_ERR(("%s: Failed to create an event\n", BSTD_FUNCTION));
        goto error;
    }
    /* Create Event group */
    liveStreamingHandle->eventGroup = B_EventGroup_Create( NULL );
    PBIP_CHECK_GOTO(( liveStreamingHandle->eventGroup ), ( "Event Group Creation Failed" ), error, B_ERROR_UNKNOWN, rc );
    rc = B_EventGroup_AddEvent( liveStreamingHandle->eventGroup, liveStreamingHandle->resumeStreamingEvent );
    liveStreamingHandle->maxTriggeredEvents++;
    PBIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, B_ERROR_UNKNOWN, rc );
    rc = B_EventGroup_AddEvent( liveStreamingHandle->eventGroup, liveStreamingHandle->dataReadyEvent );
    PBIP_CHECK_GOTO(( !rc ), ( "Failed to Add event to group" ), error, B_ERROR_UNKNOWN, rc );
    liveStreamingHandle->maxTriggeredEvents++;

    if (liveStreamingSettings->enableTimeshifting)
        liveStreamingThreadFunc = liveStreamingThreadFromFifo;
#ifdef HLS_LOW_LATENCY_STREAMING
    else if (liveStreamingSettings->hlsSession)
        liveStreamingThreadFunc = hlsStreamingThreadFromRaveBuffer;
#endif
    else
        liveStreamingThreadFunc = liveStreamingThreadFromRaveBuffer;
    if (startLiveStreamingThread(liveStreamingThreadFunc, liveStreamingHandle) < 0)
        goto error;
    return B_ERROR_SUCCESS;

error:
    B_PlaybackIp_LiveStreamingStop(liveStreamingHandle);
#ifdef B_HAS_DTCP_IP
    B_PlaybackIp_UtilsDtcpServerCtxClose(&liveStreamingHandle->data);
#endif
    B_PlaybackIp_UtilsStreamingCtxClose(&liveStreamingHandle->data);
    return B_ERROR_UNKNOWN;
}


/***************************************************************************
Summary:
This function stops streaming content from a file.
***************************************************************************/
void
B_PlaybackIp_LiveStreamingStop(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle
    )
{
    BERR_Code rc;
    BDBG_MSG(("%s: %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
    BDBG_ASSERT(liveStreamingHandle);
    if (liveStreamingHandle->state == B_PlaybackIpLiveStreamingState_eStopped) {
        BDBG_WRN(("%s: CTX %p is already stopped", BSTD_FUNCTION, (void *)liveStreamingHandle));
        return;
    }
    liveStreamingHandle->stop = true;
    liveStreamingHandle->data.stopStreaming = true;

    liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eStopping;
    if (liveStreamingHandle->stopStreamingEvent) {
        /* send an event to the feeder thread indicating */
        /* that it no longer needs to wait for data as app is stopping this session */
        if (liveStreamingHandle->dataReadyEvent)
            BKNI_SetEvent(liveStreamingHandle->dataReadyEvent);
        /* thread could even be waiting for resumeStreamingEvent, generate that as well */
        if (liveStreamingHandle->resumeStreamingEvent)
            BKNI_SetEvent(liveStreamingHandle->resumeStreamingEvent);
        if (liveStreamingHandle->threadRunning) {
            rc = BKNI_WaitForEvent(liveStreamingHandle->stopStreamingEvent, 1000);
            if (rc == BERR_TIMEOUT)
                BDBG_WRN(("%s: stopStreamingEvent timed out", BSTD_FUNCTION));
            else
            if (rc != 0) {
                BDBG_ERR(("%s: failed to stop the file streaming thread", BSTD_FUNCTION));
            }
            liveStreamingHandle->threadRunning = false;
        }
        BKNI_DestroyEvent(liveStreamingHandle->stopStreamingEvent);
        liveStreamingHandle->stopStreamingEvent = NULL;
    }
    switch (liveStreamingHandle->settings.protocol) {
        case B_PlaybackIpProtocol_eRtp:
        case B_PlaybackIpProtocol_eUdp: {
            B_PlaybackIp_UtilsRtpUdpStreamingCtxClose(&liveStreamingHandle->data);
            break;
        }
        case B_PlaybackIpProtocol_eHttp:
        default:
            break;
    }
    if (liveStreamingHandle->startStreamingEvent) {
        BKNI_DestroyEvent(liveStreamingHandle->startStreamingEvent);
        liveStreamingHandle->startStreamingEvent = NULL;
    }
    if (liveStreamingHandle->eventGroup && liveStreamingHandle->resumeStreamingEvent) {
        B_EventGroup_RemoveEvent(liveStreamingHandle->eventGroup, liveStreamingHandle->resumeStreamingEvent);
        BKNI_DestroyEvent(liveStreamingHandle->resumeStreamingEvent);
        liveStreamingHandle->resumeStreamingEvent = NULL;
    }
    if (liveStreamingHandle->eventGroup && liveStreamingHandle->dataReadyEvent) {
        B_EventGroup_RemoveEvent(liveStreamingHandle->eventGroup, liveStreamingHandle->dataReadyEvent);
        /* we dont destroy this event as it is created by the app */
        liveStreamingHandle->dataReadyEvent = NULL;
    }
    if (liveStreamingHandle->eventGroup) {
        B_EventGroup_Destroy(liveStreamingHandle->eventGroup);
        liveStreamingHandle->eventGroup = NULL;
    }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    B_PlaybackIp_UtilsPvrDecryptionCtxClose(&liveStreamingHandle->data);
#endif
    if (liveStreamingHandle->streamingThread) {
        B_Thread_Destroy(liveStreamingHandle->streamingThread);
        liveStreamingHandle->streamingThread = NULL;
    }
    BDBG_MSG(("%s: DONE %p", BSTD_FUNCTION, (void *)liveStreamingHandle));
    liveStreamingHandle->state = B_PlaybackIpLiveStreamingState_eStopped;
    return;
}

void
B_PlaybackIp_LiveStreamingGetStatus(
    B_PlaybackIpLiveStreamingHandle liveStreamingHandle,
    B_PlaybackIpLiveStreamingStatus *status
    )
{
    BDBG_ASSERT(status);
    BDBG_ASSERT(liveStreamingHandle);
    status->bytesStreamed = liveStreamingHandle->totalBytesStreamed;
    status->connectionState = liveStreamingHandle->connectionState;
}

#endif /* LINUX || VxWorks */
