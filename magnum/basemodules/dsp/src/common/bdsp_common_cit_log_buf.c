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
#include "bdsp_common_cit_priv.h"

BDBG_MODULE(bdsp_buf);

void BDSP_P_Analyse_CIT_PrintBufferAddrSize(BDSP_AF_P_sDRAM_BUFFER Buffer, uint32_t count)
{
	if(Buffer.ui32DramBufferAddress)
		BDBG_MSG(("		Buffer[%d]	Addr: " BDSP_MSG_FMT "\tSize: 0x%x",count, BDSP_MSG_ARG(Buffer.ui32DramBufferAddress), Buffer.ui32BufferSizeInBytes));
}
void BDSP_P_Analyse_CIT_PrintBufferAddr(BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *psCircBuff, BDSP_AF_P_BufferType eBufferType)
{
	if( ( eBufferType == BDSP_AF_P_BufferType_eRDB ) ||
		( eBufferType == BDSP_AF_P_BufferType_eRDBPool) ||
		( eBufferType == BDSP_AF_P_BufferType_eFMM ) ||
		( eBufferType == BDSP_AF_P_BufferType_eFMMSlave) ||
		( eBufferType == BDSP_AF_P_BufferType_eRAVE ) )
	{
		/*	The circular buffer will contain address of registers and not the actual DRAM addresses */
		BDBG_MSG(("      Address of Base Register : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32BaseAddr)));
		BDBG_MSG(("      Address of End Register  : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32EndAddr)));
		BDBG_MSG(("      Address of Write Register: " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32WriteAddr)));
		BDBG_MSG(("      Address of Read Register : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32ReadAddr)));
		BDBG_MSG(("      Address of Wrap Register : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32WrapAddr)));
	}
	else if((eBufferType == BDSP_AF_P_BufferType_eDRAM_IS )||
			(eBufferType == BDSP_AF_P_BufferType_eDRAM))
	{
		/*	The circular buffer the actual DRAM addresses as the buffer is Inter-stage */
		BDBG_MSG(("      DRAM Base Address : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32BaseAddr)));
		BDBG_MSG(("      DRAM End Address  : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32EndAddr)));
		BDBG_MSG(("      DRAM Write Address: " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32WriteAddr)));
		BDBG_MSG(("      DRAM Read Address : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32ReadAddr)));
		BDBG_MSG(("      DRAM Wrap Address : " BDSP_MSG_FMT, BDSP_MSG_ARG(psCircBuff->ui32WrapAddr)));
	}
	else
	{
		BDBG_MSG(("      Buffers have not been configured properly check your settings properly"));
	}
}

void BDSP_P_Analyse_CIT_BuffCfgStruct(void *pBuffer, BDSP_P_IO_BufferType eIOBufftype)
{
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *psCircBuff;
	if(BDSP_P_IO_BufferType_IOGen== eIOBufftype)
	{
		BDSP_AF_P_sIO_GENERIC_BUFFER	*psIoGenericBuffStruct;
		psIoGenericBuffStruct = (BDSP_AF_P_sIO_GENERIC_BUFFER *)pBuffer;
		BDBG_MSG(("**************"));
		BDBG_MSG(("Configuration details of IO GENERIC BUFFFER"));
		BDBG_MSG(("Number of buffers: %d", psIoGenericBuffStruct->ui32NumBuffers));
		BDBG_MSG(("    BUFFER %d",0));
		psCircBuff = &psIoGenericBuffStruct->sCircBuffer;
		BDSP_P_Analyse_CIT_PrintBufferAddr(psCircBuff,psIoGenericBuffStruct->eBufferType);
		BDBG_MSG(("**************"));
    }
	else if(BDSP_P_IO_BufferType_IO == eIOBufftype)
	{
		BDSP_AF_P_sIO_BUFFER	*psIoBuffStruct;
		uint32_t i;
		psIoBuffStruct = (BDSP_AF_P_sIO_BUFFER *)pBuffer;
		BDBG_MSG(("**************"));
		BDBG_MSG(("Configuration details of IO BUFFFER"));
		BDBG_MSG(("Number of buffers: %d", psIoBuffStruct->ui32NumBuffers));
		for(i=0;i<psIoBuffStruct->ui32NumBuffers;i++)
		{
			BDBG_MSG(("    BUFFER %d",i));
			psCircBuff = &psIoBuffStruct->sCircBuffer[i];
			BDSP_P_Analyse_CIT_PrintBufferAddr(psCircBuff, psIoBuffStruct->eBufferType);
			BDBG_MSG(("**************"));
		}
	}
}
