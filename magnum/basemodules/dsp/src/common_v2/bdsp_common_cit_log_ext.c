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
BDBG_MODULE(bdsp_ext);

void BDSP_P_Analyse_CIT_PortDetails(
	BDSP_AF_P_sIoPort *pPortDetails,
	BDSP_P_FwBuffer   *pDescriptorMemory
)
{
	unsigned i = 0;
	BDBG_MSG(("\t Port Type                = %s",PortType[pPortDetails->ePortType]));
	BDBG_MSG(("\t Distinct Output Type     = %s",DistinctOutputType[pPortDetails->ePortDataType]));
	BDBG_MSG(("\t Num Port Buffer          = %d",pPortDetails->ui32numPortBuffer));
	if(pPortDetails->ui32tocIndex != BDSP_AF_P_TOC_INVALID)
	{
		BDBG_MSG(("\t TOC Index                = %d",pPortDetails->ui32tocIndex));
	}
	if(pPortDetails->ui32numBranchfromPort != BDSP_AF_P_BRANCH_INVALID)
	{
		BDBG_MSG(("\t Branch from Port         = %d",pPortDetails->ui32numBranchfromPort));
	}
	if(pPortDetails->sIOGenericBuffer != 0)
	{
		BDBG_MSG(("\t IO GEN Buffer Descriptor = "BDSP_MSG_FMT,BDSP_MSG_ARG(pPortDetails->sIOGenericBuffer)));
	}
	BDSP_P_Analyse_CIT_DataAccess(&pPortDetails->sDataAccessAttributes);
	for(i=0;i<pPortDetails->ui32numPortBuffer;i++)
	{
		unsigned j = 0;
		BDBG_MSG(("\t IO Buffer: Num Buffers   = %d",pPortDetails->sIoBuffer[i].ui32NumBuffer));
		BDBG_MSG(("\t IO Buffer: Buffer Type   = %s",BufferType[pPortDetails->sIoBuffer[i].eBufferType]));
		for(j=0; j<pPortDetails->sIoBuffer[i].ui32NumBuffer; j++)
		{
			BDSP_P_Analyse_CIT_BufferDetails(pDescriptorMemory,
				pPortDetails->sIoBuffer[i].sCircularBuffer[j],
				j);
		}
	}
}
