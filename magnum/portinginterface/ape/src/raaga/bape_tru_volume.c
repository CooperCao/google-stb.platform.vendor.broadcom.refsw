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
* API Description:
*   API name: TruVolume
*    Specific APIs related to SRS TruVolume (formerly Volume IQ) Audio Processing
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#if BAPE_CHIP_HAS_POST_PROCESSING
#include "bdsp_raaga.h"
#endif

BDBG_MODULE(bape_tru_volume);

BDBG_OBJECT_ID(BAPE_TruVolume);

#if BAPE_CHIP_HAS_POST_PROCESSING
typedef struct BAPE_TruVolume
{
    BDBG_OBJECT(BAPE_TruVolume)
    BAPE_PathNode node;
    BAPE_TruVolumeSettings settings;
    BAPE_Connector input;
    BDSP_StageHandle hStage;
} BAPE_TruVolume;

static BERR_Code BAPE_TruVolume_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_TruVolume_P_ApplyTruVolumeSettings(BAPE_TruVolumeHandle handle);
static void BAPE_TruVolume_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_TruVolume_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

/***************************************************************************
Summary:
    Get default settings for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetDefaultSettings(
    BAPE_TruVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BDSP_Raaga_Audio_TruVolumeUserConfig dspSettings;
    unsigned i, j;

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BDSP_Raaga_GetDefaultAlgorithmSettings(BDSP_Algorithm_eSrsTruVolume, 
                                           (void *)&dspSettings, 
                                           sizeof(BDSP_Raaga_Audio_TruVolumeUserConfig)
                                           );

    pSettings->enabled = dspSettings.i32TruVolume_enable?true:false;

    switch (dspSettings.i32blockSize)
    {
    case 256:
        pSettings->blockSize = BAPE_TruVolumeBlockSize_e256;
        break;
    case 512:
        pSettings->blockSize = BAPE_TruVolumeBlockSize_e512;
        break;
    case 768:
        pSettings->blockSize = BAPE_TruVolumeBlockSize_e768;
        break;
    case 1024:
        pSettings->blockSize = BAPE_TruVolumeBlockSize_e1024;
        break;
    default:
        pSettings->blockSize = BAPE_TruVolumeBlockSize_eMax;
        break;
    }

    pSettings->enableNormalGain = dspSettings.i32mEnable?true:false;
    pSettings->inputGain = 100;
    BDBG_ASSERT(dspSettings.i32mInputGain == 0x00200000);
    BDBG_ASSERT(dspSettings.i32mInputGain == (int)BAPE_P_FloatToQ521(pSettings->inputGain, 3200));
    pSettings->outputGain = 100;
    BDBG_ASSERT(dspSettings.i32mOutputGain == 0x00200000);
    BDBG_ASSERT(dspSettings.i32mOutputGain == (int)BAPE_P_FloatToQ521(pSettings->outputGain, 3200));
    pSettings->bypassGain = 100;
    BDBG_ASSERT(0x007fffff == dspSettings.i32mBypassGain);
    pSettings->referenceLevel = 50;
    BDBG_ASSERT(0x00400000 == dspSettings.i32mReferenceLevel);


    switch (dspSettings.i32mMode)
    {
    case 0:
        pSettings->mode = BAPE_TruVolumeMode_eNormal;
        break;
    case 1:
        pSettings->mode = BAPE_TruVolumeMode_eLight;
        break;
    default:
        pSettings->mode = BAPE_TruVolumeMode_eMax;
        break;
    }

    switch (dspSettings.i32mSize)
    {
    case 0:
        pSettings->speakerResolution = BAPE_TruVolumeSpeakerResolution_e20Hz;
        break;
    case 1:
        pSettings->speakerResolution = BAPE_TruVolumeSpeakerResolution_e40Hz;
        break;
    case 2:
        pSettings->speakerResolution = BAPE_TruVolumeSpeakerResolution_e110Hz;
        break;
    case 3:
        pSettings->speakerResolution = BAPE_TruVolumeSpeakerResolution_e200Hz;
        break;
    case 4:
        pSettings->speakerResolution = BAPE_TruVolumeSpeakerResolution_e315Hz;
        break;
    case 5:
        pSettings->speakerResolution = BAPE_TruVolumeSpeakerResolution_e410Hz;
        break;
    default:
        pSettings->speakerResolution = BAPE_TruVolumeSpeakerResolution_eMax;
        break;
    }

    pSettings->maxGain = 1600;
    BDBG_ASSERT(0x00080000 == dspSettings.i32mMaxGain);
    pSettings->enableDcNotchFilter = dspSettings.i32EnableDCNotchFilter?true:false;
    pSettings->enableNoiseManager = dspSettings.i32mNoiseManager?true:false;
    pSettings->noiseManagerThreshold = 10;
    BDBG_ASSERT(0x000ccccd == dspSettings.i32mNoiseManagerThreshold);
    pSettings->enableNormalizer = dspSettings.i32mNormalizerEnable?true:false;
    pSettings->calibrate = 100;
    BDBG_ASSERT(0x8000 == dspSettings.i32mCalibrate);

    /* Top-level HPF settings */
    pSettings->highPassFilter.enabled = (dspSettings.sHighPassFilterConfig.ui32mEnable)?true:false;
    switch ( dspSettings.sHighPassFilterConfig.ui32CoefGenMode )
    {
    default:
        BDBG_WRN(("Unrecognized filter coefficient gen mode in default truvolume settings."));
        /* fall through */
    case 0:
        pSettings->highPassFilter.coefficientMode = BAPE_SrsFilterCoefficientMode_eUser;        
        break;
    case 1:
        pSettings->highPassFilter.coefficientMode = BAPE_SrsFilterCoefficientMode_eSpecification;
        break;
    }
    
    /* User Coefs */
    BDBG_CASSERT((sizeof(dspSettings.sHighPassFilterConfig.sFilterCoefHpf)/sizeof(BDSP_Raaga_FilterCoefHpf)) == 3);
    for ( i = 0; i < 3; i++ )
    {
        pSettings->highPassFilter.coefficientSettings.user[i].filterOrder = dspSettings.sHighPassFilterConfig.sFilterCoefHpf[i].ui32Order;
        BDBG_CASSERT((sizeof(dspSettings.sHighPassFilterConfig.sFilterCoefHpf[0].sFilterCoefHpfTdf2)/sizeof(BDSP_Raaga_FilterCoefHpfTdf2)) == 3);
        for ( j = 0; j < 3; j++ )
        {
            pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].scale = dspSettings.sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32Scale;
            pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB0 = dspSettings.sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB0;
            pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB1 = dspSettings.sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB1;
            pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB2 = dspSettings.sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB2;
            pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA1 = dspSettings.sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB1;
            pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA2 = dspSettings.sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientA2;
        }
    }

    /* Specification */
    pSettings->highPassFilter.coefficientSettings.specification.cutoffFrequency = dspSettings.sHighPassFilterConfig.sFilterSpecHpf.ui32CutoffFrequency;
    pSettings->highPassFilter.coefficientSettings.specification.filterOrder = dspSettings.sHighPassFilterConfig.sFilterSpecHpf.ui32Order;
}

/***************************************************************************
Summary:
    Open an SRS TruVolume stage
***************************************************************************/
BERR_Code BAPE_TruVolume_Create(
                               BAPE_Handle deviceHandle,
                               const BAPE_TruVolumeSettings *pSettings,
                               BAPE_TruVolumeHandle *pHandle
                               )
{
    BAPE_TruVolumeHandle handle;
    BDSP_StageCreateSettings stageCreateSettings;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);


    handle = BKNI_Malloc(sizeof(BAPE_TruVolume));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_TruVolume));
    BDBG_OBJECT_SET(handle, BAPE_TruVolume);
    handle->settings = *pSettings;
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eTruVolume, 1, deviceHandle, handle);
    handle->node.pName = "TruVolume";
    handle->node.connectors[0].useBufferPool = true;    

    BAPE_Connector_P_GetFormat(&handle->node.connectors[0], &format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[0], &format);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_format; }

    BAPE_PathNode_P_GetInputCapabilities(&handle->node, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->node, &caps);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_caps; }

    /* Generic Routines */
    handle->node.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->node.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->node.startPathToOutput = BAPE_DSP_P_StartPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;

    /* TruVolume Specifics */
    handle->node.allocatePathFromInput = BAPE_TruVolume_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_TruVolume_P_StopPathFromInput;
    handle->node.removeInput = BAPE_TruVolume_P_RemoveInputCallback;

    /* Create Stage Handle */
    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eSrsTruVolume] = true;
    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage_create;
    }
    handle->node.connectors[0].hStage = handle->hStage;

    *pHandle = handle;
    return BERR_SUCCESS;

err_stage_create:
err_caps:
err_format:
    BAPE_TruVolume_Destroy(handle);
    return errCode;
}


/***************************************************************************
Summary:
    Close an SRS TruVolume stage
    
Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_TruVolume_Destroy(
                           BAPE_TruVolumeHandle handle
                           )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    BDBG_ASSERT(false == BAPE_PathNode_P_IsActive(&handle->node));
    BDBG_ASSERT(NULL == handle->input);
    if ( handle->hStage )
    {
        BDSP_Stage_Destroy(handle->hStage);
    }
    BDBG_OBJECT_DESTROY(handle, BAPE_TruVolume);
    BKNI_Free(handle);
}

/***************************************************************************
Summary:
    Get Settings for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetSettings(
                               BAPE_TruVolumeHandle handle,
                               BAPE_TruVolumeSettings *pSettings    /* [out] Settings */
                               )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}


/***************************************************************************
Summary:
    Set Settings for an SRS TruVolume stage
***************************************************************************/
BERR_Code BAPE_TruVolume_SetSettings(
                                    BAPE_TruVolumeHandle handle,
                                    const BAPE_TruVolumeSettings *pSettings
                                    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    BDBG_ASSERT(NULL != pSettings);

    /* Start time will always call this routine, don't copy the settings if we don't need to */
    if ( pSettings != &handle->settings )
    {
        handle->settings = *pSettings;
    }

    errCode = BAPE_TruVolume_P_ApplyTruVolumeSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
    Get the audio connector for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetConnector(
                                BAPE_TruVolumeHandle handle,
                                BAPE_Connector *pConnector
                                )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    BDBG_ASSERT(NULL != pConnector);
    *pConnector = &handle->node.connectors[0];
}


/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_AddInput(
                                 BAPE_TruVolumeHandle handle,
                                 BAPE_Connector input
                                 )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( NULL != handle->input )
    {
        BDBG_ERR(("Can not have more than one input"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    errCode = BAPE_PathNode_P_AddInput(&handle->node, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = input;
    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_RemoveInput(
                                    BAPE_TruVolumeHandle handle,
                                    BAPE_Connector input
                                    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    if ( input != handle->input )
    {
        BDBG_ERR(("Input %s %s (%p) is not connected", input->pParent->pName, input->pName, (void *)input));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    errCode = BAPE_PathNode_P_RemoveInput(&handle->node, input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    handle->input = NULL;
    return BERR_SUCCESS;
}


/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_RemoveAllInputs(
                                        BAPE_TruVolumeHandle handle
                                        )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    if ( handle->input )
    {
        return BAPE_TruVolume_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_TruVolume_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    unsigned output, input;
    BAPE_TruVolumeHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage, 
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->input),
                                        handle->hStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

BERR_Code BAPE_TruVolume_P_ConvertSettingsToDsp(
    const BAPE_TruVolumeSettings *pSettings, 
    BDSP_Raaga_Audio_TruVolumeUserConfig *pUserConfig
    )
{
    unsigned i, j;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pUserConfig);

    BDBG_MSG(("BlockSize was %u", pUserConfig->i32blockSize));
    switch (pSettings->blockSize)
    {
    case BAPE_TruVolumeBlockSize_e256:
        pUserConfig->i32blockSize = 256;
        break;
    case BAPE_TruVolumeBlockSize_e512:
        pUserConfig->i32blockSize = 512;
        break;
    case BAPE_TruVolumeBlockSize_e768:
        pUserConfig->i32blockSize = 768;
        break;
    case BAPE_TruVolumeBlockSize_e1024:
        pUserConfig->i32blockSize = 1024;
        break;
    default:
        BDBG_MSG(("Incorrect value for TruVolume Blocksize. Possible values are 256, 512, 768, 1024"));
        break;
    }
    BDBG_MSG(("BlockSize now %u", pUserConfig->i32blockSize));
    BDBG_MSG(("i32EnableDCNotchFilter was %u", pUserConfig->i32EnableDCNotchFilter));
    pUserConfig->i32EnableDCNotchFilter = pSettings->enableDcNotchFilter?1:0;
    BDBG_MSG(("i32EnableDCNotchFilter now %u", pUserConfig->i32EnableDCNotchFilter));
    BDBG_MSG(("i32mBypassGain was %u", pUserConfig->i32mBypassGain));
    pUserConfig->i32mBypassGain = BAPE_P_FloatToQ923(pSettings->bypassGain, 100);
    BDBG_MSG(("i32mBypassGain now %u", pUserConfig->i32mBypassGain));
    BDBG_MSG(("i32mCalibrate was %u", pUserConfig->i32mCalibrate));
    pUserConfig->i32mCalibrate = BAPE_P_FloatToQ815(pSettings->calibrate, 25600);
    BDBG_MSG(("i32mCalibrate now %u", pUserConfig->i32mCalibrate));
    BDBG_MSG(("i32mEnable was %u", pUserConfig->i32mEnable));
    pUserConfig->i32mEnable = pSettings->enableNormalGain?1:0;
    BDBG_MSG(("i32mEnable now %u", pUserConfig->i32mEnable));
    BDBG_MSG(("i32mInputGain was %u", pUserConfig->i32mInputGain));
    pUserConfig->i32mInputGain = BAPE_P_FloatToQ521(pSettings->inputGain, 3200);
    BDBG_MSG(("i32mInputGain now %u", pUserConfig->i32mInputGain));
    BDBG_MSG(("i32mMaxGain was %u", pUserConfig->i32mMaxGain));
    pUserConfig->i32mMaxGain = BAPE_P_FloatToQ923(pSettings->maxGain, 25600);
    if ( pUserConfig->i32mMaxGain < 0x2000  )
    {
        pUserConfig->i32mMaxGain = 0x2000;
    }
    BDBG_MSG(("i32mMaxGain now %u", pUserConfig->i32mMaxGain));
    BDBG_MSG(("i32mMode was %u", pUserConfig->i32mMode));
    switch (pSettings->mode)
    {
    case BAPE_TruVolumeMode_eLight:
        pUserConfig->i32mMode = 1;
        break;
    case BAPE_TruVolumeMode_eNormal:
        pUserConfig->i32mMode = 0;
        break;
    case BAPE_TruVolumeMode_eHeavy:
        BDBG_ERR(("Heavy TruVolume mode is not supported in this SRS version"));
        pUserConfig->i32mMode = 2;
        break;
    default:
        BDBG_WRN(("Incorrect value for TruVolume Mode. Possible values are 0, 1"));
        break;
    }
    BDBG_MSG(("i32mMode now %u", pUserConfig->i32mMode));
    BDBG_MSG(("i32mNoiseManager was %u", pUserConfig->i32mNoiseManager));
    pUserConfig->i32mNoiseManager = pSettings->enableNoiseManager?1:0;
    BDBG_MSG(("i32mNoiseManager now %u", pUserConfig->i32mNoiseManager));
    BDBG_MSG(("i32mNoiseManagerThreshold was %u", pUserConfig->i32mNoiseManagerThreshold));
    pUserConfig->i32mNoiseManagerThreshold = BAPE_P_FloatToQ923(pSettings->noiseManagerThreshold, 100);
    BDBG_MSG(("i32mNoiseManagerThreshold now %u", pUserConfig->i32mNoiseManagerThreshold));
    BDBG_MSG(("i32mNormalizerEnable was %u", pUserConfig->i32mNormalizerEnable));
    pUserConfig->i32mNormalizerEnable = pSettings->enableNormalizer?1:0;
    BDBG_MSG(("i32mNormalizerEnable now %u", pUserConfig->i32mNormalizerEnable));
    BDBG_MSG(("i32mOutputGain was %u", pUserConfig->i32mOutputGain));
    pUserConfig->i32mOutputGain = BAPE_P_FloatToQ521(pSettings->outputGain, 3200);
    BDBG_MSG(("i32mOutputGain now %u", pUserConfig->i32mOutputGain));
    BDBG_MSG(("i32mReferenceLevel was %u", pUserConfig->i32mReferenceLevel));
    pUserConfig->i32mReferenceLevel = BAPE_P_FloatToQ923(pSettings->referenceLevel, 100);
    if ( pUserConfig->i32mReferenceLevel < 0x109 )
    {
        pUserConfig->i32mReferenceLevel = 0x109;
    }
    BDBG_MSG(("i32mReferenceLevel now %u", pUserConfig->i32mReferenceLevel));
    BDBG_MSG(("i32mSize was %u", pUserConfig->i32mSize));
    switch (pSettings->speakerResolution)
    {
    case BAPE_TruVolumeSpeakerResolution_e20Hz:
        pUserConfig->i32mSize = 0;
        break;
    case BAPE_TruVolumeSpeakerResolution_e40Hz:
        pUserConfig->i32mSize = 1;
        break;
    case BAPE_TruVolumeSpeakerResolution_e110Hz:
        pUserConfig->i32mSize = 2;
        break;
    case BAPE_TruVolumeSpeakerResolution_e200Hz:
        pUserConfig->i32mSize = 3;
        break;
    case BAPE_TruVolumeSpeakerResolution_e315Hz:
        pUserConfig->i32mSize = 4;
        break;
    case BAPE_TruVolumeSpeakerResolution_e410Hz:
        pUserConfig->i32mSize = 5;
        break;
    default:
        BDBG_WRN(("Incorrect value for TruVolume Resolution. Possible values are 0, 1, 2, 3, 4, 5"));
        break;
    }
    BDBG_MSG(("i32mSize now %u", pUserConfig->i32mSize));
    BDBG_MSG(("i32nchans was %u", pUserConfig->i32nchans));
    pUserConfig->i32nchans = 2;
    BDBG_MSG(("i32nchans now %u", pUserConfig->i32nchans));
    BDBG_MSG(("i32TruVolume_enable was %u", pUserConfig->i32TruVolume_enable));
    pUserConfig->i32TruVolume_enable = pSettings->enabled?1:0;
    BDBG_MSG(("i32TruVolume_enable now %u", pUserConfig->i32TruVolume_enable));

    /* Top-level HPF settings */
    pUserConfig->sHighPassFilterConfig.ui32mEnable = (pSettings->highPassFilter.enabled)?true:false;
    switch ( pSettings->highPassFilter.coefficientMode )
    {
    default:
        BDBG_WRN(("Unrecognized filter coefficient gen mode in truvolume settings."));
        /* fall through */
    case BAPE_SrsFilterCoefficientMode_eUser:
        pUserConfig->sHighPassFilterConfig.ui32CoefGenMode = 0;
        break;
    case BAPE_SrsFilterCoefficientMode_eSpecification:
        pUserConfig->sHighPassFilterConfig.ui32CoefGenMode = 1;
        break;
    }

    /* User Coefs */
    BDBG_CASSERT((sizeof(pUserConfig->sHighPassFilterConfig.sFilterCoefHpf)/sizeof(BDSP_Raaga_FilterCoefHpf)) == 3);
    for ( i = 0; i < 3; i++ )
    {
        pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[i].ui32Order = pSettings->highPassFilter.coefficientSettings.user[i].filterOrder;
        BDBG_CASSERT((sizeof(pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[0].sFilterCoefHpfTdf2)/sizeof(BDSP_Raaga_FilterCoefHpfTdf2)) == 3);
        for ( j = 0; j < 3; j++ )
        {
            pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32Scale = pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].scale;
            pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB0 = pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB0;
            pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB1 = pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB1;
            pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB2 = pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientB2;
            pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientB1 = pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA1;
            pUserConfig->sHighPassFilterConfig.sFilterCoefHpf[i].sFilterCoefHpfTdf2[j].i32FilterCoefficientA2 = pSettings->highPassFilter.coefficientSettings.user[i].coefficients[j].coefficientA2;
        }
    }

    /* Specification */
    pUserConfig->sHighPassFilterConfig.sFilterSpecHpf.ui32CutoffFrequency = pSettings->highPassFilter.coefficientSettings.specification.cutoffFrequency;
    pUserConfig->sHighPassFilterConfig.sFilterSpecHpf.ui32Order = pSettings->highPassFilter.coefficientSettings.specification.filterOrder;
    
    return BERR_SUCCESS;    
}

static BERR_Code BAPE_TruVolume_P_ApplyTruVolumeSettings(BAPE_TruVolumeHandle handle)
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_TruVolumeUserConfig userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    userConfig.sTopLevelConfig.i32mEnable = false;  /* Disable studio sound */
    errCode = BAPE_TruVolume_P_ConvertSettingsToDsp(&handle->settings, &userConfig);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BDSP_Stage_SetSettings(handle->hStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_TruVolume_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_TruVolumeHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_TruVolume);
    BDSP_Stage_RemoveAllInputs(handle->hStage);
    BDSP_Stage_RemoveAllOutputs(handle->hStage);
}

static void BAPE_TruVolume_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_TruVolume_RemoveInput(pNode->pHandle, pConnector);
}
#else
typedef struct BAPE_TruVolume
{
    BDBG_OBJECT(BAPE_TruVolume)
} BAPE_TruVolume;

void BAPE_TruVolume_GetDefaultSettings(
    BAPE_TruVolumeSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Open an SRS TruVolume stage
***************************************************************************/
BERR_Code BAPE_TruVolume_Create(
                               BAPE_Handle deviceHandle,
                               const BAPE_TruVolumeSettings *pSettings,
                               BAPE_TruVolumeHandle *pHandle
                               )
{
    BSTD_UNUSED(deviceHandle);

    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/***************************************************************************
Summary:
    Close an SRS TruVolume stage

Description:
    Input to the stage must be removed prior to closing.
***************************************************************************/
void BAPE_TruVolume_Destroy(
                           BAPE_TruVolumeHandle handle
                           )
{
    BSTD_UNUSED(handle);
}

/***************************************************************************
Summary:
    Get Settings for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetSettings(
                               BAPE_TruVolumeHandle handle,
                               BAPE_TruVolumeSettings *pSettings    /* [out] Settings */
                               )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}


/***************************************************************************
Summary:
    Set Settings for an SRS TruVolume stage
***************************************************************************/
BERR_Code BAPE_TruVolume_SetSettings(
                                    BAPE_TruVolumeHandle handle,
                                    const BAPE_TruVolumeSettings *pSettings
                                    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/***************************************************************************
Summary:
    Get the audio connector for an SRS TruVolume stage
***************************************************************************/
void BAPE_TruVolume_GetConnector(
                                BAPE_TruVolumeHandle handle,
                                BAPE_Connector *pConnector
                                )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pConnector);
}


/***************************************************************************
Summary:
Add an input to this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_AddInput(
                                 BAPE_TruVolumeHandle handle,
                                 BAPE_Connector input
                                 )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/***************************************************************************
Summary:
Remove an input from this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_RemoveInput(
                                    BAPE_TruVolumeHandle handle,
                                    BAPE_Connector input
                                    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


/***************************************************************************
Summary:
Remove all inputs from this processing stage
***************************************************************************/
BERR_Code BAPE_TruVolume_RemoveAllInputs(
                                        BAPE_TruVolumeHandle handle
                                        )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
