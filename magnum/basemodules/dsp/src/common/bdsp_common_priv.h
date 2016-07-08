/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef BDSP_COMMON_PRIV_H_
#define BDSP_COMMON_PRIV_H_

#include "bdsp_common_priv_include.h"

#define BDSP_P_INVALID_TASK_ID            ((unsigned int)(-1))

/* This structure is used to store base pointer & size of buffers used by
  firmware like the interframe buffers & configparams buffers */
typedef struct BDSP_P_FwBuffer
{
   void       *pBaseAddr;
   uint32_t    ui32Size;
}BDSP_P_FwBuffer;

typedef struct BDSP_P_AlgoBufferOffsets
{
   uint32_t ui32IfOffset;      /* Offset to the interframe buffer */
   uint32_t ui32UserCfgOffset; /* Offset to the user config buffer */
   uint32_t ui32StatusOffset;  /* Offset to the status buffer */
}BDSP_P_AlgoBufferOffsets;
                                                                               /*(Decode + SRC + DSOLA)*/
BDBG_OBJECT_ID_DECLARE(BDSP_P_InterTaskBuffer);
typedef struct BDSP_P_InterTaskBuffer
{
    BDBG_OBJECT(BDSP_P_InterTaskBuffer)
    BDSP_InterTaskBuffer interTaskBuffer;

    BDSP_Context *pContext;
    bool inUse; /* Flag to indicate if the intertask buffer is in use */
    BDSP_DataType dataType; /* Type of data in the inter task buffer */
    BDSP_DataType numChans; /* Number of channels in the intertask buffer */
    BDSP_DataType distinctOp; /* Distinct output type of the intertask buffer */
    void *pMsgQueueParams; /* Pointer to the multiple queue parameters used per channel + one for IO generic buffer*/
    void *pIoBufferDesc; /* Io buffer descriptor pointer */
    void *pIoBufferGenericDesc; /* Io generic buffer descriptor pointer */
    BDSP_Stage *srcHandle; /* Source Stage handle */
    BDSP_Stage *dstHandle; /* Destination Stage Handle */
    int32_t    srcIndex; /* Index of the src Stage output to which the inter task buffer is connected */
    int32_t    dstIndex; /* Index of the dst Stage input to which the inter task buffer is conneceted */
    void       *pIoBuffer; /*Address of the IO buffer allocated */
    void       *pIoGenBuffer; /*Address of the IO Gen buffer allocated */
} BDSP_P_InterTaskBuffer;

void BDSP_P_GetDistinctOpTypeAndNumChans(
    BDSP_DataType dataType, /* [in] */
    unsigned *numChans, /* [out] */
    BDSP_AF_P_DistinctOpType *distinctOp /* [out] */
);

void BDSP_P_GetFreeOutputPortIndex(
    BDSP_StageSrcDstDetails *psStageOutput,
    unsigned *index
);

void BDSP_P_GetFreeInputPortIndex(
    BDSP_StageSrcDstDetails *psStageInput,
    unsigned *index
);

BERR_Code BDSP_DSP_P_InterframeRunLengthDecode(
    void *pSrc, void *pDst,
    uint32_t ui32IfBuffEncodedSize,
    uint32_t ui32AllocatedBufferSize
);

#endif /*BDSP_COMMON_PRIV_H_*/
