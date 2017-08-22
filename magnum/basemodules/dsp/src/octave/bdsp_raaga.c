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

#include "bdsp_raaga_priv_include.h"

#ifdef FIREPATH_BM
#include "mutex.h"
extern BEMU_Client_MutexHandle g_hSocketMutex;
#endif /* FIREPATH_BM */

BDBG_MODULE(bdsp_raaga);

/***********************************************************************
Name		 :	 BDSP_Raaga_GetDefaultSettings

Type		 :	 PI Interface

Input		 :	 pSettings    -	 Pointer to the structure provided by PI.

Return 	 	:	 None

Functionality	 :
 1)  Fill the default setting to PI to continue ahead with Opening of the device.
***********************************************************************/
void BDSP_Raaga_GetDefaultSettings(
BDSP_RaagaSettings *pSettings	   /* [out] */
)
{
	unsigned i;
	BDBG_ASSERT(NULL != pSettings);
	BKNI_Memset(pSettings, 0, sizeof(*pSettings));

#if BDSP_IMG_SUPPORT
	pSettings->pImageContext = BDSP_IMG_Context;
	pSettings->pImageInterface = &BDSP_IMG_Interface;
#endif

	pSettings->authenticationEnabled = false;
	pSettings->preloadImages         = false;

	/* All debug features will be disabled by default */
	pSettings->debugSettings[BDSP_Raaga_DebugType_eUart].enabled 		= false;
	pSettings->debugSettings[BDSP_Raaga_DebugType_eDramMsg].enabled		= false;
	pSettings->debugSettings[BDSP_Raaga_DebugType_eCoreDump].enabled 	= false;
	pSettings->debugSettings[BDSP_Raaga_DebugType_eTargetPrintf].enabled= false;

	pSettings->debugSettings[BDSP_Raaga_DebugType_eUart].bufferSize 		= 0x1000;  /*     4 KB by default */
	pSettings->debugSettings[BDSP_Raaga_DebugType_eDramMsg].bufferSize 		= 0x40000; /* 256 KB by default */
	pSettings->debugSettings[BDSP_Raaga_DebugType_eCoreDump].bufferSize 	= 0x90000; /* 576 KB by default */
	pSettings->debugSettings[BDSP_Raaga_DebugType_eTargetPrintf].bufferSize = BDSP_IMG_TB_BUF_MEM_SIZE;

	/*initialized to "0" to make sure that the initialization is done properly. Testing cant be done unless a new algoType is explicitly initialized */
	for(i=0;i < BDSP_AlgorithmType_eMax; i++){
		pSettings->maxAlgorithms[i]= 0;
	}

	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioDecode]        = BDSP_RAAGA_MAX_DECODE_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioPassthrough]   = BDSP_RAAGA_MAX_PASSTHROUGH_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEncode]        = BDSP_RAAGA_MAX_ENCODE_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioMixer]         = BDSP_RAAGA_MAX_MIXER_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEchoCanceller] = BDSP_RAAGA_MAX_ECHOCANCELLER_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioProcessing]    = BDSP_RAAGA_MAX_AUDIO_PROCESSING_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eVideoDecode] = 0;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eVideoEncode] = 0;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eSecurity] = 0;

	pSettings->NumDsp  	     = BDSP_RAAGA_MAX_DSP;
	pSettings->numCorePerDsp = BDSP_RAAGA_MAX_CORE_PER_DSP;
}

 /***********************************************************************
 Name		 :	 BDSP_Raaga_Open

 Type		 :	 PI Interface

 Input		 :	 pDsp		 -	 Device Handle which is returned to the PI.
				 chpHandle	 -	 Chip Handle provided by the PI.
				 regHandle	 -	 Register Handle provided by the PI.
				 memHandle	 -	 Memory Handle provided by the PI.
				 intHandle        -	 Interrupt Handle provided by the PI.
				 tmrHandle	 -	 Timer Handle provided by the PI.
				 boxHandle	 -	 BOX Mode handle provided by the PI.
				 pSettings	 -	 Device Settings provided by the PI to open the Raaga Open.

 Return 		 :	 Error Code to return SUCCESS or FAILURE

 Functionality	 :
	 1)  Allocate the memory for the Raaga device.
	 2)  Intialise all the function pointers which will be used by the PI for further processing.
	 3)  Store the Register, Chip, Memory, Interrupt, Timer handles and Settings for Opening the Device provided by PI.
	 4)  Intialise the Device Settings, Enable the Power management and Reset the DSP.
	 5)  Call the Inetrnal BDSP function "BDSP_Raaga_P_Open".
	 6)  Boot the DSP if authentication is disabled.
	 7)  Return the DSP device back to PI.
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
	BERR_Code errCode= BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_Open);

	BDBG_ASSERT(NULL != pDsp);
	BDBG_ASSERT(NULL != chpHandle);
	BDBG_ASSERT(NULL != regHandle);
	BDBG_ASSERT(NULL != memHandle);
	BDBG_ASSERT(NULL != intHandle);
#ifndef FIREPATH_BM
	BDBG_ASSERT(NULL != boxHandle);
#endif /*FIREPATH_BM*/
	BDBG_ASSERT(NULL != pSettings);

	/* tmr is not required */
	BSTD_UNUSED(tmrHandle);
	/* Alloc raaga device */
	pRaaga = BKNI_Malloc(sizeof(BDSP_Raaga));
	if ( NULL == pRaaga )
	{
	    return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
	BKNI_Memset(pRaaga, 0, sizeof(BDSP_Raaga));

	/* Init device */
	BDSP_P_InitDevice(&pRaaga->device, pRaaga);
	pRaaga->device.close = BDSP_Raaga_P_Close;
	pRaaga->device.getDefaultContextSettings = BDSP_Raaga_P_GetDefaultContextSettings;
	pRaaga->device.createContext = BDSP_Raaga_P_CreateContext;
	pRaaga->device.getStatus= BDSP_Raaga_P_GetStatus;
	pRaaga->device.powerStandby= NULL;
	pRaaga->device.powerResume= NULL;
	pRaaga->device.getAlgorithmInfo= BDSP_Raaga_P_GetAlgorithmInfo;
	pRaaga->device.allocateExternalInterrupt = NULL;
	pRaaga->device.freeExternalInterrupt = NULL;
	pRaaga->device.getExternalInterruptInfo = NULL;
	pRaaga->device.processAudioCapture = NULL;

	/* Init context lists */
	BLST_S_INIT(&pRaaga->contextList);

#if 0
	/* Init interrupt list */
	BLST_S_INIT(&pRaaga->interruptList);
#endif /* CDN TODO */
	/* Save Settings and params */
	pRaaga->deviceSettings = *pSettings;
	pRaaga->chpHandle = chpHandle;
	pRaaga->regHandle = regHandle;
	pRaaga->memHandle = memHandle;
	pRaaga->intHandle = intHandle;
	pRaaga->boxHandle = boxHandle;

	BDBG_OBJECT_SET(pRaaga, BDSP_Raaga);

#if 0
	{
		BDSP_RaagaUsageOptions Usage;
		BDSP_RaagaMemoryEstimate Estimate;
		BKNI_Memset(&Usage,0, sizeof(BDSP_RaagaUsageOptions));
		Usage.Codeclist[BDSP_Algorithm_eMpegAudioDecode]   =true;
		Usage.Codeclist[BDSP_Algorithm_ePcmWavDecode]      =true;
		Usage.Codeclist[BDSP_Algorithm_eAacAdtsDecode]     =true;
		Usage.Codeclist[BDSP_Algorithm_eAacAdtsPassthrough]=true;
		Usage.Codeclist[BDSP_Algorithm_eAacLoasDecode]     =true;
		Usage.Codeclist[BDSP_Algorithm_eAacLoasPassthrough]=true;
		Usage.Codeclist[BDSP_Algorithm_eAc3Decode]         =true;
		Usage.Codeclist[BDSP_Algorithm_eAc3Passthrough]    =true;
		Usage.Codeclist[BDSP_Algorithm_eAc3PlusDecode]     =true;
		Usage.Codeclist[BDSP_Algorithm_eAc3PlusPassthrough]=true;
		Usage.Codeclist[BDSP_Algorithm_eSrc]               =true;

		Usage.NumAudioDecoders=1;
		Usage.NumAudioPostProcesses=2;
		Usage.NumAudioPassthru=1;
		BDSP_Raaga_GetMemoryEstimate(pSettings,
			&Usage,
			boxHandle,
			&Estimate);
		BDBG_ERR(("Memory Required FIRMWARE = %d bytes(%d KB)(%d MB)  GENERAL = %d bytes(%d KB)(%d MB)",Estimate.FirmwareMemory,(Estimate.FirmwareMemory/1024),(Estimate.FirmwareMemory/(1024*1024)),
						Estimate.GeneralMemory,(Estimate.GeneralMemory/1024),(Estimate.GeneralMemory/(1024*1024))));
	}
#endif /* This is memory estimate testing */

	errCode = BDSP_Raaga_P_InitDeviceSettings(pRaaga);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_Open: Error in Initialising Settings for Raaga"));
		errCode = BERR_TRACE(errCode);
		goto err_open;
	}
	BDSP_Raaga_P_EnableAllPwrResource(pRaaga->device.pDeviceHandle, true);

	errCode = BDSP_Raaga_P_Reset(pRaaga);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_Open: Error in RESET of Raaga"));
		errCode = BERR_TRACE(errCode);
		goto err_open;
	}

	errCode = BDSP_Raaga_P_Open(pRaaga);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_Open: Error in Opening and processing Open for Raaga"));
		errCode = BERR_TRACE(errCode);
		goto err_open;
	}

	if(pRaaga->deviceSettings.authenticationEnabled == false)
	{
		errCode = BDSP_Raaga_P_Boot(pRaaga);
		if (errCode!=BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_Open: Error in Booting Raaga"));
			errCode= BERR_TRACE(errCode);
			goto err_open;
		}

        errCode = BDSP_Raaga_P_CheckDspAlive(pRaaga);
        if (errCode!=BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_Open: DSP not alive"));
            errCode = BERR_TRACE(errCode);
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
	BDBG_LEAVE(BDSP_Raaga_Open);
	return errCode;
}

BERR_Code BDSP_Raaga_GetDownloadStatus(
    BDSP_Handle handle,
    BDSP_Raaga_DownloadStatus *pStatus /* [out] */
)
{
	BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_GetDownloadStatus);
    /* Assert the function arguments*/
    BDBG_ASSERT(handle->pDeviceHandle);

    /*If Firmware Firmware authentication is Disabled*/
    if(pDevice->deviceSettings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Raaga_GetDownloadStatus should be called only if bFwAuthEnable is true"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    pStatus->pBaseAddress    = pDevice->memInfo.sROMemoryPool.Memory.pAddr;
    pStatus->physicalAddress = pDevice->memInfo.sROMemoryPool.Memory.offset;
    pStatus->length          = pDevice->memInfo.sROMemoryPool.ui32Size;

    BDBG_LEAVE(BDSP_Raaga_GetDownloadStatus);

    return errCode;
}

BERR_Code BDSP_Raaga_GetAudioDelay_isrsafe(
    BDSP_CTB_Input   *pCtbInput,
    BDSP_StageHandle  hStage,
    BDSP_CTB_Output  *pCTBOutput
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hStage, BDSP_Stage);

	pCTBOutput->ui32AudOffset = BDSP_AF_P_MAX_AUD_OFFSET;
	pCTBOutput->ui32BlockTime = BDSP_AF_P_BLOCKING_TIME;
	pCTBOutput->ui32Threshold = BDSP_AF_P_MAX_THRESHOLD+BDSP_AF_P_SAMPLE_PADDING;

	BSTD_UNUSED(pCtbInput);
    return errCode;
}

/***************************************************************************
Summary:
Get the codec copability status
***************************************************************************/
void BDSP_Raaga_GetCodecCapabilities(
	BDSP_CodecCapabilities *pSetting
)
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
            pSetting->dolbyMs.dapv2 = false;
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

BERR_Code BDSP_Raaga_Initialize(
	BDSP_Handle handle
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_Raaga *pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

    BDBG_ENTER(BDSP_Raaga_Initialize);
    /* Assert the function arguments*/
    BDBG_OBJECT_SET(pDevice, BDSP_Raaga);

    /*If Firmware authentication is Disabled*/
    if(pDevice->deviceSettings.authenticationEnabled==false)
    {
        BDBG_ERR(("BDSP_Raaga_Initialize should be called only if bFwAuthEnable is true"));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto end;
    }

    errCode = BDSP_Raaga_P_Boot(pDevice);
    if (errCode!=BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_Initialize: Error in Booting Raaga"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

    errCode = BDSP_Raaga_P_CheckDspAlive(pDevice);
    if (errCode!=BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_Initialize: DSP not alive"));
        errCode = BERR_TRACE(errCode);
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Raaga_Initialize);
    return errCode;
}

BERR_Code BDSP_Raaga_GetDefaultAlgorithmSettings(
    BDSP_Algorithm algorithm,
    void *pSettingsBuffer,        /* [out] */
    size_t settingsBufferSize   /*[In]*/
)
{
    const BDSP_P_AlgorithmInfo *pAlgoInfo;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
    BDBG_ASSERT(NULL != pSettingsBuffer);

	pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
    if(false == pAlgoSupportInfo->supported)
    {
		BDBG_ERR(("Algorithm Requested(%d) %s is not supported",pAlgoSupportInfo->algorithm, pAlgoSupportInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
    if(pAlgoInfo->algoUserConfigSize != settingsBufferSize)
    {
        BDBG_ERR(("settingsBufferSize (%lu) provided by PI is not equal to Config size (%lu) required for algorithm %u (%s)",
            (unsigned long)settingsBufferSize, (unsigned long)pAlgoInfo->algoUserConfigSize, algorithm, pAlgoInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (settingsBufferSize > 0)
    {
        BDBG_ASSERT(NULL != pAlgoInfo->pDefaultUserConfig);
        BKNI_Memcpy(pSettingsBuffer, pAlgoInfo->pDefaultUserConfig, settingsBufferSize);
    }

    return BERR_SUCCESS;
}

#if !B_REFSW_MINIMAL
BERR_Code BDSP_AudioTask_GetDefaultDatasyncSettings(
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
)
{
    if(sizeof(BDSP_AudioTaskDatasyncSettings) != settingsBufferSize)
    {
        BDBG_ERR(("settingsBufferSize (%lu) is not equal to Config size (%lu) of DataSync ",
            (unsigned long)settingsBufferSize,(unsigned long)sizeof(BDSP_AudioTaskDatasyncSettings)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_Memcpy((void *)(volatile void *)pSettingsBuffer,(void *)&BDSP_sDefaultFrameSyncSettings,settingsBufferSize);

    return BERR_SUCCESS;
}
#endif /*!B_REFSW_MINIMAL*/

BERR_Code BDSP_AudioTask_GetDefaultTsmSettings(
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
)
{
    if(sizeof(BDSP_AudioTaskTsmSettings) != settingsBufferSize)
    {
        BDBG_ERR(("settingsBufferSize (%lu) is not equal to Config size (%lu) of DataSync ",
            (unsigned long)settingsBufferSize,(unsigned long)sizeof(BDSP_AudioTaskTsmSettings)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_Memcpy((void *)(volatile void *)pSettingsBuffer,(void *)&BDSP_sDefaultTSMSettings,settingsBufferSize);

    return BERR_SUCCESS;
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
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_GetMemoryEstimate);
	BDBG_ASSERT(NULL != pSettings);
	BDBG_ASSERT(NULL != pEstimate);
	BDBG_ASSERT(NULL != pUsage);

	errCode = BDSP_Raaga_P_GetMemoryEstimate(pSettings, pUsage,boxHandle,pEstimate);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_GetMemoryEstimate: Error in calculating Memory estimate"));
	}
	BDBG_LEAVE(BDSP_Raaga_GetMemoryEstimate);
	return errCode;
}

/***********************************************************************
Name		:	BDSP_Raaga_Open

Type		:	PI Interface

Input		:	handle		-	Device Handle which is returned to the PI.
				debugtype      -     Describes which debug buffer is being probed by PI.
				dspIndex		-     For which Dsp the debug buffer is being probed by PI.
				pBuffer           -     MMA memory handle for PI to copy the data.
				psize              -     Size returned back to PI to copy.
Return		:	Error Code to return SUCCESS or FAILURE

Functionality	:
	1)	Allocate the memory for the Raaga device.
***********************************************************************/
BERR_Code BDSP_Raaga_GetDebugBuffer(
    BDSP_Handle 			handle,
    BDSP_Raaga_DebugType 	debugType,
    uint32_t 				dspIndex,
    BDSP_MMA_Memory 	   *pBuffer,
    size_t 				   *pSize
)
{
	BERR_Code errCode = BERR_SUCCESS ;
	BDSP_Raaga *pDevice;
	uint32_t ui32Offset, ui32RegOffset;
	BDSP_MMA_Memory Memory;
    dramaddr_t  BaseAddr, ReadAddr, WriteAddr, EndAddr;

	BDBG_ENTER(BDSP_Raaga_GetDebugBuffer);
	BDBG_ASSERT(handle->pDeviceHandle);
	BDBG_ASSERT(pBuffer);
	BDBG_ASSERT(pSize);

	pDevice = (BDSP_Raaga *)handle->pDeviceHandle;
	BDBG_ASSERT((dspIndex+1) <= pDevice->numDsp);

	ui32Offset = pDevice->dspOffset[dspIndex];
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    if(debugType != BDSP_Raaga_DebugType_eTargetPrintf)
    {
		ReadAddr = BDSP_ReadReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
			(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId)+
			BDSP_RAAGA_P_FIFO_READ_OFFSET);

		WriteAddr = BDSP_ReadReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
			(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId)+
			BDSP_RAAGA_P_FIFO_WRITE_OFFSET);

		BaseAddr = BDSP_ReadReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
			(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId)+
			BDSP_RAAGA_P_FIFO_BASE_OFFSET);

		EndAddr = BDSP_ReadReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
			(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId)+
			BDSP_RAAGA_P_FIFO_END_OFFSET);

		Memory = pDevice->memInfo.debugQueueParams[dspIndex][debugType].Memory;
		Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + (ReadAddr - BaseAddr));
		if(ReadAddr > WriteAddr)
		{
			/*Write Pointer has looparound, hence reach just the bottom chunk */
			*pSize = EndAddr - ReadAddr;
			BDBG_MSG(("Write Pointer has looped around, reading just the chunk till END"));
		}
		else
		{
			*pSize = WriteAddr - ReadAddr;
		}
		*pBuffer = Memory;
	}
	else
	{
	    TB_data_descriptor DataDescriptor;
		unsigned int ReadAmount = 0;
#ifdef FIREPATH_BM
        if (0 != BEMU_Client_AcquireMutex(g_hSocketMutex))
        {
            BDBG_ERR(("Failed to acquire mutex in BEMU_Client_CloseSocket\n"));
        }
#endif /* FIREPATH_BM */
		TB_peek(&(pDevice->sTbTargetPrint[dspIndex]), &DataDescriptor);

#ifdef FIREPATH_BM
		BEMU_Client_ReleaseMutex(g_hSocketMutex);
#endif /* FIREPATH_BM */
		*pSize = 0;
		Memory = pDevice->memInfo.debugQueueParams[dspIndex][BDSP_Raaga_DebugType_eTargetPrintf].Memory;
		while((ReadAmount = TB_read(&DataDescriptor, (void *)((uint8_t *)Memory.pAddr+ *pSize), BDSP_IMG_TB_BUF_MEM_SIZE, true)) > 0)
		{
			*pSize += ReadAmount;
		}
		if(ReadAmount == BDSP_IMG_TB_BUF_MEM_SIZE)
		{
			BDBG_ERR(("SDK Target Printf buffer got Full"));
		}
		*pBuffer = Memory;
	}

	BDBG_MSG(("Read (%lu) bytes of data", (unsigned long)(*pSize)));
	BDBG_LEAVE(BDSP_Raaga_GetDebugBuffer);
	return errCode;
}

BERR_Code BDSP_Raaga_ConsumeDebugData(
    BDSP_Handle 			handle,
    BDSP_Raaga_DebugType 	debugType,
    uint32_t 				dspIndex,
    size_t 					bytesConsumed
)
{
	BERR_Code errCode = BERR_SUCCESS ;
	BDSP_Raaga *pDevice;
	uint32_t ui32Offset, ui32RegOffset;
    dramaddr_t  BaseAddr, ReadAddr, EndAddr;

	BDBG_ENTER(BDSP_Raaga_ConsumeDebugData);
    BDBG_ASSERT(handle->pDeviceHandle);
    pDevice = (BDSP_Raaga *)handle->pDeviceHandle;

	BDBG_ASSERT((dspIndex+1) <= pDevice->numDsp);

	ui32Offset = pDevice->dspOffset[dspIndex];
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
                    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    if(debugType != BDSP_Raaga_DebugType_eTargetPrintf)
    {

		ReadAddr = BDSP_ReadReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
				(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId)+
				BDSP_RAAGA_P_FIFO_READ_OFFSET);

		BaseAddr = BDSP_ReadReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
			(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId)+
			BDSP_RAAGA_P_FIFO_BASE_OFFSET);

		EndAddr = BDSP_ReadReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
			(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId)+
			BDSP_RAAGA_P_FIFO_END_OFFSET);

		ReadAddr += bytesConsumed;
		if(ReadAddr >= EndAddr)
		{
			/* We never handle looparound read and if write pointer had looped around, PI would only read bottom chunk,
			     hence adjusting the read to base would sufficient*/
			ReadAddr = BaseAddr;
		}

		BDSP_WriteReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32Offset +
				(ui32RegOffset * pDevice->memInfo.debugQueueParams[dspIndex][debugType].ui32FifoId) +
				BDSP_RAAGA_P_FIFO_READ_OFFSET,
			ReadAddr); /* read */
    }
	else
	{
#ifdef FIREPATH_BM
		if (0 != BEMU_Client_AcquireMutex(g_hSocketMutex))
		{
			BDBG_ERR(("Failed to acquire mutex in BEMU_Client_CloseSocket\n"));
		}
#endif /* FIREPATH_BM */

		TB_discard(&pDevice->sTbTargetPrint[dspIndex], bytesConsumed);

#ifdef FIREPATH_BM
		BEMU_Client_ReleaseMutex(g_hSocketMutex);
#endif /* FIREPATH_BM */
	}
	BDBG_ENTER(BDSP_Raaga_ConsumeDebugData);
	return errCode;
}

BDSP_Raaga_FwStatus BDSP_Raaga_GetCoreDumpStatus (
    BDSP_Handle handle,
    uint32_t dspIndex /* [in] Gives the DSP Id for which the core dump status is required */
)
{
	BSTD_UNUSED(handle);
	BSTD_UNUSED(dspIndex);

	BDBG_ERR(("BDSP_Raaga_GetCoreDumpStatus: Core Dump is not implemented"));
	return BDSP_Raaga_FwStatus_eCoreDumpComplete;
}
