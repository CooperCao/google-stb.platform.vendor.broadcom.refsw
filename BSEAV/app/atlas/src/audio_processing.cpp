/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "audio_processing.h"
#include "atlas_os.h"
#include "nexus_audio_processing_types.h"

BDBG_MODULE(atlas_audio_processing);

CAutoVolumeLevel::CAutoVolumeLevel(CConfiguration * pCfg) :
    CAudioProcessing(pCfg),
    _autoVolumeLevel(NULL)
{
}

CAutoVolumeLevel::~CAutoVolumeLevel()
{
    close();
}

eRet CAutoVolumeLevel::open()
{
    eRet ret = eRet_Ok;
    NEXUS_AutoVolumeLevelSettings settings;

    NEXUS_AutoVolumeLevel_GetDefaultSettings(&settings);
    settings.enabled = false;
    _autoVolumeLevel = NEXUS_AutoVolumeLevel_Open(&settings);
    CHECK_PTR_ERROR_GOTO("unable to open auto volume leveling", _autoVolumeLevel, ret, eRet_OutOfMemory, error);

error:
    return(ret);
}

void CAutoVolumeLevel::close()
{
    if (NULL == _autoVolumeLevel)
    {
        return;
    }

    NEXUS_AutoVolumeLevel_RemoveAllInputs(_autoVolumeLevel);
    NEXUS_AutoVolumeLevel_Close(_autoVolumeLevel);
    _autoVolumeLevel = NULL;
}

NEXUS_AudioInput CAutoVolumeLevel::getConnector()
{
    if (NULL == _autoVolumeLevel)
    {
        return(NULL);
    }

    return(NEXUS_AutoVolumeLevel_GetConnector(_autoVolumeLevel));
}

eRet CAutoVolumeLevel::connect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    if (NULL == _autoVolumeLevel)
    {
        return(eRet_InvalidParameter);
    }

    nError = NEXUS_AutoVolumeLevel_AddInput(_autoVolumeLevel, input);
    CHECK_NEXUS_ERROR_GOTO("unable to connect input to auto volume level", ret, nError, error);

error:
    return(ret);
}

eRet CAutoVolumeLevel::disconnect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    if (NULL == _autoVolumeLevel)
    {
        return(eRet_InvalidParameter);
    }

    if (NULL == input)
    {
        nError = NEXUS_AutoVolumeLevel_RemoveAllInputs(_autoVolumeLevel);
        CHECK_NEXUS_ERROR_GOTO("unable to remove all inputs", ret, nError, error);
    }
    else
    {
        NEXUS_AutoVolumeLevel_RemoveInput(_autoVolumeLevel, input);
        CHECK_NEXUS_ERROR_GOTO("unable to remove input", ret, nError, error);
    }

error:
    return(ret);
} /* disconnect */

eRet CAutoVolumeLevel::enable(bool bEnable)
{
    eRet                          ret    = eRet_Ok;
    NEXUS_Error                   nError = NEXUS_SUCCESS;
    NEXUS_AutoVolumeLevelSettings settings;

    if (NULL == _autoVolumeLevel)
    {
        return(eRet_InvalidParameter);
    }

    NEXUS_AutoVolumeLevel_GetSettings(_autoVolumeLevel, &settings);
    settings.enabled = bEnable;
    nError           = NEXUS_AutoVolumeLevel_SetSettings(_autoVolumeLevel, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set auto volume level", ret, nError, error);

error:
    return(ret);
} /* enable */

CDolbyVolume::CDolbyVolume(CConfiguration * pCfg) :
    CAudioProcessing(pCfg),
    _dolbyVolume(NULL)
{
}

CDolbyVolume::~CDolbyVolume()
{
    close();
}

eRet CDolbyVolume::open()
{
    eRet ret = eRet_Ok;
    NEXUS_DolbyVolume258Settings settings;

    NEXUS_DolbyVolume258_GetDefaultSettings(&settings);
    settings.enabled = false;
    _dolbyVolume     = NEXUS_DolbyVolume258_Open(&settings);
    CHECK_PTR_ERROR_GOTO("unable to open dolby volume", _dolbyVolume, ret, eRet_OutOfMemory, error);

error:
    return(ret);
}

void CDolbyVolume::close()
{
    if (NULL == _dolbyVolume)
    {
        return;
    }

    NEXUS_DolbyVolume258_RemoveAllInputs(_dolbyVolume);
    NEXUS_DolbyVolume258_Close(_dolbyVolume);
    _dolbyVolume = NULL;
}

NEXUS_AudioInput CDolbyVolume::getConnector()
{
    if (NULL == _dolbyVolume)
    {
        return(NULL);
    }

    return(NEXUS_DolbyVolume258_GetConnector(_dolbyVolume));
}

eRet CDolbyVolume::connect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    if (NULL == _dolbyVolume)
    {
        return(eRet_InvalidParameter);
    }

    nError = NEXUS_DolbyVolume258_AddInput(_dolbyVolume, input);
    CHECK_NEXUS_ERROR_GOTO("unable to connect input to dolby volume", ret, nError, error);

error:
    return(ret);
}

eRet CDolbyVolume::disconnect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    if (NULL == _dolbyVolume)
    {
        return(eRet_InvalidParameter);
    }

    if (NULL == input)
    {
        nError = NEXUS_DolbyVolume258_RemoveAllInputs(_dolbyVolume);
        CHECK_NEXUS_ERROR_GOTO("unable to remove all inputs", ret, nError, error);
    }
    else
    {
        NEXUS_DolbyVolume258_RemoveInput(_dolbyVolume, input);
        CHECK_NEXUS_ERROR_GOTO("unable to remove input", ret, nError, error);
    }

error:
    return(ret);
} /* disconnect */

eRet CDolbyVolume::enable(bool bEnable)
{
    eRet                         ret    = eRet_Ok;
    NEXUS_Error                  nError = NEXUS_SUCCESS;
    NEXUS_DolbyVolume258Settings settings;

    if (NULL == _dolbyVolume)
    {
        return(eRet_InvalidParameter);
    }

    NEXUS_DolbyVolume258_GetSettings(_dolbyVolume, &settings);
    settings.enabled = bEnable;
    nError           = NEXUS_DolbyVolume258_SetSettings(_dolbyVolume, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set dolby volume", ret, nError, error);

error:
    return(ret);
} /* enable */

CTruVolume::CTruVolume(CConfiguration * pCfg) :
    CAudioProcessing(pCfg),
    _truVolume(NULL)
{
}

CTruVolume::~CTruVolume()
{
    close();
}

eRet CTruVolume::open()
{
    eRet                    ret = eRet_Ok;
    NEXUS_TruVolumeSettings settings;

    NEXUS_TruVolume_GetDefaultSettings(&settings);
    settings.enabled = false;
    _truVolume       = NEXUS_TruVolume_Open(&settings);
    CHECK_PTR_ERROR_GOTO("unable to open truvolume", _truVolume, ret, eRet_OutOfMemory, error);

error:
    return(ret);
}

void CTruVolume::close()
{
    if (NULL == _truVolume)
    {
        return;
    }

    NEXUS_TruVolume_RemoveAllInputs(_truVolume);
    NEXUS_TruVolume_Close(_truVolume);
    _truVolume = NULL;
}

NEXUS_AudioInput CTruVolume::getConnector()
{
    if (NULL == _truVolume)
    {
        return(NULL);
    }

    return(NEXUS_TruVolume_GetConnector(_truVolume));
}

eRet CTruVolume::connect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    if (NULL == _truVolume)
    {
        return(eRet_InvalidParameter);
    }

    nError = NEXUS_TruVolume_AddInput(_truVolume, input);
    CHECK_NEXUS_ERROR_GOTO("unable to connect input to truvolume", ret, nError, error);

error:
    return(ret);
}

eRet CTruVolume::disconnect(NEXUS_AudioInput input)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nError = NEXUS_SUCCESS;

    if (NULL == _truVolume)
    {
        return(eRet_InvalidParameter);
    }

    if (NULL == input)
    {
        nError = NEXUS_TruVolume_RemoveAllInputs(_truVolume);
        CHECK_NEXUS_ERROR_GOTO("unable to remove all inputs", ret, nError, error);
    }
    else
    {
        NEXUS_TruVolume_RemoveInput(_truVolume, input);
        CHECK_NEXUS_ERROR_GOTO("unable to remove input", ret, nError, error);
    }

error:
    return(ret);
} /* disconnect */

eRet CTruVolume::enable(bool bEnable)
{
    eRet                    ret    = eRet_Ok;
    NEXUS_Error             nError = NEXUS_SUCCESS;
    NEXUS_TruVolumeSettings settings;

    if (NULL == _truVolume)
    {
        return(eRet_InvalidParameter);
    }

    NEXUS_TruVolume_GetSettings(_truVolume, &settings);
    settings.enabled = bEnable;
    nError           = NEXUS_TruVolume_SetSettings(_truVolume, &settings);
    CHECK_NEXUS_ERROR_GOTO("unable to set truvolume", ret, nError, error);

error:
    return(ret);
} /* enable */