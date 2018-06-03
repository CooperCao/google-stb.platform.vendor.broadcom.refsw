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

#ifndef BDSP_ARM_CIT_PRIV_H__
#define BDSP_ARM_CIT_PRIV_H__

#define BDSP_ARM_CIT_P_MINIMUM_ALGO_THRESHOLD               ((uint32_t)(8192))  /*This is based on DTSHD HBR case */
#define BDSP_ARM_CIT_P_MAXIMUM_RESIDUAL_COLLECTION          ((uint32_t)(1024))  /*This value is based on the DSOLA*/

#define BDSP_ARM_CIT_P_BLOCKING_TIME                        ((uint32_t)(42))        /* In msec */

#define BDSP_ARM_CIT_P_MAT_BUF_SIZE                         ((uint32_t)(1536 * 1024 )) /*  In Bytes: Based on experiments done on MLP worst case streams */
#define BDSP_ARM_CIT_P_MAT_SAMPRATE_kHz                     ((uint32_t)768)

#define BDSP_ARM_CIT_P_INDEPENDENT_MAT_THRESHOLD            ((BDSP_ARM_CIT_P_MAT_BUF_SIZE >> 2 ) - ((BDSP_ARM_CIT_P_BLOCKING_TIME * 2)*BDSP_ARM_CIT_P_MAT_SAMPRATE_kHz )) /*((uint32_t)(328704))  ((base ptr -end ptr)/4 - (84*768))*/

#define BDSP_ARM_CIT_P_STANDARD_BUFFER_THRESHOLD            BDSP_ARM_CIT_P_MINIMUM_ALGO_THRESHOLD
#define BDSP_ARM_CIT_P_COMPRESSED4X_BUFFER_THRESHOLD        ((uint32_t)(1536*6*4))  /*Based on MS12 DDP Encoder where 6 blocks of 1536 samples are accumulated and since its 4X buffer we multiply by 4*/
#define BDSP_ARM_CIT_P_COMPRESSED16X_BUFFER_THRESHOLD       BDSP_ARM_CIT_P_MINIMUM_ALGO_THRESHOLD
#define BDSP_ARM_CIT_P_COMPRESSED16X_MLP_BUFFER_THRESHOLD   BDSP_ARM_CIT_P_INDEPENDENT_MAT_THRESHOLD

#define BDSP_ARM_MAX_STAGE_PER_BRANCH (2)/*MAX PP stage added externally + MAX internal stage added*/
                                                                               /*(Encode + Framse Sync)*/
#define BDSP_ARM_P_MAX_FW_STG_PER_FW_BRANCH             ((uint32_t)(BDSP_ARM_MAX_STAGE_PER_BRANCH))
#define BDSP_ARM_P_MAX_FW_BRANCH_PER_FW_TASK            ((uint32_t)BDSP_ARM_MAX_BRANCH)

/*We forsee max of only 4 tasks running*/
#define BDSP_ARM_MAX_FW_TASK_PER_CTXT                    (int32_t)BDSP_ARM_MAX_FW_TASK_PER_DSP

/*********************************************************************
Summary:
    Structure to describe the details of an interframe buffer for
    a FW node
Description:
    This structure describes the details of an interframe buffer
    created in CIT module for a FW node.
See Also:
**********************************************************************/
typedef struct BDSP_ARM_CIT_P_FwBufInfo
{
    BDSP_ARM_AF_P_AlgoId    eFwExecId;
    uint32_t                ui32InterframeBufAdr;
    uint32_t                ui32InterframeBufSize;
    uint32_t                ui32UserParamBufAdr;
    uint32_t                ui32UserParamBufSize;

    uint32_t                ui32StatusBufAdr;
    uint32_t                ui32StatusBufSize;

} BDSP_ARM_CIT_P_FwBufInfo;


/*********************************************************************
Summary:
    Structure to describe stage level auxiliary information generated
    by CIT module
Description:
    This structure describes auxiliary information generated by CIT
    module at stage level.
See Also:
**********************************************************************/

typedef struct BDSP_ARM_CIT_P_OpStgInfo{
    BDSP_Algorithm              eAlgorithm;
    uint32_t                    ui32TsmNodeIndex;
    uint32_t                    ui32StartNodeIndex;
    uint32_t                    ui32NumNodes;
    BDSP_ARM_CIT_P_FwBufInfo    sFwOpNodeInfo[BDSP_AF_P_MAX_NUM_NODES_IN_ALGO];
} BDSP_ARM_CIT_P_OpStgInfo;

/*********************************************************************
Summary:
    Structure to describe branch level auxiliary information generated
    by CIT module
Description:
    This structure describes auxiliary information generated by CIT
    module at branch level.
See Also:
**********************************************************************/

typedef struct BDSP_ARM_CIT_P_OpBranchInfo
{
    uint32_t                    ui32NumStages;
    BDSP_ARM_CIT_P_OpStgInfo        sCitStgInfo[BDSP_ARM_P_MAX_FW_STG_PER_FW_BRANCH];
} BDSP_ARM_CIT_P_OpBranchInfo;


/*********************************************************************
Summary:
    Structure to describe output of CIT module
Description:
    This structure describes output of CIT module. Output of CIT module contains
    following
    1. Completely initialized CIT structure
    2. Auxiliary information required by other  RAP PI modules to initilalize
        buffers created in CIT module
See Also:
**********************************************************************/
typedef struct BDSP_ARM_CIT_P_Output
{
    BDSP_ARM_AF_P_sTASK_CONFIG      sCit;           /* Cit Structure */

    uint32_t                        ui32NumBranches;

    BDSP_ARM_CIT_P_OpBranchInfo     sCitBranchInfo[BDSP_ARM_P_MAX_FW_BRANCH_PER_FW_TASK];

    BDSP_AF_P_sDRAM_BUFFER          sStackSwapBuff; /* Stack Swap Buffer */

    BDSP_AF_P_sDRAM_BUFFER          sSpdifUserConfigAddr[BDSP_AF_P_MAX_NUM_SPDIF_PORTS];

} BDSP_ARM_CIT_P_Output;


BERR_Code BDSP_Arm_P_GenCit(void* pTaskHandle);

#endif /* BDSP_ARM_CIT_PRIV_H__ */
