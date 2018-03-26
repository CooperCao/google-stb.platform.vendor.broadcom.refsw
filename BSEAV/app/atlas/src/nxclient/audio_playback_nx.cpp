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

#include "nexus_simple_audio_playback.h"
#include "audio_playback_nx.h"
#include "atlas_os.h"
#include "nxclient.h"

BDBG_MODULE(atlas_pcm_playback);

#define CONVERT_TO_NEXUS_LINEAR_VOL(vol, max_vol)  (((vol) * (NEXUS_AUDIO_VOLUME_LINEAR_NORMAL - NEXUS_AUDIO_VOLUME_LINEAR_MIN) / (max_vol)) + NEXUS_AUDIO_VOLUME_LINEAR_MIN)

CSimplePcmPlaybackNx::CSimplePcmPlaybackNx(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CSimplePcmPlayback(name, number, pCfg)
{
}

eRet CSimplePcmPlaybackNx::open(CWidgetEngine * pWidgetEngine)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;
    int         i      = 0;

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

    _simplePlayback = NEXUS_SimpleAudioPlayback_Acquire(getNumber());
    CHECK_PTR_ERROR_GOTO("unable to acquire simple audio playback", _simplePlayback, ret, eRet_NotAvailable, error);

    goto done;
error:
    close();
done:
    return(ret);
} /* open */

void CSimplePcmPlaybackNx::close()
{
    if (true == _bRun)
    {
        stop();
    }

    if (NULL != _simplePlayback)
    {
        NEXUS_SimpleAudioPlayback_Release(_simplePlayback);
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

    _pWidgetEngine = NULL;
} /* close */

eRet CSimplePcmPlaybackNx::connect(unsigned index)
{
    eRet                     ret    = eRet_Ok;
    NEXUS_Error              nerror = NEXUS_SUCCESS;
    NxClient_ConnectSettings connectSettings;

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleAudioPlayback[index].id = getNumber();

    nerror = NxClient_Connect(&connectSettings, &_connectId);
    CHECK_NEXUS_ERROR_GOTO("unable to connect simple pcm playback", nerror, ret, error);

error:
    return(ret);
}

void CSimplePcmPlaybackNx::disconnect()
{
    if (0 == _connectId)
    {
        return;
    }

    NxClient_Disconnect(_connectId);
}