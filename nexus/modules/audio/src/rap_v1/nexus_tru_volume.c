/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
#include "nexus_audio_priv.h"

BDBG_MODULE(nexus_tru_volume);

#if NEXUS_SRSTRUVOL

static BLST_S_HEAD(StageList, NEXUS_TruVolume) g_stageList;

typedef struct NEXUS_TruVolume
{
    NEXUS_OBJECT(NEXUS_TruVolume);
    BLST_S_ENTRY(NEXUS_TruVolume) node;
    NEXUS_AudioInputObject connector;
    NEXUS_TruVolumeSettings settings;
    NEXUS_AudioInput input;
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

    pSettings->enabled = true;
    pSettings->blockSize = NEXUS_TruVolumeBlockSize_e256;
    pSettings->enableNormalGain = true;
    pSettings->inputGain = 25;
    pSettings->outputGain = 25;
    pSettings->bypassGain = 100;
    pSettings->referenceLevel = 50 ;
    pSettings->mode = NEXUS_TruVolumeMode_eNormal;
    pSettings->speakerResolution = NEXUS_TruVolumeSpeakerResolution_e20Hz;
    pSettings->maxGain = 100;
    pSettings->enableNoiseManager = 1;
    pSettings->noiseManagerThreshold = 10;
}

/***************************************************************************
Summary:
    Open an SRS TruVolume processing stage
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

    BLST_S_INSERT_HEAD(&g_stageList, pStage, node);

    /* Apply settings */
    (void)NEXUS_TruVolume_SetSettings(pStage, &pStage->settings);

    return pStage;
}

/***************************************************************************
Summary:
    Close an SRS TruVolume processing stage
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
    NEXUS_Error errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_TruVolume, handle);

    if ( handle->connector.pMixerData )
    {
        BDBG_ERR(("This object is still connected.  Please call NEXUS_AudioInput_Shutdown first."));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    BLST_S_REMOVE(&g_stageList, handle, NEXUS_TruVolume, node);
    NEXUS_OBJECT_DESTROY(NEXUS_TruVolume, handle);
    BKNI_Free(handle);
}


/***************************************************************************
Summary:
    Get Settings for an SRS TruVolume processing stage
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
    Set Settings for an SRS TruVolume processing stage
See Also:
    NEXUS_TruVolume_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_TruVolume_SetSettings(
    NEXUS_TruVolumeHandle handle,
    const NEXUS_TruVolumeSettings *pSettings
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    if (handle->input)
    {
    	BRAP_PP_ConfigParams ppconfig_params;
    	unsigned int uiPpBranchId=0x0,uiPpStageId=0x1;
    
        NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)handle->input->pObjectHandle;
        BRAP_ChannelHandle dec = NEXUS_AudioDecoder_P_GetChannel(pDecoder);

    	errCode = BRAP_DEC_GetPpCurrentConfig (dec,BRAP_DSPCHN_PP_Algo_eSRS_TruVol,
    					  uiPpBranchId,uiPpStageId,&ppconfig_params);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    
        switch ( pSettings->blockSize )
        {
        default:
        case NEXUS_TruVolumeBlockSize_e256:
            ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32BlockSize = 256;
            break;
        case NEXUS_TruVolumeBlockSize_e512:
            ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32BlockSize = 512;
            break;
        case NEXUS_TruVolumeBlockSize_e768:
            ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32BlockSize = 768;
            break;
        case NEXUS_TruVolumeBlockSize_e1024:
            ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32BlockSize = 1024;
            break;
        }

        ppconfig_params.uConfigParams.sSRSTruVolConfigParams.bTrueVolume_enable = pSettings->enabled;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.bMEnable = pSettings->enableNormalGain;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32InputGain = pSettings->inputGain;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32OutputGain = pSettings->outputGain;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32BypassGain = pSettings->bypassGain;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32ReferenceLevel = pSettings->referenceLevel;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32Mode = (pSettings->mode == NEXUS_TruVolumeMode_eNormal)? 0:1;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32Size = pSettings->speakerResolution;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32MaxGain = pSettings->maxGain;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32NoiseManager = pSettings->enableNoiseManager;
    	ppconfig_params.uConfigParams.sSRSTruVolConfigParams.i32NoiseManagerThreshold = pSettings->noiseManagerThreshold;
        
        errCode = BRAP_DEC_SetPPConfig(dec,&ppconfig_params);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    else
    {
        BDBG_MSG(("%s(): Not ready to set settings, not connected.",__FUNCTION__));
    }
    return BERR_SUCCESS;
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

    if ( input->objectType != NEXUS_AudioInputType_eDecoder )
    {
        BDBG_ERR(("On 7400, input to TruVolume must be decoder."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", input));
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
        BDBG_ERR(("Input %p is not connected to this processing stage", input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check if new input is already running */
    if ( NEXUS_AudioInput_P_IsRunning(input) )
    {
        BDBG_ERR(("Input %p is running.  Please stop first.", input));
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

#else /* #if NEXUS_SRSTRUVOL */

typedef struct NEXUS_TruVolume
{
    NEXUS_OBJECT(NEXUS_TruVolume);
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
    BSTD_UNUSED(pSettings);
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
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
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
    BSTD_UNUSED(handle);
}


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
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
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
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
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
    BSTD_UNUSED(handle);
    return NULL;
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
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
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
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
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
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_TruVolume_P_Init(void)
{
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_TruVolume_P_Uninit(void)
{
}

#endif /* #if NEXUS_SRSTRUVOL */

NEXUS_OBJECT_CLASS_MAKE(NEXUS_TruVolume, NEXUS_TruVolume_Close);
