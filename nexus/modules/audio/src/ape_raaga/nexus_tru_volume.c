/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
*   API name: TruVolume
*    Specific APIs related to SRS TruVolume (formerly Volume IQ) Audio Processing
*
***************************************************************************/

#include "nexus_audio_module.h"

BDBG_MODULE(nexus_tru_volume);

typedef struct NEXUS_TruVolume
{
    NEXUS_OBJECT(NEXUS_TruVolume);
    NEXUS_AudioInputObject connector;
    NEXUS_TruVolumeSettings settings;
    NEXUS_AudioInputHandle input;
    BAPE_TruVolumeHandle apeHandle;
    char name[11];   /* TRU VOLUME */
} NEXUS_TruVolume;

void NEXUS_TruVolume_GetDefaultSettings(
    NEXUS_TruVolumeSettings *pSettings   /* [out] default settings */
    )
{
    unsigned i, j;
    
    BAPE_TruVolumeSettings piSettings;
    BDBG_ASSERT(NULL != pSettings);
    BAPE_TruVolume_GetDefaultSettings(&piSettings);
    pSettings->enabled = piSettings.enabled;
    pSettings->blockSize = piSettings.blockSize;
    BDBG_CASSERT(NEXUS_TruVolumeBlockSize_eMax == (NEXUS_TruVolumeBlockSize)BAPE_TruVolumeBlockSize_eMax);
    pSettings->enableNormalGain = piSettings.enableNormalGain;
    pSettings->inputGain = piSettings.inputGain;
    pSettings->outputGain = piSettings.outputGain;
    pSettings->bypassGain = piSettings.bypassGain;
    pSettings->referenceLevel = piSettings.referenceLevel;
    pSettings->mode = piSettings.mode;
    BDBG_CASSERT(BAPE_TruVolumeMode_eMax == (BAPE_TruVolumeMode)NEXUS_TruVolumeMode_eMax);
    pSettings->speakerResolution = piSettings.speakerResolution;
    BDBG_CASSERT(NEXUS_TruVolumeSpeakerResolution_eMax == (NEXUS_TruVolumeSpeakerResolution)BAPE_TruVolumeSpeakerResolution_eMax);
    pSettings->maxGain = piSettings.maxGain;
    pSettings->enableDcNotchFilter = piSettings.enableDcNotchFilter;
    pSettings->enableNoiseManager = piSettings.enableNoiseManager;
    pSettings->noiseManagerThreshold = piSettings.noiseManagerThreshold;
    pSettings->enableNormalizer = piSettings.enableNormalizer;
    pSettings->calibrate = piSettings.calibrate;
    
    /* HPF */
    pSettings->highPassFilter.enabled = piSettings.highPassFilter.enabled;
    BDBG_CASSERT((int)NEXUS_SrsFilterCoefGenMode_eFilterUser == (int)BAPE_SrsFilterCoefficientMode_eUser);
    BDBG_CASSERT((int)NEXUS_SrsFilterCoefGenMode_eFilterSpec == (int)BAPE_SrsFilterCoefficientMode_eSpecification);
    BDBG_CASSERT((int)NEXUS_SrsFilterCoefGenMode_eMax == (int)BAPE_SrsFilterCoefficientMode_eMax);
    pSettings->highPassFilter.coefGenMode = (NEXUS_SrsFilterCoefGenMode)piSettings.highPassFilter.coefficientMode;
    if ( pSettings->highPassFilter.coefGenMode == NEXUS_SrsFilterCoefGenMode_eFilterUser )
    {
        for ( i = 0; i < 3; i++ )
        {
            pSettings->highPassFilter.coefParam.highPassFilterCoefUser[i].filterOrder = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.user[i].filterOrder;
            for ( j = 0; j < 3; j++ )
            {
                pSettings->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].scale = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.user[i].coefficients[j].scale;
                pSettings->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientB0 = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB0;
                pSettings->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientB1 = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB1;
                pSettings->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientB2 = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB2;
                pSettings->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientA1 = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA1;
                pSettings->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientA2 = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA2;
            }
        }
    }
    else
    {
        pSettings->highPassFilter.coefParam.highPassFilterCoefSpec.cutOffFrequency = piSettings.highPassFilter.coefficientSettings.specification.cutoffFrequency;
        pSettings->highPassFilter.coefParam.highPassFilterCoefSpec.filterOrder = (NEXUS_SrsFilterOrder)piSettings.highPassFilter.coefficientSettings.specification.filterOrder;
    }    
}

NEXUS_TruVolumeHandle NEXUS_TruVolume_Open(
    const NEXUS_TruVolumeSettings *pSettings     /* Pass NULL for default settings */
    )
{
    NEXUS_TruVolumeHandle handle;
    BAPE_TruVolumeSettings defaults;
    BAPE_Connector connector;
    BERR_Code errCode;
    handle = BKNI_Malloc(sizeof(NEXUS_TruVolume));
    if ( NULL == handle )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_TruVolume, handle);
    BKNI_Snprintf(handle->name, sizeof(handle->name), "TRU VOLUME");
    NEXUS_AUDIO_INPUT_INIT(&handle->connector, NEXUS_AudioInputType_eTruVolume, handle);
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &handle->connector, Open);
    handle->connector.pName = handle->name;
    handle->connector.format = NEXUS_AudioInputFormat_eNone;    /* Determined by inputs */
    BAPE_TruVolume_GetDefaultSettings(&defaults);
    errCode = BAPE_TruVolume_Create(NEXUS_AUDIO_DEVICE_HANDLE, &defaults, &handle->apeHandle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        BDBG_OBJECT_DESTROY(handle, NEXUS_TruVolume);
        BKNI_Free(handle);
        return NULL;
    }
    handle->connector.format = NEXUS_AudioInputFormat_ePcmStereo;
    BAPE_TruVolume_GetConnector(handle->apeHandle, &connector);
    handle->connector.port = (size_t)connector;
    if ( NULL == pSettings )
    {
        NEXUS_TruVolume_GetDefaultSettings(&handle->settings);
    }
    else
    {
        (void)NEXUS_TruVolume_SetSettings(handle, pSettings);
    }

    return handle;
}

static void NEXUS_TruVolume_P_Finalizer(
    NEXUS_TruVolumeHandle handle
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_TruVolume, handle);
    NEXUS_AudioInput_Shutdown(&handle->connector);
    BAPE_TruVolume_Destroy(handle->apeHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_TruVolume, handle);
    BKNI_Free(handle);
}

static void NEXUS_TruVolume_P_Release(NEXUS_TruVolumeHandle handle)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &handle->connector, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_TruVolume, NEXUS_TruVolume_Close);

void NEXUS_TruVolume_GetSettings(
    NEXUS_TruVolumeHandle handle,
    NEXUS_TruVolumeSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != pSettings);
    
    BKNI_Memcpy(pSettings, &handle->settings, sizeof(NEXUS_TruVolumeSettings));
}

void NEXUS_TruVolume_P_ConvertSettings(
    const NEXUS_TruVolumeSettings *pNexus, 
    BAPE_TruVolumeSettings *pMagnum
    )
{
    unsigned i, j;
    
    BDBG_ASSERT(NULL != pNexus);
    BDBG_ASSERT(NULL != pMagnum);
    
    pMagnum->enabled = pNexus->enabled;
    pMagnum->blockSize = pNexus->blockSize;
    pMagnum->enableNormalGain = pNexus->enableNormalGain;
    pMagnum->inputGain = pNexus->inputGain;
    pMagnum->outputGain = pNexus->outputGain;
    pMagnum->bypassGain = pNexus->bypassGain;
    pMagnum->referenceLevel = pNexus->referenceLevel;
    pMagnum->mode = pNexus->mode;
    pMagnum->speakerResolution = pNexus->speakerResolution;
    pMagnum->maxGain = pNexus->maxGain;
    pMagnum->enableDcNotchFilter = pNexus->enableDcNotchFilter;
    pMagnum->enableNoiseManager = pNexus->enableNoiseManager;
    pMagnum->noiseManagerThreshold = pNexus->noiseManagerThreshold;
    pMagnum->enableNormalizer = pNexus->enableNormalizer;
    pMagnum->calibrate = pNexus->calibrate;
    pMagnum->highPassFilter.enabled = pNexus->highPassFilter.enabled;
    BDBG_CASSERT((int)BAPE_SrsFilterCoefficientMode_eUser == (int)NEXUS_SrsFilterCoefGenMode_eFilterUser);
    BDBG_CASSERT((int)BAPE_SrsFilterCoefficientMode_eSpecification == (int)NEXUS_SrsFilterCoefGenMode_eFilterSpec);
    BDBG_CASSERT((int)BAPE_SrsFilterCoefficientMode_eMax == (int)NEXUS_SrsFilterCoefGenMode_eMax);
    pMagnum->highPassFilter.coefficientMode = (BAPE_SrsFilterCoefficientMode)pNexus->highPassFilter.coefGenMode;
    if ( pNexus->highPassFilter.coefGenMode == NEXUS_SrsFilterCoefGenMode_eFilterUser )
    {
        for ( i = 0; i < 3; i++ )
        {
            pMagnum->highPassFilter.coefficientSettings.user[i].filterOrder = (BAPE_SrsFilterOrder)pNexus->highPassFilter.coefParam.highPassFilterCoefUser[i].filterOrder;    
            for ( j = 0; j < 3; j++ )
            {
                pMagnum->highPassFilter.coefficientSettings.user[i].coefficients[j].scale = pNexus->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].scale;    
                pMagnum->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB0 = pNexus->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientB0;    
                pMagnum->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB1 = pNexus->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientB1;    
                pMagnum->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB2 = pNexus->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientB2;    
                pMagnum->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA1 = pNexus->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientA1;    
                pMagnum->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA2 = pNexus->highPassFilter.coefParam.highPassFilterCoefUser[i].coefficients[j].coefficientA2;    
            }
        }        
    }
    else
    {
        pMagnum->highPassFilter.coefficientSettings.specification.cutoffFrequency = pNexus->highPassFilter.coefParam.highPassFilterCoefSpec.cutOffFrequency;
        pMagnum->highPassFilter.coefficientSettings.specification.filterOrder = (BAPE_SrsFilterOrder)pNexus->highPassFilter.coefParam.highPassFilterCoefSpec.filterOrder;
    }
}

NEXUS_Error NEXUS_TruVolume_SetSettings(
    NEXUS_TruVolumeHandle handle,
    const NEXUS_TruVolumeSettings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_TruVolumeSettings piSettings;
    
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != pSettings);

    BAPE_TruVolume_GetSettings(handle->apeHandle, &piSettings);

    NEXUS_TruVolume_P_ConvertSettings(pSettings, &piSettings);
    
    errCode = BAPE_TruVolume_SetSettings(handle->apeHandle, &piSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(NEXUS_TruVolumeSettings));
    return BERR_SUCCESS;
}

NEXUS_AudioInputHandle NEXUS_TruVolume_GetConnector(
    NEXUS_TruVolumeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    return &handle->connector;
}

NEXUS_Error NEXUS_TruVolume_AddInput(
    NEXUS_TruVolumeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != input);
    if ( NULL != handle->input )
    {
        BDBG_ERR(("Only one input can be added"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    NEXUS_AUDIO_INPUT_CHECK_FROM_DSP(input);
    errCode = BAPE_TruVolume_AddInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_AddInput(&handle->connector, input);
    if ( errCode )
    {
        (void)BAPE_TruVolume_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
        return BERR_TRACE(errCode);
    }
    handle->input = input;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_TruVolume_RemoveInput(
    NEXUS_TruVolumeHandle handle,
    NEXUS_AudioInputHandle input
    )
{
    NEXUS_Error errCode;
    BDBG_OBJECT_ASSERT(handle, NEXUS_TruVolume);
    BDBG_ASSERT(NULL != handle->input);
    if ( input != handle->input )
    {
        BDBG_ERR(("Input not connected"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_TruVolume_RemoveInput(handle->apeHandle, (BAPE_Connector)input->port);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    errCode = NEXUS_AudioInput_P_RemoveInput(&handle->connector, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = NULL;
    return BERR_SUCCESS;
}

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
