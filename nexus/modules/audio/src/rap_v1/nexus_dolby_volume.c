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
*   API name: DolbyVolume
*    Specific APIs related to Dolby Volume Audio Processing
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_audio_module.h"
#include "nexus_audio_priv.h"

BDBG_MODULE(nexus_dolby_volume);

#if NEXUS_DOLBYVOL

static BLST_S_HEAD(StageList, NEXUS_DolbyVolume) g_stageList;

typedef struct NEXUS_DolbyVolume
{
    NEXUS_OBJECT(NEXUS_DolbyVolume);
    BLST_S_ENTRY(NEXUS_DolbyVolume) node;
    NEXUS_AudioInputObject connector;
    NEXUS_DolbyVolumeSettings settings;
    NEXUS_AudioInput input;
} NEXUS_DolbyVolume;

/***************************************************************************
Summary:
    Get default settings for a Dolby Volume processing stage
See Also:
    NEXUS_DolbyVolume_Open
 ***************************************************************************/
void NEXUS_DolbyVolume_GetDefaultSettings(
    NEXUS_DolbyVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->enabled = true;
    pSettings->blockSize = 512;
    pSettings->numBands = 20;
    pSettings->inputReferenceLevel = 85;
    pSettings->outputReferenceLevel = 85;
    pSettings->calibrationOffset = 0;
    pSettings->volumeModelerEnabled = 1;
    pSettings->reset = false;
    pSettings->digitalVolumeLevel = 0;
    pSettings->analogVolumeLevel = 0;
    pSettings->volumeLevelerAmount = 0;
    pSettings->volumeLevelerEnabled = true;
    pSettings->midsideProcessingEnabled = false;
    pSettings->halfModeEnabled = false;
}

/***************************************************************************
Summary:
    Open a Dolby Volume processing stage
See Also:
    NEXUS_DolbyVolume_Close
 ***************************************************************************/
NEXUS_DolbyVolumeHandle NEXUS_DolbyVolume_Open(
    const NEXUS_DolbyVolumeSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_Error errCode;
    NEXUS_DolbyVolume *pStage;    

    pStage = BKNI_Malloc(sizeof(*pStage));
    if ( NULL == pStage )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_DolbyVolume, pStage);
    NEXUS_AUDIO_INPUT_INIT(&pStage->connector, NEXUS_AudioInputType_eDolbyVolume, pStage);
    pStage->connector.format = NEXUS_AudioInputFormat_ePcmStereo;

    if ( pSettings )
    {
        pStage->settings = *pSettings;
    }
    else
    {
        NEXUS_DolbyVolume_GetDefaultSettings(&pStage->settings);
    }


    BLST_S_INSERT_HEAD(&g_stageList, pStage, node);

    /* Apply settings */
    (void)NEXUS_DolbyVolume_SetSettings(pStage, &pStage->settings);

    return pStage;
}

/***************************************************************************
Summary:
    Close an Dolby Volume processing stage
Description:
    Input to the stage must be removed prior to closing.
See Also:
    NEXUS_DolbyVolume_Open
    NEXUS_AudioOutput_RemoveInput
    NEXUS_AudioOutput_RemoveAllInputs
 ***************************************************************************/
static void NEXUS_DolbyVolume_P_Finalizer(
    NEXUS_DolbyVolumeHandle handle
    )
{
    NEXUS_Error errCode;

    NEXUS_OBJECT_ASSERT(NEXUS_DolbyVolume, handle);

    if ( handle->connector.pMixerData )
    {
        BDBG_ERR(("This object is still connected.  Please call NEXUS_AudioInput_Shutdown first."));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    BLST_S_REMOVE(&g_stageList, handle, NEXUS_DolbyVolume, node);
    NEXUS_OBJECT_DESTROY(NEXUS_DolbyVolume, handle);
    BKNI_Free(handle);
}


/***************************************************************************
Summary:
    Get Settings for a Dolby Volume processing stage
See Also:
    NEXUS_DolbyVolume_SetSettings
 ***************************************************************************/
void NEXUS_DolbyVolume_GetSettings(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_DolbyVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}


/***************************************************************************
Summary:
    Set Settings for a Dolby Volume processing stage
See Also:
    NEXUS_DolbyVolume_GetSettings
 ***************************************************************************/
NEXUS_Error NEXUS_DolbyVolume_SetSettings(
    NEXUS_DolbyVolumeHandle handle,
    const NEXUS_DolbyVolumeSettings *pSettings
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    BDBG_ASSERT(NULL != pSettings);

    handle->settings = *pSettings;

    if (handle->input)
    {
    	BRAP_PP_ConfigParams ppconfig_params;
    	unsigned int uiPpBranchId=0x0,uiPpStageId=0x1;
    
        NEXUS_AudioDecoder *pDecoder = (NEXUS_AudioDecoder *)handle->input->pObjectHandle;
        BRAP_ChannelHandle dec = NEXUS_AudioDecoder_P_GetChannel(pDecoder);

    	errCode = BRAP_DEC_GetPpCurrentConfig (dec,BRAP_DSPCHN_PP_Algo_eDolbyVolume,
    					  uiPpBranchId,uiPpStageId,&ppconfig_params);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    
        ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.bVolume_iq_enable = pSettings->enabled;
        ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Block_size = pSettings->blockSize;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Nbands = pSettings->numBands;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Input_reference_level = pSettings->inputReferenceLevel;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Output_reference_level = pSettings->outputReferenceLevel;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Calibration = pSettings->calibrationOffset;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Reset_now_flag = pSettings->reset;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.bVlm_enable = pSettings->volumeModelerEnabled;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Digital_volume_level = pSettings->digitalVolumeLevel;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Analog_volume_level = pSettings->analogVolumeLevel;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.bLvl_disable = !pSettings->volumeLevelerEnabled;
    	ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.bEnable_midside_proc = pSettings->midsideProcessingEnabled;
        ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.i32Lvl_amount = pSettings->volumeLevelerAmount;
        ppconfig_params.uConfigParams.sDolbyVolumeConfigParams.bHalfmode_flag = pSettings->halfModeEnabled;

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

NEXUS_AudioInput NEXUS_DolbyVolume_GetConnector(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    return &handle->connector;
}

NEXUS_Error NEXUS_DolbyVolume_AddInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
    BDBG_ASSERT(NULL != input);

    if ( handle->input )
    {
        BDBG_ERR(("An input is already connected to this processing stage"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( input->objectType != NEXUS_AudioInputType_eDecoder )
    {
        BDBG_ERR(("On 7400, input to Dolby Volume must be decoder."));
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

NEXUS_Error NEXUS_DolbyVolume_RemoveInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);
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

NEXUS_Error NEXUS_DolbyVolume_RemoveAllInputs(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_DolbyVolume);

    if ( handle->input )
    {
        return NEXUS_DolbyVolume_RemoveInput(handle, handle->input);
    }

    return BERR_SUCCESS;
}
NEXUS_Error NEXUS_DolbyVolume_P_Init(void)
{
    BLST_S_INIT(&g_stageList);
    return BERR_SUCCESS;
}

void NEXUS_DolbyVolume_P_Uninit(void)
{
    NEXUS_DolbyVolume *pNode;
    while ( NULL != (pNode = BLST_S_FIRST(&g_stageList)) )
    {
        NEXUS_DolbyVolume_Close(pNode);
    }
}

#else /* #if NEXUS_DOLBYVOL */

typedef struct NEXUS_DolbyVolume
{
    NEXUS_OBJECT(NEXUS_DolbyVolume);
} NEXUS_DolbyVolume;


void NEXUS_DolbyVolume_GetDefaultSettings(
    NEXUS_DolbyVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_DolbyVolumeHandle NEXUS_DolbyVolume_Open(
    const NEXUS_DolbyVolumeSettings *pSettings     /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_DolbyVolume_P_Finalizer(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void NEXUS_DolbyVolume_GetSettings(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_DolbyVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_DolbyVolume_SetSettings(
    NEXUS_DolbyVolumeHandle handle,
    const NEXUS_DolbyVolumeSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_AudioInput NEXUS_DolbyVolume_GetConnector(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

NEXUS_Error NEXUS_DolbyVolume_AddInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyVolume_RemoveInput(
    NEXUS_DolbyVolumeHandle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyVolume_RemoveAllInputs(
    NEXUS_DolbyVolumeHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyVolume_P_Init(void)
{
    return BERR_SUCCESS;
}

void NEXUS_DolbyVolume_P_Uninit(void)
{
}

#endif /* #if NEXUS_DOLBYVOL */

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DolbyVolume, NEXUS_DolbyVolume_Close);

typedef struct NEXUS_DolbyVolume258 {
    NEXUS_OBJECT(NEXUS_DolbyVolume258);
} NEXUS_DolbyVolume258;

void NEXUS_DolbyVolume258_GetDefaultSettings(
    NEXUS_DolbyVolume258Settings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_DolbyVolume258Handle NEXUS_DolbyVolume258_Open(
    const NEXUS_DolbyVolume258Settings *pSettings     /* Pass NULL for default settings */
    )
{
    BSTD_UNUSED(pSettings);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_DolbyVolume258_P_Finalizer(
    NEXUS_DolbyVolume258Handle handle
    )
{
    BSTD_UNUSED(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_DolbyVolume258, NEXUS_DolbyVolume258_Close);

void NEXUS_DolbyVolume258_GetSettings(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_DolbyVolume258Settings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_DolbyVolume258_SetSettings(
    NEXUS_DolbyVolume258Handle handle,
    const NEXUS_DolbyVolume258Settings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_AudioInput NEXUS_DolbyVolume258_GetConnector(
    NEXUS_DolbyVolume258Handle handle
    )
{
    BSTD_UNUSED(handle);
    return NULL;
}

NEXUS_Error NEXUS_DolbyVolume258_AddInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_DolbyVolume258_RemoveInput(
    NEXUS_DolbyVolume258Handle handle,
    NEXUS_AudioInput input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


NEXUS_Error NEXUS_DolbyVolume258_RemoveAllInputs(
    NEXUS_DolbyVolume258Handle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

