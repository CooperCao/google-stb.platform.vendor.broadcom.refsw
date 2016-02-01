/***************************************************************************
 *     Copyright (c) 2006-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bmem.h"
#include "belf.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_misc.h"
#include "bchp_aud_fmm_bf_ctrl.h"

BDBG_MODULE(bape_dsp_priv);

BDBG_OBJECT_ID(BAPE_Dsp);

static BAPE_DspContextHandle BAPE_Dsp_P_TaskIdToContext_isr(BAPE_DspHandle dsp, uint32_t taskId);
static BAPE_DecoderHandle BAPE_Dsp_P_TaskIdToDecoder_isr(BAPE_DspHandle dsp, uint32_t taskId);
static BERR_Code BAPE_Dsp_P_CodecToImg(BAVC_AudioCompressionStd codec, BAPE_Img_Id *pId, bool passthrough);

static void BAPE_Dsp_P_Timer_isr(void *pParam1, int param2);
static void BAPE_Dsp_P_StateChange_isr(void *pParam1, int param2);

static BERR_Code BAPE_Dsp_P_SendMessage(BAPE_DspHandle handle, const BAPE_Message *pMessage, BAPE_Reply *pReply);
static void BAPE_Dsp_P_Ipc_isr(void *pParam1, int param2);
static BERR_Code BAPE_Dsp_P_Boot(BAPE_DspHandle handle);
static void BAPE_Dsp_P_Halt(BAPE_DspHandle dsp);

static void *BAPE_Dsp_P_HostToDsp(BAPE_DspHandle dsp, const void *pAddress);
#if 0
static void *BAPE_Dsp_P_DspToCached(BAPE_DspHandle dsp, const void *pAddress);
static void *BAPE_Dsp_P_DspToUncached(BAPE_DspHandle dsp, const void *pAddress);
#endif

#ifndef BAPE_EMULATION
#include "bchp_int_id_ipi0_intr2.h"
#include "bchp_int_id_ipi1_intr2.h"

static BERR_Code BAPE_Dsp_P_LoadImage(BAPE_DspHandle handle, BAPE_Img_Id id, unsigned extraMemory);
static void      BAPE_Dsp_P_UnloadImage(BAPE_DspHandle handle, BAPE_Img_Id id);
static BERR_Code BAPE_Dsp_P_RelocateImage(BAPE_DspHandle handle, BAPE_Img_Id id, void *pImageBase, unsigned imageSize, unsigned extraMemory);
static void BAPE_Dsp_P_Reset(BAPE_DspHandle handle);
static BERR_Code BAPE_DspContext_P_RequestModules(BAPE_DspContext *handle);
#endif

static void BAPE_DspContext_P_ConfigSourceChannel(BAPE_DspContextHandle handle, BAPE_MixerInput connector, uint32_t sourceChannelId, uint32_t masterChannelId, bool compressed, BAPE_ChannelPair channelPair);

static unsigned BAPE_Dsp_P_BytesToAlign(unsigned byteAlign)
{
    unsigned i;

    for ( i = 0; (i < 32U) && ((1U<<i) < byteAlign); i++ );

    return i;
}

void BAPE_Dsp_P_GetDefaultSettings(
    BAPE_DspSettings *pSettings /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->heapSize = 4*1024*1024;
    pSettings->stackSize = 32*1024;
}

BERR_Code BAPE_Dsp_P_Open(
    BAPE_Handle deviceHandle,
    const BAPE_DspSettings *pSettings,
    BAPE_DspHandle *pHandle     /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BAPE_DspHandle handle;
    BTMR_Settings tmrSettings;
    unsigned i;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    handle = BKNI_Malloc(sizeof(BAPE_Dsp));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_Dsp));
    BDBG_OBJECT_SET(handle, BAPE_Dsp);
    handle->logReadCnt = 0;
    handle->settings = *pSettings;
    handle->deviceHandle = deviceHandle;
    for ( i = 0; i < BAPE_Img_Id_eMax; i++ )
    {
        handle->images[i].moduleId = BAPE_DSP_MODULE_ID_INVALID;
    }

    errCode = BKNI_CreateEvent(&handle->replyEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_event;
    }

#ifndef BAPE_EMULATION
    handle->pInitMemory = BMEM_AllocAligned(deviceHandle->memHandle, (1024*1024)+16, BAPE_Dsp_P_BytesToAlign(1024*1024), 0);
    if ( NULL == handle->pInitMemory )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_init_memory;
    }

    errCode = BINT_CreateCallback(&handle->stateChangeInterrupt, deviceHandle->intHandle, BCHP_INT_ID_IPI0_INTR2_TP0, BAPE_Dsp_P_StateChange_isr, handle, 0);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_state_change;
    }

    errCode = BINT_CreateCallback(&handle->ipcInterrupt, deviceHandle->intHandle, BCHP_INT_ID_IPI0_INTR2_TP1, BAPE_Dsp_P_Ipc_isr, handle, 0);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_ipc;
    }

    /* Load the DSP code */
    errCode = BAPE_Dsp_P_LoadImage(handle, BAPE_Img_Id_eBkernel, pSettings->heapSize + pSettings->stackSize + pSettings->sharedMemorySize);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_load;
    }
#endif

    /* Boot the DSP */
    errCode = BAPE_Dsp_P_Boot(handle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_boot;
    }

    /* Wait for the first tick from the DSP */
    for ( i = 0; i < 5000; i += 100 )
    {
        if ( handle->pIpc->targetTicks != 0 )
        {
            break;
        }
        BKNI_Sleep(100);
    }
    if ( i >= 5000 )
    {
        BDBG_ERR(("DSP failed to boot"));
        goto err_ack;
    }

    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        handle->contexts[i].taskId = BAPE_DSP_TASK_ID_INVALID;
    }

    *pHandle = handle;

#ifndef BAPE_EMULATION
    BINT_EnableCallback(handle->stateChangeInterrupt);
    BINT_EnableCallback(handle->ipcInterrupt);
#endif

    BTMR_GetDefaultTimerSettings(&tmrSettings);
    tmrSettings.type = BTMR_Type_eCountDown;
    tmrSettings.cb_isr = BAPE_Dsp_P_Timer_isr;
    tmrSettings.pParm1 = handle;
    tmrSettings.parm2 = 100*1000;   /* Interrupt every 100ms */
    errCode = BTMR_CreateTimer(handle->deviceHandle->tmrHandle, &handle->timerHandle, &tmrSettings);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_tmr_create;
    }
    errCode = BTMR_StartTimer(handle->timerHandle, tmrSettings.parm2);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_tmr_start;
    }

    return BERR_SUCCESS;

err_tmr_start:
    BTMR_DestroyTimer(handle->timerHandle);
err_tmr_create:
err_ack:
#ifndef BAPE_EMULATION
    BAPE_Dsp_P_Reset(handle);
err_boot:
    BMEM_Free(handle->deviceHandle->memHandle, handle->images[BAPE_Img_Id_eBkernel].pBase);
err_load:
    BINT_DestroyCallback(handle->ipcInterrupt);
err_ipc:
    BINT_DestroyCallback(handle->stateChangeInterrupt);
err_state_change:    
    BMEM_Free(handle->deviceHandle->memHandle, handle->pInitMemory);
err_init_memory:
#endif
    BKNI_DestroyEvent(handle->replyEvent);   
err_event:
    BDBG_OBJECT_DESTROY(handle, BAPE_Dsp);
    BKNI_Free(handle);

    return errCode;
}

/***************************************************************************
Summary:
Close a DSP handle
***************************************************************************/
void BAPE_Dsp_P_Close(
    BAPE_DspHandle handle
    )
{
#ifndef BAPE_EMULATION
    unsigned i;
#endif

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);

    BKNI_EnterCriticalSection();
    BINT_DisableCallback_isr(handle->stateChangeInterrupt);
    BTMR_StopTimer_isr(handle->timerHandle);
    BKNI_LeaveCriticalSection();
    BTMR_DestroyTimer(handle->timerHandle);
    /* Halt the DSP */
#ifdef BAPE_EMULATION
    BAPE_P_DspHalt(handle->deviceHandle);
    BMEM_Free(handle->deviceHandle->memHandle, handle->pIpcBase);
#else
    BAPE_Dsp_P_Halt(handle);
    BINT_DestroyCallback(handle->ipcInterrupt);
    BINT_DestroyCallback(handle->stateChangeInterrupt);

    /* Free all modules */
    for ( i = 0; i < BAPE_Img_Id_eMax; i++ )
    {
        if ( handle->images[i].moduleId != BAPE_DSP_MODULE_ID_INVALID )
        {
            BDBG_MSG(("Releasing module ID %u", handle->images[i].moduleId));
            handle->images[i].usageCount = 0;
            handle->images[i].moduleId = BAPE_DSP_MODULE_ID_INVALID;
            BMEM_Free(handle->deviceHandle->memHandle, handle->images[i].pBase);
            handle->images[i].pBase = NULL;
            BAPE_Dsp_P_UnloadImage(handle, i);
        }
    }

    /* Free the kernel memory */
    if ( handle->images[BAPE_Img_Id_eBkernel].pBase )
    {
        BMEM_Free(handle->deviceHandle->memHandle, handle->images[BAPE_Img_Id_eBkernel].pBase);
        handle->images[BAPE_Img_Id_eBkernel].pBase = NULL;
        BAPE_Dsp_P_UnloadImage(handle, BAPE_Img_Id_eBkernel);
    }

    BKNI_DestroyEvent(handle->replyEvent);
    BMEM_Free(handle->deviceHandle->memHandle, handle->pInitMemory);
#endif
       
    BDBG_OBJECT_DESTROY(handle, BAPE_Dsp);
    BKNI_Free(handle);
}

BERR_Code BAPE_DspContext_P_Create(
    BAPE_DspHandle dspHandle,
    BAPE_DecoderHandle decoder,
    BAPE_DspContextHandle *pHandle
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(dspHandle, BAPE_Dsp);
    BDBG_OBJECT_ASSERT(decoder, BAPE_Decoder);
    BDBG_ASSERT(NULL != pHandle);

    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( dspHandle->contexts[i].decoder == NULL )
        {
            break;
        }
    }

    if ( i == BAPE_CHIP_MAX_DECODERS )
    {
        BDBG_ERR(("Insufficient DSP contexts available."));
        *pHandle = NULL;
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    dspHandle->contexts[i].dsp = dspHandle;
    dspHandle->contexts[i].decoder = decoder;
    dspHandle->contexts[i].taskId = BAPE_DSP_TASK_ID_INVALID;
    dspHandle->contexts[i].index = i;

    *pHandle = &dspHandle->contexts[i];
    return BERR_SUCCESS;
}

void BAPE_DspContext_P_Destroy(
    BAPE_DspContextHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId == BAPE_DSP_TASK_ID_INVALID);

    handle->decoder = NULL;
    handle->dsp = NULL;
}

BERR_Code BAPE_DspContext_P_PrepareTask(
    BAPE_DspContextHandle handle
    )
{
    BERR_Code errCode;
    BAPE_TaskDescriptor *pDescriptor;
    unsigned i,j,num;
    BAPE_MixerInputLink *pLink, *pMaster;
    BAPE_DecoderHandle decoder;
    BAPE_Img_Id imgId;
    BAPE_DspHandle dsp;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId == BAPE_DSP_TASK_ID_INVALID);

    decoder = handle->decoder;
    dsp = handle->dsp;

    pDescriptor = &handle->dsp->pIpc->descriptors[decoder->index];

    BKNI_Memset(pDescriptor, 0, sizeof(BAPE_TaskDescriptor));

    handle->pTsmSettingsBegin = &handle->dsp->pIpc->tsmSettingsSequence[decoder->index][0];
    handle->pTsmSettings = &handle->dsp->pIpc->tsmSettings[decoder->index];
    handle->pTsmSettingsEnd = &handle->dsp->pIpc->tsmSettingsSequence[decoder->index][1];
    BKNI_EnterCriticalSection();
    BAPE_DspContext_P_SetTsmSettings_isr(handle, &decoder->tsmSettings);
    BKNI_LeaveCriticalSection();
    pDescriptor->pTsmSettingsBegin = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->tsmSettingsSequence[decoder->index][0]);
    pDescriptor->pTsmSettings = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->tsmSettings[decoder->index]);
    pDescriptor->pTsmSettingsEnd = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->tsmSettingsSequence[decoder->index][1]);
    BDBG_ASSERT(NULL != pDescriptor->pTsmSettings);
    handle->pTsmStatusBegin = &handle->dsp->pIpc->tsmStatusSequence[decoder->index][0];
    *handle->pTsmStatusBegin = 0;
    handle->pTsmStatus = &handle->dsp->pIpc->tsmStatus[decoder->index];
    handle->pTsmStatusEnd = &handle->dsp->pIpc->tsmStatusSequence[decoder->index][1];
    *handle->pTsmStatusEnd = 0;
    pDescriptor->pTsmStatusBegin = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->tsmStatusSequence[decoder->index][0]);
    pDescriptor->pTsmStatus = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->tsmStatus[decoder->index]);
    pDescriptor->pTsmStatusEnd = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->tsmStatusSequence[decoder->index][1]);
    BDBG_ASSERT(NULL != pDescriptor->pTsmStatus);
    BKNI_Memset(&handle->dsp->pIpc->tsmStatus[decoder->index], 0, sizeof(BAPE_DecoderTsmStatus));
    handle->pFmmSampleRate = &handle->dsp->pIpc->fmmSampleRate[decoder->index];
    *handle->pFmmSampleRate = 0;    /* Mark sample rate invalid before start */
    pDescriptor->pSampleRateBuffer = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->fmmSampleRate[decoder->index]);
    handle->pPlaybackStcValid = &handle->dsp->pIpc->playbackStcValid[decoder->index];
    *handle->pPlaybackStcValid = 0;
    pDescriptor->pPlaybackStcValid = BAPE_Dsp_P_HostToDsp(handle->dsp, &handle->dsp->pIpc->playbackStcValid[decoder->index]);
    
    BDBG_ASSERT(NULL != pDescriptor->pSampleRateBuffer);    
    
#ifndef BAPE_EMULATION
    /* Load modules */
    errCode = BAPE_DspContext_P_RequestModules(handle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }
#endif

    /* STC Reg */
    if ( decoder->startSettings.stcIndex >= BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END )
    {
        BDBG_ERR(("Invalid STC index %u (max is %u)", decoder->startSettings.stcIndex, BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_END));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    pDescriptor->stcReg = BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_BASE + (decoder->startSettings.stcIndex * (BCHP_AUD_FMM_MISC_STC_UPPERi_ARRAY_ELEMENT_SIZE/8));
    /* RAVE or RBUF */
    if ( decoder->startSettings.pContextMap )
    {
        pDescriptor->inputRaveBase = decoder->startSettings.pContextMap->CDB_Read;
        pDescriptor->inputRbufBase = 0;
    }
    else
    {
        BDBG_ERR(("Ringbuffer input not supported"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    /* Decode info */
    BAPE_Dsp_P_CodecToImg(decoder->startSettings.codec, &imgId, decoder->passthrough);    /* will always succeed if request modules succeeded */
#ifdef BAPE_EMULATION
    pDescriptor->decodeModuleId = imgId-1;
#else
    pDescriptor->decodeModuleId = handle->dsp->images[imgId].moduleId;
#endif
    pDescriptor->decodeCodec = decoder->startSettings.codec;
    pDescriptor->decodeStreamType = decoder->startSettings.streamType;
    pMaster = NULL;
    for ( num = 0, pLink = BLST_S_FIRST(&decoder->stereoConnector.mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        if ( pLink->sourceChannelId[0] != 0xff )
        {
            num++;
            if ( NULL == pMaster )
            {
                pMaster = pLink;
            }
        }
    }

    i = 0;
    for ( pLink = BLST_S_FIRST(&decoder->stereoConnector.mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        if ( pLink->sourceChannelId[0] != 0xff )
        {
            pDescriptor->outputs[i].inputBranchId = 0xffffffff;
            pDescriptor->outputs[i].inputStageId = 0;   /* stereo */
            pDescriptor->outputs[i].master = (pLink==pMaster)?true:false;
            pDescriptor->outputs[i].numChannelPairs = 1;
            pDescriptor->outputs[i].rbufBase[0] = BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(pLink->sourceChannelId[0]);
            pDescriptor->outputs[i].gateReg[0] = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + ((uint32_t)pLink->sourceChannelId[0]*(BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_ELEMENT_SIZE/8));
            pDescriptor->outputs[i].ppmReg[0] = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + ((uint32_t)pLink->sourceChannelId[0]*(BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_1-BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0));

            BAPE_DspContext_P_ConfigSourceChannel(handle, &decoder->stereoConnector, pLink->sourceChannelId[0], (num > 1)?pMaster->sourceChannelId[0]:0xff, false, 0);
            i++;
        }
    }
    pMaster = NULL;
    for ( num = 0, pLink = BLST_S_FIRST(&decoder->multichannelConnector.mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        if ( pLink->sourceChannelId[0] != 0xff )
        {
            num++;
            if ( NULL == pMaster )
            {
                pMaster = pLink;
            }
        }
    }
    for ( pLink = BLST_S_FIRST(&decoder->multichannelConnector.mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        for ( j = 0; j < decoder->multichannelConnector.numChannelPairs; j++ )
        {
            if ( pLink->sourceChannelId[j] != 0xff )
            {
                pDescriptor->outputs[i].inputBranchId = 0xffffffff;
                pDescriptor->outputs[i].inputStageId = 1;   /* multichannel */
                pDescriptor->outputs[i].master = (pLink==pMaster)?true:false;
                pDescriptor->outputs[i].numChannelPairs = decoder->multichannelConnector.numChannelPairs;
                pDescriptor->outputs[i].rbufBase[j] = BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(pLink->sourceChannelId[j]);
                pDescriptor->outputs[i].gateReg[j] = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + ((uint32_t)pLink->sourceChannelId[j]*(BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_ELEMENT_SIZE/8));
                pDescriptor->outputs[i].ppmReg[j] = BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0 + ((uint32_t)pLink->sourceChannelId[j]*(BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_1-BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0));

                BAPE_DspContext_P_ConfigSourceChannel(handle, &decoder->multichannelConnector, pLink->sourceChannelId[j], (num > 1)?pMaster->sourceChannelId[j]:0xff, false, j);
            }
        }
        i++;
    }
    pMaster = NULL;
    for ( num = 0, pLink = BLST_S_FIRST(&decoder->compressedConnector.mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        if ( pLink->sourceChannelId[0] != 0xff )
        {
            num++;
            if ( NULL == pMaster )
            {
                pMaster = pLink;
            }
        }
    }
    for ( pLink = BLST_S_FIRST(&decoder->compressedConnector.mixerList);
          pLink != NULL;
          pLink = BLST_S_NEXT(pLink, node) )
    {
        if ( pLink->sourceChannelId[0] != 0xff )
        {
            pDescriptor->outputs[i].inputBranchId = 0xffffffff;
            pDescriptor->outputs[i].inputStageId = 2;   /* compressed */
            pDescriptor->outputs[i].master = (pLink==pMaster)?true:false;
            pDescriptor->outputs[i].numChannelPairs = 1;
            pDescriptor->outputs[i].rbufBase[0] = BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(pLink->sourceChannelId[0]);
            pDescriptor->outputs[i].gateReg[0] = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_BASE + ((uint32_t)pLink->sourceChannelId[0]*(BCHP_AUD_FMM_BF_CTRL_SOURCECH_CTRLi_ARRAY_ELEMENT_SIZE/8));
            pDescriptor->outputs[i].ppmReg[0] = 0;

            BAPE_DspContext_P_ConfigSourceChannel(handle, &decoder->compressedConnector, pLink->sourceChannelId[0], (num > 1)?pMaster->sourceChannelId[0]:0xff, true, 0);
            i++;
        }
    }
    pDescriptor->numDecodeOutputs = i;

    /* Setup task settings */
    pDescriptor->taskSettings.dualMonoMode = decoder->settings.dualMonoMode;
    pDescriptor->taskSettings.outputMode = decoder->settings.outputMode;
    pDescriptor->taskSettings.outputLfe = decoder->settings.outputLfe;

    /* Form message */
    BKNI_Memset(&dsp->msg, 0, sizeof(BAPE_Message));
    BKNI_Memset(&dsp->reply, 0, sizeof(BAPE_Reply));
    dsp->msg.id = BAPE_MessageId_eCreateTask;
    dsp->msg.data.createTask.descriptor = *pDescriptor;
    handle->pDescriptor = pDescriptor;
    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &dsp->msg, &dsp->reply);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    BDBG_MSG(("Allocated DSP Task ID %u", dsp->reply.data.createTask.taskId));

    handle->taskId = dsp->reply.data.createTask.taskId;

    return BERR_SUCCESS;
}

BERR_Code BAPE_DspContext_P_StartTask(
    BAPE_DspContextHandle handle
    )
{
    BERR_Code errCode;
    BAPE_Message message;
    BAPE_Reply reply;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId != BAPE_DSP_TASK_ID_INVALID);

    /* TODO: Set Settings first? */

    message.id = BAPE_MessageId_eStartTask;
    message.data.startTask.taskId = handle->taskId;
    *handle->pPlaybackStcValid = false;

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void BAPE_DspContext_P_StopTask(
    BAPE_DspContextHandle handle
    )
{
    BERR_Code errCode;
    BAPE_Message message;
    BAPE_Reply reply;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId != BAPE_DSP_TASK_ID_INVALID);

    if ( !handle->dsp->watchdogAsserted )
    {
        message.id = BAPE_MessageId_eStopTask;
        message.data.stopTask.taskId = handle->taskId;
    
        errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
}

void BAPE_DspContext_P_CleanupTask(
    BAPE_DspContextHandle handle
    )
{
    BERR_Code errCode;
    BAPE_Message message;
    BAPE_Reply reply;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId != BAPE_DSP_TASK_ID_INVALID);

    /* If we're cleaning up after a watchdog don't throw errors */
    if ( !handle->dsp->watchdogAsserted )
    {
        message.id = BAPE_MessageId_eDestroyTask;
        message.data.destroyTask.taskId = handle->taskId;
    
        errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
        }    
    }

    handle->taskId = BAPE_DSP_TASK_ID_INVALID;
}

BERR_Code BAPE_DspContext_P_PauseTask(
    BAPE_DspContextHandle handle
    )
{
    BERR_Code errCode;
    BAPE_Message message;
    BAPE_Reply reply;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId != BAPE_DSP_TASK_ID_INVALID);

    message.id = BAPE_MessageId_ePauseTask;
    message.data.pauseTask.taskId = handle->taskId;
    message.data.pauseTask.pauseEnabled = 1;

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_DspContext_P_ResumeTask(
    BAPE_DspContextHandle handle
    )
{
    BERR_Code errCode;
    BAPE_Message message;
    BAPE_Reply reply;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId != BAPE_DSP_TASK_ID_INVALID);

    message.id = BAPE_MessageId_ePauseTask;
    message.data.pauseTask.taskId = handle->taskId;
    message.data.pauseTask.pauseEnabled = 0;

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_DspContext_P_AdvanceTask(
    BAPE_DspContextHandle handle,
    unsigned ms
    )
{
    BERR_Code errCode;
    BAPE_Message message;
    BAPE_Reply reply;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_ASSERT(handle->taskId != BAPE_DSP_TASK_ID_INVALID);

    message.id = BAPE_MessageId_eAdvanceTask;
    message.data.advanceTask.taskId = handle->taskId;
    message.data.advanceTask.ptsUnits = ms * 45;        /* Convert to 45kHz units */

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_DspContext_P_SetTsmSettings_isr(
    BAPE_DspContextHandle handle,
    const BAPE_DecoderTsmSettings *pSettings
    )
{
    uint32_t sequence;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    sequence = *handle->pTsmSettingsBegin + 1;
    *handle->pTsmSettingsBegin = sequence;
    handle->pTsmSettings->tsmEnabled = pSettings->tsmEnabled;
    handle->pTsmSettings->astmEnabled = pSettings->astmEnabled;
    handle->pTsmSettings->playback = pSettings->playback;
    handle->pTsmSettings->ptsOffset = (BAPE_CHIP_MAX_PATH_DELAY*45) + pSettings->ptsOffset;
    handle->pTsmSettings->trackingRange.plus = pSettings->thresholds.grossAdjustment*45;
    handle->pTsmSettings->trackingRange.minus = pSettings->thresholds.grossAdjustment*45;
    handle->pTsmSettings->inSyncRange.plus = 2*45;  /* We use 2ms for PPM thresholds */
    handle->pTsmSettings->inSyncRange.minus = 2*45;
    handle->pTsmSettings->startRange.plus = 1*45;
    handle->pTsmSettings->startRange.minus = 1*45;
    handle->pTsmSettings->discardThreshold.plus = pSettings->thresholds.discard*45;
    handle->pTsmSettings->discardThreshold.minus = pSettings->thresholds.discard*45;
    /* TODO: Audio Master Mode, transition threshold(?) */
    *handle->pTsmSettingsEnd = sequence;
    return BERR_SUCCESS;
}

BERR_Code BAPE_DspContext_P_SetStcValid_isr(
    BAPE_DspContextHandle handle
    )
{
    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    *handle->pPlaybackStcValid = true;
    return BERR_SUCCESS;
}

BERR_Code BAPE_DspContext_P_GetTsmStatus_isr(
    BAPE_DspContextHandle handle,
    BAPE_DecoderTsmStatus *pStatus      /* [out] */
    )
{
    unsigned timeout = 1000;
    uint32_t s1,s2,s3;
    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    /* Try up to 1ms to read status */
    while ( timeout > 0 && !handle->dsp->watchdogAsserted )
    {
        s1 = *handle->pTsmStatusBegin;
        pStatus->ptsInfo = handle->pTsmStatus->ptsInfo;
        pStatus->ptsStcDifference = handle->pTsmStatus->ptsStcDifference;
        s2 = *handle->pTsmStatusEnd;
        s3 = *handle->pTsmStatusBegin;
        if ( s1 == s2 && s2 == s3 )
        {
            return BERR_SUCCESS;
        }
        timeout -= 10;
        BKNI_Delay(10);
    }

    return BERR_TRACE(BERR_TIMEOUT);
}

BERR_Code BAPE_DspContext_P_SetSettings(
    BAPE_DspContextHandle handle,
    const BAPE_DecoderSettings *pSettings
    )
{
    BAPE_Message message;
    BAPE_Reply reply;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);

    message.id = BAPE_MessageId_eSetTaskSettings;
    message.data.setTaskSettings.taskId = handle->taskId;
    message.data.setTaskSettings.settings.dualMonoMode = pSettings->dualMonoMode;
    message.data.setTaskSettings.settings.outputMode = pSettings->outputMode;
    message.data.setTaskSettings.settings.outputLfe = pSettings->outputLfe;

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}

void BAPE_DspContext_P_GetStatus(
    BAPE_DspContextHandle handle,
    BAPE_DecoderStatus *pStatus      /* [out] */
    )
{
    BAPE_Message message;
    BAPE_Reply reply;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pStatus);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);

    BKNI_EnterCriticalSection();
    BAPE_DspContext_P_GetTsmStatus_isr(handle, &pStatus->tsmStatus);
    BKNI_LeaveCriticalSection();

    message.id = BAPE_MessageId_eGetTaskStatus;
    message.data.getTaskStatus.taskId = handle->taskId;

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return;
    }

    pStatus->sampleRate = BAPE_P_GetSampleRateEnum_isrsafe(reply.data.getTaskStatus.status.sampleRate);
    pStatus->framesDecoded = reply.data.getTaskStatus.status.framesDecoded;
    pStatus->frameErrors = reply.data.getTaskStatus.status.frameErrors;
    pStatus->dummyFrames = reply.data.getTaskStatus.status.dummyFrames;

    message.id = BAPE_MessageId_eGetStageStatus;
    message.data.getStageStatus.taskId = handle->taskId;
    message.data.getStageStatus.branchId = 0xffffffff;
    message.data.getStageStatus.stageId = 0;
    message.data.getStageStatus.statusSize = 0;
    message.data.getStageStatus.pStatus = BAPE_Dsp_P_HostToDsp(handle->dsp, handle->dsp->pIpc->dataBuffer);

    switch ( handle->decoder->startSettings.codec )
    {
    case BAVC_AudioCompressionStd_eMpegL1:
    case BAVC_AudioCompressionStd_eMpegL2:
    case BAVC_AudioCompressionStd_eMpegL3:
        message.data.getStageStatus.statusSize = sizeof(BAPE_MpegStatus);
        break;
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eAc3Plus:
        message.data.getStageStatus.statusSize = sizeof(BAPE_Ac3Status);
        break;
    case BAVC_AudioCompressionStd_eAacLoas:
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        message.data.getStageStatus.statusSize = sizeof(BAPE_AacStatus);
        break;
    case BAVC_AudioCompressionStd_eWmaStd:
        message.data.getStageStatus.statusSize = sizeof(BAPE_WmaStatus);
        break;
    default:
        return;
    }

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);        
        return;
    }

    switch ( handle->decoder->startSettings.codec )
    {
    case BAVC_AudioCompressionStd_eMpegL1:
    case BAVC_AudioCompressionStd_eMpegL2:
    case BAVC_AudioCompressionStd_eMpegL3:
        BKNI_Memcpy(&pStatus->codecStatus.mpeg, handle->dsp->pIpc->dataBuffer, sizeof(pStatus->codecStatus.mpeg));
        pStatus->codecStatus.mpeg.originalSampleRate = BAPE_P_GetSampleRateEnum_isrsafe(pStatus->codecStatus.mpeg.originalSampleRate);
        break;
    case BAVC_AudioCompressionStd_eAc3:
    case BAVC_AudioCompressionStd_eAc3Plus:
        BKNI_Memcpy(&pStatus->codecStatus.ac3, handle->dsp->pIpc->dataBuffer, sizeof(pStatus->codecStatus.ac3));
        pStatus->codecStatus.ac3.originalSampleRate = BAPE_P_GetSampleRateEnum_isrsafe(pStatus->codecStatus.ac3.originalSampleRate);
        break;
    case BAVC_AudioCompressionStd_eAacLoas:
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacPlusLoas:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        BKNI_Memcpy(&pStatus->codecStatus.aac, handle->dsp->pIpc->dataBuffer, sizeof(pStatus->codecStatus.aac));
        pStatus->codecStatus.aac.originalSampleRate = BAPE_P_GetSampleRateEnum_isrsafe(pStatus->codecStatus.aac.originalSampleRate);
        break;
    case BAVC_AudioCompressionStd_eWmaStd:
        BKNI_Memcpy(&pStatus->codecStatus.wma, handle->dsp->pIpc->dataBuffer, sizeof(pStatus->codecStatus.wma));
        pStatus->codecStatus.wma.originalSampleRate = BAPE_P_GetSampleRateEnum_isrsafe(pStatus->codecStatus.wma.originalSampleRate);
        break;
    default:
        break;
    }
    
    return;
}

BERR_Code BAPE_DspContext_P_SetCodecSettings(
    BAPE_DspContextHandle handle,
    const BAPE_DecoderCodecSettings *pSettings
    )
{
    BAPE_Message message;
    BAPE_Reply reply;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != handle);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);

    message.id = BAPE_MessageId_eSetStageSettings;
    message.data.setStageSettings.taskId = handle->taskId;
    message.data.setStageSettings.branchId = 0xffffffff;
    message.data.setStageSettings.stageId = 0;
    message.data.setStageSettings.pSettings = BAPE_Dsp_P_HostToDsp(handle->dsp, handle->dsp->pIpc->dataBuffer);
    message.data.setStageSettings.settingsSize = 0;

    switch ( pSettings->codec )
    {
    case BAVC_AudioCompressionStd_eAc3:
        BKNI_Memcpy(handle->dsp->pIpc->dataBuffer, &pSettings->codecSettings.ac3, sizeof(pSettings->codecSettings.ac3));
        message.data.setStageSettings.settingsSize = sizeof(pSettings->codecSettings.ac3);
        break;
    case BAVC_AudioCompressionStd_eAc3Plus:
        BKNI_Memcpy(handle->dsp->pIpc->dataBuffer, &pSettings->codecSettings.ac3Plus, sizeof(pSettings->codecSettings.ac3Plus));
        message.data.setStageSettings.settingsSize = sizeof(pSettings->codecSettings.ac3Plus);
        break;
    case BAVC_AudioCompressionStd_eAacAdts:
    case BAVC_AudioCompressionStd_eAacLoas:
        BKNI_Memcpy(handle->dsp->pIpc->dataBuffer, &pSettings->codecSettings.aac, sizeof(pSettings->codecSettings.aac));
        message.data.setStageSettings.settingsSize = sizeof(pSettings->codecSettings.aac);
        break;
    case BAVC_AudioCompressionStd_eAacPlusLoas:
    case BAVC_AudioCompressionStd_eAacPlusAdts:
        BKNI_Memcpy(handle->dsp->pIpc->dataBuffer, &pSettings->codecSettings.aacPlus, sizeof(pSettings->codecSettings.aacPlus));
        message.data.setStageSettings.settingsSize = sizeof(pSettings->codecSettings.aacPlus);
        break;
    default:
        /* No settings for this codec */
        return BERR_SUCCESS;
    }

    errCode = BAPE_Dsp_P_SendMessage(handle->dsp, &message, &reply);
    if ( errCode )
    {
        return BERR_TRACE(errCode);        
    }

    return BERR_SUCCESS;
}

#ifndef BAPE_EMULATION
#if BAPE_FILE_IMAGE
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

static BERR_Code BAPE_Dsp_P_LoadImage(BAPE_DspHandle handle, BAPE_Img_Id id, unsigned extraMemory)
{
    const char *fname=NULL;
    int fd;
    struct stat st;
    void *pMem;
    BERR_Code rc;

    fname = BAPE_Dsp_P_ImgIdToName(id);
    if(fname==NULL) {return BERR_TRACE(BERR_NOT_SUPPORTED);}

    fd = open(fname, O_RDONLY);
    if ( fd < 0 ) { 
        BDBG_ERR(("can't open %s", fname));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( fstat(fd, &st)!=0 ) { return BERR_TRACE(BERR_NOT_SUPPORTED);}
    pMem = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if ( pMem == NULL || pMem == MAP_FAILED ) { return BERR_TRACE(BERR_NOT_SUPPORTED);}

    handle->images[id].elf = NULL;
    rc=BAPE_Dsp_P_RelocateImage(handle, id, pMem, st.st_size, extraMemory);
    handle->images[id].pImageBase = pMem;
    handle->images[id].imageSize = st.st_size;
    handle->images[id].imageParam = fd;
    if ( NULL == handle->images[id].elf )
    {
        BAPE_Dsp_P_UnloadImage(handle, id);
    }
    if(rc!=BERR_SUCCESS) { return BERR_TRACE(rc);}

    return BERR_SUCCESS;
}

static void BAPE_Dsp_P_UnloadImage(BAPE_DspHandle handle, BAPE_Img_Id id)
{
    if ( handle->images[id].pImageBase )
    {
        munmap(handle->images[id].pImageBase, handle->images[id].imageSize);
        close(handle->images[id].imageParam);
        handle->images[id].pImageBase = NULL;
        handle->images[id].imageSize = 0;
        handle->images[id].imageParam = -1;
        if ( handle->images[id].elf )
        {
            BELF_CloseImage(handle->images[id].elf);
            handle->images[id].elf = NULL;
        }
    }
}
#else /* BAPE_FILE_IMAGE */
static BERR_Code BAPE_Dsp_P_LoadImage(BAPE_DspHandle handle, BAPE_Img_Id id, unsigned extraMemory)
{
    void *image;
    BERR_Code rc;
    const void *data;
    const uint32_t *header;
    unsigned blockSize;
    unsigned blockCount;
    unsigned imageSize;
    unsigned i;
    void *temp, *cached;
    uint8_t *dest;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);
    BDBG_ASSERT(id < BAPE_Img_Id_eMax);
    BDBG_ASSERT(NULL != handle->settings.pImageInterface);
    BDBG_ASSERT(NULL != handle->settings.pImageInterface->open);
    BDBG_ASSERT(NULL != handle->settings.pImageInterface->next);
    BDBG_ASSERT(NULL != handle->settings.pImageInterface->close);

    rc = handle->settings.pImageInterface->open(handle->settings.pImageContext, &image, id);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_open; }
    rc = handle->settings.pImageInterface->next(image, 0, &data, sizeof(uint32_t)*3); 
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_header; }
    header = data;
    blockSize = header[0];
    blockCount = header[1];
    imageSize = header[2];
    BDBG_MSG(("Allocating temporary storage buffer of %u bytes", imageSize));
    temp = BMEM_Alloc(handle->deviceHandle->memHandle, imageSize);
    if(!temp) {rc=BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);goto err_malloc; }

    BMEM_ConvertAddressToCached(handle->deviceHandle->memHandle, temp, &cached);
    BKNI_Memset(cached, 0x0, imageSize);
    for(dest=cached,i=1;i<=blockCount;i++) {
        unsigned size;

        if(i!=blockCount) { 
            size=blockSize;
        } else {
            size=imageSize%blockSize;
        }
        rc = handle->settings.pImageInterface->next(image, i, &data, size);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_next; }
        BKNI_Memcpy(dest, data, size);
        dest += size;
    }

    rc=BAPE_Dsp_P_RelocateImage(handle, id, cached, imageSize, extraMemory);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_relocate; }
    if ( NULL == handle->images[id].elf )
    {
        BMEM_Free(handle->deviceHandle->memHandle, temp);
    }
    else
    {
        handle->images[id].pImageBase = temp;
        handle->images[id].imageSize = imageSize;
    }
    handle->settings.pImageInterface->close(image);
    return BERR_SUCCESS;
err_relocate:
err_next:
    BMEM_Free(handle->deviceHandle->memHandle, temp);
err_malloc:
err_header:
    handle->settings.pImageInterface->close(image);
err_open:
    return rc;
}

static void BAPE_Dsp_P_UnloadImage(BAPE_DspHandle handle, BAPE_Img_Id id)
{
    if ( handle->images[id].pImageBase )
    {
        BMEM_Free(handle->deviceHandle->memHandle, handle->images[id].pImageBase);
        handle->images[id].pImageBase = NULL;
        handle->images[id].imageSize = 0;
        if ( handle->images[id].elf )
        {
            BELF_CloseImage(handle->images[id].elf);
            handle->images[id].elf = NULL;
        }
    }
}
#endif /* BAPE_FILE_IMAGE */

static BERR_Code BAPE_Dsp_P_RelocateImage(BAPE_DspHandle handle, BAPE_Img_Id id, void *pImage, unsigned imageSize, unsigned extraMemory)
{
    BELF_SectionDetails sectionDetails;
    BELF_ImageSettings imageSettings;
    BELF_ImageDetails imageDetails;
    BELF_ImageHandle image;
    BERR_Code rc;
    uint32_t offset;
    unsigned i;
    void *block;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);
    BDBG_ASSERT(NULL != pImage);
    BDBG_ASSERT(imageSize > 0);

    handle->images[id].size = 0;
    handle->images[id].elf = NULL;

    BELF_GetDefaultImageSettings(&imageSettings);
    imageSettings.pImageData = pImage;
    imageSettings.imageLength = imageSize;
    rc = BELF_OpenImage(&imageSettings, &image);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_elf_open; }
    BELF_GetImageDetails(image, &imageDetails);

    handle->images[id].pTextBase = NULL;
    handle->images[id].textSize = 0;

    #if BDBG_DEBUG_BUILD
    BDBG_MSG(("Sections"));
    /* Print the section info */
    for ( i = 1; i < imageDetails.numSections; i++ ) {
        rc = BELF_LookupSectionByIndex(image, i, &sectionDetails);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_section; }
        BDBG_MSG(("\t%02u: '%s' %u bytes address %#x", i, sectionDetails.pName, sectionDetails.size, (unsigned)sectionDetails.pAddress));
    }
    #endif

    if ( !imageDetails.hasRelocations ) { rc = BERR_TRACE(BERR_NOT_SUPPORTED); /* Only able to load relocatable images */ goto err_relocations; }

    BDBG_MSG(("Image requires %u bytes of space aligned to %u bytes to execute (%u extra bytes)", imageDetails.memoryRequired, imageDetails.memoryAlignment, extraMemory));
    handle->images[id].size = imageDetails.memoryRequired + (imageDetails.memoryAlignment-1);
    handle->images[id].size = handle->images[id].size - (handle->images[id].size % imageDetails.memoryAlignment);
    handle->images[id].pBase = BMEM_AllocAligned(handle->deviceHandle->memHandle, handle->images[id].size + extraMemory, BAPE_Dsp_P_BytesToAlign(imageDetails.memoryAlignment), 0);
    if ( NULL == handle->images[id].pBase )
    {
        rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto err_malloc;
    }
    
    BMEM_ConvertAddressToCached(handle->deviceHandle->memHandle, handle->images[id].pBase, &block);
    rc = BMEM_ConvertAddressToOffset(handle->deviceHandle->memHandle, handle->images[id].pBase, &offset);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_offset; }

    handle->images[id].pEntry = NULL;
    rc = BELF_LoadImage(image, block, 0x80000000|offset, &handle->images[id].pEntry);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_load; }
    BDBG_MSG(("Entry %#x", (uint32_t)handle->images[id].pEntry-(uint32_t)block+offset));
    for ( i = 1; i < imageDetails.numSections; i++ ) 
    {
        rc = BELF_LookupSectionByIndex(image, i, &sectionDetails);
        if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc);goto err_text; }
        if ( !BKNI_Memcmp(sectionDetails.pName, ".text", 6) )
        {
            BDBG_MSG((".text section base %#x length %d bytes", ((uint32_t)sectionDetails.pAddress-(uint32_t)block)+offset, sectionDetails.size));
            handle->images[id].pTextBase = sectionDetails.pAddress;
            /* This address is relative to the loaded address */
            handle->images[id].pTextBaseUncached = (void *)(((uint32_t)sectionDetails.pAddress - (uint32_t)block) + (uint32_t)handle->images[id].pBase);
            handle->images[id].textSize = sectionDetails.size;
        }
    }

    if ( NULL == handle->images[id].pTextBase || 0 == handle->images[id].textSize )
    {
        BDBG_ERR(("Invalid .text section"));
        goto err_text;
    }

    /* Flush data cache for executable section only */
    BMEM_FlushCache(handle->deviceHandle->memHandle, handle->images[id].pTextBase, handle->images[id].textSize);

    /* Make entry relative to uncached address */
    handle->images[id].pEntry = (void *)(((unsigned)handle->images[id].pEntry - (unsigned)block)+(unsigned)handle->images[id].pBase);

    #if BDBG_DEBUG_BUILD
    if ( imageDetails.numSymbols > 0 )
    {
        BDBG_MSG(("ELF image has symbols.  Keeping image in memory for debug."));
        handle->images[id].elf = image;
    }
    else
    #endif
    {
        BELF_CloseImage(image);
    }

    return BERR_SUCCESS;

err_text:
err_load:
err_offset:
    BMEM_Free(handle->deviceHandle->memHandle, handle->images[id].pBase);
    handle->images[id].pBase = NULL;
err_malloc:
err_relocations:
#if BDBG_DEBUG_BUILD
err_section:
#endif
    BELF_CloseImage(image);
err_elf_open:
    return rc;
}

static BERR_Code BAPE_Dsp_P_Boot(BAPE_DspHandle handle)
{
    uint32_t image_base, offset, entry;
    unsigned heap_base;
    unsigned heap_size;
    BERR_Code rc;
    void *pCachedBase;
    unsigned sharedMemorySize;
    uint32_t reg;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);

    BAPE_Dsp_P_Reset(handle);

    sharedMemorySize = sizeof(BAPE_DspIpcBlock);
    if(sharedMemorySize> handle->settings.sharedMemorySize) { return BERR_TRACE(BERR_NOT_SUPPORTED); }

    BDBG_MSG(("Kernel Base %#x", handle->images[BAPE_Img_Id_eBkernel].pBase));
    rc = BMEM_ConvertAddressToOffset(handle->deviceHandle->memHandle, handle->images[BAPE_Img_Id_eBkernel].pBase, &image_base);
    if(rc!=BERR_SUCCESS){return BERR_TRACE(rc);}
    heap_size = handle->settings.heapSize + handle->settings.sharedMemorySize + handle->settings.stackSize;
    heap_base = image_base + handle->images[BAPE_Img_Id_eBkernel].size;


    BMEM_ConvertAddressToCached(handle->deviceHandle->memHandle, handle->images[BAPE_Img_Id_eBkernel].pBase, &pCachedBase);
    BMEM_FlushCache(handle->deviceHandle->memHandle, pCachedBase, handle->images[BAPE_Img_Id_eBkernel].size+heap_size);
    handle->pIpc = (void *)((uint8_t *)pCachedBase + (heap_base - image_base));
    BDBG_MSG(("image_size:%u image_base:%#x,%#x,%#x heap_size:%u heap_base:%#x,%#x target ticks %#x", handle->images[BAPE_Img_Id_eBkernel].size, image_base, handle->images[BAPE_Img_Id_eBkernel].pBase, pCachedBase, heap_size, heap_base, handle->pIpc, &handle->pIpc->targetTicks ));
    BKNI_Memset(handle->pIpc, 0, sizeof(BAPE_DspIpcBlock));
    handle->pIpc->heapInfo.sharedMemorySize = handle->settings.sharedMemorySize;
    handle->pIpc->heapInfo.messageQueueLength = BAPE_MAX_INTERRUPTS;
    handle->pIpc->heapInfo.flags = 0;
    handle->pIpc->size = sizeof(BAPE_DspIpcBlock);
    handle->pIpc->maxTasks = BAPE_CHIP_MAX_DECODERS;
    handle->pIpc->maxModules = BAPE_Img_Id_eMax-1;  /* Does not include the kernel image */
    BDBG_MSG(("IPC size %u", handle->pIpc->size));

    /* Setup boot header */
    BDBG_ASSERT(NULL != handle->settings.writeReserved);
    handle->settings.writeReserved(handle->settings.pParam1, handle->settings.param2, 0xff0, heap_base);
    handle->settings.writeReserved(handle->settings.pParam1, handle->settings.param2, 0xff4, heap_size);
    handle->settings.writeReserved(handle->settings.pParam1, handle->settings.param2, 0xff8, handle->settings.stackSize);
    entry = 0xa0000000|(image_base+(unsigned long)handle->images[BAPE_Img_Id_eBkernel].pEntry - (unsigned long)handle->images[BAPE_Img_Id_eBkernel].pBase);
    handle->settings.writeReserved(handle->settings.pParam1, handle->settings.param2, 0xffc, entry);

    /* Pause for TP1 to accept new command if running already */
    BKNI_Delay(100);

    /* Kickstart TP1 if required */
#define BMIPS_RELO_VECTOR_CONTROL_1 0x38000
    BDBG_ASSERT(NULL != handle->settings.readCoreRegister);
    reg = handle->settings.readCoreRegister(handle->settings.pParam1, handle->settings.param2, BMIPS_RELO_VECTOR_CONTROL_1);
    BDBG_MSG(("RELO_VECTOR_CONTROL %#x", reg));
    /*if ( 0 == reg )*/
    {
        uint32_t regVal;
        uint32_t reloVec, readback;
        volatile uint32_t *pBootMemory;
        void *pCachedBootMem, *pUncachedBootMem;
        /* TP1 is in reset.  Setup exception base to boot into the init memory */
        BMEM_ConvertAddressToOffset(handle->deviceHandle->memHandle, handle->pInitMemory, &offset);
        /* Round up to 1MB boundary until BMEM is fixed to align to offset instead of address. */
        offset = (offset + (1024*1024)) & ~((1024*1024)-1);
        BDBG_MSG(("Init memory located at offset %#x, entry %#x", offset, entry));
        /* Write RELO_VECTOR_CONTROL_1 with the proper base address value */
        BDBG_ASSERT(NULL != handle->settings.writeCoreRegister);
        reloVec = 0xa0000000 | offset | (1<<19);
        #if 1
        /* Exceptions at 0x800 */
        reloVec |= (0x4<<10);
        #else
        /* Exceptions at 0x400 */
        reloVec |= (0x2<<10);
        #endif
        readback = handle->settings.readCoreRegister(handle->settings.pParam1, handle->settings.param2, BMIPS_RELO_VECTOR_CONTROL_1);
        if ( readback == reloVec )
        {
            BDBG_MSG(("TP1 already started"));
        }
        else
        {
            handle->settings.writeCoreRegister(handle->settings.pParam1, handle->settings.param2, BMIPS_RELO_VECTOR_CONTROL_1, reloVec);            
            readback = handle->settings.readCoreRegister(handle->settings.pParam1, handle->settings.param2, BMIPS_RELO_VECTOR_CONTROL_1);
            BDBG_MSG(("RELO_VECTOR_CONTROL_1: %#x", readback));
            /* Write init sequence to boot memory */
            BMEM_ConvertOffsetToAddress(handle->deviceHandle->memHandle, offset, &pUncachedBootMem);
            BMEM_ConvertAddressToCached(handle->deviceHandle->memHandle, pUncachedBootMem, &pCachedBootMem);
            pBootMemory = (volatile uint32_t *)pCachedBootMem;
            pBootMemory[0] = 0x3c1a0000 | (entry >> 16);        /* lui k0, HI(entry) */
            pBootMemory[1] = 0x375a0000 | (entry & 0xffff);     /* ori k0, LO(entry) */
            pBootMemory[2] = 0x03400008;                        /* jr k0 */
            pBootMemory[3] = 0x00000000;                        /* nop */
            BMEM_FlushCache(handle->deviceHandle->memHandle, pCachedBootMem, 16);
            /* Take TP1 out of reset */
            BDBG_ASSERT(NULL != handle->settings.readCmtControl);
            BDBG_ASSERT(NULL != handle->settings.writeCmtControl);
            regVal = handle->settings.readCmtControl(handle->settings.pParam1, handle->settings.param2);
            #if 1
            regVal &= ~(3<<30);
            regVal |= 1<<30;    /* Enable performance counters for TP1 */
            regVal &= ~(3<<4);  
            regVal |= 2<<4;     /* Enable TP1 priority for shared resources */
            #endif
            regVal |= 1;        /* Take TP1 out of reset */
            BDBG_MSG(("Kickstarting TP1 (CMT Control %#x)", regVal));
            handle->settings.writeCmtControl(handle->settings.pParam1, handle->settings.param2, regVal);
            regVal = handle->settings.readCmtControl(handle->settings.pParam1, handle->settings.param2);
        }
    }
    #if 0
    else
    {
        BDBG_MSG(("TP1 already started"));
    }
    #endif
    return BERR_SUCCESS;
}


static void BAPE_Dsp_P_Reset(BAPE_DspHandle handle)
{
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_IPI1_INTR2_CPU_SET, BCHP_FIELD_DATA(IPI1_INTR2_CPU_SET, TP1, 1));
    BKNI_Sleep(100); /* give TP1 enough time to handle this interrupt, and jump into the safe region */
    BREG_Write32(handle->deviceHandle->regHandle, BCHP_IPI1_INTR2_CPU_CLEAR, BCHP_FIELD_DATA(IPI1_INTR2_CPU_CLEAR, TP1, 1));
}

static void BAPE_Dsp_P_Halt(BAPE_DspHandle dsp)
{
    BAPE_Message message;
    BAPE_Reply reply;
    BERR_Code errCode;

    message.id = BAPE_MessageId_eHalt;

    errCode = BAPE_Dsp_P_SendMessage(dsp, &message, &reply);
    if ( errCode!=BERR_SUCCESS )
    {
        /* ignore error and reset DSP */
        BAPE_Dsp_P_Reset(dsp);
    }

    return ;
}

static BERR_Code BAPE_DspContext_P_RequestModules(BAPE_DspContext *handle)
{
    BERR_Code errCode;
    BAPE_DspHandle dsp;
    BAPE_Img_Id imgId;
    unsigned i;

    BDBG_ASSERT(NULL != handle);
    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);

    dsp = handle->dsp;

    errCode = BAPE_Dsp_P_CodecToImg(handle->decoder->startSettings.codec, &imgId, handle->decoder->passthrough);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if ( dsp->images[imgId].usageCount > 0 )
    {
        dsp->images[imgId].usageCount++;
        BDBG_MSG(("IMG ID %u already loaded (usage count now %u)", dsp->images[imgId].usageCount));
    }
    else
    {
        /* Garbage collect unused modules */
        for ( i = 0; i < BAPE_Img_Id_eMax; i++ )
        {
            if ( dsp->images[i].usageCount == 0 && dsp->images[i].moduleId != BAPE_DSP_MODULE_ID_INVALID )
            {
                BDBG_MSG(("Releasing unused module ID %u", dsp->images[i].moduleId));
                BKNI_Memset(&dsp->msg, 0, sizeof(BAPE_Message));
                dsp->msg.id = BAPE_MessageId_eUnregisterModule;
                dsp->msg.data.unregisterModule.moduleId = dsp->images[i].moduleId;
                errCode = BAPE_Dsp_P_SendMessage(dsp, &dsp->msg, &dsp->reply);
                BDBG_ASSERT(errCode == BERR_SUCCESS);   /* If this fails, we're out of sync and lost -- should raise watchdog */
                dsp->images[i].moduleId = BAPE_DSP_MODULE_ID_INVALID;
                BMEM_Free(dsp->deviceHandle->memHandle, dsp->images[i].pBase);
                dsp->images[i].pBase = NULL;
                BAPE_Dsp_P_UnloadImage(dsp, i);
            }
        }
        /* Now load our module */
        errCode = BAPE_Dsp_P_LoadImage(dsp, imgId, 0);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        BDBG_MSG(("Registering module for image ID %u", imgId));
        BKNI_Memset(&dsp->msg, 0, sizeof(BAPE_Message));
        BKNI_Memset(&dsp->reply, 0, sizeof(BAPE_Reply));
        dsp->msg.id = BAPE_MessageId_eRegisterModule;
        dsp->msg.data.registerModule.pEntry = BAPE_Dsp_P_HostToDsp(dsp, dsp->images[imgId].pEntry);
        dsp->msg.data.registerModule.pCodeSegmentBase = BAPE_Dsp_P_HostToDsp(dsp, dsp->images[imgId].pTextBaseUncached);
        dsp->msg.data.registerModule.codeSegmentLength = dsp->images[imgId].textSize;
        errCode = BAPE_Dsp_P_SendMessage(dsp, &dsp->msg, &dsp->reply);
        if ( errCode )
        {
            BMEM_Free(dsp->deviceHandle->memHandle, dsp->images[imgId].pBase);
            dsp->images[imgId].pBase = NULL;
            return BERR_TRACE(errCode);
        }
        dsp->images[imgId].usageCount = 1;
        dsp->images[imgId].moduleId = dsp->reply.data.registerModule.moduleId;
    }

    return BERR_SUCCESS;
}
#endif  /* #ifndef BAPE_EMULATION */

static BAPE_DspContextHandle BAPE_Dsp_P_TaskIdToContext_isr(BAPE_DspHandle dsp, uint32_t taskId)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(dsp, BAPE_Dsp);

    for ( i = 0; i < BAPE_CHIP_MAX_DECODERS; i++ )
    {
        if ( dsp->contexts[i].taskId == taskId )
        {
            return &dsp->contexts[i];
        }
    }

    return NULL;
}

static BAPE_DecoderHandle BAPE_Dsp_P_TaskIdToDecoder_isr(BAPE_DspHandle dsp, uint32_t taskId)
{
    BAPE_DspContextHandle context;

    context = BAPE_Dsp_P_TaskIdToContext_isr(dsp, taskId);
    if ( context )
    {
        return context->decoder;
    }
    return NULL;
}

static void *BAPE_Dsp_P_HostToDsp(BAPE_DspHandle dsp, const void *pAddress)
{
#ifdef BAPE_EMULATION
    BSTD_UNUSED(dsp);
    return (void *)((uint32_t)pAddress);
#else
    BERR_Code errCode;
    void *pUncached;
    uint32_t cachedHi, cachedLo, offset;

    BDBG_OBJECT_ASSERT(dsp, BAPE_Dsp);
    BDBG_ASSERT(NULL != pAddress);

    cachedLo = (uint32_t)dsp->pIpc;
    cachedHi = cachedLo + sizeof(BAPE_DspIpcBlock);

    if ( (uint32_t)pAddress >= cachedLo && (uint32_t)pAddress < cachedHi )
    {
        pUncached = (void *)(((uint32_t)pAddress - cachedLo) + ((uint32_t)dsp->images[BAPE_Img_Id_eBkernel].pBase + dsp->images[BAPE_Img_Id_eBkernel].size));
    }
    else
    {
        pUncached = (void *)(uint32_t)pAddress;
    }

    errCode = BMEM_ConvertAddressToOffset(dsp->deviceHandle->memHandle, pUncached, &offset);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return NULL;
    }
    return (void *)offset;
#endif
}

#if 0
static void *BAPE_Dsp_P_DspToCached(BAPE_DspHandle dsp, const void *pAddress)
{
#ifdef BAPE_EMULATION
    BSTD_UNUSED(dsp);
    return pAddress;
#else
    void *pUncached;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(dsp, BAPE_Dsp);
    BDBG_ASSERT(NULL != pAddress);

    pUncached = BAPE_Dsp_P_DspToUncached(dsp, pAddress);
    if ( pUncached )
    {
        void *pCached;
        errCode = BMEM_ConvertAddressToCached(dsp->deviceHandle->memHandle, pUncached, &pCached);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return NULL;
        }
        return pCached;
    }
    return NULL;
#endif
}

static void *BAPE_Dsp_P_DspToUncached(BAPE_DspHandle dsp, const void *pAddress)
{
#ifdef BAPE_EMULATION
    BSTD_UNUSED(dsp);
    return pAddress;
#else
    void *pUncached;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(dsp, BAPE_Dsp);
    BDBG_ASSERT(NULL != pAddress);

    errCode = BMEM_ConvertOffsetToAddress(dsp->deviceHandle->memHandle, (uint32_t)pAddress, &pUncached);
    if ( errCode )
    {
        (void)BERR_TRACE(errCode);
        return NULL;
    }

    return pUncached;
#endif
}
#endif

static BERR_Code BAPE_Dsp_P_SendMessage(BAPE_DspHandle handle, const BAPE_Message *pMessage, BAPE_Reply *pReply)
{
    const unsigned timeout=1000;  /* 1 second timeout */
    unsigned retries = 2;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);
    BDBG_ASSERT(NULL != pMessage);
    BDBG_ASSERT(NULL != pReply);

    if ( handle->watchdogAsserted )
    {
        BDBG_MSG(("Watchdog Asserted.  Ignoring message."));
        return BERR_TIMEOUT;
    }

    BKNI_ResetEvent(handle->replyEvent);

    /* Populate message buffer */
    BKNI_Memcpy(&handle->pIpc->message, pMessage, sizeof(BAPE_Message));
    handle->pIpc->messageNum++;

    BDBG_MSG(("Send message number %u", handle->pIpc->messageNum));

    do
    {
        /* Raise IPC interrupt */
        #ifdef BAPE_EMULATION
        errCode = BAPE_P_DspMessageReady(handle->deviceHandle);
        BDBG_ASSERT(errCode == BERR_SUCCESS);
        #else
        BREG_Write32(handle->deviceHandle->regHandle, BCHP_IPI1_INTR2_CPU_SET, BCHP_FIELD_DATA(IPI1_INTR2_CPU_SET, TP0, 1));
        #endif

        /* Wait for Reply */
        errCode = BKNI_WaitForEvent(handle->replyEvent, timeout);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
        BDBG_MSG(("Received reply number %u", handle->pIpc->replyNum));
    } while ( handle->pIpc->replyNum != handle->pIpc->messageNum && --retries > 0 && !handle->watchdogAsserted );

    if ( handle->pIpc->messageNum == handle->pIpc->replyNum )
    {
        /* Get Reply Buffer */
        BKNI_Memcpy(pReply, &handle->pIpc->reply, sizeof(BAPE_Reply));

        if ( pReply->errCode )
        {
            return BERR_TRACE(pReply->errCode);
        }
    }
    else
    {
        /* Reply never received. */
        return BERR_TRACE(BERR_TIMEOUT);
    }

    return BERR_SUCCESS;
}

static void BAPE_DspContext_P_ConfigSourceChannel(BAPE_DspContextHandle handle, BAPE_MixerInput connector, uint32_t sourceChannelId, uint32_t masterChannelId, bool compressed, BAPE_ChannelPair channelPair)
{
    uint32_t regAddr, regVal;
    BREG_Handle regHandle;
    BAPE_BufferNode *pBuffer;

    BDBG_OBJECT_ASSERT(handle->dsp, BAPE_Dsp);
    BDBG_OBJECT_ASSERT(connector, BAPE_MixerInput);

    regHandle = handle->dsp->deviceHandle->regHandle;

    /* Set the source channel configuration */
    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_CFGi_ARRAY_BASE + (4*sourceChannelId);
    regVal = BREG_Read32(regHandle, regAddr);
    regVal &= ~(
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_ID_HIGH)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, REVERSE_ENDIAN)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARED_SBUF_ID)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, DMA_READ_DISABLE)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, NOT_PAUSE_WHEN_EMPTY)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, START_SELECTION)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, STCSYNC_ENABLE)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BUFFER_PAIR_ENABLE)|
                BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_ENABLE)
                );
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, PROCESS_SEQ_ID_VALID, 1);
    if ( compressed )
    {
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, 16);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_CH_MODE, 0);
    }
    else
    {
        #if BAPE_CHIP_BYTES_PER_COMPRESSED_SAMPLE == 2
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, 16);
        #elif BAPE_CHIP_BYTES_PER_COMPRESSED_SAMPLE == 4
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, BIT_RESOLUTION, 0);
        #else
        #error Unsupported bytes per sample
        #endif
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SAMPLE_REPEAT_ENABLE, handle->dsp->deviceHandle->settings.rampPcmSamples?1:0);
    }
    if ( masterChannelId != sourceChannelId && masterChannelId != 0xff )
    {
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARED_SBUF_ID, masterChannelId);
        regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SHARE_SBUF, 1);
    }
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SFIFO_START_HALFFULL, 1);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_BF_CTRL_SOURCECH_CFGi, SOURCEFIFO_SIZE_DOUBLE, 1);
    BREG_Write32(regHandle, regAddr, regVal);    

    /* Never invert MSB for decoded data - it's always signed (or compressed) */
    regAddr = BCHP_AUD_FMM_BF_CTRL_SOURCECH_GRPi_ARRAY_BASE + (4*sourceChannelId);
    regVal = BREG_Read32(regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_BF_CTRL_SOURCECH_GRPi, INVERT_MSB);
    BREG_Write32(regHandle, regAddr, regVal);

    /* Program ringbuffer registers based on buffer pool */
    pBuffer = connector->pBuffers[channelPair];
    BDBG_OBJECT_ASSERT(pBuffer, BAPE_BufferNode);
    regAddr = BAPE_P_SOURCE_CHANNEL_TO_RDADDR_REG(sourceChannelId);
    BREG_Write32(regHandle, regAddr, pBuffer->offset);
    regAddr += 4;   /* Write is next */
    BREG_Write32(regHandle, regAddr, pBuffer->offset);
    regAddr += 4;   /* Base is next */
    BREG_Write32(regHandle, regAddr, pBuffer->offset);
    regAddr += 4;   /* End is next */
    BREG_Write32(regHandle, regAddr, pBuffer->offset + pBuffer->bufferSize - 1);
    regAddr += 4;   /* Freefull is next */
    BREG_Write32(regHandle, regAddr, pBuffer->offset);
    regAddr += 4;   /* WRPOINT is last */
    BREG_Write32(regHandle, regAddr, pBuffer->offset);
}

static BERR_Code BAPE_Dsp_P_CodecToImg(BAVC_AudioCompressionStd codec, BAPE_Img_Id *pId, bool passthrough)
{
    if ( passthrough )
    {
        switch ( codec )
        {
#if BAPE_MPEG_SUPPORT
        case BAVC_AudioCompressionStd_eMpegL1:
        case BAVC_AudioCompressionStd_eMpegL2:
        case BAVC_AudioCompressionStd_eMpegL3:
            *pId = BAPE_Img_Id_eMpegDecode;
            return BERR_SUCCESS;
#endif
#if BAPE_AC3_PASSTHROUGH_SUPPORT
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eAc3Plus:
            *pId = BAPE_Img_Id_eAc3Passthrough;
            return BERR_SUCCESS;
#endif
#if BAPE_AAC_SUPPORT
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
            *pId = BAPE_Img_Id_eAacDecode;
            return BERR_SUCCESS;
#endif
        default:
            *pId = BAPE_Img_Id_eMax;
            BDBG_ERR(("Codec %u not supported for passthrough", codec));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
    else
    {
        switch ( codec )
        {
#if BAPE_MPEG_SUPPORT
        case BAVC_AudioCompressionStd_eMpegL1:
        case BAVC_AudioCompressionStd_eMpegL2:
        case BAVC_AudioCompressionStd_eMpegL3:
            *pId = BAPE_Img_Id_eMpegDecode;
            return BERR_SUCCESS;
#endif
#if BAPE_DDP_SUPPORT
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eAc3Plus:
            *pId = BAPE_Img_Id_eDdpDecode;
            return BERR_SUCCESS;
#elif BAPE_AC3_SUPPORT
        case BAVC_AudioCompressionStd_eAc3:
            *pId = BAPE_Img_Id_eAc3Decode;
            return BERR_SUCCESS;
#endif
#if BAPE_AAC_SUPPORT
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
            *pId = BAPE_Img_Id_eAacDecode;
            return BERR_SUCCESS;
#endif
#if BAPE_WMA_SUPPORT
        case BAVC_AudioCompressionStd_eWmaStd:
            *pId = BAPE_Img_Id_eWmaDecode;
            return BERR_SUCCESS;
#endif
#if BAPE_PCMWAV_SUPPORT
        case BAVC_AudioCompressionStd_ePcmWav:
            *pId = BAPE_Img_Id_ePcmWav;
            return BERR_SUCCESS;
#endif
        default:
            *pId = BAPE_Img_Id_eMax;
            BDBG_ERR(("Codec %u not supported for decode", codec));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }
}

static void BAPE_Dsp_P_Ipc_isr(void *pParam1, int param2)
{
    BAPE_DspHandle handle = pParam1;
    BAPE_Interrupt *pInterrupt;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);
    BSTD_UNUSED(param2);

    BDBG_MSG(("DSP IPC Interrupt"));

    /* Check for interrupts */
    for ( pInterrupt = &handle->pIpc->interrupts[handle->pIpc->interruptRead]; pInterrupt->id != BAPE_InterruptId_eNone; )
    {
        BAPE_DecoderHandle decoder;
        decoder = BAPE_Dsp_P_TaskIdToDecoder_isr(handle, pInterrupt->taskId);
        if ( NULL == decoder )
        {
            BDBG_ERR(("Invalid task ID interrupt received (ID %u)", pInterrupt->taskId));
        }
        else
        {
            switch ( pInterrupt->id )
            {
            case BAPE_InterruptId_eLock:
                if ( decoder->interrupts.lock.pCallback_isr )
                {
                    decoder->interrupts.lock.pCallback_isr(
                        decoder->interrupts.lock.pParam1,
                        decoder->interrupts.lock.param2);
                }
                break;
            case BAPE_InterruptId_eUnlock:
                if ( decoder->interrupts.unlock.pCallback_isr )
                {
                    decoder->interrupts.unlock.pCallback_isr(
                        decoder->interrupts.unlock.pParam1,
                        decoder->interrupts.unlock.param2);
                }
                break;
            case BAPE_InterruptId_eSampleRateChanged:
                {
                    BAVC_AudioSamplingRate eSampleRate;

                    eSampleRate = BAPE_P_GetSampleRateEnum_isrsafe(pInterrupt->data.sampleRateChanged.sampleRate);
                    BDBG_MSG(("New decoder sample rate %u (enum %u) received", pInterrupt->data.sampleRateChanged.sampleRate, eSampleRate));
                    BAPE_Decoder_P_SetSampleRate_isr(decoder, eSampleRate);
                    /* ACK the sample rate back so the DSP can resume execution */
                    handle->pIpc->fmmSampleRate[decoder->index] = pInterrupt->data.sampleRateChanged.sampleRate;
                    if ( decoder->interrupts.sampleRateChange.pCallback_isr )
                    {
                        BDBG_MSG(("Sending sample rate change ISR to application"));
                        decoder->interrupts.sampleRateChange.pCallback_isr(
                            decoder->interrupts.sampleRateChange.pParam1,
                            decoder->interrupts.sampleRateChange.param2,
                            eSampleRate);
                    }
                }
                break;
            case BAPE_InterruptId_eFirstPts:
                if ( decoder->interrupts.firstPts.pCallback_isr )
                {
                    BAPE_DecoderTsmStatus tsmStatus;
                    BDBG_MSG(("First PTS ISR (decoder %u)", decoder->index));
                    tsmStatus.ptsStcDifference = 0;
                    tsmStatus.ptsInfo = pInterrupt->data.firstPts.ptsInfo;
                    decoder->interrupts.firstPts.pCallback_isr(
                        decoder->interrupts.firstPts.pParam1,
                        decoder->interrupts.firstPts.param2,
                        &tsmStatus);
                }
                break;
            case BAPE_InterruptId_eTsmFail:
                if ( decoder->interrupts.tsmFail.pCallback_isr )
                {
                    BAPE_DecoderTsmStatus tsmStatus;
                    tsmStatus.ptsInfo = pInterrupt->data.tsmFail.ptsInfo;
                    tsmStatus.ptsStcDifference = pInterrupt->data.tsmFail.ptsStcDifference;
                    BDBG_MSG(("TSM Fail ISR (decoder %u) pts-stc %d (%d ms) pts %#x type %u", decoder->index, tsmStatus.ptsStcDifference, tsmStatus.ptsStcDifference/45, tsmStatus.ptsInfo.ui32CurrentPTS, tsmStatus.ptsInfo.ePTSType));
                    decoder->interrupts.firstPts.pCallback_isr(
                        decoder->interrupts.tsmFail.pParam1,
                        decoder->interrupts.tsmFail.param2,
                        &tsmStatus);
                }
                break;
            default:
                BDBG_MSG(("Unhandled interrupt ID %u", pInterrupt->id));
                break;
            }
        }
                        
        pInterrupt->id = BAPE_InterruptId_eNone;  /* Invalidate this entry before advancing read pointer */
        if ( handle->pIpc->interruptRead == (BAPE_MAX_INTERRUPTS-1) )
        {
            handle->pIpc->interruptRead = 0;
            pInterrupt = handle->pIpc->interrupts;
        }
        else
        {
            handle->pIpc->interruptRead++;
            pInterrupt++;
        }
    }

    /* Check for IPC reply */
    if ( handle->pIpc->prevReplyNum != handle->pIpc->replyNum )
    {
        BDBG_MSG(("IPC Reply Received.  Notifying task context."));
        handle->pIpc->prevReplyNum = handle->pIpc->replyNum;
        BKNI_SetEvent_isr(handle->replyEvent);
    }
}

static void BAPE_Dsp_P_Timer_isr(void *pParam1, int param2)
{
    BAPE_DspHandle handle = pParam1;
    uint32_t timerVal = param2;
    uint32_t newTicks;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);

    #define BAPE_MAX_DSP_TIMEOUTS (2)

#if 0
    /* Make ticks visible by BBS */
    BMEM_FlushCache(handle->deviceHandle->memHandle, &handle->pIpc->hostTicks, 8);
#endif

    /* Update host ticks */
    handle->pIpc->hostTicks++;
    /* Get Target Ticks */
    newTicks = handle->pIpc->targetTicks;
    /* If target ticks have not changed... */
    if ( newTicks == handle->prevTicks )
    {
        handle->violations++;
        handle->totalViolations++;
        BDBG_WRN(("DSP is not responding... %u of %u (%u total)", handle->violations, BAPE_MAX_DSP_TIMEOUTS, handle->totalViolations));
        if ( handle->violations >= BAPE_MAX_DSP_TIMEOUTS )
        {
            BAPE_Dsp_P_StateChange_isr(handle, 0);
            return;
        }
    }
    else
    {
        handle->violations = 0;
    }
    handle->prevTicks = newTicks;

    errCode = BTMR_StartTimer_isr(handle->timerHandle, timerVal);
    BDBG_ASSERT(BERR_SUCCESS == errCode);
}

static void BAPE_Dsp_P_StateChange_isr(void *pParam1, int param2)
{
    BAPE_DspHandle handle = pParam1;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);
    BSTD_UNUSED(param2);

    /* Stop timer */
    BTMR_StopTimer_isr(handle->timerHandle);

    /* Mark DSP as "dead" */
    handle->watchdogAsserted = true;
    /* Break any message sender out of their wait */
    BKNI_SetEvent_isr(handle->replyEvent);

    /* Raise watchdog interrupt if installed */
    if ( handle->deviceHandle->interrupts.watchdog.pCallback_isr )
    {
        BDBG_MSG(("Raising watchdog interrupt"));
        handle->deviceHandle->interrupts.watchdog.pCallback_isr(
            handle->deviceHandle->interrupts.watchdog.pParam1,
            handle->deviceHandle->interrupts.watchdog.param2);
    }
    else
    {
        BDBG_ERR(("Watchdog handler not installed.  Terminating application."));
        BDBG_ASSERT(0);
    }
}

#ifdef BAPE_EMULATION
void BAPE_P_DspIpcInterrupt_isr(
    BAPE_Handle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BAPE_Dsp_P_Ipc_isr(handle->dsp, 0);
}

static BERR_Code BAPE_Dsp_P_Boot(BAPE_DspHandle handle)
{
    BERR_Code errCode;
    void *pCached;

    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);

    handle->pIpcBase = BMEM_Alloc(handle->deviceHandle->memHandle, sizeof(BAPE_DspIpcBlock));
    if ( NULL == handle->pIpcBase )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BMEM_ConvertAddressToCached(handle->deviceHandle->memHandle, handle->pIpcBase, &pCached);
    handle->pIpc = pCached;
    BKNI_Memset(handle->pIpc, 0, sizeof(BAPE_DspIpcBlock));
    handle->pIpc->size = sizeof(BAPE_DspIpcBlock);
    handle->pIpc->maxTasks = BAPE_CHIP_MAX_DECODERS;
    handle->pIpc->maxModules = BAPE_Img_Id_eMax-1;  /* Does not include the kernel image */

    errCode = BAPE_P_DspBoot(handle->deviceHandle, handle->pIpc);
    if ( errCode )
    {
        BMEM_Free(handle->deviceHandle->memHandle, handle->pIpcBase);
        handle->pIpc = NULL;
        return BERR_TRACE(errCode);
    }

    return BERR_SUCCESS;
}
#endif

BERR_Code BAPE_Dsp_P_ProcessWatchdog(
    BAPE_DspHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Dsp);

    #if BDBG_DEBUG_BUILD
    BAPE_Dsp_P_Backtrace(handle);
    #endif

    return BERR_SUCCESS;
}

BERR_Code 
BAPE_GetDebugLog(BAPE_Handle handle, char *buf, size_t buf_len)
{
    const char *data;
    uint32_t logOffset;
    void *logAddr;
    void *logAddrUncached;
    const BAPE_DebugLogFifo *log;
    int diff;
    size_t writeCnt;
    BERR_Code rc;
    size_t offset;
    char ch;
    size_t bufferSize;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(buf_len>0);
    BDBG_ASSERT(buf);
    log = handle->dsp->pIpc->debugLog;
    if(!log) {
        goto done;
    }
    logOffset = (uint32_t)log;
    logOffset &= ~0xC0000000;
    rc = BMEM_ConvertOffsetToAddress(handle->memHandle, logOffset, &logAddrUncached);
    if(rc!=BERR_SUCCESS) { goto done; }
    rc = BMEM_ConvertAddressToCached(handle->memHandle, logAddrUncached, &logAddr);
    if(rc!=BERR_SUCCESS) { goto done; }
    log = logAddr;
    bufferSize = log->bufferSize;
    if(bufferSize==0) {
        goto done;
    }
    data  = log->buffer;
    writeCnt = log->writeCnt;
    diff = writeCnt - handle->dsp->logReadCnt;
    if(diff<0) {
        handle->dsp->logReadCnt = writeCnt;
        goto done;
    }
    /* skip over zeroes */
    while(diff>0) {
        offset = handle->dsp->logReadCnt % bufferSize;
        ch = data[offset];

        if(ch!='\0') {
            break;
        }
        handle->dsp->logReadCnt++;
    }
    if(diff==0) {
        goto done;
    }
    while(buf_len>1 && diff>0) {
        diff--;
        offset = handle->dsp->logReadCnt % bufferSize;
        handle->dsp->logReadCnt++;
        ch = data[offset];
        if(ch=='\0') { /* ignore '\r' and '\0' */
            continue;
        }
        if(ch=='\r') {
            ch='\n'; /* console buffer internally replaces '\n' with '\r' */
        }
        *buf = ch;
        buf++;
        buf_len--;
        if(ch=='\n') {
            break; /* captured entire line */
        }
    }

done:
    *buf = '\0';
    return BERR_SUCCESS;
}

