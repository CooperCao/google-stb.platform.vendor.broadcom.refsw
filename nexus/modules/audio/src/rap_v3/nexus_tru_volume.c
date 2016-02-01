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
*   API name: TruVolume
*    Specific APIs related to SRS TruVolume (formerly Volume IQ) Audio Processing
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_tru_volume);

static BLST_S_HEAD(StageList, NEXUS_TruVolume) g_stageList;

typedef struct NEXUS_TruVolume
{
    NEXUS_OBJECT(NEXUS_TruVolume);
    BLST_S_ENTRY(NEXUS_TruVolume) node;
    NEXUS_AudioInputObject connector;
    NEXUS_TruVolumeSettings settings;
    NEXUS_AudioInput input;
    BRAP_ProcessingStageHandle stage;
} NEXUS_TruVolume;

/***************************************************************************
Summary:
    Get default settings for an SRS-HD processing stage
See Also:
    NEXUS_TruVolume_Open
 ***************************************************************************/
void NEXUS_TruVolume_GetDefaultSettings(
    NEXUS_TruVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eSrsTruVolume,
                                           &g_NEXUS_StageSettings);
    pSettings->enabled = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bSrsTruVolumeEnable;
    switch ( g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiBlockSize )
    {
    default:
    case 256:
        pSettings->blockSize = NEXUS_TruVolumeBlockSize_e256;
        break;
    case 512:
        pSettings->blockSize = NEXUS_TruVolumeBlockSize_e512;
        break;
    case 768:
        pSettings->blockSize = NEXUS_TruVolumeBlockSize_e768;
        break;
    case 1024:
        pSettings->blockSize = NEXUS_TruVolumeBlockSize_e1024;
        break;
    }
    pSettings->enableNormalGain = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalGain;
    pSettings->inputGain = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiInputGain*4;
    pSettings->outputGain = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiOutputGain*4;
    pSettings->bypassGain = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiBypassGain;
    pSettings->referenceLevel = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiReferenceLevel;
    switch ( g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eMode )
    {
    default:
        BDBG_ERR(("Unsupported TruVolume mode %u", g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eMode));
        /* Fall through */
    case BRAP_DSPCHN_SrsTruVolumeMode_eNormal:
        pSettings->mode = NEXUS_TruVolumeMode_eNormal;
        break;
    case BRAP_DSPCHN_SrsTruVolumeMode_eLight:
        pSettings->mode = NEXUS_TruVolumeMode_eLight;
        break;
    case BRAP_DSPCHN_SrsTruVolumeMode_eHeavy:
        pSettings->mode = NEXUS_TruVolumeMode_eHeavy;
        break;
    }
    switch ( g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution )
    {
    case BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_20Hz:
        pSettings->speakerResolution = NEXUS_TruVolumeSpeakerResolution_e20Hz;
        break;
    default:
    case BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_40Hz:
        pSettings->speakerResolution = NEXUS_TruVolumeSpeakerResolution_e40Hz;
        break;
    case BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_110Hz:
        pSettings->speakerResolution = NEXUS_TruVolumeSpeakerResolution_e110Hz;
        break;
    case BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_200Hz:
        pSettings->speakerResolution = NEXUS_TruVolumeSpeakerResolution_e200Hz;
        break;
    case BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_315Hz:
        pSettings->speakerResolution = NEXUS_TruVolumeSpeakerResolution_e315Hz;
    break;
    case BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_410Hz:
        pSettings->speakerResolution = NEXUS_TruVolumeSpeakerResolution_e410Hz;
        break;
    }
    pSettings->maxGain = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiMaxGain*25;
    pSettings->enableDcNotchFilter = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableDCNotchFilter;
    pSettings->enableNoiseManager = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNoiseManager;
    pSettings->noiseManagerThreshold = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiNoiseManagerThreshold/2;
    pSettings->enableNormalizer = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalizer;
    pSettings->calibrate = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiCalibrate;
#if BRAP_VER >= 4 && BCHP_CHIP >= 35000
    pSettings->highPassFilter.enabled = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.bEnable;
    pSettings->highPassFilter.coefGenMode = g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.eCoefGenMode;
    if ( NEXUS_SrsFilterCoefGenMode_eFilterUser == pSettings->highPassFilter.coefGenMode )
    {
        /* Note: Use memcpy here but please make sure NEXUS_SrsHighPassFilterCoefUserParam matches BRAP_DSPCHN_HPFilterCoef */
        BKNI_Memcpy(&pSettings->highPassFilter.coefParam.highPassFilterCoefUser[0],
                    &g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterCoef[0],
                    sizeof(pSettings->highPassFilter.coefParam.highPassFilterCoefUser));
    }
    else
    {
        pSettings->highPassFilter.coefParam.highPassFilterCoefSpec.cutOffFrequency =
            g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency;
    
        pSettings->highPassFilter.coefParam.highPassFilterCoefSpec.filterOrder =
            g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.eFilterOrder;
    }
#endif
}

/***************************************************************************
Summary:
    Open a SRS-HD processing stage
See Also:
    NEXUS_TruVolume_Close
 ***************************************************************************/
NEXUS_TruVolumeHandle NEXUS_TruVolume_Open(
    const NEXUS_TruVolumeSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;
    NEXUS_TruVolume *pStage;    

    pStage = BKNI_Malloc(sizeof(*pStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_TruVolume, pStage);
    NEXUS_AUDIO_INPUT_INIT(&pStage->connector, NEXUS_AudioInputType_eTruVolume, pStage);
    pStage->connector.format = NEXUS_AudioInputFormat_eNone;

    if ( pSettings )
    {
        pStage->settings = *pSettings;
    }
    else
    {
        NEXUS_TruVolume_GetDefaultSettings(&pStage->settings);
    }

    BRAP_GetDefaultProcessingStageSettings(BRAP_ProcessingType_eSrsTruVolume,
                                           &g_NEXUS_StageSettings);
    errCode = BRAP_CreateProcessingStage(g_NEXUS_audioModuleData.hRap,
                                         &g_NEXUS_StageSettings,
                                         &pStage->stage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        NEXUS_TruVolume_Close(pStage);
        return NULL;
    }

    BLST_S_INSERT_HEAD(&g_stageList, pStage, node);

    /* Apply settings */
    (void)NEXUS_TruVolume_SetSettings(pStage, &pStage->settings);

    return pStage;
}

/***************************************************************************
Summary:
    Close a SRS-HD processing stage
Description:
    Input to the stage must be removed prior to closing.
See Also:
    NEXUS_TruVolume_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
static void NEXUS_TruVolume_P_Finalizer(
    NEXUS_TruVolumeHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_TruVolume, handle);

    if ( handle->connector.pMixerData )
    {
        NEXUS_AudioInput_Shutdown(&handle->connector);
    }

    if ( NULL != handle->stage )
    {
        BRAP_DestroyProcessingStage(handle->stage);
    }

    BLST_S_REMOVE(&g_stageList, handle, NEXUS_TruVolume, node);
    NEXUS_OBJECT_DESTROY(NEXUS_TruVolume, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_TruVolume, NEXUS_TruVolume_Close);


/***************************************************************************
Summary:
    Get Settings for a SRS-HD processing stage
See Also:
    NEXUS_TruVolume_SetSettings
 ***************************************************************************/
void NEXUS_TruVolume_GetSettings(
    NEXUS_TruVolumeHandle handle,
    NEXUS_TruVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}


/***************************************************************************
Summary:
    Set Settings for a SRS-HD processing stage
See Also:
    NEXUS_TruVolume_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_TruVolume_SetSettings(
    NEXUS_TruVolumeHandle handle,
    const NEXUS_TruVolumeSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    return ( NEXUS_TruVolume_P_SetSettings(handle->stage, pSettings, false, NULL) );
}

/***************************************************************************
Summary:
    Get the audio connector for a SRS-HD processing stage
See Also:

 ***************************************************************************/
NEXUS_AudioInput NEXUS_TruVolume_GetConnector(
    NEXUS_TruVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    return &handle->connector;
}

/***************************************************************************
Summary:
    Add an input to this processing stage
See Also:
    NEXUS_TruVolume_RemoveInput
    NEXUS_TruVolume_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_TruVolume_AddInput(
    NEXUS_TruVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
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

    /* Connect at input node */
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);

    if ( BERR_SUCCESS == errCode )
    {
        handle->input = input;
    }

    return errCode;
}

/***************************************************************************
Summary:
    Add an input to this processing stage
See Also:
    NEXUS_TruVolume_AddInput
    NEXUS_TruVolume_RemoveAllInputs
 ***************************************************************************/
NEXUS_Error NEXUS_TruVolume_RemoveInput(
    NEXUS_TruVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != input);

    if ( handle->input != input )
    {
        BDBG_ERR(("Input %p is not connected to this processing stage", (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", (void *)input));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Disconnect at input node */
    handle->input = NULL;
    return NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
}

/***************************************************************************
Summary:
    Add an input to this processing stage
See Also:
    NEXUS_TruVolume_AddInput
    NEXUS_TruVolume_RemoveInput
 ***************************************************************************/
NEXUS_Error NEXUS_TruVolume_RemoveAllInputs(
    NEXUS_TruVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);

    if ( handle->input )
    {
        return NEXUS_TruVolume_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_TruVolume_P_Init(void)
{
    BLST_S_INIT(&g_stageList);
    return BERR_SUCCESS;
}

void NEXUS_TruVolume_P_Uninit(void)
{
    NEXUS_TruVolume *pNode;
    while ( NULL != (pNode = BLST_S_FIRST(&g_stageList)) )
    {
        NEXUS_TruVolume_Close(pNode);
    }
}

BRAP_ProcessingStageHandle NEXUS_TruVolume_P_GetStageHandle(NEXUS_TruVolumeHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    return handle->stage;
}

NEXUS_Error NEXUS_TruVolume_P_SetSettings(
    BRAP_ProcessingStageHandle stage,
    const NEXUS_TruVolumeSettings *pSettings,
    bool bIsStudioSound,
    void *pProcessingSettings
    )
{
    NEXUS_Error errCode;

    BRAP_GetCurrentProcessingStageSettings(stage, &g_NEXUS_StageSettings);

#if NEXUS_HAS_AUDIO_POST_PROCESSING
    if ( bIsStudioSound )
    {
        NEXUS_StudioSoundSettings * pStudioSoundSettings = (NEXUS_StudioSoundSettings *)pProcessingSettings;

		BRAP_DSPCHN_StudioSoundMode mode;
		switch ( pStudioSoundSettings->mode )
		{
		case NEXUS_StudioSoundMode_eMode1:
			mode = BRAP_DSPCHN_StudioSoundMode_eMode1;
			break;
		case NEXUS_StudioSoundMode_eMode2:
			mode = BRAP_DSPCHN_StudioSoundMode_eMode2;
			break;
		default:
			BDBG_ERR(("Unsupported StudioSound mode value."));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
		}

        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.bIsStudioSound = true;
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.eStudioSoundMode = mode;
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.bEnable = pStudioSoundSettings->enabled;
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32InputGain = pStudioSoundSettings->inputGain;
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32OutputGain = pStudioSoundSettings->outputGain;
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32BypassGain = pStudioSoundSettings->bypassGain;
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32HeadroomGain = pStudioSoundSettings->headroomGain;
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.eInputMode = pStudioSoundSettings->inputMode;
    }
#else
    BSTD_UNUSED(pProcessingSettings);
    BSTD_UNUSED(bIsStudioSound);
#endif

    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bSrsTruVolumeEnable = pSettings->enabled;
    switch ( pSettings->blockSize )
    {
    default:
    case NEXUS_TruVolumeBlockSize_e256:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiBlockSize = 256;
        break;
    case NEXUS_TruVolumeBlockSize_e512:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiBlockSize = 512;
        break;
    case NEXUS_TruVolumeBlockSize_e768:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiBlockSize = 768;
        break;
    case NEXUS_TruVolumeBlockSize_e1024:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiBlockSize = 1024;
        break;
    }
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalGain = pSettings->enableNormalGain;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiInputGain = pSettings->inputGain/4;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiOutputGain = pSettings->outputGain/4;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiBypassGain = pSettings->bypassGain;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiReferenceLevel = pSettings->referenceLevel;
    switch ( pSettings->mode )
    {
    case NEXUS_TruVolumeMode_eNormal:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eMode = BRAP_DSPCHN_SrsTruVolumeMode_eNormal;
        break;
    case NEXUS_TruVolumeMode_eLight:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eMode = BRAP_DSPCHN_SrsTruVolumeMode_eLight;
        break;
    case NEXUS_TruVolumeMode_eHeavy:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eMode = BRAP_DSPCHN_SrsTruVolumeMode_eHeavy;
        break;
    default:
        BDBG_ERR(("Unsupported TruVolume mode %u", pSettings->mode));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    switch ( pSettings->speakerResolution )
    {
    case NEXUS_TruVolumeSpeakerResolution_e20Hz:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution = BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_20Hz;
        break;
    default:
    case NEXUS_TruVolumeSpeakerResolution_e40Hz:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution = BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_40Hz;
        break;
    case NEXUS_TruVolumeSpeakerResolution_e110Hz:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution = BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_110Hz;
        break;
    case NEXUS_TruVolumeSpeakerResolution_e200Hz:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution = BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_200Hz;
        break;
    case NEXUS_TruVolumeSpeakerResolution_e315Hz:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution = BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_315Hz;
        break;
    case NEXUS_TruVolumeSpeakerResolution_e410Hz:
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution = BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_410Hz;
        break;
    }
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiMaxGain = pSettings->maxGain/25;
    if ( g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiMaxGain == 0 )
    {
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiMaxGain = 1;
    }
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableDCNotchFilter = pSettings->enableDcNotchFilter;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNoiseManager = pSettings->enableNoiseManager;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiNoiseManagerThreshold = pSettings->noiseManagerThreshold*2;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalizer = pSettings->enableNormalizer;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.uiCalibrate = pSettings->calibrate;
#if BRAP_VER >= 4 && BCHP_CHIP >= 35000
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.bEnable = pSettings->highPassFilter.enabled;
    g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.eCoefGenMode = pSettings->highPassFilter.coefGenMode;
    if ( NEXUS_SrsFilterCoefGenMode_eFilterUser == pSettings->highPassFilter.coefGenMode )
    {
        /* Note: Use memcpy here but please make sure NEXUS_SrsHighPassFilterCoefUserParam matches BRAP_DSPCHN_HPFilterCoef */
        BKNI_Memcpy(&g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterCoef[0],
                    &pSettings->highPassFilter.coefParam.highPassFilterCoefUser[0],
                    sizeof(pSettings->highPassFilter.coefParam.highPassFilterCoefUser));
    }
    else
    {
        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency =
        pSettings->highPassFilter.coefParam.highPassFilterCoefSpec.cutOffFrequency;

        g_NEXUS_StageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.eFilterOrder =
        pSettings->highPassFilter.coefParam.highPassFilterCoefSpec.filterOrder;
    }
#endif
    errCode = BRAP_SetProcessingStageSettings(stage, &g_NEXUS_StageSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;

}

