/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "nexus_audio_capture.h"
#include "audio_capture_nx.h"
#include "atlas_os.h"
#ifdef NETAPP_SUPPORT
#include "netapp_os.h"
#include "netapp.h"
#endif
/* included for writing to a file */

BDBG_MODULE(atlas_audio_capture);
#ifdef NETAPP_SUPPORT
#define CALLBACK_AUDIO_CAPTURE_DATA        "callbackAudioCaptuerData"
#define CALLBACK_AUDIO_CAPTURE_SAMPLERATE  "callbackAudioCaptureSampleRate"

#define PCM_FILE                           "videos/out.pcm"
#include "bstd.h"
#include "bdbg.h"
#include <stdio.h>
#include <string.h>

/**
 * NOTE: This API is only example code. It is subject to change and
 * is not supported as a standard reference software deliverable.
 **/

struct wave_header
{
    unsigned long    riff;        /* 'RIFF' */
    unsigned long    riffCSize;   /* size in bytes of file - 8 */
    unsigned long    wave;        /* 'WAVE' */
    unsigned long    fmt;         /* 'fmt ' */
    unsigned long    headerLen;   /* header length (should be 16 for PCM) */
    unsigned short   format;      /* 1 - pcm */
    unsigned short   channels;    /* 1 - mono, 2 - stereo */
    unsigned long    samplesSec;  /* samples / second */
    unsigned long    bytesSec;    /* bytes / second */
    unsigned short   chbits;      /* channels * bits/sample /8 */
    unsigned short   bps;         /* bits per sample (8 or 16) */

    /* Extensible format */
    unsigned short   cbSize;             /* 2 Size of the extension (0 or 22)  */
    unsigned short   validBitsPerSample; /* 2 Number of valid bits  */
    unsigned short   channelMask;        /* 4 Speaker position mask  */
    unsigned char    subFormat[16];      /* SubFormat */

    unsigned long   dataSig;      /* 'data' */
    unsigned long   dataLen;      /* length of data */
};

void get_default_wave_header(struct wave_header * wh);
int  write_wave_header(FILE * file, const struct wave_header * wh);

/* based on nexus/examples/audio/audio_playback.c */

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
#define SWAP32(a)  do { a = ((a&0xFF)<<24|(a&0xFF00)<<8|(a&0xFF0000)>>8|(a&0xFF000000)>>24); } while (0)
#define SWAP16(a)  do { a = ((a&0xFF)<<8|(a&0xFF00)>>8); } while (0)
#else
#define SWAP32(a)
#define SWAP16(a)
#endif /* if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG */

#define READ32(VAL, FILE)   do { if (fread(&(VAL), 4, 1, (FILE)) != 1) { goto done; } SWAP32(VAL); } while (0)
#define READ16(VAL, FILE)   do { if (fread(&(VAL), 2, 1, (FILE)) != 1) { goto done; } SWAP16(VAL); } while (0)

#define WRITE32(VAL, FILE)  do { SWAP32(VAL); if (fwrite(&(VAL), 4, 1, (FILE)) != 1) { goto done; } } while (0)
#define WRITE16(VAL, FILE)  do { SWAP16(VAL); if (fwrite(&(VAL), 2, 1, (FILE)) != 1) { goto done; } } while (0)

#define WAVE_VALUE_RIFF        0x46464952
#define WAVE_VALUE_WAV         0x45564157
#define WAVE_VALUE_DATA_CHUNK  0x61746164

void get_default_wave_header(struct wave_header * wh)
{
    memset(wh, 0, sizeof(*wh));
    wh->riff       = WAVE_VALUE_RIFF;
    wh->wave       = WAVE_VALUE_WAV;
    wh->headerLen  = 16;
    wh->format     = 1;
    wh->channels   = 2;
    wh->samplesSec = 44100;
    wh->bytesSec   = 0;
    wh->chbits     = 0;
    wh->bps        = 16;
    wh->dataSig    = WAVE_VALUE_DATA_CHUNK;
}

int write_wave_header(
        FILE *                     file,
        const struct wave_header * _wh
        )
{
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
    /* copy to that SWAP macros can work in place */
    struct wave_header   copy = *_wh;
    struct wave_header * wh   = &copy;
#else
    const struct wave_header * wh = _wh;
#endif /* if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG */

    WRITE32(wh->riff, file);
    WRITE32(wh->riffCSize, file);
    WRITE32(wh->wave, file);
    WRITE32(wh->fmt, file);
    WRITE32(wh->headerLen, file);
    WRITE16(wh->format, file);
    WRITE16(wh->channels, file);
    WRITE32(wh->samplesSec, file);
    WRITE32(wh->bytesSec, file);
    WRITE16(wh->chbits, file);
    WRITE16(wh->bps, file);
    if ((wh->headerLen == 40) && (wh->format == 0xfffe)) /* WAVE_FORMAT_EXTENSIBLE */
    {
        WRITE16(wh->cbSize, file);             /* 2 Size of the extension (0 or 22)  */
        WRITE16(wh->validBitsPerSample, file); /* 2 Number of valid bits  */
        WRITE32(wh->channelMask, file);        /* 4 Speaker position mask  */
        fwrite(&wh->subFormat, 16, 1, file);   /* SubFormat GUID */
    }
    else
    if ((wh->headerLen == 18) && (wh->format == 1)) /* oddball WAVE format */
    {
        WRITE16(wh->cbSize, file); /* 2 Size of the extension (0 or 22) ?*/
    }
    WRITE32(wh->dataSig, file);
    WRITE32(wh->dataLen, file);
    return(0);

done:
    fseek(file, 0, SEEK_SET);
    return(-1);
} /* write_wave_header */

static void NexusBtAvSource_P_CaptureCallback(
        void *       pObject,
        const char * strCallback
        );

static void NexusBtAvSource_P_CaptureTask(
        void * pParam);

#if 0

static void bwinDumpToFileTask(
        void * pParam);

static void bwinDumpToFile_P_CaptureCallback(
        void *       pObject,
        const char * strCallback
        );

/* Call this instead of  bwin Capture callback */
static void bwinDumpToFile_P_CaptureCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CAudioCaptureNx * pAudioCaptureNx = (CAudioCaptureNx *)pObject;

    BSTD_UNUSED(strCallback);

    if (pAudioCaptureNx->TaskId == NULL)
    {
        pAudioCaptureNx->TaskId = NetAppOSTaskSpawn("bwinDumpToFileTask",
                NETAPP_OS_PRIORITY_NORMAL, 64*1024, bwinDumpToFileTask, pAudioCaptureNx);
    }
}

static void bwinDumpToFileTask(void * pObject)
{
    CAudioCaptureNx * pAudioCaptureNx = (CAudioCaptureNx *)pObject;
    NEXUS_Error       rc;

    FILE * file = NULL;

    file = fopen(PCM_FILE, "w+");
    if (!file)
    {
        fprintf(stderr, "### unable to open %s\n", PCM_FILE);
        return;
    }

    if (file)
    {
        BDBG_WRN(("write WAV header"));
        /* write a WAV header */
        NEXUS_AudioCaptureSettings settings;
        NEXUS_AudioCaptureStatus   status;
        struct wave_header         wave_header;

        get_default_wave_header(&wave_header);
        NEXUS_AudioCapture_GetSettings(pAudioCaptureNx->hCapture, &settings);
        switch (settings.format)
        {
        default:
        case NEXUS_AudioCaptureFormat_e16BitStereo:
            wave_header.bps      = 16;
            wave_header.channels = 2;
            break;
        case NEXUS_AudioCaptureFormat_e24BitStereo:
        case NEXUS_AudioCaptureFormat_e24Bit5_1:
            wave_header.bps      = 24;
            wave_header.channels = 2;
            break;
        case NEXUS_AudioCaptureFormat_e16BitMonoLeft:
        case NEXUS_AudioCaptureFormat_e16BitMonoRight:
        case NEXUS_AudioCaptureFormat_e16BitMono:
            wave_header.bps      = 16;
            wave_header.channels = 1;
            break;
        } /* switch */
        rc = NEXUS_AudioCapture_GetStatus(pAudioCaptureNx->hCapture, &status);
        BDBG_ASSERT(!rc);
        wave_header.samplesSec = status.sampleRate;
        write_wave_header(file, &wave_header);
    }

    while (!pAudioCaptureNx->bShutdown)
    {
        void *   buffer;
        unsigned size;
        NEXUS_AudioCapture_GetBuffer(pAudioCaptureNx->hCapture, &buffer, &size);
        if (!size)
        {
            BKNI_Sleep(10); /* could register for callback and wait for event */
            continue;
        }
        if (file)
        {
            fwrite(buffer, size, 1, file);
        }
        BDBG_WRN(("read %d bytes", size));
        NEXUS_AudioCapture_ReadComplete(pAudioCaptureNx->hCapture, size);
    }

    NEXUS_AudioCapture_Stop(pAudioCaptureNx->hCapture);
    if (file)
    {
        fclose(file);
    }
} /* bwinDumpToFileTask */

#endif /* if 0 */

/* Call this instead of  bwin Capture callback
 * TODO get rid of the NETAPP OS calls in order to make it compatible when NETAPP SUPPORT = n */ \
    static void NexusBtAvSource_P_CaptureCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CAudioCaptureNx * pAudioCaptureNx = (CAudioCaptureNx *)pObject;

    BSTD_UNUSED(strCallback);

    if (pAudioCaptureNx->TaskId == NULL)
    {
        pAudioCaptureNx->TaskId = NetAppOSTaskSpawn("NexusBtAvSource_P_CaptureTask",
                NETAPP_OS_PRIORITY_NORMAL, 64*1024, NexusBtAvSource_P_CaptureTask, pAudioCaptureNx);
    }
}

static void NexusBtAvSource_P_CaptureTask(void * pParam)
{
    CAudioCaptureNx *     pAudioCaptureNx     = (CAudioCaptureNx *)pParam;
    CAudioCaptureClient * pAudioCaptureClient = NULL;
    NEXUS_Error           errCode;
    eRet                  ret = eRet_Ok;

    while (!pAudioCaptureNx->bShutdown)
    {
        void * pBuffer;
        size_t bufferSize;
        /* Check available buffer space */
        errCode = NEXUS_AudioCapture_GetBuffer(pAudioCaptureNx->hCapture, (void **)&pBuffer, &bufferSize);
        if (errCode)
        {
            BDBG_ERR(("Error getting capture buffer"));
            NEXUS_AudioCapture_Stop(pAudioCaptureNx->hCapture);
            return;
        }

        if (bufferSize > 0)
        {
            /* Go through list  */
            MListItr <CAudioCaptureClient> itr(pAudioCaptureNx->getClientList());
            /* update bluetooth  info if already in bt device list */
            for (pAudioCaptureClient = itr.first(); pAudioCaptureClient; pAudioCaptureClient = itr.next())
            {
                ret = pAudioCaptureClient->processAudioData(&pBuffer, bufferSize);
                CHECK_ERROR("AudioCaptureClient processAudio Data  buffer failed", ret);
            }

            /*
             * fprintf(stderr, "Data callback - Wrote %d bytes\n", (int)bufferSize);
             *     BDBG_WRN(("read %d bytes", bufferSize));
             */

            errCode = NEXUS_AudioCapture_ReadComplete(pAudioCaptureNx->hCapture, bufferSize);
            if (errCode)
            {
                BDBG_ERR(("Error committing capture buffer"));
                NEXUS_AudioCapture_Stop(pAudioCaptureNx->hCapture);
                return;
            }
        }
        else
        {
            NetAppOSTaskDelayMsec(10);
        }
    }
    NetAppOSTaskExit();
} /* NexusBtAvSource_P_CaptureTask */

#if 0
static void bwinAudioCaptureDataCallback(
        void *       pObject,
        const char * strCallback
        )
{
    eRet                  ret = eRet_Ok;
    unsigned              bufferSize;
    void *                buffer;
    CAudioCaptureNx *     pAudioCaptureNx     = (CAudioCaptureNx *)pObject;
    CAudioCaptureClient * pAudioCaptureClient = NULL;

    BSTD_UNUSED(strCallback);
    BDBG_ASSERT(NULL != pAudioCaptureNx);

    /* get the audio Data */
    ret = pAudioCaptureNx->getBuffer(&buffer, &bufferSize);
    CHECK_ERROR_GOTO("audio capture get buffer failed", ret, error);

    BDBG_MSG(("bwin Audio Data Capture  callback bufferSize %d ", bufferSize));
    if (bufferSize > 0)
    {
        /* Go through list  */
        MListItr <CAudioCaptureClient> itr(pAudioCaptureNx->getClientList());
        /* update bluetooth  info if already in bt device list */
        for (pAudioCaptureClient = itr.first(); pAudioCaptureClient; pAudioCaptureClient = itr.next())
        {
            ret = pAudioCaptureClient->processAudioData(&buffer, bufferSize);
            CHECK_ERROR("AudioCaptureClient processAudio Data  buffer failed", ret);
        }

        BDBG_MSG(("calling readComplete buffer Size %d ", bufferSize));
        /* Tell nexus finish with audio data */
        ret = pAudioCaptureNx->readComplete(bufferSize);
        CHECK_ERROR_GOTO("audio capture read complete failed", ret, error);
    }
    else
    {
        BKNI_Sleep(10);
    }

    return;

error:
    pAudioCaptureNx->stop();
    return;
} /* bwinAudioCaptureDataCallback */

#endif /* if 0 */

static void bwinAudioCaptureSampleRateCallback(
        void *       pObject,
        const char * strCallback
        )
{
    CAudioCaptureNx *        pAudioCaptureNx = (CAudioCaptureNx *)pObject;
    NEXUS_AudioCaptureStatus captureStatus;

    BDBG_ASSERT(NULL != pAudioCaptureNx);
    CAudioCaptureClient * pAudioCaptureClient = NULL;
    BSTD_UNUSED(strCallback);
    BDBG_MSG(("bwin Audio Sample Rate Capture  callback "));

    NEXUS_AudioCapture_GetStatus(pAudioCaptureNx->hCapture, &captureStatus);

    /* Go through list  */
    MListItr <CAudioCaptureClient> itr(pAudioCaptureNx->getClientList());
    /* update bluetooth  info if already in bt device list */
    for (pAudioCaptureClient = itr.first(); pAudioCaptureClient; pAudioCaptureClient = itr.next())
    {
        pAudioCaptureClient->processAudioSampleRate(captureStatus.sampleRate);
    }
} /* bwinAudioCaptureSampleRateCallback */

/* Call back entered into nexus */
static void NexusAudioCaptureDataCallback(
        void * context,
        int    param
        )
{
    CAudioCaptureNx * pAudioCaptureNx = (CAudioCaptureNx *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pAudioCaptureNx);

    CWidgetEngine * pWidgetEngine = pAudioCaptureNx->getWidgetEngine();

    BDBG_MSG(("%s ", __FUNCTION__));
    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pAudioCaptureNx, CALLBACK_AUDIO_CAPTURE_DATA);
    }
}

static void NexusAudioCaptureDataCallbackSampleRateCallback(
        void * context,
        int    param
        )
{
    CAudioCaptureNx * pAudioCaptureNx = (CAudioCaptureNx *)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != pAudioCaptureNx);

    CWidgetEngine * pWidgetEngine = pAudioCaptureNx->getWidgetEngine();

    BDBG_MSG(("%s ", __FUNCTION__));
    /* sync with bwin loop */
    if (NULL != pWidgetEngine)
    {
        pWidgetEngine->syncCallback(pAudioCaptureNx, CALLBACK_AUDIO_CAPTURE_SAMPLERATE);
    }
}

CAudioCaptureNx::CAudioCaptureNx(const char * name) :
    CAudioCapture(name)
{
    eRet ret = eRet_Ok;

    BDBG_ASSERT(eRet_Ok == ret);
    hCapture = NULL;
    memset(&allocResults, 0, sizeof(NxClient_AllocResults));
    TaskId   = NULL;
    bShutdown = false;
    _state    = eAudioCaptureState_Off;
}

CAudioCaptureNx::~CAudioCaptureNx()
{
}

eRet CAudioCaptureNx::open(CWidgetEngine * pWidgetEngine)
{
    eRet                   ret     = eRet_Ok;
    NEXUS_Error            errCode = NEXUS_SUCCESS;
    NxClient_AllocSettings allocSettings;
    /* NxClient_AllocResults allocResults; */

    NEXUS_AudioCaptureOpenSettings tCaptureOpenSettings;

    _pWidgetEngine = pWidgetEngine;

    if (NULL != _pWidgetEngine)
    {
#if 0
        _pWidgetEngine->addCallback(this, CALLBACK_AUDIO_CAPTURE_DATA, bwinAudioCaptureDataCallback);
#else
#if 1
        _pWidgetEngine->addCallback(this, CALLBACK_AUDIO_CAPTURE_DATA, NexusBtAvSource_P_CaptureCallback);
#else
        _pWidgetEngine->addCallback(this, CALLBACK_AUDIO_CAPTURE_DATA, bwinDumpToFile_P_CaptureCallback);
#endif
#endif /* if 0 */
        _pWidgetEngine->addCallback(this, CALLBACK_AUDIO_CAPTURE_SAMPLERATE, bwinAudioCaptureSampleRateCallback);
    }

#if 0
    file = fopen(PCM_FILE, "w+");
    if (!file)
    {
        BDBG_ERR(("### unable to open %s\n", PCM_FILE));
        return(-1);
    }
#endif /* if 0 */

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.audioCapture = 1;
    errCode                    = NxClient_Alloc(&allocSettings, &allocResults);
    CHECK_NEXUS_ERROR_GOTO("Error getting capture buffer", ret, errCode, error);
    NEXUS_AudioCapture_GetDefaultOpenSettings(&tCaptureOpenSettings);
    tCaptureOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_eStereo;
    hCapture = NEXUS_AudioCapture_Open(allocResults.audioCapture.id, &tCaptureOpenSettings);
    CHECK_PTR_ERROR_GOTO("unable to acquire audio capture", hCapture, ret, eRet_NotAvailable, error);

error:
    return(ret);
} /* open */

eRet CAudioCaptureNx::close(void)
{
    eRet ret = eRet_Ok;

    if (NULL != _pWidgetEngine)
    {
        _pWidgetEngine->removeCallback(this, CALLBACK_AUDIO_CAPTURE_DATA);
        _pWidgetEngine->removeCallback(this, CALLBACK_AUDIO_CAPTURE_SAMPLERATE);
        _pWidgetEngine = NULL;
    }

    NEXUS_AudioCapture_Close(hCapture);
    return(ret);
}

eRet CAudioCaptureNx::start(void)
{
    eRet ret = eRet_Ok;

    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_AudioCaptureStartSettings startSettings;

    if (_state == eAudioCaptureState_Off)
    {
        NEXUS_AudioCapture_GetDefaultStartSettings(&startSettings);
        startSettings.dataCallback.callback       = NexusAudioCaptureDataCallback;
        startSettings.dataCallback.context        = this;
        startSettings.sampleRateCallback.callback = NexusAudioCaptureDataCallbackSampleRateCallback;
        startSettings.sampleRateCallback.context  = this;

        BDBG_WRN(("%s: Audio Capture Start ", __FUNCTION__));
        errCode = NEXUS_AudioCapture_Start(hCapture, &startSettings);
        CHECK_NEXUS_ERROR_GOTO("Error starting Audio Capture", ret, errCode, error);
        _state = eAudioCaptureState_On;
    }
error:
    return(ret);
} /* start */

eRet CAudioCaptureNx::stop(void)
{
    eRet ret = eRet_Ok;

    if (this == NULL)
    {
        printf("%s::%s():Not initialized first!", __FILE__, __FUNCTION__);
        ret = eRet_NotAvailable;
    }
    if (_state == eAudioCaptureState_On)
    {
        if (TaskId)
        {
            bShutdown = true;

#ifdef NETAPP_SUPPORT
            NetAppOSTaskJoin(TaskId);
            NetAppOSTaskDelete(TaskId);
#endif
            bShutdown = false;
            TaskId   = NULL;
        }

        BDBG_WRN(("%s: Audio Capture Stop ", __FUNCTION__));
        NEXUS_AudioCapture_Stop(hCapture);
        _state = eAudioCaptureState_Off;
    }
    return(ret);
} /* stop */

eRet CAudioCaptureNx::getBuffer(
        void **    pBuffer,
        unsigned * pBufferSize
        )
{
    eRet ret = eRet_Ok;

    NEXUS_Error errCode = NEXUS_SUCCESS;

    errCode = NEXUS_AudioCapture_GetBuffer(hCapture, (void **)pBuffer, pBufferSize);
    CHECK_NEXUS_ERROR_GOTO("Error getting capture buffer", ret, errCode, error);

error:
    return(ret);
}

eRet CAudioCaptureNx::readComplete(unsigned bufferSize)
{
    eRet        ret = eRet_Ok;
    NEXUS_Error errCode;

    errCode = NEXUS_AudioCapture_ReadComplete(hCapture, bufferSize);
    CHECK_NEXUS_ERROR_GOTO("Error committing capture buffer", ret, errCode, error);
error:
    return(ret);
}

#endif /* ifdef NETAPP_SUPPORT */