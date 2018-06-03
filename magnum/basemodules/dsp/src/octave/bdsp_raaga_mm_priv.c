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

BDBG_MODULE(bdsp_raaga_mm);

void BDSP_Raaga_P_CalculateInitMemory(
    unsigned *pMemReqd
)
{
    BDBG_ENTER(BDSP_Raaga_P_CalculateInitMemory);
    *pMemReqd = 0;

    /*Memory for Command Queue, accounting for maximun commands per task(10) for max (12) tasks in a single DSP*/
    *pMemReqd += (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Command)*BDSP_MAX_FW_TASK_PER_DSP);
    /* Memory for Generic Response Queue, Maximum of (10) responses from the DSP */
    *pMemReqd += (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));

	BDBG_MSG(("INIT Memory	= %d (%d KB) (%d MB)",*pMemReqd, (*pMemReqd/1024), (*pMemReqd/(1024*1024))));
    BDBG_LEAVE(BDSP_Raaga_P_CalculateInitMemory);
}

void BDSP_Raaga_P_CalculateDebugMemory(
    const BDSP_RaagaSettings *pSettings,
    unsigned           *pMemReqd
)
{
    unsigned index= 0;
    unsigned MemoryRequired = 0;
    BDBG_ENTER(BDSP_Raaga_P_CalculateDebugMemory);

    for (index = 0; index < BDSP_DebugType_eLast; index++)
    {
        if(pSettings->debugSettings[index].enabled)
        {
            MemoryRequired += pSettings->debugSettings[index].bufferSize;
        }
        else
        {
            MemoryRequired += BDSP_MIN_DEBUG_BUFFER_SIZE;
        }

        if(BDSP_DebugType_eTargetPrintf == index)
        {
            /* For Shared Target print buffer to bet set in kernel
             * In case of Target Print We need 2 buffers, one for application and one to set in firmware
             * This will be removed when there is new API support in fp_sdk to read directly from Target print buffer */
            if(pSettings->debugSettings[index].enabled)
            {
                MemoryRequired += pSettings->debugSettings[index].bufferSize;
            }
            else
            {
                /* Make sure you are not allocating any memory. If you add any Memory here we should assign equal memory as well.
                 * Otherwise, Kernel memory alignment will change  */
                MemoryRequired +=  BDSP_MIN_DEBUG_BUFFER_SIZE;
            }
        }
    }

    /* Memory used by interactive debug service process to send and receive gdb packets */
    MemoryRequired += PM_HOST_DEBUG_BUFFER_SIZE; /* debug channel memory for process manager */
    *pMemReqd = MemoryRequired;
	BDBG_MSG(("Debug Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
    BDBG_LEAVE(BDSP_Raaga_P_CalculateDebugMemory);
}

static void BDSP_Raaga_P_CalculateProcessRWMemory(
	unsigned *pMemReqd
)
{
    unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateProcessRWMemory);
    *pMemReqd = 0;

    MemoryRequired += (BDSP_IMG_USER_PROCESS_SPAWN_MEM_SIZE*BDSP_MAX_NUM_USER_PROCESS);
    MemoryRequired += BDSP_IMG_DEFAULT_MM_PROC_HEAP_SIZE;
    MemoryRequired += BDSP_IMG_INIT_PROCESS_MEM_SIZE;
    MemoryRequired += BDSP_IMG_TOPLEVEL_PROCESS_MEM_SIZE;

    *pMemReqd = MemoryRequired;
	BDBG_MSG(("Process RW Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

    BDBG_LEAVE(BDSP_Raaga_P_CalculateProcessRWMemory);
}

static void BDSP_Raaga_P_CalculateScratchAndInterStageMemory(
    BDSP_Raaga *pDevice,
    unsigned    dspIndex,
    unsigned    numCorePerDsp,
    unsigned   *pMemReqd,
    bool        ifMemApiTool,
	const BDSP_UsageOptions *pUsage
)
{
    unsigned MemoryRequired = 0;
	unsigned preemptionLevel = 0;
	BDSP_Algorithm algorithm;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	const BDSP_P_AlgorithmCodeInfo *pAlgoCodeInfo;
	bool supported = false;

    BDBG_ENTER(BDSP_Raaga_P_CalculateScratchAndInterStageMemory);
    *pMemReqd = 0;

	for(preemptionLevel=0; preemptionLevel<BDSP_MAX_NUM_SCHED_LEVELS; preemptionLevel++)
	{
		unsigned interStageBufferReqd=0, scratchBufferReqd = 0;
		for(algorithm=0; algorithm<BDSP_Algorithm_eMax; algorithm++)
		{
			pAlgoInfo = BDSP_P_LookupAlgorithmInfo(algorithm);
            pAlgoCodeInfo = BDSP_Raaga_P_LookupAlgorithmCodeInfo(algorithm);

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
				if(scratchBufferReqd < pAlgoCodeInfo->scratchBufferSize)
					scratchBufferReqd = pAlgoCodeInfo->scratchBufferSize;
				if(interStageBufferReqd < ((pAlgoInfo->maxChannelsSupported*pAlgoInfo->samplesPerChannel*4)+INTERSTAGE_EXTRA_SIZE_ALIGNED))
					interStageBufferReqd = ((pAlgoInfo->maxChannelsSupported*pAlgoInfo->samplesPerChannel*4)+INTERSTAGE_EXTRA_SIZE_ALIGNED);
			}
		}
		scratchBufferReqd = BDSP_ALIGN_SIZE(scratchBufferReqd,4096);
		interStageBufferReqd = BDSP_ALIGN_SIZE(interStageBufferReqd,4096);
		MemoryRequired += (scratchBufferReqd+(interStageBufferReqd*(BDSP_MAX_BRANCH+1)));
		if(!ifMemApiTool)
		{
			/* Normal Path */
			pDevice->memInfo.WorkBufferMemory[dspIndex][preemptionLevel].ui32Size =
			              (scratchBufferReqd+(interStageBufferReqd*(BDSP_MAX_BRANCH+1)));
		}
		BDBG_MSG(("Work Buffer Allocation for Preemption Level %d",preemptionLevel));
		BDBG_MSG(("Scratch = %d InterStage = %d Total(Scratch + %d Interstage) = %d",scratchBufferReqd,interStageBufferReqd,
			(BDSP_MAX_BRANCH+1),(scratchBufferReqd+(interStageBufferReqd*(BDSP_MAX_BRANCH+1)))));
	}

    MemoryRequired = (MemoryRequired*numCorePerDsp);
	*pMemReqd = MemoryRequired;
	BDBG_MSG(("Work buffer Memory for(%d) cores = %d (%d KB) (%d MB)",numCorePerDsp, MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
    BDBG_LEAVE(BDSP_Raaga_P_CalculateScratchAndInterStageMemory);
}

void BDSP_Raaga_P_CalculateStageMemory(
	unsigned    *pMemReqd,
	BDSP_AlgorithmType  algoType,
    bool        ifMemApiTool,
	const BDSP_UsageOptions *pUsage
)
{
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
	const BDSP_P_AlgorithmInfo        *pAlgoInfo;
	const BDSP_P_AlgorithmCodeInfo    *pAlgoCodeInfo;
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
			pAlgoInfo = BDSP_P_LookupAlgorithmInfo(i);
            pAlgoCodeInfo = BDSP_Raaga_P_LookupAlgorithmCodeInfo(i);
            if(pAlgoInfo->type == algoType)
            {
                tempSize += pAlgoCodeInfo->interFrameSize;
                tempSize += (pAlgoInfo->algoUserConfigSize*2); /* Host copy  and DSP copy*/
                tempSize += pAlgoInfo->algoStatusBufferSize;
                tempSize += pAlgoInfo->idsStatusBufferSize;
                tempSize += pAlgoInfo->tsmStatusBufferSize;
                tempSize += sizeof(BDSP_AudioTaskDatasyncSettings);
                tempSize += sizeof(BDSP_AudioTaskTsmSettings);
                tempSize += BDSP_MAX_HOST_DSP_L2C_SIZE; /* Hole memory to beat Cache coherency*/
            }
			if(tempSize > MemoryRequired)
				MemoryRequired = tempSize;
		}
    }

	MemoryRequired = BDSP_ALIGN_SIZE(MemoryRequired, 4096);  /* Align the size to 4k */
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
	MemoryRequired += (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));
	MemoryRequired += (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_CONFIG); /* CIT Memory */
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_CONFIG); /* CIT Re-Config Memory */
	MemoryRequired += sizeof(BDSP_AF_P_sOpSamplingFreq); /*Sample Rate LUT*/
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG);
	MemoryRequired += sizeof(BDSP_AF_P_sTASK_GATEOPEN_CONFIG);
    MemoryRequired += sizeof(BDSP_AF_P_sSTC_TRIGGER_CONFIG);
	MemoryRequired += BDSP_MAX_HOST_DSP_L2C_SIZE;/* Hole memory to beat Cache coherency*/

	MemoryRequired = BDSP_ALIGN_SIZE(MemoryRequired, 4096);  /* Align the size to 4k */
	*pMemReqd = MemoryRequired;

	BDBG_MSG(("BDSP_Raaga_P_CalculateTaskMemory: Memory Allocated for Task = %d", MemoryRequired));
	BDBG_LEAVE(BDSP_Raaga_P_CalculateTaskMemory);
}

void BDSP_Raaga_P_CalculateKernelRWMemory(
    unsigned   *pMemReqd
)
{
	*pMemReqd = BDSP_IMG_KERNEL_RW_IMG_SIZE;
	*pMemReqd = BDSP_ALIGN_SIZE(*pMemReqd, 4096);/* Align the size to 4k */

	BDBG_MSG(("Process RW Memory = %d (%d KB) (%d MB)",*pMemReqd, (*pMemReqd/1024), (*pMemReqd/(1024*1024))));
}

void BDSP_Raaga_P_CalculateHostFWsharedRWMemory(
    const BDSP_RaagaSettings *pdeviceSettings,
    unsigned    dspIndex,
    unsigned   *pMemReqd
)
{
    unsigned MemoryRequired = 0;
    BDSP_Raaga_P_CalculateInitMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

	*pMemReqd += BDSP_MAX_HOST_DSP_L2C_SIZE; /* Hole memory to beat Cache coherency between Init and Target Print Memory*/
    BDSP_P_CalculateDescriptorMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

	*pMemReqd += BDSP_MAX_HOST_DSP_L2C_SIZE; /* Hole memory to beat Cache coherency between Init and Descriptor Memory*/
    BDSP_Raaga_P_CalculateDebugMemory(pdeviceSettings, &MemoryRequired);
    *pMemReqd += MemoryRequired;

    *pMemReqd = BDSP_ALIGN_SIZE(*pMemReqd, 4096);/* Align the size to 4k */
	BDBG_MSG(("HOST-FIRMWARE SHARED Memory = %d (%d KB) (%d MB)",*pMemReqd, (*pMemReqd/1024), (*pMemReqd/(1024*1024))));
	BSTD_UNUSED(dspIndex);
}


void BDSP_Raaga_P_CalculateDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned    dspIndex,
    unsigned   *pMemReqd
)
{
    unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Raaga_P_CalculateDeviceRWMemory);

    BDSP_Raaga_P_CalculateScratchAndInterStageMemory(pDevice, dspIndex, pDevice->numCorePerDsp, &MemoryRequired, false, NULL);
    *pMemReqd += MemoryRequired;

    BDSP_Raaga_P_CalculateProcessRWMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

    *pMemReqd = BDSP_ALIGN_SIZE(*pMemReqd, 4096);/* Align the size to 4k */

	BDBG_MSG(("Device Only RW Memory = %d (%d KB) (%d MB)",*pMemReqd, (*pMemReqd/1024), (*pMemReqd/(1024*1024))));
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
	*pMemReqd = BDSP_ALIGN_SIZE(*pMemReqd,4096);

	BDBG_MSG(("BDSP_Raaga_P_CalculateDeviceROMemory: Memory Allocated for Device RO = %d", *pMemReqd));
	BDBG_LEAVE(BDSP_Raaga_P_CalculateDeviceROMemory);
}

static BERR_Code BDSP_Raaga_P_AssignKernalMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Raaga_P_AssignKernalMemory);

    errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sKernelRWMemoryPool[dspindex], BDSP_IMG_KERNEL_RW_IMG_SIZE, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignKernalMemory: Unable to Assign Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.KernalMemory[dspindex].Buffer   = Memory;
    pDevice->memInfo.KernalMemory[dspindex].ui32Size = BDSP_IMG_KERNEL_RW_IMG_SIZE;

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignKernalMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseKernalMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_ReleaseKernalMemory);

    errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sKernelRWMemoryPool[dspindex],
		pDevice->memInfo.KernalMemory[dspindex].ui32Size,
		&pDevice->memInfo.KernalMemory[dspindex].Buffer);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_DeAssignKernalMemory: Unable to DeAssign Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.KernalMemory[dspindex].ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseKernalMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_AssignTargetBufferMemory(
    BDSP_Raaga *pDevice,
    unsigned dspIndex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Raaga_P_AssignTargetBufferMemory);

    /* For Shared Target print buffer to bet set in kernel */
    if(pDevice->deviceSettings.debugSettings[BDSP_DebugType_eTargetPrintf].enabled)
    {
        errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspIndex], pDevice->deviceSettings.debugSettings[BDSP_DebugType_eTargetPrintf].bufferSize, &Memory);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_AssignTargetBufferMemory: Unable to Assign Target buffer Memory for dsp %d!!!!",dspIndex));
            goto end;
        }

        pDevice->memInfo.TargetBufferMemory[dspIndex].Buffer   = Memory;
        pDevice->memInfo.TargetBufferMemory[dspIndex].ui32Size = pDevice->deviceSettings.debugSettings[BDSP_DebugType_eTargetPrintf].bufferSize;
    }
    else
    {
        BKNI_Memset((void *) &pDevice->memInfo.TargetBufferMemory[dspIndex].Buffer.hBlock, 0, sizeof(BDSP_P_FwBuffer));
    }

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignTargetBufferMemory);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseTargetBufferMemory(
    BDSP_Raaga *pDevice,
    unsigned dspIndex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Raaga_P_ReleaseTargetBufferMemory);

    if(pDevice->deviceSettings.debugSettings[BDSP_DebugType_eTargetPrintf].enabled)
    {
        errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspIndex],
		    pDevice->memInfo.TargetBufferMemory[dspIndex].ui32Size,
		    &pDevice->memInfo.TargetBufferMemory[dspIndex].Buffer);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_ReleaseTargetBufferMemory: Unable to release target buffer Memory for dsp %d!!!!",dspIndex));
            goto end;
        }
    }
    pDevice->memInfo.TargetBufferMemory[dspIndex].ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseTargetBufferMemory);
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

    ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
    errCode  = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to assign Cache Hole of INIT Memory for DSP %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.CacheHole1[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.CacheHole1[dspindex].Buffer   = Memory;

    errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice,dspindex,&(pDevice->memInfo.cmdQueueParams[dspindex].ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to find free fifo for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    ui32Size = BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Command)*BDSP_MAX_FW_TASK_PER_DSP;
    errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to allocate RW memory for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.cmdQueueParams[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.cmdQueueParams[dspindex].Memory   = Memory;

    errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice,dspindex,&(pDevice->memInfo.genRspQueueParams[dspindex].ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to find free fifo for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    ui32Size = (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));
    errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
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
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release fifo for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex],
		pDevice->memInfo.cmdQueueParams[dspindex].ui32Size,
		&pDevice->memInfo.cmdQueueParams[dspindex].Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release RW memory for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
	pDevice->memInfo.cmdQueueParams[dspindex].ui32Size 	= 0;

    errCode = BDSP_Raaga_P_ReleaseFIFO(pDevice,dspindex,&(pDevice->memInfo.genRspQueueParams[dspindex].ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release fifo for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }

	  errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex],
		pDevice->memInfo.genRspQueueParams[dspindex].ui32Size,
		&pDevice->memInfo.genRspQueueParams[dspindex].Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseInitMemory: Unable to release RW memory for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
		goto end;
	}
	pDevice->memInfo.genRspQueueParams[dspindex].ui32Size 	= 0;

	errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex],
		pDevice->memInfo.CacheHole1[dspindex].ui32Size,
		&pDevice->memInfo.CacheHole1[dspindex].Buffer);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseDescriptorMemory: Unable to Release Cache hole for Init Memory for dsp %d!!!!",dspindex));
		goto end;
	}
	pDevice->memInfo.CacheHole1[dspindex].ui32Size = 0;

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
	uint32_t index=0;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Raaga_P_AssignDescriptorMemory);

    ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
    errCode  = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignInitMemory: Unable to assign Cache Hole Memory of Descriptor Memory for DSP %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.CacheHole2[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.CacheHole2[dspindex].Buffer   = Memory;

	for(index=0;index<BDSP_MAX_POOL_OF_DESCRIPTORS;index++)
	{
	    ui32Size= BDSP_ALIGN_SIZE((BDSP_MAX_DESCRIPTORS_PER_POOL*sizeof(BDSP_AF_P_sCIRCULAR_BUFFER)),BDSP_MAX_HOST_DSP_L2C_SIZE);
	    errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex], ui32Size, &Memory);
	    if(errCode != BERR_SUCCESS)
	    {
	        BDBG_ERR(("BDSP_Raaga_P_AssignDescriptorMemory: Unable to allocate RW memory for Descriptors for pool (%d) for dsp %d!!!!",index,dspindex));
	        goto end;
	    }
	    pDevice->memInfo.DescriptorMemory[dspindex][index].ui32Size = ui32Size;
	    pDevice->memInfo.DescriptorMemory[dspindex][index].Buffer   = Memory;
	}

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
	uint32_t index =0;
    BDBG_ENTER(BDSP_Raaga_P_ReleaseDescriptorMemory);

	for(index=0;index<BDSP_MAX_POOL_OF_DESCRIPTORS;index++)
	{
	    errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex],
			pDevice->memInfo.DescriptorMemory[dspindex][index].ui32Size,
			&pDevice->memInfo.DescriptorMemory[dspindex][index].Buffer);
	    if(errCode != BERR_SUCCESS)
	    {
	        BDBG_ERR(("BDSP_Raaga_P_ReleaseDescriptorMemory: Unable to Release RW memory for Descriptors for pool(%d) for dsp %d!!!!",index, dspindex));
	        goto end;
	    }
		pDevice->memInfo.DescriptorMemory[dspindex][index].ui32Size = 0;
	}

	errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex],
		pDevice->memInfo.CacheHole2[dspindex].ui32Size,
		&pDevice->memInfo.CacheHole2[dspindex].Buffer);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseDescriptorMemory: Unable to Release Cache hole for Descriptor Memory for dsp %d!!!!",dspindex));
		goto end;
	}
	pDevice->memInfo.CacheHole2[dspindex].ui32Size = 0;

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
	BDSP_MMA_Memory Memory = {0};
	BDBG_ENTER(BDSP_Raaga_P_AssignDebugMemory);

	for(index = 0; index < BDSP_DebugType_eLast;index++)
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
			case BDSP_DebugType_eDramMsg:
				pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_RAAGA_DRAM_FIFO;
				break;
			case BDSP_DebugType_eUart:
				pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_RAAGA_UART_FIFO;
				break;
			case BDSP_DebugType_eCoreDump:
				pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_RAAGA_CORE_FIFO;
				break;
			case BDSP_DebugType_eTargetPrintf:
#ifdef FIREPATH_BM
				pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId =
									(BCHP_PHYSICAL_OFFSET | BCHP_RAAGA_DSP_FP_MISC_0_CORESTATE_SYS_MBX0);
#else
				pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BCHP_RAAGA_DSP_FP_MISC_0_CORESTATE_SYS_MBX0;
#endif /*FIREPATH_BM*/
		}
		if(pDevice->deviceSettings.debugSettings[index].enabled)
		{
			ui32Size = pDevice->deviceSettings.debugSettings[index].bufferSize;
			errCode  = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex], ui32Size, &Memory);
            if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_AssignDebugMemory: Unable to allocate RW memory for Debug type %d for dsp %d!!!!",index, dspindex));
				goto end;
			}
		}
		else
		{
			ui32Size = BDSP_MIN_DEBUG_BUFFER_SIZE;
		}
		pDevice->memInfo.debugQueueParams[dspindex][index].ui32Size = ui32Size;
		pDevice->memInfo.debugQueueParams[dspindex][index].Memory   = Memory;
	}

	/* For Shared Debug service memory used by Debug process for Interactive Deubg Service.
	 * This memory is always required for the debug service to run */
	errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex], PM_HOST_DEBUG_BUFFER_SIZE, &Memory);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignDeubgSerivceMemory: Unable to Assign Debug service Memory for dsp %d!!!!",dspindex));
		goto end;
	}
	pDevice->memInfo.DeubgServiceMemory[dspindex].Buffer   = Memory;
	pDevice->memInfo.DeubgServiceMemory[dspindex].ui32Size = PM_HOST_DEBUG_BUFFER_SIZE;

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

	for(index = 0; index < BDSP_DebugType_eLast;index++)
	{
		if(pDevice->deviceSettings.debugSettings[index].enabled)
		{
			errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex],
				pDevice->memInfo.debugQueueParams[dspindex][index].ui32Size,
				&pDevice->memInfo.debugQueueParams[dspindex][index].Memory);
            if(errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BDSP_Raaga_P_ReleaseDebugMemory: Unable to release RW memory for Debug type %d for dsp %d!!!!",index,dspindex));
				goto end;
			}
			pDevice->memInfo.debugQueueParams[dspindex][index].ui32Size	= 0;
			pDevice->memInfo.debugQueueParams[dspindex][index].ui32FifoId = BDSP_FIFO_INVALID;
		}
	}


	errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sHostSharedRWMemoryPool[dspindex],
			pDevice->memInfo.DeubgServiceMemory[dspindex].ui32Size,
			&pDevice->memInfo.DeubgServiceMemory[dspindex].Buffer);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseDebugServiceMemory: Unable to release debug service Memory for dsp %d!!!!",dspindex));
		goto end;
	}

end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseDebugMemory);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_AssignWorkBufferMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ui32Size = 0;
    BDSP_MMA_Memory Memory;
	unsigned preemptionLevel = 0;
    BDBG_ENTER(BDSP_Raaga_P_AssignWorkBufferMemory);

    for(preemptionLevel=0; preemptionLevel<BDSP_MAX_NUM_SCHED_LEVELS; preemptionLevel++)
    {
        ui32Size = pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size;
        errCode  = BDSP_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_AssignWorkBufferMemory: Unable to allocate RW memory for dsp %d with Preemption level =%d !!!!",dspindex, preemptionLevel));
            goto end;
        }
        pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].Buffer  = Memory;
		pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size= ui32Size;
    }

end:
	BDBG_LEAVE(BDSP_Raaga_P_AssignWorkBufferMemory);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseWorkBufferMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
	unsigned preemptionLevel = 0;
    BDBG_ENTER(BDSP_Raaga_P_ReleaseWorkBufferMemory);

    for(preemptionLevel=0; preemptionLevel<BDSP_MAX_NUM_SCHED_LEVELS; preemptionLevel++)
    {
        errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size,
		&pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].Buffer);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_ReleaseWorkBufferMemory: Unable to Release RW memory for dsp %d with Preemption level =%d !!!!",dspindex, preemptionLevel));
            goto end;
        }
	pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size = 0;
    }
end:
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseWorkBufferMemory);
	return errCode;
}

BERR_Code BDSP_Raaga_P_AssignDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Raaga_P_AssignDeviceRWMemory);

    errCode = BDSP_Raaga_P_AssignKernalMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Shared Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    /* Make sure to assign Target buffer Memory right after Kernel Memory.
     * As the Target Buffer start addr ( Firmware virtual address) starts right after Kernel RW Memory
     * This requirement will go, once redesign happens to handle Secure and non secure regions */
    errCode = BDSP_Raaga_P_AssignTargetBufferMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Shared Target Buffer Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_AssignInitMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Init Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_AssignDescriptorMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
      BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Descriptor Memory for dsp %d!!!!",dspindex));
      goto end;
    }

	errCode = BDSP_Raaga_P_AssignDebugMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Debug Memory for dsp %d!!!!",dspindex));
		goto end;
	}

    errCode = BDSP_Raaga_P_AssignWorkBufferMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignDeviceRWMemory: Unable to Assign Work Buffer Memory for dsp %d!!!!",dspindex));
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

    errCode = BDSP_Raaga_P_ReleaseKernalMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Kernal Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_ReleaseTargetBufferMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release target buffer Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_ReleaseDebugMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Debug Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Raaga_P_ReleaseInitMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Init Memory for dsp %d!!!!",dspindex));
        goto end;
    }

	errCode = BDSP_Raaga_P_ReleaseDescriptorMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Descriptor Memory for dsp %d!!!!",dspindex));
		goto end;
	}

    errCode = BDSP_Raaga_P_ReleaseWorkBufferMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseDeviceRWMemory: Unable to Release Workbuffer Memory for dsp %d!!!!",dspindex));
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
	pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr = (void *)BKNI_Malloc(BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
	if(NULL == pRaagaTask->taskMemInfo.hostAsyncQueue.pAddr)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to allocate BKNI Memory for Async buffer copy of HOST for Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.hostAsyncQueue.ui32Size = (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));

    ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for CIT Buffer of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCITMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sCITMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for CIT Reconfig Buffer of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sHostCITMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sHostCITMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sOpSamplingFreq);
	errCode  = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for Sample Rate LUT of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSampleRateLUTMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sSampleRateLUTMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for Scheduling Config of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSchedulingConfigMemory.Buffer  = Memory;
	pRaagaTask->taskMemInfo.sSchedulingConfigMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_GATEOPEN_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for Gate Open Config of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sGateOpenConfigMemory.Buffer	= Memory;
	pRaagaTask->taskMemInfo.sGateOpenConfigMemory.ui32Size  = ui32Size;


	ui32Size = sizeof(BDSP_AF_P_sSTC_TRIGGER_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for STC Trigger Config of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.Buffer	  = Memory;
	pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.ui32Size  = ui32Size;

    ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
    errCode  = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign Cache Hole Memory of Task %p",(void *)pRaagaTask));
        goto end;
    }
    pRaagaTask->taskMemInfo.sCacheHole.ui32Size = ui32Size;
    pRaagaTask->taskMemInfo.sCacheHole.Buffer   = Memory;

    errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice, dspIndex,&(pRaagaTask->taskMemInfo.syncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to find free fifo for SYNC QUEUE of TASK %p",(void *)pRaagaTask));
        goto end;
    }
    ui32Size = (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));
    errCode = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for SYNC QUEUE of Task %p",(void *)pRaagaTask));
        goto end;
    }
    pRaagaTask->taskMemInfo.syncQueueParams.ui32Size = ui32Size;
    pRaagaTask->taskMemInfo.syncQueueParams.Memory   = Memory;

	errCode = BDSP_Raaga_P_AssignFreeFIFO(pDevice, dspIndex,&(pRaagaTask->taskMemInfo.asyncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to find free fifo for ASYNC QUEUE of TASK %p",(void *)pRaagaTask));
		goto end;
	}
	ui32Size = (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
	errCode = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for ASYNC QUEUE of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.asyncQueueParams.ui32Size = ui32Size;
	pRaagaTask->taskMemInfo.asyncQueueParams.Memory	  = Memory;

	ui32Size = BDSP_IMG_MP_AP_SHARED_MEMORY_SIZE;
	errCode = BDSP_P_RequestMemory(&pRaagaTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignTaskMemory: Unable to assign RW memory for MP Shared Memory of Task %p",(void *)pRaagaTask));
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

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sCITMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sCITMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release CIT Memory of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCITMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sHostCITMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sHostCITMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release CIT ReConfig Memory of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sHostCITMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sSampleRateLUTMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sSampleRateLUTMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Sample Rate LUT of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSampleRateLUTMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sSchedulingConfigMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sSchedulingConfigMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Scheduling Config of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sSchedulingConfigMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sGateOpenConfigMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sGateOpenConfigMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Gate Open Config of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sGateOpenConfigMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Stc Trigger Config of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sStcTriggerConfigMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sCacheHole.ui32Size,
		&pRaagaTask->taskMemInfo.sCacheHole.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release Cache Hole Memory of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.sCacheHole.ui32Size = 0;

    errCode = BDSP_Raaga_P_ReleaseFIFO(pDevice,dspIndex,&(pRaagaTask->taskMemInfo.syncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release fifo for SYNC QUEUE of Task %p",(void *)pRaagaTask));
        goto end;
    }
    errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.syncQueueParams.ui32Size,
		&pRaagaTask->taskMemInfo.syncQueueParams.Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release RW memory for SYNC QUEUE of Task %p",(void *)pRaagaTask));
        goto end;
    }
	pRaagaTask->taskMemInfo.syncQueueParams.ui32Size = 0;

	errCode = BDSP_Raaga_P_ReleaseFIFO(pDevice,dspIndex,&(pRaagaTask->taskMemInfo.asyncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release fifo for ASYNC QUEUE of Task %p",(void *)pRaagaTask));
		goto end;
	}
	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.asyncQueueParams.ui32Size,
		&pRaagaTask->taskMemInfo.asyncQueueParams.Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release RW memory for ASYNC QUEUE of Task %p",(void *)pRaagaTask));
		goto end;
	}
	pRaagaTask->taskMemInfo.asyncQueueParams.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaTask->taskMemInfo.sMemoryPool,
		pRaagaTask->taskMemInfo.sMPSharedMemory.ui32Size,
		&pRaagaTask->taskMemInfo.sMPSharedMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseTaskMemory: Unable to release RW memory for MP Shared of Task %p",(void *)pRaagaTask));
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
	const BDSP_P_AlgorithmInfo     *pAlgoInfo;
	const BDSP_P_AlgorithmCodeInfo *pAlgoCodeInfo;
    uint32_t ui32Size =0;
	BDSP_MMA_Memory Memory;

	BDBG_ENTER(BDSP_Raaga_P_AssignStageMemory);
	BDBG_OBJECT_ASSERT(pRaagaStage, BDSP_RaagaStage);

	pAlgoInfo = BDSP_P_LookupAlgorithmInfo(pRaagaStage->eAlgorithm);
    pAlgoCodeInfo = BDSP_Raaga_P_LookupAlgorithmCodeInfo(pRaagaStage->eAlgorithm);

	ui32Size = pAlgoCodeInfo->interFrameSize;
	errCode = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Interframe memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pRaagaStage->stageMemInfo.sInterframe.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sInterframe.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoUserConfigSize;
	errCode  = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign UserConfig memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoStatusBufferSize;
	errCode = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Status memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sAlgoStatus.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->idsStatusBufferSize;
	errCode = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign IDS Status memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sIdsStatus.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sIdsStatus.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->tsmStatusBufferSize;
	errCode = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign TSM Status memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sTsmStatus.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sTsmStatus.ui32Size= ui32Size;

	/* Allocation of 512 bytes of hole memory to beat Cache Coherancy*/
	ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
	errCode  = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Cache Hole memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sCacheHole.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sCacheHole.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AudioTaskDatasyncSettings);
	errCode  = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign DataSync Config memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pRaagaStage->stageMemInfo.sDataSyncSettings.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sDataSyncSettings.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AudioTaskTsmSettings);
	errCode  = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign TSM Config memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pRaagaStage->stageMemInfo.sTsmSettings.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sTsmSettings.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoUserConfigSize;
	errCode = BDSP_P_RequestMemory(&pRaagaStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_AssignStageMemory: Unable to assign Host UserConfig memory for Stage(%d) %s",pRaagaStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer  = Memory;
	pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size= ui32Size;

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

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size,
		&pRaagaStage->stageMemInfo.sHostAlgoUserConfig.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release Host UserConfig memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sHostAlgoUserConfig.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sTsmSettings.ui32Size,
		&pRaagaStage->stageMemInfo.sTsmSettings.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release TSM Settings memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sTsmSettings.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sDataSyncSettings.ui32Size,
		&pRaagaStage->stageMemInfo.sDataSyncSettings.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release IDS Settings memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sDataSyncSettings.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sCacheHole.ui32Size,
		&pRaagaStage->stageMemInfo.sCacheHole.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release Cache Hole memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sCacheHole.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sTsmStatus.ui32Size,
		&pRaagaStage->stageMemInfo.sTsmStatus.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release TSM Status memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sTsmStatus.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sIdsStatus.ui32Size,
		&pRaagaStage->stageMemInfo.sIdsStatus.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release IDS Status memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sIdsStatus.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size,
		&pRaagaStage->stageMemInfo.sAlgoStatus.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release Algo Status memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sAlgoStatus.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size,
		&pRaagaStage->stageMemInfo.sAlgoUserConfig.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseStageMemory: Unable to release UserConfig memory for Stage(%d)",pRaagaStage->eAlgorithm));
		goto end;
	}
	pRaagaStage->stageMemInfo.sAlgoUserConfig.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pRaagaStage->stageMemInfo.sMemoryPool,
		pRaagaStage->stageMemInfo.sInterframe.ui32Size,
		&pRaagaStage->stageMemInfo.sInterframe.Buffer);
    if(errCode != BERR_SUCCESS)
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
	while(allocatedDescriptors < numDescriptors)
	{
		for(;index<BDSP_MAX_POOL_OF_DESCRIPTORS; index++)
		{
			if(pDevice->hardwareStatus.descriptor[dspIndex][index]== false)
			{
				BDBG_MSG(("BDSP_Raaga_P_AssignDescriptor:Pool index =%d",index));
				break;
			}
		}
		if(index < BDSP_MAX_POOL_OF_DESCRIPTORS)
		{
			uint32_t poolIndex = 0;
			uint32_t count = (((numDescriptors-allocatedDescriptors)>BDSP_MAX_DESCRIPTORS_PER_POOL)?BDSP_MAX_DESCRIPTORS_PER_POOL:(numDescriptors-allocatedDescriptors));
			pDevice->hardwareStatus.descriptor[dspIndex][index]=true;
			Memory = pDevice->memInfo.DescriptorMemory[dspIndex][index].Buffer;
			while(poolIndex < count)
			{
				BDBG_MSG(("BDSP_Raaga_P_AssignDescriptor pAddr = %p Offset ="BDSP_MSG_FMT, Memory.pAddr, BDSP_MSG_ARG(Memory.offset)));
				*pMemory = Memory;
				pMemory++;
				Memory.pAddr = (void *)((uint8_t *)Memory.pAddr+sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
				Memory.offset= Memory.offset+sizeof(BDSP_AF_P_sCIRCULAR_BUFFER);
				allocatedDescriptors++;
				poolIndex++;
			}
		}
		else
		{
			BDBG_ERR(("BDSP_Raaga_P_AssignDescriptor: Ran out of Descriptors requested = %d, assigned = %d",numDescriptors,allocatedDescriptors));
			errCode = BERR_INVALID_PARAMETER;
			break;
		}
	}
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	BDBG_LEAVE(BDSP_Raaga_P_AssignDescriptor);
	return errCode;
}

static BERR_Code BDSP_Raaga_P_ReleaseDescriptor(
	BDSP_Raaga *pDevice,
	unsigned dspIndex,
	dramaddr_t *pOffset,
	unsigned numDescriptors
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned index = 0, releasedDescriptors = 0;

	BDBG_ENTER(BDSP_Raaga_P_ReleaseDescriptor);
	BKNI_AcquireMutex(pDevice->deviceMutex);

	BDBG_MSG(("BDSP_Raaga_P_ReleaseDescriptor: Number of descriptor = %d",numDescriptors));
	while(releasedDescriptors < numDescriptors)
	{
		uint32_t count =(((numDescriptors-releasedDescriptors)>BDSP_MAX_DESCRIPTORS_PER_POOL)?BDSP_MAX_DESCRIPTORS_PER_POOL:(numDescriptors-releasedDescriptors));
		BDBG_MSG(("BDSP_Raaga_P_ReleaseDescriptor: Descriptor ="BDSP_MSG_FMT,BDSP_MSG_ARG(*pOffset)));
		for(;index<BDSP_MAX_POOL_OF_DESCRIPTORS; index++)
		{
			if(*pOffset == pDevice->memInfo.DescriptorMemory[dspIndex][index].Buffer.offset)
			{
				BDBG_MSG(("BDSP_Raaga_P_ReleaseDescriptor:Pool index =%d",index));
				pDevice->hardwareStatus.descriptor[dspIndex][index]=false;
				releasedDescriptors = releasedDescriptors+count;
				pOffset = pOffset+count;
				break;
			}
		}
	}
	if(releasedDescriptors != numDescriptors)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleaseDescriptor: Number Descriptors Released (%d), Requested (%d)",releasedDescriptors,numDescriptors));
		errCode = BERR_INVALID_PARAMETER;
	}
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	BDBG_LEAVE(BDSP_Raaga_P_ReleaseDescriptor);
	return errCode;
}

BERR_Code BDSP_Raaga_P_ReleasePortDescriptors(
	BDSP_Raaga *pDevice,
	unsigned    dspIndex,
	BDSP_AF_P_sIoPort *psIoPort
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned j=0, i=0, k=0;
	dramaddr_t decriptorArray[BDSP_MAX_DESCRIPTORS_PER_POOL];
	for(j=0;j<psIoPort->ui32numPortBuffer;j++)
	{
		for(i=0;i<psIoPort->sIoBuffer[j].ui32NumBuffer;i++)
		{
			decriptorArray[k++] = psIoPort->sIoBuffer[j].sCircularBuffer[i];
			psIoPort->sIoBuffer[j].sCircularBuffer[i]=0;
		}
	}
	if(k > BDSP_MAX_DESCRIPTORS_PER_POOL)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleasePortDescriptors: Number of Descriptors being Released(%d) is more than Cacheline(%d)",k,BDSP_MAX_DESCRIPTORS_PER_POOL));
	}
	errCode = BDSP_Raaga_P_ReleaseDescriptor(
		pDevice,
		dspIndex,
		&decriptorArray[0],
		k);
	if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Raaga_P_ReleasePortDescriptors: Error in Cleanup of Descriptor"));
	}
	return errCode;
}

BERR_Code BDSP_Raaga_P_GetMemoryEstimate(
	const BDSP_RaagaSettings     *pSettings,
	const BDSP_UsageOptions      *pUsage,
	BBOX_Handle                   boxHandle,
	BDSP_MemoryEstimate          *pEstimate /*[out]*/
)
{
	BERR_Code errCode = BERR_SUCCESS;
	unsigned NumDsp = BDSP_RAAGA_MAX_DSP, NumCores = BDSP_RAAGA_MAX_CORE_PER_DSP;
	unsigned MemoryRequired = 0, InterTaskMemory = 0, index = 0;
    unsigned NumChannels = 0;
	BDSP_AF_P_DistinctOpType distinctOp;
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
		pEstimate->FirmwareMemory = BDSP_ALIGN_SIZE(pEstimate->FirmwareMemory,4096);
		BDBG_MSG(("Total RO Memory = %d (%d KB) (%d MB)", pEstimate->FirmwareMemory, (pEstimate->FirmwareMemory/1024), (pEstimate->FirmwareMemory/(1024*1024))));
		BKNI_Free(pCodeInfo);
	}

	/*Calculate General Heap Memory */
	{
		/*RW Section*/
		for(index=0; index<NumDsp; index++)
		{
			BDSP_Raaga_P_CalculateKernelRWMemory(&MemoryRequired);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("RW Section 1: KERNAL RW Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			BDSP_Raaga_P_CalculateHostFWsharedRWMemory(pSettings, index, &MemoryRequired);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("RW Section 2: HOST-FIRMWARE SHARED Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			BDSP_Raaga_P_CalculateProcessRWMemory(&MemoryRequired);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("RW Section 3.1: PROCESS RW Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			BDSP_Raaga_P_CalculateScratchAndInterStageMemory(NULL, index, NumCores, &MemoryRequired, true, pUsage);
			pEstimate->GeneralMemory += MemoryRequired;
			BDBG_MSG(("RW Section 3.2: WORK BUFFER Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

			pEstimate->GeneralMemory = BDSP_ALIGN_SIZE(pEstimate->GeneralMemory, 4096);/* Align the size to 4k */
		}
		BDBG_MSG(("Total RW Memory(%d DSP) = %d (%d KB) (%d MB)",NumDsp, pEstimate->GeneralMemory, (pEstimate->GeneralMemory/1024), (pEstimate->GeneralMemory/(1024*1024))));

		/* Task Memory */
		BDSP_Raaga_P_CalculateTaskMemory(&MemoryRequired);
		pEstimate->GeneralMemory += (MemoryRequired*BDSP_MAX_FW_TASK_PER_DSP);
		BDBG_MSG(("Per Task Memory(%d Tasks) = %d (%d KB) (%d MB)",BDSP_MAX_FW_TASK_PER_DSP,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));

        /* Memory Allocation for Inter-task buffer */
        BDSP_P_GetDistinctOpTypeAndNumChans(pUsage->IntertaskBufferDataType, &NumChannels, &distinctOp);
        BDSP_P_CalculateInterTaskMemory(&InterTaskMemory, NumChannels);
        BDBG_MSG(("Per InterTask Memory = %d (%d KB) (%d MB)", InterTaskMemory, (InterTaskMemory/1024), (InterTaskMemory/(1024*1024))));

		/* Stage Memory*/
        /* Memory Allocation associated with Decoder */
        if(pUsage->NumAudioDecoders)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eAudioDecode, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumAudioDecoders);
            BDBG_MSG(("Decode Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumAudioDecoders,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }

        /* Memory Allocation associated with Post Processing */
        if(pUsage->NumAudioPostProcesses)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eAudioProcessing, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumAudioPostProcesses);
            BDBG_MSG(("P.P Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumAudioPostProcesses,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }

        /* Memory Allocation associated with Passthru */
        if(pUsage->NumAudioPassthru)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eAudioPassthrough, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumAudioPassthru);
            BDBG_MSG(("Passthru Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumAudioPassthru,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }

        /* Memory Allocation associated with Audio Encoder */
        if(pUsage->NumAudioEncoders)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eAudioEncode, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumAudioEncoders);
            BDBG_MSG(("Encoder Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumAudioEncoders,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }

        /* Memory Allocation associated with Mixer
             We estimate that we can connect atmost 4 Intertask buffers per Mixer*/
        if(pUsage->NumAudioMixers)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eAudioMixer, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumAudioMixers);
            BDBG_MSG(("Mixer Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumAudioMixers,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;

            MemoryRequired = (InterTaskMemory*pUsage->NumAudioMixers*BDSP_MAX_INTERTASKBUFFER_INPUT_TO_MIXER);
            BDBG_MSG(("Mixer InterTask Memory(%d InterTask buffers) = %d (%d KB) (%d MB)",BDSP_MAX_INTERTASKBUFFER_INPUT_TO_MIXER,MemoryRequired,(MemoryRequired/1024),(MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }

        /* Memory Allocation associated with Echocanceller
             We estimate that we can connect atmost 1 Intertask buffers per Echocanceller*/
        if(pUsage->NumAudioEchocancellers)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eAudioEchoCanceller, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumAudioEchocancellers);
            BDBG_MSG(("EchoCanceller Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumAudioEchocancellers,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;

            MemoryRequired = (InterTaskMemory*pUsage->NumAudioEchocancellers*BDSP_MAX_INTERTASKBUFFER_INPUT_TO_ECHOCANCELLER);
            BDBG_MSG(("EchoCanceller InterTask Memory(%d InterTask buffers) = %d (%d KB) (%d MB)",BDSP_MAX_INTERTASKBUFFER_INPUT_TO_ECHOCANCELLER,MemoryRequired,(MemoryRequired/1024),(MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }

        /* Memory Allocation associated with Video Decoder */
        if(pUsage->NumVideoDecoders)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eVideoDecode, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumVideoDecoders);
            BDBG_MSG(("Video Decoder Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumVideoDecoders,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }

        /* Memory Allocation associated with Video Encoder */
        if(pUsage->NumVideoEncoders)
        {
            BDSP_Raaga_P_CalculateStageMemory(&MemoryRequired, BDSP_AlgorithmType_eVideoEncode, true, pUsage);
            MemoryRequired = (MemoryRequired*pUsage->NumVideoEncoders);
            BDBG_MSG(("Video Encoder Stage Memory(%d Stages) = %d (%d KB) (%d MB)",pUsage->NumVideoEncoders,MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
            pEstimate->GeneralMemory += MemoryRequired;
        }
	}
	BDBG_MSG(("Memory Required FIRMWARE = %d bytes(%d KB)  GENERAL = %d bytes(%d KB)",pEstimate->FirmwareMemory,(pEstimate->FirmwareMemory/1024),
					pEstimate->GeneralMemory,(pEstimate->GeneralMemory/1024)));
end:
	BDBG_LEAVE(BDSP_Raaga_P_GetMemoryEstimate);
	return errCode;
}
