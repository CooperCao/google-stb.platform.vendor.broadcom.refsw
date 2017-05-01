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
* API Description:
*   API name: DolbyDigitalReencode
*    Specific APIs related to Dolby Digital Reencoding used in Dolby MS11
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_dolby_digital_reencode);

typedef struct NEXUS_DolbyDigitalReencode
{
    NEXUS_OBJECT(NEXUS_DolbyDigitalReencode);
    /*NEXUS_AudioInputObject connector;*/
    NEXUS_DolbyDigitalReencodeSettings settings;
    NEXUS_AudioInputObject connectors[NEXUS_AudioConnectorType_eMax];
    NEXUS_AudioInputHandle input;
    BAPE_DolbyDigitalReencodeHandle apeHandle;
    char name[5];   /* DDRE */
} NEXUS_DolbyDigitalReencode;


void NEXUS_DolbyDigitalReencode_GetDefaultSettings(
    NEXUS_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    )
{
    BAPE_DolbyDigitalReencodeSettings piSettings;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BAPE_DolbyDigitalReencode_GetDefaultSettings(&piSettings);
    pSettings->encodeSettings.spdifHeaderEnabled = piSettings.encodeSettings.spdifHeaderEnabled;
    pSettings->encodeSettings.certificationMode = piSettings.encodeSettings.certificationMode;
    pSettings->loudnessEquivalenceEnabled = piSettings.loudnessEquivalenceEnabled;
    pSettings->externalPcmMode = piSettings.externalPcmMode;
    BDBG_CASSERT((int)NEXUS_DolbyDigitalReencodeProfile_eMax == (int)BAPE_DolbyDigitalReencodeProfile_eMax);
    pSettings->profile = piSettings.profile;
    pSettings->centerMixLevel = piSettings.centerMixLevel;      /* These are defined by the AC3 spec and don't require CASSERT. */
    pSettings->surroundMixLevel = piSettings.surroundMixLevel;  /* These are defined by the AC3 spec and don't require CASSERT. */
    pSettings->dolbySurround = piSettings.dolbySurround;        /* These are defined by the AC3 spec and don't require CASSERT. */
    pSettings->dialogLevel = piSettings.dialogLevel;
    pSettings->drcMode = piSettings.drcMode == BAPE_DolbyDigitalReencodeDrcMode_eLine?NEXUS_AudioDecoderDolbyDrcMode_eLine:NEXUS_AudioDecoderDolbyDrcMode_eRf;
    pSettings->drcModeDownmix = piSettings.drcModeDownmix == BAPE_DolbyDigitalReencodeDrcMode_eLine?NEXUS_AudioDecoderDolbyDrcMode_eLine:NEXUS_AudioDecoderDolbyDrcMode_eRf;
    pSettings->cut = piSettings.drcScaleHi;
    pSettings->boost = piSettings.drcScaleLow;
    pSettings->stereoDownmixMode = piSettings.stereoMode;
    BDBG_CASSERT((int)NEXUS_DolbyDigitalReencodeDownmixMode_eMax == (int)BAPE_DolbyDigitalReencodeStereoMode_eMax);
    pSettings->dualMonoMode = piSettings.dualMonoMode;
    BDBG_CASSERT((int)NEXUS_AudioDecoderDualMonoMode_eMax == (int)BAPE_DualMonoMode_eMax);
    pSettings->multichannelFormat = (piSettings.multichannelFormat == BAPE_MultichannelFormat_e7_1) ? NEXUS_AudioMultichannelFormat_e7_1 : NEXUS_AudioMultichannelFormat_e5_1;
    pSettings->fixedEncoderFormat = piSettings.fixedEncoderFormat;
}

NEXUS_DolbyDigitalReencodeHandle NEXUS_DolbyDigitalReencode_Open(
    const NEXUS_DolbyDigitalReencodeSettings *pSettings
    )
{
    BAPE_DolbyDigitalReencodeSettings piSettings;
    NEXUS_DolbyDigitalReencodeSettings defaults;
    NEXUS_DolbyDigitalReencodeHandle handle;
    BAPE_Connector connector;
    BERR_Code errCode;

    NEXUS_DolbyDigitalReencode_GetDefaultSettings(&defaults);
    if ( NULL == pSettings )
    {
        pSettings = &defaults;
    }

    if ( pSettings->multichannelFormat == NEXUS_AudioMultichannelFormat_e7_1 &&
         defaults.multichannelFormat != NEXUS_AudioMultichannelFormat_e7_1 )
    {
        BDBG_ERR(("The Current System configuration does not appear to support 7.1 output"));
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    handle = BKNI_Malloc(sizeof(NEXUS_DolbyDigitalReencode));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BKNI_Memset(handle, 0, sizeof(NEXUS_DolbyDigitalReencode));
    NEXUS_OBJECT_INIT(NEXUS_DolbyDigitalReencode, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "DDRE");

    BAPE_DolbyDigitalReencode_GetDefaultSettings(&piSettings);
    if ( g_NEXUS_audioModuleData.armHandle && !NEXUS_GetEnv("disable_arm_audio") )
    {
        piSettings.encoderDeviceIndex = 6;
        piSettings.encoderMixerRequired = false;
        piSettings.encoderTaskRequired = true;
    }
    else if ( NEXUS_GetEnv("ms12_dual_raaga") )
    {
        piSettings.encoderDeviceIndex = 1;
        piSettings.encoderMixerRequired = true;
        piSettings.encoderTaskRequired = true;
    }

    if ( pSettings->multichannelFormat == NEXUS_AudioMultichannelFormat_e7_1 )
    {
        piSettings.multichannelFormat = BAPE_MultichannelFormat_e7_1;
    }
    else
    {
        piSettings.multichannelFormat = BAPE_MultichannelFormat_e5_1;
    }
    piSettings.fixedEncoderFormat = pSettings->fixedEncoderFormat;
    errCode = BAPE_DolbyDigitalReencode_Create(NEXUS_AUDIO_DEVICE_HANDLE, &piSettings, &handle->apeHandle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_ape_handle;
    }

    NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_AudioConnectorType_eStereo], NEXUS_AudioInputType_eDolbyDigitalReencode, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eStereo], Open);
    handle->connectors[NEXUS_AudioConnectorType_eStereo].pName = handle->name;
    handle->connectors[NEXUS_AudioConnectorType_eStereo].format = NEXUS_AudioInputFormat_ePcmStereo;
    BAPE_DolbyDigitalReencode_GetConnector(handle->apeHandle, BAPE_ConnectorFormat_eStereo, &connector);
    handle->connectors[NEXUS_AudioConnectorType_eStereo].port = (size_t)connector;
    NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], NEXUS_AudioInputType_eDolbyDigitalReencode, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eMultichannel], Open);
    handle->connectors[NEXUS_AudioConnectorType_eMultichannel].pName = handle->name;
    if ( pSettings->multichannelFormat == NEXUS_AudioMultichannelFormat_e7_1 )
    {
        handle->connectors[NEXUS_AudioConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm7_1;
    }
    else
    {
        handle->connectors[NEXUS_AudioConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm5_1;
    }
    BAPE_DolbyDigitalReencode_GetConnector(handle->apeHandle, BAPE_ConnectorFormat_eMultichannel, &connector);
    handle->connectors[NEXUS_AudioConnectorType_eMultichannel].port = (size_t)connector;
    NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_AudioConnectorType_eCompressed], NEXUS_AudioInputType_eDolbyDigitalReencode, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eCompressed], Open);
    handle->connectors[NEXUS_AudioConnectorType_eCompressed].pName = handle->name;
    handle->connectors[NEXUS_AudioConnectorType_eCompressed].format = NEXUS_AudioInputFormat_eCompressed;
    BAPE_DolbyDigitalReencode_GetConnector(handle->apeHandle, BAPE_ConnectorFormat_eCompressed, &connector);
    handle->connectors[NEXUS_AudioConnectorType_eCompressed].port = (size_t)connector;
    if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_AudioConnectorType_eCompressed4x], NEXUS_AudioInputType_eDolbyDigitalReencode, handle);
        NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connectors[NEXUS_AudioConnectorType_eCompressed4x], Open);
        handle->connectors[NEXUS_AudioConnectorType_eCompressed4x].pName = handle->name;
        handle->connectors[NEXUS_AudioConnectorType_eCompressed4x].format = NEXUS_AudioInputFormat_eCompressed;
        BAPE_DolbyDigitalReencode_GetConnector(handle->apeHandle, BAPE_ConnectorFormat_eCompressed4x, &connector);
        handle->connectors[NEXUS_AudioConnectorType_eCompressed4x].port = (size_t)connector;
    }

    errCode = NEXUS_DolbyDigitalReencode_SetSettings(handle, pSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_settings;
    }

    return handle;

err_settings:
    BAPE_DolbyDigitalReencode_Destroy(handle->apeHandle);
    BDBG_OBJECT_DESTROY(handle, NEXUS_DolbyDigitalReencode);
err_ape_handle:
    BKNI_Free(handle);
err_malloc:
    return NULL;
}

static void NEXUS_DolbyDigitalReencode_P_Finalizer(
    NEXUS_DolbyDigitalReencodeHandle handle
    )
{
    unsigned i;
    NEXUS_OBJECT_ASSERT(NEXUS_DolbyDigitalReencode, handle);
    for ( i = 0; i < NEXUS_AudioConnectorType_eMax; i++ )
    {
        switch ( i )
        {
        case NEXUS_AudioConnectorType_eStereo:
        case NEXUS_AudioConnectorType_eMultichannel:
        case NEXUS_AudioConnectorType_eCompressed:
            NEXUS_AudioInput_Shutdown(&handle->connectors[i]);
            NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connectors[i], Close);
            break;
        case NEXUS_AudioConnectorType_eCompressed4x:
            if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
            {
                NEXUS_AudioInput_Shutdown(&handle->connectors[i]);
                NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connectors[i], Close);
            }
            break;
        default:
            break;
        }
    }
    BAPE_DolbyDigitalReencode_Destroy(handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_DolbyDigitalReencode, handle);
    BKNI_Free(handle);
}

static void NEXUS_DolbyDigitalReencode_P_Release(NEXUS_DolbyDigitalReencodeHandle handle)
{
    unsigned i;
    for ( i = 0; i < NEXUS_AudioConnectorType_eMax; i++ )
    {
        switch ( i )
        {
        case NEXUS_AudioConnectorType_eStereo:
        case NEXUS_AudioConnectorType_eMultichannel:
        case NEXUS_AudioConnectorType_eCompressed:
            NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connectors[i], Close);
            break;
        case NEXUS_AudioConnectorType_eCompressed4x:
            if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
            {
                NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connectors[i], Close);
            }
            break;
        default:
            break;
        }
    }
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_DolbyDigitalReencode, NEXUS_DolbyDigitalReencode_Close);

void NEXUS_DolbyDigitalReencode_GetSettings(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_DolbyDigitalReencode_SetSettings(
    NEXUS_DolbyDigitalReencodeHandle handle,
    const NEXUS_DolbyDigitalReencodeSettings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_DolbyDigitalReencodeSettings piSettings;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_DolbyDigitalReencode_GetSettings(handle->apeHandle, &piSettings);

    piSettings.encodeSettings.spdifHeaderEnabled = pSettings->encodeSettings.spdifHeaderEnabled;
    piSettings.encodeSettings.certificationMode = pSettings->encodeSettings.certificationMode;
    piSettings.loudnessEquivalenceEnabled = pSettings->loudnessEquivalenceEnabled;
    piSettings.externalPcmMode = pSettings->externalPcmMode;
    piSettings.profile = pSettings->profile;
    piSettings.centerMixLevel = pSettings->centerMixLevel;
    piSettings.surroundMixLevel = pSettings->surroundMixLevel;
    piSettings.dolbySurround = pSettings->dolbySurround;
    piSettings.dialogLevel = pSettings->dialogLevel;
    piSettings.drcMode = (pSettings->drcMode == NEXUS_AudioDecoderDolbyDrcMode_eRf) ? BAPE_DolbyDigitalReencodeDrcMode_eRf : BAPE_DolbyDigitalReencodeDrcMode_eLine;
    piSettings.drcModeDownmix = (pSettings->drcModeDownmix == NEXUS_AudioDecoderDolbyDrcMode_eRf) ? BAPE_DolbyDigitalReencodeDrcMode_eRf : BAPE_DolbyDigitalReencodeDrcMode_eLine;;
    piSettings.drcScaleHi = pSettings->cut;
    piSettings.drcScaleLow = pSettings->boost;
    piSettings.drcScaleHiDownmix = pSettings->cut;
    piSettings.drcScaleLowDownmix = pSettings->boost;
    piSettings.stereoMode = pSettings->stereoDownmixMode;
    piSettings.dualMonoMode = pSettings->dualMonoMode;
    piSettings.fixedEncoderFormat = pSettings->fixedEncoderFormat;

    errCode = BAPE_DolbyDigitalReencode_SetSettings(handle->apeHandle, &piSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
}

NEXUS_AudioInputHandle NEXUS_DolbyDigitalReencode_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioConnectorType connectorType
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);

    if (BAPE_GetDolbyMSVersion() != BAPE_DolbyMSVersion_eMS12 &&
        BAPE_GetDolbyMSVersion() != BAPE_DolbyMSVersion_eMS11)
    {
        BDBG_ERR(("Connector %u not available. MS11 or MS12 is required to use DDRE", connectorType));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    switch ( connectorType )
    {
    case NEXUS_AudioConnectorType_eStereo:
    case NEXUS_AudioConnectorType_eMultichannel:
    case NEXUS_AudioConnectorType_eCompressed:
        break;
    case NEXUS_AudioConnectorType_eCompressed4x:
        if (BAPE_GetDolbyMSVersion() != BAPE_DolbyMSVersion_eMS12)
        {
            BDBG_ERR(("MS12 does not appear to be enabled and is required to use 'eCompressed4x' Connector"));
            (void)BERR_TRACE(BERR_INVALID_PARAMETER);
            return NULL;
        }
        else
        {
            BAPE_Capabilities caps;
            BAPE_GetCapabilities(g_NEXUS_audioModuleData.apeHandle, &caps);
            /* check for Config C */
            if ( !caps.dsp.codecs[BAVC_AudioCompressionStd_eAc3Plus].encode )
            {
                BDBG_ERR(("The current MS12 Config does not appear to support 'eCompressed4x' Connector"));
                (void)BERR_TRACE(BERR_INVALID_PARAMETER);
                return NULL;
            }
        }
        break;
    default:
        BDBG_ERR(("Unsupported connector type %u", connectorType));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }

    return &handle->connectors[connectorType];
}

NEXUS_Error NEXUS_DolbyDigitalReencode_AddInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);

    if ( handle->input )
    {
        BDBG_ERR(("Already connected to another input source"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Add to the PI */
    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    if (input->objectType != NEXUS_AudioInputType_eDspMixer &&
        input->objectType != NEXUS_AudioInputType_eDolbyVolume258 &&
        input->objectType != NEXUS_AudioInputType_eAudioProcessor)
    {
        BDBG_ERR(("input type %d (%s) is not a valid input type for DDRE.", input->objectType, input->pName));
        BDBG_ERR(("    DDRE requires either DspMixer->DDRE or DspMixer->DV258->DDRE"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    errCode = BAPE_DolbyDigitalReencode_AddInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_ape;
    }
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[NEXUS_AudioConnectorType_eStereo], input);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stereo;
    }
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], input);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_multichannel;
    }
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[NEXUS_AudioConnectorType_eCompressed], input);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_compressed;
    }
    if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[NEXUS_AudioConnectorType_eCompressed4x], input);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_compressed4x;
        }
    }
    handle->input = input;

    return BERR_SUCCESS;

err_compressed4x:
    NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eCompressed], input);
err_compressed:
    NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], input);
err_multichannel:
    NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eStereo], input);
err_stereo:
    BAPE_DolbyDigitalReencode_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
err_ape:
    return errCode;
}

NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_OBJECT_ASSERT(input, NEXUS_AudioInput);

    if ( input != handle->input )
    {
        BDBG_ERR(("Not connected to input %p", (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eCompressed4x], input);
    }
    NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eCompressed], input);
    NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], input);
    NEXUS_AudioInput_P_RemoveInput(&handle->connectors[NEXUS_AudioConnectorType_eStereo], input);
    BAPE_DolbyDigitalReencode_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
    handle->input = NULL;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveAllInputs(
    NEXUS_DolbyDigitalReencodeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    if ( handle->input )
    {
        return NEXUS_DolbyDigitalReencode_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}
