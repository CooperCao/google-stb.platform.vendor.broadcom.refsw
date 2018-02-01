/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Mixer Interface for DSP Mixers
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#if BAPE_CHIP_MAX_DSP_MIXERS
#include "bdsp.h"
#endif

BDBG_MODULE(bape_mixer_dsp);
BDBG_FILE_MODULE(bape_mixer_lb);

#if BAPE_CHIP_MAX_DSP_MIXERS
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
static BERR_Code BAPE_DspMixer_P_SetSettings(BAPE_MixerHandle handle, const BAPE_MixerSettings *pSettings);
static BERR_Code BAPE_DspMixer_P_RemoveAllInputs(BAPE_MixerHandle handle);
static BERR_Code BAPE_DspMixer_P_CreateLoopBackMixer (BAPE_MixerHandle handle);
static BERR_Code BAPE_DspMixer_P_CreateLoopBackPath (BAPE_MixerHandle handle);
static BERR_Code BAPE_DspMixer_P_StartTask(BAPE_MixerHandle handle);
static void BAPE_DspMixer_P_StopTask(BAPE_MixerHandle handle);
static void BAPE_DspMixer_P_FreeConnectionResources(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static BERR_Code BAPE_DspMixer_P_AllocateLoopbackInput(BAPE_MixerHandle handle, BAPE_PathConnection *pConnection);
static void BAPE_DspMixer_P_SetSampleRate_isr(BAPE_MixerHandle mixer, unsigned sampleRate);
static void BAPE_DspMixer_P_SetInputSRC_isr(BAPE_MixerHandle mixer, BAPE_Connector input, unsigned inputRate, unsigned outputRate);
static BERR_Code BAPE_DspMixer_P_ApplyInputVolume(BAPE_MixerHandle handle, unsigned index);
static void BAPE_DspMixer_P_EncoderOverflow_isr(void *pParam1, int param2);
static void BAPE_DspMixer_P_SampleRateChange_isr(void *pParam1, int param2, unsigned streamSampleRate, unsigned baseSampleRate);
static void BAPE_DspMixer_P_Destroy(BAPE_MixerHandle handle);
static BAPE_MultichannelFormat BAPE_DspMixer_P_GetDdreMultichannelFormat(BAPE_MixerHandle handle);
static bool BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(BAPE_MixerHandle handle);

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

typedef union BAPE_UnifiedFWMixerUserConfig
{
    BDSP_Raaga_Audio_MixerDapv2ConfigParams userConfigDapv2;
    BDSP_Raaga_Audio_MixerConfigParams userConfig;
} BAPE_UnifiedFWMixerUserConfig;

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
#define BAPE_DOLBY_DAP_CERT_ENABLE_SURROUND_DECODER     ((unsigned)1<<6)
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
    BAPE_MixerSettings * pDefaultSettings = NULL;
    BDSP_TaskCreateSettings taskCreateSettings;
    BDSP_StageCreateSettings stageCreateSettings;
    BAPE_FMT_Descriptor format;
    BAPE_FMT_Capabilities caps;
    unsigned dspIndex = 0;
    BDSP_ContextHandle dspContext = NULL;
    unsigned dspIndexBase = 0;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    if ( NULL == pSettings )
    {
        pDefaultSettings = BKNI_Malloc(sizeof(BAPE_MixerSettings));
        if ( pDefaultSettings == NULL )
        {
            BDBG_ERR(("Unable to allocate memory for BAPE_MixerSettings"));
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }
        BAPE_Mixer_GetDefaultSettings(pDefaultSettings);
        pSettings = pDefaultSettings;
    }

    dspIndex = pSettings->dspIndex;

    /* Shift the dsp index to the arm if dsp is not enabled, but arm audio is*/
    if ( pSettings->dspIndex <= BAPE_DEVICE_DSP_LAST &&
         deviceHandle->dspHandle == NULL && deviceHandle->armHandle != NULL )
    {
        dspIndex += BAPE_DEVICE_ARM_FIRST;
    }

    if ( dspIndex >= BAPE_DEVICE_ARM_FIRST &&
         ( dspIndex >= (BAPE_DEVICE_ARM_FIRST + deviceHandle->numArms) ||
           deviceHandle->armHandle == NULL ) )
    {
        BDBG_ERR(("ARM device index %u is not available.  This system has %u ARM Audio Processors, arm handle %p.", dspIndex, deviceHandle->numArms, (void*)deviceHandle->armHandle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( dspIndex <= BAPE_DEVICE_DSP_LAST && (dspIndex >= deviceHandle->numDsps || deviceHandle->dspHandle == NULL ) )
    {
        BDBG_ERR(("DSP %u is not available.  This system has %u DSPs, dsp handle %p.", dspIndex, deviceHandle->numDsps, (void*)deviceHandle->dspHandle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( dspIndex >= BAPE_DEVICE_ARM_FIRST )
    {
        dspContext = deviceHandle->armContext;
        dspIndexBase = BAPE_DEVICE_ARM_FIRST;
    }
    else
    {
        dspContext = deviceHandle->dspContext;
        dspIndexBase = BAPE_DEVICE_DSP_FIRST;
    }

    if ( dspContext == NULL )
    {
        BDBG_ERR(("No DSP or ARM device available."));
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
    handle->settings.dspIndex = dspIndex;
    handle->dspIndex = dspIndex;
    handle->dspContext = dspContext;
    handle->interface = &g_dspMixerInterface;
    handle->deviceHandle = deviceHandle;
    BLST_S_INSERT_HEAD(&deviceHandle->mixerList, handle, node);
    handle->fs = BAPE_FS_INVALID;
    BAPE_P_InitPathNode(&handle->pathNode, BAPE_PathNodeType_eMixer, handle->settings.type, 1, deviceHandle, handle);
    handle->pathNode.deviceIndex = handle->dspIndex;
    handle->pathNode.deviceContext = (void*)handle->dspContext;
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

    BDSP_Task_GetDefaultCreateSettings(handle->dspContext, &taskCreateSettings);
    taskCreateSettings.masterTask = true;
    taskCreateSettings.numSrc = 3;
    taskCreateSettings.dspIndex = BAPE_DSP_DEVICE_INDEX(handle->dspIndex, dspIndexBase);

    errCode = BDSP_Task_Create(handle->dspContext, &taskCreateSettings, &handle->hTask);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_task_create;
    }

    BDSP_Stage_GetDefaultCreateSettings(handle->dspContext, BDSP_AlgorithmType_eAudioMixer, &stageCreateSettings);
    errCode = BDSP_Stage_Create(handle->dspContext, &stageCreateSettings, &handle->hMixerStage);
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

    BDSP_Stage_GetDefaultCreateSettings(handle->dspContext, BDSP_AlgorithmType_eAudioProcessing, &stageCreateSettings);
    BKNI_Memset(&stageCreateSettings.algorithmSupported, 0, sizeof(stageCreateSettings.algorithmSupported));
    stageCreateSettings.algorithmSupported[BDSP_Algorithm_eSrc] = true;
    errCode = BDSP_Stage_Create(handle->dspContext, &stageCreateSettings, &handle->hSrcStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_src_stage;
    }

    *pHandle = handle;

    if ( pDefaultSettings )
    {
        BKNI_Free(pDefaultSettings);
        pDefaultSettings = NULL;
    }

    return BERR_SUCCESS;

err_src_stage:
err_mixer_stage:
err_task_create:
err_caps:
err_format:
    BAPE_DspMixer_P_Destroy(handle);
err_handle:
    if ( pDefaultSettings )
    {
        BKNI_Free(pDefaultSettings);
        pDefaultSettings = NULL;
    }
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

    if (handle->running && handle->startedExplicitly)
    {
        if ( !handle->settings.loopbackMixerEnabled &&
             handle->loopbackGroup != NULL &&
             input->format.source != BAPE_DataSource_eDspBuffer)
        {
            BDBG_ERR(("DspMixer was started with loopbackMixerEnabled=false and loopback is already in use. Another FMM input cannot be supported."));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        if ( input->format.source != BAPE_DataSource_eDspBuffer &&
             handle->loopbackGroup == NULL)
        {
            if ( handle->settings.loopbackMixerEnabled )
            {
                errCode = BAPE_DspMixer_P_CreateLoopBackMixer(handle);
                if (errCode)
                {
                    BDBG_ERR(("Unable to create loopback mixer"));
                    return BERR_TRACE(errCode);
                }
            }

            errCode = BAPE_DspMixer_P_CreateLoopBackPath(handle);
            if (errCode)
            {
                BDBG_ERR(("Unable to create loopback mixer"));
                return BERR_TRACE(errCode);
            }
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
    BAPE_PathNode_P_FindConsumersByType_isrsafe(&handle->pathNode, BAPE_PathNodeType_eMuxOutput, 1, &numFound, &pNode);
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
    BERR_Code errCode;

    BDBG_ASSERT(NULL == handle->loopbackMixerGroup);

    BDBG_MSG(("Allocate DspMixer Loopback Mixer"));
    BAPE_MixerGroup_P_GetDefaultCreateSettings(&mixerCreateSettings);
    mixerCreateSettings.numChannelPairs = 1;
    errCode = BAPE_MixerGroup_P_Create(handle->deviceHandle, &mixerCreateSettings, &handle->loopbackMixerGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode;
    }

    return errCode;
}

static BERR_Code BAPE_DspMixer_P_CreateLoopBackPath (BAPE_MixerHandle handle)
{
    BAPE_LoopbackGroupCreateSettings loopbackCreateSettings;
    BAPE_LoopbackGroupSettings loopbackSettings;
    BAPE_DfifoGroupCreateSettings dfifoCreateSettings;
    BAPE_DfifoGroupSettings dfifoSettings;
    BAPE_FMT_Descriptor format;
    BERR_Code errCode;

    BDBG_ASSERT(NULL == handle->loopbackGroup);
    BDBG_ASSERT(NULL == handle->loopbackDfifoGroup);
    BDBG_ASSERT(NULL == handle->pLoopbackBuffers[0]);

    BDBG_MSG(("Create DspMixer Loopback Path"));

    BAPE_LoopbackGroup_P_GetDefaultCreateSettings(&loopbackCreateSettings);
    loopbackCreateSettings.numChannelPairs = 1;
    errCode = BAPE_LoopbackGroup_P_Create(handle->deviceHandle, &loopbackCreateSettings, &handle->loopbackGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_loopback_create;
    }

    BAPE_FMT_P_InitDescriptor(&format);
    format.source = BAPE_DataSource_eDfifo;
    format.type = BAPE_DataType_ePcmStereo;
    format.sampleRate = 48000;
    errCode = BAPE_P_AllocateBuffers(handle->deviceHandle, &format, handle->pLoopbackBuffers);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_allocate_buffers;
    }

    BAPE_DfifoGroup_P_GetDefaultCreateSettings(&dfifoCreateSettings);
    dfifoCreateSettings.numChannelPairs = 1;
    errCode = BAPE_DfifoGroup_P_Create(handle->deviceHandle, &dfifoCreateSettings, &handle->loopbackDfifoGroup);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_difo_create;
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
        goto err_difo_settings;
    }

    /* Loopback found, allocate an FS timing object */
#if BAPE_CHIP_MAX_FS > 0
    BDBG_ASSERT(handle->fs == BAPE_FS_INVALID);
    handle->fs = BAPE_P_AllocateFs(handle->deviceHandle);
    if ( handle->fs == BAPE_FS_INVALID )
    {
        BDBG_ERR(("Unable to allocate FS resources for input loopback."));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto err_difo_settings;
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
    goto err_loopback_settings;
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
    if ( handle->loopbackMixerGroup )
    {
        BAPE_MixerGroup_P_GetOutputFciIds(handle->loopbackMixerGroup, 0, &loopbackSettings.input);
    }
    /* if we don't have a mixer, wait until we know the source fci id during start */

    BKNI_EnterCriticalSection();
    errCode = BAPE_LoopbackGroup_P_SetSettings_isr(handle->loopbackGroup, &loopbackSettings);
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_loopback_settings;
    }
    return errCode;

err_loopback_settings:
    if ( handle->mclkSource != BAPE_MclkSource_eNone )
    {
        #if BAPE_CHIP_MAX_PLLS > 0
            if ( BAPE_MCLKSOURCE_IS_PLL(handle->mclkSource))
            {
                BAPE_P_DetachMixerFromPll_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_ePll0);
            }
        #endif
        #if BAPE_CHIP_MAX_NCOS > 0
            if ( BAPE_MCLKSOURCE_IS_NCO(handle->mclkSource))
            {
                BAPE_P_DetachMixerFromNco_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_eNco0);
            }
        #endif
        handle->mclkSource = BAPE_MclkSource_eNone;
    }
    if (handle->fs != BAPE_FS_INVALID)
    {
        BAPE_P_FreeFs(handle->deviceHandle, handle->fs);
        handle->fs = BAPE_FS_INVALID;
    }
err_difo_settings:
    if ( handle->loopbackDfifoGroup )
    {
        BAPE_DfifoGroup_P_Destroy(handle->loopbackDfifoGroup);
        handle->loopbackDfifoGroup = NULL;
    }
err_difo_create:
        BAPE_P_FreeBuffers(handle->deviceHandle, handle->pLoopbackBuffers);
err_allocate_buffers:
    if ( handle->loopbackGroup )
    {
        BAPE_LoopbackGroup_P_Destroy(handle->loopbackGroup);
        handle->loopbackGroup = NULL;
    }
err_loopback_create:
    return errCode;
}

static BERR_Code BAPE_DspMixer_P_StartTask(BAPE_MixerHandle handle)
{
    BERR_Code errCode;
    unsigned fmmInputs = 0;
    unsigned input, output;
    BDSP_TaskStartSettings taskStartSettings;
    BAPE_PathConnection *pInputConnection;
    BAPE_PathNode *pNode;
    unsigned numFound, i;
    uint32_t sampleRate = 0;

    BDBG_ASSERT(false == handle->taskStarted);

    /* Apply latest and greatest settings */
    BAPE_DspMixer_P_SetSettings(handle, NULL);

    /* Scan inputs and determine if we need a FS and PLL linkage for loopback */
    for ( pInputConnection = BLST_S_FIRST(&handle->pathNode.upstreamList);
          pInputConnection != NULL;
          pInputConnection = BLST_S_NEXT(pInputConnection, upstreamNode) )
    {
        if ( pInputConnection->pSource->format.source != BAPE_DataSource_eDspBuffer )
        {
            fmmInputs++;
        }
    }

    if ( fmmInputs > 0 )
    {
        bool createMixer = false;
        if ( handle->settings.loopbackMixerEnabled )
        {
            createMixer = true;
        }
        else if ( fmmInputs > 1 )
        {
            BDBG_ERR(("loopback mixer has been explicitly disabled for this dsp mixer, but multiple fmm inputs to the mixer are found. This is not supported."));
            errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto err_acquire_resources;
        }

        if ( createMixer )
        {
            errCode = BAPE_DspMixer_P_CreateLoopBackMixer(handle);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_create_loopback_mixer;
            }
        }

        errCode = BAPE_DspMixer_P_CreateLoopBackPath(handle);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_create_loopback_path;
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
    if (!handle->settings.mixerEnableZeros) {
        taskStartSettings.openGateAtStart = false;
    }
    taskStartSettings.timeBaseType = BDSP_AF_P_TimeBaseType_e45Khz;
    taskStartSettings.schedulingMode = BDSP_TaskSchedulingMode_eMaster;

    /* Add sample rate converter if required */
    handle->pathNode.connectors[0].hStage = handle->hMixerStage;

    if ( BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->pathNode.connectors[0].format) )
    {
        /* IF DDRE is present downstream, output samplerate must be fixed to 48000 */
        if ( BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(handle) )
        {
            BAPE_FMT_Descriptor format;
            /* DDRE cases do not require an SRC stage, so just set the samplerate and go */
            sampleRate = BAPE_DSPMIXER_DDRE_INPUT_SR;

            BAPE_Connector_P_GetFormat(&handle->pathNode.connectors[0], &format);

            format.type = BAPE_DataType_ePcm5_1;
            if ( BAPE_DspMixer_P_GetDdreMultichannelFormat(handle) == BAPE_MultichannelFormat_e7_1 )
            {
                format.type = BAPE_DataType_ePcm7_1;
            }
            errCode = BAPE_Connector_P_SetFormat(&handle->pathNode.connectors[0], &format);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_ddre;
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
    BAPE_DspMixer_P_SetSampleRate_isr(handle, sampleRate == 0 ? 48000 : sampleRate);
    BKNI_LeaveCriticalSection();

    errCode = BAPE_DSP_P_DeriveTaskStartSettings(&handle->pathNode, &taskStartSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_consumers;
    }

    /* Validate we either have real-time outputs or don't depending on system requirements */
    BAPE_PathNode_P_FindConsumersBySubtype_isrsafe(&handle->pathNode, BAPE_PathNodeType_eMixer, BAPE_MixerType_eStandard, 1, &numFound, &pNode);
    if ( taskStartSettings.realtimeMode == BDSP_TaskRealtimeMode_eNonRealTime )
    {
        if ( numFound != 0 )
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
    BAPE_PathNode_P_FindConsumersByType_isrsafe(&handle->pathNode, BAPE_PathNodeType_eMuxOutput, 1, &numFound, &pNode);
    if ( numFound > 0 )
    {
        BDSP_AudioInterruptHandlers interrupts;
        handle->hMuxOutput = (BAPE_MuxOutputHandle)pNode->pHandle;
        BKNI_EnterCriticalSection();
        BDSP_AudioTask_GetInterruptHandlers_isr(handle->hTask, &interrupts);
        interrupts.encoderOutputOverflow.pCallback_isr = BAPE_DspMixer_P_EncoderOverflow_isr;
        interrupts.encoderOutputOverflow.pParam1 = handle;
        interrupts.sampleRateChange.pCallback_isr = BAPE_DspMixer_P_SampleRateChange_isr;
        interrupts.sampleRateChange.pParam1 = handle;
        BDSP_AudioTask_SetInterruptHandlers_isr(handle->hTask, &interrupts);
        BKNI_LeaveCriticalSection();
    }
    else
    {
        BDSP_AudioInterruptHandlers interrupts;
        handle->hMuxOutput = NULL;
        BKNI_EnterCriticalSection();
        BDSP_AudioTask_GetInterruptHandlers_isr(handle->hTask, &interrupts);
        interrupts.sampleRateChange.pCallback_isr = BAPE_DspMixer_P_SampleRateChange_isr;
        interrupts.sampleRateChange.pParam1 = handle;
        BDSP_AudioTask_SetInterruptHandlers_isr(handle->hTask, &interrupts);
        BKNI_LeaveCriticalSection();
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintDownstreamNodes(&handle->pathNode);
    #endif

    if ( BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(handle) )
    {
        BAPE_PathNode *pNode;
        unsigned numFound;

        BAPE_PathNode_P_FindConsumersBySubtype_isrsafe(&handle->pathNode, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDdre, 1, &numFound, &pNode);
        switch ( numFound )
        {
        case 1:
            /* check if we are running the encoder on another device */
            if ( NULL != BAPE_DolbyDigitalReencode_P_GetEncoderTaskHandle((BAPE_DolbyDigitalReencodeHandle)pNode->pHandle) &&
                 BAPE_DolbyDigitalReencode_P_HasCompressedConsumers((BAPE_DolbyDigitalReencodeHandle)pNode->pHandle) )
            {
                taskStartSettings.DependentTaskInfo.numTasks = 1;
                taskStartSettings.DependentTaskInfo.DependentTask[0] = BAPE_DolbyDigitalReencode_P_GetEncoderTaskHandle((BAPE_DolbyDigitalReencodeHandle)pNode->pHandle);
            }
            break;
        default:
            BDBG_ERR(("Expected a DDRE downstream, but none was found."));
            break;
        }
    }

    /* Configure Stage Data sync settings */
    {
        BDSP_AudioTaskDatasyncSettings datasyncSettings;
        errCode = BDSP_AudioStage_GetDatasyncSettings(handle->hMixerStage, &datasyncSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        datasyncSettings.uAlgoSpecConfigStruct.sMixerConfig.eContinuousFMMInput = handle->settings.loopbackMixerEnabled ? BDSP_AF_P_eDisable : BDSP_AF_P_eEnable;

        errCode = BDSP_AudioStage_SetDatasyncSettings(handle->hMixerStage, &datasyncSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

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
    BAPE_PathNode_P_StopPaths(&handle->pathNode);
err_start_path:
err_config_path:
err_consumers:
    BAPE_PathNode_P_ReleasePathResources(&handle->pathNode);
err_acquire_resources:
    BDSP_Stage_RemoveAllInputs(handle->hMixerStage);
    BDSP_Stage_RemoveAllOutputs(handle->hMixerStage);
    BDSP_Stage_RemoveAllInputs(handle->hSrcStage);
    BDSP_Stage_RemoveAllOutputs(handle->hSrcStage);
err_stage:
err_ddre:
    if ( handle->loopbackDfifoGroup )
    {
        BAPE_DfifoGroup_P_Destroy(handle->loopbackDfifoGroup);
        handle->loopbackDfifoGroup = NULL;
    }
    if ( handle->loopbackGroup )
    {
        BAPE_LoopbackGroup_P_Destroy(handle->loopbackGroup);
        handle->loopbackGroup = NULL;
    }
    BAPE_P_FreeBuffers(handle->deviceHandle, handle->pLoopbackBuffers);
err_create_loopback_path:
    if ( handle->loopbackMixerGroup )
    {
        BAPE_MixerGroup_P_Destroy(handle->loopbackMixerGroup);
        handle->loopbackMixerGroup = NULL;
    }
     /* Unlink from PLL and give up FS if allocated */
    if ( handle->mclkSource != BAPE_MclkSource_eNone )
    {
        #if BAPE_CHIP_MAX_PLLS > 0
            if ( BAPE_MCLKSOURCE_IS_PLL(handle->mclkSource))
            {
                BAPE_P_DetachMixerFromPll_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_ePll0);
            }
        #endif
        #if BAPE_CHIP_MAX_NCOS > 0
            if ( BAPE_MCLKSOURCE_IS_NCO(handle->mclkSource))
            {
                BAPE_P_DetachMixerFromNco_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_eNco0);
            }
        #endif

        handle->mclkSource = BAPE_MclkSource_eNone;
    }
    #if BAPE_CHIP_MAX_FS > 0
    if (handle->fs != BAPE_FS_INVALID)
    {
        BAPE_P_FreeFs(handle->deviceHandle, handle->fs);
        handle->fs = BAPE_FS_INVALID;
    }
    #endif
err_create_loopback_mixer:
    return errCode;
}

bool BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(BAPE_MixerHandle handle)
{
    bool foundDdre = false;
    BAPE_PathNode *pNode;
    unsigned numFound;

    BAPE_PathNode_P_FindConsumersBySubtype_isrsafe(&handle->pathNode, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDdre, 1, &numFound, &pNode);
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
    #if 1
    /* instead of querying DDRE for it's multichannel output format, in Config A,
       we want to route 7.1ch data into DDRE, and let DDRE's PCMR downmix to 5.1 if needed. */
    BSTD_UNUSED(handle);

    if ( BAPE_P_DolbyCapabilities_MultichannelPcmFormat() == BAPE_MultichannelFormat_e7_1 )
    {
        return BAPE_MultichannelFormat_e7_1;
    }
    #else
    BAPE_PathNode *pNode;
    unsigned numFound;

    BAPE_PathNode_P_FindConsumersBySubtype_isrsafe(&handle->pathNode, BAPE_PathNodeType_ePostProcessor, BAPE_PostProcessorType_eDdre, 1, &numFound, &pNode);
    switch ( numFound )
    {
    case 1:
        return BAPE_DolbyDigitalReencode_P_GetMultichannelFormat((BAPE_DolbyDigitalReencodeHandle)pNode->pHandle);
        break;
    default:
        BDBG_ERR(("No DDRE found downstream"));
        break;
    }
    #endif

    return BAPE_MultichannelFormat_e5_1;
}

BAPE_DolbyMSVersion BAPE_P_FwMixer_GetDolbyUsageVersion_isrsafe(BAPE_MixerHandle handle)
{
    if ( BAPE_P_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        BDSP_AlgorithmInfo algoInfo;
        BDSP_Handle hDsp = NULL;
        bool ms11MixerSupported;
        bool ms12MixerSupported;

        if ( handle->dspContext != NULL && handle->dspContext == handle->deviceHandle->dspContext )
        {
            hDsp = handle->deviceHandle->dspHandle;
        }
        else if ( handle->dspContext != NULL && handle->dspContext == handle->deviceHandle->armContext )
        {
            hDsp = handle->deviceHandle->armHandle;
        }

        if ( !hDsp )
        {
            BDBG_ERR(("No DSP/ARM device configured, returning BAPE_DolbyMSVersion_eMax"));
            BERR_TRACE(BERR_INVALID_PARAMETER);
            return BAPE_DolbyMSVersion_eMax;
        }

        BDSP_GetAlgorithmInfo(hDsp, BDSP_Algorithm_eMixer, &algoInfo);
        ms11MixerSupported = algoInfo.supported;
        BDSP_GetAlgorithmInfo(hDsp, BDSP_Algorithm_eMixerDapv2, &algoInfo);
        ms12MixerSupported = algoInfo.supported;

        /* IF we find a DDRE downstream, use MS12 mixer,
           else use legacy mixer (transcode, etc) */
        if ( BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(handle) ||
             (!ms11MixerSupported && ms12MixerSupported) )
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

static BERR_Code BAPE_DspMixer_P_ValidateInput(
    BAPE_MixerHandle handle,
    struct BAPE_PathConnection *pConnection
    )
{
    unsigned i;

    if ( pConnection->pSource->format.source != BAPE_DataSource_eDspBuffer )
    {
        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            if ( handle->inputs[i] && handle->inputs[i] != pConnection->pSource )
            {
                BAPE_DecoderHandle decoder;
                if ( handle->inputs[i]->pParent->type == BAPE_PathNodeType_eDecoder )
                {
                    decoder = (BAPE_DecoderHandle)handle->inputs[i]->pParent->pHandle;
                    if ( decoder != NULL )
                    {
                        BDBG_OBJECT_ASSERT(decoder, BAPE_Decoder);
                        if ( decoder->state == BAPE_DecoderState_eStarted ||
                             decoder->state == BAPE_DecoderState_eStarting ||
                             decoder->state == BAPE_DecoderState_ePaused ||
                             decoder->state == BAPE_DecoderState_eFrozen)
                        {
                            if ( decoder->startSettings.mixingMode == BAPE_DecoderMixingMode_eSoundEffects )
                            {
                                BDBG_ERR(("Unable to start Non-DSP path: Sound Effects input to Mixer is already in use"));
                                return BERR_NOT_SUPPORTED;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        BAPE_DecoderHandle decoder = (BAPE_DecoderHandle)pConnection->pSource->pParent->pHandle;
        if ( decoder != NULL )
        {
            BAPE_DecoderMixingMode mixingMode = decoder->startSettings.mixingMode;
            BDBG_OBJECT_ASSERT(decoder, BAPE_Decoder);

            for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
            {
                if ( handle->inputs[i] && handle->inputs[i] != pConnection->pSource )
                {
                    if ( handle->inputs[i]->pParent->type == BAPE_PathNodeType_eDecoder )
                    {
                        decoder = (BAPE_DecoderHandle)handle->inputs[i]->pParent->pHandle;
                        if ( decoder != NULL )
                        {
                            BDBG_OBJECT_ASSERT(decoder, BAPE_Decoder);
                            if ( decoder->state == BAPE_DecoderState_eStarted ||
                                 decoder->state == BAPE_DecoderState_eStarting ||
                                 decoder->state == BAPE_DecoderState_ePaused ||
                                 decoder->state == BAPE_DecoderState_eFrozen)
                            {
                                if ( (decoder->startSettings.mixingMode == BAPE_DecoderMixingMode_eSoundEffects  && mixingMode == BAPE_DecoderMixingMode_eSoundEffects) ||
                                     (decoder->startSettings.mixingMode == BAPE_DecoderMixingMode_eApplicationAudio  && mixingMode == BAPE_DecoderMixingMode_eApplicationAudio))
                                {
                                    BDBG_ERR(("Unable to start path: %d input to Mixer is already in use", mixingMode));
                                    return BERR_NOT_SUPPORTED;
                                }
                            }
                        }
                    }
                    else if ( mixingMode == BAPE_DecoderMixingMode_eSoundEffects && handle->loopbackRunning &&
                              ( handle->inputs[i]->pParent->type == BAPE_PathNodeType_ePlayback ||
                               handle->inputs[i]->pParent->type == BAPE_PathNodeType_eInputCapture ))
                    {
                        BDBG_ERR(("Unable to start DSP path: Sound Effects input to Mixer is already in use"));
                        return BERR_NOT_SUPPORTED;
                    }
                }
            }
        }
    }
    return BERR_SUCCESS;

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
        if ( ( pConnection->pSource->pParent->deviceIndex != BAPE_DSP_ID_INVALID &&
               handle->settings.dspIndex != pConnection->pSource->pParent->deviceIndex ) ||
               ( pConnection->pSource->pParent->deviceContext != NULL &&
                 handle->dspContext != pConnection->pSource->pParent->deviceContext ) )
        {
            BDBG_ERR(("All inputs to a DSP mixer must run on the same DSP as the DSP mixer."));
            BDBG_ERR(("This mixer is configured for DSP(%p) %u but the input %s is configured for DSP(%p) %u",
                      (void*)handle->dspContext, handle->dspIndex, pConnection->pSource->pParent->pName,
                      (void*)pConnection->pSource->pParent->deviceContext, pConnection->pSource->pParent->deviceIndex));
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

    if ( BAPE_P_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
    {
        errCode = BAPE_DspMixer_P_ValidateInput(handle, pConnection);
        if (errCode)
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
            errCode = BDSP_InterTaskBuffer_Create(handle->dspContext,
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

        /* if we don't have a mixer group, link the upstream src directly to the loopback */
        if ( !handle->loopbackMixerGroup )
        {
            BAPE_LoopbackGroupSettings loopbackSettings;

            BKNI_EnterCriticalSection();
            BAPE_LoopbackGroup_P_GetSettings_isr(handle->loopbackGroup, &loopbackSettings);
            BKNI_LeaveCriticalSection();
            BAPE_SrcGroup_P_GetOutputFciIds(pConnection->srcGroup, &loopbackSettings.input);
            BKNI_EnterCriticalSection();
            errCode = BAPE_LoopbackGroup_P_SetSettings_isr(handle->loopbackGroup, &loopbackSettings);
            BKNI_LeaveCriticalSection();
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                return errCode;
            }
        }
        errCode = BAPE_LoopbackGroup_P_Start(handle->loopbackGroup);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_loopback_start;
        }
        if ( handle->loopbackMixerGroup )
        {
            errCode = BAPE_MixerGroup_P_StartOutput(handle->loopbackMixerGroup, 0);
            if ( errCode )
            {
                (void)BERR_TRACE(errCode);
                goto err_mixer_output;
            }
        }
        /* Refresh input scaling into the loopback mixer */
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
        /* Refresh input scaling for the loopback mixer into dsp mixer */
        errCode = BAPE_DspMixer_P_ApplyInputVolume(handle, inputIndex);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_input_vol;
        }
    }
    else if ( !handle->loopbackMixerGroup ) /* need to stop and restart the loopback */
    {
        /* since we don't have a mixer group, link the upstream src directly to the loopback */
        BAPE_LoopbackGroupSettings loopbackSettings;

        BAPE_LoopbackGroup_P_Stop(handle->loopbackGroup);

        BKNI_EnterCriticalSection();
        BAPE_LoopbackGroup_P_GetSettings_isr(handle->loopbackGroup, &loopbackSettings);
        BKNI_LeaveCriticalSection();

        BAPE_SrcGroup_P_GetOutputFciIds(pConnection->srcGroup, &loopbackSettings.input);

        BKNI_EnterCriticalSection();
        errCode = BAPE_LoopbackGroup_P_SetSettings_isr(handle->loopbackGroup, &loopbackSettings);
        BKNI_LeaveCriticalSection();
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            return errCode;
        }
        errCode = BAPE_LoopbackGroup_P_Start(handle->loopbackGroup);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            goto err_loopback_start;
        }
    }
    handle->loopbackRunning++;
    if ( handle->loopbackMixerGroup )
    {
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
    if ( handle->loopbackMixerGroup )
    {
        BAPE_MixerGroup_P_StopInput(handle->loopbackMixerGroup, inputIndex);
    }
err_mixer_input:
    handle->loopbackRunning--;
    if ( 0 == handle->loopbackRunning )
    {
err_input_vol:
        BDSP_Stage_RemoveInput(handle->hMixerStage, handle->loopbackDspInput);
        handle->loopbackDspInput = BAPE_DSPMIXER_INPUT_INVALID;
err_add_input:
err_input_scaling:
        if ( handle->loopbackMixerGroup )
        {
            BAPE_MixerGroup_P_StopOutput(handle->loopbackMixerGroup, 0);
        }
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
        if ( handle->loopbackRunning == 0 )
        {
            BDBG_MSG(("Input already stopped"));
            return;
        }

        BDBG_ASSERT(handle->running > 0);

        handle->loopbackRunning--;

        if ( pConnection->srcGroup )
        {
            BAPE_SrcGroup_P_Stop(pConnection->srcGroup);
        }
        if ( handle->loopbackMixerGroup )
        {
            BAPE_MixerGroup_P_StopInput(handle->loopbackMixerGroup, inputIndex);
        }

        if ( handle->loopbackRunning == 0 )
        {
            BDSP_Stage_RemoveInput(handle->hMixerStage, handle->loopbackDspInput);
            handle->loopbackDspInput = BAPE_DSPMIXER_INPUT_INVALID;
            if ( handle->loopbackMixerGroup )
            {
                BAPE_MixerGroup_P_StopOutput(handle->loopbackMixerGroup, 0);
            }
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
    else if ( BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(handle) && BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->pathNode.connectors[0].format) )
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
        else if (!handle->master &&
                 pConnection->pSource->pParent->type == BAPE_PathNodeType_ePlayback &&
                 !BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(handle)) { /* If master is null, its pcm playback and no DDRE */
            BAPE_DspMixer_P_SetSampleRate_isr(handle, handle->settings.defaultSampleRate);
        }
    }

    /* Update SRCs accordingly. */
    if (pConnection->pSource->pParent->type == BAPE_PathNodeType_ePlayback)
    {
        BAPE_DspMixer_P_SetInputSRC_isr(handle, pConnection->pSource, sampleRate, 48000);
    }
    else
    {
        BAPE_DspMixer_P_SetInputSRC_isr(handle, pConnection->pSource, sampleRate, BAPE_Mixer_P_GetOutputSampleRate_isr(handle));
    }
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

    BDBG_MSG(("%s: old type %lu, new type %lu", BSTD_FUNCTION, (unsigned long)pConnection->format.type, (unsigned long)pNewFormat->type));

    /* See if input format has changed.  If not, do nothing. */
    if ( pNewFormat->type == pConnection->format.type )
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

    if ( BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(handle) )
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
                BAPE_P_DetachMixerFromPll_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_ePll0);
            }
        #endif
        #if BAPE_CHIP_MAX_NCOS > 0
            if ( BAPE_MCLKSOURCE_IS_NCO(handle->mclkSource))
            {
                BAPE_P_DetachMixerFromNco_isrsafe(handle, handle->mclkSource - BAPE_MclkSource_eNco0);
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

    BAPE_Connector input;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    input = pConnection->pSource;

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
            BAPE_SfifoGroup_P_GetOutputFciIds_isrsafe(pConnection->sfifoGroup, &srcSettings.input);
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
                if (mixer->inputs[i]->pParent->type == BAPE_PathNodeType_ePlayback)
                {
                    BAPE_DspMixer_P_SetInputSRC_isr(mixer, mixer->inputs[i], mixer->inputs[i]->format.sampleRate, 48000);
                }
                else
                {
                    BAPE_DspMixer_P_SetInputSRC_isr(mixer, mixer->inputs[i], mixer->inputs[i]->format.sampleRate, sampleRate);
                }
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
static BERR_Code BAPE_DspMixer_P_ApplyInputVolume(BAPE_MixerHandle handle, unsigned index)
{
    BAPE_Connector source = handle->inputs[index];
    BERR_Code errCode;
    unsigned i, j, taskInputIndex;
    BAPE_PathConnection *pConnection;
    int32_t coeffs[BAPE_Channel_eMax][BAPE_Channel_eMax];
    uint32_t scaleValue[BAPE_Channel_eMax];
    int32_t enableCustomMixing;
    BAPE_UnifiedFWMixerUserConfig * pUserConfig = NULL;

    BDBG_OBJECT_ASSERT(source, BAPE_PathConnector);

    /* Make sure we have a valid input index */
    if ( index == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Find Connection */
    pConnection = BAPE_Connector_P_GetConnectionToSink_isrsafe(handle->inputs[index], &handle->pathNode);
    if ( NULL == pConnection )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Is this an FMM input? */
    if ( pConnection->pSource->format.source != BAPE_DataSource_eDspBuffer )
    {
        /* FMM Input - Apply volume to mixer if allocated */
        if ( handle->loopbackMixerGroup )
        {
            BAPE_MixerGroupInputSettings inputSettings;

            BAPE_MixerGroup_P_GetInputSettings(handle->loopbackMixerGroup, index, &inputSettings);
            for ( i = 0; i < BAPE_ChannelPair_eMax; i++ )
            {
                if ( handle->inputVolume[index].muted )
                {
                    inputSettings.coefficients[i][0][0] = 0;
                    inputSettings.coefficients[i][1][0] = 0;
                    inputSettings.coefficients[i][0][1] = 0;
                    inputSettings.coefficients[i][1][1] = 0;
                }
                else
                {
                    inputSettings.coefficients[i][0][0] = handle->inputVolume[index].coefficients[2*i][2*i];
                    inputSettings.coefficients[i][1][0] = handle->inputVolume[index].coefficients[(2*i)+1][2*i];
                    inputSettings.coefficients[i][0][1] = handle->inputVolume[index].coefficients[2*i][(2*i)+1];
                    inputSettings.coefficients[i][1][1] = handle->inputVolume[index].coefficients[(2*i)+1][(2*i)+1];
                }
            }
            errCode = BAPE_MixerGroup_P_SetInputSettings(handle->loopbackMixerGroup, index, &inputSettings);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }

            taskInputIndex = handle->loopbackDspInput;
            BKNI_Memcpy(&coeffs, &unityLoopbackCoefficients, sizeof(coeffs));
        }
        else
        {
            /* We could get into this if there is not a loopback mixer created. */
            return BERR_SUCCESS;
        }
    }
    else
    {
        taskInputIndex = pConnection->dspInputIndex;
        BKNI_Memcpy(&coeffs, &handle->inputVolume[index].coefficients, sizeof(coeffs));
    }

    /* DSP Input if we get to here */

    /* Input may not be running yet, check that. */
    if ( taskInputIndex == BAPE_DSPMIXER_INPUT_INVALID )
    {
        return BERR_SUCCESS;
    }

    if ( taskInputIndex >= BDSP_AF_P_MAX_IP_FORKS )
    {
        BDBG_ASSERT(taskInputIndex < BDSP_AF_P_MAX_IP_FORKS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for ( i = 0; i < BAPE_Channel_eMax; i++ )
    {
        if ( handle->inputVolume[index].muted ||
             ( pConnection->pSource->format.source == BAPE_DataSource_eDspBuffer && (i >= (2*BAPE_FMT_P_GetNumChannelPairs_isrsafe(&handle->inputs[index]->format)))) )
        {
            scaleValue[i] = 0;
        }
        else
        {
            scaleValue[i] = coeffs[i][i];
            if ( scaleValue[i] > 0x7FFFFFF )
            {
                BDBG_WRN(("Input coefficients out of range for input %s %s - saturating at 0x7FFFFFF", handle->inputs[index]->pParent->pName, handle->inputs[index]->pName));
                scaleValue[i] = 0xFFFFFFFF;
            }
            else
            {
                scaleValue[i] <<= 5;   /* Convert from 5.23 (HW) to 4.28 (DSP) */
            }
        }
        BDBG_MSG(("Setting FW Mixing Coefs[%u][%u] to 0x%08x (mute=%u value=%#x)", taskInputIndex, i, scaleValue[i], handle->inputVolume[index].muted, coeffs[i][i]));
    }

    pUserConfig = BKNI_Malloc(sizeof(BAPE_UnifiedFWMixerUserConfig));
    if ( pUserConfig == NULL )
    {
        BDBG_ERR(("Failed to allocate BAPE_UnifiedFWMixerUserConfig"));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Apply settings to FW Mixer */
    if ( BAPE_P_FwMixer_GetDolbyUsageVersion(handle) == BAPE_DolbyMSVersion_eMS12 )
    {
        BDSP_AudioTaskDatasyncSettings datasyncSettings;
        BAPE_DecoderMixingMode mixingMode = BAPE_DecoderMixingMode_eMax;

        /*BDSP_Stage_SetAlgorithm(mixer->hMixerStage, BDSP_Algorithm_eMixerDapv2);*/
        errCode = BDSP_Stage_GetSettings(handle->hMixerStage, &(pUserConfig->userConfigDapv2), sizeof(pUserConfig->userConfigDapv2));
        if ( errCode )
        {
            goto err_cleanup;
        }

        /* set volumes */
        for ( i = 0; i < BAPE_Channel_eMax; i++ )
        {
            pUserConfig->userConfigDapv2.i32MixerVolumeControlGains[taskInputIndex][i] = scaleValue[i];
        }

        /* check input types and modes */
        for ( i = 0; i < BAPE_CHIP_MAX_MIXER_INPUTS; i++ )
        {
            BAPE_DecoderHandle decoder;
            if ( handle->inputs[i] && handle->inputs[i]->pParent->type == BAPE_PathNodeType_eDecoder )
            {
                decoder = (BAPE_DecoderHandle)handle->inputs[i]->pParent->pHandle;
                if ( decoder->state == BAPE_DecoderState_eStarted ||
                     decoder->state == BAPE_DecoderState_eStarting ||
                     decoder->state == BAPE_DecoderState_ePaused ||
                     decoder->state == BAPE_DecoderState_eFrozen ||
                     i == index )
                {
                    if ( mixingMode == BAPE_DecoderMixingMode_eMax &&
                         (decoder->startSettings.mixingMode == BAPE_DecoderMixingMode_eDescription ||
                          decoder->startSettings.mixingMode == BAPE_DecoderMixingMode_eStandalone) )
                    {
                        mixingMode = decoder->startSettings.mixingMode;
                    }

                    if ( (decoder->startSettings.mixingMode == BAPE_DecoderMixingMode_eDescription && mixingMode == BAPE_DecoderMixingMode_eStandalone) ||
                         (decoder->startSettings.mixingMode == BAPE_DecoderMixingMode_eStandalone && mixingMode == BAPE_DecoderMixingMode_eDescription) )
                    {
                        BDBG_WRN(("WARNING: Mismatch between Decoders. One has mixingMode %d and another has mixingMode %d. Using %d to configure BDSP.", mixingMode, decoder->startSettings.mixingMode, mixingMode));
                    }
                }
            }
        }

        errCode = BDSP_AudioStage_GetDatasyncSettings(handle->hMixerStage, &datasyncSettings);
        if ( errCode )
        {
            goto err_cleanup;
        }

        datasyncSettings.uAlgoSpecConfigStruct.sMixerDapv2Config.ui32DualMainModeEnable = false;
        switch ( mixingMode )
        {
        case BAPE_DecoderMixingMode_eDescription:
            BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, i32MixingMode, 0);
            break;
        case BAPE_DecoderMixingMode_eStandalone:
            datasyncSettings.uAlgoSpecConfigStruct.sMixerDapv2Config.ui32DualMainModeEnable = true;
            BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, i32MixingMode, 2);
            break;
        default:
            BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, i32MixingMode, 1);
            break;
        }

        errCode = BDSP_AudioStage_SetDatasyncSettings(handle->hMixerStage, &datasyncSettings);
        if ( errCode )
        {
            goto err_cleanup;
        }

        /* update fade configuration for this input */
        /* in cert mode, allow volumes to pass through as dB, without alteration, in production, convert to Q1.31 format */
        if ( (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_MODE) )
        {
            if ( (unsigned)pUserConfig->userConfigDapv2.sFadeControl[taskInputIndex].i32attenuation != handle->inputVolume[index].fade.level )
            {
                BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, sFadeControl[taskInputIndex].i32attenuation, (int32_t)handle->inputVolume[index].fade.level);
                BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, sFadeControl[taskInputIndex].i32type, (int32_t)handle->inputVolume[index].fade.type);
                BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, sFadeControl[taskInputIndex].i32duration, (int32_t)handle->inputVolume[index].fade.duration);
            }
        }
        else
        {
            /* scale percentage to Q1.31 format for production usage case.
               BDSP will convert to dB from there, for the range -96 dB to 0 dB */
            unsigned levelQ131 = (unsigned) ((((int64_t)handle->inputVolume[index].fade.level)<<31) / 100);

            if ( levelQ131 > 0x7FFFFFFF )
            {
                levelQ131 = 0x7FFFFFFF;
            }
            if ( levelQ131 != (unsigned)pUserConfig->userConfigDapv2.sFadeControl[taskInputIndex].i32attenuation )
            {
                BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, sFadeControl[taskInputIndex].i32attenuation, (int32_t)levelQ131);
                BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, sFadeControl[taskInputIndex].i32type, (int32_t)handle->inputVolume[index].fade.type);
                BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, sFadeControl[taskInputIndex].i32duration, (int32_t)handle->inputVolume[index].fade.duration);
            }
        }

        BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfigDapv2, i32MixerUserBalance, handle->settings.multiStreamBalance);
        errCode = BDSP_Stage_SetSettings(handle->hMixerStage, &(pUserConfig->userConfigDapv2), sizeof(pUserConfig->userConfigDapv2));
        if ( errCode )
        {
            goto err_cleanup;
        }
    }
    else
    {
        /*BDSP_Stage_SetAlgorithm(mixer->hMixerStage, BDSP_Algorithm_eMixer);*/
        errCode = BDSP_Stage_GetSettings(handle->hMixerStage, &(pUserConfig->userConfig), sizeof(pUserConfig->userConfig));
        if ( errCode )
        {
            goto err_cleanup;
        }

        for ( i = 0; i < BAPE_Channel_eMax; i++ )
        {
            BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfig, MixingCoeffs[taskInputIndex][i], scaleValue[i]);
        }

        enableCustomMixing = false;
        for ( i = 0; i < 6; i++ )
        {
            for ( j = 0; j < 6; j++ )
            {
                BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfig, i32CustomMixingCoefficients[i][j], handle->settings.loopbackVolumeMatrix[i][j]<<7);
                if ( (i != j && handle->settings.loopbackVolumeMatrix[i][j] != 0) ||
                     (i == j && handle->settings.loopbackVolumeMatrix[i][j] != BAPE_VOLUME_NORMAL) )
                {
                    enableCustomMixing = true;
                }
            }
        }

        #if BDBG_DEBUG_BUILD
        BDBG_MODULE_MSG(bape_mixer_lb, ("Setting Loopback FW Mixing Coefs (enableCustomMixing=%u):", enableCustomMixing));
        for ( i = 0; i < 6; i++ )
        {
            BDBG_MODULE_MSG(bape_mixer_lb, ("%08x %08x %08x %08x %08x %08x",
                      pUserConfig->userConfig.i32CustomMixingCoefficients[i][0],
                      pUserConfig->userConfig.i32CustomMixingCoefficients[i][1],
                      pUserConfig->userConfig.i32CustomMixingCoefficients[i][2],
                      pUserConfig->userConfig.i32CustomMixingCoefficients[i][3],
                      pUserConfig->userConfig.i32CustomMixingCoefficients[i][4],
                      pUserConfig->userConfig.i32CustomMixingCoefficients[i][5]));
        }
        #endif
        BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfig, i32CustomEffectsAudioMixingEnable, enableCustomMixing);
        BAPE_DSP_P_SET_VARIABLE(pUserConfig->userConfig, i32UserMixBalance, handle->settings.multiStreamBalance);
        errCode = BDSP_Stage_SetSettings(handle->hMixerStage, &(pUserConfig->userConfig), sizeof(pUserConfig->userConfig));
        if ( errCode )
        {
            goto err_cleanup;
        }
    }

    if ( pUserConfig )
    {
        BKNI_Free(pUserConfig);
        pUserConfig = NULL;
    }

    return BERR_SUCCESS;
err_cleanup:
    if ( pUserConfig )
    {
        BKNI_Free(pUserConfig);
        pUserConfig = NULL;
    }
    return BERR_TRACE(errCode);
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
    BAPE_MixerHandle handle,
    const BAPE_MixerSettings *pSettings
    )
{
    BERR_Code errCode;
    BAPE_UnifiedFWMixerUserConfig userConfig;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);

    if ( pSettings != NULL )
    {
        handle->settings = *pSettings;
    }

    /* Apply settings to FW Mixer */
    if ( BAPE_P_FwMixer_GetDolbyUsageVersion(handle) == BAPE_DolbyMSVersion_eMS12 )
    {
        unsigned i;
        if (!handle->taskStarted)
        {
            BDSP_Stage_SetAlgorithm(handle->hMixerStage, BDSP_Algorithm_eMixerDapv2);
        }
        errCode = BDSP_Stage_GetSettings(handle->hMixerStage, &userConfig.userConfigDapv2, sizeof(userConfig.userConfigDapv2));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        /* --------------------------------------------------------- */
        /* Set MS12 related userconfig params */
        /* --------------------------------------------------------- */

        /* We will only end up in MS12 mode if compiled in MS12 mode AND we find DDRE downstream.
           So we will always set ms12 to 1 here. */
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, i32Ms12Flag, 1);
        if ( BAPE_P_DolbyCapabilities_Dapv2() )
        {
            BDBG_MSG(("DAP v2 is supported"));
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, i32EnableDapv2, handle->settings.enablePostProcessing ? 1 : 0);
        }
        else
        {
            BDBG_MSG(("DAP v2 is NOT supported"));
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, i32EnableDapv2, 0);
        }
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, i32MixerUserBalance, handle->settings.multiStreamBalance);
        /* Hardcode to the most minimal processing mode */
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32Mode, 3);

        /* --------------------------------------------------------- */
        /* Certification Params */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, i32Certification_flag, (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_MODE) ? 1 : 0);
        if ( (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_MODE) )
        {
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32MiProcessDisable, (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI) ? 1 : 0);
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32MiEnableSurrCompressorSteering, (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_SURROUNDCOMP) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32MiEnableDialogueEnhancerSteering, (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_DIALOGENHANCER) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32MiEnableVolumeLevelerSteering, (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_VOLUMELIMITER) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32MiEnableIEQSteering, (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_DISABLE_MI_INTELLIGENTEQ) ? 0 : 1);
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32EnableSurroundDecoder, (handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_ENABLE_SURROUND_DECODER) ? 1 : 0);
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32IEamount,
                                    BAPE_DSP_P_VALIDATE_VARIABLE_UPPER((handle->settings.certificationMode & BAPE_DOLBY_DAP_CERT_EQAMOUNT) >> BAPE_DOLBY_DAP_CERT_EQAMOUNT_SHIFT, 16));
            if ( (unsigned)userConfig.userConfigDapv2.sFadeControl[4].i32attenuation != handle->settings.fade.mainDecodeFade.level )
            {
                BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sFadeControl[4].i32attenuation, (int32_t)handle->settings.fade.mainDecodeFade.level);
                BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sFadeControl[4].i32type, (int32_t)handle->settings.fade.mainDecodeFade.type);
                BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sFadeControl[4].i32duration, (int32_t)handle->settings.fade.mainDecodeFade.duration);
            }
        }
        else
        {
            /* scale percentage to Q1.31 format for production usage case.
               BDSP will convert to dB from there, for the range -96 dB to 0 dB */
            unsigned levelQ131 = (unsigned) (((int64_t)handle->settings.fade.mainDecodeFade.level)<<31) / 100;

            if ( levelQ131 > 0x7FFFFFFF )
            {
                levelQ131 = 0x7FFFFFFF;
            }
            if ( levelQ131 != (unsigned)userConfig.userConfigDapv2.sFadeControl[4].i32attenuation )
            {
                BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sFadeControl[4].i32attenuation, (int32_t)levelQ131);
                BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sFadeControl[4].i32type, (int32_t)handle->settings.fade.mainDecodeFade.type);
                BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sFadeControl[4].i32duration, (int32_t)handle->settings.fade.mainDecodeFade.duration);
            }

            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32MiProcessDisable, 1);
        }

        /* --------------------------------------------------------- */
        /* Configure Volume Leveler */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32EnableVolLeveler, handle->settings.volumeLimiter.enableVolumeLimiting ? 1 : 0);
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32leveler_ignore_il, handle->settings.volumeLimiter.enableIntelligentLoudness ? 0 : 1);
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32VolLevelInputValue,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(handle->settings.volumeLimiter.volumeLimiterAmount, 10));

        /* --------------------------------------------------------- */
        /* Configure Dialog Enhancer */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32EnableDialogEnhancer, handle->settings.dialogEnhancer.enableDialogEnhancer);
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32DialogEnhancerLevel,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(handle->settings.dialogEnhancer.dialogEnhancerLevel, 16));
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32DialogEnhancerDuckLevel,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(handle->settings.dialogEnhancer.contentSuppressionLevel, 16));

        /* --------------------------------------------------------- */
        /* Configure Intelligent EQ */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32EnableIntelligentEqualizer, handle->settings.intelligentEqualizer.enabled ? 1 : 0);
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32IEQFrequency_count,
                                BAPE_DSP_P_VALIDATE_VARIABLE_UPPER(handle->settings.intelligentEqualizer.numBands, 20));
        for ( i = 0; i < userConfig.userConfigDapv2.sDapv2UserConfig.ui32IEQFrequency_count; i++ )
        {
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.ui32IEQFrequency[i],
                                    BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(handle->settings.intelligentEqualizer.band[i].frequency, 20, 20000));
            BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, sDapv2UserConfig.i32IEQInputGain[i],
                                    BAPE_DSP_P_VALIDATE_VARIABLE_RANGE(handle->settings.intelligentEqualizer.band[i].gain, -480, 480));
        }

        /* --------------------------------------------------------- */
        /* Fade */
        /* --------------------------------------------------------- */
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfigDapv2, i32RampMode, handle->settings.fade.continuousFading ? 1 : 0);
        errCode = BDSP_Stage_SetSettings(handle->hMixerStage, &userConfig.userConfigDapv2, sizeof(userConfig.userConfigDapv2));
    }
    else
    {
        unsigned i, j;
        int32_t enableCustomMixing;

        if (!handle->taskStarted)
        {
            errCode = BDSP_Stage_SetAlgorithm(handle->hMixerStage, BDSP_Algorithm_eMixer);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        errCode = BDSP_Stage_GetSettings(handle->hMixerStage, &userConfig.userConfig, sizeof(userConfig.userConfig));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Set Legacy Mixer Settings*/
        enableCustomMixing = false;
        for ( i = 0; i < 6; i++ )
        {
            for ( j = 0; j < 6; j++ )
            {
                BAPE_DSP_P_SET_VARIABLE(userConfig.userConfig, i32CustomMixingCoefficients[i][j], handle->settings.loopbackVolumeMatrix[i][j] << 7);
                if ( (i != j && handle->settings.loopbackVolumeMatrix[i][j] != 0) ||
                     (i == j && handle->settings.loopbackVolumeMatrix[i][j] != BAPE_VOLUME_NORMAL) )
                {
                    enableCustomMixing = true;
                }
            }
        }

        #if BDBG_DEBUG_BUILD
        BDBG_MODULE_MSG(bape_mixer_lb, ("Setting Loopback FW Mixing Coefs (enableCustomMixing=%u):", enableCustomMixing));
        for ( i = 0; i < 6; i++ )
        {
            BDBG_MODULE_MSG(bape_mixer_lb, ("%08x %08x %08x %08x %08x %08x",
                      userConfig.userConfig.i32CustomMixingCoefficients[i][0],
                      userConfig.userConfig.i32CustomMixingCoefficients[i][1],
                      userConfig.userConfig.i32CustomMixingCoefficients[i][2],
                      userConfig.userConfig.i32CustomMixingCoefficients[i][3],
                      userConfig.userConfig.i32CustomMixingCoefficients[i][4],
                      userConfig.userConfig.i32CustomMixingCoefficients[i][5]));
        }
        #endif
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfig, i32CustomEffectsAudioMixingEnable, enableCustomMixing);
        BAPE_DSP_P_SET_VARIABLE(userConfig.userConfig, i32UserMixBalance, handle->settings.multiStreamBalance);
        errCode = BDSP_Stage_SetSettings(handle->hMixerStage, &userConfig.userConfig, sizeof(userConfig.userConfig));
    }

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


/*************************************************************************/
static BERR_Code BAPE_DspMixer_P_GetInputStatus(
    BAPE_MixerHandle handle,
    BAPE_Connector input,
    BAPE_MixerInputStatus *pStatus      /* [out] */
    )
{
    BERR_Code errCode;
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_OBJECT_ASSERT(input, BAPE_PathConnector);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    i = BAPE_Mixer_P_FindInputIndex_isrsafe(handle, input);
    if ( i == BAPE_MIXER_INPUT_INDEX_INVALID )
    {
        BDBG_ERR(("Input %p is not connected to mixer %p", (void *)input, (void *)handle));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( BAPE_P_FwMixer_GetDolbyUsageVersion(handle) == BAPE_DolbyMSVersion_eMS12 )
    {
        BDSP_Raaga_MixerDapv2PPStatus dspStatus;
        BAPE_PathConnection * pConnection;
        unsigned taskInputIndex;

        /* Find Connection */
        pConnection = BAPE_Connector_P_GetConnectionToSink_isrsafe(input, &handle->pathNode);
        if ( NULL == pConnection )
        {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        taskInputIndex = pConnection->dspInputIndex;

        /* Input may not be running yet, check that. */
        if ( taskInputIndex == BAPE_DSPMIXER_INPUT_INVALID )
        {
            return BERR_SUCCESS;
        }

        if ( taskInputIndex >= BDSP_AF_P_MAX_IP_FORKS )
        {
            BDBG_ASSERT(taskInputIndex < BDSP_AF_P_MAX_IP_FORKS);
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        if (!handle->taskStarted)
        {
            BDSP_Stage_SetAlgorithm(handle->hMixerStage, BDSP_Algorithm_eMixerDapv2);
        }
        errCode = BDSP_Stage_GetStatus(handle->hMixerStage, &dspStatus, sizeof(dspStatus));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        if ( dspStatus.FadeCtrl_Info[taskInputIndex].ui32StatusValid == 0 )
        {
            pStatus->fade.active = (dspStatus.FadeCtrl_Info[taskInputIndex].ui32FadeActiveStatus == 1);
            pStatus->fade.remaining = dspStatus.FadeCtrl_Info[taskInputIndex].ui32RemainingDuration;
            pStatus->fade.level = dspStatus.FadeCtrl_Info[taskInputIndex].ui32CurrentVolumeLevel;
            BDBG_MSG(("status for fade ctrl idx %d, active %d, remaining %d, level %d", taskInputIndex, pStatus->fade.active, pStatus->fade.remaining, pStatus->fade.level));
            return BERR_SUCCESS;
        }
        return BERR_NOT_AVAILABLE;
    }

    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

/*************************************************************************/
static BERR_Code BAPE_DspMixer_P_GetStatus(
    BAPE_MixerHandle handle,
    BAPE_MixerStatus *pStatus      /* [out] */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if ( BAPE_P_FwMixer_GetDolbyUsageVersion(handle) == BAPE_DolbyMSVersion_eMS12 )
    {
        BDSP_Raaga_MixerDapv2PPStatus dspStatus;
        if (!handle->taskStarted)
        {
            BDSP_Stage_SetAlgorithm(handle->hMixerStage, BDSP_Algorithm_eMixerDapv2);
        }
        errCode = BDSP_Stage_GetStatus(handle->hMixerStage, &dspStatus, sizeof(dspStatus));
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        if ( dspStatus.FadeCtrl_Info[4].ui32StatusValid )
        {
            pStatus->mainDecodeFade.active = (dspStatus.FadeCtrl_Info[4].ui32FadeActiveStatus == 1);
            pStatus->mainDecodeFade.remaining = dspStatus.FadeCtrl_Info[4].ui32RemainingDuration;
            pStatus->mainDecodeFade.level = dspStatus.FadeCtrl_Info[4].ui32CurrentVolumeLevel;
            return BERR_SUCCESS;
        }
        return BERR_NOT_AVAILABLE;
    }

    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static void BAPE_DspMixer_P_EncoderOverflow_isr(void *pParam1, int param2)
{
    BAPE_MixerHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BSTD_UNUSED(param2);

    BDBG_MSG(("Encoder output overflow interrupt"));

    if ( handle->hMuxOutput )
    {
        BAPE_MuxOutput_P_Overflow_isr(handle->hMuxOutput);
    }
}

static void BAPE_DspMixer_P_SampleRateChange_isr(void *pParam1, int param2, unsigned streamSampleRate, unsigned baseSampleRate)
{
    BAPE_MixerHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Mixer);
    BSTD_UNUSED(param2);

    BDBG_MSG(("DSP Mixer Sample Rate changed"));
    BDBG_MSG(("%s: streamSampleRate %d, baseSampleRate %d", BSTD_FUNCTION, streamSampleRate, baseSampleRate));
    if (!BAPE_DspMixer_P_DdrePresentDownstream_isrsafe(handle)) {
        if (handle->settings.mixerSampleRate && BAPE_FMT_P_IsLinearPcm_isrsafe(&handle->pathNode.connectors[0].format)) {
            /* Set mixer sample rate. */
            BAPE_DspMixer_P_SetSampleRate_isr(handle, handle->settings.mixerSampleRate);
        }
        else {
            if ( BAPE_Mixer_P_GetOutputSampleRate_isr(handle) != baseSampleRate ) {
                BAPE_DspMixer_P_SetSampleRate_isr(handle, baseSampleRate);
            }
        }
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
    NOT_SUPPORTED(BAPE_DspMixer_P_GetInputSettings),
    NOT_SUPPORTED(BAPE_DspMixer_P_SetInputSettings),
    NOT_SUPPORTED(BAPE_DspMixer_P_ApplyOutputVolume),
    BAPE_DspMixer_P_SetSettings,
    NOT_SUPPORTED(BAPE_DspMixer_P_ApplyStereoMode),
    BAPE_DspMixer_P_GetInputStatus,
    BAPE_DspMixer_P_GetStatus
};
#endif
