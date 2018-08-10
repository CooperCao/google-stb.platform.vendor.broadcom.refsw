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

#include "bdsp_raaga_priv_include.h"

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
	pSettings->debugSettings[BDSP_DebugType_eUart].enabled 		= false;
	pSettings->debugSettings[BDSP_DebugType_eDramMsg].enabled	= false;
	pSettings->debugSettings[BDSP_DebugType_eCoreDump].enabled 	= false;
	pSettings->debugSettings[BDSP_DebugType_eTargetPrintf].enabled= false;

	pSettings->debugSettings[BDSP_DebugType_eUart].bufferSize 		= 0x1000;  /*     4 KB by default */
	pSettings->debugSettings[BDSP_DebugType_eDramMsg].bufferSize 	= 0x40000; /* 256 KB by default */
	pSettings->debugSettings[BDSP_DebugType_eCoreDump].bufferSize 	= 0x90000; /* 576 KB by default */
	pSettings->debugSettings[BDSP_DebugType_eTargetPrintf].bufferSize = BDSP_TARGET_BUF_MEM_SIZE;

	/*initialized to "0" to make sure that the initialization is done properly. Testing cant be done unless a new algoType is explicitly initialized */
	for(i=0;i < BDSP_AlgorithmType_eMax; i++){
		pSettings->maxAlgorithms[i]= 0;
	}

        /*initialized to "0" to make sure that the initialization is done properly */
        for(i=0;i < BDSP_RAAGA_MAX_NUM_HEAPS; i++){
            pSettings->heap[i].baseAddress = (BSTD_DeviceOffset) 0;
            pSettings->heap[i].size = (uint64_t) 0;
        }

	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioDecode]        = BDSP_MAX_DECODE_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioPassthrough]   = BDSP_MAX_PASSTHROUGH_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEncode]        = BDSP_MAX_ENCODE_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioMixer]         = BDSP_MAX_MIXER_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioEchoCanceller] = BDSP_MAX_ECHOCANCELLER_CTXT;
	pSettings->maxAlgorithms[BDSP_AlgorithmType_eAudioProcessing]    = BDSP_MAX_AUDIO_PROCESSING_CTXT;
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
    pRaaga->device.getDownloadStatus = BDSP_Raaga_P_GetDownloadStatus;
    pRaaga->device.initialize = BDSP_Raaga_P_Initialize;
    pRaaga->device.getRRRAddrRange = NULL;
    pRaaga->device.processPAK = NULL;

#ifdef BDSP_RAAGA_DEBUG_SERVICE
    pRaaga->device.runDebugService = BDSP_Raaga_P_RunDebugService;
#else
    pRaaga->device.runDebugService = NULL;
#endif /* BDSP_RAAGA_DEBUG_SERVICE */

#if !B_REFSW_MINIMAL
    pRaaga->device.getDefaultDatasyncSettings = BDSP_P_GetDefaultDatasyncSettings;
#endif /*!B_REFSW_MINIMAL*/
    pRaaga->device.getDefaultTsmSettings = BDSP_P_GetDefaultTsmSettings;

	/* Init context lists */
	BLST_S_INIT(&pRaaga->contextList);
    /* Init interrupt list */
	BLST_S_INIT(&pRaaga->interruptList);

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
		BDSP_UsageOptions Usage;
		BDSP_MemoryEstimate Estimate;
		BKNI_Memset(&Usage,0, sizeof(BDSP_UsageOptions));
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
		Usage.Codeclist[BDSP_Algorithm_eUdcDecode]         =true;
		Usage.Codeclist[BDSP_Algorithm_eUdcPassthrough]    =true;
		Usage.Codeclist[BDSP_Algorithm_eSrc]               =true;
        /*MS12 configurations*/
		Usage.Codeclist[BDSP_Algorithm_eDpcmr]             =true;
		Usage.Codeclist[BDSP_Algorithm_eDDPEncode]         =true;
		Usage.Codeclist[BDSP_Algorithm_eMixerDapv2]        =true;

        /*Transcode configurations*/
		/*Usage.Codeclist[BDSP_Algorithm_eGenCdbItb]         =true;
		Usage.Codeclist[BDSP_Algorithm_eAacEncode]         =true;
		Usage.Codeclist[BDSP_Algorithm_eMixer]             =true;*/

		Usage.NumAudioDecoders=4;
		Usage.NumAudioPostProcesses=4;
		Usage.NumAudioPassthru=0;
        Usage.NumAudioEncoders=1;
        Usage.NumAudioMixers=1;
        Usage.IntertaskBufferDataType=BDSP_DataType_ePcm7_1;

		BDSP_Raaga_GetMemoryEstimate(pSettings,
			&Usage,
			boxHandle,
			&Estimate);
		BDBG_ERR(("Memory Required FIRMWARE = %d bytes(%d KB)(%d MB)  GENERAL = %d bytes(%d KB)(%d MB)",Estimate.FirmwareMemory,(Estimate.FirmwareMemory/1024),(Estimate.FirmwareMemory/(1024*1024)),
						Estimate.GeneralMemory,(Estimate.GeneralMemory/1024),(Estimate.GeneralMemory/(1024*1024))));
	}
#endif /* This is for memory estimate testing */

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

/***************************************************************************
Name        :    BDSP_Raaga_RunDebugService
Type        :    PI Interface

Input       :    hDsp        -   Device handle.
                 dspIndex    -   dspIndex where debug service to be run

 Return	    :	 Error Code to return SUCCESS or FAILURE

 Functionality	 :
	 1) Trigger DBG_service if enabled
***************************************************************************/
BERR_Code BDSP_Raaga_RunDebugService(
    BDSP_Handle hDsp,
    uint32_t dspIndex
)
{
    BERR_Code   errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hDsp, BDSP_Device);

    if(hDsp->runDebugService)
    {
        errCode = hDsp->runDebugService(hDsp->pDeviceHandle, dspIndex);
        if(BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("Failed to run Debug service"));
	    BERR_TRACE(BERR_UNKNOWN);
        }
    }
    else
    {
        BDBG_ERR(("Debug service is not supported"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    return errCode;
}
