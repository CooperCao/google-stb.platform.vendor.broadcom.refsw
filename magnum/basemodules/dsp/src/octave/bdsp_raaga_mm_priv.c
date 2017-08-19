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

BDBG_MODULE(bdsp_raaga_mm);

void BDSP_Raaga_P_CalculateInitMemory(
	unsigned *pMemReqd
)
{
    BDBG_ENTER(BDSP_Raaga_P_CalculateInitMemory);
    *pMemReqd = 0;

    /*Memory for Command Queue, accounting for maximun commands per task(10) for max (12) tasks in a single DSP*/
    *pMemReqd += (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP);
    /* Memory for Generic Response Queue, Maximum of (10) responses from the DSP */
    *pMemReqd += (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response));

	BDBG_MSG(("Init Memory = %d",*pMemReqd));
    BDBG_LEAVE(BDSP_Raaga_P_CalculateInitMemory);
}

void BDSP_Raaga_P_CalculateDebugMemory(
    BDSP_RaagaSettings *pSettings,
    unsigned           *pMemReqd
)
{
    unsigned index= 0;
    unsigned MemoryRequired = 0;
    BDBG_ENTER(BDSP_Raaga_P_CalculateDebugMemory);

    for (index = 0; index < BDSP_Raaga_DebugType_eLast; index++)
    {
        if(pSettings->debugSettings[index].enabled)
        {
            MemoryRequired += pSettings->debugSettings[index].bufferSize;
        }
        else
        {
            MemoryRequired += 4;
        }
    }

    *pMemReqd = MemoryRequired;
	BDBG_MSG(("Debug Memory = %d",MemoryRequired));
    BDBG_LEAVE(BDSP_Raaga_P_CalculateDebugMemory);
}

static void BDSP_Raaga_P_CalculateProcessRWMemory(
	unsigned *pMemReqd
)
{
    unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateProcessRWMemory);
    *pMemReqd = 0;

    MemoryRequired += BDSP_IMG_KERNEL_RW_IMG_SIZE;
    MemoryRequired += (BDSP_IMG_USER_PROCESS_SPAWN_MEM_SIZE*BDSP_RAAGA_MAX_NUM_USER_PROCESS);
    MemoryRequired += BDSP_IMG_DEFAULT_MM_PROC_HEAP_SIZE;
	MemoryRequired += BDSP_IMG_INIT_PROCESS_MEM_SIZE;
    MemoryRequired += BDSP_IMG_TOPLEVEL_PROCESS_MEM_SIZE;

    *pMemReqd = MemoryRequired;
	BDBG_MSG(("Process RW Memory = %d",MemoryRequired));

    BDBG_LEAVE(BDSP_Raaga_P_CalculateProcessRWMemory);
}

static void BDSP_Raaga_P_CalculateScratchAndInterStageMemory(
    BDSP_Raaga *pDevice,
    unsigned    numCorePerDsp,
    unsigned   *pMemReqd,
    bool        ifMemApiTool,
	const BDSP_RaagaUsageOptions *pUsage
)
{
    unsigned MemoryRequired = 0;
	unsigned preemptionLevel = 0;
	BDSP_Algorithm algorithm;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	bool supported = false;

    BDBG_ENTER(BDSP_Raaga_P_CalculateScratchAndInterStageMemory);
    *pMemReqd = 0;

	for(preemptionLevel=0; preemptionLevel<BDSP_RAAGA_MAX_NUM_PREEMPTION_LEVELS; preemptionLevel++)
	{
		unsigned interStageBufferReqd=0, scratchBufferReqd = 0;
		for(algorithm=0; algorithm<BDSP_Algorithm_eMax; algorithm++)
		{
			pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(algorithm);

			if(ifMemApiTool)
			{
				/*Memory Estimate API */
				supported = pUsage->Codeclist[algorithm];
			}
			else
			{
				/* Normal Path */
				const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
				pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(algorithm);
				supported = pAlgoSupportInfo->supported;
			}

			if((supported)&&(pAlgoInfo->bPreemptionLevelSupported[preemptionLevel]))
			{
				if(scratchBufferReqd < pAlgoInfo->scratchBufferSize)
					scratchBufferReqd = pAlgoInfo->scratchBufferSize;
				if(interStageBufferReqd < (pAlgoInfo->maxChannelsSupported*pAlgoInfo->samplesPerChannel*4))
					interStageBufferReqd = (pAlgoInfo->maxChannelsSupported*pAlgoInfo->samplesPerChannel*4);
			}
		}
		MemoryRequired += (scratchBufferReqd+
			((interStageBufferReqd + INTERSTAGE_EXTRA_SIZE_ALIGNED) *(BDSP_RAAGA_MAX_BRANCH+1)));
		if(!ifMemApiTool)
		{
			/* Normal Path */
			pDevice->memInfo.WorkBufferMemory[preemptionLevel].ui32Size = (scratchBufferReqd+
					((interStageBufferReqd + INTERSTAGE_EXTRA_SIZE_ALIGNED)*(BDSP_RAAGA_MAX_BRANCH+1)));
		}
		BDBG_MSG(("Work Buffer Allocation for Preemption Level %d",preemptionLevel));
		BDBG_MSG(("Scratch = %d InterStage = %d Total(Scratch + %d Interstage) = %d",scratchBufferReqd,interStageBufferReqd,
			(BDSP_RAAGA_MAX_BRANCH+1),MemoryRequired));
	}

    *pMemReqd = (MemoryRequired*numCorePerDsp);
	BDBG_MSG(("Total Work Buffer Allocation for %d Cores = %d",numCorePerDsp,*pMemReqd));
    BDBG_LEAVE(BDSP_Raaga_P_CalculateScratchAndInterStageMemory);
}

static void BDSP_Raaga_P_CalculateDescriptorMemory(
	unsigned *pMemReqd
)
{
    unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateDescriptorMemory);
    *pMemReqd = 0;

	MemoryRequired = (BDSP_RAAGA_MAX_DESCRIPTORS * sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
    *pMemReqd = MemoryRequired;

	BDBG_MSG(("Descriptor Memory = %d",MemoryRequired));
    BDBG_LEAVE(BDSP_Raaga_P_CalculateDescriptorMemory);
}

void BDSP_Raaga_P_CalculateStageMemory(
	unsigned    *pMemReqd,
    bool        ifMemApiTool,
	const BDSP_RaagaUsageOptions *pUsage
)
{
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
	const BDSP_P_AlgorithmInfo        *pAlgoInfo;
	unsigned i = 0;
	unsigned MemoryRequired = 0, tempSize = 0;
	bool supported = false;

    BDBG_ENTER(BDSP_Raaga_P_CalculateStageMemory);
	*pMemReqd = 0;
    for(i=0; i < BDSP_Algorithm_eMax; i++)
    {
		tempSize = 0;
		supported =  false;
		if(ifMemApiTool)
		{
			/*Memory Estimate API */
			supported = pUsage->Codeclist[i];
		}
		else
		{
			/* Normal Path */
			pAlgoSupportInfo = BDSP_Raaga_P_LookupAlgorithmSupportInfo(i);
			supported = pAlgoSupportInfo->supported;
		}
		if(supported)
		{
			pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(i);
			tempSize += pAlgoInfo->interFrameSize;
			tempSize += (pAlgoInfo->algoUserConfigSize*2); /* Host copy  and DSP copy*/
			tempSize += pAlgoInfo->algoStatusBufferSize;
			tempSize += pAlgoInfo->idsStatusBufferSize;
			tempSize += pAlgoInfo->tsmStatusBufferSize;
			tempSize += sizeof(BDSP_AudioTaskDatasyncSettings);
			tempSize += sizeof(BDSP_AudioTaskTsmSettings);
			tempSize += BDSP_MAX_HOST_DSP_L2C_SIZE; /* Hole memory to beat Cache coherency*/
			if(tempSize > MemoryRequired)
				MemoryRequired = tempSize;
		}
    }

	MemoryRequired = BDSP_RAAGA_ALIGN_SIZE(MemoryRequired, 4096);  /* Align the size to 4k */
	BDBG_MSG(("BDSP_Raaga_P_CalculateStageMemory: Memory Allocated for Stage = %d", MemoryRequired));
	*pMemReqd = MemoryRequired;

	BDBG_LEAVE(BDSP_Raaga_P_CalculateStageMemory);
}

void BDSP_Raaga_P_CalculateTaskMemory(
	unsigned *pMemReqd
)
{
	unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateTaskMemory);
	*pMemReqd = 0;

	MemoryRequired += BDSP_IMG_MP_AP_SHARED_MEMORY_SIZE;
	MemoryRequired += (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response));
	MemoryRequired += (BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynMsg));
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_CONFIG); /* CIT Memory */
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_CONFIG); /* CIT Re-Config Memory */
	MemoryRequired += sizeof(BDSP_AF_P_sOpSamplingFreq); /*Sample Rate LUT*/
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG);
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_GATEOPEN_CONFIG);
	MemoryRequired += BDSP_MAX_HOST_DSP_L2C_SIZE;/* Hole memory to beat Cache coherency*/

	MemoryRequired = BDSP_RAAGA_ALIGN_SIZE(MemoryRequired, 4096);  /* Align the size to 4k */
	*pMemReqd = MemoryRequired;

	BDBG_MSG(("BDSP_Raaga_P_CalculateTaskMemory: Memory Allocated for Task = %d", MemoryRequired));
	BDBG_LEAVE(BDSP_Raaga_P_CalculateTaskMemory);
}

void BDSP_Raaga_P_CalculateDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned   *pMemReqd)
{
    unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateDeviceRWMemory);

    BDSP_Raaga_P_CalculateInitMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

    BDSP_Raaga_P_CalculateDebugMemory(&pDevice->deviceSettings, &MemoryRequired);
    *pMemReqd += MemoryRequired;

    BDSP_Raaga_P_CalculateScratchAndInterStageMemory(pDevice, pDevice->numCorePerDsp, &MemoryRequired, false, NULL);
    *pMemReqd += MemoryRequired;

    BDSP_Raaga_P_CalculateProcessRWMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

    BDSP_Raaga_P_CalculateDescriptorMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

	*pMemReqd = BDSP_RAAGA_ALIGN_SIZE(*pMemReqd, 4096);/* Align the size to 4k */

	BDBG_MSG(("BDSP_Raaga_P_CalculateDeviceRWMemory: Memory Allocated for Device RW = %d", *pMemReqd));
	BDBG_LEAVE(BDSP_Raaga_P_CalculateDeviceRWMemory);
}

void BDSP_Raaga_P_CalculateDeviceROMemory(
    BDSP_Raaga *pDevice,
    unsigned *pMemReqd
)
{
    unsigned ResidentMemorySize = 0, LoadableMemorySize = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateDeviceROMemory);
	*pMemReqd = 0;

	BDSP_Raaga_P_ComputeResidentSection(&pDevice->codeInfo, &ResidentMemorySize);
	BDBG_MSG(("BDSP_Raaga_P_CalculateDeviceROMemory: Resident code Memory Requirement = %d",ResidentMemorySize));
	*pMemReqd += ResidentMemorySize;

	BDSP_Raaga_P_ComputeLoadbleSection((void *)pDevice, &LoadableMemorySize);
	BDBG_MSG(("BDSP_Raaga_P_CalculateDeviceROMemory: Loadable code Memory Requirement = %d",LoadableMemorySize));
	*pMemReqd += LoadableMemorySize;

	*pMemReqd += 1024;
	*pMemReqd = BDSP_RAAGA_ALIGN_SIZE(*pMemReqd,4096);

	BDBG_MSG(("BDSP_Raaga_P_CalculateDeviceROMemory: Memory Allocated for Device RO = %d", *pMemReqd));
	BDBG_LEAVE(BDSP_Raaga_P_CalculateDeviceROMemory);
}

BERR_Code BDSP_Raaga_P_RequestMemory(
    BDSP_P_MemoryPool *pMemoryPool,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_RequestMemory);
    if((pMemoryPool->ui32UsedSize + ui32Size) > pMemoryPool->ui32Size)
    {
        BDBG_ERR(("BDSP_Raaga_P_RequestMemory: Cannot Assign the requested Size"));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }

    *pMemory = pMemoryPool->Memory;
    pMemory->offset = pMemory->offset + pMemoryPool->ui32UsedSize;
    pMemory->pAddr  = (void *)((uint8_t *)pMemory->pAddr + pMemoryPool->ui32UsedSize);

    pMemoryPool->ui32UsedSize += ui32Size;

end:
    BDBG_LEAVE(BDSP_Raaga_P_RequestMemory);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ReleaseMemory(
    BDSP_P_MemoryPool *pMemoryPool,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_ReleaseMemory);
    if(pMemoryPool->ui32UsedSize < ui32Size)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseMemory: Trying the release memory more than used"));
        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto end;
    }

    pMemoryPool->ui32UsedSize -= ui32Size;
	BKNI_Memset(pMemory, 0, sizeof(BDSP_MMA_Memory));

end:
    BDBG_LEAVE(BDSP_Raaga_P_ReleaseMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_AssignSharedKernalMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Raaga_P_AssignSharedKernalMemory);

    errCode = BDSP_Raaga_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], BDSP_IMG_KERNEL_RW_IMG_SIZE, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignSharedKernalMemory: Unable to Assign Shared Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.SharedKernalMemory[dspindex].Buffer   = Memory;
    pDevice->memInfo.SharedKernalMemory[dspindex].ui32Size = BDSP_IMG_KERNEL_RW_IMG_SIZE;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignSharedKernalMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseSharedKernalMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_ReleaseSharedKernalMemory);

    errCode = BDSP_Raaga_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.SharedKernalMemory[dspindex].ui32Size,
		&pDevice->memInfo.SharedKernalMemory[dspindex].Buffer);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_DeAssignSharedKernalMemory: Unable to DeAssign Shared Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.SharedKernalMemory[dspindex].ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseSharedKernalMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_AssignInitMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ui32Size = 0;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Raaga_P_AssignInitMemory);

    errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice,dspindex,&(pDevice->memInfo.cmdQueueParams[dspindex].ui32FifoId), 1);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to find free fifo for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    ui32Size = BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Command)*BDSP_RAAGA_MAX_FW_TASK_PER_DSP;
    errCode = BDSP_Raaga_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to allocate RW memory for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.cmdQueueParams[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.cmdQueueParams[dspindex].Memory   = Memory;

    errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice,dspindex,&(pDevice->memInfo.genRspQueueParams[dspindex].ui32FifoId), 1);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to find free fifo for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    ui32Size = (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response));
    errCode = BDSP_Raaga_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to allocate RW memory for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.genRspQueueParams[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.genRspQueueParams[dspindex].Memory   = Memory;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignInitMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseInitMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_ReleaseInitMemory);

    errCode = BDSP_Raaga_P_ReleaseFIFO(pDevice,dspindex,&(pDevice->memInfo.cmdQueueParams[dspindex].ui32FifoId), 1);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release fifo for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    errCode = BDSP_Raaga_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.cmdQueueParams[dspindex].ui32Size,
		&pDevice->memInfo.cmdQueueParams[dspindex].Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release RW memory for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
	pDevice->memInfo.cmdQueueParams[dspindex].ui32Size 	= 0;

    errCode = BDSP_Raaga_P_ReleaseFIFO(pDevice,dspindex,&(pDevice->memInfo.genRspQueueParams[dspindex].ui32FifoId), 1);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release fifo for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }

	errCode = BDSP_Raaga_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.genRspQueueParams[dspindex].ui32Size,
		&pDevice->memInfo.genRspQueueParams[dspindex].Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release RW memory for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
		goto end;
	}
	pDevice->memInfo.genRspQueueParams[dspindex].ui32Size 	= 0;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseInitMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_AssignDescriptorMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ui32Size = 0;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Raaga_P_AssignDescriptorMemory);

    ui32Size = (BDSP_RAAGA_MAX_DESCRIPTORS * sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
    errCode = BDSP_Raaga_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to allocate RW memory for Descriptors for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.DescriptorMemory[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.DescriptorMemory[dspindex].Buffer   = Memory;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignDescriptorMemory);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseDescriptorMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_ReleaseDescriptorMemory);

    errCode = BDSP_Raaga_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.DescriptorMemory[dspindex].ui32Size,
		&pDevice->memInfo.DescriptorMemory[dspindex].Buffer);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDescriptorMemory: Unable to Release RW memory for Descriptors for dsp %d!!!!",dspindex));
        goto end;
    }
	pDevice->memInfo.DescriptorMemory[dspindex].ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseDescriptorMemory);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_AssignDebugMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	uint32_t ui32Size = 0, index =0;
	BDSP_MMA_Memory Memory;
	BDBG_ENTER(BDSP_Raaga_P_AssignDebugMemory);

	for(index = 0; index < BDSP_Raaga_DebugType_eLast;index++)
	{
		if(pDevice->deviceSettings.debugSettings[index].enabled)
		{
			/* For Debug Infrastructure, the FIFOs have been pre defined compile time in rdbvars
			     No need to have call a resource manager call to get fifo
				59 - DRAM
				60 - UART
				61 - CORE
				62 - TARGET PRINT
			*/
			switch(index)
			{
				case BDSP_Raaga_DebugType_eDramMsg:
					pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_RAAGA_DRAM_FIFO;
					break;
				case BDSP_Raaga_DebugType_eUart:
					pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_RAAGA_UART_FIFO;
					break;
				case BDSP_Raaga_DebugType_eCoreDump:
					pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_RAAGA_CORE_FIFO;
					break;
				case BDSP_Raaga_DebugType_eTargetPrintf:
#ifdef FIREPATH_BM
					pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId =
										(BCHP_PHYSICAL_OFFSET | BCHP_RAAGA_DSP_FP_MISC_0_CORESTATE_SYS_MBX0);
#else
					pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BCHP_RAAGA_DSP_FP_MISC_0_CORESTATE_SYS_MBX0;
#endif /*FIREPATH_BM*/
				default:
					break;
			}
			ui32Size = pDevice->deviceSettings.debugSettings[index].bufferSize;
			errCode  = BDSP_Raaga_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
			if(errCode)
			{
				BDBG_ERR(("BDSP_Raaga_P_AssignDebugMemory: Unable to allocate RW memory for Debug type %d for dsp %d!!!!",index, dspindex));
				goto end;
			}
			pDevice->memInfo.debugQueueParams[dspindex][index].ui32Size = ui32Size;
			pDevice->memInfo.debugQueueParams[dspindex][index].Memory   = Memory;
		}
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignDebugMemory);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseDebugMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned index =0;
	BDBG_ENTER(BDSP_Raaga_P_ReleaseDebugMemory);

	for(index = 0; index < BDSP_Raaga_DebugType_eLast;index++)
	{
		if(pDevice->deviceSettings.debugSettings[index].enabled)
		{
			errCode = BDSP_Raaga_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
				pDevice->memInfo.debugQueueParams[dspindex][index].ui32Size,
				&pDevice->memInfo.debugQueueParams[dspindex][index].Memory);
			if(errCode)
			{
				BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release RW memory for Debug type %d for dsp %d!!!!",index,dspindex));
				goto end;
			}
			pDevice->memInfo.debugQueueParams[dspindex][index].ui32Size	= 0;
			pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_RAAGA_FIFO_INVALID;
		}
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseDebugMemory);
	return errCode;
}

BERR_Code BDSP_Raaga_P_AssignDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_AssignDeviceRWMemory);

    errCode = BDSP_Raaga_P_AssignSharedKernalMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Shared Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_AssignInitMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Init Memory for dsp %d!!!!",dspindex));
        goto end;
    }

	errCode = BDSP_Raaga_P_AssignDebugMemory(pDevice, dspindex);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Debug Memory for dsp %d!!!!",dspindex));
		goto end;
	}

	errCode = BDSP_Raaga_P_AssignDescriptorMemory(pDevice, dspindex);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Descriptor Memory for dsp %d!!!!",dspindex));
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignDeviceRWMemory);
    return errCode;
}

BERR_Code BDSP_Raaga_P_ReleaseDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_ReleaseDeviceRWMemory);

    errCode = BDSP_Raaga_P_ReleaseSharedKernalMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Shared Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_ReleaseDebugMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Debug Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_ReleaseInitMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Init Memory for dsp %d!!!!",dspindex));
        goto end;
    }

	errCode = BDSP_Raaga_P_ReleaseDescriptorMemory(pDevice, dspindex);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Descriptor Memory for dsp %d!!!!",dspindex));
		goto end;
	}
end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseDeviceRWMemory);
    return errCode;
}

BERR_Code BDSP_Raaga_P_AssignTaskMemory(
	void *pTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTask;
	BDSP_Raaga *pDevice;
	unsigned dspIndex =0;
	uint32_t ui32Size =0;
	BDSP_MMA_Memory Memory;

    BDBG_ENTER(BDSP_Raaga_P_AssignTaskMemory);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

	pDevice = (BDSP_Raaga *)pRaagaTask->pContext->pDevice;
	dspIndex = pRaagaTask->createSettings.dspIndex;

	/*Host Copy of Async Response Queue */
	pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr = BKNI_Malloc(BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynMsg));
	if(NULL == pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to allocate BKNI Memory for Async buffer copy of HOST %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.hostAsyncQueue.ui32Size = (BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynMsg));

    ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for CIT Buffer of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCITMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sCITMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for CIT Reconfig Buffer of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCITReConfigMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sCITReConfigMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sOpSamplingFreq);
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for Sample Rate LUT of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSampleRateLUTMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sSampleRateLUTMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG);
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for Scheduling Config of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSchedulingConfigMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sSchedulingConfigMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_GATEOPEN_CONFIG);
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for Gate Open Config of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sGateOpenConfigMemory.Buffer	= Memory;
	pRaagaTask->taskMemInfo.sGateOpenConfigMemory.ui32Size  = ui32Size;

    ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
    errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign Cache Hole Memory of Task %d",pRaagaTask->taskParams.taskId));
        goto end;
    }
    pRaagaTask->taskMemInfo.sCacheHole.ui32Size = ui32Size;
    pRaagaTask->taskMemInfo.sCacheHole.Buffer   = Memory;

    errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice, dspIndex,&(pRaagaTask->taskMemInfo.syncQueueParams.ui32FifoId), 1);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to find free fifo for SYNC QUEUE of TASK %d",pRaagaTask->taskParams.taskId));
        goto end;
    }
    ui32Size = (BDSP_RAAGA_MAX_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_Response));
    errCode = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for SYNC QUEUE of Task %d",pRaagaTask->taskParams.taskId));
        goto end;
    }
    pRaagaTask->taskMemInfo.syncQueueParams.ui32Size = ui32Size;
    pRaagaTask->taskMemInfo.syncQueueParams.Memory   = Memory;

	errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice, dspIndex,&(pRaagaTask->taskMemInfo.asyncQueueParams.ui32FifoId), 1);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to find free fifo for ASYNC QUEUE of TASK %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	ui32Size = (BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_Raaga_P_AsynMsg));
	errCode = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for ASYNC QUEUE of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.asyncQueueParams.ui32Size = ui32Size;
	pRaagaTask->taskMemInfo.asyncQueueParams.Memory	  = Memory;

	ui32Size = BDSP_IMG_MP_AP_SHARED_MEMORY_SIZE;
	errCode = BDSP_Raaga_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for MP Shared Memory of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sMPSharedMemory.ui32Size = ui32Size;
	pRaagaTask->taskMemInfo.sMPSharedMemory.Buffer	 = Memory;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignTaskMemory);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ReleaseTaskMemory(
	void *pTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTask;
	BDSP_Raaga *pDevice;
	unsigned dspIndex =0;

    BDBG_ENTER(BDSP_Raaga_P_ReleaseTaskMemory);
	BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
	pDevice = (BDSP_Raaga *)pRaagaTask->pContext->pDevice;
	dspIndex = pRaagaTask->createSettings.dspIndex;

	if(pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr)
	{
		BKNI_Free(pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr);
		pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr = 0;
		pRaagaTask->taskMemInfo.hostAsyncQueue.ui32Size = 0;
	}

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sCITMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sCITMemory.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release CIT Memory of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCITMemory.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sCITReConfigMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sCITReConfigMemory.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release CIT ReConfig Memory of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCITReConfigMemory.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sSampleRateLUTMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sSampleRateLUTMemory.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Sample Rate LUT of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSampleRateLUTMemory.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sSchedulingConfigMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sSchedulingConfigMemory.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Scheduling Config of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSchedulingConfigMemory.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sGateOpenConfigMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sGateOpenConfigMemory.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Gate Open Config of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sGateOpenConfigMemory.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sCacheHole.ui32Size,
		&pRaagaTask->taskMemInfo.sCacheHole.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Cache Hole Memory of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCacheHole.ui32Size = 0;

    errCode = BDSP_Raaga_P_ReleaseFIFO(pDevice,dspIndex,&(pRaagaTask->taskMemInfo.syncQueueParams.ui32FifoId), 1);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release fifo for SYNC QUEUE of Task %d",pRaagaTask->taskParams.taskId));
        goto end;
    }
    errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.syncQueueParams.ui32Size,
		&pRaagaTask->taskMemInfo.syncQueueParams.Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release RW memory for SYNC QUEUE of Task %d",pRaagaTask->taskParams.taskId));
        goto end;
    }
	pRaagaTask->taskMemInfo.syncQueueParams.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseFIFO(pDevice,dspIndex,&(pRaagaTask->taskMemInfo.asyncQueueParams.ui32FifoId), 1);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release fifo for ASYNC QUEUE of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.asyncQueueParams.ui32Size,
		&pRaagaTask->taskMemInfo.asyncQueueParams.Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release RW memory for ASYNC QUEUE of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.asyncQueueParams.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sMPSharedMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sMPSharedMemory.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release RW memory for MP Shared of Task %d",pRaagaTask->taskParams.taskId));
		goto end;
	}
	pRaagaTask->taskMemInfo.sMPSharedMemory.ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseTaskMemory);
	return errCode;
}

BERR_Code BDSP_Raaga_P_AssignStageMemory(
	void *pStage
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStage;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	uint32_t ui32Size =0;
	BDSP_MMA_Memory Memory;

	BDBG_ENTER(BDSP_Raaga_P_AssignStageMemory);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);

	pAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);

	ui32Size = sizeof(BDSP_AudioTaskDatasyncSettings);
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign DataSync Config memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pRaagaStage->stageMemInfo.sDataSyncSettings.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sDataSyncSettings.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AudioTaskTsmSettings);
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign TSM Config memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pRaagaStage->stageMemInfo.sTsmSettings.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sTsmSettings.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoUserConfigSize;
	errCode = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Host UserConfig memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size= ui32Size;

	/* Allocation of 512 bytes of hole memory to beat Cache Coherancy*/
	ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Cache Hole memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sCacheHole.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sCacheHole.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoUserConfigSize;
	errCode  = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign UserConfig memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoStatusBufferSize;
	errCode = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Status memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sAlgoStatus.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->idsStatusBufferSize;
	errCode = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign IDS Status memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sIdsStatus.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sIdsStatus.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->tsmStatusBufferSize;
	errCode = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign TSM Status memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sTsmStatus.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sTsmStatus.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->interFrameSize;
	errCode = BDSP_Raaga_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Interframe memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pRaagaStage->stageMemInfo.sInterframe.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sInterframe.ui32Size= ui32Size;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignStageMemory);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ReleaseStageMemory(
	void *pStage
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_RaagaStage *pRaagaStage = (BDSP_RaagaStage *)pStage;

	BDBG_ENTER(BDSP_Raaga_P_ReleaseStageMemory);

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sDataSyncSettings.ui32Size,
		&pRaagaStage->stageMemInfo.sDataSyncSettings.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release IDS Settings memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sDataSyncSettings.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sTsmSettings.ui32Size,
		&pRaagaStage->stageMemInfo.sTsmSettings.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release TSM Settings memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sTsmSettings.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size,
		&pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release Host UserConfig memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sCacheHole.ui32Size,
		&pRaagaStage->stageMemInfo.sCacheHole.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release Cache Hole memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sCacheHole.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size,
		&pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release UserConfig memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size,
		&pRaagaStage->stageMemInfo.sAlgoStatus.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release Algo Status memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sIdsStatus.ui32Size,
		&pRaagaStage->stageMemInfo.sIdsStatus.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release IDS Status memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sIdsStatus.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sTsmStatus.ui32Size,
		&pRaagaStage->stageMemInfo.sTsmStatus.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release TSM Status memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sTsmStatus.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sInterframe.ui32Size,
		&pRaagaStage->stageMemInfo.sInterframe.Buffer);
	if(errCode)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release Interframe memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sInterframe.ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseStageMemory);
	return errCode;
}

BERR_Code BDSP_Raaga_P_AssignDescriptor(
	void *pDeviceHandle,
	unsigned dspIndex,
	BDSP_MMA_Memory *pMemory,
	unsigned numDescriptors
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice;
	unsigned index = 0, allocatedDescriptors = 0;
	BDSP_MMA_Memory Memory;
	BDBG_ENTER(BDSP_Raaga_P_AssignDescriptor);
	pDevice = (BDSP_Raaga *)pDeviceHandle;
	BKNI_AcquireMutex(pDevice->deviceMutex);

	BDBG_MSG(("BDSP_Raaga_P_AssignDescriptor: Number of descriptor = %d",numDescriptors));

	while(allocatedDescriptors != numDescriptors)
	{
		for(;index<BDSP_RAAGA_MAX_DESCRIPTORS; index++)
		{
			if(pDevice->hardwareStatus.descriptor[dspIndex][index]== false)
			{
				BDBG_MSG(("BDSP_Raaga_P_AssignDescriptor:index =%d",index));
				break;
			}
		}
		if(index < BDSP_RAAGA_MAX_DESCRIPTORS)
		{
			pDevice->hardwareStatus.descriptor[dspIndex][index]=true;
			Memory = pDevice->memInfo.DescriptorMemory[dspIndex].Buffer;
			Memory.pAddr = (void *)((uint8_t *)pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.pAddr+(index *sizeof(BDSP_AF_P_sCIRCULAR_BUFFER)));
			Memory.offset= pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset+(index *sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
			BDBG_MSG(("BDSP_Raaga_P_AssignDescriptor pAddr = %p Offset ="BDSP_MSG_FMT, Memory.pAddr, BDSP_MSG_ARG(Memory.offset)));
			*pMemory = Memory;
			pMemory++;
			allocatedDescriptors++;
		}
		else
		{
			BDBG_ERR(("BDSP_Raaga_P_AssignDescriptor: Ran out of Descriptors requested =%d, assigned = %d",numDescriptors,allocatedDescriptors));
			errCode = BERR_INVALID_PARAMETER;
			break;
		}
	}
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	BDBG_LEAVE(BDSP_Raaga_P_AssignDescriptor);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ReleaseDescriptor(
	void *pDeviceHandle,
	unsigned dspIndex,
	dramaddr_t *pOffset,
	unsigned numDescriptors
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Raaga *pDevice;
	unsigned index = 0, releasedDescriptors = 0;

	BDBG_ENTER(BDSP_Raaga_P_ReleaseDescriptor);
	pDevice = (BDSP_Raaga *)pDeviceHandle;
	BKNI_AcquireMutex(pDevice->deviceMutex);

	BDBG_MSG(("BDSP_Raaga_P_ReleaseDescriptor: Number of descriptor = %d",numDescriptors));
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	while(releasedDescriptors != numDescriptors)
	{
		BDBG_MSG(("BDSP_Raaga_P_ReleaseDescriptor: Descriptor ="BDSP_MSG_FMT,BDSP_MSG_ARG(*pOffset)));
		if((*pOffset >= pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset)&&
		(*pOffset <=(pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset + pDevice->memInfo.DescriptorMemory[dspIndex].ui32Size)))
		{
			index = (((*pOffset) - pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset)/sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
			BDBG_MSG(("BDSP_Raaga_P_ReleaseDescriptor: index = %d", index));
			pDevice->hardwareStatus.descriptor[dspIndex][index]=false;
			pOffset++;
			releasedDescriptors++;
		}
		else
		{
			BDBG_ERR(("BDSP_Raaga_P_ReleaseDescriptor: Trying to release a descriptor("BDSP_MSG_FMT") not in the block ",BDSP_MSG_ARG(*pOffset)));
			errCode = BERR_INVALID_PARAMETER;
			break;
		}
	}
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseDescriptor);
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetMemoryEstimate(
	const BDSP_RaagaSettings     *pSettings,
	const BDSP_RaagaUsageOptions *pUsage,
	BBOX_Handle                   boxHandle,
	BDSP_RaagaMemoryEstimate     *pEstimate /*[out]*/
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned NumDsp = BDSP_RAAGA_MAX_DSP, NumCores = BDSP_RAAGA_MAX_CORE_PER_DSP;
	unsigned MemoryRequired = 0, index = 0, NumStages = 0;
	BDBG_ENTER(BDSP_Raaga_P_GetMemoryEstimate);

	/* Initialise the values */
	pEstimate->FirmwareMemory = 0;
	pEstimate->GeneralMemory  = 0;

	if(NULL != boxHandle)
	{
		BDSP_Raaga_P_GetNumberOfDspandCores(boxHandle, &NumDsp, &NumCores);
	}
	else
	{
		NumDsp   = pSettings->NumDsp;
		NumCores = pSettings->numCorePerDsp;
	}
	BDBG_MSG(("Num DSP = %d, Num Cores = %d",NumDsp, NumCores));

	/* Calculate Code Memory*/
	{
		BDSP_Raaga_P_CodeDownloadInfo *pCodeInfo;
		unsigned ResidentCode = 0, LoadableCode = 0;

		pCodeInfo = (BDSP_Raaga_P_CodeDownloadInfo *)BKNI_Malloc(sizeof(BDSP_Raaga_P_CodeDownloadInfo));
		if(pCodeInfo == NULL)
		{
			BDBG_ERR(("BDSP_Raaga_P_GetMemoryEstimate: Unable to Allocate memory for sizing of Code"));
			errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto end;

		}
		BDSP_Raaga_P_AssignAlgoSize_APITool(pUsage, &pCodeInfo->imgInfo[0]);

		BDSP_Raaga_P_ComputeResidentSection(pCodeInfo, &ResidentCode);
		pEstimate->FirmwareMemory += ResidentCode;
		BDBG_MSG(("Resident code = %d (%d KB) (%d MB)",ResidentCode, (ResidentCode/1024), (ResidentCode/(1024*1024))));

		BDSP_Raaga_P_ComputeLoadbleSection_APITool(pSettings, pUsage, pCodeInfo, &LoadableCode);
		pEstimate->FirmwareMemory += LoadableCode;
		BDBG_MSG(("Loadble code  = %d (%d KB) (%d MB)",LoadableCode, (LoadableCode/1024), (LoadableCode/(1024*1024))));

		pEstimate->FirmwareMemory += 1024;
		pEstimate->FirmwareMemory = BDSP_RAAGA_ALIGN_SIZE(pEstimate->FirmwareMemory,4096);
		BDBG_MSG(("Total RO Memory = %d (%d KB) (%d MB)", pEstimate->FirmwareMemory, (pEstimate->FirmwareMemory/1024), (pEstimate->FirmwareMemory/(1024*1024))));
		BKNI_Free(pCodeInfo);
	}

	/*Calculate General Heap Memory */
	{
		/*RW Section*/
		for(index=0; index<NumDsp; index++)
		{
			BDSP_Raaga_P_CalculateInitMemory(&MemoryRequired);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("INIT Memory  = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			BDSP_Raaga_P_CalculateDebugMemory((BDSP_RaagaSettings *)pSettings, &MemoryRequired);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("Debug Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			BDSP_Raaga_P_CalculateProcessRWMemory(&MemoryRequired);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("Process RW Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			BDSP_Raaga_P_CalculateScratchAndInterStageMemory(NULL, NumCores, &MemoryRequired, true, pUsage);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("Work buffer Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			BDSP_Raaga_P_CalculateDescriptorMemory(&MemoryRequired);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("Descriptor Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			pEstimate->GeneralMemory = BDSP_RAAGA_ALIGN_SIZE(pEstimate->GeneralMemory, 4096);/* Align the size to 4k */
		}
		BDBG_MSG(("Total RW Memory(%d DSP) = %d (%d KB) (%d MB)",NumDsp, pEstimate->GeneralMemory, (pEstimate->GeneralMemory/1024), (pEstimate->GeneralMemory/(1024*1024))));

		/* Task Memory */
		BDSP_Raaga_P_CalculateTaskMemory(&MemoryRequired);
		pEstimate->GeneralMemory += (MemoryRequired*BDSP_RAAGA_MAX_FW_TASK_PER_DSP);
		BDBG_MSG(("Per Task Memory(%d Tasks) = %d (%d KB) (%d MB)",BDSP_RAAGA_MAX_FW_TASK_PER_DSP,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

		/* Stage Memory*/
		BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, true, (BDSP_RaagaUsageOptions *)pUsage);
		NumStages = pUsage->NumAudioDecoders + pUsage->NumAudioPostProcesses +
					pUsage->NumAudioEncoders + pUsage->NumAudioMixers +
					pUsage->NumAudioPassthru + pUsage->NumAudioEchocancellers +
					pUsage->NumVideoDecoders + pUsage->NumVideoEncoders;

		pEstimate->GeneralMemory += MemoryRequired*NumStages;
		BDBG_MSG(("Per Stage Memory(%d Stages) = %d (%d KB) (%d MB)",NumStages,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
		/* TO DO - InterTask Buffer Memory Accounting*/
	}
	BDBG_MSG(("Memory Required FIRMWARE = %d bytes(%d KB)  GENERAL = %d bytes(%d KB)",pEstimate->FirmwareMemory,(pEstimate->FirmwareMemory/1024),
					pEstimate->GeneralMemory,(pEstimate->GeneralMemory/1024)));
end:
	BDBG_LEAVE(BDSP_Raaga_P_GetMemoryEstimate);
	return errCode;
}
