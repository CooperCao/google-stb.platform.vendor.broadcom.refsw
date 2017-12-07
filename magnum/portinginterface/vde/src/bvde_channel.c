/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description: Video Channel Interface for DSP
 *
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvde.h"
#include "bvde_priv.h"
#include "bdsp_raaga.h"
#include "bvde_dsp_utils_priv.h"
#include "bxdm_picture.h"

BDBG_MODULE(bvde_channel);

BDBG_OBJECT_ID(BVDE_Channel);

#define BVDE_DISABLE_DSP 0  /* Enable this to check for CIT errors and avoid starting the DSP */

static void BVDE_Channel_P_SetupDefaults(BVDE_ChannelHandle handle);
static BERR_Code BVDE_Channel_P_Start(BVDE_ChannelHandle handle);
static void BVDE_Channel_P_UnlinkStages(BVDE_ChannelHandle handle);
static void BVDE_Channel_P_Stop(BVDE_ChannelHandle handle);

static const  BAVC_VideoCompressionStd  g_codeclist [] =
{
   BAVC_VideoCompressionStd_eVP6
};

static void BVDE_P_PopulateSupportedBDSPAlgos( 
    BDSP_AlgorithmType type, /* [in] */
    const BAVC_VideoCompressionStd * pSupportedCodecs, /* [in] */
    unsigned numSupportedCodecs, /* [in] */
    const bool * inAlgorithmSupported, /* [in] */
    bool * outAlgorithmSupported /* [out] */
    )
{
    unsigned i;
    BDSP_Algorithm algo;
    bool algoSupported[BDSP_Algorithm_eMax];

    BKNI_Memset(algoSupported, 0, sizeof(algoSupported));
    for ( i = 0; i < numSupportedCodecs; i++ )
    {
        /* Look for a valid entry in the VDE table */
        switch ( type )
        {
            case BDSP_AlgorithmType_eVideoDecode:
                algo = BVDE_P_GetCodecVideoDecode(pSupportedCodecs[i]);
                break;
            default:
                algo = BDSP_Algorithm_eMax;
        }
        if ( algo != BDSP_Algorithm_eMax )
        {
            /* Now that we see VDE supports it, look for a valid entry in the BDSP table */
            if ( inAlgorithmSupported[algo] )
            {
                BDBG_MSG(("%s - %s BAVC codec %d is supported", BSTD_FUNCTION,
                    (type==BDSP_AlgorithmType_eVideoDecode) ? "Decode" : 
                    "Invalid",
                    i));
                algoSupported[algo] = true;
            }
        }
        else
        {
            BDBG_WRN(("%s - requested BAVC codec %d is not enabled by VDE/BDSP", BSTD_FUNCTION, i));
        }
    }

    for ( i = 0; i < BDSP_Algorithm_eMax; i++ )
    {
        outAlgorithmSupported[i] = algoSupported[i];
    }
}

void BVDE_Channel_GetDefaultOpenSettings(
    BVDE_ChannelOpenSettings *pSettings     /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codecCount = sizeof(g_codeclist)/sizeof(BAVC_VideoCompressionStd); 
    pSettings->pCodecList = (BAVC_VideoCompressionStd *) g_codeclist;
    pSettings->resolution = BVDE_Resolution_ePAL;
    pSettings->memPicHandle = NULL;
 }

BERR_Code BVDE_Channel_Open(
    BVDE_Handle deviceHandle,
    unsigned index,
    const BVDE_ChannelOpenSettings *pSettings, 
    BVDE_ChannelHandle *pHandle                 /* [out] */
    )
{
    BERR_Code errCode;
    BVDE_ChannelOpenSettings defaults;
    BVDE_ChannelHandle handle;
    BDSP_TaskCreateSettings dspSettings;
    BDSP_StageCreateSettings stageCreateSettings;

    BDBG_OBJECT_ASSERT(deviceHandle, BVDE_Device);
    
    if ( NULL == pSettings )
    {
        BDBG_WRN (("pSettings is NULL. Using Defaults with memPicHandle as NULL"));
        BVDE_Channel_GetDefaultOpenSettings(&defaults);
        pSettings = &defaults;
    }

    if ( index >= BVDE_MAX_CHANNELS )
    {
        BDBG_ERR(("This chip only supports %u channels. Cannot open channel %u", BVDE_MAX_CHANNELS, index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->channels[index] )
    {
        BDBG_ERR(("Channel %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BVDE_Channel));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(handle, 0, sizeof(BVDE_Channel));

    handle->deviceHandle = deviceHandle;
    handle->index = index;
    BKNI_Snprintf(handle->name, sizeof(handle->name), "Channel %u", index);
    handle->state = BVDE_ChannelState_eStopped;
    BKNI_Memcpy(&handle->settings, pSettings, sizeof(handle->settings));
    BVDE_Channel_GetDefaultStartSettings(&handle->startSettings);
    BDBG_OBJECT_SET(handle, BVDE_Channel);
    BVDE_Channel_P_SetupDefaults(handle);

    BDSP_Task_GetDefaultCreateSettings(deviceHandle->dspContext, &dspSettings);
    errCode = BDSP_Task_Create(deviceHandle->dspContext, &dspSettings, &handle->hTask);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_task_create;
    }

    BDSP_Stage_GetDefaultCreateSettings(deviceHandle->dspContext, BDSP_AlgorithmType_eVideoDecode, &stageCreateSettings);
    
    if ( pSettings->pCodecList )
    {
         BVDE_P_PopulateSupportedBDSPAlgos(
            BDSP_AlgorithmType_eVideoDecode, 
            pSettings->pCodecList, 
            pSettings->codecCount, 
            (const bool *)stageCreateSettings.algorithmSupported, 
            stageCreateSettings.algorithmSupported);
    }

    errCode = BDSP_Stage_Create(deviceHandle->dspContext, &stageCreateSettings, &handle->hPrimaryStage);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stage;
    }


    /* Allocate Memory for Display Buffers and UPBs*/
    errCode = BVDE_Channel_P_AllocateFrameBuffer (handle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_alloc_framebuf;
    }
    
    /* Success */
    *pHandle = handle;
    deviceHandle->channels[index] = handle;
    return BERR_SUCCESS;

err_stage:
err_task_create:
err_alloc_framebuf:
    BVDE_Channel_Close (handle);
        
    return errCode;
}

void BVDE_Channel_Close(
    BVDE_ChannelHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);

    if ( handle->state != BVDE_ChannelState_eStopped )
    {
        BDBG_WRN(("Implicitly stopping channel %u on shutdown.", handle->index));
        BVDE_Channel_Stop(handle);
    }

    /* Cleanup */
    handle->deviceHandle->channels[handle->index] = NULL;

    BVDE_Channel_P_DeAllocateFrameBuffer (handle);

    if ( handle->hTask )
    {
        BDSP_Task_Destroy(handle->hTask);
    } 

    if ( handle->hPrimaryStage )
    {
        BDSP_Stage_Destroy(handle->hPrimaryStage);
    }
    
    
    BDBG_OBJECT_DESTROY(handle, BVDE_Channel);
    BKNI_Free(handle);
}

void BVDE_Channel_GetDefaultStartSettings(
    BVDE_ChannelStartSettings *pSettings    /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->codec = BAVC_VideoCompressionStd_eVP6;
    pSettings->pContextMap = NULL;
}

static BERR_Code BVDE_Channel_P_ValidateDecodeSettings(
    BVDE_ChannelHandle handle,
    const BVDE_ChannelStartSettings *pSettings,
    BVDE_ChannelStartSettings *pOutputSettings
    )
{
    BDSP_Algorithm videoAlgorithm;
    BDSP_AlgorithmInfo algoInfo;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pOutputSettings);
    
    /* Start by copying the existing settings */
    BKNI_Memcpy(pOutputSettings, pSettings, sizeof(BVDE_ChannelStartSettings));

    /* Check for valid input */
    if ( NULL == pSettings->pContextMap )
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
    
    videoAlgorithm = BVDE_P_GetCodecVideoDecode(pSettings->codec);

    /* Check for FW availability */
    errCode = BDSP_GetAlgorithmInfo(handle->deviceHandle->dspHandle, videoAlgorithm, &algoInfo);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
    else if ( !algoInfo.supported )
    {
        BDBG_ERR(("Codec %s (%u) DSP algorithm %s (%u) is not supported.", 
                  BVDE_P_GetCodecName(pSettings->codec), pSettings->codec, 
                  algoInfo.pName, videoAlgorithm));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return BERR_SUCCESS;
}

static BERR_Code BVDE_Channel_P_Start(
    BVDE_ChannelHandle handle
    )
{
    BERR_Code errCode;
    unsigned input;
    const BVDE_ChannelStartSettings *pSettings;
    BDSP_TaskStartSettings taskStartSettings;

	unsigned int            	i=0,  ui32BaseAddr=0,ui32UsedSize=0;
 
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    pSettings = &handle->startSettings;

    BDBG_MSG(("BVDE_Channel_P_Start(%#x) [index %u]", handle, handle->index));

    /* Setup Task Parameters */
    BDSP_Task_GetDefaultStartSettings(handle->hTask, &taskStartSettings);

    taskStartSettings.primaryStage = handle->hPrimaryStage;

    errCode = BDSP_Stage_SetAlgorithm(handle->hPrimaryStage, BVDE_P_GetCodecVideoDecode(handle->startSettings.codec));
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_stages;
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

    taskStartSettings.psVDecoderIPBuffCfg =(BDSP_sVDecoderIPBuffCfg *)BKNI_Malloc(sizeof(BDSP_sVDecoderIPBuffCfg));
    if (NULL == taskStartSettings.psVDecoderIPBuffCfg)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
	BKNI_Memset(taskStartSettings.psVDecoderIPBuffCfg, 0 , sizeof(BDSP_sVDecoderIPBuffCfg));

    ui32BaseAddr = (uint32_t) handle->videoMemory.uFrameOffset;
    ui32UsedSize = 0;
    
    /*sLumaFrameBuffParams and sChromaFrameBuffParams*/
    taskStartSettings.psVDecoderIPBuffCfg->sDisplayFrameBuffParams.ui32NumBuffAvl = handle->videoMemory.totalFrames;
    for(i=0; i<handle->videoMemory.totalFrames ; i++)
    {
        BXDM_Picture *hXDMPicture = NULL;
        BXDM_Picture *hXDMPicture_Cached = NULL;
        hXDMPicture_Cached = &(((BXDM_Picture*) handle->videoMemory.pUpbCached)[i]);

        hXDMPicture =  (BXDM_Picture*) (handle->videoMemory.uUpbOffset + sizeof(BXDM_Picture)*i);
                                    
        /* Reset the entire structure */
        BKNI_Memset(hXDMPicture_Cached, 0, sizeof(BXDM_Picture));

        hXDMPicture_Cached->stBufferInfo.hLuminanceFrameBufferBlock = handle->videoMemory.hFrameBufferBlock;
        hXDMPicture_Cached->stBufferInfo.ulLuminanceFrameBufferBlockOffset = ui32UsedSize;
        
        taskStartSettings.psVDecoderIPBuffCfg->sDisplayFrameBuffParams.sBuffParams[i].sFrameBuffLuma.ui32DramBufferAddress = (uint32_t)(ui32BaseAddr + ui32UsedSize);
        taskStartSettings.psVDecoderIPBuffCfg->sDisplayFrameBuffParams.sBuffParams[i].sFrameBuffLuma.ui32BufferSizeInBytes = handle->videoMemory.lumasize;
        ui32UsedSize += handle->videoMemory.lumasize;

        hXDMPicture_Cached->stBufferInfo.hChrominanceFrameBufferBlock = handle->videoMemory.hFrameBufferBlock;
        hXDMPicture_Cached->stBufferInfo.ulChrominanceFrameBufferBlockOffset = ui32UsedSize;
        
        taskStartSettings.psVDecoderIPBuffCfg->sDisplayFrameBuffParams.sBuffParams[i].sFrameBuffChroma.ui32DramBufferAddress = (uint32_t)(ui32BaseAddr + ui32UsedSize);
        taskStartSettings.psVDecoderIPBuffCfg->sDisplayFrameBuffParams.sBuffParams[i].sFrameBuffChroma.ui32BufferSizeInBytes = handle->videoMemory.chromasize;
        ui32UsedSize += handle->videoMemory.chromasize;

        BMMA_FlushCache(handle->videoMemory.hUpbMemoryBlock, hXDMPicture_Cached, sizeof(BXDM_Picture));

        taskStartSettings.psVDecoderIPBuffCfg->sUPBs[i].ui32DramBufferAddress = (uint32_t)hXDMPicture;
        taskStartSettings.psVDecoderIPBuffCfg->sUPBs[i].ui32BufferSizeInBytes = sizeof(BXDM_Picture);
    }

    taskStartSettings.psVDecoderIPBuffCfg->sReferenceBuffParams.ui32NumBuffAvl = handle->videoMemory.totalRefFrames;

    for(i=0; i<handle->videoMemory.totalRefFrames ; i++)
    {
        taskStartSettings.psVDecoderIPBuffCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffLuma.ui32DramBufferAddress = (uint32_t)(ui32BaseAddr + ui32UsedSize);
        taskStartSettings.psVDecoderIPBuffCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffLuma.ui32BufferSizeInBytes = handle->videoMemory.lumarefsize;
        ui32UsedSize += handle->videoMemory.lumarefsize;

        taskStartSettings.psVDecoderIPBuffCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffChroma.ui32DramBufferAddress = (uint32_t)(ui32BaseAddr + ui32UsedSize);
        taskStartSettings.psVDecoderIPBuffCfg->sReferenceBuffParams.sBuffParams[i].sFrameBuffChroma.ui32BufferSizeInBytes = handle->videoMemory.chromarefsize;
        ui32UsedSize += handle->videoMemory.chromarefsize;
    }      

    /* Destination Details for this Task is not provided. If required we need to add to BDSP_CIT_P_FwStgSrcDstType a new type for video */

    /* Ready */

    /* Apply codec settings */
    errCode = BVDE_Channel_P_ApplyCodecSettings(handle);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        goto err_codec_settings;
    }

    
#if BVDE_DISABLE_DSP
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

    handle->state = BVDE_ChannelState_eStarted;
    BKNI_Free (taskStartSettings.psVDecoderIPBuffCfg);
    return BERR_SUCCESS;


err_start_task:
err_codec_settings:    
    handle->state = BVDE_ChannelState_eStopped;
    BKNI_Free (taskStartSettings.psVDecoderIPBuffCfg);

err_stages:
    BVDE_Channel_P_UnlinkStages (handle);
    return errCode;
}

BERR_Code BVDE_Channel_Start(
    BVDE_ChannelHandle handle,
    const BVDE_ChannelStartSettings *pSettings
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pSettings);

    BDBG_MSG(("BVDE_Channel_Start(%#x) [index %u]", handle, handle->index));

    if ( NULL == handle->deviceHandle->dspContext )
    {
        BDBG_ERR(("DSP Not avaliable"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->state != BVDE_ChannelState_eStopped )
    {
        BDBG_ERR(("Already running, cannot start"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Sanity check settings */
    errCode = BVDE_Channel_P_ValidateDecodeSettings(handle, pSettings, &handle->startSettings);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    errCode = BVDE_Channel_P_Start(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Success */
    return BERR_SUCCESS;
}

static void BVDE_Channel_P_Stop(
    BVDE_ChannelHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);

    if ( NULL == handle->hTask )
    {
        BDBG_MSG(("BVDE_Channel_P_Stop: Channel %u already stopped.", handle->index));
        return;
    }

#if BVDE_DISABLE_DSP
    #warning Task Start is Disabled!
    BDBG_ERR(("NOT STOPPING DSP"));
#else
    BDSP_Task_Stop(handle->hTask);
#endif

    BVDE_Channel_P_UnlinkStages(handle); 
}

void BVDE_Channel_Stop(
    BVDE_ChannelHandle handle
    )
{
    bool unmute = false;

    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);

    BDBG_MSG(("BVDE_Channel_Stop(%#x) [index %u]", handle, handle->index));

    switch ( handle->state )
    {
    case BVDE_ChannelState_eStopped:
        BDBG_WRN(("Channel %u Already Stopped.", handle->index));
        return;
    case BVDE_ChannelState_ePaused:
    case BVDE_ChannelState_eDisabledPaused:
        unmute = true;
        break;
    default:
        break;
    }

    /* Stop the task first */
    handle->state = BVDE_ChannelState_eStopped;
    /* Serialize with critical section prior to stopping the task, guarantees isrs are not updating while we stop (they check the state first) */
    BKNI_EnterCriticalSection();
    BKNI_LeaveCriticalSection();

    BVDE_Channel_P_Stop(handle);
}


BERR_Code BVDE_Channel_DisableForFlush(
    BVDE_ChannelHandle handle
    )
{
    BVDE_ChannelState newState = BVDE_ChannelState_eMax;

    BDBG_MSG(("BVDE_Channel_DisableForFlush(%#x) [index %u]", handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BVDE_ChannelState_eStopped:
        BDBG_ERR(("Channel %u is not started, cannot disable for flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BVDE_ChannelState_eStarted:
        newState = BVDE_ChannelState_eDisabled;
        break;
    case BVDE_ChannelState_ePaused:
        newState = BVDE_ChannelState_eDisabledPaused;
        break;
    case BVDE_ChannelState_eDisabled:
    case BVDE_ChannelState_eDisabledPaused:
        /* No change */
        return BERR_SUCCESS;
    default:
        BDBG_ERR(("Unexpected Channel state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Transition State */
    handle->state = newState;
    BKNI_EnterCriticalSection();
    BKNI_LeaveCriticalSection();
    BVDE_Channel_P_Stop(handle);

    return BERR_SUCCESS;
}

BERR_Code BVDE_Channel_Flush(
    BVDE_ChannelHandle handle
    )
{
    BERR_Code errCode;
    bool paused = false;

    BDBG_MSG(("BVDE_Channel_Flush(%#x) [index %u]", handle, handle->index));

    /* Make sure we're performing a valid state transition */
    switch ( handle->state )
    {
    case BVDE_ChannelState_eStopped:
        BDBG_ERR(("Channel %u is not started, cannot flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BVDE_ChannelState_eStarted:
    case BVDE_ChannelState_ePaused:
        BDBG_ERR(("Channel %u is not disabled, cannot flush.", handle->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    case BVDE_ChannelState_eDisabled:
        break;
    case BVDE_ChannelState_eDisabledPaused:
        paused = true;
        break;
    default:
        BDBG_ERR(("Unexpected Channel state %u", handle->state));
        BDBG_ASSERT(0);
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    errCode = BVDE_Channel_P_Start(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}


void BVDE_Channel_GetSettings(
    BVDE_ChannelHandle handle,
    BVDE_ChannelOpenSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = handle->settings;
}

BERR_Code BVDE_Channel_SetSettings(
    BVDE_ChannelHandle handle,
    const BVDE_ChannelOpenSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pSettings);


    handle->settings = *pSettings;

    if ( handle->state != BVDE_ChannelState_eStopped )
    {
        errCode = BVDE_Channel_P_ApplyCodecSettings(handle);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

void BVDE_Channel_GetDefaultCdbItbConfig(
    BVDE_ChannelHandle handle,
    BAVC_CdbItbConfig *pConfig  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BSTD_UNUSED(handle);
    BKNI_Memset(pConfig, 0, sizeof(BAVC_CdbItbConfig));
    pConfig->Cdb.Length = 256*1024;
    pConfig->Cdb.Alignment = 8; /* Rave expects CDB aligned at 8 */
    pConfig->Itb.Length = 128*1024;
    pConfig->Itb.Alignment = 7; /* Rave expects CDB aligned at 7 */
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    pConfig->Cdb.LittleEndian = true;
#else
    pConfig->Cdb.LittleEndian = false;
#endif
}

static void BVDE_Channel_P_SetupDefaults(BVDE_ChannelHandle handle)
{
    /* BVDE_Channel_P_GetDefaultCodecSettings(handle); */
    BSTD_UNUSED(handle);
}

static void BVDE_Channel_P_UnlinkStages(BVDE_ChannelHandle handle)
{
    if ( handle->hPrimaryStage )
    {
        BDSP_Stage_RemoveAllInputs(handle->hPrimaryStage);
        BDSP_Stage_RemoveAllOutputs(handle->hPrimaryStage);
    }
}

BERR_Code BVDE_Channel_P_ApplyCodecSettings(BVDE_ChannelHandle handle)
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);

    if ( handle->state != BVDE_ChannelState_eStopped )
    {
        switch ( handle->startSettings.codec )
        {
        case BAVC_VideoCompressionStd_eVP6:
            /* return BVDE_Decoder_P_ApplyVP6Settings(handle); */
        default:
            break;
        }
    }

    return BERR_SUCCESS;
}

int ceiling (unsigned num, unsigned den)
{
    return (num%den?num/den + 1:num/den);
}


BERR_Code BVDE_Channel_P_AllocateFrameBuffer(BVDE_ChannelHandle handle)
{
    unsigned size, numframes, numrefframes;
    unsigned height, width, luma, chroma, lumaRef, chromaRef, stripWidth;
    unsigned horzpad = BVDE_TOTAL_HORZIZONTAL_PADDING, vertluma = BVDE_TOTAL_VERTICAL_LUMA_PADDING, vertchroma = BVDE_TOTAL_VERTICAL_CHROMA_PADDING;
    BMMA_Heap_Handle mmaHandle = NULL;

    if (handle->settings.memPicHandle != NULL)
    {
        BDBG_WRN(("Frame buffer from %#x (default %#x)", mmaHandle, handle->deviceHandle->mmaHandle));
        mmaHandle = handle->settings.memPicHandle;
    }
    else
    {
        BDBG_WRN(("Frame buffer from default"));
        mmaHandle = handle->deviceHandle->mmaHandle;
    }

    /* Get the height and width for this resolution */
    width = BVDE_Channel_P_GetResolutionWidth (handle->settings.resolution);
    height = BVDE_Channel_P_GetResolutionHeight (handle->settings.resolution);
    stripWidth = handle->deviceHandle->settings.stripeWidth;

    luma = (ceiling(width,stripWidth)*stripWidth) * (ceiling(height,16)*16);
    chroma = (ceiling(width,stripWidth)*stripWidth) * (ceiling(height,32)*16);

    lumaRef = (ceiling(((width+horzpad)*2),stripWidth)*stripWidth) * (ceiling ((height+vertluma),16)*16);
    chromaRef = (ceiling(((width+horzpad)*2),stripWidth)*stripWidth) * (ceiling ((height+vertchroma),16)*16);

    numframes = BDSP_FWMAX_VIDEO_BUFF_AVAIL;
    numrefframes = BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL;

    /* Store the information in handle */
    handle->videoMemory.lumasize = luma;
    handle->videoMemory.chromasize = chroma;
    handle->videoMemory.lumarefsize = lumaRef;
    handle->videoMemory.chromarefsize = chromaRef;
    handle->videoMemory.totalFrames = numframes;
    handle->videoMemory.totalRefFrames = numrefframes;

    /* Video Memory allocation*/
    size = (luma + chroma) * numframes + (lumaRef + chromaRef) * numrefframes;

    /* Video FW requires the buffers to be 256 byte alligned for DMA */
    handle->videoMemory.hFrameBufferBlock = BMMA_Alloc(mmaHandle,size, 256, 0);
    if ( NULL == handle->videoMemory.hFrameBufferBlock )
    {
        BDBG_ERR(("BVDE_Channel_P_AllocateFrameBuffer: Unable to Allocate memory !"));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    handle->videoMemory.uFrameOffset = BMMA_LockOffset(handle->videoMemory.hFrameBufferBlock);

    /* UPB Allocation */
    size = sizeof(BXDM_Picture) * numframes;
    handle->videoMemory.hUpbMemoryBlock = BMMA_Alloc(mmaHandle,size, 256, 0);
    if(NULL == handle->videoMemory.hUpbMemoryBlock)
    {
        BDBG_ERR(("BVDE_Channel_P_AllocateFrameBuffer: Unable to Allocate memory !"));
        BMMA_Free (handle->videoMemory.hFrameBufferBlock);
        handle->videoMemory.hFrameBufferBlock = NULL;
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    handle->videoMemory.pUpbCached = BMMA_Lock(handle->videoMemory.hUpbMemoryBlock);
    handle->videoMemory.uUpbOffset = BMMA_LockOffset(handle->videoMemory.hUpbMemoryBlock);

    return BERR_SUCCESS;
}

void BVDE_Channel_P_DeAllocateFrameBuffer(BVDE_ChannelHandle handle)
{
    BMMA_Heap_Handle mmaHandle = NULL;

    if (handle->settings.memPicHandle != NULL)
        mmaHandle = handle->settings.memPicHandle;
    else
        mmaHandle = handle->deviceHandle->mmaHandle;

    if (handle->videoMemory.hFrameBufferBlock)
    {
        BMMA_UnlockOffset(handle->videoMemory.hFrameBufferBlock, handle->videoMemory.uFrameOffset);
        BMMA_Free (handle->videoMemory.hFrameBufferBlock);
    }

    if (handle->videoMemory.hUpbMemoryBlock)
    {
        BMMA_Unlock(handle->videoMemory.hUpbMemoryBlock, handle->videoMemory.pUpbCached);
        BMMA_UnlockOffset(handle->videoMemory.hUpbMemoryBlock, handle->videoMemory.uUpbOffset);
        BMMA_Free (handle->videoMemory.hUpbMemoryBlock);    
    }
    

}
unsigned BVDE_Channel_P_GetResolutionWidth (BVDE_Resolution resolution)
{
    switch (resolution)
    {
        case BVDE_Resolution_eFullHD: return 1920;        
        case BVDE_Resolution_eHD: return 1280;    
        case BVDE_Resolution_ePAL: return 720;
        case BVDE_Resolution_eSD: return 720;
        case BVDE_Resolution_eCIF: return 352;
        case BVDE_Resolution_eQCIF: return 176;
        case BVDE_Resolution_eMaxModes: return 720;
    }

    return 0;
}

unsigned BVDE_Channel_P_GetResolutionHeight (BVDE_Resolution resolution)
{
    switch (resolution)
    {
        case BVDE_Resolution_eFullHD: return 1080;        
        case BVDE_Resolution_eHD: return 720;    
        case BVDE_Resolution_ePAL: return 576;
        case BVDE_Resolution_eSD: return 480;
        case BVDE_Resolution_eCIF: return 288;
        case BVDE_Resolution_eQCIF: return 144;
        case BVDE_Resolution_eMaxModes: return 576;
    }

    return 0;    
}


BERR_Code BVDE_Channel_GetPictureCount_isr(
    BVDE_ChannelHandle handle,
    unsigned *pPictureCount
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pPictureCount);

    if ( handle->hTask )
    {    
        return BDSP_Video_GetPictureCount_isr (handle->hTask, pPictureCount);
    }
    else
    {
        *pPictureCount = 0;
        return BERR_SUCCESS;
    }
}

BERR_Code BVDE_Channel_PeekAtPicture_isr(
    BVDE_ChannelHandle handle,
    unsigned index,
    BXDM_Picture **pUnifiedPicture    
    )
{
    BERR_Code ret;
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BSTD_UNUSED(index);
    BDBG_ASSERT(NULL != pUnifiedPicture);


    if ( handle->hTask )
    {    
        ret = BDSP_Video_PeekAtPicture_isr (handle->hTask, index, (uint32_t **)pUnifiedPicture);
        *pUnifiedPicture = &(((BXDM_Picture*) handle->videoMemory.pUpbCached)[((unsigned) *pUnifiedPicture - handle->videoMemory.uUpbOffset)/sizeof(BXDM_Picture)]);
        return ret;
    }
    else
    {
        *pUnifiedPicture = NULL;    /* ?? */
        return BERR_SUCCESS;
    }
}

BERR_Code BVDE_Channel_GetNextPicture_isr(
    BVDE_ChannelHandle handle,
    BXDM_Picture **pUnifiedPicture    
    )
{
    BERR_Code ret;
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pUnifiedPicture);


    if ( handle->hTask )
    {    
        ret = BDSP_Video_GetNextPicture_isr (handle->hTask, (uint32_t **)pUnifiedPicture);
        *pUnifiedPicture = &(((BXDM_Picture*) handle->videoMemory.pUpbCached)[((unsigned) *pUnifiedPicture - handle->videoMemory.uUpbOffset)/sizeof(BXDM_Picture)]);

        return ret;
        
    }
    else
    {
        *pUnifiedPicture = NULL;    /* ?? */
        return BERR_SUCCESS;
    }
}

BERR_Code BVDE_Channel_ReleasePicture_isr(
    BVDE_ChannelHandle handle,
    BXDM_Picture *pUnifiedPicture    
    )
{
    BERR_Code ret;
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pUnifiedPicture);

    if ( handle->hTask )
    {    
        pUnifiedPicture = (BXDM_Picture*) (handle->videoMemory.uUpbOffset + sizeof(BXDM_Picture)*((unsigned) pUnifiedPicture - (unsigned) handle->videoMemory.pUpbCached)/sizeof(BXDM_Picture));
        ret = BDSP_Video_ReleasePicture_isr (handle->hTask, (uint32_t *)pUnifiedPicture);
        return ret;
    }
    else
    {
        return BERR_SUCCESS;
    }
}

BERR_Code BVDE_Channel_GetPictureDropPendingCount_isr(
    BVDE_ChannelHandle handle,
    unsigned *pPictureDropPendingCount
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pPictureDropPendingCount);

    if ( handle->hTask )
    {    
        return BDSP_Video_GetPictureDropPendingCount_isr (handle->hTask, pPictureDropPendingCount);
    }
    else
    {
        *pPictureDropPendingCount = 0;
        return BERR_SUCCESS;
    }
}

BERR_Code BVDE_Channel_RequestPictureDrop_isr(
    BVDE_ChannelHandle handle,
    unsigned *pPictureDropRequestCount
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pPictureDropRequestCount);

    if ( handle->hTask )
    {    
        return BDSP_Video_RequestPictureDrop_isr (handle->hTask, pPictureDropRequestCount);
    }
    else
    {
        *pPictureDropRequestCount = 0;
        return BERR_SUCCESS;
    }
}

BERR_Code BVDE_Channel_DisplayInterruptEvent_isr(
    BVDE_ChannelHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);

    if ( handle->hTask )    
    {
        return BDSP_Video_DisplayInterruptEvent_isr (handle->hTask);
    }
    else
    {
        return BERR_SUCCESS;
    }
}

BERR_Code BVDE_Channel_GetDMInterface(
    BVDE_ChannelHandle handle,
    BXDM_Decoder_Interface *pDMInterface,
    void **pContext
    )
{
    BDBG_OBJECT_ASSERT(handle, BVDE_Channel);
    BDBG_ASSERT(NULL != pDMInterface);
    BDBG_ASSERT(NULL != pContext);

    pDMInterface->getPictureCount_isr = (BXDM_Decoder_GetPictureCount_isr) BVDE_Channel_GetPictureCount_isr;
    pDMInterface->peekAtPicture_isr = (BXDM_Decoder_PeekAtPicture_isr) BVDE_Channel_PeekAtPicture_isr;
    pDMInterface->getNextPicture_isr = (BXDM_Decoder_GetNextPicture_isr) BVDE_Channel_GetNextPicture_isr;
    pDMInterface->releasePicture_isr = (BXDM_Decoder_ReleasePicture_isr) BVDE_Channel_ReleasePicture_isr;
    pDMInterface->getPictureDropPendingCount_isr = (BXDM_Decoder_GetPictureDropPendingCount_isr) BVDE_Channel_GetPictureDropPendingCount_isr;
    pDMInterface->requestPictureDrop_isr = (BXDM_Decoder_RequestPictureDrop_isr) BVDE_Channel_RequestPictureDrop_isr;
    pDMInterface->displayInterruptEvent_isr = (BXDM_Decoder_DisplayInterruptEvent_isr) BVDE_Channel_DisplayInterruptEvent_isr;

   *pContext = handle;

   return BERR_SUCCESS;
}
