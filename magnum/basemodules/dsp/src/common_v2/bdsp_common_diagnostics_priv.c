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

#include "bdsp_common_priv_include.h"
BDBG_MODULE(bdsp_diag);

void BDSP_P_Task_Memory_details(
	void 			*pMemInfoPtr,
	unsigned         taskID
)
{
	BDSP_P_TaskMemoryInfo *pMemInfo = (BDSP_P_TaskMemoryInfo *)pMemInfoPtr;
	BDBG_ENTER(BDSP_P_Task_Memory_details);
	BDBG_MSG(("================================================"));
	BDBG_MSG(("\t TASK %d MEMORY FOOTPRINT",taskID));
	BDBG_MSG(("\t ALLOCATED MEMORY RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Allocated = %d \t Used = %d",
		BDSP_MSG_ARG(pMemInfo->sMemoryPool.Memory.offset),
		BDSP_MSG_ARG(pMemInfo->sMemoryPool.Memory.offset+pMemInfo->sMemoryPool.ui32Size),
		pMemInfo->sMemoryPool.ui32Size,
		pMemInfo->sMemoryPool.ui32UsedSize));
	BDBG_MSG(("\t FW CIT RANGE \t \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sCITMemory.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sCITMemory.Buffer.offset+ pMemInfo->sCITMemory.ui32Size),
		pMemInfo->sCITMemory.ui32Size));
	BDBG_MSG(("\t HOST CIT RANGE \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sHostCITMemory.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sHostCITMemory.Buffer.offset+ pMemInfo->sHostCITMemory.ui32Size),
		pMemInfo->sHostCITMemory.ui32Size));
	BDBG_MSG(("\t SAMPLE RATE LUT RANGE \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sSampleRateLUTMemory.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sSampleRateLUTMemory.Buffer.offset+ pMemInfo->sSampleRateLUTMemory.ui32Size),
		pMemInfo->sSampleRateLUTMemory.ui32Size));
	BDBG_MSG(("\t SCHEDULING CONFIG RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sSchedulingConfigMemory.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sSchedulingConfigMemory.Buffer.offset+ pMemInfo->sSchedulingConfigMemory.ui32Size),
		pMemInfo->sSchedulingConfigMemory.ui32Size));
	BDBG_MSG(("\t GATE OPEN CONFIG RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sGateOpenConfigMemory.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sGateOpenConfigMemory.Buffer.offset+ pMemInfo->sGateOpenConfigMemory.ui32Size),
		pMemInfo->sGateOpenConfigMemory.ui32Size));
	BDBG_MSG(("\t STCTRIGGER CONFIG RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sStcTriggerConfigMemory.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sStcTriggerConfigMemory.Buffer.offset+ pMemInfo->sStcTriggerConfigMemory.ui32Size),
		pMemInfo->sStcTriggerConfigMemory.ui32Size));
	BDBG_MSG(("\t CACHE HOLE RANGE\t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sCacheHole.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sCacheHole.Buffer.offset+pMemInfo->sCacheHole.ui32Size),
		pMemInfo->sCacheHole.ui32Size));
	BDBG_MSG(("\t SYNC QUEUE RANGE \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->syncQueueParams.Memory.offset),
		BDSP_MSG_ARG(pMemInfo->syncQueueParams.Memory.offset+ pMemInfo->syncQueueParams.ui32Size),
		pMemInfo->syncQueueParams.ui32Size));
	BDBG_MSG(("\t ASYNC QUEUE RANGE \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->asyncQueueParams.Memory.offset),
		BDSP_MSG_ARG(pMemInfo->asyncQueueParams.Memory.offset+ pMemInfo->asyncQueueParams.ui32Size),
		pMemInfo->asyncQueueParams.ui32Size));
	BDBG_MSG(("\t MP-AP SHARED RANGE \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sMPSharedMemory.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sMPSharedMemory.Buffer.offset+ pMemInfo->sMPSharedMemory.ui32Size),
		pMemInfo->sMPSharedMemory.ui32Size));
	BDBG_MSG(("-------------------------------------------- "));
	BDBG_LEAVE(BDSP_P_Task_Memory_details);
}

void BDSP_P_Stage_Memory_details(
	void 			*pMemInfoPtr,
	BDSP_Algorithm   algorithm,
	unsigned         taskID
)
{
	BDSP_P_StageMemoryInfo *pMemInfo = (BDSP_P_StageMemoryInfo *)pMemInfoPtr;
	BDBG_ENTER(BDSP_P_Stage_Memory_details);
	BDBG_MSG(("-------------------------------------------- "));
	BDBG_MSG(("\t Task %d '%s' STAGE MEMORY FOOTPRINT",taskID,Algorithm2Name[algorithm]));
	BDBG_MSG(("\t ALLOCATED MEMORY RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Allocated = %d \t Used = %d",
		BDSP_MSG_ARG(pMemInfo->sMemoryPool.Memory.offset),
		BDSP_MSG_ARG(pMemInfo->sMemoryPool.Memory.offset+ pMemInfo->sMemoryPool.ui32Size),
		pMemInfo->sMemoryPool.ui32Size,
		pMemInfo->sMemoryPool.ui32UsedSize));
	BDBG_MSG(("\t INTERFRAME RANGE \t\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sInterframe.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sInterframe.Buffer.offset+ pMemInfo->sInterframe.ui32Size),
		pMemInfo->sInterframe.ui32Size));
	BDBG_MSG(("\t USER CONFIG RANGE \t\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sAlgoUserConfig.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sAlgoUserConfig.Buffer.offset+ pMemInfo->sAlgoUserConfig.ui32Size),
		pMemInfo->sAlgoUserConfig.ui32Size));
	BDBG_MSG(("\t ALGO STATUS RANGE \t\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sAlgoStatus.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sAlgoStatus.Buffer.offset+ pMemInfo->sAlgoStatus.ui32Size),
		pMemInfo->sAlgoStatus.ui32Size));
	BDBG_MSG(("\t IDS STATUS RANGE \t\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sIdsStatus.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sIdsStatus.Buffer.offset+ pMemInfo->sIdsStatus.ui32Size),
		pMemInfo->sIdsStatus.ui32Size));
	BDBG_MSG(("\t TSM STATUS RANGE \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sTsmStatus.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sTsmStatus.Buffer.offset+ pMemInfo->sTsmStatus.ui32Size),
		pMemInfo->sTsmStatus.ui32Size));
	BDBG_MSG(("\t CACHE HOLE RANGE \t \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sCacheHole.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sCacheHole.Buffer.offset+pMemInfo->sCacheHole.ui32Size),
		pMemInfo->sCacheHole.ui32Size));
	BDBG_MSG(("\t DATASYNC SETTINGS RANGE\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sDataSyncSettings.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sDataSyncSettings.Buffer.offset+ pMemInfo->sDataSyncSettings.ui32Size),
		pMemInfo->sDataSyncSettings.ui32Size));
	BDBG_MSG(("\t TSM SETTINGS RANGE \t\t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sTsmSettings.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sTsmSettings.Buffer.offset+ pMemInfo->sTsmSettings.ui32Size),
		pMemInfo->sTsmSettings.ui32Size));
	BDBG_MSG(("\t HOST USER CONFIG RANGE \t["BDSP_MSG_FMT":"BDSP_MSG_FMT"] Size =%d", BDSP_MSG_ARG(pMemInfo->sHostAlgoUserConfig.Buffer.offset),
		BDSP_MSG_ARG(pMemInfo->sHostAlgoUserConfig.Buffer.offset+ pMemInfo->sHostAlgoUserConfig.ui32Size),
		pMemInfo->sHostAlgoUserConfig.ui32Size));
	BDBG_MSG(("-------------------------------------------- "));
	BDBG_LEAVE(BDSP_P_Stage_Memory_details);
}
