/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: DolbyDigitalReencode
*    Specific APIs related to Dolby Digital Reencoding used in Dolby MS11
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_dolby_digital_reencode);

static BLST_S_HEAD(StageList, NEXUS_DolbyDigitalReencode) g_stageList;

BDBG_OBJECT_ID(NEXUS_DolbyDigitalReencode);
typedef struct NEXUS_DolbyDigitalReencode
{
    BDBG_OBJECT(NEXUS_DolbyDigitalReencode)
    BLST_S_ENTRY(NEXUS_DolbyDigitalReencode) node;
    NEXUS_AudioInputObject connectors[NEXUS_DolbyDigitalReencodeConnectorType_eMax];
    NEXUS_DolbyDigitalReencodeSettings settings;
    NEXUS_AudioInput input;
    BRAP_ProcessingStageHandle ddre, encoder;
} NEXUS_DolbyDigitalReencode;

void NEXUS_DolbyDigitalReencode_GetDefaultSettings(
    NEXUS_DolbyDigitalReencodeSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eDDRE, &g_NEXUS_StageSettings);
    NEXUS_Ac3Encode_GetDefaultSettings(&pSettings->encodeSettings);

    pSettings->dualMonoMode = NEXUS_AudioDecoderDualMonoMode_eStereo;
    pSettings->outputMode = NEXUS_AudioDecoderOutputMode_eAuto;
    pSettings->outputLfeMode = NEXUS_AudioDecoderOutputLfeMode_eAuto;

    switch ( g_NEXUS_StageSettings.uConfigParams.sDDREParams.eCompMode )
    {
    case BRAP_DSPCHN_DDRE_CompMode_eLine:
        pSettings->drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
        break;
    default:
    case BRAP_DSPCHN_DDRE_CompMode_eRf:
        pSettings->drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
        break;
    }
    switch ( g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDownmixType )
    {
    default:
    case BRAP_DSPCHN_DDRE_DownmixType_eLtRt:
        pSettings->stereoDownmixMode = NEXUS_DolbyDigitalReencodeDownmixMode_eLtRt;
        break;
    case BRAP_DSPCHN_DDRE_DownmixType_eLoRo:
        pSettings->stereoDownmixMode = NEXUS_DolbyDigitalReencodeDownmixMode_eLoRo;
        break;
    case BRAP_DSPCHN_DDRE_DownmixType_eArib:
        pSettings->stereoDownmixMode = NEXUS_DolbyDigitalReencodeDownmixMode_eArib;
        break;
    }
    pSettings->cut = g_NEXUS_StageSettings.uConfigParams.sDDREParams.ui32DrcCutFac;
    pSettings->boost = g_NEXUS_StageSettings.uConfigParams.sDDREParams.ui32DrcBoostFac;

    pSettings->externalPcmMode = g_NEXUS_StageSettings.uConfigParams.sDDREParams.bExternalPcmModeEnabled;    
    pSettings->certificationMode = g_NEXUS_StageSettings.uConfigParams.sDDREParams.bCertificationModeEnabled;
    switch ( g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile )
    {
    case BRAP_DSPCHN_DDREProfile_eNoCompression:            pSettings->profile = NEXUS_DolbyDigitalReencodeProfile_eNoCompression; break;
    default:
    case BRAP_DSPCHN_DDREProfile_eFilmStandardCompression:  pSettings->profile = NEXUS_DolbyDigitalReencodeProfile_eFilmStandardCompression; break;
    case BRAP_DSPCHN_DDREProfile_eFilmLightCompression:     pSettings->profile = NEXUS_DolbyDigitalReencodeProfile_eFilmLightCompression; break;
    case BRAP_DSPCHN_DDREProfile_eMusicStandardCompression: pSettings->profile = NEXUS_DolbyDigitalReencodeProfile_eMusicStandardCompression; break;
    case BRAP_DSPCHN_DDREProfile_eMusicLightCompression:    pSettings->profile = NEXUS_DolbyDigitalReencodeProfile_eMusicLightCompression; break;
    case BRAP_DSPCHN_DDREProfile_eSpeechCompression:        pSettings->profile = NEXUS_DolbyDigitalReencodeProfile_eSpeechCompression; break;
    }
    /* These values are defined by the AC3 spec and don't need CASSERT */
    pSettings->centerMixLevel = g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eCentreMixLevel;
    pSettings->surroundMixLevel = g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eSurroundMixLevel;
    pSettings->dolbySurround = g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eDolbySurroundMode;
    pSettings->dialogLevel = g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.ui32DialogLevel;
    pSettings->inputAcmod = g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eInputStreamAcmod;
    pSettings->inputHasLfe = g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.bInputHasLfe;
    pSettings->inputHasDv258Applied = g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.bDv258ProcessedInput;
}

NEXUS_DolbyDigitalReencodeHandle NEXUS_DolbyDigitalReencode_Open(
    const NEXUS_DolbyDigitalReencodeSettings *pSettings
    )
{
#if NEXUS_HAS_MS11_SUPPORT
    NEXUS_DolbyDigitalReencodeHandle handle;
    BERR_Code errCode;

    handle = BKNI_Malloc(sizeof(NEXUS_DolbyDigitalReencode));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    BKNI_Memset(handle, 0, sizeof(NEXUS_DolbyDigitalReencode));
    NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_DolbyDigitalReencodeConnectorType_eStereo], NEXUS_AudioInputType_eDolbyDigitalReencode, handle);
    handle->connectors[NEXUS_DolbyDigitalReencodeConnectorType_eStereo].format = NEXUS_AudioInputFormat_ePcmStereo;
    NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_DolbyDigitalReencodeConnectorType_eMultichannel], NEXUS_AudioInputType_eDolbyDigitalReencode, handle);
    handle->connectors[NEXUS_DolbyDigitalReencodeConnectorType_eMultichannel].format = NEXUS_AudioInputFormat_ePcm5_1;
    NEXUS_AUDIO_INPUT_INIT(&handle->connectors[NEXUS_DolbyDigitalReencodeConnectorType_eCompressed], NEXUS_AudioInputType_eDolbyDigitalReencode, handle);
    handle->connectors[NEXUS_DolbyDigitalReencodeConnectorType_eCompressed].format = NEXUS_AudioInputFormat_eCompressed;
    
    BDBG_OBJECT_SET(handle, NEXUS_DolbyDigitalReencode);

    if ( NULL == pSettings )
    {
        NEXUS_DolbyDigitalReencode_GetDefaultSettings(&handle->settings);
    }
    else
    {
        handle->settings = *pSettings;
    }
    
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eDDRE,
                                           &g_NEXUS_StageSettings);

    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &handle->ddre);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_DolbyDigitalReencode_Close(handle);
        return NULL;
    }

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eEncodeAc3,
                                           &g_NEXUS_StageSettings);

    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &handle->encoder);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_DolbyDigitalReencode_Close(handle);
        return NULL;
    }

    BLST_S_INSERT_HEAD(&g_stageList, handle, node);

    (void)NEXUS_DolbyDigitalReencode_SetSettings(handle, &handle->settings);

    return handle;
#else
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    BSTD_UNUSED(pSettings);
    return NULL;
#endif
}

void NEXUS_DolbyDigitalReencode_Close(
    NEXUS_DolbyDigitalReencodeHandle handle
    )
{
    int i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);

    for ( i = 0; i < NEXUS_DolbyDigitalReencodeConnectorType_eMax; i++ )
    {
        if ( handle->connectors[i].pMixerData )
        {
            BDBG_MSG(("Shutting down connector %p", (void *)&handle->connectors[i]));
            NEXUS_AudioInput_Shutdown(&handle->connectors[i]);
        }
    }

    if ( NULL != handle->encoder )
    {
        BLST_S_REMOVE(&g_stageList, handle, NEXUS_DolbyDigitalReencode, node);
        BRAP_DestroyProcessingStage(handle->encoder);
    }
    if ( NULL != handle->ddre )
    {
        BRAP_DestroyProcessingStage(handle->ddre);
    }

    BKNI_Memset(handle, 0, sizeof(*handle));    /* This clears the debug object */
    BKNI_Free(handle);
}

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
#if NEXUS_HAS_MS11_SUPPORT
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != pSettings);

    BRAP_GetCurrentProcessingStageSettings(handle->ddre, &g_NEXUS_StageSettings);

    switch ( pSettings->dualMonoMode )
    {
    default:
    case NEXUS_AudioDecoderDualMonoMode_eStereo:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eLeft:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eLeftMono;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eRight:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eRightMono;
        break;
    case NEXUS_AudioDecoderDualMonoMode_eMix:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eDualMonoMix;
        break;
    }

    if ( pSettings->outputMode == NEXUS_AudioDecoderOutputMode_eAuto )
    {
        NEXUS_AudioOutputList outputList;
        /* Determine output mode based on connections */
        NEXUS_AudioInput_P_GetOutputs(&handle->connectors[NEXUS_AudioConnectorType_eMultichannel], &outputList, false);
        if ( outputList.outputs[0] != NULL )
        {
            BDBG_MSG(("Auto output mode 3/2"));
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e3_2;
        }
        else
        {
            BDBG_MSG(("Auto output mode 2/0"));
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e2_0;
        }
    }
    else
    {
        switch ( pSettings->outputMode )
        {
        default:
            (void)BERR_TRACE(BERR_INVALID_PARAMETER);
            /* Fall through */
        case NEXUS_AudioDecoderOutputMode_eAuto:
        case NEXUS_AudioDecoderOutputMode_e2_0:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e2_0;
            break;
        case NEXUS_AudioDecoderOutputMode_e1_0:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e1_0;
            break;
        case NEXUS_AudioDecoderOutputMode_e1_1:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e1_1;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_0:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e3_0;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_1:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e3_1;
            break;
        case NEXUS_AudioDecoderOutputMode_e2_2:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e2_2;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_2:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e3_2;
            break;
        case NEXUS_AudioDecoderOutputMode_e3_4:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode = BRAP_OutputMode_e3_4;
            break;
        }
    }
    switch ( pSettings->outputLfeMode )
    {
    default:
    case NEXUS_AudioDecoderOutputLfeMode_eAuto:
        switch ( g_NEXUS_StageSettings.uConfigParams.sDDREParams.eOutputMode )
        {
        case BRAP_OutputMode_e1_0:
        case BRAP_OutputMode_e1_1:
        case BRAP_OutputMode_e2_0:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.bOutputLfeOn = false;
            break;
        default:
            g_NEXUS_StageSettings.uConfigParams.sDDREParams.bOutputLfeOn = true;
            break;
        }
        break;
    case NEXUS_AudioDecoderOutputLfeMode_eOn:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.bOutputLfeOn = true;
        break;
    case NEXUS_AudioDecoderOutputLfeMode_eOff:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.bOutputLfeOn = false;
        break;
    }

    switch ( pSettings->drcMode )
    {
    default:
    case NEXUS_AudioDecoderDolbyDrcMode_eRf:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eCompMode = BRAP_DSPCHN_DDRE_CompMode_eRf;
        break;
    case NEXUS_AudioDecoderDolbyDrcMode_eLine:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eCompMode = BRAP_DSPCHN_DDRE_CompMode_eLine;
        break;
    }

    switch ( pSettings->stereoDownmixMode )
    {
    default:
    case NEXUS_DolbyDigitalReencodeDownmixMode_eLtRt:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDownmixType = BRAP_DSPCHN_DDRE_DownmixType_eLtRt;
        break;
    case NEXUS_DolbyDigitalReencodeDownmixMode_eLoRo:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDownmixType = BRAP_DSPCHN_DDRE_DownmixType_eLoRo;
        break;
    case NEXUS_DolbyDigitalReencodeDownmixMode_eArib:
        g_NEXUS_StageSettings.uConfigParams.sDDREParams.eDownmixType = BRAP_DSPCHN_DDRE_DownmixType_eArib;
        break;
    }

    g_NEXUS_StageSettings.uConfigParams.sDDREParams.ui32DrcCutFac = pSettings->cut;
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.ui32DrcBoostFac = pSettings->boost;

    g_NEXUS_StageSettings.uConfigParams.sDDREParams.bExternalPcmModeEnabled = pSettings->externalPcmMode;    
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.bCertificationModeEnabled = pSettings->certificationMode;    
    switch ( pSettings->profile )
    {
    case NEXUS_DolbyDigitalReencodeProfile_eNoCompression:              g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile = BRAP_DSPCHN_DDREProfile_eNoCompression; break;
    default:
    case NEXUS_DolbyDigitalReencodeProfile_eFilmStandardCompression:    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile = BRAP_DSPCHN_DDREProfile_eFilmStandardCompression; break;
    case NEXUS_DolbyDigitalReencodeProfile_eFilmLightCompression:       g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile = BRAP_DSPCHN_DDREProfile_eFilmLightCompression; break;
    case NEXUS_DolbyDigitalReencodeProfile_eMusicStandardCompression:   g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile = BRAP_DSPCHN_DDREProfile_eMusicStandardCompression; break;
    case NEXUS_DolbyDigitalReencodeProfile_eMusicLightCompression:      g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile = BRAP_DSPCHN_DDREProfile_eMusicLightCompression; break;
    case NEXUS_DolbyDigitalReencodeProfile_eSpeechCompression:          g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile = BRAP_DSPCHN_DDREProfile_eSpeechCompression; break;
    }
    /* These values are defined by the AC3 spec and don't need CASSERT */
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eCentreMixLevel = pSettings->centerMixLevel;
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eSurroundMixLevel = pSettings->surroundMixLevel;
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eDolbySurroundMode = pSettings->dolbySurround;
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.ui32DialogLevel = pSettings->dialogLevel;

    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eInputStreamAcmod = pSettings->inputAcmod;    
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.bInputHasLfe = pSettings->inputHasLfe;    
    g_NEXUS_StageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.bDv258ProcessedInput = pSettings->inputHasDv258Applied;    

    errCode = BRAP_SetProcessingStageSettings(handle->ddre, &g_NEXUS_StageSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BRAP_GetCurrentProcessingStageSettings(handle->encoder, &g_NEXUS_StageSettings);
    g_NEXUS_StageSettings.uConfigParams.sAc3ENCParams.bSpdifHeaderEnable = pSettings->encodeSettings.spdifHeaderEnabled;
    g_NEXUS_StageSettings.uConfigParams.sAc3ENCParams.bTranscodeEnable = !pSettings->encodeSettings.certificationMode;
    errCode = BRAP_SetProcessingStageSettings(handle->encoder, &g_NEXUS_StageSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->settings = *pSettings;

    return BERR_SUCCESS;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif
}

NEXUS_AudioInput NEXUS_DolbyDigitalReencode_GetConnector( /* attr{shutdown=NEXUS_AudioInput_Shutdown} */
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_DolbyDigitalReencodeConnectorType connectorType
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);

    switch ( connectorType )
    {
    case NEXUS_AudioConnectorType_eStereo:
    case NEXUS_AudioConnectorType_eMultichannel:
    case NEXUS_AudioConnectorType_eCompressed:
        break;
    default:
        BDBG_ERR(("Unsupported connector type %u", connectorType));
        return NULL;
    }

    return &handle->connectors[connectorType];
}

NEXUS_Error NEXUS_DolbyDigitalReencode_AddInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;
    int i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != input);
    
    if ( handle->input )
    {
        BDBG_ERR(("An input is already connected to this processing stage"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    for ( i = 0; i < NEXUS_DolbyDigitalReencodeConnectorType_eMax; i++ )
    {
        switch ( i )        
        {
        case NEXUS_AudioConnectorType_eStereo:
        case NEXUS_AudioConnectorType_eMultichannel:
        case NEXUS_AudioConnectorType_eCompressed:
            break;
        default:
            continue;
        }
        errCode = NEXUS_AudioInput_P_AddInput(&handle->connectors[i], input);
        if ( errCode )
        {
            break;
        }
    }

    if ( i < NEXUS_DolbyDigitalReencodeConnectorType_eMax )
    {
        for ( ; i >= 0; i-- )
        {
            switch ( i )        
            {
            case NEXUS_AudioConnectorType_eStereo:
            case NEXUS_AudioConnectorType_eMultichannel:
            case NEXUS_AudioConnectorType_eCompressed:
                break;
            default:
                continue;
            }
            (void)NEXUS_AudioInput_P_RemoveInput(&handle->connectors[i], input);
        }
        return BERR_TRACE(errCode);
    }

    handle->input = input;

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveInput(
    NEXUS_DolbyDigitalReencodeHandle handle,
    NEXUS_AudioInput input
    )
{
    unsigned i;
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != input);

    if ( input != handle->input )
    {
        BDBG_ERR(("Invalid input handle"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for ( i = 0; i < NEXUS_DolbyDigitalReencodeConnectorType_eMax; i++ )
    {
        switch ( i )        
        {
        case NEXUS_AudioConnectorType_eStereo:
        case NEXUS_AudioConnectorType_eMultichannel:
        case NEXUS_AudioConnectorType_eCompressed:
            break;
        default:
            continue;
        }
        errCode = NEXUS_AudioInput_P_RemoveInput(&handle->connectors[i], input);
        BDBG_ASSERT(errCode == BERR_SUCCESS);   /* If this fails, there is no easy way to recover */
    }
    
    handle->input = NULL;
        
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_DolbyDigitalReencode_RemoveAllInputs(
    NEXUS_DolbyDigitalReencodeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    return NEXUS_DolbyDigitalReencode_RemoveInput(handle, handle->input);
}

NEXUS_Error NEXUS_DolbyDigitalReencode_P_Init(void)
{
    BLST_S_INIT(&g_stageList);
    return BERR_SUCCESS;
}

void NEXUS_DolbyDigitalReencode_P_Uninit(void)
{
    NEXUS_DolbyDigitalReencode *pNode;
    while ( NULL != (pNode = BLST_S_FIRST(&g_stageList)) )
    {
        NEXUS_DolbyDigitalReencode_Close(pNode);
    }
}

BRAP_ProcessingStageHandle NEXUS_DolbyDigitalReencode_P_GetStageHandle(NEXUS_DolbyDigitalReencodeHandle handle, NEXUS_AudioInput input, BRAP_ProcessingStageHandle *pEncoder)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != input);
    BDBG_ASSERT(NULL != pEncoder);

    (void)NEXUS_DolbyDigitalReencode_SetSettings(handle, &handle->settings);

    if ( input->format == NEXUS_AudioInputFormat_eCompressed )
    {
        *pEncoder = handle->encoder;
    }
    else
    {
        *pEncoder = NULL;
    }

    return handle->ddre;
}

