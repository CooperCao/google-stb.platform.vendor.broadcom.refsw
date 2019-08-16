/***************************************************************************
*  Copyright (C) 2019 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* API Description:
*   API name: DolbyDigitalReencode
*    Specific APIs related to Dolby Digital Reencoding used in Dolby MS11
*
***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#if BAPE_CHIP_HAS_POST_PROCESSING
#include "bdsp.h"
#endif

BDBG_MODULE(bape_ddre);

BDBG_OBJECT_ID(BAPE_DolbyDigitalReencode);

#if BAPE_CHIP_HAS_POST_PROCESSING
typedef struct BAPE_DolbyDigitalReencode
{
    BDBG_OBJECT(BAPE_DolbyDigitalReencode)
    BAPE_PathNode node;
    BAPE_DolbyDigitalReencodeSettings settings;
    BAPE_PathNodeOutputStatus outputStatus;
    BAPE_Connector input;
    BAPE_DecoderHandle masterDecoder;
    BDSP_StageHandle hRendererStage, hTranscodeStage;
    BDSP_Algorithm encodeAlgo, rendererAlgo;
    BAPE_DolbyMSVersion version;
    BAPE_Handle deviceHandle;
    BAPE_MixerHandle dspMixer;

    /* multi-device architecture */
    bool encoderTaskRequired;
    bool encoderMixerRequired;
    unsigned encoderDeviceIndex;
    BDSP_TaskHandle hEncoderTask;
    BDSP_StageHandle hMixerStage;
    BDSP_ContextHandle bdspEncoderContext;
    bool taskStarted;
} BAPE_DolbyDigitalReencode;

static BERR_Code BAPE_DolbyDigitalReencode_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
static BERR_Code BAPE_DolbyDigitalReencode_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_DolbyDigitalReencode_P_ApplyDspSettings(BAPE_DolbyDigitalReencodeHandle handle);
static void BAPE_DolbyDigitalReencode_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void BAPE_DolbyDigitalReencode_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);
static void BAPE_DolbyDigitalReencode_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate);

static bool BAPE_DolbyDigitalReencode_P_Has4xCompressedOutput(BAPE_DolbyDigitalReencodeHandle handle)
{
    return (handle->version == BAPE_DolbyMSVersion_eMS12 && (handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed4x].directConnections > 0)) ? true : false;
}

static bool BAPE_DolbyDigitalReencode_P_HasCompressedOutput(BAPE_DolbyDigitalReencodeHandle handle)
{
    return (handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed].directConnections > 0) ? true : false;
}

#if !BAPE_DSP_MS12_SUPPORT
static bool BAPE_DolbyDigitalReencode_P_HasMultichannelOutput(BAPE_DolbyDigitalReencodeHandle handle)
{
    return (handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections > 0) ? true : false;
}

static bool BAPE_DolbyDigitalReencode_P_HasStereoOutput(BAPE_DolbyDigitalReencodeHandle handle)
{
    return (handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].directConnections > 0) ? true : false;
}
#endif

void BAPE_DolbyDigitalReencode_GetDefaultSettings(
    BAPE_DolbyDigitalReencodeSettings *pSettings   /* [out] default settings */
    )
{
    #if BAPE_DSP_MS12_SUPPORT
    BDSP_Raaga_Audio_DpcmrConfigParams * pRendererStageSettings;
    BDSP_Raaga_Audio_DDPEncConfigParams * pEncodeStageSettings;
    BDSP_Raaga_Audio_MixerDapv2ConfigParams * pDapv2Settings;
    #else
    BDSP_Raaga_Audio_DDReencodeConfigParams * pRendererStageSettings;
    BDSP_Raaga_Audio_DDTranscodeConfigParams * pEncodeStageSettings;
    #endif

    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    #if BAPE_DSP_MS12_SUPPORT
    pRendererStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DpcmrConfigParams));
    pEncodeStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DDPEncConfigParams));
    pDapv2Settings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_MixerDapv2ConfigParams));
    if ( pRendererStageSettings == NULL || pEncodeStageSettings == NULL || pDapv2Settings == NULL )
    {
        BDBG_ERR(("Unable to alloc memory for DDRE default structures (MS12)"));
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto cleanup;
    }
    #else
    pRendererStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DDReencodeConfigParams));
    pEncodeStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DDTranscodeConfigParams));
    if ( pRendererStageSettings == NULL || pEncodeStageSettings == NULL )
    {
        BDBG_ERR(("Unable to alloc memory for DDRE default structures"));
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto cleanup;
    }
    #endif

    #if BAPE_DSP_MS12_SUPPORT
    BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDpcmr, (void *)pRendererStageSettings, sizeof(BDSP_Raaga_Audio_DpcmrConfigParams));
    BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDDPEncode, (void *)pEncodeStageSettings, sizeof(BDSP_Raaga_Audio_DDPEncConfigParams));
    BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eMixerDapv2, (void *)pDapv2Settings, sizeof(BDSP_Raaga_Audio_MixerDapv2ConfigParams));
    pSettings->externalPcmMode = false; /* not supported in MS12 */
    pSettings->profile = (BAPE_DolbyDigitalReencodeProfile)pRendererStageSettings->ui32CompressorProfile;
    pSettings->centerMixLevel = (BAPE_Ac3CenterMixLevel)pEncodeStageSettings->ui32CenterMixLevel;
    pSettings->surroundMixLevel = (BAPE_Ac3SurroundMixLevel)pEncodeStageSettings->ui32SurroundMixLevel;
    pSettings->dolbySurround = (BAPE_Ac3DolbySurround)pEncodeStageSettings->ui32DolbySurroundMode;
    pSettings->fixedAtmosOutput = (pEncodeStageSettings->ui32AtmosLockEnabled == 1) ? true : false;
    pSettings->dialogLevel = pEncodeStageSettings->ui32DialNorm;
    pSettings->multichannelFormat = BAPE_P_DolbyCapabilities_MultichannelPcmFormat();
    pSettings->encodeSettings.certificationMode = (pEncodeStageSettings->ui32DolbyCertificationFlag)?false:true;
    pSettings->encodeSettings.spdifHeaderEnabled = true;
    pSettings->fixedEncoderFormat = (pDapv2Settings->ui32ChannelLockModeEnable == 1) ? true : false;
    #else
    BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eDdre, (void *)pRendererStageSettings, sizeof(BDSP_Raaga_Audio_DDReencodeConfigParams));
    BDSP_GetDefaultAlgorithmSettings(BDSP_Algorithm_eAc3Encode, (void *)pEncodeStageSettings, sizeof(BDSP_Raaga_Audio_DDTranscodeConfigParams));
    pSettings->externalPcmMode = (pRendererStageSettings->ui32ExternalPcmEnabled)?true:false;
    pSettings->profile = (BAPE_DolbyDigitalReencodeProfile)pRendererStageSettings->ui32CompProfile;
    pSettings->centerMixLevel = (BAPE_Ac3CenterMixLevel)pRendererStageSettings->ui32CmixLev;
    pSettings->surroundMixLevel = (BAPE_Ac3SurroundMixLevel)pRendererStageSettings->ui32SurmixLev;
    pSettings->dolbySurround = (BAPE_Ac3DolbySurround)pRendererStageSettings->ui32DsurMod;
    pSettings->dialogLevel = pRendererStageSettings->ui32DialNorm;
    pSettings->multichannelFormat = BAPE_MultichannelFormat_e5_1;
    pSettings->encodeSettings.certificationMode = (pEncodeStageSettings->eTranscodeEnable)?false:true;
    pSettings->encodeSettings.spdifHeaderEnabled = (pEncodeStageSettings->eSpdifHeaderEnable)?true:false;
    #endif

    pSettings->stereoOutputMode = BAPE_ChannelMode_e2_0;

    pSettings->drcMode = BAPE_DolbyDigitalReencodeDrcMode_eLine;
    pSettings->drcModeDownmix = BAPE_DolbyDigitalReencodeDrcMode_eRf;
    pSettings->drcScaleHi = 100;
    pSettings->drcScaleLow = 100;
    pSettings->drcScaleHiDownmix = 100;
    pSettings->drcScaleLowDownmix = 100;
    pSettings->loudnessEquivalenceEnabled = true;
    pSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eLtRt;
    pSettings->dualMonoMode = BAPE_DualMonoMode_eStereo;

cleanup:
    if ( pRendererStageSettings )
    {
        BKNI_Free(pRendererStageSettings);
    }
    if ( pEncodeStageSettings )
    {
        BKNI_Free(pEncodeStageSettings);
    }
    #if BAPE_DSP_MS12_SUPPORT
    if ( pDapv2Settings )
    {
        BKNI_Free(pDapv2Settings);
    }
    #endif
}

/***************************************************************************
Summary:
    Open an SRS DolbyDigitalReencode stage
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DolbyDigitalReencodeSettings *pSettings,
    BAPE_DolbyDigitalReencodeHandle *pHandle
    )
{
    BAPE_DolbyDigitalReencodeHandle handle;
    BAPE_DolbyDigitalReencodeSettings defaults;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_ContextHandle bdspEncoderContext = NULL;
    unsigned dspIndexBase = 0;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( NULL == pSettings )
    {
        pSettings = &defaults;
        BAPE_DolbyDigitalReencode_GetDefaultSettings(&defaults);
    }

    handle = BKNI_Malloc(sizeof(BAPE_DolbyDigitalReencode));
    if ( NULL == handle )
    {
        BDBG_ERR(("Unable to allocate memory for DDRE handle"));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BDBG_MSG(("Dolby Version %d", BAPE_P_GetDolbyMSVersion()));
    if ( BAPE_P_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        BDBG_MSG(("MS12 Config %d, multichannelFormat %d", BAPE_P_GetDolbyMS12Config(), pSettings->multichannelFormat));
    }

    /* MS12 Config A check */
    if ( pSettings->multichannelFormat == BAPE_MultichannelFormat_e7_1 &&
         BAPE_P_DolbyCapabilities_MultichannelPcmFormat() != BAPE_MultichannelFormat_e7_1 )
    {
        BDBG_WRN(("export BDSP_MS12_SUPPORT=A is required for 7.1ch DDRE support"));
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_teardown;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_DolbyDigitalReencode));
    BDBG_OBJECT_SET(handle, BAPE_DolbyDigitalReencode);
    handle->deviceHandle = deviceHandle;
    handle->version = BAPE_P_GetDolbyMSVersion();
    handle->settings = *pSettings;
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDdre, 4, deviceHandle, handle);
    handle->node.pName = "DDRE";
    handle->node.connectors[BAPE_ConnectorFormat_eStereo].pName = "stereo";
    handle->node.connectors[BAPE_ConnectorFormat_eStereo].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].pName = "multichannel";
    handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed].pName = "compressed";
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed].useBufferPool = true;
    if (handle->version == BAPE_DolbyMSVersion_eMS12)
    {
        handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].pName = "compressed4x";
        handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].useBufferPool = true;
    }

    BAPE_FMT_P_InitDescriptor(&format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    format.sampleRate = 48000;              /* DDRE output is fixed @ 48k */
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_teardown; }

    if ( pSettings->multichannelFormat == BAPE_MultichannelFormat_e7_1 )
    {
        format.type = BAPE_DataType_ePcm7_1;
    }
    else
    {
        format.type = BAPE_DataType_ePcm5_1;
    }
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_teardown; }

    format.type = BAPE_DataType_eIec61937;
    BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, BAVC_AudioCompressionStd_eAc3);
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_teardown; }

    if (handle->version == BAPE_DolbyMSVersion_eMS12)
    {
        format.type = BAPE_DataType_eIec61937x4;
        BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, BAVC_AudioCompressionStd_eAc3Plus);
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x], &format);
        if ( errCode ) { (void)BERR_TRACE(errCode); goto err_teardown; }
    }

    BAPE_PathNode_P_GetInputCapabilities(&handle->node, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
    if ( BAPE_P_DolbyCapabilities_MultichannelPcmFormat() == BAPE_MultichannelFormat_e7_1 )
    {
        BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm7_1);
    }
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->node, &caps);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_teardown; }

    /* Generic Routines */
    handle->node.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->node.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->node.startPathToOutput = BAPE_DSP_P_StartPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;

    /* DolbyDigitalReencode Specifics */
    handle->node.inputFormatChange = BAPE_DolbyDigitalReencode_P_InputFormatChange;
    handle->node.allocatePathFromInput = BAPE_DolbyDigitalReencode_P_AllocatePathFromInput;
    handle->node.stopPathFromInput = BAPE_DolbyDigitalReencode_P_StopPathFromInput;
    handle->node.removeInput = BAPE_DolbyDigitalReencode_P_RemoveInputCallback;
    handle->node.inputSampleRateChange_isr = BAPE_DolbyDigitalReencode_P_InputSampleRateChange_isr;

    handle->node.deviceContext = NULL; /* Will be initialized during Allocate sequence */

    /* Set Algo versions */
    if (handle->version == BAPE_DolbyMSVersion_eMS12 || handle->version == BAPE_DolbyMSVersion_eMS11Plus)
    {
        handle->encodeAlgo = BDSP_Algorithm_eDDPEncode;
        handle->rendererAlgo = BDSP_Algorithm_eDpcmr;
    }
    else
    {
        handle->encodeAlgo = BDSP_Algorithm_eAc3Encode;
        handle->rendererAlgo = BDSP_Algorithm_eDdre;
    }

    if ( handle->version == BAPE_DolbyMSVersion_eMS12 )
    {
        if ( pSettings->encoderTaskRequired )
        {
            handle->encoderTaskRequired = true;
            handle->encoderMixerRequired = pSettings->encoderMixerRequired;
            handle->encoderDeviceIndex = pSettings->encoderDeviceIndex;

            if ( handle->encoderDeviceIndex <= BAPE_DEVICE_DSP_LAST && deviceHandle->dspContext != NULL )
            {
                bdspEncoderContext = deviceHandle->dspContext;
                dspIndexBase = BAPE_DEVICE_DSP_FIRST;
            }
            else if ( handle->encoderDeviceIndex <= BAPE_DEVICE_ARM_LAST && deviceHandle->armContext != NULL )
            {
                bdspEncoderContext = deviceHandle->armContext;
                dspIndexBase = BAPE_DEVICE_ARM_FIRST;
            }
            else
            {
                BDBG_ERR(("Invalid BDSP device index %lu, or context not available - Raaga (%p), Arm (%p). Use %lu to %lu for Raaga and %lu to %lu for Arm.",
                          (unsigned long)handle->encoderDeviceIndex, (void*)deviceHandle->dspContext, (void*)deviceHandle->armContext,
                          (unsigned long)BAPE_DEVICE_DSP_FIRST, (unsigned long)BAPE_DEVICE_DSP_LAST,
                          (unsigned long)BAPE_DEVICE_ARM_FIRST, (unsigned long)BAPE_DEVICE_ARM_LAST));
                errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                goto err_teardown;
            }

            if ( bdspEncoderContext == NULL )
            {
                BDBG_ERR(("BDSP Context requested is not available"));
                errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                goto err_teardown;
            }

            handle->bdspEncoderContext = bdspEncoderContext;
        }
    }

    /* DSP + ARM case */
    if ( handle->encoderTaskRequired )
    {
        BDSP_TaskCreateSettings taskCreateSettings;
        BDSP_StageCreateSettings stageCreateSettings;

        /* create mixer task for dsp n */
        BDSP_Task_GetDefaultCreateSettings(handle->bdspEncoderContext, &taskCreateSettings);
        taskCreateSettings.masterTask = true;
        taskCreateSettings.numSrc = 3;
        taskCreateSettings.dspIndex = BAPE_DSP_DEVICE_INDEX(handle->encoderDeviceIndex, dspIndexBase);
        errCode = BDSP_Task_Create(handle->bdspEncoderContext, &taskCreateSettings, &handle->hEncoderTask);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_teardown;
        }

        if ( handle->encoderMixerRequired )
        {
            BDSP_Stage_GetDefaultCreateSettings(handle->bdspEncoderContext, BDSP_AlgorithmType_eAudioMixer, &stageCreateSettings);
            BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
            stageCreateSettings.algorithmSupported[BAPE_P_GetCodecMixer()] = true;
            errCode = BDSP_Stage_Create(handle->bdspEncoderContext, &stageCreateSettings, &handle->hMixerStage);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_teardown;
            }

            errCode = BDSP_Stage_SetAlgorithm(handle->hMixerStage, BAPE_P_GetCodecMixer());
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_teardown;
            }
        }
    }

    *pHandle = handle;
    return BERR_SUCCESS;

err_teardown:
    BAPE_DolbyDigitalReencode_Destroy(handle);
    return errCode;
}

void BAPE_DolbyDigitalReencode_Destroy(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);
    BDBG_ASSERT(false == BAPE_PathNode_P_IsActive(&handle->node));
    BDBG_ASSERT(NULL == handle->input);

    if (handle->hMixerStage)
    {
        BDSP_Stage_Destroy(handle->hMixerStage);
        handle->hMixerStage = NULL;
    }

    if (handle->hEncoderTask)
    {
        BDSP_Task_Destroy(handle->hEncoderTask);
        handle->hEncoderTask = NULL;
    }

    if ( handle->hRendererStage )
    {
        BDSP_Stage_Destroy(handle->hRendererStage);
        handle->hRendererStage = NULL;
    }

    if ( handle->hTranscodeStage )
    {
        BDSP_Stage_Destroy(handle->hTranscodeStage);
        handle->hTranscodeStage = NULL;
    }

    BDBG_OBJECT_DESTROY(handle, BAPE_DolbyDigitalReencode);
    BKNI_Free(handle);
}

void BAPE_DolbyDigitalReencode_GetSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DolbyDigitalReencodeSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_DolbyDigitalReencode_SetSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    const BAPE_DolbyDigitalReencodeSettings *pSettings
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings != &handle->settings )
    {
        handle->settings = *pSettings;
    }

    errCode = BAPE_DolbyDigitalReencode_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


void BAPE_DolbyDigitalReencode_GetConnector(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);
    BDBG_ASSERT(NULL != pConnector);
    switch ( format )
    {
    case BAPE_ConnectorFormat_eStereo:
    case BAPE_ConnectorFormat_eMultichannel:
    case BAPE_ConnectorFormat_eCompressed:
        /* Valid */
        break;
    case BAPE_ConnectorFormat_eCompressed4x:
        {
            if ( !BAPE_P_DolbyCapabilities_DdpEncode(BAPE_MultichannelFormat_e5_1) )
            {
                BDBG_MSG(("Connector format %u is only supported by MS12 Config A or B BAPE_DolbyDigitalReencode (export BDSP_MS12_SUPPORT=A/B)", format));
                *pConnector = NULL;
                return;
            }
            /* else Valid */
        }
        break;
    default:
        BDBG_ERR(("Connector format %u not supported by BAPE_DolbyDigitalReencode", format));
        *pConnector = NULL;
        return;
    }
    *pConnector = &handle->node.connectors[format];
}


BERR_Code BAPE_DolbyDigitalReencode_AddInput(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);
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


BERR_Code BAPE_DolbyDigitalReencode_RemoveInput(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_Connector input
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);
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


BERR_Code BAPE_DolbyDigitalReencode_RemoveAllInputs(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);
    if ( handle->input )
    {
        return BAPE_DolbyDigitalReencode_RemoveInput(handle, handle->input);
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_DolbyDigitalReencode_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat)
{
    BAPE_DolbyDigitalReencodeHandle handle;
    BAPE_FMT_Descriptor outputFormat;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pNewFormat);

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);

    /* DDRE output formats are fixed.  All other fields are passthrough. */
    outputFormat = *pNewFormat;
    outputFormat.type = BAPE_DataType_ePcmStereo;
    outputFormat.sampleRate = 48000;              /* DDRE output is fixed @ 48k */
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eStereo], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    outputFormat = *pNewFormat;
    if ( handle->settings.multichannelFormat == BAPE_MultichannelFormat_e7_1 )
    {
        outputFormat.type = BAPE_DataType_ePcm7_1;
    }
    else
    {
        outputFormat.type = BAPE_DataType_ePcm5_1;
    }
    outputFormat.sampleRate = 48000;              /* DDRE output is fixed @ 48k */
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eMultichannel], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    outputFormat = *pNewFormat;
    outputFormat.type = BAPE_DataType_eIec61937;
    outputFormat.sampleRate = 48000;              /* DDRE output is fixed @ 48k */
    BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&outputFormat, BAVC_AudioCompressionStd_eAc3);
    errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eCompressed], &outputFormat);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    if (handle->version == BAPE_DolbyMSVersion_eMS12)
    {
        outputFormat = *pNewFormat;
        outputFormat.type = BAPE_DataType_eIec61937x4;
        outputFormat.sampleRate = 192000;         /* DDRE 4x output is fixed @ 192k */
        BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&outputFormat, BAVC_AudioCompressionStd_eAc3Plus);
        errCode = BAPE_Connector_P_SetFormat(&pNode->connectors[BAPE_ConnectorFormat_eCompressed4x], &outputFormat);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    return BERR_SUCCESS;
}

static BERR_Code BAPE_DolbyDigitalReencode_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    unsigned input, output;
    BAPE_DolbyDigitalReencodeHandle handle;
    BDSP_ContextHandle bdspEncoderContext = NULL;
    BDSP_ContextHandle bdspRendererContext = NULL;
    BDSP_StageCreateSettings stageCreateSettings;
    BAPE_PathNode *pNodes[2];
    unsigned numFound = 0;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_ASSERT(NULL != pConnection->pSource->hStage);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);

    if ( pConnection->pSource->pParent->deviceContext == NULL )
    {
        BDBG_ERR(("Upstream node (%s) has NULL dspContext", pConnection->pSource->pParent->pName));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }


    /* DSP + ARM case */
    if ( handle->encoderTaskRequired )
    {
        bdspEncoderContext = handle->bdspEncoderContext;
        bdspRendererContext = pConnection->pSource->pParent->deviceContext;
        pNode->deviceContext = bdspRendererContext;
    }
    else /* Single Device Case */
    {
        bdspRendererContext = pConnection->pSource->pParent->deviceContext;
        pNode->deviceContext = bdspEncoderContext = bdspRendererContext;
    }

    /* make sure that the source node's context matches our context for the renderer.
       Since the renderer is a slave stage, the context must match the parent. */
    if ( pConnection->pSource->pParent->deviceContext != bdspRendererContext && bdspRendererContext != NULL )
    {
        BDBG_ERR(("Upstream node (%s) device context (%p) does not match our renderer context (%p)", pConnection->pSource->pParent->pName, pConnection->pSource->pParent->deviceContext, pNode->deviceContext));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Create Stages */
    BDSP_Stage_GetDefaultCreateSettings(bdspRendererContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[handle->rendererAlgo] = true;
    errCode = BDSP_Stage_Create(bdspRendererContext, &stageCreateSettings, &handle->hRendererStage);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BDSP_Stage_GetDefaultCreateSettings(bdspEncoderContext, BDSP_AlgorithmType_eAudioEncode, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[handle->encodeAlgo] = true;
    errCode = BDSP_Stage_Create(bdspEncoderContext, &stageCreateSettings, &handle->hTranscodeStage);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* connect upstream -> Renderer */
    errCode = BDSP_Stage_AddOutputStage(pConnection->pSource->hStage,
                                        BAPE_DSP_P_GetDataTypeFromConnector(handle->input),
                                        handle->hRendererStage,
                                        &output, &input);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Set PCM connections to source data from Renderer */
    pNode->connectors[BAPE_ConnectorFormat_eStereo].hStage = handle->hRendererStage;
    pNode->connectors[BAPE_ConnectorFormat_eMultichannel].hStage = handle->hRendererStage;

    /* Determine number of outputs on compressed connector */
    BAPE_PathNode_P_GetOutputStatus(pNode, &handle->outputStatus);
    /* If we have at least one compressed consumer, add the transcoder */
    if ( BAPE_DolbyDigitalReencode_P_Has4xCompressedOutput(handle) ||
         BAPE_DolbyDigitalReencode_P_HasCompressedOutput(handle) )
    {
#define BAPE_DSP_TRANSCODE_TYPE_FOR_INTERTASK   1
        if ( handle->encoderTaskRequired )
        {
            unsigned index, input, output;
            /* connect renderer -> intertask buffer -> mixer */
            BDBG_MSG(("Multiple Device Architecture - Create Intertask buffer DspDataType %d", BAPE_FMT_P_GetDspDataType_isrsafe(&pConnection->pSource->format)));
            errCode = BDSP_InterTaskBuffer_Create(pNode->deviceContext,
                                                  #if BAPE_DSP_TRANSCODE_TYPE_FOR_INTERTASK
                                                  BDSP_DataType_eDolbyTranscodeData,
                                                  #else
                                                  BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource),
                                                  #endif
                                                  BDSP_BufferType_eRDB,
                                                  &pConnection->hInterTaskBuffer);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }

            /* BDSP_InterTaskBuffer_Flush(pConnection->hInterTaskBuffer); */

            /* render -> intertask */
            BDBG_MSG(("Multiple Device Architecture - connect Render -> Intertask"));
            #if BAPE_DSP_TRANSCODE_TYPE_FOR_INTERTASK
            errCode = BDSP_Stage_AddInterTaskBufferOutput(handle->hRendererStage, BDSP_DataType_eDolbyTranscodeData, pConnection->hInterTaskBuffer, &index);
            #else
            errCode = BDSP_Stage_AddInterTaskBufferOutput(handle->hRendererStage, BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource), pConnection->hInterTaskBuffer, &index);
            #endif
            if ( errCode )
            {
                BERR_TRACE(errCode);
                goto err_intertask;
            }
            if ( handle->encoderMixerRequired )
            {
                /* intertask -> mixer */
                BDBG_MSG(("Multiple Device Architecture - connect Intertask -> Mixer"));
                #if BAPE_DSP_TRANSCODE_TYPE_FOR_INTERTASK
                errCode = BDSP_Stage_AddInterTaskBufferInput(handle->hMixerStage, BDSP_DataType_eDolbyTranscodeData, pConnection->hInterTaskBuffer, &index);
                #else
                errCode = BDSP_Stage_AddInterTaskBufferInput(handle->hMixerStage, BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource), pConnection->hInterTaskBuffer, &index);
                #endif
                if ( errCode )
                {
                    BERR_TRACE(errCode);
                    goto err_intertask;
                }

                /* mixer -> transcoder */
                BDBG_MSG(("Multiple Device Architecture - connect Mixer -> Transcoder"));
                #if BAPE_DSP_TRANSCODE_TYPE_FOR_INTERTASK
                errCode = BDSP_Stage_AddOutputStage(handle->hMixerStage, BDSP_DataType_eDolbyTranscodeData, handle->hTranscodeStage, &output, &input);
                #else
                errCode = BDSP_Stage_AddOutputStage(handle->hMixerStage, BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource), handle->hTranscodeStage, &output, &input);
                #endif
                if ( errCode )
                {
                    BERR_TRACE(errCode);
                    goto err_stage;
                }
            }
            else
            {
                /* intertask -> transcoder */
                BDBG_MSG(("Multiple Device Architecture - connect Intertask -> Transcoder"));
                #if BAPE_DSP_TRANSCODE_TYPE_FOR_INTERTASK
                errCode = BDSP_Stage_AddInterTaskBufferInput(handle->hTranscodeStage, BDSP_DataType_eDolbyTranscodeData, pConnection->hInterTaskBuffer, &index);
                #else
                errCode = BDSP_Stage_AddInterTaskBufferInput(handle->hTranscodeStage, BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource), pConnection->hInterTaskBuffer, &index);
                #endif
                if ( errCode )
                {
                    BERR_TRACE(errCode);
                    goto err_intertask;
                }
            }
        }
        else /* MS11, Single Raaga Mode */
        {
            errCode = BDSP_Stage_AddOutputStage(handle->hRendererStage,
                                                BDSP_DataType_eDolbyTranscodeData,
                                                handle->hTranscodeStage,
                                                &output, &input);
            if ( errCode )
            {
                BERR_TRACE(errCode);
                goto err_stage_connections;
            }
        }
        pNode->connectors[BAPE_ConnectorFormat_eCompressed].hStage = handle->hTranscodeStage;
        if ( handle->version == BAPE_DolbyMSVersion_eMS12 )
        {
            pNode->connectors[BAPE_ConnectorFormat_eCompressed4x].hStage = handle->hTranscodeStage;
        }

        BAPE_PathNode_P_FindProducersBySubtype_isrsafe(&handle->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, pNodes);
        #if BAPE_DSP_MS12_SUPPORT
        switch ( numFound ) {
        default:
        case 0:
            break;
        case 1:
            handle->dspMixer = pNodes[0]->pHandle;
            break;
        }
        #endif

        errCode = BAPE_DolbyDigitalReencode_P_ApplyDspSettings(handle);
        if ( errCode )
        {
            BERR_TRACE(errCode);
            goto err_apply_settings;
        }

        if ( handle->encoderTaskRequired )
        {
            BDSP_TaskStartSettings taskStartSettings;
            bool disablePauseBursts = false;

            /* Prepare subnodes */
            errCode = BAPE_PathNode_P_AcquirePathResources(&handle->node);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_acquire_resources;
            }

            /* Link the path resources */
            errCode = BAPE_PathNode_P_ConfigurePathResources(&handle->node);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_config_path;
            }

            /* Start the consumers */
            errCode = BAPE_PathNode_P_StartPaths(&handle->node);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_start_path;
            }

            disablePauseBursts = BAPE_PathNode_P_PauseBurstDisabled(&handle->node);

            /* Setup Task and Start */
            BDSP_Task_GetDefaultStartSettings(handle->hEncoderTask, &taskStartSettings);
            if ( handle->encoderMixerRequired )
            {
                taskStartSettings.primaryStage = handle->hMixerStage;
            }
            else
            {
                taskStartSettings.primaryStage = handle->hTranscodeStage;
            }
            taskStartSettings.openGateAtStart = false;
            taskStartSettings.timeBaseType = BDSP_AF_P_TimeBaseType_e45Khz;
            taskStartSettings.schedulingMode = BDSP_TaskSchedulingMode_eStandalone;
            taskStartSettings.maxIndependentDelay = handle->deviceHandle->settings.maxIndependentDelay;
            taskStartSettings.gateOpenReqd = false; /* tells the BDSP task whether or not to start the FMM outputs during task start */

            if (disablePauseBursts == false) {
                taskStartSettings.eSpdifPauseWidth = BDSP_AF_P_SpdifPauseWidth_eHundredEightyEightWord;
                taskStartSettings.eFMMPauseBurstType = BDSP_AF_P_BurstFill_ePauseBurst;
            }
            else {
                taskStartSettings.eFMMPauseBurstType = BDSP_AF_P_BurstFill_eZeroes;
            }

            /* Start the DSP Task */
            BDBG_MSG(("Multiple Device Architecture - Starting Mixer/Encoder task"));
            errCode = BDSP_Task_Start(handle->hEncoderTask, &taskStartSettings);
            if ( errCode )
            {
                BERR_TRACE(errCode);
                goto err_task_start;
            }

            #if BAPE_MANUAL_DSP_CMD_BLOCKING_START
            if ( handle->deviceHandle->settings.dspManualCmdBlocking )
            {
                if ( handle->node.deviceContext )
                {
                    errCode = BDSP_Context_ProcessPing(handle->node.deviceContext);
                    if ( errCode )
                    {
                        BERR_TRACE(errCode);
                        goto err_task_start;
                    }
                }
                else
                {
                    BDBG_WRN(("No valid context found. Unable to wait for command ACK from DSP!!!"));
                    BERR_TRACE(BERR_UNKNOWN);
                    /* proceed anyway */
                }
            }
            #endif

            handle->taskStarted = true;
        }
    }
    else {
        BAPE_PathNode_P_FindProducersBySubtype_isrsafe(&handle->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, pNodes);
        #if BAPE_DSP_MS12_SUPPORT
        switch ( numFound ) {
        default:
        case 0:
            break;
        case 1:
            handle->dspMixer = pNodes[0]->pHandle;
            break;
        }
        #endif
    }
    errCode = BAPE_DolbyDigitalReencode_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;

err_task_start:
err_start_path:
err_config_path:
err_acquire_resources:
    BAPE_PathNode_P_ReleasePathResources(&handle->node);
err_apply_settings:
err_stage_connections:
err_stage:
err_intertask:
    if (pConnection->hInterTaskBuffer)
    {
        if ( handle->hMixerStage )
        {
            BDSP_Stage_RemoveAllInputs(handle->hMixerStage);
        }
        BDSP_Stage_RemoveAllOutputs(handle->hRendererStage);
        BDSP_InterTaskBuffer_Destroy(pConnection->hInterTaskBuffer);
        pConnection->hInterTaskBuffer = NULL;
    }
    return errCode;
}

static void BAPE_DolbyDigitalReencode_P_ApplyAc3DecoderSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DolbyDigitalReencodeSettings *ddreSettings,
    const BAPE_Ac3Settings *pSettings)
{
    BDBG_ASSERT(NULL != ddreSettings);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_MSG(("Applying master decoder AC3 settings"));

    switch (pSettings->drcMode) {
    default:
    case BAPE_Ac3DrcMode_eLine:
        ddreSettings->drcMode = BAPE_DolbyDigitalReencodeDrcMode_eLine;
        break;
    case BAPE_Ac3DrcMode_eRf:
        ddreSettings->drcMode = BAPE_DolbyDigitalReencodeDrcMode_eRf;
        break;
    }

    switch (pSettings->drcModeDownmix) {
    case BAPE_Ac3DrcMode_eLine:
        ddreSettings->drcModeDownmix = BAPE_DolbyDigitalReencodeDrcMode_eLine;
        break;
    default:
    case BAPE_Ac3DrcMode_eRf:
        ddreSettings->drcModeDownmix = BAPE_DolbyDigitalReencodeDrcMode_eRf;
        break;
    }

    ddreSettings->dualMonoMode = handle->masterDecoder->settings.dualMonoMode;
    ddreSettings->fixedAtmosOutput = handle->settings.fixedAtmosOutput;
    ddreSettings->profile = handle->settings.profile;
    switch ( pSettings->stereoMode )
    {
    default:
    case BAPE_Ac3StereoMode_eLtRt:
        ddreSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eLtRt;
        break;
    case BAPE_Ac3StereoMode_eLoRo:
        ddreSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eLoRo;
        break;
    }
    ddreSettings->drcScaleHi = pSettings->drcScaleHi;
    ddreSettings->drcScaleLow = pSettings->drcScaleLow;
    ddreSettings->drcScaleHiDownmix = pSettings->drcScaleHiDownmix;
    ddreSettings->drcScaleLowDownmix = pSettings->drcScaleLowDownmix;
}

static void BAPE_DolbyDigitalReencode_P_ApplyAacDecoderSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DolbyDigitalReencodeSettings *ddreSettings,
    const BAPE_AacSettings *pSettings)
{
    BDBG_ASSERT(NULL != ddreSettings);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_MSG(("Applying master decoder AAC settings"));

    switch (pSettings->drcMode) {
    default:
    case BAPE_DolbyPulseDrcMode_eLine:
        ddreSettings->drcMode = BAPE_DolbyDigitalReencodeDrcMode_eLine;
        ddreSettings->drcModeDownmix = BAPE_DolbyDigitalReencodeDrcMode_eLine;
        break;
    case BAPE_DolbyPulseDrcMode_eRf:
        ddreSettings->drcMode = BAPE_DolbyDigitalReencodeDrcMode_eRf;
        ddreSettings->drcModeDownmix = BAPE_DolbyDigitalReencodeDrcMode_eRf;
        break;
    }

    ddreSettings->dualMonoMode = handle->masterDecoder->settings.dualMonoMode;
    ddreSettings->fixedAtmosOutput = handle->settings.fixedAtmosOutput;
    ddreSettings->profile = handle->settings.profile;
    switch ( pSettings->downmixMode )
    {
    default:
    case BAPE_AacStereoMode_eLtRt:
        ddreSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eLtRt;
        break;
    case BAPE_AacStereoMode_eLoRo:
        ddreSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eLoRo;
        break;
    case BAPE_AacStereoMode_eArib:
        ddreSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eArib;
        break;
    }
    ddreSettings->drcScaleHi = pSettings->drcScaleHi;
    ddreSettings->drcScaleLow = pSettings->drcScaleLow;
    ddreSettings->drcScaleHiDownmix = pSettings->drcScaleHi;
    ddreSettings->drcScaleLowDownmix = pSettings->drcScaleLow;
}

static void BAPE_DolbyDigitalReencode_P_ApplyAc4DecoderSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DolbyDigitalReencodeSettings *ddreSettings,
    const BAPE_Ac4Settings *pSettings)
{
    BDBG_ASSERT(NULL != ddreSettings);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_MSG(("Applying master decoder AC4 settings"));

    switch (pSettings->drcMode) {
    default:
    case BAPE_DolbyDrcMode_eLine:
        ddreSettings->drcMode = BAPE_DolbyDigitalReencodeDrcMode_eLine;
        break;
    case BAPE_DolbyDrcMode_eRf:
        ddreSettings->drcMode = BAPE_DolbyDigitalReencodeDrcMode_eRf;
        break;
    }

    switch (pSettings->drcModeDownmix) {
    case BAPE_DolbyDrcMode_eLine:
        ddreSettings->drcModeDownmix = BAPE_DolbyDigitalReencodeDrcMode_eLine;
        break;
    default:
    case BAPE_DolbyDrcMode_eRf:
        ddreSettings->drcModeDownmix = BAPE_DolbyDigitalReencodeDrcMode_eRf;
        break;
    }

    ddreSettings->dualMonoMode = handle->masterDecoder->settings.dualMonoMode;
    ddreSettings->fixedAtmosOutput = handle->settings.fixedAtmosOutput;
    ddreSettings->profile = handle->settings.profile;
    switch ( pSettings->stereoMode )
    {
    default:
    case BAPE_DolbyStereoMode_eLtRt:
        ddreSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eLtRt;
        break;
    case BAPE_DolbyStereoMode_eLoRo:
        ddreSettings->stereoMode = BAPE_DolbyDigitalReencodeStereoMode_eLoRo;
        break;
    }

    ddreSettings->drcScaleHi = pSettings->drcScaleHi;
    ddreSettings->drcScaleLow = pSettings->drcScaleLow;
    ddreSettings->drcScaleHiDownmix = pSettings->drcScaleHi;
    ddreSettings->drcScaleLowDownmix = pSettings->drcScaleLow;
}

#if BAPE_DSP_MS12_SUPPORT
static void BAPE_DolbyDigitalReencode_P_TranslateDdreToBdspSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BDSP_Raaga_Audio_DpcmrConfigParams *rendererStageSettings,
    BDSP_Raaga_Audio_DDPEncConfigParams *encodeStageSettings,
    const BAPE_DolbyDigitalReencodeSettings *ddreSettings)
{
    bool forceDrcModes = false;

    BSTD_UNUSED(handle);

    /* Handle loudness equivalence. */
    if ( handle->node.deviceHandle->settings.loudnessSettings.loudnessMode != BAPE_LoudnessEquivalenceMode_eNone )
    {
        forceDrcModes = true;
    }

    /* Set Encode Stage Params */
    BAPE_DSP_P_SET_VARIABLE((*encodeStageSettings), ui32CenterMixLevel, (unsigned)ddreSettings->centerMixLevel);
    BAPE_DSP_P_SET_VARIABLE((*encodeStageSettings), ui32SurroundMixLevel, (unsigned)ddreSettings->surroundMixLevel);
    BAPE_DSP_P_SET_VARIABLE((*encodeStageSettings), ui32DolbySurroundMode, (unsigned)ddreSettings->dolbySurround);
    BAPE_DSP_P_SET_VARIABLE((*encodeStageSettings), ui32DialNorm, (unsigned)ddreSettings->dialogLevel);
    BAPE_DSP_P_SET_VARIABLE((*encodeStageSettings), ui32AtmosLockEnabled, (unsigned)ddreSettings->fixedAtmosOutput ? 1 : 0);

    /* Set Renderer Stage Params */
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32CompressorProfile, (unsigned)ddreSettings->profile);

    /* input level to DDRE is expected to always be -31 */
    BAPE_DSP_P_SET_VARIABLE((*encodeStageSettings), ui32DialNorm, 31);
    /* set output level of PCM output ports using DrcMode */
    if (forceDrcModes)
    {
        /* Force to RF mode based on LE setting */
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DrcMode, 0);
        switch (handle->node.deviceHandle->settings.loudnessSettings.loudnessMode)
        {
        case BAPE_LoudnessEquivalenceMode_eAtscA85:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DrcMode, 3); /* -24dB */
            break;
        case BAPE_LoudnessEquivalenceMode_eEbuR128:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DrcMode, 2); /* -23dB */
            break;
        default:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DrcMode, 1); /* RF mode */
            break;
        }
    }
    else
    {
        switch (ddreSettings->drcMode)
        {
        default:
        case BAPE_DolbyDigitalReencodeDrcMode_eLine:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DrcMode, 0);
            break;
        case BAPE_DolbyDigitalReencodeDrcMode_eRf:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DrcMode, 1);
            break;
        }
        switch ( ddreSettings->drcModeDownmix )
        {
        default:
        case BAPE_DolbyDigitalReencodeDrcMode_eLine:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DrcMode, 0);
            break;
        case BAPE_DolbyDigitalReencodeDrcMode_eRf:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DrcMode, 1);
            break;
        }
    }
    switch ( ddreSettings->dualMonoMode )
    {
    default:
    case BAPE_DualMonoMode_eMix:
    case BAPE_DualMonoMode_eStereo:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DualMode, 0);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DualMode, 0);
        break;
    case BAPE_DualMonoMode_eLeft:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DualMode, 1);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DualMode, 1);
        break;
    case BAPE_DualMonoMode_eRight:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DualMode, 2);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DualMode, 2);
        break;
    }
    switch ( ddreSettings->stereoMode )
    {
    default:
    case BAPE_DolbyDigitalReencodeStereoMode_eLtRt:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DmxMode, 0);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DmxMode, 0);
        break;
    case BAPE_DolbyDigitalReencodeStereoMode_eLoRo:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DmxMode, 1);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DmxMode, 1);
        break;
    case BAPE_DolbyDigitalReencodeStereoMode_eArib:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DmxMode, 2);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DmxMode, 2);
        break;
    }
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DrcCut, ddreSettings->drcScaleHi);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sMultiChannelOutputPort[0].ui32DrcBoost, ddreSettings->drcScaleLow);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DrcCut, ddreSettings->drcScaleHiDownmix);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sStereoDownmixedPort.ui32DrcBoost, ddreSettings->drcScaleLowDownmix);

    /* if Config A MS12, but the encoder only has legacy compressed consumer, limit PCMR output to 6 chs */
    if ( BAPE_P_DolbyCapabilities_DdpEncode(BAPE_MultichannelFormat_e7_1) &&
         BAPE_DolbyDigitalReencode_P_Has4xCompressedOutput(handle) )
    {
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32NumRawMcChannels, 8);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32NumRawMcChannels, 6);
    }
}
#else
static void BAPE_DolbyDigitalReencode_P_TranslateDdreToBdspSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BDSP_Raaga_Audio_DDReencodeConfigParams *rendererStageSettings,
    BDSP_Raaga_Audio_DDTranscodeConfigParams *encodeStageSettings,
    const BAPE_DolbyDigitalReencodeSettings *ddreSettings)
{
    bool forceDrcModes = false;
    BAPE_ChannelMode channelMode, stereoChannelMode;
    bool lfe;
    unsigned stereoOutputPort;
    unsigned multichOutputPort;

    BSTD_UNUSED(encodeStageSettings);

    if ( BAPE_DolbyDigitalReencode_P_HasStereoOutput(handle) && BAPE_DolbyDigitalReencode_P_HasMultichannelOutput(handle) )
    {
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32NumOutPorts, 2);
        multichOutputPort = 0;
        stereoOutputPort = 1;
    }
    else if ( BAPE_DolbyDigitalReencode_P_HasStereoOutput(handle) )
    {
        /* only Stereo output port */
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32NumOutPorts, 1);
        stereoOutputPort = 0;
        multichOutputPort = 1; /* This will be populate values but not be used */
    }
    else if ( BAPE_DolbyDigitalReencode_P_HasMultichannelOutput(handle) )
    {
        /* only Multich output port */
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32NumOutPorts, 2);
        multichOutputPort = 0;
        stereoOutputPort = 1; /* This will be populate values but not be used */
    }
    else
    {
        BDBG_MSG(("%s, No Valid Output Port found!", BSTD_FUNCTION));
        return;
    }

    /* Handle loudness equivalence. */
    if ( handle->node.deviceHandle->settings.loudnessSettings.loudnessMode != BAPE_LoudnessEquivalenceMode_eNone )
    {
        forceDrcModes = true;
    }

    /* Setup correct number of outputs and derive parameters from master decoder if required */
    /* Outputcfg[0] is used for multichannel if enabled otherwise is used for stereo */
    lfe = true;
    channelMode = BAPE_DSP_P_GetChannelMode(BAVC_AudioCompressionStd_eAc3, BAPE_ChannelMode_e3_2, BAPE_DolbyDigitalReencode_P_HasMultichannelOutput(handle), BAPE_MultichannelFormat_e5_1);
    if ( !BAPE_DSP_P_IsLfePermitted(channelMode) )
    {
        lfe = false;
    }
    stereoChannelMode = handle->settings.stereoOutputMode;
    BDBG_ASSERT((stereoChannelMode <= BAPE_ChannelMode_e2_0));

    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32OutMode, BAPE_DSP_P_ChannelModeToDsp(channelMode));
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32OutLfe, lfe?1:0);
    BAPE_DSP_P_GetChannelMatrix(channelMode, lfe, rendererStageSettings->sUserOutputCfg[multichOutputPort].ui32OutputChannelMatrix);

    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32OutMode, BAPE_DSP_P_ChannelModeToDsp(stereoChannelMode));
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32OutLfe, 0);
    BAPE_DSP_P_GetChannelMatrix(stereoChannelMode, false, rendererStageSettings->sUserOutputCfg[stereoOutputPort].ui32OutputChannelMatrix);

    /* To simplify setting UserOutputCfg settings for stereo and multichannel we will always have a valid index for
       multichannel and stereo outputs.  The index will be dependent on what outputs are connected but if only one
       output type is connected the values set for index 1 would not be utilized. */
    if ( forceDrcModes )
    {
        /* Force to RF mode based on LE setting */
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32CompMode, 0);   /* Line (-31dB) */

        /* input level to DDRE is expected to always be -31 */
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32DialNorm, 31);

        switch (handle->node.deviceHandle->settings.loudnessSettings.loudnessMode)
        {
        case BAPE_LoudnessEquivalenceMode_eAtscA85:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32CompMode, 3); /* -24dB */
            break;
        case BAPE_LoudnessEquivalenceMode_eEbuR128:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32CompMode, 2); /* -23dB */
            break;
        default:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32CompMode, 1); /* RF mode */
            break;
        }
    }
    else
    {
        switch ( ddreSettings->drcMode )
        {
        default:
        case BAPE_DolbyDigitalReencodeDrcMode_eLine:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32CompMode, 0);
            break;
        case BAPE_DolbyDigitalReencodeDrcMode_eRf:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32CompMode, 1);
            break;
        }
        switch ( ddreSettings->drcModeDownmix )
        {
        default:
        case BAPE_DolbyDigitalReencodeDrcMode_eLine:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32CompMode, 0);
            break;
        case BAPE_DolbyDigitalReencodeDrcMode_eRf:
            BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32CompMode, 1);
            break;
        }
    }

    switch ( ddreSettings->dualMonoMode )
    {
    default:
    case BAPE_DualMonoMode_eStereo:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32DualMode, 0);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32DualMode, 0);
        break;
    case BAPE_DualMonoMode_eLeft:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32DualMode, 1);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32DualMode, 1);
        break;
    case BAPE_DualMonoMode_eRight:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32DualMode, 2);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32DualMode, 2);
        break;
    case BAPE_DualMonoMode_eMix:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32DualMode, 3);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32DualMode, 3);
        break;
    }
    switch ( ddreSettings->stereoMode )
    {
    default:
    case BAPE_DolbyDigitalReencodeStereoMode_eLtRt:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32StereoMode, 0);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32StereoMode, 0);
        break;
    case BAPE_DolbyDigitalReencodeStereoMode_eLoRo:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32StereoMode, 1);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32StereoMode, 1);
        break;
    case BAPE_DolbyDigitalReencodeStereoMode_eArib:
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32StereoMode, 2);
        BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32StereoMode, 2);
        break;
    }
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32DrcCutFac, BAPE_P_FloatToQ131(ddreSettings->drcScaleHi, 100));
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[multichOutputPort].ui32DrcBoostFac, BAPE_P_FloatToQ131(ddreSettings->drcScaleLow, 100));
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32DrcCutFac, BAPE_P_FloatToQ131(ddreSettings->drcScaleHiDownmix, 100));
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), sUserOutputCfg[stereoOutputPort].ui32DrcBoostFac, BAPE_P_FloatToQ131(ddreSettings->drcScaleLowDownmix, 100));

    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32ExternalPcmEnabled, ddreSettings->externalPcmMode?1:0);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32CompProfile, handle->settings.profile);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32CmixLev, handle->settings.centerMixLevel);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32SurmixLev, handle->settings.surroundMixLevel);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32DsurMod, handle->settings.dolbySurround);
    BAPE_DSP_P_SET_VARIABLE((*rendererStageSettings), ui32DialNorm, handle->settings.dialogLevel);
}
#endif

static BERR_Code BAPE_DolbyDigitalReencode_P_ApplyDspSettings(BAPE_DolbyDigitalReencodeHandle handle)
{
    bool externalPcmMode;
    BERR_Code errCode = BERR_SUCCESS;
#if BAPE_DSP_MS12_SUPPORT
    BDSP_Raaga_Audio_DpcmrConfigParams *pRendererStageSettings = NULL;
    BDSP_Raaga_Audio_DDPEncConfigParams *pEncodeStageSettings = NULL;
#else
    BDSP_Raaga_Audio_DDReencodeConfigParams *pRendererStageSettings = NULL;
    BDSP_Raaga_Audio_DDTranscodeConfigParams *pEncodeStageSettings = NULL;
#endif
    BAPE_DolbyDigitalReencodeSettings *pLocalRendererSettings = NULL;

    #if BAPE_DSP_MS12_SUPPORT
    pRendererStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DpcmrConfigParams));
    pEncodeStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DDPEncConfigParams));
    #else
    pRendererStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DDReencodeConfigParams));
    pEncodeStageSettings = BKNI_Malloc(sizeof(BDSP_Raaga_Audio_DDTranscodeConfigParams));
    #endif
    pLocalRendererSettings = BKNI_Malloc(sizeof(BAPE_DolbyDigitalReencodeSettings));

    if ( pRendererStageSettings == NULL || pEncodeStageSettings == NULL || pLocalRendererSettings == NULL )
    {
        BDBG_ERR(("Unable to alloc memory for DDRE Apply Settings"));
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto cleanup;
    }

    /* Get Defaults - to be filled out by decoder settings */
    BAPE_DolbyDigitalReencode_GetDefaultSettings(pLocalRendererSettings);

    /* Get the current renderer and transcoder settings prior to calling BAPE_DolbyDigitalReencode_P_TranslateDdreToBdspSettings */
    if ( handle->hRendererStage )
    {
        #if BAPE_DSP_MS12_SUPPORT
        errCode = BDSP_Stage_GetSettings(handle->hRendererStage, pRendererStageSettings, sizeof(BDSP_Raaga_Audio_DpcmrConfigParams));
        #else
        errCode = BDSP_Stage_GetSettings(handle->hRendererStage, pRendererStageSettings, sizeof(BDSP_Raaga_Audio_DDReencodeConfigParams));
        #endif
        if ( errCode )
        {
            BERR_TRACE(errCode);
            goto cleanup;
        }
    }

    if ( handle->hTranscodeStage )
    {
        #if BAPE_DSP_MS12_SUPPORT
        errCode = BDSP_Stage_GetSettings(handle->hTranscodeStage, pEncodeStageSettings, sizeof(BDSP_Raaga_Audio_DDPEncConfigParams));
        #else
        errCode = BDSP_Stage_GetSettings(handle->hTranscodeStage, pEncodeStageSettings, sizeof(BDSP_Raaga_Audio_DDTranscodeConfigParams));
        #endif
        if ( errCode )
        {
            BERR_TRACE(errCode);
            goto cleanup;
        }
    }

    if ( handle->hRendererStage )
    {
        externalPcmMode = handle->settings.externalPcmMode;
        if ( NULL == handle->masterDecoder )
        {
            if ( !externalPcmMode )
            {
                BDBG_MSG(("No master input detected.  Forcing external PCM mode."));
                externalPcmMode = true;
            }
        }
        else
        {
            bool msDecoder=false;
            BAVC_AudioCompressionStd codec;

            codec = handle->masterDecoder->startSettings.codec;
            switch ( codec )
            {
            case BAVC_AudioCompressionStd_eAc4:
            case BAVC_AudioCompressionStd_eAc3:
            case BAVC_AudioCompressionStd_eAc3Plus:
            case BAVC_AudioCompressionStd_eAacAdts:
            case BAVC_AudioCompressionStd_eAacLoas:
            case BAVC_AudioCompressionStd_eAacPlusAdts:
            case BAVC_AudioCompressionStd_eAacPlusLoas:
                BDBG_MSG(("Dolby Multistream Input Detected."));
                msDecoder = true;
                if ( externalPcmMode )
                {
                    BDBG_WRN(("External PCM Mode enabled, but master input is a dolby multi-stream codec (%s).  Using External PCM Mode.",
                              BAPE_P_GetCodecName(codec)));
                }
                break;
            case BAVC_AudioCompressionStd_ePcmWav:
                if ( !externalPcmMode )
                {
                    /* This is permitted for certification purposes, but not recommended for general usage. */
                    BDBG_WRN(("PCM data is being received, but externalPcmMode is not active."));
                    BDBG_WRN(("BAPE_DolbyDigitalReencodeSettings.externalPcmMode = true is recommended for PCM."));
                }
                break;
            default:
                if ( !externalPcmMode )
                {
                    BDBG_WRN(("Master input detected, but codec is not a dolby multi-stream codec (%s).  Forcing external PCM mode.",
                              BAPE_P_GetCodecName(codec)));
                    externalPcmMode = true;
                }
                break;
            }
            if ( handle->input->format.type == BAPE_DataType_ePcmStereo && !externalPcmMode )
            {
                BDBG_MSG(("Stereo input detected.  Forcing external PCM mode."));
                externalPcmMode = true;
            }
            if ( msDecoder )
            {
                BAPE_DecoderCodecSettings decoderSettings;
                BAPE_Decoder_GetCodecSettings(handle->masterDecoder, handle->masterDecoder->startSettings.codec, &decoderSettings);
                /* Would prefer a switch statement but coverity will flag dead code based on the msdecoder flag. */
                if ( codec == BAVC_AudioCompressionStd_eAc4 )
                {
                    BAPE_DolbyDigitalReencode_P_ApplyAc4DecoderSettings(handle, pLocalRendererSettings, &decoderSettings.codecSettings.ac4);
                }
                if ( codec == BAVC_AudioCompressionStd_eAc3 )
                {
                    BAPE_DolbyDigitalReencode_P_ApplyAc3DecoderSettings(handle, pLocalRendererSettings, &decoderSettings.codecSettings.ac3);
                }
                else if ( codec == BAVC_AudioCompressionStd_eAc3Plus )
                {
                    BAPE_DolbyDigitalReencode_P_ApplyAc3DecoderSettings(handle, pLocalRendererSettings, &decoderSettings.codecSettings.ac3Plus);
                }
                else if ( codec == BAVC_AudioCompressionStd_eAacAdts ||
                          codec == BAVC_AudioCompressionStd_eAacLoas )
                {
                    BAPE_DolbyDigitalReencode_P_ApplyAacDecoderSettings(handle, pLocalRendererSettings, &decoderSettings.codecSettings.aac);
                }
                else  /* AAC-Plus */
                {
                    BAPE_DolbyDigitalReencode_P_ApplyAacDecoderSettings(handle, pLocalRendererSettings, &decoderSettings.codecSettings.aacPlus);
                }
            }
        }

        /* Handle external PCM mode DRC/downmix settings */
        if ( externalPcmMode )
        {
            BKNI_Memcpy(pLocalRendererSettings, &handle->settings, sizeof(BAPE_DolbyDigitalReencodeSettings));
            pLocalRendererSettings->externalPcmMode = true;
        }

#if BAPE_DSP_MS12_SUPPORT
        if (handle->dspMixer) {
            BAPE_MixerSettings mixerSettings;
            BAPE_Mixer_GetSettings(handle->dspMixer, &mixerSettings);
            if (mixerSettings.fixedEncoderFormat != handle->settings.fixedEncoderFormat) {
                mixerSettings.fixedEncoderFormat = handle->settings.fixedEncoderFormat;
                errCode = BAPE_Mixer_SetSettings(handle->dspMixer, &mixerSettings);
                if (errCode) {
                    BERR_TRACE(errCode);
                }
            }
        }
#endif
        /* Translate DDRE settings to BDSP settings */
        BAPE_DolbyDigitalReencode_P_TranslateDdreToBdspSettings(handle, pRendererStageSettings, pEncodeStageSettings, pLocalRendererSettings);

        #if BAPE_DSP_MS12_SUPPORT
        errCode = BDSP_Stage_SetSettings(handle->hRendererStage, pRendererStageSettings, sizeof(BDSP_Raaga_Audio_DpcmrConfigParams));
        #else
        errCode = BDSP_Stage_SetSettings(handle->hRendererStage, pRendererStageSettings, sizeof(BDSP_Raaga_Audio_DDReencodeConfigParams));
        #endif
        if ( errCode )
        {
            BERR_TRACE(errCode);
            goto cleanup;
        }
    }

    if ( handle->hTranscodeStage )
    {
        if ( BAPE_DolbyDigitalReencode_P_HasCompressedOutput(handle) || BAPE_DolbyDigitalReencode_P_Has4xCompressedOutput(handle) )
        {
            #if BAPE_DSP_MS12_SUPPORT
            BAPE_DSP_P_SET_VARIABLE((*pEncodeStageSettings), ui32DolbyCertificationFlag, handle->settings.encodeSettings.certificationMode ? BDSP_AF_P_eDisable : BDSP_AF_P_eEnable);
            BAPE_DSP_P_SET_VARIABLE((*pEncodeStageSettings), ui32Mode, BAPE_DolbyDigitalReencode_P_Has4xCompressedOutput(handle) ? 8 : 9);
            errCode = BDSP_Stage_SetSettings(handle->hTranscodeStage, pEncodeStageSettings, sizeof(BDSP_Raaga_Audio_DDPEncConfigParams));
            #else
            BAPE_DSP_P_SET_VARIABLE((*pEncodeStageSettings), eTranscodeEnable, handle->settings.encodeSettings.certificationMode ? BDSP_AF_P_eDisable : BDSP_AF_P_eEnable);
            BAPE_DSP_P_SET_VARIABLE((*pEncodeStageSettings), eSpdifHeaderEnable, handle->settings.encodeSettings.spdifHeaderEnabled ? BDSP_AF_P_eEnable : BDSP_AF_P_eDisable);
            errCode = BDSP_Stage_SetSettings(handle->hTranscodeStage, pEncodeStageSettings, sizeof(BDSP_Raaga_Audio_DDTranscodeConfigParams));
            #endif
            if ( errCode )
            {
                BERR_TRACE(errCode);
                goto cleanup;
            }
        }
    }
    errCode = BERR_SUCCESS;
cleanup:
    if (pRendererStageSettings) {
        BKNI_Free(pRendererStageSettings);
        pRendererStageSettings = NULL;
    }
    if (pLocalRendererSettings) {
        BKNI_Free(pLocalRendererSettings);
        pLocalRendererSettings = NULL;
    }
    if (pEncodeStageSettings) {
        BKNI_Free(pEncodeStageSettings);
        pEncodeStageSettings = NULL;
    }
    return errCode;
}

static void BAPE_DolbyDigitalReencode_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BAPE_DolbyDigitalReencodeHandle handle;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BSTD_UNUSED(pConnection);
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);

    /* MDA mode teardown */
    if ( handle->hEncoderTask && handle->taskStarted )
    {
        /* Start the DSP Task */
        errCode = BDSP_Task_Stop(handle->hEncoderTask);
        if ( errCode )
        {
            BERR_TRACE(errCode);
        }
            #if BAPE_MANUAL_DSP_CMD_BLOCKING_STOP
            if ( handle->deviceHandle->settings.dspManualCmdBlocking )
            {
                if ( handle->node.deviceContext )
                {
                    BERR_TRACE(BDSP_Context_ProcessPing(handle->node.deviceContext));
                }
                else
                {
                    BDBG_WRN(("No valid context found. Unable to wait for command ACK from DSP!!!"));
                    BERR_TRACE(BERR_UNKNOWN);
                    /* proceed anyway */
                }
            }
            #endif

        handle->taskStarted = false;

        if ( handle->hMixerStage )
        {
            BDSP_Stage_RemoveAllInputs(handle->hMixerStage);
            BDSP_Stage_RemoveAllOutputs(handle->hMixerStage);
        }

        /* Stop the consumers */
        BAPE_PathNode_P_StopPaths(&handle->node);
    }

    /* Common teardown */
    if ( handle->hRendererStage )
    {
        BDSP_Stage_RemoveAllInputs(handle->hRendererStage);
        BDSP_Stage_RemoveAllOutputs(handle->hRendererStage);
        BDSP_Stage_Destroy(handle->hRendererStage);
        handle->hRendererStage = NULL;
        pNode->deviceContext = NULL;
    }
    if ( handle->hTranscodeStage )
    {
        BDSP_Stage_RemoveAllInputs(handle->hTranscodeStage);
        BDSP_Stage_RemoveAllOutputs(handle->hTranscodeStage);
        BDSP_Stage_Destroy(handle->hTranscodeStage);
        handle->hTranscodeStage = NULL;
    }

    /* MDA mode teardown */
    if ( pConnection->hInterTaskBuffer )
    {
        BDSP_InterTaskBuffer_Destroy(pConnection->hInterTaskBuffer);
        pConnection->hInterTaskBuffer = NULL;
    }
    handle->dspMixer = NULL;
}

static void BAPE_DolbyDigitalReencode_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    (void)BAPE_DolbyDigitalReencode_RemoveInput(pNode->pHandle, pConnector);
}

static void BAPE_DolbyDigitalReencode_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_MSG(("Input sample rate changed to %u Hz.  Overriding to 48kHz output rate.", newSampleRate));

    /* propagate the sample rate change to our output connecters */
    for ( i = 0; i < pNode->numConnectors; i++ )
    {
        BAPE_FMT_Descriptor format;
        BAPE_Connector_P_GetFormat_isrsafe(&pNode->connectors[i], &format);
        if (format.type == BAPE_DataType_eIec61937x4)
        {
            BAPE_Connector_P_SetSampleRate_isr(&pNode->connectors[i], 192000);
        }
        else
        {
            BAPE_Connector_P_SetSampleRate_isr(&pNode->connectors[i], 48000);
        }
    }
}

BERR_Code BAPE_DolbyDigitalReencode_P_SettingsChanged(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DecoderHandle decoder
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);

    if ( decoder != NULL )
    {
        BDBG_OBJECT_ASSERT(decoder, BAPE_Decoder);
        BDBG_ASSERT(decoder->ddre == handle);
        if ( decoder->fwMixer && !decoder->fwMixerMaster )
        {
            /* We only care about the master's settings */
            return BERR_SUCCESS;
        }
    }

    BDBG_MSG(("Master decoder input settings have changed"));
    handle->masterDecoder = decoder;
    errCode = BAPE_DolbyDigitalReencode_P_ApplyDspSettings(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

BAPE_MultichannelFormat BAPE_DolbyDigitalReencode_P_GetMultichannelFormat(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_DolbyDigitalReencode);

    return handle->settings.multichannelFormat;
}

void * BAPE_DolbyDigitalReencode_P_GetEncoderTaskHandle(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    return (void*)handle->hEncoderTask;
}

bool BAPE_DolbyDigitalReencode_P_HasCompressedConsumers(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    return BAPE_DolbyDigitalReencode_P_Has4xCompressedOutput(handle) || BAPE_DolbyDigitalReencode_P_HasCompressedOutput(handle);
}

unsigned BAPE_DolbyDigitalReencode_P_GetDeviceIndex(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    return handle->encoderDeviceIndex;
}
#else
typedef struct BAPE_DolbyDigitalReencode
{
    BDBG_OBJECT(BAPE_DolbyDigitalReencode)
} BAPE_DolbyDigitalReencode;

void BAPE_DolbyDigitalReencode_GetDefaultSettings(
    BAPE_DolbyDigitalReencodeSettings *pSettings   /* [out] default settings */
    )
{
    BSTD_UNUSED(pSettings);
}

/***************************************************************************
Summary:
    Open an SRS DolbyDigitalReencode stage
***************************************************************************/
BERR_Code BAPE_DolbyDigitalReencode_Create(
    BAPE_Handle deviceHandle,
    const BAPE_DolbyDigitalReencodeSettings *pSettings,
    BAPE_DolbyDigitalReencodeHandle *pHandle
    )
{
    BSTD_UNUSED(deviceHandle);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_DolbyDigitalReencode_Destroy(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    BSTD_UNUSED(handle);
}

void BAPE_DolbyDigitalReencode_GetSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_DolbyDigitalReencodeSettings *pSettings    /* [out] Settings */
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_DolbyDigitalReencode_SetSettings(
    BAPE_DolbyDigitalReencodeHandle handle,
    const BAPE_DolbyDigitalReencodeSettings *pSettings
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


void BAPE_DolbyDigitalReencode_GetConnector(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(format);
    BSTD_UNUSED(pConnector);
}


BERR_Code BAPE_DolbyDigitalReencode_AddInput(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_DolbyDigitalReencode_RemoveInput(
    BAPE_DolbyDigitalReencodeHandle handle,
    BAPE_Connector input
    )
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(input);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}


BERR_Code BAPE_DolbyDigitalReencode_RemoveAllInputs(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    BSTD_UNUSED(handle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

unsigned BAPE_DolbyDigitalReencode_P_GetDeviceIndex(
    BAPE_DolbyDigitalReencodeHandle handle
    )
{
    BSTD_UNUSED(handle);
    return -1;
}
#endif
