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

BDBG_MODULE(bdsp_raaga_priv);
BDBG_OBJECT_ID(BDSP_Raaga);
BDBG_OBJECT_ID(BDSP_RaagaContext);
BDBG_OBJECT_ID(BDSP_RaagaTask);
BDBG_OBJECT_ID(BDSP_RaagaStage);

#ifdef FIREPATH_BM
 extern DSP dspInst;
#endif
const BDSP_Version BDSP_sRaagaVersion = {BDSP_RAAGA_MAJOR_VERSION, BDSP_RAAGA_MINOR_VERSION, BDSP_RAAGA_BRANCH_VERSION, BDSP_RAAGA_BRANCH_SUBVERSION};

static unsigned BDSP_Raaga_P_GetFreeTaskId(BDSP_P_TaskInfo *pTaskInfo)
{
	unsigned taskId =0;
	for (taskId = 0 ; taskId < BDSP_RAAGA_MAX_FW_TASK_PER_DSP; taskId++)
	{
		if (pTaskInfo->taskId[taskId] == false)
		{
			pTaskInfo->taskId[taskId] = true;
			return taskId;
		}
	}
	return BDSP_P_INVALID_TASK_ID;
}

static void BDSP_Raaga_P_ReleaseTaskId(BDSP_P_TaskInfo *pTaskInfo, unsigned *taskId)
{
	pTaskInfo->taskId[*taskId] = false;
	*taskId = BDSP_P_INVALID_TASK_ID;
}

/***********************************************************************
 Name		 :	 BDSP_Raaga_P_GetNumberOfDspandCores

 Type		 :	 BDSP Internal

 Input		 :	 boxHandle	 -	 Box Handle provided by the PI.
				 pNumDsp       -	 Pointer to return the number of DSP in the System.
				 pNumCores    -	 Pointer to return the number of Cores per DSP in the System.

 Return 	 :	 Error Code to return SUCCESS or FAILURE

 Functionality	 :	 Following are the operations performed.
		 1)  Calculate the Number of DSP  and Number of cores based on the BOX MODE.
 ***********************************************************************/
BERR_Code BDSP_Raaga_P_GetNumberOfDspandCores(
	BBOX_Handle boxHandle,
	unsigned *pNumDsp,
	unsigned *pNumCores
)
{
	BBOX_Config *pConfig;
	BERR_Code ret = BERR_SUCCESS;

	pConfig = (BBOX_Config *)BKNI_Malloc(sizeof(BBOX_Config));
    if(pConfig == NULL)
    {
        BDBG_ERR(("BDSP_Raaga_P_GetNumberOfDspandCores: Unable to allocate memory for BOX Config"));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
	BKNI_Memset(pConfig, 0 , sizeof(BBOX_Config));

	ret = BBOX_GetConfig(boxHandle, pConfig);
	if(BERR_SUCCESS != ret)
	{
		BDBG_ERR(("BDSP_Raaga_P_GetNumberOfDsp: Error in retrieving the Num DSP from BOX MODE"));
	}
	else
	{
		*pNumDsp = pConfig->stAudio.numDsps;
		/*TODO CDN: Derive the number of cores after updating the BOX Mode API */
		*pNumCores = BDSP_RAAGA_MAX_CORE_PER_DSP;
	}
	BKNI_Free(pConfig);
	return ret;
}

static void BDSP_Raaga_P_ValidateCodeDownloadSettings(BDSP_Raaga *pDevice)
{
	unsigned i=0;

	for(i=0;i<BDSP_AlgorithmType_eMax ; i++ )
	{
		if (pDevice->deviceSettings.maxAlgorithms[i] > BDSP_RAAGA_MAX_DOWNLOAD_BUFFERS )
		{
			BDBG_ERR(("Maximum number of Algorithms for Algorithm type(%d) is %d which is exceeding downloading limits(%d)",
				i,pDevice->deviceSettings.maxAlgorithms[i],BDSP_RAAGA_MAX_DOWNLOAD_BUFFERS));
			BDBG_ASSERT(0);
		}
	}
}

 /***********************************************************************
 Name		 :	 BDSP_Raaga_P_InitDeviceSettings

 Type		 :	 BDSP Internal

 Input		 :	 pDeviceHandle	 -	 Device Handle

 Return 		 :	 Error Code to return SUCCESS or FAILURE

 Functionality	 :	 Following are the operations performed.
		 1) Set the Lower Clock settings for the DSP.
		 2) Retrieve the Numer of DSPs and Cores.
		 3) Assign the DSP offsets
 ***********************************************************************/
BERR_Code BDSP_Raaga_P_InitDeviceSettings(BDSP_Raaga *pRaaga)
{
	unsigned dspIndex;
#if 0
	unsigned lowerDspClkRate;
#endif/*CDN TODO*/
	BERR_Code err = BERR_SUCCESS;

	pRaaga->numDsp       = BDSP_RAAGA_MAX_DSP;
	pRaaga->numCorePerDsp= BDSP_RAAGA_MAX_CORE_PER_DSP;

#if 0
	for (dspIndex=0 ; dspIndex < pRaaga->numDsp; dspIndex++)
	{
		err = BDSP_Raaga_P_GetDefaultClkRate(pDeviceHandle, dspIndex, &pRaaga->dpmInfo.defaultDspClkRate[dspIndex] );
		if( err == BERR_SUCCESS )
		{
			BDSP_Raaga_P_GetLowerDspClkRate(pRaaga->dpmInfo.defaultDspClkRate[dspIndex], &lowerDspClkRate);
			BDSP_Raaga_P_SetDspClkRate( pDeviceHandle, lowerDspClkRate, dspIndex );
			BDBG_MSG(("PWR: Dynamic frequency scaling enabled for DSP %d", dspIndex));
		}
		else
		{
			BDBG_MSG(("PWR: Dynamic frequency scaling not enabled for DSP %d", dspIndex));
		}
	}
#endif /*CDN TODO*/
	err = BDSP_Raaga_P_GetNumberOfDspandCores( pRaaga->boxHandle, &pRaaga->numDsp, &pRaaga->numCorePerDsp);
	if(err != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_InitDeviceSettings: Error in retreiving the Number of DSPs and CorePerDsp from BOX MODE"));
		BDBG_ERR(("Falling back to default value, DSP (%d) CorePerDsp(%d)",pRaaga->numDsp,pRaaga->numCorePerDsp));
	}

	for (dspIndex=0 ; dspIndex < pRaaga->numDsp; dspIndex++)
	{
#ifdef BCHP_RAAGA_DSP_RGR_1_REG_START
		pRaaga->dspOffset[dspIndex] = dspIndex * (BCHP_RAAGA_DSP_RGR_1_REVISION - BCHP_RAAGA_DSP_RGR_REVISION);
#else
		pRaaga->dspOffset[dspIndex] = dspIndex * 0; /*(BCHP_RAAGA_DSP1_RGR_REVISION - BCHP_RAAGA_DSP0_RGR_REVISION)*/
#endif
	}
	return BERR_SUCCESS;
}

static void BDSP_Raaga_P_InitDevice(BDSP_Raaga *pDevice )
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned dspindex=0, index=0;

    BDBG_ENTER(BDSP_Raaga_P_InitDevice);

	errCode = BKNI_CreateMutex(&(pDevice->deviceMutex));
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("Unable to Allocate the Mutex for Device"));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		BDBG_ASSERT(0);
	}

	for (dspindex=0; dspindex<pDevice->numDsp; dspindex++)
	{
        errCode = BKNI_CreateEvent(&(pDevice->hEvent[dspindex]));
        if (BERR_SUCCESS != errCode)
        {
            BDBG_ERR(("Unable to create event for DSP %d",dspindex));
            errCode = BERR_TRACE(errCode);
            BDBG_ASSERT(0);
        }
        BKNI_ResetEvent(pDevice->hEvent[dspindex]);

		for (index=0;index<BDSP_RAAGA_NUM_FIFOS;index++)
		{
			pDevice->hardwareStatus.dspFifo[dspindex][index] = false;
		}

		for (index=0;index<BDSP_RAAGA_MAX_DESCRIPTORS;index++)
		{
			pDevice->hardwareStatus.descriptor[dspindex][index] = false;
		}

		for (index=0 ; index< BDSP_RAAGA_MAX_FW_TASK_PER_DSP; index++)
		{
			pDevice->taskDetails[dspindex].taskId[index] = false;
			pDevice->taskDetails[dspindex].pTask[index]  = NULL;
			pDevice->taskDetails[dspindex].numActiveTasks= 0;
		}
	}
    BDBG_LEAVE(BDSP_Raaga_P_InitDevice);
}

static void BDSP_Raaga_P_DeInitDevice( BDSP_Raaga *pDevice)
{
	unsigned dspindex=0, index=0;

	BDBG_ENTER(BDSP_Raaga_P_DeInitDevice);

	BKNI_DestroyMutex(pDevice->deviceMutex);
	for (dspindex=0; dspindex<pDevice->numDsp; dspindex++)
	{
	    BKNI_DestroyEvent(pDevice->hEvent[dspindex]);
		for (index=0;index<BDSP_RAAGA_NUM_FIFOS;index++)
		{
			pDevice->hardwareStatus.dspFifo[dspindex][index] = false;
		}

		for (index=0;index<BDSP_RAAGA_MAX_DESCRIPTORS;index++)
		{
			pDevice->hardwareStatus.descriptor[dspindex][index] = false;
		}
	}
	BDBG_LEAVE(BDSP_Raaga_P_DeInitDevice);
}

static BERR_Code BDSP_Raaga_P_InitDebugInfrastructure(
	BDSP_Raaga *pDevice
)
{
	BERR_Code errCode = BERR_SUCCESS;
	uint32_t ui32RegOffset = 0, ui32DspOffset =0, ui32FifoId =0;
    dramaddr_t  BaseOffset=0, EndOffset=0;
	unsigned index =0, dspIndex =0;
	DSP *pDspInst = NULL;
	DSP_PARAMETERS dsp_parameters;
	DSP_RET  eRetVal;

	BDBG_ENTER(BDSP_Raaga_P_InitDebugInfrastructure);

	/* Hardware programming of UART */
	BDSP_Raaga_P_DirectRaagaUartToPort(pDevice->regHandle);

#ifdef FIREPATH_BM
	/* Since Firepath BM implicitly used the libdsp control, the DSP INIT would have happened up ahead, hence not doing it */
	pDspInst = &dspInst;
	DSP_setOption(pDspInst, DSP_OPTION_REFRESH_ATU_INDEX, NULL);
#else
	pDspInst = &pDevice->sLibDsp;
	DSP_initParameters(&dsp_parameters);
	dsp_parameters.hReg = pDevice->regHandle;
	dsp_parameters.hMem = pDevice->memHandle;

	/* The information provided to DSP INIT is the memory which is part of the kernal RW section, which the
	FW SDK uses to write into when prints are invoked in the FW.	The same buffer will to be accessed by the
	HOST LIBDSP to peek/copy/consume the data */
	for(dspIndex=0;dspIndex<pDevice->numDsp;dspIndex++)
	{
		BDSP_MMA_Memory Memory;
		uint32_t ui32PhyOffset =0;

		Memory = pDevice->memInfo.SharedKernalMemory[dspIndex].Buffer;
		ui32PhyOffset = BDSP_IMG_TB_BUF_START_ADDR - BDSP_ATU_VIRTUAL_RW_MEM_START_ADDR;
		if((ui32PhyOffset+BDSP_IMG_TB_BUF_MEM_SIZE) > BDSP_IMG_KERNEL_RW_IMG_SIZE)
		{
			BDBG_ERR(("BDSP_Raaga_P_InitDebugInfrastructure: TARGET PRINTF is trying to access memory outside the RW KERNAL SECTION"));
			errCode = BERR_INVALID_PARAMETER;
			goto end;
		}
		Memory.offset = Memory.offset+ui32PhyOffset;
		Memory.pAddr = (void *)((uint8_t *)Memory.pAddr + ui32PhyOffset);
		dsp_parameters.sMmaBuffer[dspIndex].Buffer	= Memory;
		dsp_parameters.sMmaBuffer[dspIndex].ui32Size= BDSP_IMG_TB_BUF_MEM_SIZE;
	}

	dsp_parameters.ui32MmaBufferValidEntries = pDevice->numDsp;
	eRetVal = DSP_init(pDspInst, &dsp_parameters);
	BDBG_ASSERT(eRetVal == DSP_SUCCESS);
#endif
	for(dspIndex=0;dspIndex<pDevice->numDsp;dspIndex++)
	{
		for(index=0; index< BDSP_Raaga_DebugType_eLast;index++)
		{
			if(pDevice->deviceSettings.debugSettings[index].enabled)
			{
				if(index != BDSP_Raaga_DebugType_eTargetPrintf)
				{
					/* Initilaising the FIFO Registers */
					ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
									BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

					ui32DspOffset = pDevice->dspOffset[dspIndex];
				    BaseOffset = pDevice->memInfo.debugQueueParams[dspIndex][index].Memory.offset;
					ui32FifoId = pDevice->memInfo.debugQueueParams[dspIndex][index].ui32FifoId;
				    EndOffset  = BaseOffset + pDevice->memInfo.debugQueueParams[dspIndex][index].ui32Size;

				    BDSP_WriteReg64( pDevice->regHandle,
				        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32DspOffset +
				            (ui32RegOffset * ui32FifoId) +
				            BDSP_RAAGA_P_FIFO_BASE_OFFSET,
				        BaseOffset); /* base */

				    BDSP_WriteReg64(pDevice->regHandle,
				        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32DspOffset +
				            (ui32RegOffset * ui32FifoId) +
				            BDSP_RAAGA_P_FIFO_READ_OFFSET,
				        BaseOffset); /* read */

				    BDSP_WriteReg64(pDevice->regHandle,
				        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32DspOffset +
				            (ui32RegOffset * ui32FifoId) +
				            BDSP_RAAGA_P_FIFO_WRITE_OFFSET,
				        BaseOffset); /* write */

			       BDSP_WriteReg64(pDevice->regHandle,
				        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + ui32DspOffset +
				            (ui32RegOffset * ui32FifoId) +
				            BDSP_RAAGA_P_FIFO_END_OFFSET,
				        EndOffset); /* end */
				}
				else
				{
					ADDR tbAddr;
					tbAddr.addr_space = ADDR_SPACE_SHARED;
					tbAddr.addr.shared = (SHARED_ADDR)
					       pDevice->memInfo.debugQueueParams[dspIndex][BDSP_Raaga_DebugType_eTargetPrintf].ui32FifoId;
					BDBG_MSG(("Enabling TARGET PRINT"));
					TB_init(&pDevice->sTbTargetPrint[dspIndex],
						pDspInst,
						tbAddr,
						(TB_BUFF_PTR) BDSP_IMG_TB_BUF_START_ADDR,
						ADDR_SPACE_DSP,
						BDSP_IMG_TB_BUF_MEM_SIZE/1024,
						TB_NO_NAME);
				}
			}
		}
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_InitDebugInfrastructure);
	return errCode;
}

BERR_Code BDSP_Raaga_P_CheckDspAlive(BDSP_Raaga *pDevice)
{
	BERR_Code errCode = BERR_SUCCESS;
    unsigned i=0;

	BDBG_ENTER(BDSP_Raaga_P_CheckDspAlive);
    for(i=0; i< pDevice->numDsp;i++)
    {
        errCode = BDSP_Raaga_P_ProcessPingCommand(pDevice, i);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_CheckDspAlive: DSP %d not Alive", i));
            goto end;
        }
    }

end:
	BDBG_LEAVE(BDSP_Raaga_P_CheckDspAlive);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_ReleaseFIFO

Type        :   BDSP Internal

Input       :   pDevice     -   Handle of the Device.
				dspIndex        -   Index of the DSP.
				i32Fifo     -   Start index of the FIFO's Allocated.
				numfifos        -   Number of FIFO's allocated.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Acquire the FIFOID mutex of the DSP.
		2)  Free the FIFOs by setting the array to FALSE.
		3)  Release the Mutex before returning.
***********************************************************************/
BERR_Code BDSP_Raaga_P_ReleaseFIFO(
	BDSP_Raaga *pDevice,
	unsigned dspIndex,
	uint32_t* ui32Fifo,
	unsigned numfifos)
{
	unsigned i;
	BKNI_AcquireMutex(pDevice->deviceMutex);
	for(i=0; i < numfifos; i++)
	{
		BDBG_MSG(("Freeing (Fifo)RDB %d", (*ui32Fifo+i-BDSP_RAAGA_FIFO_0_INDEX)));
		pDevice->hardwareStatus.dspFifo[dspIndex][(*ui32Fifo+i-BDSP_RAAGA_FIFO_0_INDEX)] = false;
	}
	*ui32Fifo = BDSP_RAAGA_FIFO_INVALID;
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	return BERR_SUCCESS;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AssignFreeFIFO

Type        :   BDSP Internal

Input       :   pDevice     -   Handle of the Device.
				dspIndex        -   Index of the DSP.
				i32Fifo     -   Start index of the FIFO's Allocated to be returned back.
				numfifosreqd    -   Number of FIFO's requested.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Acquire the FIFOID mutex of the DSP.
		2)  Poll for the contiguous free FIFOs requested for in the DSP list.
		3)  Acquire the FIFOs if available by setting the value to TRUE.
		4)  Return the start index of the FIFOs from where the contiguous list is free.
		4)  Release the Mutex before returning.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AssignFreeFIFO(
	BDSP_Raaga *pDevice,
	unsigned dspIndex,
	uint32_t *pui32Fifo,
	unsigned numfifosreqd
)
{
	BERR_Code   err=BERR_SUCCESS;
	unsigned count = 0;
	int32_t i =0, start_index = 0;
	BKNI_AcquireMutex(pDevice->deviceMutex);
	/* Find free Fifo Ids */
	for (i=0; i < (int32_t)BDSP_RAAGA_NUM_FIFOS; i++)
	{
		if (false == pDevice->hardwareStatus.dspFifo[dspIndex][i])
		{
			count++;
			/* Found enough contiguous RDBs. Remember 1 FIFO has 4 RDBs */
			if(count >= numfifosreqd)
			{
				break;
			}
		}
		else
		{
			count = 0;
		}
	}
	if (i >= (int32_t)BDSP_RAAGA_NUM_FIFOS)
	{
		BKNI_ReleaseMutex(pDevice->deviceMutex);
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		return err;
	}
	else
	{
		start_index =  i - numfifosreqd + 1;
		for(;i >= start_index; i--)
		{
			BDBG_MSG(("Allocating fifo (RDB) %d", i));
			pDevice->hardwareStatus.dspFifo[dspIndex][i] = true;
		}
		/* This is the fifo ID from where RDBs are free */
		*pui32Fifo = BDSP_RAAGA_FIFO_0_INDEX + start_index;
	}
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	return BERR_SUCCESS;
}

static BERR_Code BDSP_Raaga_P_CopyStartTaskSettings(
	BDSP_RaagaTask *pRaagaTask,
	BDSP_TaskStartSettings *pStartSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pRaagaTask->pContext;

	BDBG_ENTER(BDSP_Raaga_P_CopyStartTaskSettings);

	pRaagaTask->startSettings = *pStartSettings;

	/*Pointers provided by PI are allocated by them respectively and should not hold them*/
	pRaagaTask->startSettings.pSampleRateMap      = NULL;
	pRaagaTask->startSettings.psVDecoderIPBuffCfg = NULL;
	pRaagaTask->startSettings.psVEncoderIPConfig  = NULL;

	if(BDSP_ContextType_eVideo == pRaagaContext->settings.contextType )
	{
		if(NULL == pStartSettings->psVDecoderIPBuffCfg)
		{
			BDBG_ERR(("BDSP_Raaga_P_CopyStartTaskSettings: Input Config structure not provided for Video Decode Task by PI"));
			errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
			return errCode;
		}
		pRaagaTask->startSettings.psVDecoderIPBuffCfg = (BDSP_sVDecoderIPBuffCfg *)BKNI_Malloc(sizeof(BDSP_sVDecoderIPBuffCfg));
		BKNI_Memcpy((void*)pRaagaTask->startSettings.psVDecoderIPBuffCfg,
			(void *)pStartSettings->psVDecoderIPBuffCfg,
			sizeof(BDSP_sVDecoderIPBuffCfg));
		goto end;
	}

	if(BDSP_ContextType_eVideoEncode == pRaagaContext->settings.contextType )
	{
		if(NULL == pStartSettings->psVEncoderIPConfig)
		{
			BDBG_ERR(("BDSP_Raaga_P_CopyStartTaskSettings: Input Config structure not provided for Video Encode Task by PI"));
			errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
			return errCode;
		}
		pRaagaTask->startSettings.psVEncoderIPConfig = (BDSP_sVEncoderIPConfig *)BKNI_Malloc(sizeof(BDSP_sVEncoderIPConfig));
		BKNI_Memcpy((void*)pRaagaTask->startSettings.psVEncoderIPConfig,
			(void *)pStartSettings->psVEncoderIPConfig,
			sizeof(BDSP_sVEncoderIPConfig));
		goto end;
	}
	if(BDSP_ContextType_eAudio == pRaagaContext->settings.contextType )
	{
		/* If APE doesnt provide the Sample Rate Map table then BDSP will internally fill it. No error handling required*/
		if(NULL != pStartSettings->pSampleRateMap)
		{
			pRaagaTask->startSettings.pSampleRateMap = (BDSP_AF_P_sOpSamplingFreq *)BKNI_Malloc(sizeof(BDSP_AF_P_sOpSamplingFreq));
			BKNI_Memcpy((void*)pRaagaTask->startSettings.pSampleRateMap,
				(void *)pStartSettings->pSampleRateMap,
				sizeof(BDSP_AF_P_sOpSamplingFreq));
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_CopyStartTaskSettings);
	return errCode;
}

static void BDSP_Raaga_P_DeleteStartTaskSettings(
	BDSP_RaagaTask 			*pRaagaTask
)
{
	BDBG_ENTER(BDSP_Raaga_P_DeleteStartTaskSettings);

	/* Release the Memory used to store the input Configuration*/
	if(pRaagaTask->startSettings.pSampleRateMap)
		BKNI_Free(pRaagaTask->startSettings.pSampleRateMap);
	if(pRaagaTask->startSettings.psVDecoderIPBuffCfg)
		BKNI_Free(pRaagaTask->startSettings.psVDecoderIPBuffCfg);
	if(pRaagaTask->startSettings.psVEncoderIPConfig)
		BKNI_Free(pRaagaTask->startSettings.psVEncoderIPConfig);

	BKNI_Memset((void *)&pRaagaTask->startSettings, 0, sizeof(pRaagaTask->startSettings));
	BDBG_LEAVE(BDSP_Raaga_P_DeleteStartTaskSettings);
}

static BERR_Code BDSP_Raaga_P_InitAtTaskCreate(
	BDSP_RaagaTask 			*pRaagaTask
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice = pRaagaTask->pContext->pDevice;
	unsigned dspIndex =0;
	BDBG_ENTER(BDSP_Raaga_P_InitAtTaskCreate);

	dspIndex = pRaagaTask->createSettings.dspIndex;
	pRaagaTask->taskParams.isRunning 	   = false;
	pRaagaTask->taskParams.paused	 	   = false;
	pRaagaTask->taskParams.frozen	 	   = false;
	pRaagaTask->taskParams.commandCounter  = 0;
	pRaagaTask->taskParams.lastCommand     = BDSP_P_CommandID_INVALID;
	pRaagaTask->taskParams.masterTaskId    = BDSP_P_INVALID_TASK_ID;
	BKNI_AcquireMutex(pDevice->deviceMutex);
	pRaagaTask->taskParams.taskId          = BDSP_Raaga_P_GetFreeTaskId(&pDevice->taskDetails[dspIndex]);
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	errCode = BKNI_CreateEvent(&pRaagaTask->hEvent);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("Unable to create event for TASK %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		BDBG_ASSERT(0);
	}
	BKNI_ResetEvent(pRaagaTask->hEvent);

	if(pRaagaTask->taskParams.taskId == BDSP_P_INVALID_TASK_ID)
	{
		BDBG_ERR(("BDSP_Raaga_P_InitAtTaskCreate: Cannot create task as already Max(%d) tasks created on DSP",BDSP_RAAGA_MAX_FW_TASK_PER_DSP));
		BDBG_ASSERT(0);
	}

	BKNI_Memset(&pRaagaTask->audioInterruptHandlers, 0, sizeof(pRaagaTask->audioInterruptHandlers));

	BDBG_LEAVE(BDSP_Raaga_P_InitAtTaskCreate);
	return errCode;
}

static void BDSP_Raaga_P_UnInitAtTaskDestroy(
	BDSP_RaagaTask 			*pRaagaTask
)
{
	BDSP_RaagaContext *pRaagaContext = pRaagaTask->pContext;
	BDSP_Raaga        *pDevice       = pRaagaContext->pDevice;

	BDBG_ENTER(BDSP_Raaga_P_UnInitAtTaskDestroy);

	BKNI_Memset(&pRaagaTask->audioInterruptHandlers, 0, sizeof(pRaagaTask->audioInterruptHandlers));
	BKNI_DestroyEvent(pRaagaTask->hEvent);
	BKNI_AcquireMutex(pDevice->deviceMutex);
	BDSP_Raaga_P_ReleaseTaskId(&pDevice->taskDetails[pRaagaTask->createSettings.dspIndex], &pRaagaTask->taskParams.taskId);
	BLST_S_REMOVE(&pRaagaTask->pContext->taskList,pRaagaTask,BDSP_RaagaTask,node);
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	BDBG_LEAVE(BDSP_Raaga_P_UnInitAtTaskDestroy);
}

static BERR_Code BDSP_Raaga_P_InitAtStartTask(
	BDSP_RaagaTask 			*pRaagaTask,
	BDSP_TaskStartSettings  *pStartSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaTask  *pMasterRaagaTask;
	BDSP_Raaga      *pDevice;
	BDSP_RaagaStage *pRaagaPrimaryStage;
	unsigned stageIndex = 0;

	BDBG_ENTER(BDSP_Raaga_P_InitAtStartTask);
	pDevice = (BDSP_Raaga *)pRaagaTask->pContext->pDevice;

	if(pDevice->taskDetails[pRaagaTask->createSettings.dspIndex].numActiveTasks >= BDSP_RAAGA_MAX_NUM_TASKS)
	{
		BDBG_ERR(("BDSP_Raaga_P_InitAtStartTask: Max tasks(%d) already running on DSP(%d), cannot start task (%d)",BDSP_RAAGA_MAX_NUM_TASKS,
			pRaagaTask->createSettings.dspIndex, pRaagaTask->taskParams.taskId));
		BDBG_ASSERT(0);
	}

	errCode = BDSP_Raaga_P_CopyStartTaskSettings(pRaagaTask, pStartSettings);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_InitAtStartTask: Start Settings couldn't be copied for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	pRaagaTask->taskParams.masterTaskId = BDSP_P_INVALID_TASK_ID;
	if ((pStartSettings->schedulingMode== BDSP_TaskSchedulingMode_eSlave) && (pStartSettings->masterTask !=NULL))
	{
		pMasterRaagaTask = (BDSP_RaagaTask *)(pStartSettings->masterTask->pTaskHandle);
		BDBG_OBJECT_ASSERT(pMasterRaagaTask, BDSP_RaagaTask);
		pRaagaTask->taskParams.masterTaskId = pMasterRaagaTask->taskParams.taskId;
	}

	BKNI_AcquireMutex(pDevice->deviceMutex);
	pDevice->taskDetails[pRaagaTask->createSettings.dspIndex].numActiveTasks++;
	pDevice->taskDetails[pRaagaTask->createSettings.dspIndex].pTask[pRaagaTask->taskParams.taskId] = (void *)pRaagaTask;
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroBrId);
	BSTD_UNUSED(macroStId);
	{
		pRaagaConnectStage->pRaagaTask = pRaagaTask;
		pRaagaConnectStage->stageID    = stageIndex;
		pRaagaConnectStage->running    = true;
		stageIndex++;
	}
	BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

end:
	BDBG_LEAVE(BDSP_Raaga_P_InitAtStartTask);
	return errCode;
}

static void BDSP_Raaga_P_UnInitAtStopTask(
	BDSP_RaagaTask 			*pRaagaTask
)
{
	BDSP_Raaga     *pDevice;
	BDSP_RaagaStage *pRaagaPrimaryStage;

	BDBG_ENTER(BDSP_Raaga_P_UnInitAtStopTask);
	pDevice = (BDSP_Raaga *)pRaagaTask->pContext->pDevice;

	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
	BSTD_UNUSED(macroBrId);
	BSTD_UNUSED(macroStId);
	{
		pRaagaConnectStage->pRaagaTask = NULL;
		pRaagaConnectStage->stageID    = 0;
		pRaagaConnectStage->running    = false;
	}
	BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

	pRaagaTask->taskParams.isRunning     = false;
	pRaagaTask->taskParams.lastCommand   = BDSP_P_CommandID_STOP_TASK;
	pRaagaTask->taskParams.masterTaskId  = BDSP_P_INVALID_TASK_ID;
	pRaagaTask->taskParams.commandCounter= 0;

	BKNI_AcquireMutex(pRaagaTask->pContext->pDevice->deviceMutex);
	pDevice->taskDetails[pRaagaTask->createSettings.dspIndex].numActiveTasks--;
	pDevice->taskDetails[pRaagaTask->createSettings.dspIndex].pTask[pRaagaTask->taskParams.taskId] = NULL;
	BKNI_ReleaseMutex(pRaagaTask->pContext->pDevice->deviceMutex);

	BDSP_Raaga_P_DeleteStartTaskSettings(pRaagaTask);

	BDBG_LEAVE(BDSP_Raaga_P_UnInitAtStopTask);
}

static BERR_Code BDSP_Raaga_P_DownloadRuntimeAlgorithm(
		BDSP_RaagaTask	*pRaagaTask
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaPrimaryStage;

	BDBG_ENTER(BDSP_Raaga_P_DownloadRuntimeAlgorithm);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        errCode = BDSP_Raaga_P_DownloadAlgorithm(pRaagaTask->pContext->pDevice, pStageIterator->eAlgorithm);
        if (errCode != BERR_SUCCESS)
        {
            errCode = BERR_TRACE(errCode);
			BDBG_ERR(("BDSP_Raaga_P_DownloadRuntimeAlgorithm: Error in downloading Algorithm(%d)",pStageIterator->eAlgorithm));
            goto end;
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pStageIterator)

end:
	BDBG_LEAVE(BDSP_Raaga_P_DownloadRuntimeAlgorithm);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseRuntimeAlgorithm(
		BDSP_RaagaTask	*pRaagaTask
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaPrimaryStage;

	BDBG_ENTER(BDSP_Raaga_P_ReleaseRuntimeAlgorithm);
	pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pStageIterator)
    BSTD_UNUSED(macroStId);
    BSTD_UNUSED(macroBrId);
    {
        errCode = BDSP_Raaga_P_ReleaseAlgorithm(pRaagaTask->pContext->pDevice, pStageIterator->eAlgorithm);
        if (errCode != BERR_SUCCESS)
        {
            errCode = BERR_TRACE(errCode);
			BDBG_ERR(("BDSP_Raaga_P_ReleaseRuntimeAlgorithm: Error in Releasing Algorithm(%d)",pStageIterator->eAlgorithm));
            goto end;
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pStageIterator)

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseRuntimeAlgorithm);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_InitInterframe(
	BDSP_RaagaStage *pPrimaryRaagaStage
)
{
	BERR_Code   errCode = BERR_SUCCESS;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	BDSP_Raaga	*pDevice;

	BDBG_ENTER(BDSP_Raaga_P_InitInterframe);
	pDevice = pPrimaryRaagaStage->pContext->pDevice;

	BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pPrimaryRaagaStage, pStageIterator)
	BSTD_UNUSED(macroStId);
	BSTD_UNUSED(macroBrId);
	{
		BDSP_MMA_Memory Memory;
		pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pStageIterator->eAlgorithm);
		Memory    = pStageIterator->stageMemInfo.sInterframe.Buffer;
		errCode = BDSP_P_InterframeRunLengthDecode(
			Memory.pAddr, /*Destination*/
			pDevice->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(pStageIterator->eAlgorithm)].Buffer.pAddr,/*Encoded Interframe Image Address*/
			pDevice->codeInfo.imgInfo[BDSP_IMG_ID_IFRAME(pStageIterator->eAlgorithm)].ui32Size,/*Encoded Interframe Image Size */
			pStageIterator->stageMemInfo.sInterframe.ui32Size);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_InitInterframe: Error for Algorithm(%d) %s",pStageIterator->eAlgorithm, pAlgoInfo->pName));
			goto end;
		}
		BDSP_MMA_P_FlushCache(Memory, pStageIterator->stageMemInfo.sInterframe.ui32Size);
	}
	BDSP_STAGE_TRAVERSE_LOOP_END(pStageIterator)

end:
	BDBG_LEAVE(BDSP_Raaga_P_InitInterframe);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_PopulateSchedulingInfo(
	BDSP_RaagaTask *pRaagaTask,
	BDSP_P_StartTaskCommand *psCommand
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_PopulateSchedulingInfo);

	switch(pRaagaTask->startSettings.schedulingMode)
	{
		case BDSP_TaskSchedulingMode_eStandalone:
		case BDSP_TaskSchedulingMode_eMaster:
			psCommand->eSchedulingMode = BDSP_P_SchedulingMode_eMaster;
			break;
		case BDSP_TaskSchedulingMode_eSlave:
			psCommand->eSchedulingMode = BDSP_P_SchedulingMode_eSlave;
			break;
		default:
			BDBG_ERR(("BDSP_Raaga_P_PopulateSchedulingInfo: Invalid Scheduling mode being Set"));
			errCode = BERR_INVALID_PARAMETER;
			goto end;
	}
	switch(pRaagaTask->startSettings.realtimeMode)
	{
		case BDSP_TaskRealtimeMode_eRealTime:
			if(pRaagaTask->pContext->settings.contextType == BDSP_ContextType_eVideoEncode)
			{
				psCommand->eTaskType = BDSP_P_TaskType_eInterruptBased;
				psCommand->ui32SchedulingLevel= BDSP_P_TaskType_eInterruptBased;
			}
			else
			{
				psCommand->eTaskType = BDSP_P_TaskType_eRealtime;
				psCommand->ui32SchedulingLevel= BDSP_P_TaskType_eRealtime;
			}
			break;
		case BDSP_TaskRealtimeMode_eNonRealTime:
			psCommand->eTaskType = BDSP_P_TaskType_eAFAP;
			psCommand->ui32SchedulingLevel= BDSP_P_TaskType_eAFAP;
			break;
		case BDSP_TaskRealtimeMode_eOnDemand:
			psCommand->eTaskType = BDSP_P_TaskType_eOnDemand;
			psCommand->ui32SchedulingLevel= BDSP_P_TaskType_eOnDemand;
			break;
		default:
			BDBG_ERR(("BDSP_Raaga_P_PopulateSchedulingInfo: Invalid Tasktype being Set"));
			errCode = BERR_INVALID_PARAMETER;
			goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_PopulateSchedulingInfo);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Open

Type        :   BDSP Internal

Input       :   pRaaga - Handle which needs to be opened/created

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :
***********************************************************************/
BERR_Code BDSP_Raaga_P_Open(BDSP_Raaga *pDevice)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned dspindex=0, MemoryRequired=0;

	BDBG_ENTER(BDSP_Raaga_P_Open);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
	BDBG_MSG((" device settings : auth enabled = %d", pDevice->deviceSettings.authenticationEnabled));

	BDSP_Raaga_P_ValidateCodeDownloadSettings(pDevice);

	if(pDevice->hardwareStatus.deviceWatchdogFlag == false)
	{
		if((pDevice->deviceSettings.authenticationEnabled == true)||
			(pDevice->deviceSettings.preloadImages == true))
			pDevice->codeInfo.preloadImages = true;

		BDSP_Raaga_P_InitDevice(pDevice);

		errCode = BDSP_Raaga_P_AssignAlgoSize(pDevice->deviceSettings.pImageInterface,
			pDevice->deviceSettings.pImageContext,
			&pDevice->codeInfo.imgInfo[0]);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Assign algorithm sizes for Images"));
            BDBG_ASSERT(0);
        }

		BDSP_Raaga_P_CalculateDeviceROMemory(pDevice, &MemoryRequired);
        errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
								MemoryRequired,
								&(pDevice->memInfo.sROMemoryPool.Memory),
								BDSP_MMA_Alignment_4KByte);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Allocate Read Only Memory for Raaga"));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BDBG_ASSERT(0);
        }
		BDBG_MSG(("BDSP_Raaga_P_Open: RO Memory Required = %d",MemoryRequired));
        pDevice->memInfo.sROMemoryPool.ui32Size     = MemoryRequired;
        pDevice->memInfo.sROMemoryPool.ui32UsedSize = 0;

		for(dspindex=0; dspindex<pDevice->numDsp; dspindex++)
		{
			BDSP_Raaga_P_CalculateDeviceRWMemory(pDevice, &MemoryRequired);
			errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
									MemoryRequired,
									&(pDevice->memInfo.sRWMemoryPool[dspindex].Memory),
									BDSP_MMA_Alignment_4KByte);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Allocate Read Write Memory for Raaga"));
				errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				BDBG_ASSERT(0);
			}
			BDBG_MSG(("BDSP_Raaga_P_Open: RW Memory Required for Dsp(%d)= %d",dspindex, MemoryRequired));
			pDevice->memInfo.sRWMemoryPool[dspindex].ui32Size     = MemoryRequired;
			pDevice->memInfo.sRWMemoryPool[dspindex].ui32UsedSize = 0;
		}
		errCode = BDSP_Raaga_P_DownloadCode((void *)pDevice);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_Open: Unable to complete the Code download"));
			errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			BDBG_ASSERT(0);
		}
	}

    for (dspindex=0; dspindex<pDevice->numDsp; dspindex++)
    {
        errCode = BDSP_Raaga_P_ProgramAtuEntries(pDevice, dspindex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Program ATU for Raaga DSP %d", dspindex));
            BDBG_ASSERT(0);
        }

        if(pDevice->hardwareStatus.deviceWatchdogFlag == false)
        {
	        errCode = BDSP_Raaga_P_AssignDeviceRWMemory(pDevice, dspindex);
	        if(errCode != BERR_SUCCESS)
	        {
	            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Assign Read Write Memory for Raaga DSP %d", dspindex));
	            BDBG_ASSERT(0);
	        }

            errCode = BDSP_Raaga_P_CreateMsgQueue(
                    &pDevice->memInfo.cmdQueueParams[dspindex],
                    pDevice->regHandle,
                    pDevice->dspOffset[dspindex],
                    &(pDevice->hCmdQueue[dspindex]));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Create Command Queue for Raaga DSP %d", dspindex));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }

            errCode = BDSP_Raaga_P_CreateMsgQueue(
                &pDevice->memInfo.genRspQueueParams[dspindex],
                pDevice->regHandle,
                pDevice->dspOffset[dspindex],
                &(pDevice->hGenRespQueue[dspindex]));
            if(errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Create Generic Response Queue for Raaga DSP %d", dspindex));
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BDBG_ASSERT(0);
            }
        }

        errCode = BDSP_Raaga_P_InitMsgQueue(pDevice->hCmdQueue[dspindex]);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Initialise Command Queue for Raaga DSP %d", dspindex));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BDBG_ASSERT(0);
        }

        errCode = BDSP_Raaga_P_InitMsgQueue(pDevice->hGenRespQueue[dspindex]);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Initialise Generic Response Queue for Raaga DSP %d", dspindex));
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BDBG_ASSERT(0);
        }

		errCode = BDSP_Raaga_P_DeviceInterruptInstall((void *)pDevice, dspindex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Install Device Interrupt callback for Raaga DSP %d", dspindex));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            BDBG_ASSERT(0);
        }

		/* Program command queue & generic response queue */
		BDSP_WriteReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID + pDevice->dspOffset[dspindex],
			pDevice->hCmdQueue[dspindex]->ui32FifoId);
		BDSP_WriteReg64(pDevice->regHandle,
			BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED1 + pDevice->dspOffset[dspindex],
			pDevice->hGenRespQueue[dspindex]->ui32FifoId);

        errCode = BDSP_Raaga_P_ProcessInitCommand(pDevice, dspindex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Process the INIT Command for Raaga DSP %d", dspindex));
            BDBG_ASSERT(0);
        }
    }

	/* Program the Debug Infrastructure */
	errCode = BDSP_Raaga_P_InitDebugInfrastructure(pDevice);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_Open: Unable to Initialise Debug Infrastructure for Raaga DSP"));
		errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
		BDBG_ASSERT(0);
	}

	BDBG_LEAVE(BDSP_Raaga_P_Open);
	return errCode;
}

/***********************************************************************
Name            :   BDSP_Raaga_P_Close

Type            :   PI Interface

Input           :   pDeviceHandle -Device Handle which needs to be closed.

Return          :   None

Functionality       :
	1)  Destroy any Context, Interrupt Handle, RDB register left Open.
	2)  UnInstall Acknowledgment for the DSP.
	3)  De-Allocate Scratch, Interstage & Interstage interface memory requirements.
	4)  Destroy the Mutexs for task, capture, Fifos, DspInterrupt, RDB, Watchdog.
	5)  Free the Firmware Executables.
	6)  De-Allocate memories for CIT Buffers, Command Queue and Generic Response Queue.
	7)  Disable the Watchdog timer.
	8)  Reset the Hardware for each DSP.
	9)  Free the memory allocated for Device Handle.
***********************************************************************/
void BDSP_Raaga_P_Close(
	void *pDeviceHandle
	)
{
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	unsigned dspIndex=0;
	BERR_Code errCode=BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_Close);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);

	for(dspIndex = 0; dspIndex< pDevice->numDsp; dspIndex++)
	{
		errCode = BDSP_Raaga_P_DestroyMsgQueue(pDevice->hCmdQueue[dspIndex]);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_Close: CMD queue destroy failed for DSP %d!!!", dspIndex));
			errCode = BERR_TRACE(errCode);
		}

		errCode = BDSP_Raaga_P_DestroyMsgQueue(pDevice->hGenRespQueue[dspIndex]);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_Close: Generic RSP queue destroy failed for DSP %d!!!", dspIndex));
			errCode = BERR_TRACE(errCode);
		}

		errCode = BDSP_Raaga_P_ReleaseDeviceRWMemory(pDevice, dspIndex);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_Close: Unable to Release RW memory for DSP %d!!!", dspIndex));
			errCode = BERR_TRACE(errCode);
		}

		errCode = BDSP_Raaga_P_DeviceInterruptUninstall((void *)pDevice, dspIndex);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_Close: Unable to Un-Install Interrupt callbacks for DSP %d", dspIndex));
            errCode = BERR_TRACE(errCode);
        }

		BDSP_MMA_P_FreeMemory(&(pDevice->memInfo.sRWMemoryPool[dspIndex].Memory));
	}

	BDSP_MMA_P_FreeMemory(&(pDevice->memInfo.sROMemoryPool.Memory));

	errCode = BDSP_Raaga_P_Reset(pDevice);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_Close: Unable to RESET DSP"));
		errCode = BERR_TRACE(errCode);
	}

	BDSP_Raaga_P_EnableAllPwrResource(pDeviceHandle, false);

	BDSP_Raaga_P_DeInitDevice(pDevice);

	/* Invalidate and free the device structure */
	BDBG_OBJECT_DESTROY(pDevice, BDSP_Raaga);
	BKNI_Free(pDevice);

	BDBG_LEAVE(BDSP_Raaga_P_Close);
}

void BDSP_Raaga_P_GetDefaultContextSettings(
	void 					   *pDeviceHandle,
    BDSP_ContextType 			contextType,
    BDSP_ContextCreateSettings *pSettings
)
{
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;

	BDBG_ENTER(BDSP_Raaga_P_GetDefaultContextSettings);
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    if (contextType == BDSP_ContextType_eAudio)
    {
        pSettings->maxTasks    		 = BDSP_RAAGA_MAX_FW_TASK_PER_AUDIO_CTXT;
        pSettings->contextType 		 = contextType;
        pSettings->maxBranch   		 = BDSP_RAAGA_MAX_BRANCH;
        pSettings->maxStagePerBranch = BDSP_RAAGA_MAX_STAGE_PER_BRANCH;
    }
	else if(contextType == BDSP_ContextType_eVideoEncode)
	{
		/* Added to beat the context create in playback application.
		     Load appropriate values when Video encode is supported*/
        pSettings->maxTasks 		 = 1;
        pSettings->contextType		 = contextType;
        pSettings->maxBranch 		 = 1;
        pSettings->maxStagePerBranch = 1;
	}
    else
    {
        BDBG_ERR(("BDSP_Raaga_P_GetDefaultContextSettings: Trying to create a Context(%d) other that Audio on RAAGA which is not supported!!!!!",contextType));
    }

	BDBG_LEAVE(BDSP_Raaga_P_GetDefaultContextSettings);
}

BERR_Code BDSP_Raaga_P_CreateContext(
	void 							 *pDeviceHandle,
	const BDSP_ContextCreateSettings *pSettings,
	BDSP_ContextHandle 				 *pContextHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	BDSP_RaagaContext *pRaagaContext;

	BDBG_ENTER( BDSP_Raaga_P_CreateContext );
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
	*pContextHandle = NULL;
	pRaagaContext = BKNI_Malloc(sizeof(BDSP_RaagaContext));
	if ( NULL == pRaagaContext )
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateContext: Unable to allocate Memory for Creating the Context"));
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto end;
	}
	BKNI_Memset(pRaagaContext, 0, sizeof(*pRaagaContext));
	BDBG_OBJECT_SET(pRaagaContext, BDSP_RaagaContext);
	pRaagaContext->pDevice  = pDevice;
	pRaagaContext->settings = *pSettings;

	BDSP_P_InitContext(&pRaagaContext->context, pRaagaContext);

	pRaagaContext->context.destroy = BDSP_Raaga_P_DestroyContext;
	pRaagaContext->context.getDefaultTaskSettings = BDSP_Raaga_P_GetDefaultTaskSettings;
	pRaagaContext->context.createTask  = BDSP_Raaga_P_CreateTask;
	pRaagaContext->context.getDefaultStageCreateSettings = BDSP_Raaga_P_GetDefaultStageCreateSettings;
	pRaagaContext->context.createStage = BDSP_Raaga_P_CreateStage;

	pRaagaContext->context.createInterTaskBuffer = NULL;
	pRaagaContext->context.getDefaultQueueSettings = NULL;
	pRaagaContext->context.createQueue = NULL;

	pRaagaContext->context.getInterruptHandlers = BDSP_Raaga_P_GetContextInterruptHandlers;
	pRaagaContext->context.setInterruptHandlers= BDSP_Raaga_P_SetContextInterruptHandlers;
	pRaagaContext->context.processWatchdogInterrupt= BDSP_Raaga_P_ProcessContextWatchdogInterrupt;
	pRaagaContext->context.createCapture = NULL;

	BKNI_AcquireMutex(pDevice->deviceMutex);
	BLST_S_INSERT_HEAD(&pDevice->contextList, pRaagaContext, node);
	BLST_S_INIT(&pRaagaContext->taskList);
	*pContextHandle= &pRaagaContext->context;
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	goto end;

end:
	BDBG_LEAVE( BDSP_Raaga_P_CreateContext );
	return errCode;
}

void BDSP_Raaga_P_DestroyContext(
	void *pContextHandle
)
{
	BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
	BDSP_Raaga *pDevice;
	BDSP_RaagaTask *pRaagaTask;

	BDBG_ENTER(BDSP_Raaga_P_DestroyContext);
	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
	pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

	while((pRaagaTask = BLST_S_FIRST(&pRaagaContext->taskList)))
	{
		BDSP_Raaga_P_DestroyTask((void *)pRaagaTask);
	}

	BKNI_AcquireMutex(pDevice->deviceMutex);
	BLST_S_REMOVE(&pDevice->contextList, pRaagaContext, BDSP_RaagaContext, node);
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	BDBG_OBJECT_DESTROY(pRaagaContext, BDSP_RaagaContext);
	BKNI_Free(pRaagaContext);
	BDBG_LEAVE(BDSP_Raaga_P_DestroyContext);
}

void BDSP_Raaga_P_GetDefaultTaskSettings(
	void *pContextHandle,
	BDSP_TaskCreateSettings *pSettings
	)
{
	BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;

	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
	BKNI_Memset(pSettings, 0, sizeof(*pSettings));
	pSettings->masterTask = false;
}

BERR_Code BDSP_Raaga_P_CreateTask(
	void *pContextHandle,
	const BDSP_TaskCreateSettings *pSettings,
	BDSP_TaskHandle *pTaskHandle
)
{
	BERR_Code   errCode = BERR_SUCCESS;
	BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
	BDSP_RaagaTask *pRaagaTask;
	BDSP_Raaga	*pDevice;
	unsigned MemoryRequired=0;

	BDBG_ENTER(BDSP_Raaga_P_CreateTask);
	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
	pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;
	*pTaskHandle = NULL;

	pRaagaTask = BKNI_Malloc(sizeof(BDSP_RaagaTask));
	if(NULL == pRaagaTask)
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateTask: Unable to allocate Memory for Creating the Task"));
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ASSERT(0);
	}
	BKNI_Memset(pRaagaTask, 0, sizeof(BDSP_RaagaTask));
	BDBG_OBJECT_SET(pRaagaTask, BDSP_RaagaTask);
	pRaagaTask->pContext = pRaagaContext;
	pRaagaTask->createSettings = *pSettings;

	BDSP_P_InitTask(&pRaagaTask->task, pRaagaTask);

	pRaagaTask->task.destroy = BDSP_Raaga_P_DestroyTask;
	pRaagaTask->task.getDefaultTaskStartSettings = BDSP_Raaga_P_GetDefaultTaskStartSettings;
	pRaagaTask->task.start = BDSP_Raaga_P_StartTask;
	pRaagaTask->task.stop = BDSP_Raaga_P_StopTask;
	pRaagaTask->task.retreiveGateOpenSettings = NULL;
	if (pRaagaContext->settings.contextType == BDSP_ContextType_eAudio)
	{
		pRaagaTask->task.pause= BDSP_Raaga_P_Pause;
		pRaagaTask->task.resume = BDSP_Raaga_P_Resume;
		pRaagaTask->task.advance = BDSP_Raaga_P_Advance;
		pRaagaTask->task.getAudioInterruptHandlers_isr = BDSP_Raaga_P_GetTaskInterruptHandlers_isr;
		pRaagaTask->task.setAudioInterruptHandlers_isr = BDSP_Raaga_P_SetTaskInterruptHandlers_isr;
		pRaagaTask->task.audioGapFill = BDSP_Raaga_P_AudioGapFill;
		pRaagaTask->task.freeze = BDSP_Raaga_P_Freeze;
		pRaagaTask->task.unfreeze = BDSP_Raaga_P_UnFreeze;
	}
	else
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateTask: Trying to create a Task other than Audio"));
	}

	errCode = BDSP_Raaga_P_InitAtTaskCreate(pRaagaTask);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateTask: Unable to Initialise Parameters for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		BDBG_ASSERT(0);
	}

	BDSP_Raaga_P_CalculateTaskMemory(&MemoryRequired);
	errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							MemoryRequired,
							&(pRaagaTask->taskMemInfo.sMemoryPool.Memory),
							BDSP_MMA_Alignment_4KByte);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateTask: Unable to Allocate Memory for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		BDBG_ASSERT(0);
	}
	pRaagaTask->taskMemInfo.sMemoryPool.ui32Size 	 = MemoryRequired;
	pRaagaTask->taskMemInfo.sMemoryPool.ui32UsedSize = 0;

	errCode = BDSP_Raaga_P_AssignTaskMemory((void *)pRaagaTask);
	if (errCode != BERR_SUCCESS )
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateTask: Unable to assign Task memory for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ASSERT(0);
	}

	errCode = BDSP_Raaga_P_CreateMsgQueue(
			&pRaagaTask->taskMemInfo.syncQueueParams,
			pDevice->regHandle,
			pDevice->dspOffset[pSettings->dspIndex],
			&pRaagaTask->hSyncQueue);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateTask: Unable to Create Sync Resp Queue for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		BDBG_ASSERT(0);
	}

	errCode = BDSP_Raaga_P_CreateMsgQueue(
			&pRaagaTask->taskMemInfo.asyncQueueParams,
			pDevice->regHandle,
			pDevice->dspOffset[pSettings->dspIndex],
			&pRaagaTask->hAsyncQueue);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateTask: Unable to Create ASync Resp Queue for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		BDBG_ASSERT(0);
	}

	errCode = BDSP_Raaga_P_TaskInterruptInstall((void *)pRaagaTask);
	if ( BERR_SUCCESS!= errCode )
	{
		BDBG_ERR(("Unable to Install Interrupt for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		BDBG_ASSERT(0);
	}

	BKNI_AcquireMutex(pDevice->deviceMutex);
	BLST_S_INSERT_HEAD(&pRaagaContext->taskList, pRaagaTask, node);
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	*pTaskHandle = &pRaagaTask->task;

	BDBG_LEAVE(BDSP_Raaga_P_CreateTask);
	return errCode;
}

void BDSP_Raaga_P_DestroyTask(
	void *pTaskHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask =(BDSP_RaagaTask *)pTaskHandle;

	BDBG_ENTER(BDSP_Raaga_P_DestroyTask);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if(pRaagaTask->taskParams.isRunning == true)
	{
		BDBG_ERR(("BDSP_Raaga_P_DestroyTask: Task (%d) is still running, Stopping it by force",pRaagaTask->taskParams.taskId));
		BDSP_Raaga_P_StopTask(pTaskHandle);
	}

	errCode = BDSP_Raaga_P_TaskInterruptUninstall(pTaskHandle);
	if ( BERR_SUCCESS!=errCode )
	{
		BDBG_ERR(("Unable to Un-Install Interrupt for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		BDBG_ASSERT(0);
	}

	errCode = BDSP_Raaga_P_DestroyMsgQueue(pRaagaTask->hSyncQueue);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_DestroyTask: SYNC queue destroy failed for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
	}

	errCode = BDSP_Raaga_P_DestroyMsgQueue(pRaagaTask->hAsyncQueue);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_DestroyTask: ASYNC queue destroy failed for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
	}

	errCode = BDSP_Raaga_P_ReleaseTaskMemory((void *)pRaagaTask);
	if ( BERR_SUCCESS !=errCode )
	{
		BDBG_ERR(("BDSP_Raaga_P_DestroyTask: Unable to Release Memory for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
	}
	BDSP_MMA_P_FreeMemory(&pRaagaTask->taskMemInfo.sMemoryPool.Memory);

	BDSP_Raaga_P_UnInitAtTaskDestroy(pRaagaTask);

	BDBG_OBJECT_DESTROY(pRaagaTask, BDSP_RaagaTask);
	BKNI_Free(pRaagaTask);

	BDBG_LEAVE(BDSP_Raaga_P_DestroyTask);
}

void BDSP_Raaga_P_GetDefaultTaskStartSettings(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pSettings    /* [out] */
)
{

	BDSP_RaagaTask *pRaagaTask;
	pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
	BDBG_ASSERT(NULL != pSettings);

	BKNI_Memset((void *)pSettings,0,sizeof(BDSP_TaskStartSettings));

	pSettings->primaryStage   = NULL;
	pSettings->schedulingMode = BDSP_TaskSchedulingMode_eStandalone;
	pSettings->realtimeMode   = BDSP_TaskRealtimeMode_eRealTime;
	pSettings->masterTask     = NULL;
	pSettings->audioTaskDelayMode = BDSP_AudioTaskDelayMode_eDefault;
	pSettings->timeBaseType       = BDSP_AF_P_TimeBaseType_e45Khz;
	pSettings->ppmCorrection      = false;
	pSettings->openGateAtStart    = false;
	pSettings->stcIncrementConfig.enableStcTrigger = false;
	pSettings->extInterruptConfig.enableInterrupts = false;
	pSettings->eZeroPhaseCorrEnable=true; /*Enable Zero Phase Correction in default settings*/
	pSettings->pSampleRateMap      = NULL;
	pSettings->maxIndependentDelay = BDSP_MAX_INDEPENDENT_DELAY_IN_MS;
	pSettings->gateOpenReqd        = true;
}

BERR_Code BDSP_Raaga_P_StartTask(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pStartSettings
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	BDSP_RaagaStage *pPrimaryStage;
	unsigned dspIndex =0;
	BDSP_P_StartTaskCommand sPayload;

	BDBG_ENTER(BDSP_Raaga_P_StartTask);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
	dspIndex = pRaagaTask->createSettings.dspIndex;
	pPrimaryStage = (BDSP_RaagaStage *)pStartSettings->primaryStage->pStageHandle;

	BKNI_Memset(&sPayload,0,sizeof(BDSP_P_StartTaskCommand));

	BDBG_MSG(("Start task (%d) on DSP %d",pRaagaTask->taskParams.taskId, dspIndex));

	errCode = BDSP_Raaga_P_InitAtStartTask(pRaagaTask, pStartSettings);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StartTask: Unable to Initialise the Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	errCode = BDSP_Raaga_P_InitMsgQueue(pRaagaTask->hSyncQueue);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StartTask: Sync Queue Init failed for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	errCode = BDSP_Raaga_P_InitMsgQueue(pRaagaTask->hAsyncQueue);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StartTask: ASync Queue Init failed for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	if(pRaagaTask->pContext->pDevice->codeInfo.preloadImages == false)
	{
		errCode = BDSP_Raaga_P_DownloadRuntimeAlgorithm(pRaagaTask);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_StartTask: Error in Runtime Algorithm Download for Task %d",pRaagaTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}

	errCode = BDSP_Raaga_P_InitInterframe(pPrimaryStage);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StartTask: Decode the Interframe for all stage for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	errCode = BDSP_Raaga_P_GenCit(pRaagaTask);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StartTask: Error in Forming CIT Network for Task %d",pRaagaTask->taskParams.taskId));
		errCode =BERR_TRACE(errCode);
		goto end;
	}

	errCode = BDSP_Raaga_P_PopulateSchedulingInfo(pRaagaTask, &sPayload);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StartTask: Unable to populate Scheduling Info for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}
	sPayload.ui32TaskId          = pRaagaTask->taskParams.taskId;
	sPayload.ui32MasterTaskId    = pRaagaTask->taskParams.masterTaskId;
	sPayload.ui32SyncQueueFifoId = pRaagaTask->hSyncQueue->ui32FifoId;
	sPayload.ui32AsyncQueueFifoId= pRaagaTask->hAsyncQueue->ui32FifoId;
	sPayload.ui32EventEnableMask = pRaagaTask->taskParams.eventEnabledMask;
	sPayload.sTaskMemoryInfo.BaseAddr   = pRaagaTask->taskMemInfo.sMemoryPool.Memory.offset;
	sPayload.sTaskMemoryInfo.Size       = pRaagaTask->taskMemInfo.sMemoryPool.ui32Size;
	sPayload.sSharedMemoryInfo.BaseAddr = pRaagaTask->taskMemInfo.sMPSharedMemory.Buffer.offset;
	sPayload.sSharedMemoryInfo.Size     = pRaagaTask->taskMemInfo.sMPSharedMemory.ui32Size;
	sPayload.sConfigMemoryInfo.BaseAddr = pRaagaTask->taskMemInfo.sCITMemory.Buffer.offset;
	sPayload.sConfigMemoryInfo.Size     = pRaagaTask->taskMemInfo.sCITMemory.ui32Size;
	sPayload.sPrimaryStageMemoryInfo.BaseAddr   = pPrimaryStage->stageMemInfo.sMemoryPool.Memory.offset;
	sPayload.sPrimaryStageMemoryInfo.Size       = pPrimaryStage->stageMemInfo.sMemoryPool.ui32Size;

	errCode = BDSP_Raaga_P_ProcessStartTaskCommand(pRaagaTask, &sPayload);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StartTask: Error in Start Task command processing for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto err_start_task;
	}

	pRaagaTask->taskParams.isRunning   = true;
	pRaagaTask->taskParams.lastCommand = BDSP_P_CommandID_START_TASK;
	goto end;

err_start_task:
	BDSP_Raaga_P_UnInitAtStopTask(pRaagaTask);
end:
	BDBG_LEAVE(BDSP_Raaga_P_StartTask);
	return errCode;
}

BERR_Code BDSP_Raaga_P_StopTask(
	void *pTaskHandle
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;

	BDBG_ENTER(BDSP_Raaga_P_StopTask);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if(pRaagaTask->pContext->contextWatchdogFlag == false)
	{
		errCode = BDSP_Raaga_P_ProcessStopTaskCommand(pRaagaTask);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_StopTask: Error in Stop Task command processing for Task %d",pRaagaTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}

	errCode = BDSP_Raaga_P_CleanupCit(pRaagaTask);
	if (BERR_SUCCESS != errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_StopTask: Error in Cleanup of CIT for Task %d",pRaagaTask->taskParams.taskId));
		errCode = BERR_TRACE(errCode);
		goto end;
	}

	if(pRaagaTask->pContext->pDevice->codeInfo.preloadImages == false)
	{
		errCode = BDSP_Raaga_P_ReleaseRuntimeAlgorithm(pRaagaTask);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_StopTask: Error in Releasing Algorithms for Task %d",pRaagaTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}
	BDSP_Raaga_P_UnInitAtStopTask(pRaagaTask);

end:
	BDBG_LEAVE(BDSP_Raaga_P_StopTask);
	return errCode;
}

void BDSP_Raaga_P_GetDefaultStageCreateSettings(
	BDSP_AlgorithmType algoType,
	BDSP_StageCreateSettings *pSettings /* [out] */
)
{
	unsigned i;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;

	BDBG_ENTER(BDSP_Raaga_P_GetDefaultStageCreateSettings);
	BDBG_ASSERT(NULL != pSettings);
	BDBG_ASSERT(algoType < BDSP_AlgorithmType_eMax);

	pSettings->algoType = algoType;
	for (i = 0; i < BDSP_Algorithm_eMax; i++)
	{
		pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(i);
		pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(i);

		if (algoType == pAlgoInfo->type)
		{
			pSettings->algorithmSupported[i] = pAlgoSupportInfo->supported;
		}
		else
		{
			pSettings->algorithmSupported[i] = false;
		}
	}

	switch (algoType)
	{
		case BDSP_AlgorithmType_eAudioDecode:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 3;
			break;
		case BDSP_AlgorithmType_eAudioPassthrough:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioEncode:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioMixer:
			pSettings->maxInputs = 3;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioEchoCanceller:
			pSettings->maxInputs = 2;
			pSettings->maxOutputs = 1;
			break;
		case BDSP_AlgorithmType_eAudioProcessing:
			pSettings->maxInputs = 1;
			pSettings->maxOutputs = 2;
			break;
		default:
			BDBG_ERR(("AlgoType is not yet supported in the system"));
			break;
	}
	BDBG_LEAVE(BDSP_Raaga_P_GetDefaultStageCreateSettings);
}

BERR_Code BDSP_Raaga_P_CreateStage(
	void *pContextHandle,
	const BDSP_StageCreateSettings *pSettings,
	BDSP_StageHandle *pStageHandle /* [out] */
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage;
	BDSP_RaagaContext *pRaagaContext = (BDSP_RaagaContext *)pContextHandle;
	BDSP_Raaga	*pDevice;
	unsigned    MemoryRequired =0, i=0;

	BDBG_ENTER(BDSP_Raaga_P_CreateStage);
	BDBG_OBJECT_ASSERT(pRaagaContext, BDSP_RaagaContext);
	pDevice = (BDSP_Raaga *)pRaagaContext->pDevice;
	pRaagaStage = BKNI_Malloc(sizeof(BDSP_RaagaStage));
	if ( NULL == pRaagaStage )
	{
		errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ASSERT(0);
	}
	BKNI_Memset(pRaagaStage, 0, sizeof(*pRaagaStage));
	BDBG_OBJECT_SET(pRaagaStage, BDSP_RaagaStage);

	pRaagaStage->eAlgorithm = BDSP_Algorithm_eMax;
	pRaagaStage->settings   = *pSettings;
	pRaagaStage->pContext   = pRaagaContext;
	pRaagaStage->pRaagaTask = NULL;
	pRaagaStage->running    = false;
	pRaagaStage->stageID    = 0;


	BDSP_P_InitStage(&pRaagaStage->stage, pRaagaStage);

	/* Initialize the stage apis */
	pRaagaStage->stage.destroy = BDSP_Raaga_P_DestroyStage;
	pRaagaStage->stage.setAlgorithm = BDSP_Raaga_P_SetAlgorithm;
	pRaagaStage->stage.getStageSettings = BDSP_Raaga_P_GetStageSettings;
	pRaagaStage->stage.setStageSettings = BDSP_Raaga_P_SetStageSettings;
	pRaagaStage->stage.getStageStatus = BDSP_Raaga_P_GetStageStatus;

	pRaagaStage->stage.getTsmSettings_isr = BDSP_Raaga_P_GetTsmSettings_isr;
	pRaagaStage->stage.setTsmSettings_isr = BDSP_Raaga_P_SetTsmSettings_isr;
	pRaagaStage->stage.getTsmStatus_isr = BDSP_Raaga_P_GetTsmStatus_isr;

	pRaagaStage->stage.getDatasyncSettings = BDSP_Raaga_P_GetDatasyncSettings;
	pRaagaStage->stage.getDatasyncSettings_isr = BDSP_Raaga_P_GetDatasyncSettings_isr;
	pRaagaStage->stage.setDatasyncSettings = BDSP_Raaga_P_SetDatasyncSettings;
	pRaagaStage->stage.getDatasyncStatus_isr = BDSP_Raaga_P_GetDatasyncStatus_isr;

	pRaagaStage->stage.addOutputStage = BDSP_Raaga_P_AddOutputStage;

	pRaagaStage->stage.addFmmOutput = BDSP_Raaga_P_AddFmmOutput;
	pRaagaStage->stage.addFmmInput = NULL;

	pRaagaStage->stage.addRaveOutput = NULL;
	pRaagaStage->stage.addRaveInput = BDSP_Raaga_P_AddRaveInput;

	pRaagaStage->stage.addInterTaskBufferOutput = NULL;
	pRaagaStage->stage.addInterTaskBufferInput = NULL;

	pRaagaStage->stage.addQueueOutput = NULL;
#if !B_REFSW_MINIMAL
	pRaagaStage->stage.addQueueInput = NULL;
#endif /*!B_REFSW_MINIMAL*/

	pRaagaStage->stage.removeAllOutputs = BDSP_Raaga_P_RemoveAllOutputs;
#if !B_REFSW_MINIMAL
	pRaagaStage->stage.removeOutput = BDSP_Raaga_P_RemoveOutput;
#endif /*!B_REFSW_MINIMAL*/

	pRaagaStage->stage.removeInput = BDSP_Raaga_P_RemoveInput;
	pRaagaStage->stage.removeAllInputs = BDSP_Raaga_P_RemoveAllInputs;

	if (BDSP_ContextType_eVideoEncode == pRaagaContext->settings.contextType)
	{
		pRaagaStage->stage.getVideoEncodeDatasyncSettings = NULL;
		pRaagaStage->stage.setVideoEncodeDatasyncSettings = NULL;
	}

	BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, false, NULL);
	errCode = BDSP_MMA_P_AllocateAlignedMemory(pDevice->memHandle,
							MemoryRequired,
							&(pRaagaStage->stageMemInfo.sMemoryPool.Memory),
							BDSP_MMA_Alignment_4KByte);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_CreateStage: Unable to Allocate Memory for Stage %d",pRaagaStage->eAlgorithm));
		errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		BDBG_ASSERT(0);
	}
	pRaagaStage->stageMemInfo.sMemoryPool.ui32Size 	   = MemoryRequired;
	pRaagaStage->stageMemInfo.sMemoryPool.ui32UsedSize = 0;

	for (i=0; i<BDSP_Algorithm_eMax; i++)
	{
		if (pSettings->algorithmSupported[i])
		{
			errCode = BDSP_Raaga_P_SetAlgorithm((void *)pRaagaStage, i);
			break;
		}
	}
	if (errCode != BERR_SUCCESS)
	{
		errCode = BERR_TRACE(errCode);
		goto err_set_algorithm;
	}

	*pStageHandle=&pRaagaStage->stage;
	goto end;

err_set_algorithm:
	BDSP_MMA_P_FreeMemory(&pRaagaStage->stageMemInfo.sMemoryPool.Memory);
	BDBG_OBJECT_DESTROY(pRaagaStage, BDSP_RaagaStage);
	BKNI_Free(pRaagaStage);
end:
	BDBG_LEAVE(BDSP_Raaga_P_CreateStage);
	return errCode;
}

void BDSP_Raaga_P_DestroyStage(
	void *pStageHandle
)
{
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	if(pRaagaStage->running)
	{
		BDBG_ERR(("BDSP_Raaga_P_DestroyStage: Trying to Destroy a Running stage(%d) on Task (%d)",
			pRaagaStage->eAlgorithm, pRaagaStage->pRaagaTask->taskParams.taskId));
		BDBG_ASSERT(0);
	}

	BDSP_MMA_P_FreeMemory(&pRaagaStage->stageMemInfo.sMemoryPool.Memory);
	BDBG_OBJECT_DESTROY(pRaagaStage, BDSP_RaagaStage);
	BKNI_Free(pRaagaStage);
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetAlgorithmInfo

Type        :   PI Interface

Input       :   algorithm -The algorithm for the which the data is requested by PI.
				pInfo - Pointer where the specific data releated to algorithm is returned back to the PI.

Return      :   None

Functionality   :   Returns the following data back to the PI.
	1)  Name of the algorithm.
	2)  Whether the algorithm is supported or not.
	3)  To which category the algothm belongs to.
	4)  The size of the User Configuration required by the algorithm.
	5)  The size of the Status Buffer required by the algorithm.
***********************************************************************/
void BDSP_Raaga_P_GetAlgorithmInfo(
	BDSP_Algorithm algorithm,
	BDSP_AlgorithmInfo *pInfo /* [out] */
)
{
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;

	BDBG_ENTER(BDSP_Raaga_P_GetAlgorithmInfo);
	BDBG_ASSERT(pInfo);

	pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
	pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
	pInfo->supported = pAlgoSupportInfo->supported;
	pInfo->pName     = pAlgoSupportInfo->pName;
	if(pAlgoSupportInfo->supported)
	{
		pInfo->type         = pAlgoInfo->type;
		pInfo->settingsSize = pAlgoInfo->algoUserConfigSize;
		pInfo->statusSize   = pAlgoInfo->algoStatusBufferSize;
	}
	else
	{
		pInfo->type         = BDSP_AlgorithmType_eMax;
		pInfo->settingsSize = 0;
		pInfo->statusSize   = 0;
	}

	BDBG_LEAVE(BDSP_Raaga_P_GetAlgorithmInfo);
}

BERR_Code BDSP_Raaga_P_SetAlgorithm(
	void *pStageHandle,
	BDSP_Algorithm algorithm
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = pStageHandle;
	const BDSP_P_AlgorithmInfo *pCurrAlgoInfo, *pAlgoInfo;
	bool valid = false;
	unsigned index = 0;

	BDBG_ENTER(BDSP_Raaga_P_SetAlgorithm);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_MSG(("Setting Algo (%d) for Stage (%p)", algorithm, (void *)pRaagaStage));

	pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);
	pCurrAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);
	/*  Return error if the stage is running.*/
	if (pRaagaStage->running)
	{
		BDBG_ERR(("Cannot set algorithm when the stage is running : stage handle = 0x%p : Current algo = %s",
				   pStageHandle, pCurrAlgoInfo->pName));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
	if (pRaagaStage->totalOutputs)
	{
		BDBG_ERR(("Stage has non-zero (%d) output connections at set algorithm", pRaagaStage->totalOutputs));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	if (pRaagaStage->totalInputs)
	{
		BDBG_ERR(("Stage has non-zero (%d) input connections at set algorithm", pRaagaStage->totalInputs));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	if(!pRaagaStage->settings.algorithmSupported[algorithm])
	{
		BDBG_ERR((" algorithm %s (%d) being passed in %s which was not enabled during CreateStage call ",
					pAlgoInfo->pName,algorithm, BSTD_FUNCTION));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	/*  Return error if there is an attempt to change the stage type */
	if((pCurrAlgoInfo->type == BDSP_AlgorithmType_eAudioDecode)||
		(pCurrAlgoInfo->type == BDSP_AlgorithmType_eAudioPassthrough))
	{
		switch (pAlgoInfo->type)
		{
			case BDSP_AlgorithmType_eAudioDecode:
			case BDSP_AlgorithmType_eAudioPassthrough:
				valid = true;
				break;
			default:
				valid = false;
				break;
		}
	}
	else if((pCurrAlgoInfo->type == BDSP_AlgorithmType_eMax)&&
			 (pAlgoInfo->type != BDSP_AlgorithmType_eMax))
	{
		valid = true;
	}
	else
	{
		valid = (pRaagaStage->settings.algoType == pAlgoInfo->type) ? true : false;
	}
	if(!valid)
	{
		BDBG_ERR(("Cannot change the algo type of the stage from %d (%s) to %d (%s)",
					pCurrAlgoInfo->type, pCurrAlgoInfo->pName, pAlgoInfo->type, pAlgoInfo->pName));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	pRaagaStage->eAlgorithm = pAlgoInfo->algorithm;
	if(pRaagaStage->stageMemInfo.sMemoryPool.ui32UsedSize != 0)
	{
		errCode = BDSP_Raaga_P_ReleaseStageMemory((void *)pRaagaStage);
		if(errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BDSP_Raaga_P_SetAlgorithm: Unable to Release memory for Stage(%p)", (void *)pRaagaStage));
			goto end;
		}
	}
	errCode = BDSP_Raaga_P_AssignStageMemory((void *)pRaagaStage);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_SetAlgorithm: Unable to Assign memory for algorithm(%d) %s", algorithm, pAlgoInfo->pName));
		goto end;
	}

	/* Reset the Algorithm User Config */
	if( pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size)
	{
		BKNI_Memcpy(pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer.pAddr,
			(void *)pAlgoInfo->pDefaultUserConfig,
			pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size);
		BDSP_MMA_P_FlushCache(pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer, pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size);

		BKNI_Memcpy(pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer.pAddr,
			(void *)pAlgoInfo->pDefaultUserConfig,
			pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size);
		BDSP_MMA_P_FlushCache(pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer, pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size);
	}

	/* Reset the Algorithm Status buffer */
	if( pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size)
	{
		BKNI_Memset(pRaagaStage->stageMemInfo.sAlgoStatus.Buffer.pAddr, 0xFF, pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size);
		BDSP_MMA_P_FlushCache(pRaagaStage->stageMemInfo.sAlgoStatus.Buffer, pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size);
	}

	/* Reset the IDS Status buffer */
	if( pRaagaStage->stageMemInfo.sIdsStatus.ui32Size)
	{
		BKNI_Memset(pRaagaStage->stageMemInfo.sIdsStatus.Buffer.pAddr, 0xFF, pRaagaStage->stageMemInfo.sIdsStatus.ui32Size);
		BDSP_MMA_P_FlushCache(pRaagaStage->stageMemInfo.sIdsStatus.Buffer, pRaagaStage->stageMemInfo.sIdsStatus.ui32Size);
	}

	/* Reset the TSM Status buffer */
	if( pRaagaStage->stageMemInfo.sTsmStatus.ui32Size)
	{
		BKNI_Memset(pRaagaStage->stageMemInfo.sTsmStatus.Buffer.pAddr, 0xFF, pRaagaStage->stageMemInfo.sTsmStatus.ui32Size);
		BDSP_MMA_P_FlushCache(pRaagaStage->stageMemInfo.sTsmStatus.Buffer, pRaagaStage->stageMemInfo.sTsmStatus.ui32Size);
	}

	/* Load the Default TSM and DataSync settings for the Stage*/
	BDSP_MMA_P_CopyDataToDram(&pRaagaStage->stageMemInfo.sDataSyncSettings.Buffer,
		(void *)&BDSP_sDefaultFrameSyncSettings ,
		sizeof(BDSP_AudioTaskDatasyncSettings));

	BDSP_MMA_P_CopyDataToDram(&pRaagaStage->stageMemInfo.sTsmSettings.Buffer,
		(void *)&BDSP_sDefaultTSMSettings ,
		sizeof(BDSP_AudioTaskTsmSettings));

	/*Reset the InterStage Port Information*/
	for(index=0; index< BDSP_AF_P_MAX_OP_FORKS; index++)
	{
		pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[index].branchFromPort = 0;
		pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[index].tocIndex       = BDSP_AF_P_TOC_INVALID;
		pRaagaStage->sStageConnectionInfo.sInterStagePortInfo[index].ePortDataType  = BDSP_AF_P_DistinctOpType_eMax;
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_SetAlgorithm);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetStageSettings(
	void *pStageHandle,
	void *pSettingsBuffer,
	size_t settingsSize
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Raaga_P_GetStageSettings);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pSettingsBuffer);

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);
    if (settingsSize != pInfo->algoUserConfigSize )
    {
        BDBG_ERR(("BDSP_Raaga_P_GetStageSettings: Settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsSize, (unsigned long)pInfo->algoUserConfigSize, pRaagaStage->eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

	BDSP_MMA_P_CopyDataFromDram(pSettingsBuffer, &pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer,settingsSize);

	BDBG_LEAVE(BDSP_Raaga_P_GetStageSettings);
	return errCode;
}

BERR_Code BDSP_Raaga_P_SetStageSettings(
	void *pStageHandle,
	const void *pSettingsBuffer,
	size_t settingsSize
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Raaga_P_SetStageSettings);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pSettingsBuffer);
	BDBG_ASSERT(pRaagaStage->eAlgorithm < BDSP_Algorithm_eMax);

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);
    if (settingsSize != pInfo->algoUserConfigSize )
    {
        BDBG_ERR(("BDSP_Raaga_P_SetStageSettings: Settings buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)settingsSize, (unsigned long)pInfo->algoUserConfigSize, pRaagaStage->eAlgorithm, pInfo->pName));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

	BDSP_MMA_P_CopyDataToDram(&pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer, (void *)pSettingsBuffer, settingsSize);
	if(pRaagaStage->running)
	{
		BDSP_P_AlgoReconfigCommand sPayload;
		BDSP_RaagaTask *pRaagaTask;

		BKNI_Memset(&sPayload,0,sizeof(BDSP_P_AlgoReconfigCommand));
		pRaagaTask = pRaagaStage->pRaagaTask;

		sPayload.eAlgorithm = pRaagaStage->eAlgorithm;
		sPayload.sStageMemoryInfo.BaseAddr		= pRaagaStage->stageMemInfo.sMemoryPool.Memory.offset;
		sPayload.sStageMemoryInfo.Size     		= pRaagaStage->stageMemInfo.sMemoryPool.ui32Size;
		sPayload.sHostConfigMemoryInfo.BaseAddr = pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer.offset;
		sPayload.sHostConfigMemoryInfo.Size     = pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size;
		sPayload.sFwConfigMemoryInfo.BaseAddr 	= pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer.offset;
		sPayload.sFwConfigMemoryInfo.Size     	= pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size;

		errCode = BDSP_Raaga_P_ProcessAlgoReconfigCommand(pRaagaTask, &sPayload);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_SetStageSettings: Error in AlgoConfig Change command processing of Stage(%d) %s for Task %d",
				pRaagaStage->eAlgorithm, pInfo->pName, pRaagaTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}
	else
	{
		/* If the Stage is not running then copy into algo user config too to maintain consistency*/
		BDSP_MMA_P_CopyDataToDram(&pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer, (void *)pSettingsBuffer, settingsSize);
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_SetStageSettings);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetDatasyncSettings(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_GetDatasyncSettings);
	BDBG_ASSERT(pStageHandle);
	BDBG_ASSERT(pSettingsBuffer);

	BKNI_EnterCriticalSection();
	errCode = BDSP_Raaga_P_GetDatasyncSettings_isr(pStageHandle, pSettingsBuffer);
	BKNI_LeaveCriticalSection();
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_GetDatasyncSettings: Error in retreiving the settings"));
	}

	BDBG_LEAVE(BDSP_Raaga_P_GetDatasyncSettings);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetDatasyncSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

	BDBG_ENTER(BDSP_Raaga_P_GetDatasyncSettings_isr);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataFromDram((void *)pSettingsBuffer, &pRaagaStage->stageMemInfo.sDataSyncSettings.Buffer, sizeof(BDSP_AudioTaskDatasyncSettings));

	BDBG_LEAVE(BDSP_Raaga_P_GetDatasyncSettings_isr);
	return errCode;
}

BERR_Code BDSP_Raaga_P_SetDatasyncSettings(
	void *pStageHandle,
	const BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

	BDBG_ENTER(BDSP_Raaga_P_SetDatasyncSettings);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataToDram(&pRaagaStage->stageMemInfo.sDataSyncSettings.Buffer, (void *)pSettingsBuffer, sizeof(BDSP_AudioTaskDatasyncSettings));
	if(pRaagaStage->running)
	{
		BDSP_RaagaTask	*pRaagaTask = pRaagaStage->pRaagaTask;
		BDSP_P_DataSyncReconfigCommand sPayload;

		BKNI_Memset(&sPayload,0,sizeof(BDSP_P_DataSyncReconfigCommand));
		BKNI_Memcpy(&sPayload.sDataSyncSettings, pSettingsBuffer, sizeof(BDSP_AudioTaskDatasyncSettings));

		errCode = BDSP_Raaga_P_ProcessDataSyncReconfigCommand(pRaagaTask, &sPayload);
		if (BERR_SUCCESS != errCode)
		{
			BDBG_ERR(("BDSP_Raaga_P_SetDatasyncSettings: Error in DataSync ReConfig command processing for Task %d", pRaagaTask->taskParams.taskId));
			errCode = BERR_TRACE(errCode);
			goto end;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_SetDatasyncSettings);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetTsmSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmSettings  *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

	BDBG_ENTER(BDSP_Raaga_P_GetTsmSettings_isr);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataFromDram((void *)pSettingsBuffer, &pRaagaStage->stageMemInfo.sTsmSettings.Buffer, sizeof(BDSP_AudioTaskTsmSettings));

	BDBG_LEAVE(BDSP_Raaga_P_GetTsmSettings_isr);
	return errCode;
}

BERR_Code BDSP_Raaga_P_SetTsmSettings_isr(
	void *pStageHandle,
	const BDSP_AudioTaskTsmSettings *pSettingsBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;

	BDBG_ENTER(BDSP_Raaga_P_SetTsmSettings_isr);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pSettingsBuffer);

	BDSP_MMA_P_CopyDataToDram_isr(&pRaagaStage->stageMemInfo.sTsmSettings.Buffer, (void *)pSettingsBuffer, sizeof(BDSP_AudioTaskTsmSettings));
	BDBG_MSG(("TSM RE-CONFIG"));
	BDSP_Raaga_P_Analyse_CIT_TSMConfig(pRaagaStage->stageMemInfo.sTsmSettings.Buffer);
	if(pRaagaStage->running)
	{
		BDSP_RaagaTask  *pRaagaTask = pRaagaStage->pRaagaTask;
		BDSP_P_TsmReconfigCommand sPayload;

		BKNI_Memset(&sPayload,0,sizeof(BDSP_P_TsmReconfigCommand));
		BKNI_Memcpy(&sPayload.sTsmSettings, pSettingsBuffer, sizeof(BDSP_AudioTaskTsmSettings));

		if(pRaagaTask->taskParams.isRunning == true)
		{
			errCode = BDSP_Raaga_P_ProcessTsmReconfigCommand_isr(pRaagaTask, &sPayload);
			if (BERR_SUCCESS != errCode)
			{
				BDBG_ERR(("BDSP_Raaga_P_SetTsmSettings_isr: Error in TSM Re-Config command processing for Task %d", pRaagaTask->taskParams.taskId));
				errCode = BERR_TRACE(errCode);
				goto end;
			}
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_SetTsmSettings_isr);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetStageStatus(
	void *pStageHandle,
	void *pStatusBuffer,
	size_t statusSize
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	uint32_t *pStatusValid;

	BDBG_ENTER(BDSP_Raaga_P_GetStageStatus);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pStatusBuffer);

    pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);
    if (statusSize != pAlgoInfo->algoStatusBufferSize)
    {
        BDBG_ERR(("Status buffer size provided (%lu) does not match expected size (%lu) for algorithm %u (%s)",
                  (unsigned long)statusSize, (unsigned long)pAlgoInfo->algoStatusBufferSize, pRaagaStage->eAlgorithm, pAlgoInfo->pName));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto end;
    }

	BDSP_MMA_P_CopyDataFromDram(pStatusBuffer, &pRaagaStage->stageMemInfo.sAlgoStatus.Buffer,statusSize);

    if (pAlgoInfo->statusValidOffset != 0xffffffff)
    {
        pStatusValid = (void *)((uint8_t *)pStatusBuffer + pAlgoInfo->statusValidOffset);
        if ( 0 != *pStatusValid )
        {
            BDBG_MSG(("Status buffer for algorithm %d (%s) marked invalid",pRaagaStage->eAlgorithm, pAlgoInfo->pName));
            errCode = BDSP_ERR_BAD_DEVICE_STATE;   /* BERR_TRACE intentionally omitted */
        }
    }

end:
	BDBG_LEAVE(BDSP_Raaga_P_GetStageStatus);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetDatasyncStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncStatus *pStatusBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Raaga_P_GetDatasyncStatus_isr);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pStatusBuffer);

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);

	BDSP_MMA_P_CopyDataFromDram_isr(pStatusBuffer, &pRaagaStage->stageMemInfo.sIdsStatus.Buffer, pInfo->idsStatusBufferSize);

	if (0 != pStatusBuffer->ui32StatusValid)
	{
		BDBG_MSG(("BDSP_Raaga_P_GetDatasyncStatus_isr: Datasync Status buffer is not in valid status"));
		errCode = BDSP_ERR_BAD_DEVICE_STATE;
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_GetDatasyncStatus_isr);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetTsmStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmStatus *pStatusBuffer
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStageHandle;
	const BDSP_P_AlgorithmInfo *pInfo;

	BDBG_ENTER(BDSP_Raaga_P_GetTsmStatus_isr);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);
	BDBG_ASSERT(pStatusBuffer);

    pInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);

	BDSP_MMA_P_CopyDataFromDram_isr(pStatusBuffer, &pRaagaStage->stageMemInfo.sTsmStatus.Buffer, pInfo->tsmStatusBufferSize);
	if (0 != pStatusBuffer->ui32StatusValid)
	{
		BDBG_MSG(("BDSP_Raaga_P_GetTsmStatus_isr: TSM Status buffer is not in valid status"));
		errCode = BDSP_ERR_BAD_DEVICE_STATE;
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_GetTsmStatus_isr);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetStatus

Type        :   PI Interface

Input       :   pDeviceHandle -Device Handle which needs to be closed.
				pStatus - pointer where the Data will be filled and returned to PI

Return      :   None

Functionality   :   Return the following data to the PI.
	1)  Number of DSPs supported.
	2)  Number of Watchdog Events.
	3)  Details of Firmware version.
***********************************************************************/
void BDSP_Raaga_P_GetStatus(
	void *pDeviceHandle,
	BDSP_Status *pStatus
)
{
	BDSP_Raaga *pDevice = (BDSP_Raaga *)pDeviceHandle;
	BDBG_OBJECT_ASSERT(pDevice, BDSP_Raaga);
	pStatus->numDsp = pDevice->numDsp;
	pStatus->numWatchdogEvents=0;
	pStatus->firmwareVersion.majorVersion = BDSP_sRaagaVersion.majorVersion;
	pStatus->firmwareVersion.minorVersion = BDSP_sRaagaVersion.minorVersion;
	pStatus->firmwareVersion.branchVersion = BDSP_sRaagaVersion.branchVersion;
	pStatus->firmwareVersion.branchSubVersion = BDSP_sRaagaVersion.branchSubVersion;

	return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Pause

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Paused.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and not already paused.
		2)  Send the Pause Command to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Raaga_P_Pause(
	void *pTaskHandle
)
{
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_Pause);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if (pRaagaTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Raaga_P_Pause: Task(%d) is not started yet. Ignoring the Pause command",pRaagaTask->taskParams.taskId));
	}
	else
	{
		if (pRaagaTask->taskParams.paused == true)
		{
			BDBG_WRN(("TBDSP_Raaga_P_Pause: Task(%d) is already in Pause state. Ignoring the Pause command",pRaagaTask->taskParams.taskId));
		}
		else
		{
			errCode = BDSP_Raaga_P_ProcessPauseCommand(pRaagaTask);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_Pause: PAUSE of Task (%d) failed", pRaagaTask->taskParams.taskId));
				goto end;
			}
			pRaagaTask->taskParams.paused = true;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_Pause);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Resume

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Resumed.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and in Paused state.
		2)  Send the Resume Command to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Raaga_P_Resume(
	void *pTaskHandle
)
{
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_Resume);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if (pRaagaTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Raaga_P_Resume: Task(%d) is not started yet. Ignoring the Resume command",pRaagaTask->taskParams.taskId));
	}
	else
	{
		if (pRaagaTask->taskParams.paused == false)
		{
			BDBG_WRN(("BDSP_Raaga_P_Resume: Task(%d) is already in Resume state. Ignoring the Resume command",pRaagaTask->taskParams.taskId));
		}
		else
		{
			errCode = BDSP_Raaga_P_ProcessResumeCommand(pRaagaTask);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_Resume: PAUSE of Task (%d) failed",pRaagaTask->taskParams.taskId));
				goto end;
			}
			pRaagaTask->taskParams.paused = false;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_Resume);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_AudioGapFill

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be zero Gap fill.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		This command puts FW in zero fill mode. This is needed to handle NRT Xcode case. In NRT xcode, decoder
		has to trigger STC to move it forward and if there is no data in the input CDB, it is supposed to do nothing
		so if there is an audio gap, audio stalls and does not trigger STC which in turn stall video also by virtue of
		AV_WINDOW. This command will be called by upper layer when it detects such kind of gap in stream and
		this will make sure aduio fills zeroes and STC moves forward avoiding the deadlock.
***********************************************************************/
BERR_Code BDSP_Raaga_P_AudioGapFill(
	void *pTaskHandle
)
{
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;

	BDBG_ENTER(BDSP_Raaga_P_AudioGapFill);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if (pRaagaTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Raaga_P_AudioGapFill: Task(%d) is not started yet. Ignoring the Audio Gap Fill command",pRaagaTask->taskParams.taskId));
	}
	else
	{
		if (pRaagaTask->taskParams.paused == true)
		{
			BDBG_WRN(("BDSP_Raaga_P_AudioGapFill: Task(%d) is already in Paused state. Ignoring the Audio Gap Fill command",pRaagaTask->taskParams.taskId));
		}
		else
		{
			errCode = BDSP_Raaga_P_ProcessAudioGapFillCommand(pRaagaTask);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_AudioGapFill: Audio Gap Fill of Task (%d) failed",pRaagaTask->taskParams.taskId));
				goto end;
			}
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_AudioGapFill);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Advance

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Advanced.
		     timeInMs      -   Time provided to advance in milli-seconds.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and in Paused state.
		2)  Send the Advance Command to the FW with time and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Raaga_P_Advance(
	void *pTaskHandle,
	unsigned timeInMs
)
{
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_FrameAdvanceCommand PayLoad;

	BDBG_ENTER(BDSP_Raaga_P_Advance);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if (pRaagaTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Raaga_P_Advance: Task(%d) is not started yet. Ignoring the Advance command",pRaagaTask->taskParams.taskId));
	}
	else
	{
		if (pRaagaTask->taskParams.paused == false)
		{
			BDBG_WRN(("BDSP_Raaga_P_Advance: Task (%d) is not in Pause state. Ignoring the Advance command",pRaagaTask->taskParams.taskId));
		}
		else
		{
			PayLoad.ui32DurationOfFrameAdv = timeInMs * 45;
			errCode = BDSP_Raaga_P_ProcessFrameAdvanceCommand(pRaagaTask, &PayLoad);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_Advance: FRAME ADVANCE of Task (%d) failed",pRaagaTask->taskParams.taskId));
			}
		}
	}

	BDBG_LEAVE(BDSP_Raaga_P_Advance);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_Freeze

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Freezed.
				pSettings       -   Setting to be used to Freeze

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and not in frozen state.
		2)  Retreive the STC address ( lower 32 bit) and provide it to FW for internal processing of STC snapshotting.
		3)  Send the Freeze Command with dummy FMM port details to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Raaga_P_Freeze(
	void *pTaskHandle,
	const BDSP_AudioTaskFreezeSettings *pSettings
)
{
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_AudioOutputFreezeCommand PayLoad;

	BDBG_ENTER(BDSP_Raaga_P_Freeze);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if (pRaagaTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Raaga_P_Freeze: Task(%d) is not started yet. Ignoring the Audio Freeze command",pRaagaTask->taskParams.taskId));
	}
	else
	{
		if(pRaagaTask->taskParams.frozen == true)
		{
			BDBG_WRN(("BDSP_Raaga_P_Freeze: Task (%d) is already in Frozen state. Ignoring the Audio Freeze command",pRaagaTask->taskParams.taskId));
		}
		else
		{
			BDBG_ERR(("BDSP_Raaga_P_Freeze: Implementation is not yet complete"));
			BDBG_MSG(("FMM OUTPUT ADDR = 0x%x",pSettings->fmmOutputAddress));
			BDBG_MSG(("FMM OUTPUT MASK = 0x%x",pSettings->fmmOutputMask));
			BDBG_MSG(("FMM OUTPUT VAL  = 0x%x",pSettings->fmmOutputValue));
			PayLoad.dummy = 0xFF;
			errCode = BDSP_Raaga_P_ProcessAudioOutputFreezeCommand(pRaagaTask, &PayLoad);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_Freeze: AUDIO FREEZE of Task (%d) failed",pRaagaTask->taskParams.taskId));
				goto end;
			}
			pRaagaTask->taskParams.frozen = true;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_Freeze);
	return errCode;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_UnFreeze

Type        :   PI Interface

Input       :   pTaskHandle -   Handle of the Task which needs to be Freezed.
				pSettings       -   Setting to be used to Un-Freeze

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
		1)  Check if the task is started and in frozen state.
		2)  Retreive the STC address ( lower 32 bit) and provide it to FW for internal processing of STC snapshotting.
		3)  Send the UnFreeze Command with dummy FMM port details to the FW and wait for the acknowledgement.
***********************************************************************/
BERR_Code BDSP_Raaga_P_UnFreeze(
	void *pTaskHandle,
	const BDSP_AudioTaskUnFreezeSettings *pSettings
)
{
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_P_AudioOutputUnFreezeCommand PayLoad;

	BDBG_ENTER(BDSP_Raaga_P_UnFreeze);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	if (pRaagaTask->taskParams.isRunning == false)
	{
		BDBG_WRN(("BDSP_Raaga_P_UnFreeze: Task(%d) is not started yet. Ignoring the Audio UnFreeze command",pRaagaTask->taskParams.taskId));
	}
	else
	{
		if(pRaagaTask->taskParams.frozen == false)
		{
			BDBG_WRN(("BDSP_Raaga_P_UnFreeze: Task (%d) is already in running state. Ignoring the Audio UnFreeze command",pRaagaTask->taskParams.taskId));
		}
		else
		{
			BDBG_ERR(("BDSP_Raaga_P_UnFreeze: Implementation is not yet complete"));
			BDBG_MSG(("FMM OUTPUT ADDR = 0x%x",pSettings->fmmOutputAddress));
			BDBG_MSG(("FMM OUTPUT MASK = 0x%x",pSettings->fmmOutputMask));
			BDBG_MSG(("FMM OUTPUT VAL  = 0x%x",pSettings->fmmOutputValue));
			BDBG_MSG(("NUM BUFFERS     = %d",pSettings->ui32NumBuffers));
			PayLoad.dummy = 0xFF;
			errCode = BDSP_Raaga_P_ProcessAudioOutputUnFreezeCommand(pRaagaTask, &PayLoad);
			if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_UnFreeze: AUDIO UNFREEZE of Task (%d) failed",pRaagaTask->taskParams.taskId));
				goto end;
			}
			pRaagaTask->taskParams.frozen = false;
		}
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_UnFreeze);
	return errCode;
}
