/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_simple_decoder_module.h"
#include "nexus_simple_audio_playback.h"
#include "nexus_simple_audio_priv.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_playback.h"
#include "nexus_audio_input.h"
#endif
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_simple_audio_playback);

void NEXUS_SimpleAudioPlayback_GetDefaultServerSettings( NEXUS_SimpleAudioPlaybackServerSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static void NEXUS_SimpleAudioPlayback_P_GetDefaultSettings(NEXUS_SimpleAudioPlaybackSettings *pSettings)
{
    BDBG_CASSERT(sizeof(NEXUS_SimpleAudioPlaybackSettings) == sizeof(NEXUS_AudioPlaybackSettings));
    NEXUS_AudioPlayback_GetDefaultSettings((NEXUS_AudioPlaybackSettings *)pSettings);
}

NEXUS_SimpleAudioPlaybackHandle NEXUS_SimpleAudioPlayback_Create( NEXUS_SimpleAudioDecoderServerHandle server, unsigned index, const NEXUS_SimpleAudioPlaybackServerSettings *pSettings )
{
    NEXUS_SimpleAudioPlaybackHandle handle;
    NEXUS_Error rc;

    /* find dup */
    for (handle=BLST_S_FIRST(&server->playbacks); handle; handle=BLST_S_NEXT(handle, link)) {
        if (handle->index == index) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
    }

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_SimpleAudioPlayback, handle);
    handle->index = index;
    handle->server = server;
    BLST_S_INSERT_HEAD(&server->playbacks, handle, link);
    NEXUS_OBJECT_REGISTER(NEXUS_SimpleAudioPlayback, handle, Create);
    /* now a valid object */

    if (pSettings) {
        rc = NEXUS_SimpleAudioPlayback_SetServerSettings(server, handle, pSettings);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }
    NEXUS_SimpleAudioPlayback_P_GetDefaultSettings(&handle->settings);

    return handle;
    
error:
    NEXUS_SimpleAudioPlayback_Destroy(handle);
    return NULL;
}

static void NEXUS_SimpleAudioPlayback_P_Release( NEXUS_SimpleAudioPlaybackHandle handle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleAudioPlayback, handle);
    NEXUS_OBJECT_UNREGISTER(NEXUS_SimpleAudioPlayback, handle, Destroy);
    return;
}

static void NEXUS_SimpleAudioPlayback_P_Finalizer( NEXUS_SimpleAudioPlaybackHandle handle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleAudioPlayback, handle);

    BLST_S_REMOVE(&handle->server->playbacks, handle, NEXUS_SimpleAudioPlayback, link);
    NEXUS_OBJECT_DESTROY(NEXUS_SimpleAudioPlayback, handle);
    BKNI_Free(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_SimpleAudioPlayback, NEXUS_SimpleAudioPlayback_Destroy);

NEXUS_SimpleAudioPlaybackHandle NEXUS_SimpleAudioPlayback_Acquire(unsigned index)
{
    NEXUS_SimpleAudioPlaybackHandle handle;
    NEXUS_Error rc;

    for (handle=nexus_simple_audio_playback_p_first(); handle; handle = nexus_simple_audio_playback_p_next(handle)) {
        BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
        if (handle->index == index) {
            if (handle->acquired) {
                BERR_TRACE(NEXUS_NOT_AVAILABLE);
                return NULL;
            }
            rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(simpleAudioPlayback,Count,NEXUS_ANY_ID);
            if (rc) { rc = BERR_TRACE(rc); return NULL; }
            
            handle->acquired = true;
            return handle;
        }
    }
    return NULL;
}

void NEXUS_SimpleAudioPlayback_Release( NEXUS_SimpleAudioPlaybackHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
    /* IPC handle validation will only allow this call if handle is owned by client.
    For non-IPC used, acquiring is not required, so acquired boolean is not checked in any other API. */
    if (!handle->acquired) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return;
    }
    if (handle->started) {
        NEXUS_SimpleAudioPlayback_Stop(handle);
    }
    NEXUS_CLIENT_RESOURCES_RELEASE(simpleAudioPlayback,Count,NEXUS_ANY_ID);
    handle->acquired = false;
}

void NEXUS_SimpleAudioPlayback_GetDefaultStartSettings( NEXUS_SimpleAudioPlaybackStartSettings *pSettings )
{
#if NEXUS_HAS_AUDIO
    NEXUS_AudioPlaybackStartSettings start;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_AudioPlayback_GetDefaultStartSettings(&start);

    /* simple decoder can't use NEXUS_AudioPlaybackStartSettings directly because it has Timebase. */
    pSettings->sampleRate = start.sampleRate;
    pSettings->bitsPerSample = start.bitsPerSample;
    pSettings->startThreshold = start.startThreshold;
    pSettings->stereo = start.stereo;
    pSettings->signedData = start.signedData;
    pSettings->loopAround = start.loopAround;
    pSettings->timebase = start.timebase;
    pSettings->endian = start.endian;
#else
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#endif
}

#if NEXUS_HAS_AUDIO
NEXUS_Error nexus_simpleaudioplayback_p_internal_start(NEXUS_SimpleAudioPlaybackHandle handle)
{
    if (handle->clientStarted && !handle->started) {
        const NEXUS_SimpleAudioPlaybackStartSettings *pSettings = &handle->startSettings;
        NEXUS_Error rc;
        if (handle->serverSettings.playback) {
            NEXUS_AudioPlaybackStartSettings start;

            NEXUS_AudioPlayback_GetDefaultStartSettings(&start);
            start.sampleRate = pSettings->sampleRate;
            start.bitsPerSample = pSettings->bitsPerSample;
            start.startThreshold = pSettings->startThreshold;
            start.stereo = pSettings->stereo;
            start.signedData = pSettings->signedData;
            start.loopAround = pSettings->loopAround;
            start.dataCallback = pSettings->dataCallback;
            start.timebase = pSettings->timebase;
            start.endian = pSettings->endian;

            BDBG_MSG(("nexus_simpleaudioplayback_p_internal_start %p", (void *)handle));
            rc = NEXUS_AudioPlayback_Start(handle->serverSettings.playback, &start);
            if (rc) return BERR_TRACE(rc);
            handle->started = true;
        }
        else if (handle->serverSettings.i2sInput) {
            NEXUS_I2sInputSettings settings;
            NEXUS_I2sInput_GetSettings(handle->serverSettings.i2sInput, &settings);
            settings.sampleRate = pSettings->sampleRate;
            settings.bitsPerSample = pSettings->bitsPerSample;
            settings.timebase = pSettings->timebase;
            rc = NEXUS_I2sInput_SetSettings(handle->serverSettings.i2sInput, &settings);
            if (rc) return BERR_TRACE(rc);
            rc = NEXUS_I2sInput_Start(handle->serverSettings.i2sInput);
            if (rc) return BERR_TRACE(rc);
            handle->started = true;
        }
    }
    return 0;
}

void nexus_simpleaudioplayback_p_internal_stop(NEXUS_SimpleAudioPlaybackHandle handle)
{
    if (handle->started) {
        BDBG_MSG(("nexus_simpleaudioplayback_p_internal_stop %p", (void *)handle));
        if (handle->serverSettings.playback) {
            NEXUS_AudioPlayback_Stop(handle->serverSettings.playback);
        }
        else if (handle->serverSettings.i2sInput) {
            NEXUS_I2sInput_Stop(handle->serverSettings.i2sInput);
        }
        handle->started = false;
        handle->suspended = false;
    }
}
#else
NEXUS_Error nexus_simpleaudioplayback_p_internal_start(NEXUS_SimpleAudioPlaybackHandle handle)
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void nexus_simpleaudioplayback_p_internal_stop(NEXUS_SimpleAudioPlaybackHandle handle)
{
    BSTD_UNUSED(handle);
}
#endif

NEXUS_Error NEXUS_SimpleAudioPlayback_Start( NEXUS_SimpleAudioPlaybackHandle handle, const NEXUS_SimpleAudioPlaybackStartSettings *pSettings )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
    if (handle->clientStarted) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    handle->startSettings = *pSettings;
    handle->clientStarted = true;
    rc = nexus_simpleaudioplayback_p_internal_start(handle);
    if (rc) {
        handle->clientStarted = false;
        return BERR_TRACE(rc);
    }
    return 0;
}

void NEXUS_SimpleAudioPlayback_Stop( NEXUS_SimpleAudioPlaybackHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
    handle->clientStarted = false;
    nexus_simpleaudioplayback_p_internal_stop(handle);
}

void NEXUS_SimpleAudioPlayback_GetSettings( NEXUS_SimpleAudioPlaybackHandle playback, NEXUS_SimpleAudioPlaybackSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(playback, NEXUS_SimpleAudioPlayback);
    *pSettings = playback->settings;
}

NEXUS_Error NEXUS_SimpleAudioPlayback_SetSettings( NEXUS_SimpleAudioPlaybackHandle playback, const NEXUS_SimpleAudioPlaybackSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(playback, NEXUS_SimpleAudioPlayback);

#if NEXUS_HAS_AUDIO
    if (playback->serverSettings.playback) {
        NEXUS_Error rc;
        BDBG_CASSERT(sizeof(NEXUS_SimpleAudioPlaybackSettings) == sizeof(NEXUS_AudioPlaybackSettings));
        rc = NEXUS_AudioPlayback_SetSettings(playback->serverSettings.playback, (const NEXUS_AudioPlaybackSettings *)pSettings);
        if (rc) return BERR_TRACE(rc);
    }
#endif

    if (pSettings != &playback->settings) {
        playback->settings = *pSettings;
    }
    return 0;
}

NEXUS_Error NEXUS_SimpleAudioPlayback_GetStatus( NEXUS_SimpleAudioPlaybackHandle handle, NEXUS_SimpleAudioPlaybackStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

#if NEXUS_HAS_AUDIO
    if (handle->serverSettings.playback) {
        NEXUS_AudioPlaybackStatus status;
        NEXUS_AudioPlayback_GetStatus(handle->serverSettings.playback, &status);
    
        pStatus->started = status.started;
        pStatus->queuedBytes = status.queuedBytes;
        pStatus->fifoSize = status.fifoSize;
        pStatus->playedBytes = status.playedBytes;
    }
#endif

    return 0;
}

NEXUS_Error NEXUS_SimpleAudioPlayback_GetBuffer( NEXUS_SimpleAudioPlaybackHandle handle, void **pBuffer, size_t *pSize )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
#if NEXUS_HAS_AUDIO
    if (handle->serverSettings.playback) {
        return NEXUS_AudioPlayback_GetBuffer(handle->serverSettings.playback, pBuffer, pSize);
    }
    else 
#endif
    {
        *pBuffer = NULL;
        *pSize = 0;
        return 0;
    }
}

NEXUS_Error NEXUS_SimpleAudioPlayback_WriteComplete( NEXUS_SimpleAudioPlaybackHandle handle, size_t amountWritten )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
#if NEXUS_HAS_AUDIO
    if (handle->serverSettings.playback) {
        return NEXUS_AudioPlayback_WriteComplete(handle->serverSettings.playback, amountWritten);
    }
    else 
#else
    BSTD_UNUSED(amountWritten);
#endif
    {
        return 0;
    }
}

void NEXUS_SimpleAudioPlayback_Flush( NEXUS_SimpleAudioPlaybackHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
#if NEXUS_HAS_AUDIO
    if (handle->serverSettings.playback) {
        NEXUS_AudioPlayback_Flush(handle->serverSettings.playback);
    }
#endif
}

void NEXUS_SimpleAudioPlayback_GetServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioPlaybackHandle handle, NEXUS_SimpleAudioPlaybackServerSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
    if (server == handle->server) {
        *pSettings = handle->serverSettings;
    }
}

NEXUS_Error NEXUS_SimpleAudioPlayback_SetServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioPlaybackHandle handle, const NEXUS_SimpleAudioPlaybackServerSettings *pSettings )
{
    NEXUS_Error rc;
    
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioPlayback);
    if (server != handle->server) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    
    if (handle->started && (pSettings->playback != handle->serverSettings.playback)) {
        nexus_simpleaudioplayback_p_internal_stop(handle);
    }
    
    handle->serverSettings = *pSettings;
    
    if (handle->serverSettings.playback) {
        NEXUS_SimpleAudioPlayback_SetSettings(handle, &handle->settings);
        if (handle->clientStarted) {
            rc = nexus_simpleaudioplayback_p_internal_start(handle);
            if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        }
    }
    
    return 0;
}

/* return 0 if a suspend was done */
NEXUS_Error nexus_simpleaudioplayback_p_suspend(NEXUS_SimpleAudioPlaybackHandle handle)
{
    BDBG_MSG(("nexus_simpleaudioplayback_p_suspend %p: %d %d", (void*)handle, handle->started, handle->suspended));
    if (handle->started && !handle->suspended) {
        handle->suspended = !NEXUS_AudioPlayback_Suspend(handle->serverSettings.playback);
        if (!handle->suspended) {
            NEXUS_AudioPlayback_Stop(handle->serverSettings.playback);
            handle->started = false;
        }
        return 0;
    }
    else {
        /* no suspend */
        return -1;
    }
}

void nexus_simpleaudioplayback_p_resume(NEXUS_SimpleAudioPlaybackHandle handle)
{
    BDBG_MSG(("nexus_simpleaudioplayback_p_resume %p: %d %d", (void*)handle, handle->started, handle->suspended));
    if (handle->suspended) {
        handle->suspended = false;
        if (!handle->clientStarted) return;
        if (!handle->started) {
            nexus_simpleaudioplayback_p_internal_start(handle);
        }
        NEXUS_AudioPlayback_Resume(handle->serverSettings.playback);
    }
}
