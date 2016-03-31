/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 * Module Description: Mixer Interface for DSP Mixers
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bdsp_raaga.h"

BDBG_MODULE(bape_mixer_dsp);

/* Mixer Interface */
static const BAPE_MixerInterface  g_dspMixerInterface;

/* Node callbacks */
static BERR_Code BAPE_DspMixer_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_DspMixer_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_DspMixer_P_ConfigPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static BERR_Code BAPE_DspMixer_P_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_DspMixer_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection);
static void      BAPE_DspMixer_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned sampleRate);
static BERR_Code BAPE_DspMixer_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat);
static void      BAPE_DspMixer_P_InputMute(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, bool enabled);
static void      BAPE_DspMixer_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector);

/* Local Routines */
static BERR_Code BAPE_DspMixer_P_SetSettings(BAPE_MixerHandle hMixer, const BAPE_MixerSettings *pSettings);
static BERR_Code BAPE_DspMixer_P_RemoveAllInputs(BAPE_MixerHandle handle);
static BERR_Code BAPE_DspMixer_P_CreateLoopBackMixer (BAPE_MixerHandle handle);
static BERR_Code BAPE_DspMixer_P_StartTask(BAPE_MixerHandle handle);
static void BAPE_DspMixer_P_StopTask(BAPE_MixerHandle handle);
static void BAPE_DspMixer_P_FreeConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static BERR_Code BAPE_DspMixer_P_AllocateLoopbackInput(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static void BAPE_DspMixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, unsigned sampleRate);
static void BAPE_DspMixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_Connector input, unsigned inputRate, unsigned outputRate);
static BERR_Code BAPE_DspMixer_P_ApplyInputVolume(BAPE_MixerHandle mixer, unsigned index);
static void BAPE_DspMixer_P_EncoderOverflow_isr(void *pParam1, int param2);
static void BAPE_DspMixer_P_Destroy(BAPE_MixerHandle handle);
static BAPE_MultichannelFormat BAPE_DspMixer_P_GetDdreMultichannelFormat(BAPE_MixerHandle handle);
static BAPE_DolbyMSVersion BAPE_DspMixer_P_GetDolbyUsageVersion(BAPE_MixerHandle handle);
static bool BAPE_DspMixer_P_DdrePresentDownstream(BAPE_MixerHandle handle);

static const int32_t unityLoopbackCoefficients[BAPE_Channel_eMax][BAPE_Channel_eMax] =
{
    {0x800000, 0, 0, 0, 0, 0, 0, 0},
    {0, 0x800000, 0, 0, 0, 0, 0, 0},
    {0, 0, 0x800000, 0, 0, 0, 0, 0},
    {0, 0, 0, 0x800000, 0, 0, 0, 0},
    {0, 0, 0, 0, 0x800000, 0, 0, 0},
    {0, 0, 0, 0, 0, 0x800000, 0, 0},
    {0, 0, 0, 0, 0, 0, 0x800000, 0},
    {0, 0, 0, 0, 0, 0, 0, 0x800000}
};

#define BAPE_DSPMIXER_DDRE_INPUT_SR ((unsigned)48000)

/***************************************************************************
Summary:
Certification mode parameters for DAPv2 Mixer
***************************************************************************/
#define BAPE_DOLBY_DAP_CERT_MODE                        ((unsigned)1<<0)
#define BAPE_DOLBY_DAP_CERT_DISABLE_MI                  ((unsigned)1<<1)
#define BAPE_DOLBY_DAP_CERT_DISABLE_MI_SURROUNDCOMP     ((unsigned)1<<2)
#define BAPE_DOLBY_DAP_CERT_DISABLE_MI_DIALOGENHANCER   ((unsigned)1<<3)
#define BAPE_DOLBY_DAP_CERT_DISABLE_MI_VOLUMELIMITER    ((unsigned)1<<4)
#define BAPE_DOLBY_DAP_CERT_DISABLE_MI_INTELLIGENTEQ    ((unsigned)1<<5)
/* |1111 1xxx xxxx xxxx| */
#define BAPE_DOLBY_DAP_CERT_EQAMOUNT                    (0xf8000000)
#define BAPE_DOLBY_DAP_CERT_EQAMOUNT_SHIFT              (27)

BERR_Code BAPE_DspMixer_P_Create(
    BAPE_Handle deviceHandle,
    const BAPE_MixerSettings *pSettings,
    BAPE_MixerHandle *pHandle               /* [out] */
    )
{
    BERR_Code errCode;
    BAPE_MixerHandle handle;
    BAPE_MixerSettings defaultSettings;
    BDSP_TaskCreateSettings taskCreateSettings;
    BDSP_StageCreateSettings stageCreateSettings;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    if ( NULL == pSettings )
    {
        BAPE_Mixer_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if ( NULL == deviceHandle->dspContext )
    {
        BDBG_ERR(("DSP support is not available.  Can't create a DSP mixer."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( pSettings->dspIndex >= deviceHandle->numDsps )
    {
        BDBG_ERR(("DSP %u is not available.  This system has %u DSPs.", pSettings->dspIndex, deviceHandle->numDsps));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( pSettings->outputDelay )
    {
        BDBG_WRN(("Output delay is not supported for DSP mixers.  Ignoring outputDelay value."));
    }

    handle = BKNI_Malloc(sizeof(BAPE_Mixer));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_handle;
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Mixer));
    BDBG_OBJECT_SET(handle, BAPE_Mixer);
    handle->explicitFormat = BAPE_MixerFormat_eMax;
    handle->settings = *pSettings;
    handle->interface = &g_dspMixerInterface;
    handle->deviceHandle = deviceHandle;
    BLST_S_INSERT_HEAD(&deviceHandle->mixerList, handle, node);
    handle->fs = BAPE_FS_INVALID;
    BAPE_P_InitPathNode(&handle->pathNode, BAPE_PathNodeType_eMixer, handle->settings.type, 1, deviceHandle, handle);
    handle->pathNode.dspIndex = handle->settings.dspIndex;
    handle->pathNode.subtype = BAPE_MixerType_eDsp;
    handle->pathNode.pName = "DSP Mixer";

    handle->pathNode.connectors[0].useBufferPool = true;
    BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);
    format.sampleRate = 0;
    format.source = BAPE_DataSource_eDspBuffer;
    format.type = BAPE_DataType_ePcmStereo;
    errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_format; }

    BAPE_PathNode_P_GetInputCapabilities(&handle->pathNode, &caps);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDspBuffer);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eHostBuffer);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eDfifo);
    BAPE_FMT_P_EnableSource(&caps, BAPE_DataSource_eFci);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcmStereo);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm5_1);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_ePcm7_1);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937);
    BAPE_FMT_P_EnableType(&caps, BAPE_DataType_eIec61937x4);
    errCode = BAPE_PathNode_P_SetInputCapabilities(&handle->pathNode, &caps);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_caps; }

    /* Fill in node connection routines */
    handle->pathNode.allocatePathFromInput = BAPE_DspMixer_P_AllocatePathFromInput;
    handle->pathNode.freePathFromInput = BAPE_DspMixer_P_FreePathFromInput;
    handle->pathNode.configPathFromInput = BAPE_DspMixer_P_ConfigPathFromInput;
    handle->pathNode.startPathFromInput = BAPE_DspMixer_P_StartPathFromInput;
    handle->pathNode.stopPathFromInput = BAPE_DspMixer_P_StopPathFromInput;

    /* Generic Routines for DSP nodes */
    handle->pathNode.allocatePathToOutput = BAPE_DSP_P_AllocatePathToOutput;
    handle->pathNode.configPathToOutput = BAPE_DSP_P_ConfigPathToOutput;
    handle->pathNode.stopPathToOutput = BAPE_DSP_P_StopPathToOutput;
    handle->pathNode.startPathToOutput = BAPE_DSP_P_StartPathToOutput;

    /* Other misc. node routines for mixers */
    handle->pathNode.inputSampleRateChange_isr = BAPE_DspMixer_P_InputSampleRateChange_isr;
    handle->pathNode.inputFormatChange = BAPE_DspMixer_P_InputFormatChange;
    handle->pathNode.inputMute = BAPE_DspMixer_P_InputMute;
    handle->pathNode.removeInput = BAPE_DspMixer_P_RemoveInputCallback;

    BDSP_Task_GetDefaultCreateSettings(deviceHandle->dspContext, &taskCreateSettings);
    taskCreateSettings.masterTask = true;
    taskCreateSettings.numSrc = 3;
    taskCreateSettings.dspIndex = pSettings->dspIndex;

    errCode = BDSP_Task_Create(deviceHandle->dspContext, &taskCreateSettings, &handle->hTask);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_task_create;
    }

    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioMixer, &stageCreateSettings);
    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hMixerStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_mixer_stage;
    }

    errCode = BDSP_Stage_SetAlgorithm(handle->hMixerStage, BAPE_P_GetCodecMixer());
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_mixer_stage;
    }

    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eSrc] = true;
    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hSrcStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_src_stage;
    }

    *pHandle = handle;

    return BERR_SUCCESS;

err_src_stage:
err_mixer_stage:
err_task_create:
err_caps:
err_format:
    BAPE_DspMixer_P_Destroy(handle);
err_handle:
    return errCode;
}

static void BAPE_DspMixer_P_Destroy(
    BAPE_MixerHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Make sure no inputs are running. */
    if ( handle->running )
    {
        BDBG_ERR(("Mixer %p (%d) is running.  Cannot close.", (void *)handle, handle->index));
        BDBG_ASSERT(false == handle->running);
        return;
    }

    /* Remove all inputs */
    BAPE_DspMixer_P_RemoveAllInputs(handle);

    /* Destroy stages and task */
    if ( handle->hMixerStage )
    {
        BDSP_Stage_Destroy(handle->hMixerStage);
    }
    if ( handle->hSrcStage )
    {
        BDSP_Stage_Destroy(handle->hSrcStage);
    }
    if ( handle->hTask )
    {
        BDSP_Task_Destroy(handle->hTask);
    }

    /* Unlink from device */
    BLST_S_REMOVE(&handle->deviceHandle->mixerList, handle, BAPE_Mixer, node);

    /* Destroy Handle */
    BDBG_OBJECT_DESTROY(handle, BAPE_Mixer);
    BKNI_Free(handle);
}

static BERR_Code BAPE_DspMixer_P_AddInput(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerAddInputSettings *pSettings     /* Optional, pass NULL for default settings */
    )
{
    unsigned i;
    BERR_Code errCode;
    BAPE_MixerAddInputSettings defaultSettings;
    bool makeMaster;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    if ( NULL == pSettings )
    {
        pSettings = &defaultSettings;
        BAPE_Mixer_GetDefaultAddInputSettings(&defaultSettings);
    }

    makeMaster = pSettings->sampleRateMaster;

    if ( handle->running && !handle->startedExplicitly )
    {
        BDBG_ERR(("Cannot change inputs while a mixer is active."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (handle->running && handle->startedExplicitly &&
        input->format.source != BAPE_DataSource_eDspBuffer &&
        handle->loopbackMixerGroup == NULL && handle->loopbackGroup == NULL)
    {
        errCode = BAPE_DspMixer_P_CreateLoopBackMixer(handle);
        if (errCode)
        {
            BDBG_ERR(("Unable to create loopback mixer"));
            return BERR_TRACE(errCode);
        }
    }

#if BDBG_DEBUG_BUILD
    /* Make sure the same input isn't added multiple times */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == input )
        {
            BDBG_ERR(("Cannot add the same input multiple times to a single mixer."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
#endif

    if ( makeMaster && input->format.source != BAPE_DataSource_eDspBuffer )
    {
        BDBG_ERR(("Only DSP inputs can be treated as master for DSP mixers"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Find empty slot */
    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] == NULL )
        {
            handle->inputs[i] = input;
            BDBG_MSG(("Adding input %s %s to DSP mixer %u as %s", input->pParent->pName, input->pName, handle->index, makeMaster?"master":"slave"));
            if ( makeMaster )
            {
                if ( handle->master )
                {
                    BDBG_MSG(("Replacing current master input with new input"));
                }
                handle->master = input;
            }
            BAPE_Mixer_P_GetDefaultInputVolume(&handle->inputVolume[i]);
            errCode = BAPE_PathNode_P_AddInput(&handle->pathNode, input);
            if ( errCode )
            {
                handle->inputs[i] = NULL;
                return BERR_TRACE(errCode);
            }
            return BERR_SUCCESS;
        }
    }

    /* Mixer has no available slots. */
    BDBG_ERR(("Mixer can not accept any more inputs.  Maximum inputs per DSP mixer is %u.", BAPE_CHIP_MAX_MIXER_INPUTS));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static BERR_Code BAPE_DspMixer_P_RemoveInput(
    BAPE_MixerHandle handle,
    BAPE_Connector input
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    if ( handle->running && !handle->startedExplicitly )
    {
        BDBG_ERR(("Cannot change inputs while a mixer is active."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Find slot containing this input */
    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i < BAPE_CHIP_MAX_MIXER_INPUTS )
    {
        errCode = BAPE_PathNode_P_RemoveInput(&handle->pathNode, input);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        handle->inputs[i] = NULL;
        if ( handle->master == input )
        {
            BDBG_MSG(("Removing master input %p", (void *)input));
            handle->master = NULL;
        }
        return BERR_SUCCESS;
    }

    /* Input not found. */
    BDBG_ERR(("Input %p is not connected to DSP mixer %p", (void *)input, (void *)handle));
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static BERR_Code BAPE_DspMixer_P_RemoveAllInputs(
    BAPE_MixerHandle handle
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
    {
        if ( handle->inputs[i] )
        {
            errCode = BAPE_DspMixer_P_RemoveInput(handle, handle->inputs[i]);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_DspMixer_P_Start(BAPE_MixerHandle handle)
{
    BERR_Code errCode;
    BAPE_PathNode *pNode;
    unsigned numFound;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( handle->startedExplicitly )
    {
        BDBG_ERR(("Mixer %p (%d) has already been explicity started.  Can't start.", (void *)handle, handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Look for MuxOutput */
    BAPE_PathNode_P_FindConsumersByType(&handle->pathNode, BAPE_PathNodeType_eMuxOutput, 1, &numFound, &pNode);
    if ( numFound > 0 )
    {
        BAPE_MuxOutputHandle hMuxOutput = (BAPE_MuxOutputHandle)pNode->pHandle;
        if ( !BAPE_MuxOutput_P_IsRunning(hMuxOutput) )
        {
            BDBG_ERR(("When explicitly starting a mixer for transcode operations, the MuxOutput object must be started first."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    handle->startedExplicitly = true;

    if ( false == handle->taskStarted )
    {
        BDBG_MSG(("BAPE_DspMixer_P_Start: Task not present.  Starting mixer task."));
        errCode = BAPE_DspMixer_P_StartTask(handle);
        if ( errCode )
        {
            handle->startedExplicitly = false;
            return BERR_TRACE(errCode);
        }
    }

    handle->running++;

    return BERR_SUCCESS;
}

static BERR_Code BAPE_DspMixer_P_CreateLoopBackMixer (BAPE_MixerHandle handle)
{

    BAPE_MixerGroupCreateSettings mixerCreateSettings;
    BAPE_LoopbackGroupCreateSettings loopbackCreateSettings;
    BAPE_LoopbackGroupSettings loopbackSettings;
    BAPE_DfifoGroupCreateSettings dfifoCreateSettings;
    BAPE_DfifoGroupSettings dfifoSettings;
    BAPE_FMT_Descriptor format;
    BERR_Code errCode;

    BDBG_ASSERT(NULL == handle->loopbackMixerGroup);
    BDBG_ASSERT(NULL == handle->loopbackGroup);
    BDBG_ASSERT(NULL == handle->loopbackDfifoGroup);
    BDBG_ASSERT(NULL == handle->pLoopbackBuffers[0]);

    BAPE_MixerGroup_P_GetDefaultCreateSettings(&mixerCreateSettings);
    mixerCreateSettings.numChannelPairs = 1;
    errCode = BAPE_MixerGroup_P_Create(handle->deviceHandle, &mixerCreateSettings, &handle->loopbackMixerGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode;
    }

    BAPE_LoopbackGroup_P_GetDefaultCreateSettings(&loopbackCreateSettings);
    loopbackCreateSettings.numChannelPairs = 1;
    errCode = BAPE_LoopbackGroup_P_Create(handle->deviceHandle, &loopbackCreateSettings, &handle->loopbackGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode;
    }

    BAPE_FMT_P_InitDescriptor(&format);
    format.source = BAPE_DataSource_eDfifo;
    format.type = BAPE_DataType_ePcmStereo;
    format.sampleRate = 48000;
    errCode = BAPE_P_AllocateBuffers(handle->deviceHandle, &format, handle->pLoopbackBuffers);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode;
    }

    BAPE_DfifoGroup_P_GetDefaultCreateSettings(&dfifoCreateSettings);
    dfifoCreateSettings.numChannelPairs = 1;
    errCode = BAPE_DfifoGroup_P_Create(handle->deviceHandle, &dfifoCreateSettings, &handle->loopbackDfifoGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode;
    }

    BAPE_DfifoGroup_P_GetSettings(handle->loopbackDfifoGroup, &dfifoSettings);
    dfifoSettings.interleaveData = false;
    dfifoSettings.dataWidth = 32;
    BAPE_LoopbackGroup_P_GetCaptureFciIds(handle->loopbackGroup, &dfifoSettings.input);
    dfifoSettings.bufferInfo[0].base = handle->pLoopbackBuffers[0]->offset;
    dfifoSettings.bufferInfo[0].length = handle->pLoopbackBuffers[0]->bufferSize/2;
    dfifoSettings.bufferInfo[1].base = handle->pLoopbackBuffers[0]->offset + dfifoSettings.bufferInfo[0].length;
    dfifoSettings.bufferInfo[1].length = dfifoSettings.bufferInfo[0].length;
    errCode = BAPE_DfifoGroup_P_SetSettings(handle->loopbackDfifoGroup, &dfifoSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode;
    }

    /* Loopback found, allocate an FS timing object */
#if BAPE_CHIP_MAX_FS > 0
    BDBG_ASSERT(handle->fs == BAPE_FS_INVALID);
    handle->fs = BAPE_P_AllocateFs(handle->deviceHandle);
    if ( handle->fs == BAPE_FS_INVALID )
    {
        BDBG_ERR(("Unable to allocate FS resources for input loopback."));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return errCode;
    }
#endif
    /* Use PLL for now, other timing sources are not available here. */
    /* TODO: Allow for selection of NCO timing source... maybe even call
     * BAPE_Mixer_P_GetMclkSource(), after moving it from "standard"
     * mixer code to common mixer code. */
    handle->mclkSource = BAPE_MclkSource_ePll0 + handle->settings.outputPll;

    /* Connect to the appropriate PLL */
    #if BAPE_CHIP_MAX_PLLS > 0
    BAPE_P_AttachMixerToPll(handle, handle->settings.outputPll);
    #else
    errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    return errCode;
    #endif

    BKNI_EnterCriticalSection();
    BAPE_LoopbackGroup_P_GetSettings_isr(handle->loopbackGroup, &loopbackSettings);
    BKNI_LeaveCriticalSection();
#if BAPE_CHIP_MAX_FS > 0
    loopbackSettings.fs = handle->fs;
#else
    loopbackSettings.mclkSource = handle->mclkSource;
    loopbackSettings.pllChannel = 0;
    loopbackSettings.mclkFreqToFsRatio = BAPE_BASE_PLL_TO_FS_RATIO;
#endif
    BAPE_MixerGroup_P_GetOutputFciIds(handle->loopbackMixerGroup, 0, &loopbackSettings.input);
    BKNI_EnterCriticalSection();
    errCode = BAPE_LoopbackGroup_P_SetSettings_isr(handle->loopbackGroup, &loopbackSettings);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode;
    }
    return errCode;
}

static BERR_Code BAPE_DspMixer_P_StartTask(BAPE_MixerHandle handle)
{
    BERR_Code errCode;
    bool fmmInputs=false;
    unsigned input, output;
    BDSP_TaskStartSettings taskStartSettings;
    BAPE_PathConnection *pInputConnection;
    BAPE_PathNode *pNode;
    unsigned numFound, i;

    BDBG_ASSERT(false == handle->taskStarted);

    /* Apply latest and greatest settings */
    BAPE_DspMixer_P_SetSettings(handle, &handle->settings);

    /* Scan inputs and determine if we need a FS and PLL linkage for loopback */
    for ( pInputConnection = BLST_S_FIRST(&handle->pathNode.upstreamList);
          pInputConnection != NULL;
          pInputConnection = BLST_S_NEXT(pInputConnection, upstreamNode) )
    {
        if ( pInputConnection->pSource->format.source != BAPE_DataSource_eDspBuffer )
        {
            fmmInputs = true;
            break;
        }
    }

    if ( fmmInputs )
    {
        errCode = BAPE_DspMixer_P_CreateLoopBackMixer(handle);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_acquire_resources;
        }
    }
    else
    {
        handle->mclkSource = BAPE_MclkSource_eNone;
    }

    /* Setup Stages and Task */
    BDSP_Task_GetDefaultStartSettings(handle->hTask, &taskStartSettings);
    taskStartSettings.primaryStage = handle->hMixerStage;
    taskStartSettings.openGateAtStart = handle->startedExplicitly;
    taskStartSettings.timeBaseType = BDSP_AF_P_TimeBaseType_e45Khz;
    taskStartSettings.schedulingMode = BDSP_TaskSchedulingMode_eMaster;

    /* Add sample rate converter if required */
    handle->pathNode.connectors[0].hStage = handle->hMixerStage;

    if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->pathNode.connectors[0].format) )
    {
        uint32_t sampleRate = 0;

        /* IF DDRE is present downstream, output samplerate must be fixed to 48000 */
        if ( BAPE_DspMixer_P_DdrePresentDownstream(handle) )
        {
            BAPE_FMT_Descriptor format;
            /* DDRE cases do not require an SRC stage, so just set the samplerate and go */
            sampleRate = BAPE_DSPMIXER_DDRE_INPUT_SR;

            BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);
            if ( BAPE_DspMixer_P_DdrePresentDownstream(handle) )
            {
                format.type = BAPE_DataType_ePcm5_1;
                if ( BAPE_DspMixer_P_GetDdreMultichannelFormat(handle) == BAPE_MultichannelFormat_e7_1 )
                {
                    format.type = BAPE_DataType_ePcm7_1;
                }
            }
            errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        else if ( handle->settings.mixerSampleRate )
        {
            sampleRate = handle->settings.mixerSampleRate;

            /* Add SRC stage after mixer if required */
            errCode = BDSP_Stage_AddOutputStage(handle->hMixerStage, BAPE_DSP_P_GetDataTypeFromConnector(&handle->pathNode.connectors[0]), handle->hSrcStage, &output, &input);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_stage;
            }
            handle->pathNode.connectors[0].hStage = handle->hSrcStage;
        }

        if ( sampleRate != 0 )
        {
            /* Program sample rate map table so all output rates match the requested rate */
            for ( i = 0; i < BDSP_AF_P_SampFreq_eMax; i++ )
            {
                handle->sampleRateMap.ui32OpSamplingFrequency[i] = sampleRate;
            }

            taskStartSettings.pSampleRateMap = &handle->sampleRateMap;  /* If not set, this will be set to the default rate table in bdsp */
        }
    }
    else if ( handle->settings.mixerSampleRate )
    {
        BDBG_WRN(("Can not set fixed output sample rate for compressed sources.  Ignoring fixed sample rate of %u", handle->settings.mixerSampleRate));
    }

    /* Prepare subnodes */
    errCode = BAPE_PathNode_P_AcquirePathResources(&handle->pathNode);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_acquire_resources;
    }

    BKNI_EnterCriticalSection();
    BAPE_Connector_P_SetSampleRate_isr(&handle->pathNode.connectors[0], (handle->master) ? (handle->master->format.sampleRate) : 0);
    BKNI_LeaveCriticalSection();

    errCode = BAPE_DSP_P_DeriveTaskStartSettings(&handle->pathNode, &taskStartSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_consumers;
    }

    /* Validate we either have real-time outputs or don't depending on system requirements */
    BAPE_PathNode_P_FindConsumersBySubtype(&handle->pathNode, BAPE_PathNodeType_eMixer, BAPE_MixerType_eStandard, 1, &numFound, &pNode);
    if ( taskStartSettings.realtimeMode == BDSP_TaskRealtimeMode_eNonRealTime )
    {
        /* If we did not find an output and there is no DDRE downstream */
        if ( numFound != 0 && !BAPE_DspMixer_P_DdrePresentDownstream(handle))
        {
            BDBG_ERR(("No outputs should be connected to a DSP mixer in non-realtime mode."));
            errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_consumers;
        }
    }
    else
    {
        if ( numFound == 0 )
        {
            BDBG_ERR(("No outputs are connected.  Cannot start."));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto err_consumers;
        }
    }

    /* Don't set openGateAtStart in NRT or for compressed input */
    if ( !BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->pathNode.connectors[0].format) || taskStartSettings.realtimeMode != BDSP_TaskRealtimeMode_eRealTime )
    {
        taskStartSettings.openGateAtStart = false;
    }

    taskStartSettings.maxIndependentDelay = handle->deviceHandle->settings.maxIndependentDelay;

    /* Link the path resources */
    errCode = BAPE_PathNode_P_ConfigurePathResources(&handle->pathNode);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_config_path;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintMixers(handle->deviceHandle);
    #endif

    /* Start the consumers */
    errCode = BAPE_PathNode_P_StartPaths(&handle->pathNode);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_start_path;
    }

    /* Setup Encoder overflow interrupt */
    BAPE_PathNode_P_FindConsumersByType(&handle->pathNode, BAPE_PathNodeType_eMuxOutput, 1, &numFound, &pNode);
    if ( numFound > 0 )
    {
        BDSP_AudioInterruptHandlers interrupts;
        handle->hMuxOutput = (BAPE_MuxOutputHandle)pNode->pHandle;
        BKNI_EnterCriticalSection();
        BDSP_AudioTask_GetInterruptHandlers_isr(handle->hTask, &interrupts);
        interrupts.encoderOutputOverflow.pCallback_isr = BAPE_DspMixer_P_EncoderOverflow_isr;
        interrupts.encoderOutputOverflow.pParam1 = handle;
        BDSP_AudioTask_SetInterruptHandlers_isr(handle->hTask, &interrupts);
        BKNI_LeaveCriticalSection();
    }
    else
    {
        handle->hMuxOutput = NULL;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintDownstreamNodes(&handle->pathNode);
    #endif

    /* Start the DSP Task */
    errCode = BDSP_Task_Start(handle->hTask, &taskStartSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_start_task;
    }

    handle->taskStarted = true;
    return BERR_SUCCESS;

err_start_task:
err_start_path:
err_config_path:
err_consumers:
err_stage:
err_acquire_resources:
    BAPE_PathNode_P_ReleasePathResources(&handle->pathNode);
    if ( handle->fs != BAPE_FS_INVALID )
    {
        BAPE_P_FreeFs(handle->deviceHandle, handle->fs);
        handle->fs = BAPE_FS_INVALID;
    }
    return errCode;
}

bool BAPE_DspMixer_P_DdrePresentDownstream(BAPE_MixerHandle handle)
{
    bool foundDdre = false;
    BAPE_PathNode *pNode;
    unsigned numFound;

    BAPE_PathNode_P_FindConsumersBySubtype(&handle->pathNode, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDdre, 1, &numFound, &pNode);
    switch ( numFound )
    {
    case 0:
        break;
    case 1:
        foundDdre = true;
        break;
    default:
        BDBG_ERR(("Multiple DDRE consumers found downstream from mixer %u.  This is not supported.", handle->index));
        return false;
    }

    return foundDdre;
}

static BAPE_MultichannelFormat BAPE_DspMixer_P_GetDdreMultichannelFormat(BAPE_MixerHandle handle)
{
    BAPE_PathNode *pNode;
    unsigned numFound;

    BAPE_PathNode_P_FindConsumersBySubtype(&handle->pathNode, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDdre, 1, &numFound, &pNode);
    switch ( numFound )
    {
    case 1:
        return BAPE_DolbyDigitalReencode_P_GetMultichannelFormat((BAPE_DolbyDigitalReencodeHandle)pNode->pHandle);
        break;
    default:
        BDBG_ERR(("No DDRE found downstream"));
        break;
    }

    return BAPE_MultichannelFormat_eMax;
}

static BAPE_DolbyMSVersion BAPE_DspMixer_P_GetDolbyUsageVersion(BAPE_MixerHandle handle)
{
    if ( BAPE_P_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        /* IF we find a DDRE downstream, use MS12 mixer,
           else use legacy mixer (transcode, etc) */
        if ( BAPE_DspMixer_P_DdrePresentDownstream(handle) )
        {
            return BAPE_DolbyMSVersion_eMS12;
        }
        else
        {
            return BAPE_DolbyMSVersion_eMS11;
        }
    }

    return BAPE_P_GetDolbyMSVersion();
}

static BERR_Code BAPE_DspMixer_P_AllocatePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_MixerHandle handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = (BAPE_MixerHandle)pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Sanity check if the dsp indexes match */
    if ( pConnection->pSource->format.source == BAPE_DataSource_eDspBuffer )
    {
        if ( pConnection->pSource->pParent->dspIndex != BAPE_DSP_ID_INVALID &&
             handle->settings.dspIndex != pConnection->pSource->pParent->dspIndex )
        {
            BDBG_ERR(("All inputs to a DSP mixer must run on the same DSP as the DSP mixer."));
            BDBG_ERR(("This mixer is configured for DSP %u but the input %s is configured for DSP %u",
                      handle->settings.dspIndex, pConnection->pSource->pParent->pName,
                      pConnection->pSource->pParent->dspIndex));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    /* First see if we need to create the mixer task itself */
    if ( false == handle->taskStarted )
    {
        BDBG_MSG(("AllocatePathFromInput: Task not present.  Starting mixer task."));
        errCode = BAPE_DspMixer_P_StartTask(handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    /* Task Handle is valid if we reach here. */

    switch ( pConnection->pSource->format.source )
    {
    case BAPE_DataSource_eDspBuffer:
        /* check if we need to recreate intertask buffer */
        if ( pConnection->hInterTaskBuffer != NULL &&
            (pConnection->pSource->format.type != pConnection->format.type) )
        {
            BDSP_InterTaskBuffer_Destroy(pConnection->hInterTaskBuffer);
            pConnection->hInterTaskBuffer = NULL;
        }

        if ( pConnection->hInterTaskBuffer == NULL )
        {
            errCode = BDSP_InterTaskBuffer_Create(handle->deviceHandle->dspContext,
                                                BAPE_FMT_P_GetDspDataType_isrsafe(&pConnection->pSource->format),
                                                BDSP_BufferType_eDRAM,
                                                &pConnection->hInterTaskBuffer);
            if ( errCode )
            {
                BAPE_DspMixer_P_FreeConnectionResources(handle, pConnection);
                return BERR_TRACE(errCode);
            }
        }
        pConnection->dspInputIndex = BAPE_DSPMIXER_INPUT_INVALID;
        break;
    case BAPE_DataSource_eHostBuffer:
    case BAPE_DataSource_eDfifo:
    case BAPE_DataSource_eFci:
        errCode = BAPE_DspMixer_P_AllocateLoopbackInput(handle, pConnection);
        if ( errCode )
        {
            BAPE_DspMixer_P_FreeConnectionResources(handle, pConnection);
            return BERR_TRACE(errCode);
        }
        break;
    default:
        BDBG_ERR(("Unsupported input type"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;
}

static void BAPE_DspMixer_P_FreePathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_MixerHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = (BAPE_MixerHandle)pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* Stop the mixer task if nobody is running. */
    if ( 0 == handle->running && handle->taskStarted )
    {
        BDBG_MSG(("FreePathFromInput: No running inputs - stop mixer task"));
        BAPE_DspMixer_P_StopTask(handle);
    }
    BAPE_DspMixer_P_FreeConnectionResources(handle, pConnection);
}

static BERR_Code BAPE_DspMixer_P_ConfigPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_MixerHandle handle;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = (BAPE_MixerHandle)pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    return BERR_SUCCESS;
}

static BERR_Code BAPE_DspMixer_P_StartPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BAPE_MixerHandle handle;
    unsigned inputIndex;
    BAPE_MixerGroupInputSettings mixerInputSettings;
    BDSP_FmmBufferDescriptor fmmInputDesc;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = (BAPE_MixerHandle)pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    BDBG_MSG(("Input %s %s starting", pConnection->pSource->pParent->pName, pConnection->pSource->pName));

    /* First, setup the sample rate since it may affect output programming. */
    BKNI_EnterCriticalSection();
    BAPE_DspMixer_P_InputSampleRateChange_isr(pNode, pConnection, pConnection->pSource->format.sampleRate);
    BKNI_LeaveCriticalSection();

    inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, pConnection->pSource);

    /* See if this is an FMM input or DSP input */
    if ( pConnection->pSource->format.source == BAPE_DataSource_eDspBuffer )
    {
        /* Inter-Task Input */
        BDSP_InterTaskBuffer_Flush(pConnection->hInterTaskBuffer);
        errCode = BDSP_Stage_AddInterTaskBufferInput(handle->hMixerStage, BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource), pConnection->hInterTaskBuffer, &pConnection->dspInputIndex);
        if ( errCode )
        {
            pConnection->dspInputIndex = BAPE_DSPMIXER_INPUT_INVALID;
            return BERR_TRACE(errCode);
        }
        /* Refresh input scaling now that the DSP input is valid to the mixer */
        errCode = BAPE_DspMixer_P_ApplyInputVolume(handle, inputIndex);
        if ( errCode )
        {
            BDSP_Stage_RemoveInput(handle->hMixerStage, pConnection->dspInputIndex);
            pConnection->dspInputIndex = BAPE_DSPMIXER_INPUT_INVALID;
            return BERR_TRACE(errCode);
        }

        handle->running++;
        return BERR_SUCCESS;
    }

    /* If we get here it's an FMM input */
    if ( 0 == handle->loopbackRunning )
    {
        errCode = BAPE_DfifoGroup_P_Start(handle->loopbackDfifoGroup, false);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_dfifo_start;
        }
        errCode = BAPE_LoopbackGroup_P_Start(handle->loopbackGroup);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_loopback_start;
        }
        errCode = BAPE_MixerGroup_P_StartOutput(handle->loopbackMixerGroup, 0);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_mixer_output;
        }
        /* Refresh input scaling before adding to the DSP */
        errCode = BAPE_DspMixer_P_ApplyInputVolume(handle, inputIndex);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_input_scaling;
        }
        /* FMM Input */
        errCode = BAPE_DSP_P_InitFmmInputDescriptor(handle->loopbackDfifoGroup, &fmmInputDesc);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_add_input;
        }
        errCode = BDSP_Stage_AddFmmInput(handle->hMixerStage, BAPE_DSP_P_GetDataTypeFromConnector(pConnection->pSource), &fmmInputDesc, &handle->loopbackDspInput);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_add_input;
        }
    }
    handle->loopbackRunning++;
    BAPE_MixerGroup_P_GetInputSettings(handle->loopbackMixerGroup, inputIndex, &mixerInputSettings);
    BAPE_SrcGroup_P_GetOutputFciIds(pConnection->srcGroup, &mixerInputSettings.input);
    errCode = BAPE_MixerGroup_P_SetInputSettings(handle->loopbackMixerGroup, inputIndex, &mixerInputSettings);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_mixer_input;
    }
    errCode = BAPE_MixerGroup_P_StartInput(handle->loopbackMixerGroup, inputIndex);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_mixer_input;
    }
    errCode = BAPE_SrcGroup_P_Start(pConnection->srcGroup);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_src_start;
    }

    handle->running++;
    return BERR_SUCCESS;

err_src_start:
    BAPE_MixerGroup_P_StopInput(handle->loopbackMixerGroup, inputIndex);
err_mixer_input:
    handle->loopbackRunning--;
    if ( 0 == handle->loopbackRunning )
    {
        BDSP_Stage_RemoveInput(handle->hMixerStage, handle->loopbackDspInput);
        handle->loopbackDspInput = BAPE_DSPMIXER_INPUT_INVALID;
err_add_input:
err_input_scaling:
        BAPE_MixerGroup_P_StopOutput(handle->loopbackMixerGroup, 0);
err_mixer_output:
        BAPE_LoopbackGroup_P_Stop(handle->loopbackGroup);
err_loopback_start:
        BAPE_DfifoGroup_P_Stop(handle->loopbackDfifoGroup);
    }
err_dfifo_start:
    return errCode;
}

static void BAPE_DspMixer_P_StopPathFromInput(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection)
{
    BAPE_MixerHandle handle;
    unsigned inputIndex;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = (BAPE_MixerHandle)pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    inputIndex = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, pConnection->pSource);

    if ( pConnection->pSource->format.source == BAPE_DataSource_eDspBuffer )
    {
        /* DSP input */
        if ( pConnection->dspInputIndex == BAPE_DSPMIXER_INPUT_INVALID )
        {
            BDBG_MSG(("Input already stopped"));
            return;
        }

        BDBG_ASSERT(handle->running > 0);

        BDBG_MSG(("Removing input from FW Mixer for %s %s (input index %u)", pConnection->pSource->pParent->pName, pConnection->pSource->pName, pConnection->dspInputIndex));
        BDSP_Stage_RemoveInput(handle->hMixerStage, pConnection->dspInputIndex);
        pConnection->dspInputIndex = BAPE_DSPMIXER_INPUT_INVALID;
    }
    else
    {
        BDBG_ASSERT(handle->loopbackRunning > 0);
        BDBG_ASSERT(handle->running > 0);

        handle->loopbackRunning--;

        if ( pConnection->srcGroup )
        {
            BAPE_SrcGroup_P_Stop(pConnection->srcGroup);
        }
        BAPE_MixerGroup_P_StopInput(handle->loopbackMixerGroup, inputIndex);

        if ( handle->loopbackRunning == 0 )
        {
            BDSP_Stage_RemoveInput(handle->hMixerStage, handle->loopbackDspInput);
            handle->loopbackDspInput = BAPE_DSPMIXER_INPUT_INVALID;
            BAPE_MixerGroup_P_StopOutput(handle->loopbackMixerGroup, 0);
            BAPE_LoopbackGroup_P_Stop(handle->loopbackGroup);
            BAPE_DfifoGroup_P_Stop(handle->loopbackDfifoGroup);
        }
    }

    handle->running--;
    /* Stop the mixer task if nobody is running. */
    if ( 0 == handle->running && handle->taskStarted )
    {
        BDBG_MSG(("FreePathFromInput: Last running input has stopped - stop mixer task"));
        BAPE_DspMixer_P_StopTask(handle);
    }
}

/* TODO: Share with other mixers */
static void BAPE_DspMixer_P_InputSampleRateChange_isr(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, unsigned newSampleRate)
{
    BAPE_MixerHandle handle;
    unsigned sampleRate;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BKNI_ASSERT_ISR_CONTEXT();

    handle = pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    sampleRate = newSampleRate;

    BDBG_MSG(("Input %s %s sample rate changed from %u to %u", pConnection->pSource->pParent->pName, pConnection->pSource->pName, pConnection->format.sampleRate, sampleRate));

    /* Mark new sample rate in connection */
    pConnection->format.sampleRate = sampleRate;

    /* Handle fixed rate output */
    if ( handle->settings.mixerSampleRate && BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->pathNode.connectors[0].format) )
    {
        /* Set mixer sample rate. */
        BAPE_DspMixer_P_SetSampleRate_isr(handle, handle->settings.mixerSampleRate);
    }
    else if ( BAPE_DspMixer_P_DdrePresentDownstream(handle) && BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->pathNode.connectors[0].format) )
    {
        /* Set mixer sample rate. */
        BAPE_DspMixer_P_SetSampleRate_isr(handle, BAPE_DSPMIXER_DDRE_INPUT_SR);
    }
    /* Handle the sample rate master or fixed rate output */
    else if ( handle->master == pConnection->pSource )
    {
        if ( sampleRate != 0 )
        {
            /* Set mixer sample rate. */
            BAPE_DspMixer_P_SetSampleRate_isr(handle, sampleRate);
        }
        else if ( BAPE_Mixer_P_GetOutputSampleRate_isr(handle) == 0 )
        {
            /* Make sure there is a valid sample rate */
            BAPE_DspMixer_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
        }
    }
    else
    {
        /* Make sure there is a valid sample rate */
        if ( BAPE_Mixer_P_GetOutputSampleRate_isr(handle) == 0 )
        {
            BAPE_DspMixer_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
        }
    }

    /* Update SRCs accordingly. */
    BAPE_DspMixer_P_SetInputSRC_isr(handle, pConnection->pSource, sampleRate, BAPE_Mixer_P_GetOutputSampleRate_isr(handle));
}

static BERR_Code BAPE_DspMixer_P_InputFormatChange(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, const BAPE_FMT_Descriptor *pNewFormat)
{
    BERR_Code errCode;
    BAPE_MixerHandle handle;
    BAPE_DataType outputDataType;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    handle = (BAPE_MixerHandle)pNode->pHandle;
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    /* See if input format has changed.  If not, do nothing. */
    if ( pNewFormat->type != pConnection->format.type )
    {
        BDBG_MSG(("Input connection format change reported, but does not affect resource requirements."));
    }
    else
    {
        BDBG_MSG(("Input connection format change reported - freeing connection resources."));
        BAPE_DspMixer_P_FreeConnectionResources(handle, pConnection);
    }

    /* Determine output format based on widest input format */
    errCode = BAPE_Mixer_P_DetermineOutputDataType(handle, &outputDataType);
    if ( errCode )
    {
        BDBG_ERR(("Unable to determine mixer ouptut data format."));
        return BERR_TRACE(errCode);
    }

    if ( BAPE_DspMixer_P_DdrePresentDownstream(handle) )
    {
        outputDataType = BAPE_DataType_ePcm5_1;
        if ( BAPE_DspMixer_P_GetDdreMultichannelFormat(handle) == BAPE_MultichannelFormat_e7_1 )
        {
            outputDataType = BAPE_DataType_ePcm7_1;
        }
    }
    /* If the output format has changed, propagate that downstream */
    if ( outputDataType != BAPE_Mixer_P_GetOutputDataType(handle) )
    {
        BAPE_FMT_Descriptor format;
        if ( handle->running  && !handle->startedExplicitly)
        {
            BDBG_ERR(("Can not change mixer output format while running."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        if ( handle->running  && handle->startedExplicitly)
        {
            return BERR_SUCCESS;
        }
        BAPE_DspMixer_P_FreeConnectionResources(handle, pConnection);   /* Resources will be allocated next time required. */
        BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);
        format.type = outputDataType;
        BDBG_MSG(("Mixer output format has changed.  Was %s now %s.", BAPE_FMT_P_GetTypeName_isrsafe(BAPE_Mixer_P_GetOutputFormat(handle)), BAPE_FMT_P_GetTypeName_isrsafe(&format)));
        errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static void BAPE_DspMixer_P_InputMute(struct BAPE_PathNode *pNode, struct BAPE_PathConnection *pConnection, bool enabled)
{
    BSTD_UNUSED(pNode);
    BSTD_UNUSED(pConnection);
    BSTD_UNUSED(enabled);
    /* TODO */
}

static void BAPE_DspMixer_P_RemoveInputCallback(struct BAPE_PathNode *pNode, BAPE_PathConnector *pConnector)
{
    BAPE_DspMixer_P_RemoveInput(pNode->pHandle, pConnector);
}

static void BAPE_DspMixer_P_Stop(BAPE_MixerHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( !handle->running )
    {
        BDBG_MSG(("Mixer %p (%d) is already stopped.", (void *)handle, handle->index));
        return;
    }

    if ( !handle->startedExplicitly )
    {
        BDBG_MSG(("Mixer %p (%d) was not started with BAPE_Mixer_Start(). Can't stop.", (void *)handle, handle->index));
        return;
    }

    handle->startedExplicitly = false;
    handle->running--;

    /* Stop the mixer task if nobody is running. */
    if ( 0 == handle->running && handle->taskStarted )
    {
        BDBG_MSG(("BAPE_DspMixer_P_Stop: Last running input has stopped - stop mixer task"));
        BAPE_DspMixer_P_StopTask(handle);
    }

    return;
}


static void BAPE_DspMixer_P_StopTask(BAPE_MixerHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( false == handle->taskStarted )
    {
        return;
    }

    BDBG_ASSERT(0 == handle->running);

    BDSP_Task_Stop(handle->hTask);
    BDSP_Stage_RemoveAllInputs(handle->hMixerStage);
    BDSP_Stage_RemoveAllOutputs(handle->hMixerStage);
    BDSP_Stage_RemoveAllInputs(handle->hSrcStage);
    BDSP_Stage_RemoveAllOutputs(handle->hSrcStage);
    BAPE_PathNode_P_StopPaths(&handle->pathNode);

    if ( handle->loopbackDfifoGroup )
    {
        BAPE_DfifoGroup_P_Destroy(handle->loopbackDfifoGroup);
        handle->loopbackDfifoGroup = NULL;
    }
    if ( handle->loopbackMixerGroup )
    {
        BAPE_MixerGroup_P_Destroy(handle->loopbackMixerGroup);
        handle->loopbackMixerGroup = NULL;
    }
    if ( handle->loopbackGroup )
    {
        BAPE_LoopbackGroup_P_Destroy(handle->loopbackGroup);
        handle->loopbackGroup = NULL;
    }
    BAPE_P_FreeBuffers(handle->deviceHandle, handle->pLoopbackBuffers);
    handle->loopbackDspInput = BAPE_DSPMIXER_INPUT_INVALID;

    /* Unlink from PLL and give up FS if allocated */
    if ( handle->mclkSource != BAPE_MclkSource_eNone )
    {
        #if BAPE_CHIP_MAX_PLLS > 0
            if ( BAPE_MCLKSOURCE_IS_PLL(handle->mclkSource))
            {
                BAPE_P_DetachMixerFromPll(handle, handle->mclkSource - BAPE_MclkSource_ePll0);
            }
        #endif
        #if BAPE_CHIP_MAX_NCOS > 0
            if ( BAPE_MCLKSOURCE_IS_NCO(handle->mclkSource))
            {
                BAPE_P_DetachMixerFromNco(handle, handle->mclkSource - BAPE_MclkSource_eNco0);
            }
        #endif

        handle->mclkSource = BAPE_MclkSource_eNone;
#if BAPE_CHIP_MAX_FS > 0
        BAPE_P_FreeFs(handle->deviceHandle, handle->fs);
        handle->fs = BAPE_FS_INVALID;
#endif
    }

    handle->taskStarted = false;
}

static BERR_Code BAPE_DspMixer_P_AllocateLoopbackInput(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    bool sfifo=false, buffers=false, buffersOnly=false;
    unsigned numChannelPairs;

    BAPE_Connector input = pConnection->pSource;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&pConnection->pSource->format);

    BDBG_MSG(("DSP Mixer %p Allocating resources for input %s %s", (void *)handle, input->pParent->pName, input->pName));

    switch ( pConnection->pSource->format.source )
    {
    case BAPE_DataSource_eDfifo:        /* Technically this could route directly to the DSP but it may require sample rate conversion */
    case BAPE_DataSource_eHostBuffer:
        sfifo = true;                   /* Playback via a Host Buffer requires a SFIFO allocation */
        buffers = pConnection->pSource->useBufferPool;
        break;
    case BAPE_DataSource_eFci:
        break;
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Check if we've already allocated resources */
    if ( pConnection->resourcesAllocated )
    {
        if ( pConnection->format.type == pConnection->pSource->format.type && pConnection->format.ppmCorrection == pConnection->pSource->format.ppmCorrection )
        {
            BDBG_MSG(("Connection %s -> %s %s already has the required resources", input->pParent->pName, pConnection->pSink->pName, input->pName));
            if ( sfifo )
            {
                BDBG_ASSERT(NULL != pConnection->sfifoGroup);
            }
            BDBG_ASSERT(NULL != pConnection->srcGroup);

            buffersOnly = true;
        }
        else
        {
            /* Connection format has changed */
            BDBG_MSG(("Connection %s -> %s %s format changed.  Releasing resources.", input->pParent->pName, pConnection->pSink->pName, input->pName));
            BAPE_DspMixer_P_FreeConnectionResources(handle, pConnection);
        }
    }

    if ( buffers )
    {
        /* This is safe to call multiple times, it only allocates if need be */
        errCode = BAPE_P_AllocateInputBuffers(handle->deviceHandle, input);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    pConnection->format = pConnection->pSource->format;

    if ( buffersOnly )
    {
        /* We don't need to re-add the remaining resources */
        return BERR_SUCCESS;
    }

    BDBG_ASSERT(NULL == pConnection->sfifoGroup);
    BDBG_ASSERT(NULL == pConnection->srcGroup);

    /* Allocate SFIFO if required. */
    if ( sfifo )
    {
        if ( NULL == pConnection->sfifoGroup )
        {
            BAPE_SfifoGroupCreateSettings sfifoCreateSettings;
            BAPE_SfifoGroup_P_GetDefaultCreateSettings(&sfifoCreateSettings);
            sfifoCreateSettings.numChannelPairs = numChannelPairs;
            sfifoCreateSettings.ppmCorrection = false;
            errCode = BAPE_SfifoGroup_P_Create(handle->deviceHandle, &sfifoCreateSettings, &pConnection->sfifoGroup);
            if ( errCode )
            {
                (void)BERR_TRACE(errCode);
                goto err_alloc_sfifo;
            }
        }
    }

    if ( NULL == pConnection->srcGroup )
    {
        BAPE_SrcGroupSettings srcSettings;
        BAPE_SrcGroupCreateSettings srcCreateSettings;
        BAPE_SrcGroup_P_GetDefaultCreateSettings(&srcCreateSettings);
        srcCreateSettings.numChannelPairs = numChannelPairs;
        errCode = BAPE_SrcGroup_P_Create(handle->deviceHandle, &srcCreateSettings, &pConnection->srcGroup);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_alloc_src;
        }
        /* Set SRC Linkage */
        BAPE_SrcGroup_P_GetSettings(pConnection->srcGroup, &srcSettings);
        if ( pConnection->sfifoGroup )
        {
            /* Input From Sfifo */
            BAPE_SfifoGroup_P_GetOutputFciIds(pConnection->sfifoGroup, &srcSettings.input);
        }
        else
        {
            /* Input from other FCI source */
            srcSettings.input = pConnection->inputFciGroup;
        }
        srcSettings.rampEnabled = false;            /* No need to ramp, output from the mixer will again be ramped. */
        srcSettings.startupRampEnabled = false;
        errCode = BAPE_SrcGroup_P_SetSettings(pConnection->srcGroup, &srcSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_src_settings;
        }
    }

    pConnection->resourcesAllocated = true;

    return BERR_SUCCESS;

err_src_settings:
err_alloc_src:
err_alloc_sfifo:
    BAPE_DspMixer_P_FreeConnectionResources(handle, pConnection);
    return errCode;
}

static void BAPE_DspMixer_P_FreeConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    if ( pConnection->hInterTaskBuffer )
    {
        BDSP_InterTaskBuffer_Destroy(pConnection->hInterTaskBuffer);
        pConnection->hInterTaskBuffer = NULL;
    }
    if ( pConnection->pSource->useBufferPool )
    {
        BAPE_P_FreeInputBuffers(handle->deviceHandle, pConnection->pSource);
    }
    if ( NULL != pConnection->sfifoGroup )
    {
        BAPE_SfifoGroup_P_Destroy(pConnection->sfifoGroup);
        pConnection->sfifoGroup = NULL;
    }
    if ( NULL != pConnection->srcGroup )
    {
        BAPE_SrcGroup_P_Destroy(pConnection->srcGroup);
        pConnection->srcGroup = NULL;
    }

    pConnection->resourcesAllocated = false;
}

/* TODO: Share with other mixers */
static void BAPE_DspMixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, unsigned sampleRate)
{
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    /* Only do this if something actually changed */
    if ( BAPE_Mixer_P_GetOutputSampleRate(mixer) != sampleRate )
    {
        unsigned i;

        BDBG_MSG(("Changing DSP mixer %p (%d) sample rate to %u [was %u]", (void *)mixer, mixer->index, sampleRate, BAPE_Mixer_P_GetOutputSampleRate(mixer)));

        /* Propagate sample rate downstream */
        (void)BAPE_Connector_P_SetSampleRate_isr(&mixer->pathNode.connectors[0], sampleRate);

        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            if ( mixer->inputs[i] )
            {
                BAPE_DspMixer_P_SetInputSRC_isr(mixer, mixer->inputs[i], mixer->inputs[i]->format.sampleRate, sampleRate);
            }
        }

        #if BAPE_CHIP_MAX_PLLS > 0
        if ( BAPE_MCLKSOURCE_IS_PLL(mixer->mclkSource) && sampleRate != 0 )
        {
            BERR_Code errCode;
            /* Update the PLL */
            errCode = BAPE_P_UpdatePll_isr(mixer->deviceHandle, mixer->settings.outputPll);
            BDBG_ASSERT(errCode == BERR_SUCCESS);
        }
        #endif

        #if BAPE_CHIP_MAX_NCOS > 0
        if ( BAPE_MCLKSOURCE_IS_NCO(mixer->mclkSource) && sampleRate != 0 )
        {
            BERR_Code errCode;
            /* Update the NCO */
            errCode = BAPE_P_UpdateNco_isr(mixer->deviceHandle, mixer->settings.outputNco);
            BDBG_ASSERT(errCode == BERR_SUCCESS);
        }
        #endif
    }
    else
        {
        BDBG_MSG(("NOT Changing DSP mixer %p (%d) sample rate to %u [currently %u]", (void *)mixer, mixer->index, sampleRate, BAPE_Mixer_P_GetOutputSampleRate(mixer)));
    }
}

/* TODO: Share with other mixers */
static void BAPE_DspMixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_Connector input, unsigned inputRate, unsigned outputRate)
{
    BAPE_PathConnection *pLink=NULL;

    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);

    /* Find connection node for this object */
    for ( pLink = BLST_SQ_FIRST(&input->connectionList);
          NULL != pLink;
          pLink = BLST_SQ_NEXT(pLink, downstreamNode) )
    {
        if ( pLink->pSink == &mixer->pathNode )
            break;
    }
    BDBG_ASSERT(NULL != pLink);

    if ( pLink->srcGroup )
    {
        BDBG_MSG(("Setting input %s SRC to %u->%u", input->pName, inputRate, outputRate));
        BAPE_SrcGroup_P_SetSampleRate_isr(pLink->srcGroup, inputRate, outputRate);
    }
}

/*************************************************************************/
static BERR_Code BAPE_DspMixer_P_GetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    BAPE_MixerInputVolume *pVolume      /* [out] */
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(NULL != pVolume);

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *pVolume = handle->inputVolume[i];

    return BERR_SUCCESS;
}

/*************************************************************************/
static BERR_Code BAPE_DspMixer_P_ApplyInputVolume(BAPE_MixerHandle mixer, unsigned index)
{
    BDSP_Raaga_Audio_MixerDapv2ConfigParams userConfigDapv2;
    BDSP_Raaga_Audio_MixerConfigParams userConfig;
    BAPE_Connector source = mixer->inputs[index];
    BERR_Code errCode;
    unsigned i, taskInputIndex;
    BAPE_PathConnection *pConnection;
    int32_t coeffs[BAPE_Channel_eMax][BAPE_Channel_eMax];

    BDBG_OBJECT_ASSERT(source, BAPE_PathConnector);

    /* Make sure we have a valid input index */
    if ( index == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Find Connection */
    pConnection = BAPE_Connector_P_GetConnectionToSink(mixer->inputs[index], &mixer->pathNode);
    if ( NULL == pConnection )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Is this an FMM input? */
    if ( pConnection->pSource->format.source != BAPE_DataSource_eDspBuffer )
    {
        /* FMM Input - Apply volume to mixer if allocated */
        if ( mixer->loopbackMixerGroup )
        {
            BAPE_MixerGroupInputSettings inputSettings;

            BAPE_MixerGroup_P_GetInputSettings(mixer->loopbackMixerGroup, index, &inputSettings);
            for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
            {
                if ( mixer->inputVolume[index].muted )
                {
                    inputSettings.coefficients[i][0][0] = 0;
                    inputSettings.coefficients[i][1][0] = 0;
                    inputSettings.coefficients[i][0][1] = 0;
                    inputSettings.coefficients[i][1][1] = 0;
                }
                else
                {
                    inputSettings.coefficients[i][0][0] = mixer->inputVolume[index].coefficients[2*i][2*i];
                    inputSettings.coefficients[i][1][0] = mixer->inputVolume[index].coefficients[(2*i)+1][2*i];
                    inputSettings.coefficients[i][0][1] = mixer->inputVolume[index].coefficients[2*i][(2*i)+1];
                    inputSettings.coefficients[i][1][1] = mixer->inputVolume[index].coefficients[(2*i)+1][(2*i)+1];
                }
            }
            errCode = BAPE_MixerGroup_P_SetInputSettings(mixer->loopbackMixerGroup, index, &inputSettings);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }

            BKNI_Memcpy(&coeffs, &unityLoopbackCoefficients, sizeof(coeffs));
        }
        else
        {
            /* We do not know what would fall into this case, its possible something would still need
               to apply coefficents but if its not a dsp input or the loopback mixer we are not sure
               how it would be handled. */
            return BERR_SUCCESS;
        }
    }
    else
    {
        BKNI_Memcpy(&coeffs, &mixer->inputVolume[index].coefficients, sizeof(coeffs));
    }

    /* DSP Input if we get to here */

    /* Input may not be running yet, check that. */
    if ( pConnection->dspInputIndex == BAPE_DSPMIXER_INPUT_INVALID )
    {
        return BERR_SUCCESS;
    }

    /* Get task input index */
    taskInputIndex = pConnection->dspInputIndex;

    if ( taskInputIndex >= BDSP_AF_P_MAX_IP_FORKS )
    {
        BDBG_ASSERT(taskInputIndex < BDSP_AF_P_MAX_IP_FORKS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Apply settings to FW Mixer */
    if ( BAPE_DspMixer_P_GetDolbyUsageVersion(mixer) == BAPE_DolbyMSVersion_eMS12 )
    {
        /*BDSP_Stage_SetAlgorithm(mixer->hMixerStage, BDSP_Algorithm_eMixerDapv2);*/
        errCode = BDSP_Stage_GetSettings(mixer->hMixerStage, &userConfigDapv2, sizeof(userConfigDapv2));
    }
    else
    {
        /*BDSP_Stage_SetAlgorithm(mixer->hMixerStage, BDSP_Algorithm_eMixer);*/
        errCode = BDSP_Stage_GetSettings(mixer->hMixerStage, &userConfig, sizeof(userConfig));
    }
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        uint32_t scaleValue;

        if ( mixer->inputVolume[index].muted || (i >= (2*BAPE_FMT_P_GetNumChannelPairs_isrsafe(&mixer->inputs[index]->format))) )
        {
            scaleValue = 0;
        }
        else
        {
            scaleValue = coeffs[i][i];
            if ( scaleValue > 0x7FFFFFF )
            {
                BDBG_WRN(("Input coefficients out of range for input %s %s - saturating at 0x7FFFFFF", mixer->inputs[index]->pParent->pName, mixer->inputs[index]->pName));
                scaleValue = 0xFFFFFFFF;
            }
            else
            {
                scaleValue <<= 5;   /* Convert from 5.23 (HW) to 4.28 (DSP) */
            }
        }
        BDBG_MSG(("Setting FW Mixing Coefs[%u][%u] to 0x%08x (mute=%u value=%#x)", taskInputIndex, i, scaleValue, mixer->inputVolume[index].muted, coeffs[i][i]));
        userConfigDapv2.i32MixerVolumeControlGains[taskInputIndex][i] = scaleValue;
        userConfig.MixingCoeffs[taskInputIndex][i] = scaleValue;
    }

    if ( BAPE_DspMixer_P_GetDolbyUsageVersion(mixer) == BAPE_DolbyMSVersion_eMS12 )
    {
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, i32MixerUserBalance, mixer->settings.multiStreamBalance);
        errCode = BDSP_Stage_SetSettings(mixer->hMixerStage, &userConfigDapv2, sizeof(userConfigDapv2));
    }
    else
    {
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32UserMixBalance, mixer->settings.multiStreamBalance);
        errCode = BDSP_Stage_SetSettings(mixer->hMixerStage, &userConfig, sizeof(userConfig));
    }
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_DspMixer_P_SetInputVolume(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    const BAPE_MixerInputVolume *pVolume
    )
{
    unsigned i;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(NULL != pVolume);

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle->inputVolume[i] = *pVolume;
    /* Apply volume if input is running */
    if ( handle->running )
    {
        errCode = BAPE_DspMixer_P_ApplyInputVolume(handle, i);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static BERR_Code BAPE_DspMixer_P_SetSettings(
    BAPE_MixerHandle hMixer,
    const BAPE_MixerSettings *pSettings
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(hMixer, BAPE_Mixer);
    BDBG_ASSERT(NULL != pSettings);

    hMixer->settings = *pSettings;

    /* Apply settings to FW Mixer */
    if ( BAPE_DspMixer_P_GetDolbyUsageVersion(hMixer) == BAPE_DolbyMSVersion_eMS12 )
    {
        unsigned i;
        BDSP_Raaga_Audio_MixerDapv2ConfigParams userConfigDapv2;
        if (!hMixer->taskStarted)
        {
            BDSP_Stage_SetAlgorithm(hMixer->hMixerStage, BDSP_Algorithm_eMixerDapv2);
        }
        errCode = BDSP_Stage_GetSettings(hMixer->hMixerStage, &userConfigDapv2, sizeof(userConfigDapv2));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        /* --------------------------------------------------------- */
        /* Set MS12 related userconfig params */
        /* --------------------------------------------------------- */

        /* We will only end up in MS12 mode if compiled in MS12 mode AND we find DDRE downstream.
           So we will always set ms12 to 1 here. */
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, i32Ms12Flag, 1);
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, i32EnableDapv2, pSettings->enablePostProcessing ? 1 : 0);
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, i32MixerUserBalance, pSettings->multiStreamBalance);
        /* Hardcode to the most minimal processing mode */
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32Mode, 3);

        /* --------------------------------------------------------- */
        /* Certification Params */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, i32Certification_flag, (pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_MODE) ? 1 : 0);
        if ( (pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_MODE) )
        {
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32MiProcessDisable, (pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI) ? 1 : 0);
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32MiEnableSurrCompressorSteering, (pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_SURROUNDCOMP) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32MiEnableDialogueEnhancerSteering, (pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_DIALOGENHANCER) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32MiEnableVolumeLevelerSteering, (pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_VOLUMELIMITER) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32MiEnableIEQSteering, (pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_INTELLIGENTEQ) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32IEamount,
                                    BAPE_DSP_P_VALIDATE_VARIABLE_UPPER((pSettings->certificationMode & BAPE_DOLBY_DAP_CERT_EQAMOUNT) >> BAPE_DOLBY_DAP_CERT_EQAMOUNT_SHIFT, 16));
        }

        /* --------------------------------------------------------- */
        /* Configure Volume Leveler */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32EnableVolLeveler, pSettings->volumeLimiter.enableVolumeLimiting ? 1 : 0);
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32leveler_ignore_il, pSettings->volumeLimiter.enableIntelligentLoudness ? 0 : 1);
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32VolLevelInputValue,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER((pSettings->volumeLimiter.enableIntelligentLoudness ? 0 : pSettings->volumeLimiter.volumeLimiterAmount), 10));

        /* --------------------------------------------------------- */
        /* Configure Dialog Enhancer */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32EnableDialogEnhancer, pSettings->dialogEnhancer.enableDialogEnhancer);
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32DialogEnhancerLevel,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(pSettings->dialogEnhancer.dialogEnhancerLevel, 16));
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32DialogEnhancerDuckLevel,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(pSettings->dialogEnhancer.contentSuppressionLevel, 16));

        /* --------------------------------------------------------- */
        /* Configure Intelligent EQ */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32EnableIntelligentEqualizer, pSettings->intelligentEqualizer.enabled ? 1 : 0);
        BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32IEQFrequency_count,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(pSettings->intelligentEqualizer.numBands, 20));
        for ( i = 0; i < userConfigDapv2.sDapv2UserConfig.ui32IEQFrequency_count; i++ )
        {
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.ui32IEQFrequency[i],
                                    BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(pSettings->intelligentEqualizer.band[i].frequency, 20, 20000));
            BAPE_DSP_P_SET_VARIABLE(userConfigDapv2, sDapv2UserConfig.i32IEQInputGain[i],
                                    BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(pSettings->intelligentEqualizer.band[i].gain, -480, 480));
        }

        errCode = BDSP_Stage_SetSettings(hMixer->hMixerStage, &userConfigDapv2, sizeof(userConfigDapv2));
    }
    else
    {
        BDSP_Raaga_Audio_MixerConfigParams userConfig;
        if (!hMixer->taskStarted)
        {
            BDSP_Stage_SetAlgorithm(hMixer->hMixerStage, BDSP_Algorithm_eMixer);
        }
        errCode = BDSP_Stage_GetSettings(hMixer->hMixerStage, &userConfig, sizeof(userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Set Legacy Mixer Settings*/
        BAPE_DSP_P_SET_VARIABLE(userConfig, i32UserMixBalance, pSettings->multiStreamBalance);
        errCode = BDSP_Stage_SetSettings(hMixer->hMixerStage, &userConfig, sizeof(userConfig));
    }

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

static void BAPE_DspMixer_P_EncoderOverflow_isr(void *pParam1, int param2)
{
    BAPE_MixerHandle hMixer = pParam1;

    BDBG_OBJECT_ASSERT(hMixer, BAPE_Mixer);
    BSTD_UNUSED(param2);

    BDBG_MSG(("Encoder output overflow interrupt"));

    if ( hMixer->hMuxOutput )
    {
        BAPE_MuxOutput_P_Overflow_isr(hMixer->hMuxOutput);
    }
}

#define NOT_SUPPORTED(x) NULL
/* If any of the APIs are not supported by a particular type of mixer,
 * just add a placeholder like this:
 *   NOT_SUPPORTED(BAPE_DspMixer_P_RemoveAllInputs)
 */

static const BAPE_MixerInterface g_dspMixerInterface = {
    BAPE_DspMixer_P_Destroy,
    BAPE_DspMixer_P_Start,
    BAPE_DspMixer_P_Stop,
    BAPE_DspMixer_P_AddInput,
    BAPE_DspMixer_P_RemoveInput,
    BAPE_DspMixer_P_RemoveAllInputs,
    NOT_SUPPORTED(BAPE_DspMixer_P_AddOutput),
    NOT_SUPPORTED(BAPE_DspMixer_P_RemoveOutput),
    NOT_SUPPORTED(BAPE_DspMixer_P_RemoveAllOutputs),
    BAPE_DspMixer_P_GetInputVolume,
    BAPE_DspMixer_P_SetInputVolume,
    NOT_SUPPORTED(BAPE_DspMixer_P_ApplyOutputVolume),
    BAPE_DspMixer_P_SetSettings
};
