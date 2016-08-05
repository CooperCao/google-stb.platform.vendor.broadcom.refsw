/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description: Audio Decoder Interface
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_bf_ctrl.h"
#include "bdsp_raaga.h"

BDBG_MODULE(bape_decoder);

BDBG_OBJECT_ID(BAPE_Decoder);

#define BAPE_DECODER_ENABLE_GENERIC_PT      0

#define BAPE_DISABLE_DSP 0  /* Enable this to check for CIT errors and avoid starting the DSP */

#define BAPE_UNDERFLOW_TIMER_DURATION 1000000 /* 1 second in microseconds */

static void BAPE_Decoder_P_SetupDefaults(BAPE_DecoderHandle handle);
static void BAPE_Decoder_P_ConvertTsmStatus_isr(BAPE_DecoderTsmStatus *pStatus, const BDSP_AudioTaskTsmStatus *pDspStatus);
static void BAPE_Decoder_P_FirstPts_isr(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus);
static void BAPE_Decoder_P_TsmFail_isr(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus);
static void BAPE_Decoder_P_TsmPass_isr(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus);
static void BAPE_Decoder_P_SampleRateChange_isr(void *pParam1, int param2, unsigned streamSampleRate, unsigned baseSampleRate);
static void BAPE_Decoder_P_ModeChange_isr(void *pParam1, int param2, unsigned mode);
static void BAPE_Decoder_P_BitrateChange_isr(void *pParam1, int param2, const BDSP_AudioBitRateChangeInfo *pInfo);
static void BAPE_Decoder_P_Overflow_isr(void *pParam1, int param2);
#if 0 /* disable due to out of control interrupts */
static void BAPE_Decoder_P_Underflow_isr(void *pParam1, int param2);
#endif
static void BAPE_Decoder_P_StatusReady_isr(void *pParam1, int param2);
static BERR_Code BAPE_Decoder_P_InputFormatChange_isr(BAPE_PathNode *pNode,BAPE_InputPort inputPort);
static void BAPE_Decoder_P_UnlinkStages(BAPE_DecoderHandle handle);
static BERR_Code BAPE_Decoder_P_ApplyDsolaSettings(BAPE_DecoderHandle handle);
static BERR_Code BAPE_Decoder_P_ApplyKaraokeSettings(BAPE_DecoderHandle handle);
static BERR_Code BAPE_Decoder_P_ApplyFramesyncSettings(BAPE_DecoderHandle handle);
static BERR_Code BAPE_Decoder_P_Start(BAPE_DecoderHandle handle);
static void BAPE_Decoder_P_Stop(BAPE_DecoderHandle handle);
static bool BAPE_Decoder_P_TaskValid_isr(BAPE_DecoderHandle handle);
static void BAPE_Decoder_P_DialnormChange_isr(void *pParam1, int param2);
static BERR_Code BAPE_Decoder_P_DeriveMultistreamLinkage(BAPE_DecoderHandle handle);
static void BAPE_Decoder_P_EncoderOverflow_isr(void *pParam1, int param2);
static bool BAPE_Decoder_P_OrphanConnector(BAPE_DecoderHandle handle, BAPE_ConnectorFormat format);
BERR_Code BAPE_Decoder_P_GetPathDelay_isrsafe(BAPE_DecoderHandle handle, unsigned *pDelay);
static void BAPE_Decoder_P_FreeDecodeToMemory(BAPE_DecoderHandle hDecoder);

#define BAVC_CODEC_IS_AAC(c) \
    (c == BAVC_AudioCompressionStd_eAacAdts || \
     c == BAVC_AudioCompressionStd_eAacLoas || \
     c == BAVC_AudioCompressionStd_eAacPlusLoas || \
     c == BAVC_AudioCompressionStd_eAacPlusAdts)

static const BAPE_DecoderDecodeToMemorySettings g_defaultDecodeToMemSettings =
{
    16, /* maxBuffers */
    192000, /* maxSampleRate */
    16, /* bitsPerSample */
    2, /* numPcmChannels */
    {BAPE_Channel_eLeft, BAPE_Channel_eRight, BAPE_Channel_eCenter, BAPE_Channel_eLfe, BAPE_Channel_eLeftSurround, BAPE_Channel_eRightSurround, BAPE_Channel_eLeftRear, BAPE_Channel_eRightRear} /* channelLayout */
};

void BAPE_Decoder_GetDefaultOpenSettings(
    BAPE_DecoderOpenSettings *pSettings     /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->rateControlSupport = true;
    if ( BAPE_P_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 &&
         BAPE_P_GetDolbyMS12Config() == BAPE_DolbyMs12Config_eA )
    {
        pSettings->multichannelFormat = BAPE_MultichannelFormat_e7_1;
    }
    else
    {
        pSettings->multichannelFormat = BAPE_MultichannelFormat_e5_1;
    }
}

BERR_Code BAPE_Decoder_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_DecoderOpenSettings *pSettings,
    BAPE_DecoderHandle *pHandle                 /* [out] */
    )
{
    BAPE_DecoderOpenSettings defaults;
    BAPE_DecoderHandle handle;
    BDSP_TaskCreateSettings dspSettings;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    BERR_Code errCode;
    BDSP_StageCreateSettings stageCreateSettings, stageCreateSettingsPT;
    bool srcEnabled=false, dsolaEnabled=false, decodeEnabled = false, karaokeEnabled=false;
    #if BAPE_DECODER_ENABLE_GENERIC_PT
    bool passthroughEnabled=false;
    #endif
    unsigned i;


    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);

    if ( NULL == pSettings )
    {
        BAPE_Decoder_GetDefaultOpenSettings(&defaults);
        pSettings = &defaults;
    }

    if ( index >= BAPE_CHIP_MAX_DECODERS )
    {
        BDBG_ERR(("This chip only supports %u decoders.  Cannot open decoder %u", BAPE_CHIP_MAX_DECODERS, index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->decoders[index] )
    {
        BDBG_ERR(("Decoder %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( pSettings->dspIndex >= deviceHandle->numDsps )
    {
        BDBG_ERR(("DSP %u is not available.  This system has %u DSPs.", pSettings->dspIndex, deviceHandle->numDsps));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_Decoder));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BAPE_Decoder));
    BDBG_OBJECT_SET(handle, BAPE_Decoder);
    handle->deviceHandle = deviceHandle;
    handle->index = index;
    handle->dspIndex = pSettings->dspIndex;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "Decoder %u", index);
    BAPE_P_InitPathNode(&handle->node, BAPE_PathNodeType_eDecoder, 0, BAPE_ConnectorFormat_eMax, deviceHandle, handle);
    BLST_S_INIT(&handle->muxOutputList);
    handle->node.dspIndex = handle->dspIndex;
    handle->node.pName = handle->name;
    handle->node.connectors[BAPE_ConnectorFormat_eStereo].pName = "stereo";
    handle->node.connectors[BAPE_ConnectorFormat_eStereo].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].pName = "multichannel";
    handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed].pName = "compressed";
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].pName = "compressed 4x";
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x].pName = "compressed 16x";
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x].useBufferPool = true;
    handle->node.connectors[BAPE_ConnectorFormat_eMono].pName = "mono";
    handle->node.connectors[BAPE_ConnectorFormat_eMono].useBufferPool = true;

    BAPE_FMT_P_InitDescriptor(&format);
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    format.type = BAPE_DataType_ePcm5_1;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    format.type = BAPE_DataType_eIec61937;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    format.type = BAPE_DataType_eIec61937x4;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    format.type = BAPE_DataType_eIec61937x16;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    format.type = BAPE_DataType_ePcmMono;
    errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eMono], &format);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_connector_format; }

    BAPE_PathNode_P_GetInputCapabilities(&handle->node, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm7_1);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x4);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x16);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->node, &caps);
    if ( errCode ) { (void)BERR_TRACE(errCode); goto err_caps; }

    /* Setup node callbacks */
    handle->node.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->node.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->node.startPathToOutput = BAPE_DSP_P_StartPathToOutput;
    handle->node.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->node.inputPortFormatChange_isr = BAPE_Decoder_P_InputFormatChange_isr;
    handle->state = BAPE_DecoderState_eStopped;
    handle->settings.dualMonoMode = BAPE_DualMonoMode_eStereo;
    handle->settings.multichannelFormat = pSettings->multichannelFormat;
    if ( handle->settings.multichannelFormat == BAPE_MultichannelFormat_e7_1 )
    {
        handle->settings.outputMode = BAPE_ChannelMode_e3_4;
    }
    else
    {
        handle->settings.outputMode = BAPE_ChannelMode_e3_2;
    }
    handle->settings.outputLfe = true;
    handle->settings.decodeRate = BAPE_NORMAL_DECODE_RATE;
    handle->settings.loudnessEquivalenceEnabled = true;
    BAPE_Decoder_GetDefaultKaraokeSettings(&handle->settings.karaokeSettings);
    BAPE_Decoder_GetDefaultStartSettings(&handle->startSettings);
    BAPE_Decoder_P_SetupDefaults(handle);

    if ( pSettings->ancillaryDataFifoSize > 0 )
    {
        BDSP_QueueCreateSettings queueSettings;

        handle->pAncDataDspBuffer = BMEM_Heap_Alloc(deviceHandle->memHandle, pSettings->ancillaryDataFifoSize);
        if ( NULL == handle->pAncDataDspBuffer )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_ancillary_buffer;
        }

        BDSP_Queue_GetDefaultSettings(handle->deviceHandle->dspContext, &queueSettings);
        queueSettings.dataType = BDSP_DataType_eRdbAnc;
        queueSettings.numBuffers = 1;
        queueSettings.bufferInfo[0].bufferSize = pSettings->ancillaryDataFifoSize;
        BMEM_Heap_ConvertAddressToOffset(deviceHandle->memHandle, handle->pAncDataDspBuffer, &queueSettings.bufferInfo[0].bufferAddress);

        errCode = BDSP_Queue_Create(deviceHandle->dspContext, pSettings->dspIndex, &queueSettings, &handle->hAncDataQueue);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_ancillary_buffer;
        }
        handle->pAncDataHostBuffer = BMEM_Heap_Alloc(deviceHandle->memHandle, pSettings->ancillaryDataFifoSize);
        if ( NULL == handle->pAncDataHostBuffer )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_ancillary_buffer;
        }

        errCode = BMEM_Heap_ConvertAddressToCached(deviceHandle->memHandle, handle->pAncDataHostBuffer, &handle->pAncDataHostCached);
        if ( NULL == handle->pAncDataHostCached )
        {
            errCode = BERR_TRACE(errCode);
            goto err_ancillary_buffer;
        }

        handle->ancDataBufferSize = pSettings->ancillaryDataFifoSize;
    }

    BDSP_Task_GetDefaultCreateSettings(deviceHandle->dspContext, &dspSettings);

    dspSettings.dspIndex = handle->dspIndex;

    /* Determine allocations and task type based on decoder type */
    handle->type = pSettings->type;
    switch ( pSettings->type )
    {
    default:
    case BAPE_DecoderType_eUniversal:
        decodeEnabled = true;
        #if BAPE_DECODER_ENABLE_GENERIC_PT
        passthroughEnabled = true;
        #endif
        break;
    case BAPE_DecoderType_eDecode:
        decodeEnabled = true;
        break;
    case BAPE_DecoderType_ePassthrough:
        #if BAPE_DECODER_ENABLE_GENERIC_PT
        passthroughEnabled = true;
        #endif
        break;
    case BAPE_DecoderType_eDecodeToMemory:
        /* Decode to memory requires only minimal configuration */
        decodeEnabled = true;
        dspSettings.numSrc = dspSettings.numDst = 1;
        dspSettings.masterTask = false;
        break;
    }

    errCode = BDSP_Task_Create(deviceHandle->dspContext, &dspSettings, &handle->hTask);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_task_create;
    }

    if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        BAPE_Capabilities apeCaps;
        errCode = BAPE_Decoder_SetDecodeToMemorySettings(handle, &g_defaultDecodeToMemSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stage;
        }

        BAPE_GetCapabilities(deviceHandle, &apeCaps);

        BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
        BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
        stageCreateSettings.algorithmSupported[BDSP_Algorithm_eOutputFormatter] = true;
        errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hOutputFormatter);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stage;
        }
    }
    else if ( decodeEnabled )
    {
        if ( pSettings->rateControlSupport )
        {
            dsolaEnabled = BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eDsola);
        }

        if ( pSettings->karaokeSupported )
        {
            karaokeEnabled = BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eKaraoke);
        }

        srcEnabled = BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eSrc);
        #if BAPE_DECODER_ENABLE_GENERIC_PT
        if ( passthroughEnabled )
        {
            BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioPassthrough, &stageCreateSettings);
            BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
            stageCreateSettings.algorithmSupported[BDSP_Algorithm_eGenericPassthrough] = true;
            errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hPassthroughStage);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_stage;
            }
        }
        #endif
    }

    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, decodeEnabled ? BDSP_AlgorithmType_eAudioDecode : BDSP_AlgorithmType_eAudioPassthrough, &stageCreateSettings);
    /* filter based on application bavc algo list and ape codec table */
    /* CITTODO - Set a list of algos from NEXUS and verify */
    BAPE_P_PopulateSupportedBDSPAlgos( decodeEnabled ? BDSP_AlgorithmType_eAudioDecode : BDSP_AlgorithmType_eAudioPassthrough,
        (pSettings->pSupportedCodecs != NULL && pSettings->numSupportedCodecs > 0) ? pSettings->pSupportedCodecs : NULL,
        pSettings->numSupportedCodecs,
        (const bool *)stageCreateSettings.algorithmSupported,
        stageCreateSettings.algorithmSupported);

    if ( decodeEnabled )
    {
        /* also enable PT on the primary decode stage. */
        BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioPassthrough, &stageCreateSettingsPT);
        /* filter based on application bavc algo list and ape codec table */
        BAPE_P_PopulateSupportedBDSPAlgos( BDSP_AlgorithmType_eAudioPassthrough,
            (pSettings->pSupportedCodecs != NULL && pSettings->numSupportedCodecs > 0) ? pSettings->pSupportedCodecs : NULL,
            pSettings->numSupportedCodecs,
            (const bool *)stageCreateSettingsPT.algorithmSupported,
            stageCreateSettingsPT.algorithmSupported);
        for ( i=0; i<BDSP_Algorithm_eMax; i++ )
        {
            if ( stageCreateSettingsPT.algorithmSupported[i] )
            {
                stageCreateSettings.algorithmSupported[i] = stageCreateSettingsPT.algorithmSupported[i];
            }
        }
    }

    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hPrimaryStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage;
    }

    if ( dsolaEnabled )
    {
        BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
        BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
        stageCreateSettings.algorithmSupported[BDSP_Algorithm_eDsola] = true;
        errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hDsolaStageStereo);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stage;
        }
        errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hDsolaStageMultichannel);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stage;
        }
    }
    if ( karaokeEnabled )
    {
        BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
        BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
        stageCreateSettings.algorithmSupported[BDSP_Algorithm_eKaraoke] = true;
        errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hKaraokeStage);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stage;
        }

    }
    if ( srcEnabled )
    {
        BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
        BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
        stageCreateSettings.algorithmSupported[BDSP_Algorithm_eSrc] = true;
        errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hSrcStageStereo);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stage;
        }
        errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hSrcStageMultichannel);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stage;
        }
    }

    /* Success */
    *pHandle = handle;
    deviceHandle->decoders[index] = handle;
    return BERR_SUCCESS;

err_stage:
err_task_create:
err_ancillary_buffer:
err_connector_format:
err_caps:
    BAPE_Decoder_Close(handle);
    return errCode;
}

void BAPE_Decoder_Close(
    BAPE_DecoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        BDBG_WRN(("Implicitly stopping decoder %u on shutdown.", handle->index));
        BAPE_Decoder_Stop(handle);
    }

    /* Disconnect from all mixers, post-processors */
    BAPE_Connector_P_RemoveAllConnections(&handle->node.connectors[BAPE_ConnectorFormat_eStereo]);
    BAPE_Connector_P_RemoveAllConnections(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel]);
    BAPE_Connector_P_RemoveAllConnections(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed]);
    BAPE_Connector_P_RemoveAllConnections(&handle->node.connectors[BAPE_ConnectorFormat_eMono]);

    /* Cleanup */
    if ( handle->pAncDataHostBuffer )
    {
        BMEM_Heap_Free(handle->deviceHandle->memHandle, handle->pAncDataHostBuffer);
    }
    if ( handle->pAncDataDspBuffer )
    {
        BMEM_Heap_Free(handle->deviceHandle->memHandle, handle->pAncDataDspBuffer);
    }
    if ( handle->hAncDataQueue )
    {
        BDSP_Queue_Destroy(handle->hAncDataQueue);
    }

    if ( handle->hTask )
    {
        BDSP_Task_Destroy(handle->hTask);
    }
    if ( handle->hPrimaryStage )
    {
        BDSP_Stage_Destroy(handle->hPrimaryStage);
    }
    if ( handle->hSrcStageStereo )
    {
        BDSP_Stage_Destroy(handle->hSrcStageStereo);
    }
    if ( handle->hSrcStageMultichannel )
    {
        BDSP_Stage_Destroy(handle->hSrcStageMultichannel);
    }
    if ( handle->hKaraokeStage )
    {
        BDSP_Stage_Destroy(handle->hKaraokeStage);
    }
    if ( handle->hDsolaStageStereo )
    {
        BDSP_Stage_Destroy(handle->hDsolaStageStereo);
    }
    if ( handle->hDsolaStageMultichannel )
    {
        BDSP_Stage_Destroy(handle->hDsolaStageMultichannel);
    }
    if ( handle->hPassthroughStage )
    {
        BDSP_Stage_Destroy(handle->hPassthroughStage);
    }
    BAPE_Decoder_P_FreeDecodeToMemory(handle);
    if ( handle->hOutputFormatter )
    {
        BDSP_Stage_Destroy(handle->hOutputFormatter);
    }
    handle->deviceHandle->decoders[handle->index] = NULL;
    BDBG_OBJECT_DESTROY(handle, BAPE_Decoder);
    BKNI_Free(handle);
}

void BAPE_Decoder_GetDefaultStartSettings(
    BAPE_DecoderStartSettings *pSettings    /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codec = BAVC_AudioCompressionStd_eMax;
    pSettings->streamType = BAVC_StreamType_eTsMpeg;
    pSettings->ppmCorrection = true;
    pSettings->targetSyncEnabled = true;
    pSettings->maxOutputRate = 48000;
}

void BAPE_Decoder_GetDefaultKaraokeSettings(
    BAPE_DecoderKaraokeSettings *pSettings /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->vocalSuppressionLevel = 90;
    pSettings->vocalSuppressionFrequency = 4688;
    pSettings->outputMakeupBoost = 3;
}

static BERR_Code BAPE_Decoder_P_ApplyDsolaSettings(
    BAPE_DecoderHandle handle
    )
{
    BDSP_Raaga_Audio_DsolaConfigParams userConfig;
    BERR_Code errCode;

    if ( handle->hDsolaStageStereo )
    {
        errCode = BDSP_Stage_GetSettings(handle->hDsolaStageStereo, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        userConfig.ui32InputPcmFrameSize = (512 * handle->settings.decodeRate)/BAPE_NORMAL_DECODE_RATE;

        errCode = BDSP_Stage_SetSettings(handle->hDsolaStageStereo, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    if ( handle->hDsolaStageMultichannel )
    {
        errCode = BDSP_Stage_GetSettings(handle->hDsolaStageMultichannel, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        userConfig.ui32InputPcmFrameSize = (512 * handle->settings.decodeRate)/BAPE_NORMAL_DECODE_RATE;

        errCode = BDSP_Stage_SetSettings(handle->hDsolaStageMultichannel, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ApplyKaraokeSettings(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode;
    BDSP_Raaga_Audio_KaraokeConfigParams userConfig;

    errCode = BDSP_Stage_GetSettings(handle->hKaraokeStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    switch (handle->settings.karaokeSettings.vocalSuppressionLevel)
    {
        case 0:
            BAPE_DSP_P_SET_VARIABLE(userConfig, level, 0);
            break;
        case 75:
            BAPE_DSP_P_SET_VARIABLE(userConfig, level, 1);
            break;
        case 85:
            BAPE_DSP_P_SET_VARIABLE(userConfig, level, 2);
            break;
        case 90:
            BAPE_DSP_P_SET_VARIABLE(userConfig, level, 3);
            break;
        case 95:
            BAPE_DSP_P_SET_VARIABLE(userConfig, level, 4);
            break;
        case 100:
            BAPE_DSP_P_SET_VARIABLE(userConfig, level, 5);
            break;
        default:
            BDBG_ERR(("Invalid Vocal Suppression Level set (%d)",handle->settings.karaokeSettings.vocalSuppressionLevel));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BAPE_DSP_P_SET_VARIABLE(userConfig, speechBins, handle->settings.karaokeSettings.vocalSuppressionFrequency*4096/48000); /* Conversion to FFT based on 48kHz */
    if (handle->settings.karaokeSettings.outputMakeupBoost > 3)
    {
        BDBG_ERR(("Invalid Output Boost set (%d)",handle->settings.karaokeSettings.outputMakeupBoost));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, scaleoutputs, handle->settings.karaokeSettings.outputMakeupBoost);
    }

    errCode = BDSP_Stage_SetSettings(handle->hKaraokeStage, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;

}

static BERR_Code BAPE_Decoder_P_SetupPassthrough(
    BAPE_DecoderHandle handle
    )
{
    BDSP_Raaga_Audio_PassthruConfigParams userConfig;
    BERR_Code errCode;
    BDSP_StageHandle hPassthrough;
    unsigned type;

    hPassthrough = handle->node.connectors[BAPE_ConnectorFormat_eCompressed].hStage;

    if ( hPassthrough )
    {
        if ( handle->simul )
        {
            if ( BAPE_P_CodecRequiresGenericPassthru(handle->startSettings.codec) )
            {
                BDBG_MSG(("Setup PT - Generic Passthrough required"));
                type = BDSP_Raaga_ePassthruType_Simul;
            }
            else
            {
                return BERR_SUCCESS;
            }
        }
        else if ( handle->passthrough )
        {
            if ( handle->startSettings.codec == BAVC_AudioCompressionStd_ePcm )
            {
                type = BDSP_Raaga_ePassthruType_PCM;
            }
            else if ( handle->startSettings.codec == BAVC_AudioCompressionStd_eMlp )
            {
                /* MLP Passthrough is a special algorithm with no configuration */
                return BERR_SUCCESS;
            }
            else
            {
                type = BDSP_Raaga_ePassthruType_SPDIF;
            }
        }
        else
        {
            return BERR_SUCCESS;
        }
    }
    else
    {
        return BERR_SUCCESS;
    }

    BDBG_MSG(("Setup PT - Configure BDSP Passthrough stage"));
    errCode = BDSP_Stage_GetSettings(hPassthrough, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    userConfig.ui32PassthruType = type;
    switch ( handle->startSettings.codec )
    {
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        userConfig.eAacHeaderType = BDSP_Raaga_eAacHeaderType_Adts;
        break;
    case BAVC_AudioCompressionStd_eAacLoas:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        userConfig.eAacHeaderType = BDSP_Raaga_eAacHeaderType_Loas;
        break;
    default:
        break;
    }

    errCode = BDSP_Stage_SetSettings(hPassthrough, &userConfig, sizeof(userConfig));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Decoder_P_ValidateDecodeSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderStartSettings *pSettings,
    BAPE_DecoderStartSettings *pOutputSettings
    )
{
    BDSP_AlgorithmInfo algoInfo;
    BERR_Code errCode;
    BAPE_PathNode *pNodes[BAPE_MAX_NODES];
    unsigned numFound;
    bool decodeSupported = true;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pOutputSettings);
    /* Start by copying the existing settings */
    BKNI_Memcpy(pOutputSettings, pSettings, sizeof(BAPE_DecoderStartSettings));
    /* Check for valid input */
    if ( NULL == pSettings->pContextMap && NULL == pSettings->inputPort )
    {
        BDBG_ERR(("Must specify an input to decode"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /* Store local copy of the RAVE context map in case it goes out of scope after start. */
    if ( pSettings->pContextMap )
    {
        BKNI_Memcpy(&handle->contextMap, pSettings->pContextMap, sizeof(BAVC_XptContextMap));
        pOutputSettings->pContextMap = &handle->contextMap;
    }
    /* Check for valid STC */
    if ( pSettings->stcIndex >= BAPE_CHIP_MAX_STCS )
    {
        BDBG_ERR(("STC Index %u out of range.  Supported values are 0..%u", pSettings->stcIndex, BAPE_CHIP_MAX_STCS-1));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check for FW availability */
    errCode = BDSP_GetAlgorithmInfo(handle->deviceHandle->dspHandle, BAPE_P_GetCodecAudioDecode(pSettings->codec), &algoInfo);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    else if ( !algoInfo.supported && !handle->passthrough )
    {
        BDBG_ERR(("Codec %s (%u) DSP algorithm %s (%u) is not supported.",
                  BAPE_P_GetCodecName(pSettings->codec), pSettings->codec,
                  algoInfo.pName, BAPE_P_GetCodecAudioDecode(pSettings->codec)));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    /* Decode is not supported but need to check to see if passthru is */
    else if ( !algoInfo.supported && handle->passthrough )
    {
        decodeSupported = false;
    }

    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMono].totalConnections > 0 )
    {
        /* Check two things.  One, you can only have mono on certain codecs.  Two, you can only have
           two active PCM paths per node.  */
        if ( !BAPE_P_CodecSupportsMono(pSettings->codec) )
        {
            BDBG_ERR(("Codec %s does not support mono output.  Please remove all mono outputs.", BAPE_P_GetCodecName(pSettings->codec)));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].totalConnections > 0 &&
             handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].totalConnections > 0 )
        {
            BDBG_ERR(("You can only have two types of PCM data from a single decoder.  Please remove outputs from either the stereo, multichannel, or mono connectors."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    if ( handle->passthrough )
    {
        /* Check for DSP PT FW availability */
        errCode = BDSP_GetAlgorithmInfo(handle->deviceHandle->dspHandle, BAPE_P_GetCodecAudioPassthrough(pSettings->codec), &algoInfo);

        /* Determine if passthrough is allowed by both APE and FW */
        if ( !BAPE_P_CodecSupportsPassthrough(pSettings->codec) || !algoInfo.supported )
        {
            BDBG_WRN(("Codec %s (%u) does not support compressed passthrough -- Compressed output will be PCM",
                      BAPE_P_GetCodecName(pSettings->codec), pSettings->codec));
            if ( !decodeSupported )
            {
                BDBG_ERR(("Codec %s (%u) DSP algorithm %s (%u) is not supported.",
                    BAPE_P_GetCodecName(pSettings->codec), pSettings->codec,
                    algoInfo.pName, BAPE_P_GetCodecAudioDecode(pSettings->codec)));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }
            handle->stereoOnCompressed = true;
            handle->passthrough = false;    /* We're really doing a decode now */
        }
        else
        {
            handle->stereoOnCompressed = false;
        }
    }
    /* Determine if we're trying to decode */
    if ( !handle->passthrough )
    {
        /* If we're trying simul mode, make sure that's valid */
        if ( handle->simul )
        {
            if ( !BAPE_P_CodecSupportsSimulMode(pSettings->codec) )
            {
                BDBG_WRN(("Codec %s (%u) does not support simultaneous PCM and compressed output - compressed output will be stereo PCM",
                          BAPE_P_GetCodecName(pSettings->codec), pSettings->codec));
                handle->stereoOnCompressed = true;
            }
            else
            {
                handle->stereoOnCompressed = false;
            }
        }
    }
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].totalConnections > 0 )
    {
        /* Determine if multichannel is valid */
        if ( BAPE_P_GetCodecMultichannelFormat(pSettings->codec) == BAPE_MultichannelFormat_e2_0 )
        {
            BDBG_WRN(("Codec %s (%u) does not support multichannel output - multichannel output will be stereo",
                      BAPE_P_GetCodecName(pSettings->codec), pSettings->codec));
            handle->stereoOnMultichannel = true;
        }
        else
        {
            handle->stereoOnMultichannel = false;
        }
    }
    /* Determine if DSOLA is possible */
    if ( pSettings->decodeRateControl )
    {
        if ( !BAPE_DSP_P_AlgorithmSupported(handle->deviceHandle, BDSP_Algorithm_eDsola) )
        {
            BDBG_WRN(("This platform does not support DSOLA for audio decoder rate control.  Disabling rate control."));
            pOutputSettings->decodeRateControl = false;
        }
        if ( handle->outputStatus.numOutputs[BAPE_DataType_ePcm7_1] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_ePcmMono] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_ePcmRf] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_eIec61937] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_eIec61937x4] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_eIec61937x16] > 0 )
        {
            BDBG_WRN(("Can not perform audio trick modes with multichannel, compressed, rf, or mono outputs connected.  Disabling rate control."));
            pOutputSettings->decodeRateControl = false;
        }
    }
    /* Determine if Karaoke is possible */
    if ( pSettings->karaokeModeEnabled )
    {
        if ( !handle->hKaraokeStage )
        {
            BDBG_WRN(("Decoder was not opened with karaoke support enabled"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        /* Need to add checks if there are any limitations */
    }
    /* Determine if PPM is possible */
    if ( pSettings->ppmCorrection )
    {
        BAPE_PathNode *pNode;
        unsigned numFound;
        bool mixedWithDsp;

        BAPE_PathNode_P_FindConsumersBySubtype(&handle->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, &pNode);
        mixedWithDsp = (numFound > 0) ? true : false;

        if ( handle->outputStatus.numOutputs[BAPE_DataType_ePcm5_1] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_ePcm7_1] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_ePcmMono] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_ePcmRf] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_eIec61937] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_eIec61937x4] > 0 ||
             handle->outputStatus.numOutputs[BAPE_DataType_eIec61937x16] > 0 )
        {
            BDBG_MSG(("PPM Correction is not permitted with multichannel, rf, compressed, or mono outptus.  Disabling PPM correction."));
            pOutputSettings->ppmCorrection = false;
        }
        else if ( pSettings->inputPort )
        {
            BDBG_MSG(("PPM Correction is not permitted with external inputs.  Disabling PPM correction."));
            pOutputSettings->ppmCorrection = false;
        }
        else if ( mixedWithDsp )
        {
            BDBG_MSG(("PPM Correction is not permitted when mixing with the DSP.  Disabling PPM correction."));
            pOutputSettings->ppmCorrection = false;
        }
        else if ( pSettings->codec == BAVC_AudioCompressionStd_eVorbis )
        {
            BDBG_MSG(("PPM Correction is not permitted when codec is BAVC codec %d.  Disabling PPM correction.", pSettings->codec));
            pOutputSettings->ppmCorrection = false;
        }
    }

    /* Find any MuxOutput consumers - required for overflow handling */
    BAPE_PathNode_P_FindConsumersByType(&handle->node, BAPE_PathNodeType_eMuxOutput, BAPE_MAX_NODES, &numFound, pNodes);
    if ( numFound > BAPE_MAX_NODES )
    {
        BDBG_ERR(("increase BAPE_MAX_NODES"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    else if ( numFound > 0 )
    {
        unsigned i;
        BDBG_MSG(("Found %d mux outputs downstream", numFound));
        for ( i=0; i<numFound; i++ )
        {
            BLST_S_INSERT_HEAD(&handle->muxOutputList, (BAPE_MuxOutput*)pNodes[i]->pHandle, decoderListNode);
        }
    }

    return BERR_SUCCESS;
}

unsigned BAPE_P_BDSPSampleFrequencyToInt( BDSP_AF_P_SampFreq bdspSF )
{
    unsigned sr = 0;

    BDBG_CASSERT( BDSP_AF_P_SampFreq_e8Khz == 0 );
    BDBG_CASSERT( BDSP_AF_P_SampFreq_eMax == 15 );
    switch ( bdspSF )
    {
    case BDSP_AF_P_SampFreq_e8Khz:
        sr = 8000; break;
    case BDSP_AF_P_SampFreq_e11_025Khz:
        sr = 11025; break;
    case BDSP_AF_P_SampFreq_e12Khz:
        sr = 12000; break;
    case BDSP_AF_P_SampFreq_e16Khz:
        sr = 16000; break;
    case BDSP_AF_P_SampFreq_e22_05Khz:
        sr = 22050; break;
    case BDSP_AF_P_SampFreq_e24Khz:
        sr = 24000; break;
    case BDSP_AF_P_SampFreq_e32Khz:
        sr = 32000; break;
    case BDSP_AF_P_SampFreq_e44_1Khz:
        sr = 44100; break;
    default:
    case BDSP_AF_P_SampFreq_e48Khz:
        sr = 48000; break;
    case BDSP_AF_P_SampFreq_e64Khz:
        sr = 64000; break;
    case BDSP_AF_P_SampFreq_e88_2Khz:
        sr = 88200; break;
    case BDSP_AF_P_SampFreq_e96Khz:
        sr = 96000; break;
    case BDSP_AF_P_SampFreq_e128Khz:
        sr = 128000; break;
    case BDSP_AF_P_SampFreq_e176_4Khz:
        sr = 176400; break;
    case BDSP_AF_P_SampFreq_e192Khz:
        sr = 192000; break;
    }

    return sr;
}

unsigned BAPE_P_GetOutputSamplerate( BDSP_AF_P_SampFreq bdspSF, bool constrainToFamily, unsigned maxSampleRate )
{
    unsigned sr = 0;

    if ( maxSampleRate == 96000 )
    {
        if ( constrainToFamily )
        {
            switch ( bdspSF )
            {
            case BDSP_AF_P_SampFreq_e8Khz:
            case BDSP_AF_P_SampFreq_e16Khz:
                bdspSF = BDSP_AF_P_SampFreq_e32Khz;
                break;
            case BDSP_AF_P_SampFreq_e11_025Khz:
            case BDSP_AF_P_SampFreq_e22_05Khz:
                bdspSF = BDSP_AF_P_SampFreq_e44_1Khz;
                break;
            case BDSP_AF_P_SampFreq_e12Khz:
            case BDSP_AF_P_SampFreq_e24Khz:
                bdspSF = BDSP_AF_P_SampFreq_e48Khz;
                break;
            case BDSP_AF_P_SampFreq_e128Khz:
                bdspSF = BDSP_AF_P_SampFreq_e64Khz;
                break;
            case BDSP_AF_P_SampFreq_e176_4Khz:
                bdspSF = BDSP_AF_P_SampFreq_e88_2Khz;
                break;
            case BDSP_AF_P_SampFreq_e192Khz:
                bdspSF = BDSP_AF_P_SampFreq_e96Khz;
                break;
            default:
                break;
            }
        }
        else
        {
            switch ( bdspSF )
            {
            case BDSP_AF_P_SampFreq_e128Khz:
                bdspSF = BDSP_AF_P_SampFreq_e64Khz;
                break;
            case BDSP_AF_P_SampFreq_e176_4Khz:
                bdspSF = BDSP_AF_P_SampFreq_e88_2Khz;
                break;
            case BDSP_AF_P_SampFreq_e192Khz:
                bdspSF = BDSP_AF_P_SampFreq_e96Khz;
                break;
            default:
                break;
            }
        }
    }
    else if ( maxSampleRate == 48000 )
    {
        if ( constrainToFamily )
        {
            switch ( bdspSF )
            {
            case BDSP_AF_P_SampFreq_e8Khz:
            case BDSP_AF_P_SampFreq_e16Khz:
            case BDSP_AF_P_SampFreq_e64Khz:
            case BDSP_AF_P_SampFreq_e128Khz:
                bdspSF = BDSP_AF_P_SampFreq_e32Khz;
                break;
            case BDSP_AF_P_SampFreq_e11_025Khz:
            case BDSP_AF_P_SampFreq_e22_05Khz:
            case BDSP_AF_P_SampFreq_e88_2Khz:
            case BDSP_AF_P_SampFreq_e176_4Khz:
                bdspSF = BDSP_AF_P_SampFreq_e44_1Khz;
                break;
            case BDSP_AF_P_SampFreq_e12Khz:
            case BDSP_AF_P_SampFreq_e24Khz:
            case BDSP_AF_P_SampFreq_e96Khz:
            case BDSP_AF_P_SampFreq_e192Khz:
                bdspSF = BDSP_AF_P_SampFreq_e48Khz;
                break;
            default:
                break;
            }
        }
        else
        {
            switch ( bdspSF )
            {
            case BDSP_AF_P_SampFreq_e64Khz:
            case BDSP_AF_P_SampFreq_e128Khz:
                bdspSF = BDSP_AF_P_SampFreq_e32Khz;
                break;
            case BDSP_AF_P_SampFreq_e88_2Khz:
            case BDSP_AF_P_SampFreq_e176_4Khz:
                bdspSF = BDSP_AF_P_SampFreq_e44_1Khz;
                break;
            case BDSP_AF_P_SampFreq_e96Khz:
            case BDSP_AF_P_SampFreq_e192Khz:
                bdspSF = BDSP_AF_P_SampFreq_e48Khz;
                break;
            default:
                break;
            }
        }
    }
    else
    {
        BDBG_ERR(("APE does not currently support max sample rates other than 48000 and 96000"));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    sr = BAPE_P_BDSPSampleFrequencyToInt(bdspSF);
    return sr;
}

static BERR_Code BAPE_Decoder_P_Start(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode;
    unsigned input, output;
    const BAPE_DecoderStartSettings *pSettings;
    BDSP_TaskStartSettings taskStartSettings;
    BAPE_FMT_Descriptor format;
    BDSP_StageHandle hStage;
    BTMR_TimerSettings timerSettings;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    pSettings = &handle->startSettings;

    BDBG_MSG(("BAPE_Decoder_P_Start(%p) [index %u]", (void *)handle, handle->index));

    /* Do we have compressed outputs directly connected? */
    BDBG_MSG(("%s: %u stereo consumers", handle->node.pName, handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].directConnections));
    BDBG_MSG(("%s: %u multichannel consumers", handle->node.pName, handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections));
    BDBG_MSG(("%s: %u compressed consumers", handle->node.pName, handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed].directConnections));
    BDBG_MSG(("%s: %u compressed 4x consumers", handle->node.pName, handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed4x].directConnections));
    BDBG_MSG(("%s: %u compressed 16x consumers", handle->node.pName, handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed16x].directConnections));
    BDBG_MSG(("%s: %u mono consumers", handle->node.pName, handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMono].directConnections));

    /* Setup Task Parameters */
    BDSP_Task_GetDefaultStartSettings(handle->hTask, &taskStartSettings);

    taskStartSettings.ppmCorrection = pSettings->ppmCorrection;
    taskStartSettings.primaryStage = handle->hPrimaryStage;
    switch ( pSettings->streamType )
    {
    case BAVC_StreamType_eDssEs:
    case BAVC_StreamType_eDssPes:
           taskStartSettings.timeBaseType = BDSP_AF_P_TimeBaseType_e27Mhz;
           break;
    default:
           taskStartSettings.timeBaseType = BDSP_AF_P_TimeBaseType_e45Khz;
           break;
    }

    taskStartSettings.audioTaskDelayMode = BDSP_AudioTaskDelayMode_eDefault;
    if ( pSettings->delayMode == BAPE_DspDelayMode_eLowVariable )
    {
        switch ( pSettings->codec )
        {
            case BAVC_AudioCompressionStd_eAacAdts:
            case BAVC_AudioCompressionStd_eAacLoas:
            case BAVC_AudioCompressionStd_eAc3:
            case BAVC_AudioCompressionStd_eLpcm1394:
                taskStartSettings.audioTaskDelayMode = BDSP_AudioTaskDelayMode_WD_eLowest;
                break;
            default:
                break;
        }
    }
    else if ( pSettings->delayMode == BAPE_DspDelayMode_eLowFixed )
    {
        switch ( pSettings->codec )
        {
            case BAVC_AudioCompressionStd_eAacAdts:
            case BAVC_AudioCompressionStd_eAacLoas:
            case BAVC_AudioCompressionStd_eAc3:
            case BAVC_AudioCompressionStd_eLpcm1394:
                taskStartSettings.audioTaskDelayMode = BDSP_AudioTaskDelayMode_WD_eLow;
                break;
            default:
                break;
        }
    }

    BDBG_MSG(("BDSP Path Delay Mode %d", taskStartSettings.audioTaskDelayMode));

    if ( taskStartSettings.audioTaskDelayMode == BDSP_AudioTaskDelayMode_WD_eLow ||
         taskStartSettings.audioTaskDelayMode == BDSP_AudioTaskDelayMode_WD_eLowest )
    {
        taskStartSettings.eZeroPhaseCorrEnable = false;
    }
    else
    {
        taskStartSettings.eZeroPhaseCorrEnable = true;
    }

    BDBG_MSG(("BDSP zero phase correction %d", taskStartSettings.eZeroPhaseCorrEnable));

    if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        taskStartSettings.realtimeMode = BDSP_TaskRealtimeMode_eOnDemand;
    }
    else
    {
        taskStartSettings.realtimeMode = pSettings->nonRealTime ? BDSP_TaskRealtimeMode_eNonRealTime : BDSP_TaskRealtimeMode_eRealTime;
    }
    errCode = BAPE_DSP_P_DeriveTaskStartSettings(&handle->node, &taskStartSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stages;
    }

    /* Setup Primary Stage */
    if ( handle->passthrough )
    {
        errCode = BDSP_Stage_SetAlgorithm(handle->hPrimaryStage, BAPE_P_GetCodecAudioPassthrough(handle->startSettings.codec));
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
    }
    else
    {
        errCode = BDSP_Stage_SetAlgorithm(handle->hPrimaryStage, BAPE_P_GetCodecAudioDecode(handle->startSettings.codec));
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
    }

    if ( pSettings->pContextMap )
    {
        errCode = BDSP_Stage_AddRaveInput(handle->hPrimaryStage, pSettings->pContextMap, &input);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
    }
    else /* DFIFO Input */
    {
        unsigned i;
        BDSP_FmmBufferDescriptor fmmInputDesc;
        BAPE_DfifoGroupCreateSettings dfifoCreateSettings;
        BAPE_DfifoGroupSettings dfifoSettings;

        /* Make sure the input is usable.  Don't reference InputPort fields until after this.  */
        errCode = BAPE_InputPort_P_AttachConsumer(pSettings->inputPort, &handle->node, &handle->inputPortFormat);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_attach_input_port;
        }

        /* BDBG_LOG(("Decoder's inputPort format: " BAPE_FMT_P_TO_PRINTF_ARGS(&pSettings->inputPort->format)));            */
        /* BDBG_LOG(("Decoder's inputPort caps: " BAPE_FMT_P_SOURCEMASK_TO_PRINTF_ARGS(&handle->node.inputCapabilities))); */
        /* BDBG_LOG(("Decoder's inputPort caps: " BAPE_FMT_P_TYPEMASK_TO_PRINTF_ARGS(&handle->node.inputCapabilities)));   */

        if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&pSettings->inputPort->format)  &&  pSettings->inputPort->format.sampleRate > 48000)
        {
            BDBG_ERR(("Decoder doesn't support PCM input sample rate of %u because it exceeds 48000", pSettings->inputPort->format.sampleRate ));
            errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_dfifo_alloc;
        }

        if ( pSettings->inputPort && pSettings->inputPort->fciSpGroup )
        {
            BAPE_FciSplitterGroupCreateSettings fciSpCreateSettings;
            BAPE_FciSplitterGroup_P_GetDefaultCreateSettings(&fciSpCreateSettings);
            if ( handle->fciSpOutput )
            {
                BAPE_FciSplitterOutputGroup_P_Destroy(handle->fciSpOutput);
                handle->fciSpOutput = NULL;
            }
            fciSpCreateSettings.numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pSettings->inputPort->format);
            errCode = BAPE_FciSplitterOutputGroup_P_Create(pSettings->inputPort->fciSpGroup, &fciSpCreateSettings, &handle->fciSpOutput);
            if ( errCode != BERR_SUCCESS )
            {
                BDBG_ERR(("Unable to allocate FCI Splitter Output Group, numChannelPairs %lu", (unsigned long)fciSpCreateSettings.numChannelPairs));
                return BERR_TRACE(errCode);
            }
        }
        BAPE_DfifoGroup_P_GetDefaultCreateSettings(&dfifoCreateSettings);
        dfifoCreateSettings.numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->inputPortFormat);
        errCode = BAPE_DfifoGroup_P_Create(handle->deviceHandle, &dfifoCreateSettings, &handle->inputDfifoGroup);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_dfifo_alloc;
        }

        BAPE_DfifoGroup_P_GetSettings(handle->inputDfifoGroup, &dfifoSettings);
        if ( handle->fciSpOutput )
        {
            BAPE_FciSplitterOutputGroup_P_GetOutputFciIds(handle->fciSpOutput, &dfifoSettings.input);
        }
        else
        {
            BAPE_InputPort_P_GetFciIds(pSettings->inputPort, &dfifoSettings.input);
        }
        dfifoSettings.highPriority = (handle->inputPortFormat.sampleRate >= 96000) ? true : false;

        if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->inputPortFormat) )
        {
            dfifoSettings.interleaveData = false;
            dfifoSettings.dataWidth = 32;
        }
        else
        {
            dfifoSettings.reverseEndian = true; /* data from input needs to be endian swapped for dsp */
            dfifoSettings.interleaveData = true;
            dfifoSettings.dataWidth = 16;
        }

        errCode = BAPE_P_AllocateBuffers(handle->deviceHandle, &handle->inputPortFormat, handle->pInputBuffers);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_input_buffers;
        }

        for ( i = 0; i < dfifoCreateSettings.numChannelPairs; i++ )
        {
            /* Setup CIT to have correct ringbuffer ID for each DFIFO -- TODO: There should be a routine in dsp_utils_priv that takes a DFIFO Group handle instead */
            dfifoSettings.bufferInfo[2*i].base = handle->pInputBuffers[i]->offset;
            if ( dfifoSettings.interleaveData )
            {
                dfifoSettings.bufferInfo[2*i].length = handle->pInputBuffers[i]->bufferSize;
            }
            else
            {
                unsigned length = handle->pInputBuffers[i]->bufferSize/2;
                dfifoSettings.bufferInfo[2*i].length = dfifoSettings.bufferInfo[(2*i)+1].length = length;
                dfifoSettings.bufferInfo[(2*i)+1].base = handle->pInputBuffers[i]->offset + length;
            }
        }
        errCode = BAPE_DfifoGroup_P_SetSettings(handle->inputDfifoGroup, &dfifoSettings);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_set_dfifo_settings;
        }

        errCode = BAPE_DSP_P_InitFmmInputDescriptor(handle->inputDfifoGroup, &fmmInputDesc);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_set_dfifo_settings;
        }

        errCode = BDSP_Stage_AddFmmInput(handle->hPrimaryStage, BAPE_FMT_P_GetDspDataType_isrsafe(&handle->inputPortFormat), &fmmInputDesc, &input);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_set_dfifo_settings;
        }
    }

    /* Determine stage outputting data to stereo PCM path  */
    hStage = handle->hPrimaryStage;
    /* Check for any orphan paths */
    BAPE_PathNode_P_FindOrphans(&handle->node);

    if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        BAPE_MultichannelFormat format = BAPE_P_GetCodecMultichannelFormat(pSettings->codec);
        BDSP_DataType dataType;
        BDSP_Raaga_OutputFormatterSettings formatterSettings;
        unsigned dummy;

        if ( handle->decodeToMem.settings.numPcmChannels > 2 )
        {
            /* Route 7.1 only if the codec is capable and the app asked for it */
            if ( format == BAPE_MultichannelFormat_e7_1 && handle->decodeToMem.settings.numPcmChannels > 6 )
            {
                dataType = BDSP_DataType_ePcm7_1;
            }
            else
            {
                dataType = BDSP_DataType_ePcm5_1;
            }
        }
        else
        {
            /* Just send stereo or mono */
            if ( handle->decodeToMem.settings.numPcmChannels == 1 && BAPE_P_CodecSupportsMono(pSettings->codec) )
            {
                dataType = BDSP_DataType_ePcmMono;
            }
            else
            {
                dataType = BDSP_DataType_ePcmStereo;
            }
        }

        errCode = BDSP_Stage_GetSettings(handle->hOutputFormatter, &formatterSettings, sizeof(formatterSettings));
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
        formatterSettings.ui32NumChannels = handle->decodeToMem.settings.numPcmChannels;
        formatterSettings.ui32NumBitsPerSample = handle->decodeToMem.settings.bitsPerSample;
        formatterSettings.ui32ChannelLayout[0] = handle->decodeToMem.settings.channelLayout[0];
        formatterSettings.ui32ChannelLayout[1] = handle->decodeToMem.settings.channelLayout[1];
        formatterSettings.ui32ChannelLayout[2] = handle->decodeToMem.settings.channelLayout[2];
        formatterSettings.ui32ChannelLayout[3] = handle->decodeToMem.settings.channelLayout[3];
        formatterSettings.ui32ChannelLayout[4] = handle->decodeToMem.settings.channelLayout[4];
        formatterSettings.ui32ChannelLayout[5] = handle->decodeToMem.settings.channelLayout[5];
        formatterSettings.ui32ChannelLayout[6] = handle->decodeToMem.settings.channelLayout[6];
        formatterSettings.ui32ChannelLayout[7] = handle->decodeToMem.settings.channelLayout[7];
        errCode = BDSP_Stage_SetSettings(handle->hOutputFormatter, &formatterSettings, sizeof(formatterSettings));
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
        errCode = BDSP_Stage_AddOutputStage(hStage, dataType, handle->hOutputFormatter, &output, &input);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
        BDSP_Queue_Flush(handle->decodeToMem.hARQ);
        errCode = BDSP_Stage_AddQueueOutput(handle->hOutputFormatter, handle->decodeToMem.hARQ, &dummy);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
        BDSP_Queue_Flush(handle->decodeToMem.hADQ);
        errCode = BDSP_Stage_AddQueueOutput(handle->hOutputFormatter, handle->decodeToMem.hADQ, &dummy);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_stages;
        }
    }
    if ( ((handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].directConnections > 0 && !BAPE_Decoder_P_OrphanConnector(handle, BAPE_ConnectorFormat_eStereo)) || handle->stereoOnMultichannel) )
    {
        /* Add SRC if required */
        if ( !handle->passthrough && BAPE_P_CodecRequiresSrc(handle->startSettings.codec) && handle->hSrcStageStereo )
        {
            errCode = BDSP_Stage_AddOutputStage(hStage, BDSP_DataType_ePcmStereo, handle->hSrcStageStereo, &output, &input);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_stages;
            }
            hStage = handle->hSrcStageStereo;
        }
        /* Add Karaokee post process if required */
        if ( pSettings->karaokeModeEnabled && handle->hKaraokeStage )
        {
            errCode = BDSP_Stage_AddOutputStage(hStage, BDSP_DataType_ePcmStereo, handle->hKaraokeStage, &output, &input);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_stages;
            }
            hStage = handle->hKaraokeStage;
        }
        /* Add DSOLA if required */
        if ( pSettings->decodeRateControl && handle->hDsolaStageStereo )
        {
            errCode = BDSP_Stage_AddOutputStage(hStage, BDSP_DataType_ePcmStereo, handle->hDsolaStageStereo, &output, &input);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_stages;
            }
            hStage = handle->hDsolaStageStereo;
        }
    }
    handle->node.connectors[BAPE_ConnectorFormat_eStereo].hStage = hStage;

    if ( handle->stereoOnMultichannel )
    {
        handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].hStage = handle->node.connectors[BAPE_ConnectorFormat_eStereo].hStage;
    }
    else
    {
        /* Determine branch/stage outputting data to multichannel PCM path  */
        hStage = handle->hPrimaryStage;
        if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections > 0 && !BAPE_Decoder_P_OrphanConnector(handle, BAPE_ConnectorFormat_eMultichannel) )
        {
            BDSP_DataType dataType = BAPE_DSP_P_GetDataTypeFromConnector(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel]);
            /* Add SRC if required */
            if ( !handle->passthrough && BAPE_P_CodecRequiresSrc(handle->startSettings.codec) && handle->hSrcStageMultichannel )
            {
                errCode = BDSP_Stage_AddOutputStage(hStage, dataType, handle->hSrcStageMultichannel, &output, &input);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                    goto err_stages;
                }
                hStage = handle->hSrcStageMultichannel;
            }
            /* Add DSOLA if required */
            if ( pSettings->decodeRateControl && handle->hDsolaStageMultichannel )
            {
                errCode = BDSP_Stage_AddOutputStage(hStage, dataType, handle->hDsolaStageMultichannel, &output, &input);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                    goto err_stages;
                }
                hStage = handle->hDsolaStageMultichannel;
            }
        }
        handle->node.connectors[BAPE_ConnectorFormat_eMultichannel].hStage = hStage;
    }

    /* Determine if we need to add passthrough */
    if ( handle->stereoOnCompressed )
    {
        handle->node.connectors[BAPE_ConnectorFormat_eCompressed].hStage = handle->node.connectors[BAPE_ConnectorFormat_eStereo].hStage;
    }
    else
    {
        hStage = handle->hPrimaryStage;
#if BAPE_DECODER_ENABLE_GENERIC_PT
        if ( handle->simul && BAPE_P_CodecRequiresGenericPassthru(handle->startSettings.codec) )
        {
            /* CITTODO - force branch?  Use aux data out? */
            errCode = BDSP_Stage_AddOutputStage(hStage, BDSP_DataType_eIec61937, handle->hPassthroughStage, &output, &input);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_stages;
            }
            hStage = handle->hPassthroughStage;
        }
#endif
        handle->node.connectors[BAPE_ConnectorFormat_eCompressed].hStage = hStage;
    }
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x].hStage =
        handle->node.connectors[BAPE_ConnectorFormat_eCompressed].hStage;
    handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x].hStage =
        handle->node.connectors[BAPE_ConnectorFormat_eCompressed].hStage;

    /* Prepare all branches to start */
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].directConnections )
    {
        BAPE_Connector_P_GetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], &format);
        format.sampleRate = 0;
        format.ppmCorrection = pSettings->ppmCorrection;
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_format_change;
        }
    }
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections )
    {
        BAPE_Connector_P_GetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], &format);
        format.sampleRate = 0;
        if ( handle->stereoOnMultichannel )
        {
            format.type = BAPE_DataType_ePcmStereo;
        }
        else
        {
            switch ( handle->settings.multichannelFormat )
            {
            default:
                /* Should never get here */
            case BAPE_MultichannelFormat_e5_1:
                format.type = BAPE_DataType_ePcm5_1;
                break;
            case BAPE_MultichannelFormat_e7_1:
                format.type = BAPE_DataType_ePcm7_1;
                break;
            }
        }
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_format_change;
        }
    }
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed].directConnections )
    {
        BAPE_Connector_P_GetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], &format);
        format.sampleRate = 0;
        if ( handle->stereoOnCompressed )
        {
            format.type = BAPE_DataType_ePcmStereo;
        }
        else
        {
            format.type = BAPE_DataType_eIec61937;
            BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, handle->startSettings.codec);

            if ( handle->startSettings.codec == BAVC_AudioCompressionStd_eAc3Plus )
            {
                if ( handle->passthrough )
                {
                    format.type = BAPE_DataType_eIec61937x4;    /* AC3+ Passthrough is carried at 4x the rate on HDMI */
                }
                else
                {
                    BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, BAVC_AudioCompressionStd_eAc3);  /* While decoding, compressed output will be AC3 not AC3+ */
                }
            }
            else if ( handle->startSettings.codec == BAVC_AudioCompressionStd_eDtshd )
            {
                BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, BAVC_AudioCompressionStd_eDts);  /* The core bitstream will be output on the 1x compressed output */
            }
        }
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_format_change;
        }
    }
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed4x].directConnections )
    {
        BAPE_Connector_P_GetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x], &format);
        format.sampleRate = 0;
        if ( handle->stereoOnCompressed )
        {
            format.type = BAPE_DataType_ePcmStereo;
        }
        else
        {
            format.type = BAPE_DataType_eIec61937x4;
            BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, handle->startSettings.codec);
        }
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_format_change;
        }
    }
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed16x].directConnections )
    {
        BAPE_Connector_P_GetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x], &format);
        format.sampleRate = 0;
        if ( handle->stereoOnCompressed )
        {
            format.type = BAPE_DataType_ePcmStereo;
        }
        else
        {
            format.type = BAPE_DataType_eIec61937x16;
            BAPE_FMT_P_SetAudioCompressionStd_isrsafe(&format, handle->startSettings.codec);
        }
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_format_change;
        }
    }
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMono].directConnections )
    {
        BAPE_Connector_P_GetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eMono], &format);
        format.sampleRate = 0;
        errCode = BAPE_Connector_P_SetFormat(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_format_change;
        }
    }

    /* Mono outputs will always start from branch/stage 0 - no SRC or DSOLA. */
    handle->node.connectors[BAPE_ConnectorFormat_eMono].hStage = handle->node.connectors[BAPE_ConnectorFormat_eStereo].hStage;

    handle->state = BAPE_DecoderState_eStarting;

    errCode = BAPE_PathNode_P_AcquirePathResources(&handle->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_build_paths;
    }

    /* Ready */
    handle->streamSampleRate = 0;
    handle->pcmOutputSampleRate = 0;
    handle->mode = (unsigned)-1;
    BKNI_Memset(&handle->bitRateInfo, 0, sizeof(handle->bitRateInfo));
    taskStartSettings.maxIndependentDelay = handle->deviceHandle->settings.maxIndependentDelay;
    errCode = BAPE_Decoder_SetInterruptHandlers(handle, &handle->interrupts);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_codec_settings;
    }

    errCode = BAPE_Decoder_P_ApplyFramesyncSettings(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_codec_settings;
    }

    /* Apply codec settings */
    errCode = BAPE_Decoder_P_ApplyCodecSettings(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_codec_settings;
    }

    if ( handle->ddre && (NULL == handle->fwMixer || handle->fwMixerMaster == true) )
    {
        errCode = BAPE_DolbyDigitalReencode_P_SettingsChanged(handle->ddre, handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Save Path Delay Mode */
    handle->pathDelayMode = taskStartSettings.audioTaskDelayMode;

    /* Apply TSM settings to Decoder */
    errCode = BAPE_Decoder_SetTsmSettings(handle, &handle->tsmSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_codec_settings;
    }

    /* Configure DSOLA */
    if ( pSettings->decodeRateControl )
    {
        errCode = BAPE_Decoder_P_ApplyDsolaSettings(handle);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_codec_settings;
        }
    }

    /* Configure Karaoke */
    if ( handle->startSettings.karaokeModeEnabled && handle->hKaraokeStage )
    {
        errCode = BAPE_Decoder_P_ApplyKaraokeSettings(handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Configure passthrough if used */
    errCode = BAPE_Decoder_P_SetupPassthrough(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_codec_settings;
    }

    /* Init ancillary data buffer */
    if ( handle->pAncDataHostCached )
    {
        BAPE_Decoder_P_InitAncillaryDataBuffer(handle);
    }

    /* Link the path resources */
    errCode = BAPE_PathNode_P_ConfigurePathResources(&handle->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_config_path_resources;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintMixers(handle->deviceHandle);
    #endif

    /* Start the consumers */
    errCode = BAPE_PathNode_P_StartPaths(&handle->node);
    if ( errCode )
    {
        goto err_start_path;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintDownstreamNodes(&handle->node);
    #endif

    {
        unsigned maxSampleRate = 48000;
        bool stayInSamplRateFamily = true;
        unsigned fixedOutputRate = 0;
        /* set up output samplerate table */
        /* For decode-to-memory mode, do no sample rate adjustments.  LSF/QSF should be output as-is and no fixed 48kHz, etc. */
        if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
        {
            if ((BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS11 || BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS10) && BAVC_CODEC_IS_AAC(pSettings->codec) )
            {
                for ( i=0; i<BDSP_AF_P_SampFreq_eMax; i++ )
                {
                    /* MS10/MS11 Dolby Pulse must convert to 48kHz */
                    handle->sampleRateMap.ui32OpSamplingFrequency[i] = 48000;
                }
            }
            else
            {
                for ( i=0; i<BDSP_AF_P_SampFreq_eMax; i++ )
                {
                    handle->sampleRateMap.ui32OpSamplingFrequency[i] = BAPE_P_BDSPSampleFrequencyToInt(i);
                }
            }
        }
        else
        {
            if ((BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS11 || BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS10) && BAVC_CODEC_IS_AAC(pSettings->codec) )
            {
                BDBG_MSG(("Setting Decoder %p to output 48k since DDRE is attached", (void *)handle));
                fixedOutputRate = 48000;
            }
            else if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 && handle->ddre && BAVC_CODEC_IS_AAC(pSettings->codec) )
            {
                BDBG_MSG(("Setting Decoder %p to output the content samplerate for AAC since DDRE is attached", (void *)handle));
                stayInSamplRateFamily = false;
                maxSampleRate = 48000;
            }
            else if ( handle->ddre )
            {
                if ( handle->startSettings.maxOutputRate > 48000 )
                {
                    BDBG_WRN(("Ignoring max samplerate of %u because DDRE is connected and only supports 48k input", handle->startSettings.maxOutputRate));
                }
                stayInSamplRateFamily = true;
                maxSampleRate = 48000;
            }
            else
            {
                BDBG_MSG(("Standard SR table"));
                switch( handle->startSettings.maxOutputRate )
                {
                    case 48000:
                        /* cap decoder output rate at 48k */
                        stayInSamplRateFamily = true;
                        maxSampleRate = 48000;
                        break;
                    case 96000:
                        /* cap decoder output rate at 96k */
                        stayInSamplRateFamily = true;
                        maxSampleRate = 96000;
                        break;
                }
            }

            BDBG_MSG(("Setting up BDSP SR Table, fixedOutputRate %d, stayInSamplRateFamily %d, maxOutputRate %d Hz", fixedOutputRate, stayInSamplRateFamily, maxSampleRate));
            /* Process Sample Rate Table */
            for ( i=0; i<BDSP_AF_P_SampFreq_eMax; i++ )
            {
                handle->sampleRateMap.ui32OpSamplingFrequency[i] = fixedOutputRate ? fixedOutputRate : BAPE_P_GetOutputSamplerate(i, stayInSamplRateFamily, maxSampleRate);
                BDBG_MSG(("Input SR %d Hz output at %d Hz", BAPE_P_BDSPSampleFrequencyToInt(i), handle->sampleRateMap.ui32OpSamplingFrequency[i]));
            }
        }
        taskStartSettings.pSampleRateMap = &handle->sampleRateMap;
    }

#if BAPE_DISABLE_DSP
    #warning Task Start is Disabled!
    BDBG_ERR(("NOT STARTING"));
#else
    errCode = BDSP_Task_Start(handle->hTask, &taskStartSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_start_task;
    }
#endif

    /* Start the DFIFOs */
    if ( pSettings->inputPort )
    {
        errCode = BAPE_DfifoGroup_P_Start(handle->inputDfifoGroup, false);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_dfifo_start;
        }
        BDBG_ASSERT(NULL != pSettings->inputPort->enable);
        pSettings->inputPort->enable(pSettings->inputPort);
    }

    /* Timer Create */
    {
       BTMR_GetDefaultTimerSettings(&timerSettings);
       timerSettings.type = BTMR_Type_eCountDown;
       timerSettings.cb_isr = BAPE_P_CheckUnderflow_isr;
       timerSettings.pParm1 = handle;
       timerSettings.parm2 = BAPE_UNDERFLOW_TIMER_DURATION;
       errCode = BTMR_CreateTimer(handle->deviceHandle->tmrHandle, &handle->underFlowTimer, &timerSettings);
       if ( errCode )
       {
           errCode = BERR_TRACE(errCode);
           goto err_timer_create;
       }

       handle->underFlowCount = 0;

       errCode = BTMR_StartTimer(handle->underFlowTimer, BAPE_UNDERFLOW_TIMER_DURATION);
       if ( errCode )
       {
           errCode = BERR_TRACE(errCode);
           goto err_timer_start;
       }
   }

    handle->pathDelayMode = taskStartSettings.audioTaskDelayMode;
    handle->state = BAPE_DecoderState_eStarted;
    handle->halted = false;
    return BERR_SUCCESS;

err_timer_start:
    if (handle->underFlowTimer)
    {
        BTMR_DestroyTimer(handle->underFlowTimer);
        handle->underFlowTimer = NULL;
    }
err_timer_create:
    if ( pSettings->inputPort )
    {
        pSettings->inputPort->disable(handle->startSettings.inputPort);
        BAPE_DfifoGroup_P_Stop(handle->inputDfifoGroup);
    }
err_dfifo_start:
    BDSP_Task_Stop(handle->hTask);
err_start_task:
    BAPE_PathNode_P_StopPaths(&handle->node);
err_start_path:
err_config_path_resources:
err_codec_settings:
    BAPE_PathNode_P_ReleasePathResources(&handle->node);
    handle->state = BAPE_DecoderState_eStopped;
err_build_paths:
err_format_change:
err_set_dfifo_settings:
    if ( pSettings->inputPort )
    {
        BAPE_P_FreeBuffers(handle->deviceHandle, handle->pInputBuffers);
    }
err_input_buffers:
    if ( pSettings->inputPort )
    {
        BAPE_DfifoGroup_P_Destroy(handle->inputDfifoGroup);
        handle->inputDfifoGroup = NULL;
    }
err_dfifo_alloc:
    if ( pSettings->inputPort )
    {
        BAPE_InputPort_P_DetachConsumer(pSettings->inputPort, &handle->node);
    }
err_attach_input_port:
err_stages:
    BAPE_Decoder_P_UnlinkStages(handle);
    handle->state = BAPE_DecoderState_eStopped;
    return errCode;
}

BERR_Code BAPE_Decoder_Start(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderStartSettings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_PathNode *pNode;
    unsigned numFound;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    BDBG_MSG(("BAPE_Decoder_Start(%p) [index %u]", (void *)handle, handle->index));

    if ( NULL == handle->deviceHandle->dspContext )
    {
        BDBG_ERR(("DSP Not avaliable"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        BDBG_ERR(("Already running, cannot start"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Get output status */
    BAPE_PathNode_P_GetOutputStatus(&handle->node, &handle->outputStatus);

    /* Validate we either have real-time outputs or don't depending on system requirements */
    BAPE_PathNode_P_FindConsumersBySubtype(&handle->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eStandard, 1, &numFound, &pNode);
    if ( pSettings->nonRealTime || handle->type == BAPE_DecoderType_eDecodeToMemory )
    {
        if ( numFound != 0 )
        {
            BDBG_ERR(("No outputs should be connected to a decoder in %s mode.", handle->type == BAPE_DecoderType_eDecodeToMemory ? "DecodeToMemory" : "non-realtime"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        if ( numFound == 0 )
        {
            BDBG_ERR(("No outputs are connected.  Cannot start."));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    /* Determine decoder "mode" based on connections first */
    if ( handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed].directConnections > 0 ||
         handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed4x].directConnections > 0 ||
         handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eCompressed16x].directConnections > 0 )
    {
        /* Do we have any stereo or multichannel outputs connected directly? */
        if ( 0 == (handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eStereo].directConnections +
                   handle->outputStatus.connectorStatus[BAPE_ConnectorFormat_eMultichannel].directConnections) )
        {
            /* No.  This is a passthrough operation. */
            handle->passthrough = true;
            handle->simul = false;
        }
        else
        {
            /* Yes.  This is "simul", decode + passthrough */
            handle->passthrough = false;
            handle->simul = true;
        }
    }
    else
    {
        /* No compressed outputs.  This is a decode operation. */
        handle->passthrough = false;
        handle->simul = false;
    }

    /* Sanity check settings */
    errCode = BAPE_Decoder_P_ValidateDecodeSettings(handle, pSettings, &handle->startSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    pSettings = &handle->startSettings;

    /* Determine multistream linkage */
    errCode = BAPE_Decoder_P_DeriveMultistreamLinkage(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( handle->ancDataBufferSize )
    {
        BAPE_Decoder_FlushAncillaryData(handle);
    }

    /* Trigger task and path startup */
    errCode = BAPE_Decoder_P_Start(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Success */
    return BERR_SUCCESS;
}

static void BAPE_Decoder_P_Stop(
    BAPE_DecoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    /* Destroy the Timer */
    if (handle->underFlowTimer)
    {
        BTMR_StopTimer(handle->underFlowTimer);
        BTMR_DestroyTimer (handle->underFlowTimer);
        handle->underFlowTimer = NULL;
    }

    /* Stop DFIFO if needed*/
    if ( handle->startSettings.inputPort )
    {
        BDBG_ASSERT(NULL != handle->startSettings.inputPort->disable);
        if ( handle->fciSpOutput )
        {
            BAPE_FciSplitterOutputGroup_P_Destroy(handle->fciSpOutput);
            handle->fciSpOutput = NULL;
        }
        handle->startSettings.inputPort->disable(handle->startSettings.inputPort);
        BAPE_DfifoGroup_P_Stop(handle->inputDfifoGroup);
    }

#if BAPE_DISABLE_DSP
    #warning Task Start is Disabled!
    BDBG_ERR(("NOT STOPPING DSP"));
#else
    BDSP_Task_Stop(handle->hTask);
#endif

    BAPE_PathNode_P_StopPaths(&handle->node);

    if ( handle->startSettings.inputPort )
    {
        BAPE_InputPort_P_DetachConsumer(handle->startSettings.inputPort, &handle->node);
        BAPE_DfifoGroup_P_Destroy(handle->inputDfifoGroup);
        handle->inputDfifoGroup = NULL;
        BAPE_P_FreeBuffers(handle->deviceHandle, handle->pInputBuffers);
    }

    BAPE_Decoder_P_UnlinkStages(handle);

    while ( BLST_S_FIRST(&handle->muxOutputList) )
    {
        BLST_S_REMOVE_HEAD(&handle->muxOutputList, decoderListNode);
    }
}

void BAPE_Decoder_Stop(
    BAPE_DecoderHandle handle
    )
{
    bool unmute = false;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Stop(%p) [index %u]", (void *)handle, handle->index));

    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_WRN(("Decoder %u Already Stopped.", handle->index));
        return;
    case BAPE_DecoderState_ePaused:
    case BAPE_DecoderState_eDisabledPaused:
        unmute = true;
        break;
    case BAPE_DecoderState_eFrozen:
        BDBG_WRN(("Decoder %u is in Freeze state, Please unfreeze before stopping.", handle->index));
        BAPE_Decoder_UnFreeze(handle);
        break;
    default:
        break;
    }

    /* Stop the task first */
    handle->state = BAPE_DecoderState_eStopped;

    /* Set lastValidPts back to 0 */
    handle->lastValidPts = 0;
    /* Serialize with critical section prior to stopping the task, guarantees isrs are not updating while we stop (they check the state first) */
    BKNI_EnterCriticalSection();
    BKNI_LeaveCriticalSection();

    BAPE_Decoder_P_Stop(handle);

    /* Reset multistream state */
    handle->ddre = NULL;
    handle->fwMixer = NULL;
    handle->fwMixerMaster = true;

    if ( unmute )
    {
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], false);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], false);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], false);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eMono], false);
    }
}

BERR_Code BAPE_Decoder_Pause(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Pause(%p) [index %u]", (void *)handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot pause.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eFrozen:
        BDBG_ERR(("Decoder %u is frozen, cannot pause.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        break;
    case BAPE_DecoderState_ePaused:
        BDBG_WRN(("Decoder %u already paused.", handle->index));
        return BERR_SUCCESS;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        BDBG_ERR(("Decoder %u is disabled for flush.  Must complete flush prior to calling Pause.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->type != BAPE_DecoderType_eDecodeToMemory )
    {
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], true);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], true);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], true);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eMono], true);

        errCode = BDSP_AudioTask_Pause(handle->hTask);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    handle->state = BAPE_DecoderState_ePaused;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_Resume(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Resume(%p) [index %u]", (void *)handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot resume.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eFrozen:
        BDBG_ERR(("Decoder %u is frozen, cannot resume.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        BDBG_WRN(("Decoder %u already running.", handle->index));
        return BERR_SUCCESS;
    case BAPE_DecoderState_ePaused:
        break;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        BDBG_ERR(("Decoder %u is disabled for flush.  Must complete flush prior to calling Resume.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->type != BAPE_DecoderType_eDecodeToMemory )
    {
        errCode = BDSP_AudioTask_Resume(handle->hTask);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], false);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], false);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], false);
        BAPE_Connector_P_SetMute(&handle->node.connectors[BAPE_ConnectorFormat_eMono], false);
    }

    handle->state = BAPE_DecoderState_eStarted;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_Advance(
    BAPE_DecoderHandle handle,
    unsigned milliseconds           /* Milliseconds to advance */
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("BAPE_Decoder_Advance(%p, %u) [index %u]", (void *)handle, milliseconds, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot advance.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eFrozen:
        BDBG_ERR(("Decoder %u is frozen, cannot advance.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        BDBG_WRN(("Decoder %u running, can't advance.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_ePaused:
        break;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        BDBG_ERR(("Decoder %u is disabled for flush.  Must complete flush prior to calling Advance.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BDSP_AudioTask_Advance(handle->hTask, milliseconds);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_Freeze(
    BAPE_DecoderHandle handle
    )
{
    BAPE_OutputPort outputs[2];
    unsigned numOutputs = 0;
    BAPE_OutputPort_P_EnableParams enableParams;
    BDSP_AudioTaskFreezeSettings freezeSettings;
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("%s(%p) [index %u]", __FUNCTION__, (void *)handle, handle->index));
    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStarted:
        break;
    case BAPE_DecoderState_ePaused:
    case BAPE_DecoderState_eStopped:
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
    case BAPE_DecoderState_eFrozen:
        BDBG_ERR(("Invalid decoder state %u.  Cannot Freeze", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unknown decoder state %u.  Cannot Freeze", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BAPE_PathNode_P_GetConnectedOutputs(&handle->node, 2, &numOutputs, outputs);

    if ( numOutputs != 1 )
    {
        BDBG_ERR(("%s - INVALID number of outputs (%d) connected to this decoder %p", __FUNCTION__, numOutputs, (void *)handle));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    if ( outputs[0]->getEnableParams == NULL )
    {
        BDBG_ERR(("%s - Cannot freeze output type (%d) connected to this decoder %p", __FUNCTION__, outputs[0]->type, (void *)handle));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    outputs[0]->getEnableParams(outputs[0], false, &enableParams);

    BDSP_AudioTask_GetDefaultFreezeSettings(&freezeSettings);
    freezeSettings.fmmOutputAddress = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( enableParams.chPairs[0].address );
    freezeSettings.fmmOutputMask = enableParams.chPairs[0].mask;
    freezeSettings.fmmOutputValue = enableParams.chPairs[0].value;

    errCode = BDSP_AudioTask_Freeze(handle->hTask, &freezeSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->state = BAPE_DecoderState_eFrozen;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_UnFreeze(
    BAPE_DecoderHandle handle
    )
{
    BAPE_OutputPort outputs[2];
    unsigned numOutputs = 0;
    BAPE_OutputPort_P_EnableParams enableParams;
    BDSP_AudioTaskUnFreezeSettings unfreezeSettings;
    BERR_Code errCode = BERR_SUCCESS;
    BAPE_PathConnection *pConnection;
    unsigned numChannelPairs, numChannels, i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BDBG_MSG(("%s(%p) [index %u]", __FUNCTION__, (void *)handle, handle->index));
    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eFrozen:
        break;
    case BAPE_DecoderState_eStarted:
    case BAPE_DecoderState_ePaused:
    case BAPE_DecoderState_eStopped:
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        BDBG_ERR(("Invalid decoder state %u.  Cannot Freeze", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    default:
        BDBG_ERR(("Unknown decoder state %u.  Cannot Freeze", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BAPE_PathNode_P_GetConnectedOutputs(&handle->node, 2, &numOutputs, outputs);

    if ( numOutputs != 1 )
    {
        BDBG_ERR(("%s - INVALID number of outputs (%d) connected to this decoder %p", __FUNCTION__, numOutputs, (void *)handle));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    if ( outputs[0]->getEnableParams == NULL )
    {
        BDBG_ERR(("%s - Cannot freeze output type (%d) connected to this decoder %p", __FUNCTION__, outputs[0]->type, (void *)handle));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    outputs[0]->getEnableParams(outputs[0], true, &enableParams);

    BDSP_AudioTask_GetDefaultUnFreezeSettings(&unfreezeSettings);
    unfreezeSettings.fmmOutputAddress = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( enableParams.chPairs[0].address );
    unfreezeSettings.fmmOutputMask = enableParams.chPairs[0].mask;
    unfreezeSettings.fmmOutputValue = enableParams.chPairs[0].value;


    BDBG_ASSERT(NULL != outputs[0]->mixer);
    pConnection = BLST_S_FIRST(&outputs[0]->mixer->pathNode.upstreamList);
    BDBG_ASSERT(NULL != pConnection);
    BDBG_ASSERT(NULL != pConnection->sfifoGroup);
    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pConnection->pSource->format);
    if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&pConnection->pSource->format) )
    {
        numChannels = 2*numChannelPairs;
    }
    else
    {
        numChannels = 1;
    }

    unfreezeSettings.ui32NumBuffers = numChannels;
    for ( i = 0; i < numChannelPairs; i++ )
    {
        unsigned hwIndex = BAPE_SfifoGroup_P_GetHwIndex(pConnection->sfifoGroup, i);
        unfreezeSettings.sCircBuffer[2*i].ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_P_SFIFO_TO_BASEADDR_REG(hwIndex) );
        unfreezeSettings.sCircBuffer[2*i].ui32EndAddr =  BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_P_SFIFO_TO_ENDADDR_REG(hwIndex) );
        unfreezeSettings.sCircBuffer[2*i].ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_P_SFIFO_TO_RDADDR_REG(hwIndex) );
        unfreezeSettings.sCircBuffer[2*i].ui32WriteAddr =  BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_P_SFIFO_TO_WRADDR_REG(hwIndex) );
        unfreezeSettings.sCircBuffer[2*i].ui32WrapAddr =  BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_P_SFIFO_TO_ENDADDR_REG(hwIndex) );
        if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&pConnection->pSource->format) )
        {
            hwIndex++;
            unfreezeSettings.sCircBuffer[(2*i)+1].ui32BaseAddr = unfreezeSettings.sCircBuffer[2*i].ui32BaseAddr + BAPE_P_RINGBUFFER_STRIDE;
            unfreezeSettings.sCircBuffer[(2*i)+1].ui32EndAddr = unfreezeSettings.sCircBuffer[2*i].ui32EndAddr + BAPE_P_RINGBUFFER_STRIDE;
            unfreezeSettings.sCircBuffer[(2*i)+1].ui32ReadAddr = unfreezeSettings.sCircBuffer[2*i].ui32ReadAddr + BAPE_P_RINGBUFFER_STRIDE;
            unfreezeSettings.sCircBuffer[(2*i)+1].ui32WriteAddr = unfreezeSettings.sCircBuffer[2*i].ui32WriteAddr + BAPE_P_RINGBUFFER_STRIDE;
            unfreezeSettings.sCircBuffer[(2*i)+1].ui32WrapAddr = unfreezeSettings.sCircBuffer[2*i].ui32WrapAddr + BAPE_P_RINGBUFFER_STRIDE;
        }
    }

    errCode = BDSP_AudioTask_UnFreeze(handle->hTask, &unfreezeSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    handle->state = BAPE_DecoderState_eStarted;
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_DisableForFlush(
    BAPE_DecoderHandle handle
    )
{
    BAPE_DecoderState newState = BAPE_DecoderState_eMax;

    BDBG_MSG(("BAPE_Decoder_DisableForFlush(%p) [index %u]", (void *)handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot disable for flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eFrozen:
        BDBG_ERR(("Decoder %u is frozen, cannot disable for flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
        newState = BAPE_DecoderState_eDisabled;
        break;
    case BAPE_DecoderState_ePaused:
        newState = BAPE_DecoderState_eDisabledPaused;
        break;
    case BAPE_DecoderState_eDisabled:
    case BAPE_DecoderState_eDisabledPaused:
        /* No change */
        return BERR_SUCCESS;
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    handle->lastValidPts = 0;

    /* Transition State */
    handle->state = newState;
    BKNI_EnterCriticalSection();
    BKNI_LeaveCriticalSection();
    BAPE_Decoder_P_Stop(handle);

    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_Flush(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode;
    bool paused = false;

    BDBG_MSG(("BAPE_Decoder_Flush(%p) [index %u]", (void *)handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStopped:
        BDBG_ERR(("Decoder %u is not started, cannot flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eFrozen:
        BDBG_ERR(("Decoder %u is frozen, cannot flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eStarted:
    case BAPE_DecoderState_ePaused:
        BDBG_ERR(("Decoder %u is not disabled, cannot flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BAPE_DecoderState_eDisabled:
        break;
    case BAPE_DecoderState_eDisabledPaused:
        paused = true;
        break;
    default:
        BDBG_ERR(("Unexpected decoder state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BAPE_Decoder_P_Start(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( paused )
    {
        errCode = BAPE_Decoder_Pause(handle);
        if ( errCode )
        {
            /* Should never happen, but just for completeness */
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

void BAPE_Decoder_GetTsmSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    )
{
    BKNI_EnterCriticalSection();
    BAPE_Decoder_GetTsmSettings_isr(handle, pSettings);
    BKNI_LeaveCriticalSection();
}

void BAPE_Decoder_GetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmSettings *pSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->tsmSettings;
}

BERR_Code BAPE_Decoder_SetTsmSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    )
{
    BERR_Code errCode;
    BKNI_EnterCriticalSection();
    errCode = BAPE_Decoder_SetTsmSettings_isr(handle, pSettings);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Set Audio Decoder TSM Settings in isr context
***************************************************************************/
BERR_Code BAPE_Decoder_SetTsmSettings_isr(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    if ( BAPE_Decoder_P_TaskValid_isr(handle) )
    {
        uint32_t pathDelay = 0;
        BDSP_AudioTaskTsmSettings tsmSettings;
        BDSP_AudioStage_GetTsmSettings_isr(handle->hPrimaryStage, &tsmSettings);
        if ( handle->startSettings.inputPort )
        {
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, eTsmEnable, BDSP_eTsmBool_False);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, eAstmEnable, BDSP_eTsmBool_False);
        }
        else
        {
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, ui32STCAddr, BDSP_RAAGA_REGSET_ADDR_FOR_DSP( BAPE_CHIP_GET_STC_ADDRESS(handle->startSettings.stcIndex))) ;
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, eTsmEnable, pSettings->tsmEnabled?BDSP_eTsmBool_True:BDSP_eTsmBool_False);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, eAstmEnable, pSettings->astmEnabled?BDSP_eTsmBool_True:BDSP_eTsmBool_False);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, ePlayBackOn, pSettings->playback?BDSP_eTsmBool_True:BDSP_eTsmBool_False);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, ui32AVOffset, pSettings->ptsOffset);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, i32TSMDiscardThreshold, pSettings->thresholds.discard*45);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, i32TSMGrossThreshold, pSettings->thresholds.grossAdjustment*45);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, i32TSMSmoothThreshold, pSettings->thresholds.smoothTrack*45);
            BAPE_DSP_P_SET_VARIABLE(tsmSettings, i32TSMSyncLimitThreshold, pSettings->thresholds.syncLimit*45);
            BDBG_MSG(("BAPE_Decoder_SetTsmSettings_isr: eSTCValid = %u", tsmSettings.eSTCValid));
            if ( handle->startSettings.nonRealTime )
            {
                BAPE_DSP_P_SET_VARIABLE(tsmSettings, ui32SwSTCOffset, pSettings->stcOffset);
            }
        }
        errCode = BAPE_Decoder_P_GetPathDelay_isrsafe(handle, &pathDelay);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        BAPE_DSP_P_SET_VARIABLE(tsmSettings, ui32AudioOffset, pathDelay*45);
        errCode = BDSP_AudioStage_SetTsmSettings_isr(handle->hPrimaryStage, &tsmSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    handle->tsmSettings = *pSettings;

    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_GetTsmStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus  /* [out] */
    )
{
    BERR_Code errCode;
    BKNI_Memset(pStatus, 0, sizeof(BAPE_DecoderTsmStatus));
    BKNI_EnterCriticalSection();
    errCode = BAPE_Decoder_GetTsmStatus_isr(handle, pStatus);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        return errCode;     /* BERR_TRACE intentionally omitted */
    }
    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_GetTsmStatus_isr(
    BAPE_DecoderHandle handle,
    BAPE_DecoderTsmStatus *pStatus  /* [out] */
    )
{
    BERR_Code errCode;
    BDSP_AudioTaskTsmStatus tsmStatus;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pStatus);

    if ( BAPE_Decoder_P_TaskValid_isr(handle) )
    {
        errCode = BDSP_AudioStage_GetTsmStatus_isr(handle->hPrimaryStage, &tsmStatus);
        if ( errCode )
        {
            BKNI_Memset(pStatus, 0, sizeof(*pStatus));

            if (handle->state == BAPE_DecoderState_eStarted || handle->state == BAPE_DecoderState_ePaused)
            {
                if (handle->lastValidPts != 0)
                {
                    pStatus->ptsInfo.ui32CurrentPTS = handle->lastValidPts;
                    pStatus->ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromValidPTS; /* Mark PTS as invalid */
                }
                else
                {
                    pStatus->ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromInvalidPTS; /* Mark PTS as invalid */
                }
            }
            else
            {
                pStatus->ptsInfo.ui32CurrentPTS = handle->lastValidPts;
                pStatus->ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromInvalidPTS; /* Mark PTS as invalid */
            }

            return errCode;     /* BERR_TRACE intentionally omitted */
        }
        else
        {
            if (handle->state == BAPE_DecoderState_eStarted || handle->state == BAPE_DecoderState_ePaused)
            {
                handle->lastValidPts = tsmStatus.ui32RunningPts;
            }
        }

        BAPE_Decoder_P_ConvertTsmStatus_isr(pStatus, &tsmStatus);
    }

    return BERR_SUCCESS;
}

void BAPE_Decoder_GetSettings(
    BAPE_DecoderHandle handle,
    BAPE_DecoderSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

BERR_Code BAPE_Decoder_SetSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings->decodeRate < (BAPE_NORMAL_DECODE_RATE/2) || pSettings->decodeRate > (2*BAPE_NORMAL_DECODE_RATE) )
    {
        BDBG_ERR(("Audio trick play is supported for 0.5x to 2x playback only. (rate=%u)", pSettings->decodeRate));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->settings.multichannelFormat != pSettings->multichannelFormat )
    {
        if ( handle->state != BAPE_DecoderState_eStopped )
        {
            BDBG_ERR(("Cannot change multichannel format while decoder is running."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        /* This will be applied later during the next start */
    }

    handle->settings = *pSettings;

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        errCode = BAPE_Decoder_P_ApplyCodecSettings(handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        if ( handle->ddre && (NULL == handle->fwMixer || handle->fwMixerMaster == true) )
        {
            errCode = BAPE_DolbyDigitalReencode_P_SettingsChanged(handle->ddre, handle);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

        if ( handle->startSettings.decodeRateControl )
        {
            errCode = BAPE_Decoder_P_ApplyDsolaSettings(handle);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        if ( handle->startSettings.karaokeModeEnabled && handle->hKaraokeStage)
        {
            errCode = BAPE_Decoder_P_ApplyKaraokeSettings(handle);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

void BAPE_Decoder_GetStatus(
    BAPE_DecoderHandle handle,
    BAPE_DecoderStatus *pStatus     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BAPE_DecoderStatus));
    pStatus->codec = BAVC_AudioCompressionStd_eMax;
    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        pStatus->codec = handle->startSettings.codec;
        BAPE_Decoder_GetTsmStatus(handle, &pStatus->tsmStatus);
        pStatus->sampleRate = handle->pcmOutputSampleRate;
        BAPE_Decoder_P_GetCodecStatus(handle, pStatus);
        pStatus->running = true;
        BKNI_EnterCriticalSection();
        BAPE_Decoder_P_GetDataSyncStatus_isr(handle, &pStatus->cdbUnderFlowCount);
        BKNI_LeaveCriticalSection();
        if ( handle->startSettings.inputPort )
        {
            pStatus->halted = handle->halted || handle->startSettings.inputPort->halted;
        }
    }
}

void BAPE_Decoder_GetPresentationInfo(
    BAPE_DecoderHandle handle,
    unsigned presentationIndex,
    BAPE_DecoderPresentationInfo *pInfo     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pInfo);

    BKNI_Memset(pInfo, 0, sizeof(BAPE_DecoderPresentationInfo));
    pInfo->codec = BAVC_AudioCompressionStd_eMax;

    if ( handle->state != BAPE_DecoderState_eStopped )
    {
        switch ( handle->startSettings.codec )
        {
        case BAVC_AudioCompressionStd_eAc4:
            BAPE_Decoder_P_GetAc4PresentationInfo(handle, presentationIndex, pInfo);
            break;
        default:
            BDBG_WRN(("%s: Presentation info not supported for BAVC codec %lu", __FUNCTION__, (unsigned long)handle->startSettings.codec));
            return;
            break;
        }
        pInfo->codec = handle->startSettings.codec;
    }
}


void BAPE_Decoder_GetCodecSettings(
    BAPE_DecoderHandle handle,
    BAVC_AudioCompressionStd codec,
    BAPE_DecoderCodecSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);
    switch ( codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
        *pSettings = handle->ac3Settings;
        break;
    case BAVC_AudioCompressionStd_eAc3Plus:
        *pSettings = handle->ac3PlusSettings;
        break;
    case BAVC_AudioCompressionStd_eAc4:
        *pSettings = handle->ac4Settings;
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        *pSettings = handle->aacSettings;
        break;
    case BAVC_AudioCompressionStd_eAacPlusAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
        *pSettings = handle->aacPlusSettings;
        break;
    case BAVC_AudioCompressionStd_eDts:
    case BAVC_AudioCompressionStd_eDtshd:
    case BAVC_AudioCompressionStd_eDtsLegacy:
        *pSettings = handle->dtsSettings;
        break;
    case BAVC_AudioCompressionStd_eDtsExpress:
        *pSettings = handle->dtsExpressSettings;
        break;
    case BAVC_AudioCompressionStd_eAdpcm:
        *pSettings = handle->adpcmSettings;
        break;
    case BAVC_AudioCompressionStd_eIlbc:
        *pSettings = handle->ilbcSettings;
        break;
    case BAVC_AudioCompressionStd_eIsac:
        *pSettings = handle->isacSettings;
        break;
    case BAVC_AudioCompressionStd_eWmaPro:
        *pSettings = handle->wmaProSettings;
        break;
    default:
        break;
    }
    pSettings->codec = codec;
}

BERR_Code BAPE_Decoder_SetCodecSettings(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderCodecSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    bool updateTask=false;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);

    /* Passthrough params are not user-changeable, so no need to set them.
       Default to update task if the codec matches for decode/simul cases. */
    if ( handle->startSettings.codec == pSettings->codec &&
         !handle->passthrough )
    {
        updateTask = true;
    }

    switch ( pSettings->codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
        handle->ac3Settings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eAc3Plus:
        handle->ac3PlusSettings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eAc4:
        handle->ac4Settings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        handle->aacSettings = *pSettings;
        if ( handle->startSettings.codec == BAVC_AudioCompressionStd_eAacLoas ||
             handle->startSettings.codec == BAVC_AudioCompressionStd_eAacAdts )
        {
            /* We don't manage AAC ADTS/LOAS as separate configs */
            updateTask = true;
        }
        break;
    case BAVC_AudioCompressionStd_eAacPlusLoas:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        handle->aacPlusSettings = *pSettings;
        if ( handle->startSettings.codec == BAVC_AudioCompressionStd_eAacPlusLoas ||
             handle->startSettings.codec == BAVC_AudioCompressionStd_eAacPlusAdts )
        {
            /* We don't manage AAC ADTS/LOAS as separate configs */
            updateTask = true;
        }
        break;
    case BAVC_AudioCompressionStd_eDts:
    case BAVC_AudioCompressionStd_eDtshd:
    case BAVC_AudioCompressionStd_eDtsLegacy:
        handle->dtsSettings = *pSettings;
        if ( handle->startSettings.codec == BAVC_AudioCompressionStd_eDts ||
             handle->startSettings.codec == BAVC_AudioCompressionStd_eDtshd ||
             handle->startSettings.codec == BAVC_AudioCompressionStd_eDtsLegacy )
        {
            /* We don't manage the multitude of DTS variants as separate configs */
            updateTask = true;
        }
        break;
    case BAVC_AudioCompressionStd_eDtsExpress:
        handle->dtsExpressSettings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eAdpcm:
        handle->adpcmSettings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eIlbc:
        handle->ilbcSettings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eIsac:
        handle->isacSettings = *pSettings;
        break;
    case BAVC_AudioCompressionStd_eWmaPro:
        handle->wmaProSettings = *pSettings;
        break;
    default:
        updateTask = false;
        break;
    }

    if ( handle->state != BAPE_DecoderState_eStopped && updateTask )
    {
        errCode = BAPE_Decoder_P_ApplyCodecSettings(handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        if ( handle->ddre && (NULL == handle->fwMixer || handle->fwMixerMaster == true) )
        {
            errCode = BAPE_DolbyDigitalReencode_P_SettingsChanged(handle->ddre, handle);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

void BAPE_Decoder_GetConnector(
    BAPE_DecoderHandle handle,
    BAPE_ConnectorFormat format,
    BAPE_Connector *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pConnector);

    switch ( format )
    {
    case BAPE_ConnectorFormat_eStereo:
    case BAPE_ConnectorFormat_eMultichannel:
    case BAPE_ConnectorFormat_eCompressed:
    case BAPE_ConnectorFormat_eCompressed4x:
    case BAPE_ConnectorFormat_eCompressed16x:
    case BAPE_ConnectorFormat_eMono:
        *pConnector = &handle->node.connectors[format];
        break;
    default:
        BDBG_ERR(("Unsupported data path format %u", format));
        *pConnector = NULL;
        break;
    }
}

void BAPE_Decoder_GetInterruptHandlers(
    BAPE_DecoderHandle handle,
    BAPE_DecoderInterruptHandlers *pInterrupts     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = handle->interrupts;
}

BERR_Code BAPE_Decoder_SetInterruptHandlers(
    BAPE_DecoderHandle handle,
    const BAPE_DecoderInterruptHandlers *pInterrupts
    )
{
    BERR_Code errCode;
    BDSP_AudioInterruptHandlers interrupts;
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BDBG_ASSERT(NULL != pInterrupts);
    BKNI_EnterCriticalSection();

    BDSP_AudioTask_GetInterruptHandlers_isr(handle->hTask, &interrupts);
        interrupts.firstPts.pCallback_isr = BAPE_Decoder_P_FirstPts_isr;
        interrupts.firstPts.pParam1 = handle;
        interrupts.tsmFail.pCallback_isr = BAPE_Decoder_P_TsmFail_isr;
        interrupts.tsmFail.pParam1 = handle;
        interrupts.tsmPass.pCallback_isr = BAPE_Decoder_P_TsmPass_isr;
        interrupts.tsmPass.pParam1 = handle;
        interrupts.sampleRateChange.pCallback_isr = BAPE_Decoder_P_SampleRateChange_isr;
        interrupts.sampleRateChange.pParam1 = handle;
        if ( handle->startSettings.inputPort )
        {
            interrupts.lock.pCallback_isr = NULL;
            interrupts.lock.pParam1 = NULL;
            interrupts.lock.param2 = 0;
            interrupts.unlock.pCallback_isr = NULL;
            interrupts.unlock.pParam1 = NULL;
            interrupts.unlock.param2 = 0;
        }
        else
        {
            interrupts.lock.pCallback_isr = pInterrupts->lock.pCallback_isr;
            interrupts.lock.pParam1 = pInterrupts->lock.pParam1;
            interrupts.lock.param2 = pInterrupts->lock.param2;
            interrupts.unlock.pCallback_isr = pInterrupts->unlock.pCallback_isr;
            interrupts.unlock.pParam1 = pInterrupts->unlock.pParam1;
            interrupts.unlock.param2 = pInterrupts->unlock.param2;
        }
        interrupts.modeChange.pCallback_isr = BAPE_Decoder_P_ModeChange_isr;
        interrupts.modeChange.pParam1 = handle;
        interrupts.bitrateChange.pCallback_isr = BAPE_Decoder_P_BitrateChange_isr;
        interrupts.bitrateChange.pParam1 = handle;
        interrupts.cdbItbOverflow.pCallback_isr = BAPE_Decoder_P_Overflow_isr;
        interrupts.cdbItbOverflow.pParam1 = handle;
#if 0 /* disable due to out of control interrupts */
        interrupts.CdbItbUnderflowAfterGateOpen.pCallback_isr = BAPE_Decoder_P_Underflow_isr;
        interrupts.CdbItbUnderflowAfterGateOpen.pParam1 = handle;
#endif
        interrupts.statusReady.pCallback_isr = BAPE_Decoder_P_StatusReady_isr;
        interrupts.statusReady.pParam1 = handle;
        interrupts.ancillaryData.pCallback_isr = pInterrupts->ancillaryData.pCallback_isr;
        interrupts.ancillaryData.pParam1 = pInterrupts->ancillaryData.pParam1;
        interrupts.ancillaryData.param2 = pInterrupts->ancillaryData.param2;
        interrupts.dialnormChange.pCallback_isr = BAPE_Decoder_P_DialnormChange_isr;
        interrupts.dialnormChange.pParam1 = handle;
        interrupts.encoderOutputOverflow.pCallback_isr = BAPE_Decoder_P_EncoderOverflow_isr;
        interrupts.encoderOutputOverflow.pParam1 = handle;
        if ( handle->type == BAPE_DecoderType_eDecodeToMemory )
        {
            interrupts.onDemandAudioFrameDelivered.pCallback_isr = pInterrupts->hostBufferReady.pCallback_isr;
            interrupts.onDemandAudioFrameDelivered.pParam1 = pInterrupts->hostBufferReady.pParam1;
            interrupts.onDemandAudioFrameDelivered.param2 = pInterrupts->hostBufferReady.param2;
        }
    errCode = BDSP_AudioTask_SetInterruptHandlers_isr(handle->hTask, &interrupts);
    if ( errCode )
    {
        BKNI_LeaveCriticalSection();
        return BERR_TRACE(errCode);
    }

    handle->interrupts = *pInterrupts;
    BKNI_LeaveCriticalSection();
    return BERR_SUCCESS;
}

void BAPE_Decoder_P_SetSampleRate_isr(BAPE_DecoderHandle handle, unsigned sampleRate)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[BAPE_ConnectorFormat_eStereo], sampleRate);
    BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[BAPE_ConnectorFormat_eMultichannel], sampleRate);
    BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[BAPE_ConnectorFormat_eMono], sampleRate);
    /* Catch AC3+ Passthrough */
    if ( handle->node.connectors[BAPE_ConnectorFormat_eCompressed].format.type == BAPE_DataType_eIec61937x4 )
    {
        if ( sampleRate == 32000 )
        {
            BDBG_WRN(("AC3 Plus compressed passthrough is not supported at 32kHz sampling rates."));
        }
        BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], sampleRate*4);
    }
    else
    {
        BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed], sampleRate);
    }
    BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed4x], sampleRate*4);
    BAPE_Connector_P_SetSampleRate_isr(&handle->node.connectors[BAPE_ConnectorFormat_eCompressed16x], sampleRate*4);    /* HBR is sent at 4x with 4 samples per clock */
}

void BAPE_Decoder_GetDefaultCdbItbConfig(
    BAPE_DecoderHandle handle,
    BAVC_CdbItbConfig *pConfig  /* [out] */
    )
{
    if (handle) {
        BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    }
    BSTD_UNUSED(handle);
    BKNI_Memset(pConfig, 0, sizeof(BAVC_CdbItbConfig));
    pConfig->Cdb.Length = 256*1024;
    pConfig->Cdb.Alignment = 6; /* cache line on 4380 */
    pConfig->Itb.Length = 128*1024;
    pConfig->Itb.Alignment = 6; /* cache line on 4380 */
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    pConfig->Cdb.LittleEndian = true;
#else
    pConfig->Cdb.LittleEndian = false;
#endif
}

BERR_Code BAPE_Decoder_SetStcValid_isr(
    BAPE_DecoderHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    if ( BAPE_Decoder_P_TaskValid_isr(handle) )
    {
        BDSP_AudioTaskTsmSettings tsmSettings;
        BERR_Code errCode;
        BDSP_AudioStage_GetTsmSettings_isr(handle->hPrimaryStage, &tsmSettings);
        BAPE_DSP_P_SET_VARIABLE(tsmSettings, eSTCValid, BDSP_eTsmBool_True);
        errCode = BDSP_AudioStage_SetTsmSettings_isr(handle->hPrimaryStage, &tsmSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    return BERR_SUCCESS;
}

static void BAPE_Decoder_P_SetupDefaults(BAPE_DecoderHandle handle)
{
    BDSP_AudioTaskTsmSettings tsmSettings;

    BDSP_AudioTask_GetDefaultTsmSettings(&tsmSettings, sizeof(tsmSettings));
    handle->tsmSettings.tsmEnabled = tsmSettings.eTsmEnable == BDSP_eTsmBool_True?true:false;
    handle->tsmSettings.astmEnabled = tsmSettings.eAstmEnable == BDSP_eTsmBool_True?true:false;
    handle->tsmSettings.playback = tsmSettings.ePlayBackOn == BDSP_eTsmBool_True?true:false;
    handle->tsmSettings.ptsOffset = tsmSettings.ui32AVOffset/45;
    handle->tsmSettings.thresholds.discard = tsmSettings.i32TSMDiscardThreshold/45;
    handle->tsmSettings.thresholds.grossAdjustment = tsmSettings.i32TSMGrossThreshold/45;
    handle->tsmSettings.thresholds.smoothTrack = tsmSettings.i32TSMSmoothThreshold/45;
    handle->tsmSettings.thresholds.syncLimit = tsmSettings.i32TSMSyncLimitThreshold/45;

    BAPE_Decoder_P_GetDefaultCodecSettings(handle);
}

static void BAPE_Decoder_P_ConvertTsmStatus_isr(BAPE_DecoderTsmStatus *pStatus, const BDSP_AudioTaskTsmStatus *pDspStatus)
{
    BKNI_Memset(&pStatus->ptsInfo, 0, sizeof(BAVC_PTSInfo));
    pStatus->ptsInfo.ui32CurrentPTS = pDspStatus->ui32RunningPts;
    switch ( pDspStatus->ePtsType )
    {
    case BDSP_PtsType_eCoded:
        pStatus->ptsInfo.ePTSType = BAVC_PTSType_eCoded;
        break;
    case BDSP_PtsType_eInterpolatedFromValidPTS:
        pStatus->ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromValidPTS;
        break;
    default:
        BDBG_WRN(("Invalid DSP PTS type %u", pDspStatus->ePtsType));
        /* Fall through */
    case BDSP_PtsType_eInterpolatedFromInvalidPTS:
        pStatus->ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromInvalidPTS;
        break;
    }
    pStatus->ptsStcDifference = pDspStatus->i32PtsToStcPhase;
    pStatus->lastFrameLength = pDspStatus->i32FrameSizeIn45khzTicks/45;
}

static void BAPE_Decoder_P_FirstPts_isr(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);

    handle->lastValidPts = pTsmStatus->ui32RunningPts;
    if ( handle->interrupts.firstPts.pCallback_isr )
    {
        BAPE_DecoderTsmStatus tsmStatus;
        BAPE_Decoder_P_ConvertTsmStatus_isr(&tsmStatus, pTsmStatus);
        handle->interrupts.firstPts.pCallback_isr(handle->interrupts.firstPts.pParam1, handle->interrupts.firstPts.param2, &tsmStatus);
    }
}

static void BAPE_Decoder_P_TsmFail_isr(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);

    if ( handle->interrupts.tsmFail.pCallback_isr )
    {
        BAPE_DecoderTsmStatus tsmStatus;
        BAPE_Decoder_P_ConvertTsmStatus_isr(&tsmStatus, pTsmStatus);
        handle->interrupts.tsmFail.pCallback_isr(handle->interrupts.tsmFail.pParam1, handle->interrupts.tsmFail.param2, &tsmStatus);
    }
}

static void BAPE_Decoder_P_TsmPass_isr(void *pParam1, int param2, const BDSP_AudioTaskTsmStatus *pTsmStatus)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);

    if ( handle->interrupts.tsmPass.pCallback_isr )
    {
        BAPE_DecoderTsmStatus tsmStatus;
        BAPE_Decoder_P_ConvertTsmStatus_isr(&tsmStatus, pTsmStatus);
        handle->interrupts.tsmPass.pCallback_isr(handle->interrupts.tsmPass.pParam1, handle->interrupts.tsmPass.param2, &tsmStatus);
    }
}

static void BAPE_Decoder_P_SampleRateChange_isr(void *pParam1, int param2, unsigned streamSampleRate, unsigned baseSampleRate)
{
    BAPE_DecoderHandle handle = pParam1;
    unsigned fmmSampleRate=0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);

    BDBG_MSG(("%s: streamSampleRate %d, baseSampleRate %d", __FUNCTION__, streamSampleRate, baseSampleRate));

    /* Store stream sample rate for user status */
    handle->streamSampleRate = streamSampleRate;

    /* Determine FMM sample rate */
    if ( handle->passthrough )
    {
        switch ( streamSampleRate )
        {
        case 192000:
        case 176400:
        case 128000:
            fmmSampleRate = streamSampleRate/4;
            break;
        case 96000:
        case 88200:
        case 64000:
            fmmSampleRate = streamSampleRate/2;
            break;
        default:
        case 48000:
        case 44100:
        case 32000:
            fmmSampleRate = streamSampleRate;
            break;
        case 24000:
        case 22050:
        case 16000:
            fmmSampleRate = streamSampleRate*2; /* LSF */
            break;
        case 12000:
        case 11025:
        case  8000:
            fmmSampleRate = streamSampleRate*4; /* QSF */
            break;
        }
    }
    else
    {
        fmmSampleRate = baseSampleRate;
    }

    handle->pcmOutputSampleRate = fmmSampleRate;

    BDBG_MSG(("Sample Rate Interrupt Received [decoder %u] Stream %u FMM %u", handle->index, streamSampleRate, fmmSampleRate));
    /* Apply sample rate downstream - passthrough is at the stream rate and decode is the base rate */
    BAPE_Decoder_P_SetSampleRate_isr(handle, fmmSampleRate);
    if ( handle->interrupts.sampleRateChange.pCallback_isr )
    {
        /* Send stream rate to the application */
        handle->interrupts.sampleRateChange.pCallback_isr(handle->interrupts.sampleRateChange.pParam1, handle->interrupts.sampleRateChange.param2, streamSampleRate);
    }

    if ( handle->interrupts.fmmSampleRateChange.pCallback_isr )
    {
        /* Send stream rate to the application */
        handle->interrupts.fmmSampleRateChange.pCallback_isr(handle->interrupts.fmmSampleRateChange.pParam1, handle->interrupts.fmmSampleRateChange.param2, fmmSampleRate);
    }
}

static void BAPE_Decoder_P_ModeChange_isr(void *pParam1, int param2, unsigned mode)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);
    handle->mode = mode;
    BDBG_MSG(("Mode Change Received [decoder %u] acmod now %u", handle->index, mode));
    if ( handle->interrupts.modeChange.pCallback_isr )
    {
        handle->interrupts.modeChange.pCallback_isr(handle->interrupts.modeChange.pParam1, handle->interrupts.modeChange.param2);
    }
}

static void BAPE_Decoder_P_BitrateChange_isr(void *pParam1, int param2, const BDSP_AudioBitRateChangeInfo *pInfo)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);
    BKNI_Memcpy(&handle->bitRateInfo, pInfo, sizeof(*pInfo));
#if 0
    BDBG_MSG(("Bitrate Interrupt Received [decoder %u]", handle->index));
#endif
    if ( handle->interrupts.bitrateChange.pCallback_isr )
    {
        handle->interrupts.bitrateChange.pCallback_isr(handle->interrupts.bitrateChange.pParam1, handle->interrupts.bitrateChange.param2);
    }
}

static void BAPE_Decoder_P_Overflow_isr(void *pParam1, int param2)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);
    BDBG_MSG(("CDB/ITB Overflow Interrupt Received [decoder %u]", handle->index));
    if ( handle->interrupts.cdbItbOverflow.pCallback_isr )
    {
        handle->interrupts.cdbItbOverflow.pCallback_isr(handle->interrupts.cdbItbOverflow.pParam1, handle->interrupts.cdbItbOverflow.param2);
    }
}

#if 0 /* disable due to out of control interrupts */
static void BAPE_Decoder_P_Underflow_isr(void *pParam1, int param2)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);
    BDBG_MSG(("CDB/ITB Underflow Interrupt Received [decoder %u]", handle->index));
    if ( handle->interrupts.cdbItbUnderflow.pCallback_isr )
    {
        handle->interrupts.cdbItbUnderflow.pCallback_isr(handle->interrupts.cdbItbUnderflow.pParam1, handle->interrupts.cdbItbUnderflow.param2);
    }
}
#endif

static void BAPE_Decoder_P_StatusReady_isr(void *pParam1, int param2)
{
    BAPE_DecoderHandle handle = pParam1;
    BDSP_AudioInterruptHandlers interrupts;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);
    BDBG_MSG(("Status Ready Interrupt Received [decoder %u]", handle->index));
    if ( handle->interrupts.statusReady.pCallback_isr )
    {
        handle->interrupts.statusReady.pCallback_isr(handle->interrupts.statusReady.pParam1, handle->interrupts.statusReady.param2);
    }

    /* This interrupt will fire each frame, but we only want the first one.  Disable the interrupt after it fires. */
    BDSP_AudioTask_GetInterruptHandlers_isr(handle->hTask, &interrupts);
    interrupts.statusReady.pCallback_isr = NULL;
    interrupts.statusReady.pParam1 = handle;
    (void)BDSP_AudioTask_SetInterruptHandlers_isr(handle->hTask, &interrupts);
}

static BERR_Code BAPE_Decoder_P_InputFormatChange_isr(
    BAPE_PathNode *pNode,
    BAPE_InputPort inputPort
    )
{
    BAPE_DecoderHandle handle;
    bool needToHalt = false;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    BKNI_ASSERT_ISR_CONTEXT();
    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&inputPort->format)  &&  inputPort->format.sampleRate > 48000 )
    {
        BDBG_MSG(("Decoder input PCM sample rate of %u exceeds 48000.  Halting capture.", inputPort->format.sampleRate ));
        needToHalt = true;
    }

    /* On the fly format changes are not possible */
    if ( (inputPort->format.type != handle->inputPortFormat.type) )
    {
        BDBG_MSG(("Input data format has changed (%s->%s).  Halting capture.", BAPE_FMT_P_GetTypeName_isrsafe(&handle->inputPortFormat), BAPE_FMT_P_GetTypeName_isrsafe(&inputPort->format)));
        needToHalt = true;
    }

    if ( needToHalt )
    {
        if ( handle->interrupts.inputHalted.pCallback_isr )
        {
           handle->interrupts.inputHalted.pCallback_isr(handle->interrupts.inputHalted.pParam1, handle->interrupts.inputHalted.param2);
        }
        handle->halted = true;
        /* Intentionally omitted BERR_TRACE */
        return BERR_NOT_SUPPORTED;
    }
    return BERR_SUCCESS;
}

static void BAPE_Decoder_P_UnlinkStages(BAPE_DecoderHandle handle)
{
    if ( handle->hPrimaryStage )
    {
        BDSP_Stage_RemoveAllInputs(handle->hPrimaryStage);
        BDSP_Stage_RemoveAllOutputs(handle->hPrimaryStage);
    }
    if ( handle->hSrcStageStereo )
    {
        BDSP_Stage_RemoveAllInputs(handle->hSrcStageStereo);
        BDSP_Stage_RemoveAllOutputs(handle->hSrcStageStereo);
    }
    if ( handle->hSrcStageMultichannel )
    {
        BDSP_Stage_RemoveAllInputs(handle->hSrcStageMultichannel);
        BDSP_Stage_RemoveAllOutputs(handle->hSrcStageMultichannel);
    }
    if ( handle->hDsolaStageStereo )
    {
        BDSP_Stage_RemoveAllInputs(handle->hDsolaStageStereo);
        BDSP_Stage_RemoveAllOutputs(handle->hDsolaStageStereo);
    }
    if ( handle->hDsolaStageMultichannel )
    {
        BDSP_Stage_RemoveAllInputs(handle->hDsolaStageMultichannel);
        BDSP_Stage_RemoveAllOutputs(handle->hDsolaStageMultichannel);
    }
    if ( handle->hKaraokeStage )
    {
        BDSP_Stage_RemoveAllInputs(handle->hKaraokeStage);
        BDSP_Stage_RemoveAllOutputs(handle->hKaraokeStage);
    }
    if ( handle->hPassthroughStage )
    {
        BDSP_Stage_RemoveAllInputs(handle->hPassthroughStage);
        BDSP_Stage_RemoveAllOutputs(handle->hPassthroughStage);
    }
    if ( handle->hOutputFormatter )
    {
        BDSP_Stage_RemoveAllInputs(handle->hOutputFormatter);
        BDSP_Stage_RemoveAllOutputs(handle->hOutputFormatter);
    }
}

static BERR_Code BAPE_Decoder_P_ApplyFramesyncSettings(BAPE_DecoderHandle handle)
{
    BDSP_AudioTaskDatasyncSettings datasyncSettings;
    BERR_Code errCode;

    errCode = BDSP_AudioStage_GetDatasyncSettings(handle->hPrimaryStage, &datasyncSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    datasyncSettings.eFrameSyncType = BDSP_Raaga_Audio_DatasyncType_eNone;
    if ( handle->startSettings.inputPort )
    {
        /* Setup input port specifics */
        datasyncSettings.eEnableTargetSync = BDSP_AF_P_eDisable;
        datasyncSettings.eForceCompleteFirstFrame = BDSP_AF_P_eEnable;
        switch ( handle->startSettings.inputPort->type )
        {
        case BAPE_InputPortType_eI2s:
            BDBG_ASSERT(0 == handle->startSettings.inputPort->index || 1 == handle->startSettings.inputPort->index);
            datasyncSettings.eAudioIpSourceType = (handle->startSettings.inputPort->index == 0) ? BDSP_Audio_AudioInputSource_eExtI2s0 : BDSP_Audio_AudioInputSource_eCapPortRfI2s;
            datasyncSettings.uAudioIpSourceDetail.ui32SamplingFrequency = handle->startSettings.inputPort->format.sampleRate;
            break;
#if defined BCHP_SPDIF_RCVR_CTRL_STATUS
        case BAPE_InputPortType_eSpdif:
            datasyncSettings.eAudioIpSourceType = BDSP_Audio_AudioInputSource_eCapPortSpdif;
            datasyncSettings.uAudioIpSourceDetail.ui32MaiCtrlStatusRegAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP((BCHP_PHYSICAL_OFFSET + BCHP_SPDIF_RCVR_CTRL_STATUS));
            break;

#elif defined BCHP_AUD_FMM_IOP_IN_SPDIF_0_STATUS
        case BAPE_InputPortType_eSpdif:
            datasyncSettings.eAudioIpSourceType = BDSP_Audio_AudioInputSource_eCapPortSpdif;
            datasyncSettings.uAudioIpSourceDetail.ui32MaiCtrlStatusRegAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP((BCHP_PHYSICAL_OFFSET + BCHP_AUD_FMM_IOP_IN_SPDIF_0_STATUS));
            break;
#endif
#if defined BCHP_HDMI_RCVR_CTRL_MAI_FORMAT
        case BAPE_InputPortType_eMai:
            datasyncSettings.eAudioIpSourceType = BDSP_Audio_AudioInputSource_eCapPortHdmi;
            datasyncSettings.uAudioIpSourceDetail.ui32MaiCtrlStatusRegAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP((BCHP_PHYSICAL_OFFSET + BCHP_HDMI_RCVR_CTRL_MAI_FORMAT));
            break;
#elif defined BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT
        case BAPE_InputPortType_eMai:
            datasyncSettings.eAudioIpSourceType = BDSP_Audio_AudioInputSource_eCapPortHdmi;
            datasyncSettings.uAudioIpSourceDetail.ui32MaiCtrlStatusRegAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP((BCHP_PHYSICAL_OFFSET + BCHP_AUD_FMM_IOP_IN_HDMI_0_MAI_FORMAT));
            break;
#endif
        default:
            BDBG_ERR(("Input %s is not supported.", handle->startSettings.inputPort->pName));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        switch ( handle->startSettings.codec )
        {
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eAc3Plus:
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
            datasyncSettings.eFrameSyncType = BDSP_Raaga_Audio_DatasyncType_eSpdif;
            break;
        default:
            break;
        }
    }
    else
    {
        datasyncSettings.eEnableTargetSync = handle->startSettings.targetSyncEnabled ? BDSP_AF_P_eEnable : BDSP_AF_P_eDisable;
        datasyncSettings.eForceCompleteFirstFrame = handle->startSettings.forceCompleteFirstFrame?BDSP_AF_P_eEnable:BDSP_AF_P_eDisable;
    }

    /* Codec-Specific Settings (currently only for WMA) */
    switch ( handle->startSettings.codec )
    {
    case BAVC_AudioCompressionStd_eWmaStd:
    case BAVC_AudioCompressionStd_eWmaPro:
        datasyncSettings.uAlgoSpecConfigStruct.sWmaConfig.eWMAIpType = BDSP_Audio_WMAIpType_eASF;
        break;
    case BAVC_AudioCompressionStd_eWmaStdTs:
        datasyncSettings.uAlgoSpecConfigStruct.sWmaConfig.eWMAIpType = BDSP_Audio_WMAIpType_eTS;
        break;
    case BAVC_AudioCompressionStd_eLpcmDvd:
        datasyncSettings.uAlgoSpecConfigStruct.sLpcmConfig.eLpcmType = BDSP_Audio_LpcmAlgoType_eDvd;
        break;
    case BAVC_AudioCompressionStd_eLpcm1394:
        datasyncSettings.uAlgoSpecConfigStruct.sLpcmConfig.eLpcmType = BDSP_Audio_LpcmAlgoType_eIeee1394;
        break;
    case BAVC_AudioCompressionStd_eLpcmBd:
        #if 0   /* TODO: Not currently defined in bdsp */
        datasyncSettings.uAlgoSpecConfigStruct.sLpcmConfig.eLpcmType = BDSP_Audio_LpcmAlgoType_eBd;
        #else
        datasyncSettings.uAlgoSpecConfigStruct.sLpcmConfig.eLpcmType = BDSP_Audio_LpcmAlgoType_eMax-1;
        #endif
        break;
    case BAVC_AudioCompressionStd_eDts:
    case BAVC_AudioCompressionStd_eDtsLegacy:
    case BAVC_AudioCompressionStd_eDtshd:
        /* Set endian of compressed input data */
        datasyncSettings.uAlgoSpecConfigStruct.sDtsConfig.eDtsEndianType =
            (handle->dtsSettings.codecSettings.dts.littleEndian)?
            BDSP_Audio_DtsEndianType_eLITTLE_ENDIAN:BDSP_Audio_DtsEndianType_eBIG_ENDIAN;
        break;
    default:
        /* Value doesn't matter */
        break;
    }

    errCode = BDSP_AudioStage_SetDatasyncSettings(handle->hPrimaryStage, &datasyncSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

/* Formerly used to synchronize interrupts so they don't try to operate on a stopping task - now tasks are persistent */
static bool BAPE_Decoder_P_TaskValid_isr(BAPE_DecoderHandle handle)
{
#if 0
    switch ( handle->state )
    {
    case BAPE_DecoderState_eStarted:
    case BAPE_DecoderState_ePaused:
        return (handle->task != NULL)?true:false;
    default:
        return false;
    }
#endif
    return (handle->hTask != NULL)?true:false;
}

static void BAPE_Decoder_P_DialnormChange_isr(void *pParam1, int param2)
{
    BAPE_DecoderHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);
    BSTD_UNUSED(param2);
    BDBG_MSG(("DialogNorm Interrupt Received [decoder %u]", handle->index));
    if ( handle->interrupts.dialnormChange.pCallback_isr )
    {
        handle->interrupts.dialnormChange.pCallback_isr(handle->interrupts.dialnormChange.pParam1, handle->interrupts.dialnormChange.param2);
    }
}

static void BAPE_Decoder_P_EncoderOverflow_isr(void *pParam1, int param2)
{
    BAPE_MuxOutputHandle pMuxOutput = NULL;
    BAPE_DecoderHandle hDecoder = pParam1;

    BDBG_OBJECT_ASSERT(hDecoder, BAPE_Decoder);
    BSTD_UNUSED(param2);

    BDBG_MSG(("Encoder output overflow interrupt on decoder %u", hDecoder->index));

    for ( pMuxOutput = BLST_S_FIRST(&hDecoder->muxOutputList);
        pMuxOutput != NULL;
        pMuxOutput = BLST_S_NEXT(pMuxOutput, decoderListNode) )
    {
        BAPE_MuxOutput_P_Overflow_isr(pMuxOutput);
    }
}

static BERR_Code BAPE_Decoder_P_DeriveMultistreamLinkage(BAPE_DecoderHandle handle)
{
    BAPE_PathNode *pNodes[BAPE_CHIP_MAX_MIXERS];
    BAPE_DolbyDigitalReencodeHandle ddre=NULL;
    BAPE_MixerHandle fwMixer=NULL;
    unsigned numFound;
    bool master=true;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    /* Find DDRE and FW Mixer.  Need to determine usage modes based on these. */
    BAPE_PathNode_P_FindConsumersBySubtype(&handle->node, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDdre, 2, &numFound, pNodes);
    switch ( numFound )
    {
    case 0:
        break;
    case 1:
        ddre = pNodes[0]->pHandle;
        break;
    default:
        BDBG_ERR(("Multiple DDRE consumers found downstream from decoder %u.  This is not supported.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BAPE_PathNode_P_GetDecodersDownstreamDspMixer(&handle->node, &fwMixer);
    if ( errCode != BERR_SUCCESS )
    {
        return BERR_TRACE(errCode);
    }

    if ( fwMixer )
    {
        BAPE_PathConnector *pMaster = fwMixer->master;
        BAPE_PathNode *pNode;
        if ( NULL == pMaster )
        {
            BDBG_ERR(("A DSP mixer was found downstream from decoder %u but it does not have a master input designated.  This is not supported.", handle->index));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        pNode = pMaster->pParent;
        if ( pNode == &handle->node )
        {
            /* I am the master directly */
            master = true;
        }
        else
        {
            /* Possibly another post-processing node is between the decoder and FW mixer.  Test if the master between this node and the mixer. */
            master = BAPE_PathNode_P_NodeIsConsumer(&handle->node, pNode);
        }
    }

    /* Success.  Store results */
    handle->ddre = ddre;
    handle->fwMixer = fwMixer;
    handle->fwMixerMaster = master;

    return BERR_SUCCESS;
}

BERR_Code BAPE_Decoder_EnterUnderflowMode(
    BAPE_DecoderHandle handle
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);

    if ( handle->state == BAPE_DecoderState_eStarted &&
         handle->startSettings.nonRealTime )
    {
        errCode = BDSP_AudioTask_AudioGapFill(handle->hTask);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        return BERR_SUCCESS;
    }

    BDBG_ERR(("Decoder must be started in non-realtime mode to enter underflow mode"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static bool BAPE_Decoder_P_OrphanConnector(BAPE_DecoderHandle handle, BAPE_ConnectorFormat format)
{
    BAPE_PathConnection *pConnection;
    bool allChildrenOrphans = true;

    for ( pConnection = BLST_SQ_FIRST(&handle->node.connectors[format].connectionList);
          NULL != pConnection;
          pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
    {
        BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
        BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);
        BAPE_PathNode_P_FindOrphans(pConnection->pSink);
        allChildrenOrphans = allChildrenOrphans && pConnection->pSink->orphan;
    }

    return allChildrenOrphans;
}

void BAPE_P_CheckUnderflow_isr (void *pParam1, int param2)
{
    BAPE_DecoderHandle handle = pParam1;
    unsigned count;

    BAPE_Decoder_P_GetDataSyncStatus_isr(handle, &count);

    if (count != handle->underFlowCount)
    {
        if (handle->interrupts.cdbItbUnderflow.pCallback_isr)
        {
            BDBG_MSG(("%s Underflow Detected calling Nexus interrupt", __FUNCTION__));
            handle->interrupts.cdbItbUnderflow.pCallback_isr(handle->interrupts.cdbItbUnderflow.pParam1, handle->interrupts.cdbItbUnderflow.param2);
        }
        handle->underFlowCount = count;
    }

    if (handle->underFlowTimer)
    {
        BTMR_StartTimer_isr(handle->underFlowTimer, param2);
    }
}

/***************************************************************************
Summary:
Get Decoder Path Delay
***************************************************************************/
BERR_Code BAPE_Decoder_P_GetPathDelay_isrsafe(
    BAPE_DecoderHandle handle,
    unsigned *pDelay    /* [out] in ms */
    )
{
    BERR_Code errCode;
    BDSP_CTB_Output bdspDelay;
    BDSP_CTB_Input  ctbInput;
    BDSP_AudioTaskDatasyncSettings datasyncSettings;

    BDBG_OBJECT_ASSERT(handle, BAPE_Decoder);


    ctbInput.audioTaskDelayMode = handle->pathDelayMode;
    ctbInput.realtimeMode = handle->startSettings.nonRealTime;
    errCode = BDSP_AudioStage_GetDatasyncSettings_isr(handle->hPrimaryStage, &datasyncSettings);
    if ( errCode != BERR_SUCCESS )
    {
        ctbInput.eAudioIpSourceType = BDSP_Audio_AudioInputSource_eInvalid;
    }
    else
    {
        ctbInput.eAudioIpSourceType = datasyncSettings.eAudioIpSourceType;
    }

    errCode = BDSP_Raaga_GetAudioDelay_isrsafe(&ctbInput, handle->hPrimaryStage, &bdspDelay);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("Decoder[%d] path delay ui32Threshold %d, ui32BlockTime %d, ui32AudOffset %d", handle->index, bdspDelay.ui32Threshold, bdspDelay.ui32BlockTime, bdspDelay.ui32AudOffset));

    *pDelay = bdspDelay.ui32AudOffset;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get Decoder Path Delay
***************************************************************************/
BERR_Code BAPE_Decoder_GetPathDelay_isr(
    BAPE_DecoderHandle handle,
    unsigned *pDelay    /* [out] in ms */
    )
{
    return BAPE_Decoder_P_GetPathDelay_isrsafe(handle, pDelay);
}

/***************************************************************************
Summary:
Get DecodeToMemory settings
***************************************************************************/
void BAPE_Decoder_GetDecodeToMemorySettings(
    BAPE_DecoderHandle hDecoder,
    BAPE_DecoderDecodeToMemorySettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hDecoder, BAPE_Decoder);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = hDecoder->decodeToMem.settings;
}

static unsigned BAPE_Decoder_P_GetSamplesPerCodec(BAVC_AudioCompressionStd codec)
{
    switch ( codec )
    {
    default:
        return 2048;
    case BAVC_AudioCompressionStd_eAc3:
        return 1536;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        return 1024;
    case BAVC_AudioCompressionStd_eAmrNb:
    case BAVC_AudioCompressionStd_eAmrWb:
        return 160;
    case BAVC_AudioCompressionStd_eFlac:
        return 65536;
    case BAVC_AudioCompressionStd_ePcmWav:
    case BAVC_AudioCompressionStd_eWmaStd:
    case BAVC_AudioCompressionStd_eWmaPro:
        return 4096;
    case BAVC_AudioCompressionStd_eOpus:
        return 5760;
    case BAVC_AudioCompressionStd_eG711:
    case BAVC_AudioCompressionStd_eG726:
        return 568;
    case BAVC_AudioCompressionStd_eG729:
        return 256;
    }
}

static unsigned BAPE_Decoder_P_GetSamplesPerFrame(BAPE_DecoderHandle hDecoder)
{
    /* TODO: Use worst-case codec allocated for this decoder.  For now use flac, it's the worst. */
    BSTD_UNUSED(hDecoder);
    return BAPE_Decoder_P_GetSamplesPerCodec(BAVC_AudioCompressionStd_eFlac);
}

/***************************************************************************
Summary:
Set DecodeToMemory settings

Description:
Set the decode to host settings for a decoder.  This can only be changed
while the decoder is stopped.
***************************************************************************/
BERR_Code BAPE_Decoder_SetDecodeToMemorySettings(
    BAPE_DecoderHandle hDecoder,
    const BAPE_DecoderDecodeToMemorySettings *pSettings
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != pSettings);
    if ( pSettings->maxBuffers == 0 )
    {
        BDBG_ERR(("Maximum number of buffers must be non-zero"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( pSettings->bitsPerSample > 32 || (pSettings->bitsPerSample % 8) )
    {
        BDBG_ERR(("Bits per sample must be a multiple of 8 and <= 32 (%u requested)", pSettings->bitsPerSample));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( pSettings->maxSampleRate == 0 )
    {
        BDBG_ERR(("Maximum sample rate must be > 0"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    for ( i = 0; i < (unsigned)BAPE_Channel_eMax; i++ )
    {
        if ( pSettings->channelLayout[i] >= BAPE_Channel_eMax )
        {
            BDBG_ERR(("Channel %u is invalid, must be < BAPE_Channel_eMax", i));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    if ( pSettings->maxBuffers != hDecoder->decodeToMem.settings.maxBuffers )
    {
        unsigned queueSize;
        BDSP_QueueCreateSettings queueSettings;

        BAPE_Decoder_P_FreeDecodeToMemory(hDecoder);

        queueSize = sizeof(uint32_t) * (pSettings->maxBuffers+1);  /* +1 to allow for the DSP's full/empty algo to work */

        /* Allocate backing memory for queues */
        hDecoder->decodeToMem.pArqMem = BMEM_Heap_Alloc(hDecoder->deviceHandle->memHandle, queueSize);
        if ( NULL == hDecoder->decodeToMem.pArqMem )
        {
            BAPE_Decoder_P_FreeDecodeToMemory(hDecoder);
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }
        hDecoder->decodeToMem.pAdqMem = BMEM_Heap_Alloc(hDecoder->deviceHandle->memHandle, queueSize);
        if ( NULL == hDecoder->decodeToMem.pAdqMem )
        {
            BAPE_Decoder_P_FreeDecodeToMemory(hDecoder);
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }

        /* Allocate queues in RDB space for DSP */
        BDSP_Queue_GetDefaultSettings(hDecoder->deviceHandle->dspContext, &queueSettings);
        queueSettings.dataType = BDSP_DataType_eRDBPool;
        queueSettings.numBuffers = 1;
        queueSettings.bufferInfo[0].bufferSize = queueSize;
        BMEM_Heap_ConvertAddressToOffset(hDecoder->deviceHandle->memHandle, hDecoder->decodeToMem.pArqMem, &queueSettings.bufferInfo[0].bufferAddress);
        queueSettings.input = true;
        errCode = BDSP_Queue_Create(hDecoder->deviceHandle->dspContext, hDecoder->dspIndex, &queueSettings, &hDecoder->decodeToMem.hARQ);
        if ( errCode )
        {
            BAPE_Decoder_P_FreeDecodeToMemory(hDecoder);
            return BERR_TRACE(errCode);
        }
        BMEM_Heap_ConvertAddressToOffset(hDecoder->deviceHandle->memHandle, hDecoder->decodeToMem.pAdqMem, &queueSettings.bufferInfo[0].bufferAddress);
        queueSettings.input = false;
        errCode = BDSP_Queue_Create(hDecoder->deviceHandle->dspContext, hDecoder->dspIndex, &queueSettings, &hDecoder->decodeToMem.hADQ);
        if ( errCode )
        {
            BAPE_Decoder_P_FreeDecodeToMemory(hDecoder);
            return BERR_TRACE(errCode);
        }

        /* Allocate queue nodes */
        hDecoder->decodeToMem.pNodes = BKNI_Malloc(sizeof(BAPE_DecodeToMemoryNode)*pSettings->maxBuffers);
        if ( NULL == hDecoder->decodeToMem.pNodes )
        {
            BAPE_Decoder_P_FreeDecodeToMemory(hDecoder);
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }
        BKNI_Memset(hDecoder->decodeToMem.pNodes, 0, sizeof(BAPE_DecodeToMemoryNode)*pSettings->maxBuffers);
        /* Add all nodes to free list */
        for ( i = 0; i < pSettings->maxBuffers; i++ )
        {
            BAPE_DecodeToMemoryNode *pNode = hDecoder->decodeToMem.pNodes+i;
            void *pCachedAddr;
            pNode->pMetadataAddr = BMEM_Heap_Alloc(hDecoder->deviceHandle->memHandle, sizeof(BDSP_OnDemand_MetaDataInfo));
            if ( NULL == pNode->pMetadataAddr )
            {
                BAPE_Decoder_P_FreeDecodeToMemory(hDecoder);
                return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            }
            BMEM_Heap_ConvertAddressToCached(hDecoder->deviceHandle->memHandle, pNode->pMetadataAddr, &pCachedAddr);
            pNode->pMetadata = pCachedAddr;
            BMEM_Heap_ConvertAddressToOffset(hDecoder->deviceHandle->memHandle, pNode->pMetadataAddr, &pNode->metadataOffset);
            BLST_Q_INSERT_TAIL(&hDecoder->decodeToMem.freeList, &hDecoder->decodeToMem.pNodes[i], node);
        }
    }
    /* Initial worst-case buffer size is 2k samples per frame */
    hDecoder->decodeToMem.status.bufferSize = BAPE_Decoder_P_GetSamplesPerFrame(hDecoder) * pSettings->numPcmChannels * (pSettings->bitsPerSample/8);
    if ( hDecoder->decodeToMem.settings.maxSampleRate > 96000 )
    {
        hDecoder->decodeToMem.status.bufferSize *= 4;
    }
    else if ( hDecoder->decodeToMem.settings.maxSampleRate > 48000 )
    {
        hDecoder->decodeToMem.status.bufferSize *= 2;
    }
    hDecoder->decodeToMem.status.completedBuffers = 0;
    hDecoder->decodeToMem.status.pendingBuffers = 0;

    hDecoder->decodeToMem.settings = *pSettings;
    return BERR_SUCCESS;
}

static void BAPE_Decoder_P_UpdateQueues(BAPE_DecoderHandle hDecoder)
{
    BERR_Code errCode;
    BAPE_DecodeToMemoryNode *pNode;
    BDSP_BufferDescriptor bufferDesc;
    uint32_t *pAddr, offset;
    unsigned numPending, numCompleted;

    numPending = hDecoder->decodeToMem.status.pendingBuffers;
    numCompleted = hDecoder->decodeToMem.status.completedBuffers;

    if ( hDecoder->state != BAPE_DecoderState_eStopped )
    {
        while ( numPending > 0 )
        {
            BDSP_Queue_GetBuffer(hDecoder->decodeToMem.hADQ, &bufferDesc);
            if ( bufferDesc.bufferSize >= 4 )
            {
                errCode = BMEM_Heap_ConvertAddressToCached(hDecoder->deviceHandle->memHandle, bufferDesc.buffers[0].pBuffer, (void **)&pAddr);
                if ( errCode )
                {
                    (void)BERR_TRACE(errCode);
                    return;
                }
                (void)BMEM_Heap_FlushCache(hDecoder->deviceHandle->memHandle, pAddr, sizeof(uint32_t));
                offset = pAddr[0];

                /* Find matching entry in pending list (should be head) */
                for ( pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.pendingList);
                      NULL != pNode;
                      pNode = BLST_Q_NEXT(pNode, node) )
                {
                    if ( (uint32_t)pNode->metadataOffset == offset)
                    {
                        /* Remove from pending list */
                        BLST_Q_REMOVE(&hDecoder->decodeToMem.pendingList, pNode, node);
                        numPending--;
                        /* Translate buffer info */
                        BMEM_Heap_FlushCache(hDecoder->deviceHandle->memHandle, pNode->pMetadata, sizeof(BDSP_OnDemand_MetaDataInfo));
                        pNode->descriptor.filledBytes = pNode->pMetadata->ui32FrameValid ? pNode->pMetadata->ui32ActualBytesFilledInBuffer : 0;
                        pNode->descriptor.sampleRate = pNode->pMetadata->ui32SampleRate;
                        pNode->descriptor.ptsInfo.ui32CurrentPTS = pNode->pMetadata->ui32PTS;
                        switch ( pNode->pMetadata->ePTSType )
                        {
                        case BDSP_PtsType_eCoded:
                            pNode->descriptor.ptsInfo.ePTSType = BAVC_PTSType_eCoded;
                            break;
                        case BDSP_PtsType_eInterpolatedFromValidPTS:
                            pNode->descriptor.ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromValidPTS;
                            break;
                        default:
                            /* Fall through */
                        case BDSP_PtsType_eInterpolatedFromInvalidPTS:
                            pNode->descriptor.ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromInvalidPTS;
                            break;
                        }
                        /* Add to completed list */
                        BLST_Q_INSERT_TAIL(&hDecoder->decodeToMem.completedList, pNode, node);
                        numCompleted++;
                        break;
                    }
                }
                if ( NULL == pNode )
                {
                    BDBG_ERR(("Invalid buffer offset %#x not found in pending list.  Dropping frame.", offset));
                }

                /* Consume entry from DSP */
                errCode = BDSP_Queue_ConsumeData(hDecoder->decodeToMem.hADQ, sizeof(uint32_t));
                if ( errCode )
                {
                    (void)BERR_TRACE(errCode);
                    return;
                }
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        /* Move all pending buffers to completed if the decoder has been stopped */
        while ( NULL != (pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.pendingList)) )
        {
            BLST_Q_REMOVE_HEAD(&hDecoder->decodeToMem.pendingList, node);
            BLST_Q_INSERT_TAIL(&hDecoder->decodeToMem.completedList, pNode, node);
            pNode->descriptor.filledBytes = 0;
            pNode->descriptor.sampleRate = 0;
            pNode->descriptor.ptsInfo.ui32CurrentPTS = 0;
            pNode->descriptor.ptsInfo.ePTSType = BAVC_PTSType_eInterpolatedFromInvalidPTS;
            numPending--;
            numCompleted++;
        }
    }

    hDecoder->decodeToMem.status.pendingBuffers = numPending;
    hDecoder->decodeToMem.status.completedBuffers = numCompleted;
}

/***************************************************************************
Summary:
Get DecodeToMemory status
***************************************************************************/
BERR_Code BAPE_Decoder_GetDecodeToMemoryStatus(
    BAPE_DecoderHandle hDecoder,
    BAPE_DecoderDecodeToMemoryStatus *pStatus /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hDecoder, BAPE_Decoder);
    BDBG_ASSERT(NULL != pStatus);

    BAPE_Decoder_P_UpdateQueues(hDecoder);

    *pStatus = hDecoder->decodeToMem.status;
    return BERR_SUCCESS;
}

static void BAPE_Decoder_P_FreeDecodeToMemory(
    BAPE_DecoderHandle hDecoder
    )
{
    BAPE_DecodeToMemoryNode *pNode;

    if ( hDecoder->decodeToMem.hARQ )
    {
        BDSP_Queue_Destroy(hDecoder->decodeToMem.hARQ);
        hDecoder->decodeToMem.hARQ = NULL;
    }
    if ( hDecoder->decodeToMem.hADQ )
    {
        BDSP_Queue_Destroy(hDecoder->decodeToMem.hADQ);
        hDecoder->decodeToMem.hADQ = NULL;
    }
    if ( hDecoder->decodeToMem.pArqMem )
    {
        BMEM_Heap_Free(hDecoder->deviceHandle->memHandle, hDecoder->decodeToMem.pArqMem);
        hDecoder->decodeToMem.pArqMem = NULL;
    }
    if ( hDecoder->decodeToMem.pAdqMem )
    {
        BMEM_Heap_Free(hDecoder->deviceHandle->memHandle, hDecoder->decodeToMem.pAdqMem);
        hDecoder->decodeToMem.pAdqMem = NULL;
    }
    while ( (pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.freeList)) )
    {
        BMEM_Heap_Free(hDecoder->deviceHandle->memHandle, pNode->pMetadataAddr);
        BLST_Q_REMOVE_HEAD(&hDecoder->decodeToMem.freeList, node);
    }
    while ( (pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.pendingList)) )
    {
        BMEM_Heap_Free(hDecoder->deviceHandle->memHandle, pNode->pMetadataAddr);
        BLST_Q_REMOVE_HEAD(&hDecoder->decodeToMem.pendingList, node);
    }
    while ( (pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.completedList)) )
    {
        BMEM_Heap_Free(hDecoder->deviceHandle->memHandle, pNode->pMetadataAddr);
        BLST_Q_REMOVE_HEAD(&hDecoder->decodeToMem.completedList, node);
    }
    if ( hDecoder->decodeToMem.pNodes )
    {
        BKNI_Free(hDecoder->decodeToMem.pNodes);
        hDecoder->decodeToMem.pNodes = NULL;
    }
}

/***************************************************************************
Summary:
Initialize DecodeToMemory buffer descriptor
***************************************************************************/
void BAPE_Decoder_InitBufferDescriptor(
    BAPE_DecoderBufferDescriptor *pDescriptor /* [out] */
    )
{
    BDBG_ASSERT(NULL != pDescriptor);
    BKNI_Memset(pDescriptor, 0, sizeof(BAPE_DecoderBufferDescriptor));
}


/***************************************************************************
Summary:
Queue a buffer for DecodeToMemory operation
***************************************************************************/
BERR_Code BAPE_Decoder_QueueBuffer(
    BAPE_DecoderHandle hDecoder,
    const BAPE_DecoderBufferDescriptor *pDescriptor
    )
{
    BERR_Code errCode;
    BAPE_DecodeToMemoryNode *pNode;
    BDSP_BufferDescriptor bufferDesc;
    uint32_t *pAddr;

    BDBG_OBJECT_ASSERT(hDecoder, BAPE_Decoder);
    BDBG_ASSERT(NULL != pDescriptor);

    if ( hDecoder->state == BAPE_DecoderState_eStopped )
    {
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    if ( pDescriptor->allocatedBytes == 0 )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( pDescriptor->memoryOffset == 0 )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Get free buffer node */
    pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.freeList);
    if ( NULL == pNode )
    {
        /* BERR_TRACE omitted intentionally */
        return BERR_NOT_AVAILABLE;
    }

    /* Save buffer info */
    pNode->descriptor = *pDescriptor;
    BKNI_Memset(pNode->pMetadata, 0, sizeof(BDSP_OnDemand_MetaDataInfo));
    pNode->pMetadata->ui32FrameBufBaseAddressLow = (uint32_t)pDescriptor->memoryOffset;
    pNode->pMetadata->ui32FrameBufBaseAddressHigh = (uint32_t)(pDescriptor->memoryOffset>>32);
    pNode->pMetadata->ui32FrameBufferSizeInBytes = (uint32_t)pDescriptor->allocatedBytes;
    BMEM_Heap_FlushCache(hDecoder->deviceHandle->memHandle, pNode->pMetadata, sizeof(BDSP_OnDemand_MetaDataInfo));

    /* Get slot in DSP queue */
    errCode = BDSP_Queue_GetBuffer(hDecoder->decodeToMem.hARQ, &bufferDesc);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    BDBG_ASSERT(bufferDesc.bufferSize >= 4);
    errCode = BMEM_Heap_ConvertAddressToCached(hDecoder->deviceHandle->memHandle, bufferDesc.buffers[0].pBuffer, (void **)&pAddr);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    (void)BMEM_Heap_FlushCache(hDecoder->deviceHandle->memHandle, pAddr, sizeof(uint32_t));
    pAddr[0] = (uint32_t)pNode->metadataOffset;
    (void)BMEM_Heap_FlushCache(hDecoder->deviceHandle->memHandle, pAddr, sizeof(uint32_t));

    /* Commit entry to DSP */
    errCode = BDSP_Queue_CommitData(hDecoder->decodeToMem.hARQ, sizeof(uint32_t));
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Move node to pending list */
    BLST_Q_REMOVE_HEAD(&hDecoder->decodeToMem.freeList, node);
    BLST_Q_INSERT_TAIL(&hDecoder->decodeToMem.pendingList, pNode, node);
    hDecoder->decodeToMem.status.pendingBuffers++;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Get completed buffers

Description:
This will return the completed frames from the audio decoder.  This is non-
destructive and the app must call BAPE_Decoder_ConsumeBuffers
to remove them from the queue.
***************************************************************************/
BERR_Code BAPE_Decoder_GetBuffers(
    BAPE_DecoderHandle hDecoder,
    BAPE_DecoderBufferDescriptor *pBuffers, /* [out] */
    unsigned maxBuffers,
    unsigned *pNumBuffers/* [out] */
    )
{
    BAPE_DecodeToMemoryNode *pNode;
    unsigned i;

    BDBG_OBJECT_ASSERT(hDecoder, BAPE_Decoder);
    BDBG_ASSERT(NULL != pBuffers);
    BDBG_ASSERT(NULL != pNumBuffers);

    *pNumBuffers = 0;

    BAPE_Decoder_P_UpdateQueues(hDecoder);

    if ( maxBuffers == 0 )
    {
        return BERR_SUCCESS;
    }

    i=0;
    for ( pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.completedList);
          NULL != pNode && i < maxBuffers;
          pNode = BLST_Q_NEXT(pNode, node) )
    {
        pBuffers[i++] = pNode->descriptor;
    }

    *pNumBuffers = i;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
Consume buffers returned from BAPE_Decoder_GetBuffers

Description:
This removes one or more completed buffers from the decoder's queue and
transfers ownership to the host.
***************************************************************************/
BERR_Code BAPE_Decoder_ConsumeBuffers(
    BAPE_DecoderHandle hDecoder,
    unsigned numBuffers
    )
{
    BAPE_DecodeToMemoryNode *pNode;
    unsigned i;

    BDBG_OBJECT_ASSERT(hDecoder, BAPE_Decoder);

    if ( numBuffers == 0 )
    {
        return BERR_SUCCESS;
    }

    if ( numBuffers > hDecoder->decodeToMem.status.completedBuffers )
    {
        BDBG_ERR(("Invalid number of buffers returned %u/%u", numBuffers, hDecoder->decodeToMem.status.completedBuffers));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for ( i = 0; i < numBuffers; i++ )
    {
        pNode = BLST_Q_FIRST(&hDecoder->decodeToMem.completedList);
        BDBG_ASSERT(NULL != pNode);
        BLST_Q_REMOVE_HEAD(&hDecoder->decodeToMem.completedList, node);
        BLST_Q_INSERT_TAIL(&hDecoder->decodeToMem.freeList, pNode, node);
        hDecoder->decodeToMem.status.completedBuffers--;
    }

    return BERR_SUCCESS;
}
