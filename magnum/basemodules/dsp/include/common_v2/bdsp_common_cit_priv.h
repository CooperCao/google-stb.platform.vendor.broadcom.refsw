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

#ifndef BDSP_COMMON_CIT_PRIV_H_
#define BDSP_COMMON_CIT_PRIV_H_

#include "bdsp_common_priv_include.h"

typedef struct BDSP_P_PortDetails{
	BDSP_AF_P_PortType          ePortType;
	BDSP_AF_P_DistinctOpType    distinctOpType;
	BDSP_AF_P_BufferType        eBufferType;
	unsigned                    numPortBuffers;
	unsigned                    numDataBuffers;
	unsigned                    numTocBuffers;
	unsigned                    numMetaDataBuffers;
	unsigned                    numObjectDataBuffers;
    unsigned                    tocIndex;
    unsigned                    numBranchFromPort;
    BDSP_AF_P_Port_sDataAccessAttributes *psDataAccesAttributes;
    dramaddr_t IoBuffer[BDSP_AF_P_MAX_PORT_BUFFERS][BDSP_AF_P_MAX_CHANNELS];
}BDSP_P_PortDetails;

typedef struct BDSP_P_RateController
{
     unsigned wrcnt;
}BDSP_P_RateController;

typedef struct BDSP_P_ConnectionDetails
{
    BDSP_AF_P_ValidInvalid  eValid;
    BDSP_ConnectionType     eConnectionType;
    BDSP_DataType           dataType;
	union
    {
        struct
            {
                BDSP_StageHandle hStage;
            } stage;
        struct
            {
                BDSP_FmmBufferDescriptor fmmDescriptor;
				BDSP_P_RateController rateController[BDSP_AF_P_MAX_CHANNEL_PAIR];
            } fmm;
        struct
            {
                BAVC_XptContextMap raveContextMap;
            } rave;
        struct
            {
                BDSP_InterTaskBufferHandle hInterTask;
            } interTask;
        struct
            {
                BDSP_QueueHandle pQHandle;
            } rdb;
    } connectionHandle;
}BDSP_P_ConnectionDetails;

void BDSP_P_FillSamplingFrequencyLut(
	BDSP_MMA_Memory *pLUTMemory
);
void BDSP_P_GetFMMDetails(
	BDSP_AF_P_DistinctOpType  eOutputType,
	BDSP_Algorithm			  eAlgorithm,
    BDSP_AF_P_FmmDstFsRate   *pBaseRateMultiplier,
    BDSP_AF_P_FmmContentType *pFMMContentType
);
BERR_Code BDSP_P_PopulateStcTriggerConfig(
    BDSP_TaskStartSettings  *pStartSettings,
    BDSP_MMA_Memory          Memory
);
void BDSP_P_UpdateResidueCollection(
    BDSP_P_ConnectionDetails *pOutputDetails,
    BDSP_AF_P_EnableDisable  *pCollectResidual
);
#endif /*BDSP_COMMON_CIT_PRIV_H_*/
