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

#include "stc.h"

BDBG_MODULE(atlas_stc);

CStc::CStc(
        const char *     name,
        const unsigned   number,
        CConfiguration * pCfg
        ) :
    CResource(name, number, eBoardResource_stcChannel, pCfg),
    _simpleStcChannel(NULL),
    _stcChannel(NULL),
    _transportType(NEXUS_TransportType_eUnknown),
    _stcType(eStcType_ParserBand),
    _pPcrPid(NULL)
{
}

CStc::~CStc()
{
}

eRet CStc::open(NEXUS_StcChannelSettings settings)
{
    eRet ret = eRet_Ok;

    if (true == GET_BOOL(_pCfg, USE_FIRST_PTS))
    {
        BDBG_WRN(("PTS auto behavior default %d,changing to %d",
                  settings.modeSettings.Auto.behavior,
                  NEXUS_StcChannelAutoModeBehavior_eFirstAvailable));
        settings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;
    }

    _stcChannel = NEXUS_StcChannel_Open(_number, &settings);
    CHECK_PTR_ERROR_GOTO("Stc channel open failure", _stcChannel, ret, eRet_ExternalError, error);

    _simpleStcChannel = NULL;

error:
    return(ret);
} /* open */

eRet CStc::open()
{
    eRet ret = eRet_Ok;
    NEXUS_SimpleStcChannelSettings settings;

    getDefaultSettings(&settings);

    if (true == GET_BOOL(_pCfg, USE_FIRST_PTS))
    {
        BDBG_WRN(("PTS auto behavior default %d,changing to %d",
                  settings.modeSettings.Auto.behavior,
                  NEXUS_StcChannelAutoModeBehavior_eFirstAvailable));
        settings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;
    }

    _simpleStcChannel = NEXUS_SimpleStcChannel_Create(&settings);
    CHECK_PTR_ERROR_GOTO("Stc channel open failure", _simpleStcChannel, ret, eRet_ExternalError, error);

error:
    return(ret);
} /* open */

void CStc::close()
{
    if (NULL != _simpleStcChannel)
    {
        NEXUS_SimpleStcChannel_Destroy(_simpleStcChannel);
        _simpleStcChannel = NULL;
    }

    if (NULL != _stcChannel)
    {
        NEXUS_StcChannel_Close(_stcChannel);
        _stcChannel = NULL;
    }
}

void CStc::getDefaultSettings(NEXUS_SimpleStcChannelSettings * pSettings)
{
    BDBG_ASSERT(NULL != pSettings);

    NEXUS_SimpleStcChannel_GetDefaultSettings(pSettings);
    pSettings->astm = GET_BOOL(_pCfg, ASTM_ENABLED);

    if (true == pSettings->astm)
    {
        /* prefer audio master mode for ASTM */
        pSettings->modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eAudioMaster;
    }
}

eRet CStc::configure(CPid * pPcrPid)
{
    eRet                           ret    = eRet_Ok;
    NEXUS_Error                    nerror = NEXUS_SUCCESS;
    NEXUS_SimpleStcChannelSettings settings;

    getDefaultSettings(&settings);
    switch (_stcType)
    {
    case eStcType_ParserBand:
        if (NULL == pPcrPid)
        {
            BDBG_ERR((" PCR is NULL"));
            ret = eRet_InvalidParameter;
            goto error;
        }
        settings.mode                         = NEXUS_StcChannelMode_ePcr; /* live */
        settings.modeSettings.pcr.pidChannel  = pPcrPid->getPidChannel();
        settings.modeSettings.highJitter.mode = NEXUS_SimpleStcChannelHighJitterMode_eNone;
        _pPcrPid                              = pPcrPid;
        BDBG_MSG(("eStcType_ParserBand "));
        break;
    case eStcType_ParserBandSyncOff:
        if (NULL == pPcrPid)
        {
            BDBG_ERR((" PCR is NULL"));
            ret = eRet_InvalidParameter;
            goto error;
        }
        settings.mode                         = NEXUS_StcChannelMode_ePcr; /* live */
        settings.sync                         = NEXUS_SimpleStcChannelSyncMode_eOff;
        settings.modeSettings.pcr.pidChannel  = pPcrPid->getPidChannel();
        settings.modeSettings.highJitter.mode = NEXUS_SimpleStcChannelHighJitterMode_eNone;
        _pPcrPid                              = pPcrPid;
        BDBG_MSG(("eStcType_ParserBandSyncOff "));
        break;
    case eStcType_TtsPacing:
        if (pPcrPid == NULL)
        {
            BDBG_ERR((" PCR is NULL"));
            ret = eRet_InvalidParameter;
            goto error;
        }
        settings.modeSettings.pcr.disableJitterAdjustment    = true;
        settings.modeSettings.pcr.disableTimestampCorrection = true;
        settings.mode                            = NEXUS_StcChannelMode_ePcr;
        settings.modeSettings.pcr.pidChannel     = pPcrPid->getPidChannel();
        settings.modeSettings.highJitter.mode    = NEXUS_SimpleStcChannelHighJitterMode_eNone;
        settings.modeSettings.Auto.transportType = _transportType;
        _pPcrPid = pPcrPid;

        BDBG_ERR(("eStcType_TtsPacing"));
        break;
    case eStcType_PcrPacing:
        BDBG_ASSERT("not supported yet");
        break;

    case eStcType_IpLivePlayback:
        if (pPcrPid == NULL)
        {
            BDBG_ERR((" PCR is NULL"));
            ret = eRet_InvalidParameter;
            goto error;
        }
        settings.mode                              = NEXUS_StcChannelMode_ePcr;
        settings.modeSettings.pcr.pidChannel       = pPcrPid->getPidChannel();
        settings.modeSettings.highJitter.threshold = GET_INT(_pCfg, IP_NETWORK_MAX_JITTER);
        settings.modeSettings.highJitter.mode      = NEXUS_SimpleStcChannelHighJitterMode_eDirect;
        settings.modeSettings.Auto.transportType   = _transportType;
        _pPcrPid = pPcrPid;

        BDBG_MSG(("eStcType_IpLivePlayback"));
        break;

    case eStcType_IpPullPlayback:
    /* Same as PvrPlayback */
    case eStcType_PvrPlayback:
        settings.mode                            = NEXUS_StcChannelMode_eAuto;
        settings.modeSettings.Auto.transportType = _transportType;
        if (pPcrPid != NULL)
        {
            _pPcrPid = pPcrPid;
        }
        else
        {
            _pPcrPid = NULL;
        }

        break;

    default:
        BDBG_ASSERT("not supported yet");
        break;
    } /* switch */

    nerror = NEXUS_SimpleStcChannel_SetSettings(_simpleStcChannel, &settings);
    CHECK_NEXUS_ERROR_GOTO("error setting stc channel settings", ret, nerror, error);

    BDBG_MSG(("Successful Call to %s", BSTD_FUNCTION));
    return(ret);

error:

    BDBG_ERR((" Reset STC Channel settings to Default"));
    /* Set Channel to default settings*/
    getDefaultSettings(&settings);
    nerror = NEXUS_SimpleStcChannel_SetSettings(_simpleStcChannel, &settings);
    BDBG_ERR(("error re-setting stc channel settings stc %p ", (void *)_simpleStcChannel));

    return(ret);
} /* configure */

eRet CStc::setSettings(NEXUS_SimpleStcChannelSettings * settings)
{
    eRet        ret    = eRet_Ok;
    NEXUS_Error nerror = NEXUS_SUCCESS;

    nerror = NEXUS_SimpleStcChannel_SetSettings(_simpleStcChannel, settings);
    CHECK_NEXUS_ERROR_GOTO("error setting stc channel settings", ret, nerror, error);

    BDBG_MSG(("Successful Call to %s", BSTD_FUNCTION));
    return(ret);

error:

    BDBG_ERR((" Reset STC Channel settings to Default"));
    /* Set Channel to default settings*/
    getDefaultSettings(settings);
    nerror = NEXUS_SimpleStcChannel_SetSettings(_simpleStcChannel, settings);
    BDBG_ERR(("error re-setting stc channel settings stc %p ", (void *)_simpleStcChannel));

    return(ret);
} /* setSettings */

/* configure */