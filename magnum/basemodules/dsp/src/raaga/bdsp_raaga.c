/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bdsp.h"
#include "bdsp_raaga.h"
#include "bdsp_raaga_priv.h"

#include "libdspcontrol/DSP.h"

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
    pSettings->debugSettings[BDSP_DebugType_eUart].enabled = false;
    pSettings->debugSettings[BDSP_DebugType_eDramMsg].enabled = false;
    pSettings->debugSettings[BDSP_DebugType_eCoreDump].enabled = false;
    pSettings->debugSettings[BDSP_DebugType_eTargetPrintf].enabled = false;

    pSettings->debugSettings[BDSP_DebugType_eUart].bufferSize = 0x1000; /* 4 KB by default */
    pSettings->debugSettings[BDSP_DebugType_eDramMsg  ].bufferSize = 0x40000; /* 256 KB by default */
    pSettings->debugSettings[BDSP_DebugType_eCoreDump].bufferSize = 0x90000; /* 512 KB by default, increasing by 64KB for 7271 due to increased code and data memory */
    pSettings->debugSettings[BDSP_DebugType_eTargetPrintf].bufferSize = 0x40000; /* 256 KB by default */

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
    pRaaga->device.getDebugBuffer = BDSP_Raaga_P_GetDebugBuffer;
    pRaaga->device.consumeDebugData = BDSP_Raaga_P_ConsumeDebugData;
    pRaaga->device.getCoreDumpStatus = BDSP_Raaga_P_GetCoreDumpStatus;
    pRaaga->device.getDownloadStatus= BDSP_Raaga_P_GetDownloadStatus;
    pRaaga->device.initialize = BDSP_Raaga_P_Initialize;
    pRaaga->device.getRRRAddrRange= BDSP_Raaga_P_GetRRRAddrRange;
#if !B_REFSW_MINIMAL
    pRaaga->device.getDefaultDatasyncSettings = BDSP_Raaga_P_GetDefaultDatasyncSettings;
#endif /*!B_REFSW_MINIMAL*/
    pRaaga->device.getDefaultTsmSettings = BDSP_Raaga_P_GetDefaultTsmSettings;
    pRaaga->device.runDebugService = NULL;

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
    const BDSP_UsageOptions      *pUsage,
    BBOX_Handle                   boxHandle,
    BDSP_MemoryEstimate          *pEstimate /*[out]*/
)
{
    BERR_Code ret= BERR_SUCCESS;
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pEstimate);
    BDBG_ASSERT(NULL != pUsage);

    ret = BDSP_Raaga_P_GetMemoryEstimate(pSettings,pUsage,boxHandle, pEstimate);

    return ret;
}

BERR_Code BDSP_GetDefaultAlgorithmSettings(
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

/***************************************************************************
Name        :    BDSP_Raaga_RunDebugService
Type        :    PI Interface

Input       :    hDsp        -   Device handle.
                 dspIndex    -   dspIndex where debug service to be run

 Return	    :	 Error Code to return SUCCESS or FAILURE

 Functionality	 :
	 1) Trigger DBG_service if enabled, Currently this is not supported
***************************************************************************/
BERR_Code BDSP_Raaga_RunDebugService(
    BDSP_Handle hDsp,
    uint32_t dspIndex
)
{
    BSTD_UNUSED(hDsp);
    BSTD_UNUSED(dspIndex);
    BDBG_ERR(("Debug service is not supported"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
