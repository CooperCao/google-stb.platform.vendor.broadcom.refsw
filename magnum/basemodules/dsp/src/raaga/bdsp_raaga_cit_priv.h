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
#ifndef BDSP_RAAGA_CIT_PRIV_H__
#define BDSP_RAAGA_CIT_PRIV_H__

#include "bdsp_common_priv_include.h"
#include "bdsp_raaga_types.h"

typedef struct BDSP_CIT_P_FwBufInfo
{
    BDSP_AF_P_AlgoId    eFwExecId;
    uint32_t            ui32InterframeBufAdr;
    uint32_t            ui32InterframeBufSize;
    uint32_t            ui32UserParamBufAdr;
    uint32_t            ui32UserParamBufSize;
    uint32_t            ui32StatusBufAdr;
    uint32_t            ui32StatusBufSize;
} BDSP_CIT_P_FwBufInfo;

typedef struct BDSP_CIT_P_OpStgInfo{
    BDSP_Algorithm              eAlgorithm;
    uint32_t                    ui32TsmNodeIndex;
    uint32_t                    ui32StartNodeIndex;
    uint32_t                    ui32NumNodes;
    BDSP_CIT_P_FwBufInfo        sFwOpNodeInfo[BDSP_AF_P_MAX_NUM_NODES_IN_ALGO];
} BDSP_CIT_P_OpStgInfo;

typedef struct BDSP_CIT_P_OpBranchInfo
{
    uint32_t                    ui32NumStages;
    BDSP_CIT_P_OpStgInfo        sCitStgInfo[BDSP_RAAGA_MAX_STAGE_PER_BRANCH];
} BDSP_CIT_P_OpBranchInfo;

typedef struct BDSP_CIT_P_Output
{
    BDSP_AF_P_sTASK_CONFIG      sCit;           /* Cit Structure */
    uint32_t                    ui32NumBranches;
    BDSP_CIT_P_OpBranchInfo     sCitBranchInfo[BDSP_RAAGA_MAX_BRANCH];
    BDSP_AF_P_sDRAM_BUFFER      sStackSwapBuff; /* Stack Swap Buffer */
    BDSP_AF_P_sDRAM_BUFFER      sSpdifUserConfigAddr[BDSP_AF_P_MAX_NUM_SPDIF_PORTS];
} BDSP_CIT_P_Output;

typedef struct BDSP_CIT_P_sTaskBuffInfo
{
    uint32_t                ui32TaskInterFrmMemSize;
    uint32_t                ui32TaskUsrCfgMemSize;
    uint32_t                ui32TaskIoBuffCfgStructMemSize;
    uint32_t                ui32TaskScratchMemSize;
    uint32_t                ui32BranchInterStgIoMemSize;
    uint32_t                ui32BranchInterStgGenericMemSize;
    uint32_t                ui32TaskInterStgIoMemSize;
    uint32_t                ui32TaskInterStgGenericMemSize;
    uint32_t                ui32NumInterStgBuffs;
    uint32_t                ui32MaxSizePerChannel;
    uint32_t                ui32MaxNumChannelsSupported;
    uint32_t                ui32TaskFwStatusBuffMemSize;
    uint32_t                ui32TaskStackMemSize;
    uint32_t                ui32TaskPortConfigMemSize;
    uint32_t                ui32TaskSPDIFConfigMemSize;
    uint32_t                ui32TaskGateOpenConfigMemSize;
    uint32_t                ui32TaskHwFwCfgMemSize;
    uint32_t                ui32SamplingFrequencyMapLutSize;
    uint32_t                ui32StcTrigConfigMemSize;
} BDSP_CIT_P_sTaskBuffInfo;

typedef union BDSP_VF_P_uTASK_CONFIG
{
    BDSP_VF_P_sDEC_TASK_CONFIG  sVideoDecTaskConfig;
    BDSP_VF_P_sENC_TASK_CONFIG  sVideoEncTaskConfig;
} BDSP_VF_P_uTASK_CONFIG;

typedef struct BDSP_CIT_P_VideoCITOutput
{
    BDSP_VF_P_uTASK_CONFIG          uVideoCit;          /* Video Cit Structure */
    uint32_t                    ui32NumBranches;
    BDSP_CIT_P_OpBranchInfo     sCitBranchInfo[BDSP_RAAGA_MAX_BRANCH];
    BDSP_AF_P_sDRAM_BUFFER      sStackSwapBuff; /* Stack Swap Buffer */
    BDSP_AF_P_sDRAM_BUFFER      sSpdifUserConfigAddr[BDSP_AF_P_MAX_NUM_SPDIF_PORTS];
} BDSP_CIT_P_VideoCITOutput;

typedef struct BDSP_CIT_P_ScmCITOutput
{
	BDSP_SCM_P_sTASK_CONFIG         sScmCit;                /* SCM Cit Structure */
	uint32_t                        ui32NumBranches;
	BDSP_CIT_P_OpBranchInfo         sCitBranchInfo[BDSP_RAAGA_MAX_BRANCH];
}BDSP_CIT_P_ScmCITOutput;

/*---------------------------------------------------*/
/* Prototype Definition for CIT Genreation functions */
/*---------------------------------------------------*/

BERR_Code BDSP_Raaga_P_GenCit(void* pTaskHandle);
BERR_Code BDSP_Raaga_P_GenScmCit(void* pTaskHandle);

BERR_Code BDSP_Raaga_P_GenVideoCit(void                *pTaskHandle,
                                               BDSP_AlgorithmType   eAlgorithm);
#endif /*BDSP_RAAGA_CIT_PRIV_H__*/
