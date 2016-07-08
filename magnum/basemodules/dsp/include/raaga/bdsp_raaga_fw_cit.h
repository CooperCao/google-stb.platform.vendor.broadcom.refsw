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


#ifndef _BRAAGA_CIT_PRIV_H__
#define _BRAAGA_CIT_PRIV_H__

#include "bdsp_raaga_fw.h"
#include "berr.h"
#include "bdsp_raaga_types.h"


/***************************************/
/*   FIRMWARE TASK CONFIGURATION       */
/***************************************/

/*  This value should be same as that of BDSP_AF_P_MAX_IP_FORKS */
#define BDSP_P_MAX_FW_STG_INPUTS                    ((uint32_t)3)
#define BDSP_P_MAX_FW_STG_OUTPUTS                   ((uint32_t)6)
#define BDSP_P_MAX_FW_STG_DISTINCT_OUTPUTS          ((uint32_t)5)   /*This is based on the maximum Port Type ever possible*/

#define BDSP_P_MAX_FW_STG_PER_FW_BRANCH             ((uint32_t)(BDSP_RAAGA_MAX_STAGE_PER_BRANCH)) /* Max No of PP Stage + 1 Decode Stage *//*       ((uint32_t)7)*/
#define BDSP_P_MAX_FW_BRANCH_PER_FW_TASK            ((uint32_t)BDSP_RAAGA_MAX_BRANCH)
#define BDSP_P_MAX_IS_BUFFER_PER_FW_TASK            ((uint32_t)BDSP_P_MAX_FW_BRANCH_PER_FW_TASK + 1)
#define BDSP_P_MAX_NODES_PER_FW_STG                 ((uint32_t)5)/*to be removed */
#define BDSP_P_MAX_NODES_IN_TASK                    ((uint32_t)20)/*to be removed */
#define BDSP_P_MAX_OUTPUTS                          ((uint32_t)8)
#define BDSP_P_MAX_ALGOS_IN_TASK                    ((uint32_t)( BDSP_P_MAX_FW_BRANCH_PER_FW_TASK * \
                                                                 BDSP_P_MAX_FW_STG_PER_FW_BRANCH ))

/*  This is the extra memory that the PI need to allocate
    in the task memory location other than the memory
    estimated from bdsp_af_priv.c file
*/
/* Threshold specific defines */
#ifdef BDSP_FLAC_SUPPORT
#define BDSP_CIT_P_MINIMUM_ALGO_THRESHOLD               ((uint32_t)(4*4608))  /*This is based on FLAC case where maximum samples of 4608 and if we attach SRC it get multiplied by 4*/
#else
#define BDSP_CIT_P_MINIMUM_ALGO_THRESHOLD               ((uint32_t)(8192))  /*This is based on DTSHD HBR case */
#endif
#define BDSP_CIT_P_MAXIMUM_RESIDUAL_COLLECTION          ((uint32_t)(1024))  /*This value is based on the DSOLA*/

#define BDSP_CIT_P_AUD_OFFSET                           ((uint32_t)(128))
#define BDSP_CIT_P_BLOCKING_TIME                        ((uint32_t)(42))        /* In msec */

#define BDSP_CIT_P_MAT_BUF_SIZE                         ((uint32_t)(1536 * 1024 )) /*  In Bytes: Based on experiments done on MLP worst case streams */
#define BDSP_CIT_P_MAT_SAMPRATE_kHz                     ((uint32_t)768)

#define BDSP_CIT_P_INDEPENDENT_MAT_THRESHOLD            ((BDSP_CIT_P_MAT_BUF_SIZE >> 2 ) - ((BDSP_CIT_P_BLOCKING_TIME * 2)*BDSP_CIT_P_MAT_SAMPRATE_kHz )) /*((uint32_t)(328704))  ((base ptr -end ptr)/4 - (84*768))*/

#define BDSP_CIT_P_STANDARD_BUFFER_THRESHOLD            BDSP_CIT_P_MINIMUM_ALGO_THRESHOLD
#define BDSP_CIT_P_COMPRESSED4X_BUFFER_THRESHOLD        ((uint32_t)(1536*6*4))  /*Based on MS12 DDP Encoder where 6 blocks of 1536 samples are accumulated and since its 4X buffer we multiply by 4*/
#define BDSP_CIT_P_COMPRESSED16X_BUFFER_THRESHOLD       BDSP_CIT_P_MINIMUM_ALGO_THRESHOLD
#define BDSP_CIT_P_COMPRESSED16X_MLP_BUFFER_THRESHOLD   BDSP_CIT_P_INDEPENDENT_MAT_THRESHOLD


/*----------------------------*/
/* CIT Module Private Defines */
/*----------------------------*/
#define BDSP_CIT_P_INVALID                          ((uint32_t)(0x7fffffff))
#define BDSP_CIT_P_NODE0                            ((uint32_t)(0))
#define BDSP_CIT_P_SRC0                             ((uint32_t)(0))
#define BDSP_CIT_P_DST0                             ((uint32_t)(0))
#define BDSP_CIT_P_NUM_SPECIAL_NODES                    ((uint32_t)(1))
#define BDSP_CIT_P_NUM_SPECIAL_NODES_IN_SCM_CXT     ((uint32_t)(0))
#define BDSP_CIT_P_TSM_NODE_INDEX                   ((uint32_t)(1))
#define BDSP_CIT_P_INVALID_NODE_IDX                 ((uint32_t)(0xFFFFFFFF))
#define BDSP_CIT_P_PI_INVALID                       ((uint32_t)(-1))

#define BDSP_CIT_MS4BITS_MASK                       (uint32_t)(0xF0000000)
#define BDSP_CIT_MS4BITS_TO_LS4BITS_SHIFT           (uint32_t)(28)


#if BCHP_RAAGA_DSP_FW_CFG_RAVE_TIMEBASE_SEL_CXT0
#define BDSP_CIT_ANCILLARY_FIFO_BASE       BCHP_RAAGA_DSP_FW_CFG_RAVE_TIMEBASE_SEL_CXT0
#define BDSP_CIT_ANCILLARY_FIFO_END        BDSP_CIT_ANCILLARY_FIFO_BASE + 4
#define BDSP_CIT_ANCILLARY_FIFO_WRITE      BDSP_CIT_ANCILLARY_FIFO_END  + 4
#define BDSP_CIT_ANCILLARY_FIFO_READ       BDSP_CIT_ANCILLARY_FIFO_WRITE + 4
#else
#define BDSP_CIT_ANCILLARY_FIFO_BASE       BCHP_RAAGA_DSP_FW_CFG_SW_UNDEFINED_SECTION0_i_ARRAY_BASE
#define BDSP_CIT_ANCILLARY_FIFO_END        BDSP_CIT_ANCILLARY_FIFO_BASE + 4
#define BDSP_CIT_ANCILLARY_FIFO_WRITE      BDSP_CIT_ANCILLARY_FIFO_END  + 4
#define BDSP_CIT_ANCILLARY_FIFO_READ       BDSP_CIT_ANCILLARY_FIFO_WRITE + 4
#endif



/* Error Defines*/
/*--------------*/
/*  Need to define error codes for each module and each scenario */
#define BDSP_CIT_P_ERROR                            ((uint32_t)0xFFFFFFFF)
#define BDSP_AF_P_DRAM_ADDR_INVALID                 ((uint32_t)0x80000000)

/*-----------------------------------------------------------------------*/

/*********************************************************************
Summary:
    Structure to describe the details of an interframe buffer for
    a FW node
Description:
    This structure describes the details of an interframe buffer
    created in CIT module for a FW node.
See Also:
**********************************************************************/
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

/*********************************************************************
Summary:
    Structure to describe stage level auxiliary information generated
    by CIT module
Description:
    This structure describes auxiliary information generated by CIT
    module at stage level.
See Also:
**********************************************************************/

typedef struct BDSP_CIT_P_OpStgInfo{
    BDSP_Algorithm              eAlgorithm;
    uint32_t                    ui32TsmNodeIndex;
    uint32_t                    ui32StartNodeIndex;
    uint32_t                    ui32NumNodes;
    BDSP_CIT_P_FwBufInfo        sFwOpNodeInfo[BDSP_AF_P_MAX_NUM_NODES_IN_ALGO];
} BDSP_CIT_P_OpStgInfo;

/*********************************************************************
Summary:
    Structure to describe branch level auxiliary information generated
    by CIT module
Description:
    This structure describes auxiliary information generated by CIT
    module at branch level.
See Also:
**********************************************************************/

typedef struct BDSP_CIT_P_OpBranchInfo
{
    uint32_t                    ui32NumStages;
    BDSP_CIT_P_OpStgInfo        sCitStgInfo[BDSP_P_MAX_FW_STG_PER_FW_BRANCH];
} BDSP_CIT_P_OpBranchInfo;

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
typedef struct BDSP_CIT_P_Output
{
    BDSP_AF_P_sTASK_CONFIG      sCit;           /* Cit Structure */

    uint32_t                    ui32NumBranches;

    BDSP_CIT_P_OpBranchInfo     sCitBranchInfo[BDSP_P_MAX_FW_BRANCH_PER_FW_TASK];

    BDSP_AF_P_sDRAM_BUFFER      sStackSwapBuff; /* Stack Swap Buffer */

    BDSP_AF_P_sDRAM_BUFFER      sSpdifUserConfigAddr[BDSP_AF_P_MAX_NUM_SPDIF_PORTS];

} BDSP_CIT_P_Output;



/*********************************************************************
Summary:
    This datastructure gives the memory requirement details for a task
    execution
Description:
    Memory requirement for a task
See Also:
**********************************************************************/
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

/*---------------------------------------------------*/
/* Prototype Definition for CIT Genreation functions */
/*---------------------------------------------------*/

uint32_t BDSP_P_GenNewCit(void* pTaskHandle);
uint32_t BDSP_P_GenNewScmCit(void* pTaskHandle);

void BDSP_P_AnalyseCit(BMEM_Handle                  hHeap,
                       BDSP_AF_P_sTASK_CONFIG       *psCit
                      );

uint32_t BDSP_P_GenNewVideoCit( void                       *pTaskHandle,
                                            BDSP_AlgorithmType          eAlgorithm);


/*--------------------------------------------------------------------------*/
/* Prototype Definition  and Structures for Calc Threshold and Block Time Fn*/
/*--------------------------------------------------------------------------*/

/*********************************************************************
Summary:
    Structure to describe the Ring Buffer properties for Src or Dst
    of a Stage.

Description:

See Also:
**********************************************************************/
typedef struct BDSP_CIT_P_sIO_BUFF_ID
{
    uint32_t                ui32NumBuffers;             /*  Defines the number of
                                                            channels in the input
                                                            or output */
    BDSP_AF_P_BufferType    eBufferType;                /*  Defines the the location
                                                            of the input or output buffer.
                                                            This can take values defined
                                                            by eBUFFER_TYPE */

    uint32_t                ui32BufferId[BDSP_AF_P_MAX_CHANNELS];
                                                        /* Valid for buffer types
                                                           BDSP_AF_P_eBufferType_RDB and
                                                           BDSP_AF_P_eBufferType_FMM */

    uint32_t                ui32AdaptRateCtrlIds[BDSP_AF_P_MAX_CHANNEL_PAIR]; /*Ids of Adapt rate ctrl block*/

}BDSP_CIT_P_sIO_BUFF_ID;

/****************************************************************************/
/****************************************************************************/
/************************* VIDEO TASK  **************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************
Summary:

    Union to describe task configuration for video decoder and encoder.

Description:
    1. Global task configuration of DSP task

    2. Details of the nodes present in DSP task

See Also:

******************************************************************************/

typedef union BDSP_VF_P_uTASK_CONFIG
{
    BDSP_VF_P_sDEC_TASK_CONFIG  sVideoDecTaskConfig;
    BDSP_VF_P_sENC_TASK_CONFIG  sVideoEncTaskConfig;

} BDSP_VF_P_uTASK_CONFIG;

/****************************************************************************
Summary:

    Structure to describe output of Video CIT module

Description:

    This structure describes output of Video CIT module. Output of Video CIT
    module contains following

    1. Completely initialized Video CIT structure

    2. Auxiliary information required by other  RAP PI modules to initilalize
        buffers created in Video CIT module

See Also:

******************************************************************************/

typedef struct BDSP_CIT_P_VideoCITOutput
{
    BDSP_VF_P_uTASK_CONFIG          uVideoCit;          /* Video Cit Structure */

    uint32_t                    ui32NumBranches;

    BDSP_CIT_P_OpBranchInfo     sCitBranchInfo[BDSP_P_MAX_FW_BRANCH_PER_FW_TASK];

    BDSP_AF_P_sDRAM_BUFFER      sStackSwapBuff; /* Stack Swap Buffer */

    BDSP_AF_P_sDRAM_BUFFER      sSpdifUserConfigAddr[BDSP_AF_P_MAX_NUM_SPDIF_PORTS];

} BDSP_CIT_P_VideoCITOutput;

/*---------------------------------------------------------*/
/* Prototype Definition for Video CIT Genreation functions */
/*---------------------------------------------------------*/

void BDSP_P_AnalyseVideoCit(
                        BMEM_Handle                 hHeap,
                        BDSP_VF_P_uTASK_CONFIG      *puCit,
                        BDSP_AlgorithmType          eAlgorithm
                      );

typedef struct BDSP_CIT_P_ScmCITOutput
{
        BDSP_SCM_P_sTASK_CONFIG         sScmCit;                /* SCM Cit Structure */

        uint32_t                                        ui32NumBranches;

        BDSP_CIT_P_OpBranchInfo         sCitBranchInfo[BDSP_P_MAX_FW_BRANCH_PER_FW_TASK];


}BDSP_CIT_P_ScmCITOutput;

/*---------------------------------------------------------------------
            Top level SCM CIT Generation Function
---------------------------------------------------------------------*/




void BDSP_P_AnalyseScmCit(  BMEM_Handle             hHeap,
                                    BDSP_SCM_P_sTASK_CONFIG *psScmCit
                                );

#endif
