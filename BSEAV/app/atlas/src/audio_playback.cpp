/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "audio_playback.h"
#include "nexus_simple_audio_playback.h"
#include "atlas_os.h"
#include "board.h"
#include "wav_file.h"

BDBG_MODULE(atlas_audio_playback);

#define CONVERT_TO_NEXUS_LINEAR_VOL(vol, max_vol)  (((vol) * (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN) / (max_vol)) + NEXUS_AUDIO_VOLUME_LINEAR_MIN)

static void nexusSimpleAudioPlaybackBufferReady(
        void * context,
        int    param
        )
{
    B_EventHandle event = (B_EventHandle)context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(NULL != event);

    B_Event_Set(event);
} /* NEXUSSimpleAudioPlaybackBufferComplete */

CSimplePcmPlayback::CSimplePcmPlayback(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_simplePcmPlayback, pCfg),
    _simplePlayback(NULL),
    _audioPlayback(NULL),
    _pSpdif(NULL),
    _pHdmi(NULL),
    _startOffset(0),
    _jobReadyEvent(NULL),
    _bufferReadyEvent(NULL),
    _playbackThread_handle(NULL),
    _resourceId(NULL),
    _bRun(true),
    _mutex(NULL),
    _pBoardResources(NULL),
    _pWidgetEngine(NULL),
    _pModel(NULL)
{
}

eRet CSimplePcmPlayback::open(CWidgetEngine * pWidgetEngine)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;
    int         i      = 0;

    NEXUS_SimpleAudioPlaybackServerSettings settings;

    if (true == isOpened())
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Simple audio playback is already opened.", ret, error);
    }

    if (NULL == _pBoardResources)
    {
        ret = eRet_InvalidState;
        CHECK_ERROR_GOTO("Set board resources before opening.", ret, error);
    }

    _pWidgetEngine = pWidgetEngine;

    _mutex = B_Mutex_Create(NULL);
    CHECK_PTR_ERROR_GOTO("unable to create mutex", _mutex, ret, eRet_ExternalError, error);

    _jobReadyEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("audio playback job ready event creation failed", _jobReadyEvent, ret, eRet_NotAvailable, error);

    _bufferReadyEvent = B_Event_Create(NULL);
    CHECK_PTR_ERROR_GOTO("audio playback buffer ready event creation failed", _bufferReadyEvent, ret, eRet_NotAvailable, error);

    _audioPlayback = NEXUS_AudioPlayback_Open(getNumber(), NULL);
    CHECK_PTR_WARN_GOTO("unable to create audio playback", _audioPlayback, ret, eRet_NotAvailable, error);

    NEXUS_SimpleAudioPlayback_GetDefaultServerSettings(&settings);
    settings.playback = _audioPlayback;

    /* if an audio spdif resource given, set up audio output */
    if (NULL != _pSpdif)
    {
        /* _pSpdif->connect(NEXUS_AudioPlayback_GetConnector(_audioPlayback)); */
        settings.compressed.enabled         = true;
        settings.compressed.spdifOutputs[0] = _pSpdif->getOutput();
    }

    /* if an audio hdmi resource given, set up audio output */
    if (NULL != _pHdmi)
    {
        /* _pHdmi->connect(NEXUS_AudioPlayback_GetConnector(_audioPlayback)); */
        settings.compressed.enabled        = true;
        settings.compressed.hdmiOutputs[0] = _pHdmi->getOutput();
    }

    _simplePlayback = NEXUS_SimpleAudioPlayback_Create(_pModel->getSimpleAudioPlaybackServer(), _number, &settings);
    CHECK_PTR_ERROR_GOTO("unable to create a simple audio playback", _simplePlayback, ret, eRet_OutOfMemory, error);

    goto done;
error:
    close();
done:
    return(ret);
} /* open */

void CSimplePcmPlayback::getSettings(NEXUS_SimpleAudioPlaybackServerSettings * pSettings)
{
    BDBG_ASSERT(NULL != _simplePlayback);

    NEXUS_SimpleAudioPlayback_GetServerSettings(_pModel->getSimpleAudioPlaybackServer(), _simplePlayback, pSettings);
}

eRet CSimplePcmPlayback::setSettings(NEXUS_SimpleAudioPlaybackServerSettings * pSettings)
{
    NEXUS_Error nerror = NEXUS_SUCCESS;
    eRet        ret    = eRet_Ok;

    BDBG_ASSERT(NULL != _simplePlayback);

    nerror = NEXUS_SimpleAudioPlayback_SetServerSettings(_pModel->getSimpleAudioPlaybackServer(), _simplePlayback, pSettings);
    CHECK_NEXUS_ERROR_GOTO("unable to set simple audio playback server settings", ret, nerror, error);

error:
    return(ret);
}

void CSimplePcmPlayback::close()
{
    if (true == _bRun)
    {
        stop();
    }

    if (NULL != _simplePlayback)
    {
        NEXUS_SimpleAudioPlayback_Destroy(_simplePlayback);
        _simplePlayback = NULL;
    }

    if (NULL != _bufferReadyEvent)
    {
        B_Event_Destroy(_bufferReadyEvent);
        _bufferReadyEvent = NULL;
    }

    if (NULL != _jobReadyEvent)
    {
        B_Event_Destroy(_jobReadyEvent);
        _jobReadyEvent = NULL;
    }

    if (NULL != _mutex)
    {
        B_Mutex_Destroy(_mutex);
        _mutex = NULL;
    }

    _pSpdif = NULL;
    _pHdmi  = NULL;

    if (NULL != _audioPlayback)
    {
        NEXUS_AudioPlayback_Close(_audioPlayback);
        _audioPlayback = NULL;
    }

    _pWidgetEngine = NULL;
} /* close */

static void playbackThread(void * pParam)
{
    CSimplePcmPlayback * pAudioPlayback = (CSimplePcmPlayback *)pParam;

    BDBG_ASSERT(NULL != pParam);

    pAudioPlayback->doPlayback();
}

void CSimplePcmPlayback::destroyThreadPlayback()
{
    if (NULL != _playbackThread_handle)
    {
        _bRun = false;

        B_Thread_Destroy(_playbackThread_handle);
        _playbackThread_handle = NULL;
    }
}

void CSimplePcmPlayback::clearPlaybackQueue()
{
    CScopedMutex jobListMutex(_mutex);

    _pcmSoundList.clear();
}

unsigned CSimplePcmPlayback::playbackJobCount()
{
    CScopedMutex jobListMutex(_mutex);

    return(_pcmSoundList.total());
}

void CSimplePcmPlayback::addPlaybackJob(MString strFilename)
{
    CScopedMutex jobListMutex(_mutex);

    _pcmSoundList.add(new MString(strFilename));
}

MString CSimplePcmPlayback::removePlaybackJob()
{
    MString * pStr = NULL;
    MString   strTmp;

    CScopedMutex jobListMutex(_mutex);

    pStr = _pcmSoundList.remove();
    if (NULL != pStr)
    {
        strTmp = *pStr;
        DEL(pStr);
    }

    return(strTmp);
}

eRet CSimplePcmPlayback::start(const char * strFilename)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    addPlaybackJob(strFilename);

    if ((false == _bRun) || (NULL == _playbackThread_handle))
    {
        _playbackThread_handle = B_Thread_Create("CSimplePcmPlayback Thread", playbackThread, this, NULL);
        CHECK_PTR_ERROR_GOTO("audio playback thread creation failed!", _playbackThread_handle, ret, eRet_ExternalError, error);
    }

    B_Event_Set(_jobReadyEvent);

    goto done;
error:
    stop();
done:
    return(ret);
} /* start */

/* thread to feed playback buffeu data to nexus - this allows playback of pcm audio to
 * not to block atlas.
 */
void CSimplePcmPlayback::doPlayback()
{
    eRet               ret    = eRet_Ok;
    NEXUS_Error        nerror = NEXUS_SUCCESS;
    B_Error            berror = B_ERROR_SUCCESS;
    struct wave_header wave_header;
    NEXUS_SimpleAudioPlaybackStartSettings startSettings;
    MString   strFilename;
    MString   strFilenameLast;
    FILE *    pFile       = NULL;
    int       nTimeouts   = 0;
    uint8_t * pCopyBuffer = NULL;

    while (1)
    {
        if (false == _bRun)
        {
            break;
        }

        if (0 == playbackJobCount())
        {
            berror = B_Event_Wait(_jobReadyEvent, 1000);
            /*CHECK_BOS_WARN("timeout waiting for audio pcm playback job", ret, berror);*/
            continue;
        }

        /* requested sounds are queued in list - get next sound to play */
        strFilename = removePlaybackJob();
        if (strFilename.isEmpty())
        {
            clearPlaybackQueue();
            continue;
        }

        if (strFilename == strFilenameLast)
        {
            /* play same file */
            fseek(pFile, _startOffset, SEEK_SET);
        }
        else
        {
            /* play new file */
            if (NULL != pFile)
            {
                fclose(pFile);
                pFile           = NULL;
                strFilenameLast = "";
            }

            pFile = fopen(strFilename, "r");
            CHECK_PTR_ERROR_GOTO("unable to open given filename", pFile, ret, eRet_NotAvailable, error);

            strFilenameLast = strFilename;
        }

        NEXUS_SimpleAudioPlayback_GetDefaultStartSettings(&startSettings);

        if (-1 != read_wave_header(pFile, &wave_header))
        {
            startSettings.sampleRate    = wave_header.samplesSec;
            startSettings.bitsPerSample = wave_header.bps;
            startSettings.stereo        = (wave_header.channels == 2) ? true : false;
            startSettings.endian        = NEXUS_EndianMode_eLittle;

            _startOffset = wave_header.dataStart;
        }
        else
        {
            startSettings.sampleRate    = 44100;
            startSettings.bitsPerSample = 16;
            startSettings.stereo        = true;
        }

        BDBG_MSG(("PCM audio, %d samples/sec, %d bits/sample, %d channel(s)\n", startSettings.sampleRate, startSettings.bitsPerSample, startSettings.stereo ? 2 : 1));

        startSettings.dataCallback.callback = nexusSimpleAudioPlaybackBufferReady;
        startSettings.dataCallback.context  = _bufferReadyEvent;
        startSettings.loopAround            = false;

        nerror = NEXUS_SimpleAudioPlayback_Start(_simplePlayback, &startSettings);
        CHECK_NEXUS_ERROR_GOTO("unable to start simple audio playback", ret, nerror, error);

        /* fill playback buffer */
        while (1)
        {
            void * buffer = NULL;
            size_t size   = 0;
            int    nBytes = 0;

            if (false == _bRun)
            {
                break;
            }

            if (3 < nTimeouts)
            {
                break;
            }

            nerror = NEXUS_SimpleAudioPlayback_GetBuffer(_simplePlayback, &buffer, &size);
            CHECK_NEXUS_ERROR_GOTO("unable to get audio playback buffer", ret, nerror, error);

            if (0 == size)
            {
                berror = B_Event_Wait(_bufferReadyEvent, 1000);
                nTimeouts++;
                /*CHECK_BOS_WARN("timeout waiting for audio playback buffer to become available", ret, berror);*/
                continue;
            }

            /* data buffer is ready so reset timeout count */
            nTimeouts = 0;

            if (24 == startSettings.bitsPerSample)
            {
                size_t     calculatedBufferSize = (size / 4) * 3;
                uint16_t * pBuffer              = NULL;
                int        i                    = 0;
                int        bufferSize           = 0;

                pCopyBuffer = (uint8_t *)BKNI_Malloc(calculatedBufferSize * sizeof(uint8_t));
                CHECK_PTR_ERROR_GOTO("unable to alloc the copy buffer", pCopyBuffer, ret, eRet_OutOfMemory, error);

                bufferSize = fread(pCopyBuffer, 1, calculatedBufferSize, pFile);
                if (0 > bufferSize)
                {
                    break;
                }
                else
                if (bufferSize == 0)
                {
                    /* eof */
                    BFRE(pCopyBuffer);
                    break;
                }

                nBytes  = 0;
                pBuffer = (uint16_t *)buffer;
                for (i = 0; i < (bufferSize / 3); i++)
                {
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG
                    pBuffer[(i * 2)]     = pCopyBuffer[(i*3)] << 8 | pCopyBuffer[(i*3)+1];
                    pBuffer[(i * 2) + 1] = pCopyBuffer[(i*3)+2] << 8 | 0x00;
#else
                    pBuffer[(i * 2)]     = pCopyBuffer[(i*3)] << 8 | 0x0;
                    pBuffer[(i * 2) + 1] = pCopyBuffer[(i * 3) + 2] << 8 | pCopyBuffer[(i * 3) + 1];
#endif /* if BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG */
                    nBytes += 4;
                }
                BFRE(pCopyBuffer);
            }
            else
            {
                nBytes = fread(buffer, 1, size, pFile);
                if (nBytes < 0)
                {
                    break;
                }
                else
                if (nBytes == 0)
                {
                    /* eof */
                    break;
                }
            }

            nerror = NEXUS_SimpleAudioPlayback_WriteComplete(_simplePlayback, nBytes);
            CHECK_NEXUS_ERROR_GOTO("error in audio playback write complete", ret, nerror, error);

            BDBG_MSG(("audio playback %d bytes added to pcm write buffer", nBytes));
        }

        waitForPcmPlaybackToComplete();
        NEXUS_SimpleAudioPlayback_Stop(_simplePlayback);
    }

error:
    BFRE(pCopyBuffer);

    if (NULL != pFile)
    {
        fclose(pFile);
        pFile = NULL;
    }

    if (NULL != _simplePlayback)
    {
        NEXUS_SimpleAudioPlayback_Stop(_simplePlayback);
    }

    _bRun = false;
} /* doPlayback */

eRet CSimplePcmPlayback::stop()
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    destroyThreadPlayback();
    _bRun = false;

    return(ret);
} /* stop */

void CSimplePcmPlayback::setResources(
        void *            id,
        CBoardResources * pResources
        )
{
    BDBG_ASSERT(NULL != id);
    BDBG_ASSERT(NULL != pResources);

    _resourceId      = id;
    _pBoardResources = pResources;
}

void CSimplePcmPlayback::waitForPcmPlaybackToComplete(long timeout)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;
    B_Time      timeTimeout;

    B_Time_Get(&timeTimeout);
    B_Time_Add(&timeTimeout, timeout);

    /* wait until file plays, then stop. we must protect against infinite looping here
     * which can happen if decimated video window is closed before audio plays. */
    while (1)
    {
        B_Time timeCurrent;
        NEXUS_SimpleAudioPlaybackStatus status;

        nerror = NEXUS_SimpleAudioPlayback_GetStatus(_simplePlayback, &status);
        CHECK_NEXUS_ERROR_GOTO("unable to get audio playback status", ret, nerror, error);

        /* BDBG_MSG(("Waiting for PCM playback to complete. queuedBytes:%d", status.queuedBytes)); */
        B_Time_Get(&timeCurrent);
        if ((-1 != timeout) && (0 > B_Time_Diff(&timeTimeout, &timeCurrent)))
        {
            BDBG_ERR(("TTTTTTTTTTTTTTTTTT timeout in %s", __FUNCTION__));
            break;
        }

        if (0 >= status.queuedBytes)
        {
            break;
        }
    }
error:
    return;
} /* waitForPcmPlaybackToComplete */