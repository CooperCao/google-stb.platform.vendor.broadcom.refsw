/******************************************************************************
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
 *****************************************************************************/

#include "bdsp.h"
#include "bdsp_raaga.h"
#include "bdsp_raaga_priv.h"

#include "libdspcontrol/DSP.h"

#if (BCHP_CHIP == 7278)
#include "libdspcontrol/src/DSP_raaga_octave_atu.h"
#endif

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#ifdef FIREPATH_BM
#include "mutex.h"
extern BEMU_Client_MutexHandle g_hSocketMutex;
#endif /* FIREPATH_BM */

BDBG_MODULE(bdsp_raaga);

#ifdef BDSP_FW_RBUF_CAPTURE
/* Settings for Firmware Ring Buffer Capture.Initialized in BRAP_RB_CBSetup() */
BDSP_P_RbufCapture Rbuf_Setting = { NULL, NULL, NULL, NULL };
#endif

void BDSP_Raaga_GetDefaultSettings(
    BDSP_RaagaSettings *pSettings     /* [out] */
    )
{
    uint32_t i;
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#if BDSP_IMG_SUPPORT
    pSettings->pImageContext = BDSP_IMG_Context;
    pSettings->pImageInterface = &BDSP_IMG_Interface;
#endif
    pSettings->preloadImages = false;
    pSettings->authenticationEnabled =false;

    /* All debug features will be disabled by default */
    pSettings->debugSettings[BDSP_Raaga_DebugType_eUart].enabled = false;
    pSettings->debugSettings[BDSP_Raaga_DebugType_eDramMsg].enabled = false;
    pSettings->debugSettings[BDSP_Raaga_DebugType_eCoreDump].enabled = false;
    pSettings->debugSettings[BDSP_Raaga_DebugType_eTargetPrintf].enabled = false;

    pSettings->debugSettings[BDSP_Raaga_DebugType_eUart].bufferSize = 0x1000; /* 4 KB by default */
    pSettings->debugSettings[BDSP_Raaga_DebugType_eDramMsg  ].bufferSize = 0x40000; /* 256 KB by default */
    pSettings->debugSettings[BDSP_Raaga_DebugType_eCoreDump].bufferSize = 0x90000; /* 512 KB by default, increasing by 64KB for 7271 due to increased code and data memory */
    pSettings->debugSettings[BDSP_Raaga_DebugType_eTargetPrintf].bufferSize = 0x40000; /* 256 KB by default */

    /*initialized to "0" to make sure that the initialization is done properly. Testing cant be done unless a new algoType is explicitly initialized */
    for(i=0;i < BDSP_AlgorithmType_eMax; i++){
        pSettings->maxAlgorithms[i]= 0;
    }

    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioDecode] = BDSP_RAAGA_MAX_DECODE_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEchoCanceller] = BDSP_RAAGA_MAX_ECHOCANCELLER_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEncode] = BDSP_RAAGA_MAX_ENCODE_CTXT;

    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioMixer] = BDSP_RAAGA_MAX_MIXER_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioProcessing] = BDSP_RAAGA_MAX_AUD_PROCESSING_CTXT; /* Keep it one always, it is just one big chunk to download all */
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioPassthrough] = BDSP_RAAGA_MAX_PASSTHROUGH_CTXT;

    pSettings->maxAlgorithms[BDSP_AlgorithmType_eVideoDecode] = BDSP_RAAGA_MAX_VIDEODECODE_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eVideoEncode] = BDSP_RAAGA_MAX_VIDEOENCODE_CTXT;
    pSettings->maxAlgorithms[BDSP_AlgorithmType_eSecurity] = BDSP_RAAGA_MAX_SCM_CTXT;

    pSettings->NumDsp = BDSP_RAAGA_MAX_DSP;

    /*pSettings->enableDebugLog = false;*/
}

/***********************************************************************
Name        :   BDSP_Raaga_Open

Type        :   PI Interface

Input       :   pDsp        -   Device Handle which is returned to the PI.
                chpHandle   -   Chip Handle provided by the PI.
                regHandle   -   Register Handle provided by the PI.
                memHandle   -   Memory Handle provided by the PI.
                intHandle       -   Interrupt Handle provided by the PI.
                tmrHandle   -   Timer Handle provided by the PI.
                boxHandle   -  BOX Mode handle provided by the PI.
                pSettings       -   Device Settings provided by the PI to open the Raaga Open.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
    1)  Allocate the memory for the Raaga device.
    2)  Intialise the Device handle for the Device to store the address.
    3)  Intialise all the function pointers which will be used by the PI for further processing.
    4)  Store the Register, Chip, Memory, Interrupt, Timer handles and Settings for Opening the Device provided by PI.
    5)  Call the Inetrnal BDSP function "BDSP_Raaga_P_Open".
    6)  Return the DSP device back to PI.
***********************************************************************/

BERR_Code BDSP_Raaga_Open(
    BDSP_Handle *pDsp,                      /* [out] */
    BCHP_Handle chpHandle,
    BREG_Handle regHandle,
    BMMA_Heap_Handle memHandle,
    BINT_Handle intHandle,
    BTMR_Handle tmrHandle,
    BBOX_Handle boxHandle,
    const BDSP_RaagaSettings *pSettings
    )
{
    BDSP_Raaga *pRaaga;
    BERR_Code ret= BERR_SUCCESS;

    BDBG_ASSERT(NULL != pDsp);
    BDBG_ASSERT(NULL != chpHandle);
    BDBG_ASSERT(NULL != regHandle);
    BDBG_ASSERT(NULL != memHandle);
    BDBG_ASSERT(NULL != intHandle);

    /* tmr is not required */
    BSTD_UNUSED(tmrHandle);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ENTER(BDSP_Raaga_Open);
    /* Alloc raaga device */
    pRaaga = BKNI_Malloc(sizeof(BDSP_Raaga));
    if ( NULL == pRaaga )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(pRaaga, 0, sizeof(BDSP_Raaga));

    /* To enable this feature #define RAAGA_UART_ENABLE has to be uncommented in
    bdsp_raaga.h*/
#ifdef RAAGA_UART_ENABLE
    BDSP_Raaga_P_DirectRaagaUartToPort1(regHandle);
#endif


    /* Init device */
    BDSP_P_InitDevice(&pRaaga->device, pRaaga);
    pRaaga->device.close = BDSP_Raaga_P_Close;
    pRaaga->device.getDefaultContextSettings = BDSP_Raaga_P_GetDefaultContextSettings;
    pRaaga->device.createContext = BDSP_Raaga_P_CreateContext;
    pRaaga->device.getStatus= BDSP_Raaga_P_GetStatus;
    pRaaga->device.powerStandby= BDSP_Raaga_P_PowerStandby;
    pRaaga->device.powerResume= BDSP_Raaga_P_PowerResume;
    pRaaga->device.getAlgorithmInfo= BDSP_Raaga_P_GetAlgorithmInfo;
    pRaaga->device.allocateExternalInterrupt = BDSP_Raaga_P_AllocateExternalInterrupt;
    pRaaga->device.freeExternalInterrupt = BDSP_Raaga_P_FreeExternalInterrupt;
    pRaaga->device.getExternalInterruptInfo = BDSP_Raaga_P_GetExternalInterruptInfo;
    pRaaga->device.processAudioCapture = BDSP_Raaga_P_ProcessAudioCapture;

    /* Init context lists */
    BLST_S_INIT(&pRaaga->contextList);

    /* Init interrupt list */
    BLST_S_INIT(&pRaaga->interruptList);

    /* Save Settings and params */
    pRaaga->settings = *pSettings;
    pRaaga->chpHandle = chpHandle;
    pRaaga->regHandle = regHandle;
    pRaaga->memHandle = memHandle;
    pRaaga->intHandle = intHandle;
    pRaaga->boxHandle = boxHandle;

#ifdef BDSP_FW_RBUF_CAPTURE
    /* Specific to FW Ring Buffer capture required for their unit testing */
    if(Rbuf_Setting.rbuf_init != NULL && Rbuf_Setting.rbuf_uninit != NULL)
    {
        Rbuf_Setting.rbuf_init((BREG_Handle)regHandle, memHandle);
    }
#endif
    BDBG_OBJECT_SET(pRaaga, BDSP_Raaga);

    BDSP_Raaga_P_InitDeviceSettings(pRaaga->device.pDeviceHandle);

    /*TODO AJ/VJ : Check if SRAM to be removed */
    BDSP_Raaga_P_EnableAllPwrResource(pRaaga->device.pDeviceHandle, true);

    BDSP_Raaga_P_Reset(pRaaga->device.pDeviceHandle);

    ret = BDSP_Raaga_P_Open(pRaaga->device.pDeviceHandle);
    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
        goto err_open;
    }

    if(pRaaga->settings.authenticationEnabled == false)
    {
        ret  = BDSP_Raaga_P_Boot(pRaaga->device.pDeviceHandle);
        if (ret!=BERR_SUCCESS)
        {
            ret= BERR_TRACE(ret);
            goto err_open;
        }
    }

    *pDsp = &pRaaga->device;

    goto open_success;

err_open:
    BDSP_Raaga_P_EnableAllPwrResource(pRaaga->device.pDeviceHandle, false);
    BDBG_OBJECT_DESTROY(pRaaga, BDSP_Raaga);
    BKNI_Free(pRaaga);

open_success:
    BDBG_LEAVE( BDSP_Raaga_Open );
    return ret;
}

/***********************************************************************
Name        :   BDSP_Raaga_GetMemoryEstimate

Type        :   PI Interface

Input       :   pSettings       -   Device Settings provided by the PI to open the Raaga Open.
                pUsage      -   Pointer to usage case scenario from which we determine the runtime memory.
                boxHandle   -     BOX Mode Handle for which the memory needs to be estimated.
                pEstimate   -   Pointer provided by the where the memory estimate from the BDSP is returned.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
    1)  Leaf function provided by the BDSP to higher layers to return the estimate of the memory required by BDSP
***********************************************************************/
BERR_Code BDSP_Raaga_GetMemoryEstimate(
    const BDSP_RaagaSettings     *pSettings,
    const BDSP_RaagaUsageOptions *pUsage,
    BBOX_Handle                   boxHandle,
    BDSP_RaagaMemoryEstimate     *pEstimate /*[out]*/
)
{
    BERR_Code ret= BERR_SUCCESS;
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pEstimate);
    BDBG_ASSERT(NULL != pUsage);

    ret = BDSP_Raaga_P_GetMemoryEstimate(pSettings,pUsage,boxHandle, pEstimate);

    return ret;
}

#ifdef BDSP_FW_RBUF_CAPTURE
void BDSP_P_RbufSetup(
      BDSP_P_RbufCapture sRbufCap
      )
{
    Rbuf_Setting.rbuf_init = sRbufCap.rbuf_init;
    Rbuf_Setting.rbuf_uninit = sRbufCap.rbuf_uninit;
    Rbuf_Setting.rbuf_capture_channel_start = sRbufCap.rbuf_capture_channel_start;
    Rbuf_Setting.rbuf_capture_stop_channel = sRbufCap.rbuf_capture_stop_channel;
}
#endif

BERR_Code BDSP_Raaga_GetDownloadStatus(
    BDSP_Handle handle,
    BDSP_Raaga_DownloadStatus *pStatus /* [out] */
    )
{

    BDSP_Raaga *pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_GetDownloadStatus);
    /* Assert the function arguments*/
    BDBG_ASSERT(handle->pDeviceHandle);

    /*If Firmware Firmware authentication is Disabled*/
    if(pDevice->settings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Raaga_GetDownloadStatus should be called only if bFwAuthEnable is true"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    pStatus->pBaseAddress = pDevice->pFwHeapMemory;
    pStatus->physicalAddress = pDevice->FwHeapOffset;
    /*Size of the executable download */
    pStatus->length = pDevice->fwHeapSize;

    BDBG_LEAVE(BDSP_Raaga_GetDownloadStatus);

    return BERR_SUCCESS;
}


BERR_Code BDSP_Raaga_Initialize(BDSP_Handle handle)
{
    BERR_Code rc = BERR_SUCCESS ;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_Initialize);
    /* Assert the function arguments*/
    BDBG_ASSERT(handle->pDeviceHandle);

    /*If Firmware Firmware authentication is Disabled*/
    if(pDevice->settings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Raaga_StartDsp should be called only if bFwAuthEnable is true"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    rc = BDSP_Raaga_P_Boot(pDevice);
    if (rc!=BERR_SUCCESS)
    {
        rc= BERR_TRACE(rc);
        goto err_boot;
    }

err_boot:
    BDBG_LEAVE(BDSP_Raaga_Initialize);
    return rc;
}

BERR_Code BDSP_Raaga_GetAudioDelay_isrsafe(
    BDSP_CTB_Input                  *pCtbInput,
    BDSP_StageHandle hStage,
    BDSP_CTB_Output *pCTBOutput
)
{
    BERR_Code err = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);

    err = BDSP_Raaga_P_CalcThresholdZeroFillTimeAudOffset_isrsafe( pCtbInput, hStage->pStageHandle, pCTBOutput );

    return err;
}


BERR_Code BDSP_Raaga_GetDefaultAlgorithmSettings(
    BDSP_Algorithm algorithm,
    void *pSettingsBuffer,        /* [out] */
    size_t settingsBufferSize   /*[In]*/
    )
{
    const BDSP_Raaga_P_AlgorithmInfo *pInfo;

    BDBG_ASSERT(NULL != pSettingsBuffer);

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
    if ( NULL == pInfo )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(pInfo->userConfigSize != settingsBufferSize)
    {
        BDBG_ERR(("settingsBufferSize (%lu) is not equal to Config size (%lu) of algorithm %u (%s)",
            (unsigned long)settingsBufferSize, (unsigned long)pInfo->userConfigSize, algorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( settingsBufferSize > 0 )
    {
        BDBG_ASSERT(NULL != pInfo->pDefaultUserConfig);
        BKNI_Memcpy(pSettingsBuffer, pInfo->pDefaultUserConfig, settingsBufferSize);
    }

    return BERR_SUCCESS;
}
/***************************************************************************
Summary:
Get Raaga Firmware Debug Data
***************************************************************************/
BERR_Code BDSP_Raaga_GetDebugBuffer(
    BDSP_Handle handle,
    BDSP_Raaga_DebugType debugType, /* [in] Gives the type of debug buffer for which the Base address is required ... UART, DRAM, CoreDump ... */
    uint32_t dspIndex, /* [in] Gives the DSP Id for which the debug buffer info is required */
    BDSP_MMA_Memory *pBuffer, /* [out] Base address of the debug buffer data */
    size_t *pSize /* [out] Contiguous length of the debug buffer data in bytes */
)
{
    BERR_Code rc = BERR_SUCCESS ;
    BDSP_Raaga *pDevice;

    dramaddr_t  ui32ReadAddr,ui32WriteAddr,
                ui32EndAddr, ui32DebugFifoOffset;
	dramaddr_t ui32BaseAddr;
    uint32_t ui32ReadSize, uiOffset;
    TB_data_descriptor DataDescriptor;
    unsigned int ReadAmount = 0;

    BDBG_ENTER(BDSP_Raaga_GetDebugBuffer);
    /* Assert the function arguments*/
    BDBG_ASSERT(handle->pDeviceHandle);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pSize);

    /* For the cases where Nexus might be configured for 2 DSP and BDSP for 1.
        Should never happen actually */
    if((dspIndex + 1) > BDSP_RAAGA_MAX_DSP){
        *pSize = 0;
        rc= BERR_TRACE( BERR_NOT_SUPPORTED );
        goto end;
    }

    pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

    uiOffset = pDevice->dspOffset[dspIndex] ;
    ui32DebugFifoOffset = (BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)
                            *(BDSP_RAAGA_DEBUG_FIFO_START_INDEX + debugType);

    if(debugType != BDSP_Raaga_DebugType_eTargetPrintf)
    {

        ui32ReadAddr  = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR  + ui32DebugFifoOffset + uiOffset);
        ui32WriteAddr = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR + ui32DebugFifoOffset + uiOffset);
        ui32EndAddr   = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR  + ui32DebugFifoOffset + uiOffset);
		*pBuffer = pDevice->memInfo.FwDebugBuf[dspIndex][debugType].Buffer;
		ui32BaseAddr = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR  + ui32DebugFifoOffset + uiOffset);

		pBuffer->pAddr = (void *)((uint8_t *)pDevice->memInfo.FwDebugBuf[dspIndex][debugType].Buffer.pAddr +
                            (ui32ReadAddr - ui32BaseAddr));

        ui32ReadSize = ui32WriteAddr - ui32ReadAddr ;
        if( ui32ReadAddr > ui32WriteAddr )
        {
            /* Bottom Chunk only - Contiguous data*/
            ui32ReadSize  = (ui32EndAddr - ui32ReadAddr);
            BDBG_MSG(("Got the Debug Data, update the size=%x", ui32ReadSize));
        }

        *pSize = ui32ReadSize;
    }
    else
    {
#if (BCHP_CHIP !=7278)
        TB_peek(&(pDevice->memInfo.sTbTargetPrint[dspIndex]), &DataDescriptor);
        *pSize = 0;
        while((ReadAmount = TB_read(&DataDescriptor, (void *)((unsigned char*)pDevice->memInfo.TargetPrintBuffer[dspIndex].pAddr + *pSize), 1024, true)) > 0)
        {
            *pSize += ReadAmount;
        }
        *pBuffer = pDevice->memInfo.TargetPrintBuffer[dspIndex];

#else /* (BCHP_CHIP !=7278) */
#ifdef FIREPATH_BM
        if (0 != BEMU_Client_AcquireMutex(g_hSocketMutex))
        {
            BDBG_ERR(("Failed to acquire mutex in BEMU_Client_CloseSocket\n"));
        }
#endif /* FIREPATH_BM */

        TB_peek(&(pDevice->memInfo.sTbTargetPrint[dspIndex]), &DataDescriptor);

#ifdef FIREPATH_BM
        BEMU_Client_ReleaseMutex(g_hSocketMutex);
#endif /* FIREPATH_BM */
        *pSize = 0;
        /*BDBG_MSG(("hBlock :%x, pAddr:%x, offset:%llx", pDevice->memInfo.TargetPrintBuffer[dspIndex].hBlock, pDevice->memInfo.TargetPrintBuffer[dspIndex].pAddr, pDevice->memInfo.TargetPrintBuffer[dspIndex].offset));*/
        ui32ReadSize = pDevice->memInfo.FwDebugBuf[dspIndex][BDSP_Raaga_DebugType_eTargetPrintf].ui32Size;
        while((ReadAmount = TB_read(&DataDescriptor, (void *)((unsigned char*)pDevice->memInfo.TargetPrintBuffer[dspIndex].pAddr + *pSize), ui32ReadSize, true)) > 0)
        {
            *pSize += ReadAmount;
            /*BDBG_MSG(("Got the TargetPrint Data, update the size=%d", (unsigned int)(*pSize)));*/
            ui32ReadSize -= ReadAmount;
            /*BDBG_MSG(("Got the TargetPrint Data, update the size=%zu", *pSize));*/
        }

        /* TODO: if our local buffer fills up, then we should get the caller to
         * read again. For now just warn, but this risks losing some target
         * buffer data when we wouldn't otherwise need to.. */
        if (ui32ReadSize == 0)
        {
            BDBG_MSG(("TargetPrintBuffer filled up in one shot!"));
        }

        *pBuffer = pDevice->memInfo.TargetPrintBuffer[dspIndex];

#ifdef FIREPATH_BM
        if (0 != BEMU_Client_AcquireMutex(g_hSocketMutex))
        {
            BDBG_ERR(("Failed to acquire mutex in BEMU_Client_CloseSocket\n"));
        }
#endif /* FIREPATH_BM */

        TB_discard(&pDevice->memInfo.sTbTargetPrint[dspIndex], *pSize);

#ifdef FIREPATH_BM
        BEMU_Client_ReleaseMutex(g_hSocketMutex);
#endif /* FIREPATH_BM */

        /*BDBG_ERR(("Target Print Data Discarded"));*/
#endif /* (BCHP_CHIP !=7278) */
    }

end:
    BDBG_LEAVE(BDSP_Raaga_GetDebugBuffer);

    return rc;
}

/***************************************************************************
Summary:
Consume debug data from the debug ringbuffer.
***************************************************************************/
BERR_Code BDSP_Raaga_ConsumeDebugData(
    BDSP_Handle handle,
    BDSP_Raaga_DebugType debugType, /* [in] Gives the type of debug buffer for which the Base address is required ... UART, DRAM, CoreDump ... */
    uint32_t dspIndex, /* [in] Gives the DSP Id for which the debug data needs to be consumed */
    size_t bytesConsumed    /* [in] Number of bytes consumed from the debug buffer */
)
{
    BERR_Code rc = BERR_SUCCESS;
    BDSP_Raaga *pDevice;

    dramaddr_t  ui32BaseAddr, ui32ReadAddr,ui32WriteAddr,
                    ui32EndAddr;
    size_t  ui32ReadSize, uiOffset,ui32DebugFifoOffset;

    BDBG_ENTER(BDSP_Raaga_ConsumeDebugData);
    /* Assert the function arguments*/
    BDBG_ASSERT(handle->pDeviceHandle);

    pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

    uiOffset = pDevice->dspOffset[dspIndex] ;
    ui32DebugFifoOffset = (BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)
                            *(BDSP_RAAGA_DEBUG_FIFO_START_INDEX + debugType);

    if(debugType != BDSP_Raaga_DebugType_eTargetPrintf)
    {

        ui32BaseAddr  = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32DebugFifoOffset + uiOffset);
        ui32ReadAddr  = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR  + ui32DebugFifoOffset + uiOffset);
        ui32WriteAddr = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR + ui32DebugFifoOffset + uiOffset);
        ui32EndAddr   = BDSP_ReadReg(pDevice->regHandle,
                            BCHP_RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR  + ui32DebugFifoOffset + uiOffset);

        /* Get the amount data available in the buffer*/
        ui32ReadSize = ui32WriteAddr - ui32ReadAddr ;
        if( ui32ReadAddr > ui32WriteAddr )
        {
            /* Bottom Chunk + Top Chunk */
            ui32ReadSize  = (ui32EndAddr - ui32ReadAddr) + (ui32WriteAddr - ui32BaseAddr);
        }

        if (bytesConsumed <= ui32ReadSize)
        {
            ui32ReadAddr += bytesConsumed;
            if(ui32ReadAddr >= ui32EndAddr)
            {
                ui32ReadAddr = ui32BaseAddr + (ui32ReadAddr - ui32EndAddr);
            }

            BDSP_WriteReg(pDevice->regHandle, BCHP_RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR  + ui32DebugFifoOffset +
                            uiOffset, ui32ReadAddr);
        }
        else
        {
            /* Return error if bytesConsumed is more
               than the data available in the buffer */
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
#ifdef FIREPATH_BM
        if (0 != BEMU_Client_AcquireMutex(g_hSocketMutex))
        {
            BDBG_ERR(("Failed to acquire mutex in BEMU_Client_CloseSocket\n"));
        }
#endif /* FIREPATH_BM */

        TB_discard(&pDevice->memInfo.sTbTargetPrint[dspIndex], bytesConsumed);

#ifdef FIREPATH_BM
        BEMU_Client_ReleaseMutex(g_hSocketMutex);
#endif /* FIREPATH_BM */

        /*BDBG_ERR(("Target Print Data Discarded"));*/
    }

    BDBG_LEAVE(BDSP_Raaga_ConsumeDebugData);

    return rc;
}

/***************************************************************************
Summary:
Returns the Status of the DSP
***************************************************************************/
BDSP_Raaga_FwStatus BDSP_Raaga_GetCoreDumpStatus (
    BDSP_Handle handle,
    uint32_t dspIndex) /* [in] Gives the DSP Id for which the core dump status is required */
{
    uint32_t            uiOffset;

    BDSP_Raaga *pDevice;
    BDSP_Raaga_FwStatus eStatus;

    BDBG_ASSERT(handle->pDeviceHandle);

    pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

    uiOffset = pDevice->dspOffset[dspIndex];
    eStatus = BDSP_Read32 (pDevice->regHandle, BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_BASE + uiOffset + 20*4);

    return eStatus;
}

/***************************************************************************
Summary:
Get the codec copability status
***************************************************************************/
void BDSP_Raaga_GetCodecCapabilities(BDSP_CodecCapabilities *pSetting)
{
#ifdef BDSP_MS12_SUPPORT
    switch (BDSP_MS12_SUPPORT)
    {
        case 'A':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'A' "));
            pSetting->dolbyMs.dapv2 = true;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = true;
            pSetting->dolbyMs.ddpEncode71 = true;
            pSetting->dolbyMs.pcm71 = true;
            break;
        case 'B':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'B' "));
            pSetting->dolbyMs.dapv2 = true;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = true;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
        case 'C':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'C' "));
            pSetting->dolbyMs.dapv2 = true;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = false;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
        case 'D':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'D' "));
            pSetting->dolbyMs.dapv2 = false;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = true;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
        default:
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'C', Displaying MS12 Capabilities as: "));
            pSetting->dolbyMs.dapv2 = true;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = false;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
    }
#else
            pSetting->dolbyMs.dapv2 = false;
#ifdef BDSP_AC3ENC_SUPPORT
            pSetting->dolbyMs.ddEncode = true;
#else
            pSetting->dolbyMs.ddEncode = false;
#endif
            pSetting->dolbyMs.ddpEncode51 = false;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
#endif

            BDBG_MSG(("pSetting->dolbyMs.dapv2 = %d", pSetting->dolbyMs.dapv2));
            BDBG_MSG(("pSetting->dolbyMs.ddEncode = %d", pSetting->dolbyMs.ddEncode));
            BDBG_MSG(("pSetting->dolbyMs.ddpEncode51 = %d", pSetting->dolbyMs.ddpEncode51));
            BDBG_MSG(("pSetting->dolbyMs.ddpEncode71 = %d", pSetting->dolbyMs.ddpEncode71));
            BDBG_MSG(("pSetting->dolbyMs.pcm71 = %d", pSetting->dolbyMs.pcm71));
}

#if (BCHP_CHIP ==7278)
void dump_l2c_tags(BREG_Handle hReg)
{
    #define NUM_CACHE_SETS 128
    #define NUM_CACHE_WAYS 10
    static uint64_t dumpTagData[NUM_CACHE_SETS][NUM_CACHE_WAYS];
    int set, way;

    BDBG_MSG(("Dumping cache tags"));

    /* Freeze client intefaces to L2C to minimise perturbation. */
    BREG_Write32(hReg, BCHP_RAAGA_DSP_L2C_CTRL5, 0xf);

    BDBG_MSG(("DSP L2C interface frozen"));

    for (set = 0; set < NUM_CACHE_SETS; set++)
    {
        for (way = 0; way < NUM_CACHE_WAYS; way++)
        {
            int addr = (set << 4) + way;
            BDSP_Write32(hReg, BCHP_RAAGA_DSP_L2C_PP_R2TD_ADDR, addr);
            BDSP_Write32(hReg, BCHP_RAAGA_DSP_L2C_PP_R2TD_CMD, 0x10); /* lower nibble = 0 -> tag, upper nibble = 1 -> read */
            while (BDSP_Read32(hReg, BCHP_RAAGA_DSP_L2C_PP_STATUS));  /* wait for the busy bits to clear up.. */
            dumpTagData[set][way] = BDSP_Read64(hReg, BCHP_RAAGA_DSP_L2C_PP_TD2R_RDATA);
        }
    }

    /* Unfreeze client interface. */
    BDSP_Write32(hReg, BCHP_RAAGA_DSP_L2C_CTRL5, 0x0);

    BDBG_MSG(("DSP L2C interface unfrozen"));

    for (set = 0; set < NUM_CACHE_SETS; set++)
    {
        for (way = 0; way < NUM_CACHE_WAYS; way++)
        {
            uint32_t vaddr = (dumpTagData[set][way] << 16) | (set << 9);
            unsigned state = (dumpTagData[set][way] >> 20) & 0x7f;
            unsigned core_id = (dumpTagData[set][way] >> 28) & 0x7;
            unsigned lock = (dumpTagData[set][way] >> 32) & 0x7;
            unsigned lru_index = (dumpTagData[set][way] >> 36) & 0xf;
            if (state != 0 && /* cache line is assigned */
                (core_id & 1) == 1 /* cache line is data */
                )
            {
                BDBG_MSG(("Set: %2x Way: %2x VADDR: %08x State: %x Core: %x-%s Lock: %x LRU: %x",
                        set,
                        way,
                        vaddr,
                        state,
                        core_id >> 1,
                        core_id & 1 ? "D" : "I",
                        lock,
                        lru_index));
            }
        }
    }
}
#endif
