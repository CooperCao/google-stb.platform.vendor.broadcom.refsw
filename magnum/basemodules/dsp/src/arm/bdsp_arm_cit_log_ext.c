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


#include "bdsp_arm_priv_include.h"

BDBG_MODULE(bdsp_ext);

void BDSP_Arm_P_Analyse_CIT_Audio_GlobalTaskConfig(BDSP_ArmTask      *pArmTask)
{
	BDSP_AF_P_sGLOBAL_TASK_CONFIG		   *psGblTaskCfg;
    BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG    *psTaskFmmGateOpenConfig;
	psGblTaskCfg = &pArmTask->citOutput.sCit.sGlobalTaskConfig;

	BDBG_MSG(("Global Task Configuration"));
	BDBG_MSG(("-------------------------------------------- "));

	BDBG_MSG(("Time base for the Task:%s",GlobalTimeBase[psGblTaskCfg->eTimeBaseType]));
	BDBG_MSG(("--"));

	/*	Dram Scratch buffer Address and Size */
	BDBG_MSG(("Scratch Buffer \t\tAddress: 0x%x \tSize Allocated: %d Bytes",psGblTaskCfg->sDramScratchBuffer.ui32DramBufferAddress,
									psGblTaskCfg->sDramScratchBuffer.ui32BufferSizeInBytes));
	/* DRAM port Configuration */
	BDBG_MSG(("Port Configuration \tAddress: 0x%x	",psGblTaskCfg->ui32FmmDestCfgAddr));
	BDBG_MSG(("--"));

	/* DRAM Gate Open Configuration */
	psTaskFmmGateOpenConfig = (BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG *)((uint8_t *)pArmTask->taskMemGrants.sTaskCfgBufInfo.Buffer.pAddr+
	                               (psGblTaskCfg->ui32FmmGateOpenConfigAddr - pArmTask->taskMemGrants.sTaskCfgBufInfo.Buffer.offset));
	BDBG_MSG(("Number of Ports for Gate Open = %d  ",psTaskFmmGateOpenConfig->ui32NumPorts));
	BDBG_MSG(("Maximum Independent Delay = %dms  ",psTaskFmmGateOpenConfig->ui32MaxIndepDelay));
	BDBG_MSG(("Gate Open Configuration address 0x%x  ",psGblTaskCfg->ui32FmmGateOpenConfigAddr));
	BDBG_MSG(("-------------------------------------------- "));
}

void BDSP_Arm_P_Analyse_CIT_NodeConfig(BDSP_ARM_AF_P_sNODE_CONFIG  *psNodeCfg)
{
	uint32_t    ui32NumSrc, ui32NumDest;
	BDSP_AF_P_sIO_BUFFER			sIoBuffer;
	BDSP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffer;
	BDBG_MSG(("--"));
	BDBG_MSG(("eCollectResidual : %s ",DisableEnable[psNodeCfg->eCollectResidual]));

	/*	Code Address and Size */
	BDBG_MSG(("Code Buffer \t\tAddress: 0x%x \t Size: %d Bytes",psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress,
				psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes));

	/*	Lookup Table Address and Size */
	if((psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress == BDSP_AF_P_DRAM_ADDR_INVALID)||
		(psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress == 0x0))
	{
		BDBG_MSG(("Lookup Table Buffer \tNot present"));
	}
	else
	{
		BDBG_MSG(("Lookup Table Buffer \tAddress: 0x%x \t Size: %d Bytes",psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress,
					psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes));
	}

	/*	Inter-frame buffer Address check */
	if((psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress == BDSP_AF_P_DRAM_ADDR_INVALID)||
		(psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress == 0x0))
	{
		BDBG_MSG(("Inter-Frame Buffer \tNot Present"));
	}
	else
	{
		BDBG_MSG(("Inter-Frame Buffer \tAddress: 0x%x  \t Size: %d Bytes",psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress,
					psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes));
	}

	/*	Node Status buffer Address check */
	if((psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress == BDSP_AF_P_DRAM_ADDR_INVALID)||
		(psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress == 0x0))
	{
		BDBG_MSG(("Node Status Buffer \tNot Present"));
	}
	else
	{
		BDBG_MSG(("Node Status Buffer \tAddress: 0x%x \t Size: %d Bytes",psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress,
					psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes));
	}

	/*	User config buffer Address check */
	if((psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress == BDSP_AF_P_DRAM_ADDR_INVALID)||
		(psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress == 0x0))
	{
		BDBG_MSG(("User Config Buffer \t Not Present"));
	}
	else
	{
		BDBG_MSG(("User Config Buffer \tAddress: 0x%x \t Size: %d Bytes",psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress,
					psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes));
	}

	/*	Input buffer configuration details */
	BDBG_MSG(("--"));
	BDBG_MSG(("Num Source feeding data to this node: %d", psNodeCfg->ui32NumSrc));
	for( ui32NumSrc=0; ui32NumSrc<psNodeCfg->ui32NumSrc; ui32NumSrc++)
	{
		BDBG_MSG(("--"));
		if(psNodeCfg->eNodeIpValidFlag[ui32NumSrc] == BDSP_AF_P_eValid)
		{
			/* IO BUFFER CONFIGURATION */
			BDBG_MSG(("Source %d Input Buffer Cfg Structure Address: 0x%x",ui32NumSrc, psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc]));
#if 0
			if(0 != psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc])
			{
				/* Getting the Virtual Address */
				BDSP_P_ReadFromOffset(hHeap,
					psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc],
					(void *)&sIoBuffer,
					(uint32_t)SIZEOF(BDSP_AF_P_sIO_BUFFER));

				/*Printing Buffer Type*/
				if((sIoBuffer.ui32NumBuffers >0) && (BDSP_AF_P_eValid == psNodeCfg->eNodeIpValidFlag[ui32NumSrc]) )
				{
					BDBG_MSG(("Source %d Input Buffer Type: %s",ui32NumSrc, BuffTypeEnum2Char[sIoBuffer.eBufferType]));
					/*	Analyze Io Buff Struct */
					BDSP_P_Analyse_CIT_BuffCfgStruct(&sIoBuffer ,BDSP_P_IO_BufferType_IO);
				}
			}
#else
			BSTD_UNUSED(sIoBuffer);
#endif /* 0 */
			/* IOGENERIC BUFFER CONFIGURATION */
			BDBG_MSG(("Source %d Input Generic Buffer Cfg Structure Address: 0x%x",ui32NumSrc, psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc]));
#if 0
			if(0 != psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc])
			{
				BDSP_P_ReadFromOffset(hHeap,
					psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc],
					(void *)&sIoGenericBuffer,
					(uint32_t)SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));

				/*Printing Buffer Type*/
				if(sIoGenericBuffer.ui32NumBuffers >0)
				{
					BDBG_MSG(("Source %d Input Generic Buffer Type: %s",ui32NumSrc,BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));
					/*	Analyze Io Genric Buff Struct */
					BDSP_P_Analyse_CIT_BuffCfgStruct(&sIoGenericBuffer ,BDSP_P_IO_BufferType_IOGen);
				}
			}
#else
			BSTD_UNUSED(sIoGenericBuffer);
#endif /* 0 */
		}
	}

	/*	Output buffer configuration details */
	BDBG_MSG(("--"));
	BDBG_MSG(("Num Destination getting data from this node: %d",psNodeCfg->ui32NumDst));
	for( ui32NumDest=0; ui32NumDest<psNodeCfg->ui32NumDst; ui32NumDest++)
	{
		BDBG_MSG(("--"));
		BDBG_MSG(("Destination %d Datatype : %s",ui32NumDest, PortDatatType[psNodeCfg->eNodeOpBuffDataType[ui32NumDest]]));
		/* IO BUFFER CONFIGURATION */
		/*-------------------------*/
		/*Printing Output Buffer Cfg Structure Address */
		BDBG_MSG(("Destination %d Output Buffer Cfg Structure Address: 0x%x",ui32NumDest, psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest]));
#if 0
		if(0 != psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest])
		{
			/* Getting contents of the Destination IO buffer */
			/* Getting the Virtual Address */
		    BDSP_P_ReadFromOffset(hHeap,
					psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest],
					(void *)&sIoBuffer,
					(uint32_t)SIZEOF(BDSP_AF_P_sIO_BUFFER));

			/*Printing Buffer Type*/
			if(sIoBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Buffer Type: %s",ui32NumDest, BuffTypeEnum2Char[sIoBuffer.eBufferType]));
				/*	Print Io Buff Struct */
				BDSP_P_Analyse_CIT_BuffCfgStruct(&sIoBuffer ,BDSP_P_IO_BufferType_IO);
			}
		}
#else
		BSTD_UNUSED(sIoBuffer);
#endif /* 0 */

		/* IOGENERIC BUFFER CONFIGURATION */
		/*--------------------------------*/
		BDBG_MSG(("Destination %d Output Generic Buffer Cfg Structure Address: 0x%x",	ui32NumDest, psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest]));
#if 0
		/*	Getting contents of the IO Generic buffer */
		if(0 != psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest])
		{
			/*Getting the Virtual Address */
			BDSP_P_ReadFromOffset(hHeap,
				psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest],
				(void *)&sIoGenericBuffer,
				(uint32_t)SIZEOF(BDSP_AF_P_sIO_GENERIC_BUFFER));

			/*Printing Buffer Type*/
			if(sIoGenericBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Generic Buffer Type: %s",ui32NumDest, BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));
				/*	Analyze Io Genric Buff Struct */
				BDSP_P_Analyse_CIT_BuffCfgStruct(&sIoGenericBuffer ,BDSP_P_IO_BufferType_IOGen);
			}
		}
#else
		BSTD_UNUSED(sIoGenericBuffer);
#endif /* 0 */
	}
}
