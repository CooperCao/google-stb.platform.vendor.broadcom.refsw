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

#include "bdsp_arm_priv_include.h"

BDBG_MODULE(bdsp_arm_mm);

void BDSP_Arm_P_CalculateInitMemory(
    unsigned *pMemReqd
)
{
    BDBG_ENTER(BDSP_Arm_P_CalculateInitMemory);
    *pMemReqd = 0;

    /*Memory for Soft Registers used for HOST/FW communication*/
    *pMemReqd += (4*sizeof(dramaddr_t));
    /*Memory for Soft Fifo created for Arm. */
    *pMemReqd += BDSP_ARM_NUM_FIFOS*(4*sizeof(dramaddr_t));
    /*Memory for Command Queue, accounting for maximun commands per task(10) for max (12) tasks in a single DSP*/
    *pMemReqd += (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Command)*BDSP_MAX_FW_TASK_PER_DSP);
    /* Memory for Generic Response Queue, Maximum of (10) responses from the DSP */
    *pMemReqd += (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));

    BDBG_MSG(("Init Memory = %d",*pMemReqd));
    BDBG_LEAVE(BDSP_Arm_P_CalculateInitMemory);
}

static void BDSP_Arm_P_CalculateScratchAndInterStageMemory(
    BDSP_Arm   *pDevice,
    unsigned    dspIndex,
    unsigned    numCorePerDsp,
    unsigned   *pMemReqd,
    bool        ifMemApiTool,
	const BDSP_ArmUsageOptions *pUsage
)
{
    unsigned MemoryRequired = 0;
	unsigned preemptionLevel = 0;
	BDSP_Algorithm algorithm;
	const BDSP_P_AlgorithmInfo *pAlgoInfo;
	const BDSP_P_AlgorithmCodeInfo *pAlgoCodeInfo;
	bool supported = false;

    BDBG_ENTER(BDSP_Arm_P_CalculateScratchAndInterStageMemory);
    *pMemReqd = 0;

	for(preemptionLevel=0; preemptionLevel<BDSP_MAX_NUM_PREEMPTION_LEVELS; preemptionLevel++)
	{
		unsigned interStageBufferReqd=0, scratchBufferReqd = 0;
		for(algorithm=0; algorithm<BDSP_Algorithm_eMax; algorithm++)
		{
			pAlgoInfo = BDSP_P_LookupAlgorithmInfo(algorithm);
            pAlgoCodeInfo = BDSP_Arm_P_LookupAlgorithmCodeInfo(algorithm);

			if(ifMemApiTool)
			{
				/*Memory Estimate API */
				supported = pUsage->Codeclist[algorithm];
			}
			else
			{
				/* Normal Path */
				const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
				pAlgoSupportInfo = BDSP_Arm_P_LookupAlgorithmSupportInfo(algorithm);
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
		MemoryRequired += (scratchBufferReqd+(interStageBufferReqd*(BDSP_MAX_BRANCH+1)));
		if(!ifMemApiTool)
		{
			/* Normal Path */
			pDevice->memInfo.WorkBufferMemory[dspIndex][preemptionLevel].ui32Size =
			               (scratchBufferReqd+(interStageBufferReqd*(BDSP_MAX_BRANCH+1)));
		}
		BDBG_MSG(("Work Buffer Allocation for Preemption Level %d",preemptionLevel));
		BDBG_MSG(("Scratch = %d InterStage = %d Total(Scratch + %d Interstage) = %d",scratchBufferReqd,interStageBufferReqd,
			(BDSP_MAX_BRANCH+1),MemoryRequired));
	}

    *pMemReqd = (MemoryRequired*numCorePerDsp);
	BDBG_MSG(("Total Work Buffer Allocation for %d Cores = %d",numCorePerDsp,*pMemReqd));
    BDBG_LEAVE(BDSP_Arm_P_CalculateScratchAndInterStageMemory);
}

void BDSP_Arm_P_CalculateStageMemory(
	unsigned    *pMemReqd,
	BDSP_AlgorithmType  algoType,
    bool        ifMemApiTool,
	const BDSP_ArmUsageOptions *pUsage
)
{
	const BDSP_P_AlgorithmSupportInfo *pAlgoSupportInfo;
	const BDSP_P_AlgorithmInfo        *pAlgoInfo;
	const BDSP_P_AlgorithmCodeInfo    *pAlgoCodeInfo;
    unsigned i = 0;
	unsigned MemoryRequired = 0, tempSize = 0;
	bool supported = false;

    BDBG_ENTER(BDSP_Arm_P_CalculateStageMemory);
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
			pAlgoSupportInfo = BDSP_Arm_P_LookupAlgorithmSupportInfo(i);
			supported = pAlgoSupportInfo->supported;
		}
		if(supported)
		{
			pAlgoInfo = BDSP_P_LookupAlgorithmInfo(i);
            pAlgoCodeInfo = BDSP_Arm_P_LookupAlgorithmCodeInfo(i);
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
	BDBG_MSG(("BDSP_Arm_P_CalculateStageMemory: Memory Allocated for Stage = %d", MemoryRequired));
	*pMemReqd = MemoryRequired;

	BDBG_LEAVE(BDSP_Arm_P_CalculateStageMemory);
}

void BDSP_Arm_P_CalculateTaskMemory(
	unsigned *pMemReqd
)
{
	unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Arm_P_CalculateTaskMemory);
	*pMemReqd = 0;

	MemoryRequired += BDSP_ARM_IMG_MP_AP_SHARED_MEMORY_SIZE;
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

	BDBG_MSG(("BDSP_Arm_P_CalculateTaskMemory: Memory Allocated for Task = %d", MemoryRequired));
	BDBG_LEAVE(BDSP_Arm_P_CalculateTaskMemory);
}

void BDSP_Arm_P_CalculateDeviceRWMemory(
    BDSP_Arm   *pDevice,
    unsigned    dspIndex,
    unsigned   *pMemReqd
)
{
    unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_Arm_P_CalculateDeviceRWMemory);

    BDSP_Arm_P_CalculateInitMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

#if 0
    BDSP_Arm_P_CalculateDebugMemory(&pDevice->deviceSettings, &MemoryRequired);
    *pMemReqd += MemoryRequired;
#endif

    BDSP_Arm_P_CalculateScratchAndInterStageMemory(pDevice, dspIndex, pDevice->numCorePerDsp, &MemoryRequired, false, NULL);
    *pMemReqd += MemoryRequired;

#if 0
    BDSP_Arm_P_CalculateProcessRWMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;
#endif

    BDSP_P_CalculateDescriptorMemory(&MemoryRequired);
    *pMemReqd += MemoryRequired;

    *pMemReqd = BDSP_ALIGN_SIZE(*pMemReqd, 4096);/* Align the size to 4k */

    BDBG_MSG(("BDSP_Arm_P_CalculateDeviceRWMemory: Memory Allocated for Device RW = %d", *pMemReqd));
    BDBG_LEAVE(BDSP_Arm_P_CalculateDeviceRWMemory);
}

void BDSP_Arm_P_CalculateDeviceIOMemory(
    BDSP_Arm *pDevice,
    unsigned *pMemReqd
)
{
    BDBG_ENTER(BDSP_Arm_P_CalculateDeviceIOMemory);
    *pMemReqd = BDSP_ARM_IMG_SYSTEM_IO_SIZE;/* Implicit allocation for IO */

    *pMemReqd = BDSP_ALIGN_SIZE(*pMemReqd,4096);

    BDBG_MSG(("BDSP_Arm_P_CalculateDeviceROMemory: Memory Allocated for Device IO(RDB Mapped Space) = %d", *pMemReqd));
    BSTD_UNUSED(pDevice);
    BDBG_LEAVE(BDSP_Arm_P_CalculateDeviceIOMemory);
}

void BDSP_Arm_P_CalculateDeviceROMemory(
    BDSP_Arm *pDevice,
    unsigned *pMemReqd
)
{
    unsigned ResidentMemorySize = 0, LoadableMemorySize = 0;

    BDBG_ENTER(BDSP_Arm_P_CalculateDeviceROMemory);

    BDSP_Arm_P_ComputeResidentSection(&pDevice->codeInfo, &ResidentMemorySize);
    BDBG_MSG(("BDSP_Arm_P_CalculateDeviceROMemory: Resident code Memory Requirement = %d",ResidentMemorySize));
    *pMemReqd += ResidentMemorySize;

    BDSP_Arm_P_ComputeLoadbleSection((void *)pDevice, &LoadableMemorySize);
    BDBG_MSG(("BDSP_Arm_P_CalculateDeviceROMemory: Loadable code Memory Requirement = %d",LoadableMemorySize));
    *pMemReqd += LoadableMemorySize;

    *pMemReqd += 1024;
    *pMemReqd = BDSP_ALIGN_SIZE(*pMemReqd,4096);

    BDBG_MSG(("BDSP_Arm_P_CalculateDeviceROMemory: Memory Allocated for Device RO = %d", *pMemReqd));
    BDBG_LEAVE(BDSP_Arm_P_CalculateDeviceROMemory);
}

static BERR_Code BDSP_Arm_P_AssignInitMemory(
    BDSP_Arm *pDevice,
    unsigned  dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ui32Size = 0;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Arm_P_AssignInitMemory);

    ui32Size = (4*sizeof(dramaddr_t));
    errCode  = BDSP_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignInitMemory: Unable to allocate RW memory for Soft Fifo for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.CfgRegisters[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.CfgRegisters[dspindex].Buffer   = Memory;

    ui32Size = BDSP_ARM_NUM_FIFOS*(4*sizeof(dramaddr_t));
    errCode  = BDSP_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignInitMemory: Unable to allocate RW memory for Soft Fifo for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.softFifo[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.softFifo[dspindex].Buffer   = Memory;

    errCode = BDSP_Arm_P_AssignFreeFIFO(pDevice,dspindex,&(pDevice->memInfo.cmdQueueParams[dspindex].ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignInitMemory: Unable to find free fifo for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    ui32Size = BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Command)*BDSP_MAX_FW_TASK_PER_DSP;
    errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignInitMemory: Unable to allocate RW memory for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.cmdQueueParams[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.cmdQueueParams[dspindex].Memory   = Memory;

    errCode = BDSP_Arm_P_AssignFreeFIFO(pDevice,dspindex,&(pDevice->memInfo.genRspQueueParams[dspindex].ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignInitMemory: Unable to find free fifo for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    ui32Size = (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));
    errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignInitMemory: Unable to allocate RW memory for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.genRspQueueParams[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.genRspQueueParams[dspindex].Memory   = Memory;

end:
    BDBG_LEAVE(BDSP_Arm_P_AssignInitMemory);
    return errCode;
}

static BERR_Code BDSP_Arm_P_ReleaseInitMemory(
    BDSP_Arm *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Arm_P_ReleaseInitMemory);

    errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.CfgRegisters[dspindex].ui32Size,
		&pDevice->memInfo.CfgRegisters[dspindex].Buffer);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseInitMemory: Unable to release CFG Memory for DSP %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.CfgRegisters[dspindex].ui32Size = 0;

    errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.softFifo[dspindex].ui32Size,
		&pDevice->memInfo.softFifo[dspindex].Buffer);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseInitMemory: Unable to release Soft FIFO Memory for DSP %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.softFifo[dspindex].ui32Size = 0;

    errCode = BDSP_Arm_P_ReleaseFIFO(pDevice,dspindex,&(pDevice->memInfo.cmdQueueParams[dspindex].ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseInitMemory: Unable to release fifo for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
    errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.cmdQueueParams[dspindex].ui32Size,
		&pDevice->memInfo.cmdQueueParams[dspindex].Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseInitMemory: Unable to release RW memory for CMD QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }
	pDevice->memInfo.cmdQueueParams[dspindex].ui32Size 	= 0;

    errCode = BDSP_Arm_P_ReleaseFIFO(pDevice,dspindex,&(pDevice->memInfo.genRspQueueParams[dspindex].ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseInitMemory: Unable to release fifo for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
        goto end;
    }

	errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.genRspQueueParams[dspindex].ui32Size,
		&pDevice->memInfo.genRspQueueParams[dspindex].Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseInitMemory: Unable to release RW memory for GENERIC RESPONSE QUEUE for dsp %d!!!!",dspindex));
		goto end;
	}
	pDevice->memInfo.genRspQueueParams[dspindex].ui32Size 	= 0;

end:
	BDBG_LEAVE(BDSP_Arm_P_ReleaseInitMemory);
    return errCode;
}

static BERR_Code BDSP_Arm_P_AssignDescriptorMemory(
    BDSP_Arm *pDevice,
    unsigned  dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ui32Size = 0;
    BDSP_MMA_Memory Memory;
    BDBG_ENTER(BDSP_Arm_P_AssignDescriptorMemory);

    ui32Size = (BDSP_MAX_DESCRIPTORS * sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
    errCode = BDSP_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignDescriptorMemory: Unable to allocate RW memory for Descriptors for dsp %d!!!!",dspindex));
        goto end;
    }
    pDevice->memInfo.DescriptorMemory[dspindex].ui32Size = ui32Size;
    pDevice->memInfo.DescriptorMemory[dspindex].Buffer   = Memory;

end:
	BDBG_LEAVE(BDSP_Arm_P_AssignDescriptorMemory);
	return errCode;
}

static BERR_Code BDSP_Arm_P_ReleaseDescriptorMemory(
    BDSP_Arm *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ENTER(BDSP_Arm_P_ReleaseDescriptorMemory);

    errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.DescriptorMemory[dspindex].ui32Size,
		&pDevice->memInfo.DescriptorMemory[dspindex].Buffer);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseDescriptorMemory: Unable to Release RW memory for Descriptors for dsp %d!!!!",dspindex));
        goto end;
    }
	pDevice->memInfo.DescriptorMemory[dspindex].ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Arm_P_ReleaseDescriptorMemory);
	return errCode;
}

static BERR_Code BDSP_Arm_P_AssignWorkBufferMemory(
    BDSP_Arm *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
    uint32_t ui32Size = 0;
    BDSP_MMA_Memory Memory;
	unsigned preemptionLevel = 0;
    BDBG_ENTER(BDSP_Arm_P_AssignWorkBufferMemory);

    for(preemptionLevel=0; preemptionLevel<BDSP_MAX_NUM_PREEMPTION_LEVELS; preemptionLevel++)
    {
        ui32Size = pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size;
        errCode  = BDSP_P_RequestMemory(&pDevice->memInfo.sRWMemoryPool[dspindex], ui32Size, &Memory);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_AssignWorkBufferMemory: Unable to allocate RW memory for dsp %d with Preemption level =%d !!!!",dspindex, preemptionLevel));
            goto end;
        }
        pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].Buffer = Memory;
    }

end:
	BDBG_LEAVE(BDSP_Arm_P_AssignWorkBufferMemory);
	return errCode;
}

static BERR_Code BDSP_Arm_P_ReleaseWorkBufferMemory(
    BDSP_Arm *pDevice,
    unsigned dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;
	unsigned preemptionLevel = 0;
    BDBG_ENTER(BDSP_Arm_P_ReleaseWorkBufferMemory);

    for(preemptionLevel=0; preemptionLevel<BDSP_MAX_NUM_PREEMPTION_LEVELS; preemptionLevel++)
    {
        errCode = BDSP_P_ReleaseMemory(&pDevice->memInfo.sRWMemoryPool[dspindex],
		pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size,
		&pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].Buffer);
        if(errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Arm_P_ReleaseWorkBufferMemory: Unable to Release RW memory for dsp %d with Preemption level =%d !!!!",dspindex, preemptionLevel));
            goto end;
        }
	pDevice->memInfo.WorkBufferMemory[dspindex][preemptionLevel].ui32Size = 0;
    }
end:
	BDBG_LEAVE(BDSP_Arm_P_ReleaseWorkBufferMemory);
	return errCode;
}

BERR_Code BDSP_Arm_P_AssignDeviceRWMemory(
    BDSP_Arm *pDevice,
    unsigned  dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Arm_P_AssignDeviceRWMemory);

    errCode = BDSP_Arm_P_AssignInitMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignDeviceRWMemory: Unable to Assign Init Memory for dsp %d!!!!",dspindex));
        goto end;
    }

#if 0
    errCode = BDSP_Arm_P_AssignDebugMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignDeviceRWMemory: Unable to Assign Debug Memory for dsp %d!!!!",dspindex));
        goto end;
    }
#endif

    errCode = BDSP_Arm_P_AssignDescriptorMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignDeviceRWMemory: Unable to Assign Descriptor Memory for dsp %d!!!!",dspindex));
        goto end;
    }

    errCode = BDSP_Arm_P_AssignWorkBufferMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignDeviceRWMemory: Unable to Assign Work Buffer Memory for dsp %d!!!!",dspindex));
        goto end;
    }

end:
    BDBG_LEAVE(BDSP_Arm_P_AssignDeviceRWMemory);
    return errCode;
}

BERR_Code BDSP_Arm_P_ReleaseDeviceRWMemory(
    BDSP_Arm *pDevice,
    unsigned  dspindex
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_Arm_P_ReleaseDeviceRWMemory);

    errCode = BDSP_Arm_P_ReleaseInitMemory(pDevice, dspindex);
    if(errCode)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseDeviceRWMemory: Unable to Release Init Memory for dsp %d!!!!",dspindex));
        goto end;
    }

#if 0
    errCode = BDSP_Arm_P_ReleaseDebugMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseDeviceRWMemory: Unable to Release Debug Memory for dsp %d!!!!",dspindex));
        goto end;
    }
#endif
	errCode = BDSP_Arm_P_ReleaseDescriptorMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseDescriptorMemory: Unable to Release Descriptor Memory for dsp %d!!!!",dspindex));
		goto end;
	}

    errCode = BDSP_Arm_P_ReleaseWorkBufferMemory(pDevice, dspindex);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseDeviceRWMemory: Unable to Release Workbuffer Memory for dsp %d!!!!",dspindex));
        goto end;
    }
end:
    BDBG_LEAVE(BDSP_Arm_P_ReleaseDeviceRWMemory);
    return errCode;
}

BERR_Code BDSP_Arm_P_AssignTaskMemory(
	void *pTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTask;
	BDSP_Arm *pDevice;
	unsigned dspIndex =0;
	uint32_t ui32Size =0;
	BDSP_MMA_Memory Memory;

    BDBG_ENTER(BDSP_Arm_P_AssignTaskMemory);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);

	pDevice = (BDSP_Arm *)pArmTask->pContext->pDevice;
	dspIndex = pArmTask->createSettings.dspIndex;

	/*Host Copy of Async Response Queue */
	pArmTask->taskMemInfo.hostAsyncQueue.pAddr = (void *)BKNI_Malloc(BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
	if(NULL == pArmTask->taskMemInfo.hostAsyncQueue.pAddr)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to allocate BKNI Memory for Async buffer copy of HOST %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.hostAsyncQueue.ui32Size = (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));

    ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for CIT Buffer of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sCITMemory.Buffer  = Memory;
	pArmTask->taskMemInfo.sCITMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for CIT Reconfig Buffer of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sHostCITMemory.Buffer  = Memory;
	pArmTask->taskMemInfo.sHostCITMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sOpSamplingFreq);
	errCode  = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for Sample Rate LUT of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sSampleRateLUTMemory.Buffer  = Memory;
	pArmTask->taskMemInfo.sSampleRateLUTMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_SCHEDULING_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for Scheduling Config of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sSchedulingConfigMemory.Buffer  = Memory;
	pArmTask->taskMemInfo.sSchedulingConfigMemory.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sTASK_GATEOPEN_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for Gate Open Config of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sGateOpenConfigMemory.Buffer	= Memory;
	pArmTask->taskMemInfo.sGateOpenConfigMemory.ui32Size  = ui32Size;

	ui32Size = sizeof(BDSP_AF_P_sSTC_TRIGGER_CONFIG);
	errCode  = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for STC Trigger Config of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sStcTriggerConfigMemory.Buffer	= Memory;
	pArmTask->taskMemInfo.sStcTriggerConfigMemory.ui32Size  = ui32Size;

    ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
    errCode  = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign Cache Hole Memory of Task %d",pArmTask->taskParams.taskId));
        goto end;
    }
    pArmTask->taskMemInfo.sCacheHole.ui32Size = ui32Size;
    pArmTask->taskMemInfo.sCacheHole.Buffer   = Memory;

    errCode = BDSP_Arm_P_AssignFreeFIFO(pDevice, dspIndex,&(pArmTask->taskMemInfo.syncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to find free fifo for SYNC QUEUE of TASK %d",pArmTask->taskParams.taskId));
        goto end;
    }
    ui32Size = (BDSP_MAX_MSGS_PER_QUEUE * sizeof(BDSP_P_Response));
    errCode = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for SYNC QUEUE of Task %d",pArmTask->taskParams.taskId));
        goto end;
    }
    pArmTask->taskMemInfo.syncQueueParams.ui32Size = ui32Size;
    pArmTask->taskMemInfo.syncQueueParams.Memory   = Memory;

	errCode = BDSP_Arm_P_AssignFreeFIFO(pDevice, dspIndex,&(pArmTask->taskMemInfo.asyncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to find free fifo for ASYNC QUEUE of TASK %d",pArmTask->taskParams.taskId));
		goto end;
	}
	ui32Size = (BDSP_MAX_ASYNC_MSGS_PER_QUEUE * sizeof(BDSP_P_AsynMsg));
	errCode = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for ASYNC QUEUE of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.asyncQueueParams.ui32Size = ui32Size;
	pArmTask->taskMemInfo.asyncQueueParams.Memory	  = Memory;

	ui32Size = BDSP_ARM_IMG_MP_AP_SHARED_MEMORY_SIZE;
	errCode = BDSP_P_RequestMemory(&pArmTask->taskMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignTaskMemory: Unable to assign RW memory for MP Shared Memory of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sMPSharedMemory.ui32Size = ui32Size;
	pArmTask->taskMemInfo.sMPSharedMemory.Buffer	 = Memory;

end:
	BDBG_LEAVE(BDSP_Arm_P_AssignTaskMemory);
	return errCode;
}

BERR_Code BDSP_Arm_P_ReleaseTaskMemory(
	void *pTask
)
{
    BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmTask *pArmTask = (BDSP_ArmTask *)pTask;
	BDSP_Arm *pDevice;
	unsigned dspIndex =0;

    BDBG_ENTER(BDSP_Arm_P_ReleaseTaskMemory);
	BDBG_OBJECT_ASSERT(pArmTask, BDSP_ArmTask);
	pDevice = (BDSP_Arm *)pArmTask->pContext->pDevice;
	dspIndex = pArmTask->createSettings.dspIndex;

	if(pArmTask->taskMemInfo.hostAsyncQueue.pAddr)
	{
		BKNI_Free(pArmTask->taskMemInfo.hostAsyncQueue.pAddr);
		pArmTask->taskMemInfo.hostAsyncQueue.pAddr = 0;
		pArmTask->taskMemInfo.hostAsyncQueue.ui32Size = 0;
	}

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sCITMemory.ui32Size,
		&pArmTask->taskMemInfo.sCITMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release CIT Memory of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sCITMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sHostCITMemory.ui32Size,
		&pArmTask->taskMemInfo.sHostCITMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release CIT ReConfig Memory of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sHostCITMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sSampleRateLUTMemory.ui32Size,
		&pArmTask->taskMemInfo.sSampleRateLUTMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release Sample Rate LUT of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sSampleRateLUTMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sSchedulingConfigMemory.ui32Size,
		&pArmTask->taskMemInfo.sSchedulingConfigMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release Scheduling Config of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sSchedulingConfigMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sGateOpenConfigMemory.ui32Size,
		&pArmTask->taskMemInfo.sGateOpenConfigMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release Gate Open Config of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sGateOpenConfigMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sStcTriggerConfigMemory.ui32Size,
		&pArmTask->taskMemInfo.sStcTriggerConfigMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release STC Trigger Config of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sStcTriggerConfigMemory.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sCacheHole.ui32Size,
		&pArmTask->taskMemInfo.sCacheHole.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release Cache Hole Memory of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sCacheHole.ui32Size = 0;

    errCode = BDSP_Arm_P_ReleaseFIFO(pDevice,dspIndex,&(pArmTask->taskMemInfo.syncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release fifo for SYNC QUEUE of Task %d",pArmTask->taskParams.taskId));
        goto end;
    }
    errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.syncQueueParams.ui32Size,
		&pArmTask->taskMemInfo.syncQueueParams.Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release RW memory for SYNC QUEUE of Task %d",pArmTask->taskParams.taskId));
        goto end;
    }
	pArmTask->taskMemInfo.syncQueueParams.ui32Size = 0;

	errCode = BDSP_Arm_P_ReleaseFIFO(pDevice,dspIndex,&(pArmTask->taskMemInfo.asyncQueueParams.ui32FifoId), 1);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release fifo for ASYNC QUEUE of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.asyncQueueParams.ui32Size,
		&pArmTask->taskMemInfo.asyncQueueParams.Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release RW memory for ASYNC QUEUE of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.asyncQueueParams.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmTask->taskMemInfo.sMemoryPool,
		pArmTask->taskMemInfo.sMPSharedMemory.ui32Size,
		&pArmTask->taskMemInfo.sMPSharedMemory.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseTaskMemory: Unable to release RW memory for MP Shared of Task %d",pArmTask->taskParams.taskId));
		goto end;
	}
	pArmTask->taskMemInfo.sMPSharedMemory.ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Arm_P_ReleaseTaskMemory);
	return errCode;
}

BERR_Code BDSP_Arm_P_AssignStageMemory(
	void *pStage
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStage;
	const BDSP_P_AlgorithmInfo     *pAlgoInfo;
	const BDSP_P_AlgorithmCodeInfo *pAlgoCodeInfo;
    uint32_t ui32Size =0;
	BDSP_MMA_Memory Memory;

	BDBG_ENTER(BDSP_Arm_P_AssignStageMemory);
	BDBG_OBJECT_ASSERT(pArmStage, BDSP_ArmStage);

	pAlgoInfo = BDSP_P_LookupAlgorithmInfo(pArmStage->eAlgorithm);
    pAlgoCodeInfo = BDSP_Arm_P_LookupAlgorithmCodeInfo(pArmStage->eAlgorithm);

	ui32Size = sizeof(BDSP_AudioTaskDatasyncSettings);
	errCode  = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign DataSync Config memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pArmStage->stageMemInfo.sDataSyncSettings.Buffer  = Memory;
	pArmStage->stageMemInfo.sDataSyncSettings.ui32Size= ui32Size;

	ui32Size = sizeof(BDSP_AudioTaskTsmSettings);
	errCode  = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign TSM Config memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pArmStage->stageMemInfo.sTsmSettings.Buffer  = Memory;
	pArmStage->stageMemInfo.sTsmSettings.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoUserConfigSize;
	errCode = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign Host UserConfig memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pArmStage->stageMemInfo.sHostAlgoUserConfig.Buffer  = Memory;
	pArmStage->stageMemInfo.sHostAlgoUserConfig.ui32Size= ui32Size;

	/* Allocation of 512 bytes of hole memory to beat Cache Coherancy*/
	ui32Size = BDSP_MAX_HOST_DSP_L2C_SIZE;
	errCode  = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign Cache Hole memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pArmStage->stageMemInfo.sCacheHole.Buffer  = Memory;
	pArmStage->stageMemInfo.sCacheHole.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoUserConfigSize;
	errCode  = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign UserConfig memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pArmStage->stageMemInfo.sAlgoUserConfig.Buffer  = Memory;
	pArmStage->stageMemInfo.sAlgoUserConfig.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->algoStatusBufferSize;
	errCode = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign Status memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pArmStage->stageMemInfo.sAlgoStatus.Buffer  = Memory;
	pArmStage->stageMemInfo.sAlgoStatus.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->idsStatusBufferSize;
	errCode = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign IDS Status memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pArmStage->stageMemInfo.sIdsStatus.Buffer  = Memory;
	pArmStage->stageMemInfo.sIdsStatus.ui32Size= ui32Size;

	ui32Size = pAlgoInfo->tsmStatusBufferSize;
	errCode = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign TSM Status memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
        goto end;
    }
	pArmStage->stageMemInfo.sTsmStatus.Buffer  = Memory;
	pArmStage->stageMemInfo.sTsmStatus.ui32Size= ui32Size;

	ui32Size = pAlgoCodeInfo->interFrameSize;
	errCode = BDSP_P_RequestMemory(&pArmStage->stageMemInfo.sMemoryPool, ui32Size, &Memory);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_AssignStageMemory: Unable to assign Interframe memory for Stage(%d) %s",pArmStage->eAlgorithm,pAlgoInfo->pName));
		goto end;
	}
	pArmStage->stageMemInfo.sInterframe.Buffer  = Memory;
	pArmStage->stageMemInfo.sInterframe.ui32Size= ui32Size;

end:
	BDBG_LEAVE(BDSP_Arm_P_AssignStageMemory);
	return errCode;
}

BERR_Code BDSP_Arm_P_ReleaseStageMemory(
	void *pStage
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_ArmStage *pArmStage = (BDSP_ArmStage *)pStage;

	BDBG_ENTER(BDSP_Arm_P_ReleaseStageMemory);

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sDataSyncSettings.ui32Size,
		&pArmStage->stageMemInfo.sDataSyncSettings.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release IDS Settings memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sDataSyncSettings.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sTsmSettings.ui32Size,
		&pArmStage->stageMemInfo.sTsmSettings.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release TSM Settings memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sTsmSettings.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sHostAlgoUserConfig.ui32Size,
		&pArmStage->stageMemInfo.sHostAlgoUserConfig.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release Host UserConfig memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sHostAlgoUserConfig.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sCacheHole.ui32Size,
		&pArmStage->stageMemInfo.sCacheHole.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release Cache Hole memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sCacheHole.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sAlgoUserConfig.ui32Size,
		&pArmStage->stageMemInfo.sAlgoUserConfig.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release UserConfig memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sAlgoUserConfig.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sAlgoStatus.ui32Size,
		&pArmStage->stageMemInfo.sAlgoStatus.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release Algo Status memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sAlgoStatus.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sIdsStatus.ui32Size,
		&pArmStage->stageMemInfo.sIdsStatus.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release IDS Status memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sIdsStatus.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sTsmStatus.ui32Size,
		&pArmStage->stageMemInfo.sTsmStatus.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release TSM Status memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sTsmStatus.ui32Size = 0;

	errCode = BDSP_P_ReleaseMemory(&pArmStage->stageMemInfo.sMemoryPool,
		pArmStage->stageMemInfo.sInterframe.ui32Size,
		&pArmStage->stageMemInfo.sInterframe.Buffer);
    if(errCode != BERR_SUCCESS)
	{
		BDBG_ERR(("BDSP_Arm_P_ReleaseStageMemory: Unable to release Interframe memory for Stage(%d)",pArmStage->eAlgorithm));
		goto end;
	}
	pArmStage->stageMemInfo.sInterframe.ui32Size = 0;

end:
	BDBG_LEAVE(BDSP_Arm_P_ReleaseStageMemory);
	return errCode;
}

BERR_Code BDSP_Arm_P_AssignDescriptor(
	void *pDeviceHandle,
	unsigned dspIndex,
	BDSP_MMA_Memory *pMemory,
	unsigned numDescriptors
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Arm *pDevice;
	unsigned index = 0, allocatedDescriptors = 0;
	BDSP_MMA_Memory Memory;
	BDBG_ENTER(BDSP_Arm_P_AssignDescriptor);
	pDevice = (BDSP_Arm *)pDeviceHandle;
	BKNI_AcquireMutex(pDevice->deviceMutex);

	BDBG_MSG(("BDSP_Arm_P_AssignDescriptor: Number of descriptor = %d",numDescriptors));

	while(allocatedDescriptors != numDescriptors)
	{
		for(;index<BDSP_MAX_DESCRIPTORS; index++)
		{
			if(pDevice->hardwareStatus.descriptor[dspIndex][index]== false)
			{
				BDBG_MSG(("BDSP_Arm_P_AssignDescriptor:index =%d",index));
				break;
			}
		}
		if(index < BDSP_MAX_DESCRIPTORS)
		{
			pDevice->hardwareStatus.descriptor[dspIndex][index]=true;
			Memory = pDevice->memInfo.DescriptorMemory[dspIndex].Buffer;
			Memory.pAddr = (void *)((uint8_t *)pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.pAddr+(index *sizeof(BDSP_AF_P_sCIRCULAR_BUFFER)));
			Memory.offset= pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset+(index *sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
			BDBG_MSG(("BDSP_Arm_P_AssignDescriptor pAddr = %p Offset ="BDSP_MSG_FMT, Memory.pAddr, BDSP_MSG_ARG(Memory.offset)));
			*pMemory = Memory;
			pMemory++;
			allocatedDescriptors++;
		}
		else
		{
			BDBG_ERR(("BDSP_Arm_P_AssignDescriptor: Ran out of Descriptors requested =%d, assigned = %d",numDescriptors,allocatedDescriptors));
			errCode = BERR_INVALID_PARAMETER;
			break;
		}
	}
	BKNI_ReleaseMutex(pDevice->deviceMutex);

	BDBG_LEAVE(BDSP_Arm_P_AssignDescriptor);
	return errCode;
}

BERR_Code BDSP_Arm_P_ReleaseDescriptor(
	void *pDeviceHandle,
	unsigned dspIndex,
	dramaddr_t *pOffset,
	unsigned numDescriptors
)
{
	BERR_Code errCode = BERR_SUCCESS;
	BDSP_Arm *pDevice;
	unsigned index = 0, releasedDescriptors = 0;

	BDBG_ENTER(BDSP_Arm_P_ReleaseDescriptor);
	pDevice = (BDSP_Arm *)pDeviceHandle;
	BKNI_AcquireMutex(pDevice->deviceMutex);

	BDBG_MSG(("BDSP_Arm_P_ReleaseDescriptor: Number of descriptor = %d",numDescriptors));
	BKNI_ReleaseMutex(pDevice->deviceMutex);
	while(releasedDescriptors != numDescriptors)
	{
		BDBG_MSG(("BDSP_Arm_P_ReleaseDescriptor: Descriptor ="BDSP_MSG_FMT,BDSP_MSG_ARG(*pOffset)));
		if((*pOffset >= pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset)&&
		(*pOffset <=(pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset + pDevice->memInfo.DescriptorMemory[dspIndex].ui32Size)))
		{
			index = (((*pOffset) - pDevice->memInfo.DescriptorMemory[dspIndex].Buffer.offset)/sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
			BDBG_MSG(("BDSP_Arm_P_ReleaseDescriptor: index = %d", index));
			pDevice->hardwareStatus.descriptor[dspIndex][index]=false;
			pOffset++;
			releasedDescriptors++;
		}
		else
		{
			BDBG_ERR(("BDSP_Arm_P_ReleaseDescriptor: Trying to release a descriptor("BDSP_MSG_FMT") not in the block ",BDSP_MSG_ARG(*pOffset)));
			errCode = BERR_INVALID_PARAMETER;
			break;
		}
	}
	BDBG_LEAVE(BDSP_Arm_P_ReleaseDescriptor);
	return errCode;
}
