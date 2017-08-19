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

#ifndef BDSP_RAAGA_MM_PRIV_H_
#define BDSP_RAAGA_MM_PRIV_H_

#include "bdsp_raaga_priv_include.h"

#define BDSP_RAAGA_INVALID_DRAM_ADDRESS   0xFFFFFFFF

#define BDSP_RAAGA_MAX_MSGS_PER_QUEUE     	10
#define BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE 40
#define BDSP_RAAGA_MAX_FW_TASK_PER_DSP    	12

#define BDSP_RAAGA_MAX_BRANCH   		   	 2
#define BDSP_RAAGA_MAX_STAGE_PER_BRANCH   	10

#define BDSP_RAAGA_MAX_FW_TASK_PER_AUDIO_CTXT   12

#define BDSP_RAAGA_MAX_DESCRIPTORS			 100
#define BDSP_ATU_VIRTUAL_RO_MEM_START_ADDR   (0x0)		 /* RO memory mapped to addr : 0x0 */
#define BDSP_ATU_VIRTUAL_RW_MEM_START_ADDR   (0x10000000) /* RW memory mapped to addr : 0x10000000 (256MB)*/

#define BDSP_MAX_HOST_DSP_L2C_SIZE           512
#define BDSP_RAAGA_ALIGN_SIZE(x,y)   (((x+(y-1))/y)*y)
typedef enum BDSP_Raaga_P_ATUEntry{
	BDSP_Raaga_P_ATUEntry_ROMem =0,
	BDSP_Raaga_P_ATUEntry_RWMem,
	BDSP_Raaga_P_ATUEntry_Max,
	BDSP_Raaga_P_ATUEntry_Invalid = 0x7FFFFFFF
}BDSP_Raaga_P_ATUEntry;

typedef struct BDSP_Raaga_P_ATUInfo{
	BDSP_Raaga_P_ATUEntry eATUIndex;
	unsigned  size;
	uint32_t   ui32StartAddr;
	dramaddr_t offset;
}BDSP_Raaga_P_ATUInfo;

void BDSP_Raaga_P_CalculateInitMemory(
	unsigned *pMemReqd
);
void BDSP_Raaga_P_CalculateDebugMemory(
    BDSP_RaagaSettings *pSettings,
    unsigned           *pMemReqd
);
BERR_Code BDSP_Raaga_P_RequestMemory(
    BDSP_P_MemoryPool *pChunkBuffer,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
);
BERR_Code BDSP_Raaga_P_ReleaseMemory(
    BDSP_P_MemoryPool *pChunkBuffer,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
);

BERR_Code BDSP_Raaga_P_AssignTaskMemory(
	void *pTask
);

BERR_Code BDSP_Raaga_P_ReleaseTaskMemory(
	void *pTask
);
BERR_Code BDSP_Raaga_P_AssignStageMemory(
	void *pStage
);
BERR_Code BDSP_Raaga_P_ReleaseStageMemory(
	void *pStage
);
BERR_Code BDSP_Raaga_P_AssignDescriptor(
	void *pDeviceHandle,
	unsigned dspIndex,
	BDSP_MMA_Memory *pMemory,
	unsigned numDescriptors
);
BERR_Code BDSP_Raaga_P_ReleaseDescriptor(
	void *pDeviceHandle,
	unsigned dspIndex,
	dramaddr_t *pOffset,
	unsigned numDescriptors
);
BERR_Code BDSP_Raaga_P_GetMemoryEstimate(
	const BDSP_RaagaSettings     *pSettings,
	const BDSP_RaagaUsageOptions *pUsage,
	BBOX_Handle                   boxHandle,
	BDSP_RaagaMemoryEstimate     *pEstimate /*[out]*/
);
#endif /*BDSP_RAAGA_MM_PRIV_H_*/
