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


#ifndef BDSP_RAAGA_MM_PRIV_H_
#define BDSP_RAAGA_MM_PRIV_H_
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/TB.h"
#include "bdsp_raaga_fwinterface_priv.h"
#include "bdsp_common_mm_priv.h"

#define BDSP_RAAGA_MAX_MSGS_PER_QUEUE     10
#define BDSP_RAAGA_MAX_ASYNC_MSGS_PER_QUEUE     40
#define BDSP_RAAGA_REALVIDEO_MAX_MSGS_PER_QUEUE   32
#define BDSP_RAAGA_CC_DATA_MSGS_PER_QUEUE   1200


#ifdef BDSP_AUDIODESC_SUPPORT
#define BDSP_RAAGA_AUDDESC_TASK 1
#else
#define BDSP_RAAGA_AUDDESC_TASK 0
#endif

#define BDSP_RAAGA_CAPTURE_TASK 1

/* To support 1 MS-11 usage + dual seamless transcods usage cases - 8 tasks */
/* To support 6 seamless transcodes - 12 tasks */
#define BDSP_RAAGA_MAX_FW_TASK_PER_CTXT (int32_t)12

#define BDSP_RAAGA_MAX_FW_TASK_PER_DSP (int32_t)12  /*There are 39 FIFO registers avalailable now, 1 is used for
debug message, 1 is used for Global command and 1 for generic response. So upto 18 tasks can be supported. */



#define BDSP_RAAGA_INVALID_DRAM_ADDRESS         0xFFFFFFFF

/* Note that the local number of buffers allocated is dependent on these defines. Right now the max locations per algo type has been kept as 8
BDSP_Raaga_P_DwnldBufUsageInfo DwnldBufUsageInfo[BDSP_RAAGA_MAX_DWNLD_BUFS] */

#define BDSP_RAAGA_MAX_AUD_PROCESSING_CTXT 1                    /* all the audio processing algos are downloaded always. So it is just one big chunk for the cumulative size */
#define BDSP_RAAGA_MAX_DECODE_CTXT 4                       /* If mixer task is not counted then only 6 */
#define BDSP_RAAGA_MAX_ENCODE_CTXT 2
#define BDSP_RAAGA_MAX_ECHOCANCELLER_CTXT 1
#define BDSP_RAAGA_MAX_MIXER_CTXT   4
#define BDSP_RAAGA_MAX_PASSTHROUGH_CTXT 4

#define BDSP_RAAGA_MAX_VIDEODECODE_CTXT 1
/* settings for video encoder on Raaga */
#define BDSP_RAAGA_MAX_VIDEOENCODE_CTXT                 1
#define BDSP_RAAGA_MAX_FW_TASK_PER_VIDEO_ENCODE_CTXT    1
#define BDSP_RAAGA_MAX_SCM_CTXT                 (int32_t)1
#define BDSP_RAAGA_MAX_FW_TASK_PER_SCM_CTXT     (int32_t)1

/* MAX_DWNLD_BUFS will be the max of the above defines.I have kept it slightly more  */
#define BDSP_RAAGA_MAX_DWNLD_BUFS BDSP_RAAGA_MAX_DECODE_CTXT + BDSP_RAAGA_MAX_ENCODE_CTXT

/* Memory Block */
typedef struct BDSP_Raaga_P_MemBlock
{
    void     *pAddr;
    uint32_t    ui32Size;
}BDSP_Raaga_P_MemBlock;

typedef struct BDSP_Raaga_P_DwnldBufUsageInfo
{
    void *pAddr;
    BDSP_Algorithm algorithm;
    int32_t numUser;
    bool bIsExistingDwnldValid;
}BDSP_Raaga_P_DwnldBufUsageInfo;


typedef struct BDSP_Raaga_P_AlgoTypeImgBuf
{
    uint32_t                ui32Size;
    /* Max of existing defines is enough, but keeping 4+2 = 6 buffers for now. */
    BDSP_Raaga_P_DwnldBufUsageInfo DwnldBufUsageInfo[BDSP_RAAGA_MAX_DWNLD_BUFS];
}BDSP_Raaga_P_AlgoTypeImgBuf;



/***************************************************************************
Summary:
    This structure contains the Buffer Properties.
***************************************************************************/
typedef struct BDSP_Raaga_P_DwnldMemInfo
{
    bool IsImagePreLoaded;              /* To distinguish between all binary download & the one limited by max number of instances*/
    uint32_t ui32TotalSupportedBinSize; /* Sum of all binary sizes */
    uint32_t ui32AllocatedBinSize;      /* Memory allocated for heap in this run */
    uint32_t ui32AllocwithGuardBand;        /* Memory allocated for heap in this run */
    void *pImgBuf;                      /* Start address of the complete image buf */
    BDSP_Raaga_P_AlgoTypeImgBuf AlgoTypeBufs[BDSP_AlgorithmType_eMax];
}BDSP_Raaga_P_DwnldMemInfo;

/* contains Task Sync & Async Queue info */
typedef struct BDSP_Raaga_P_TaskQueues
{
    BDSP_Raaga_P_MsgQueueParams    sTaskSyncQueue;     /* Synchronous queue */
    BDSP_Raaga_P_MsgQueueParams    sTaskAsyncQueue;    /* Asynchronous queue */
    BDSP_P_FwBuffer                sAsyncMsgBufmem;    /* Memory for Async */

    /*Video Task related Queues*/
    BDSP_Raaga_P_MsgQueueParams    sPDQueue;     /* PD queue */
    BDSP_Raaga_P_MsgQueueParams    sPRQueue;     /* PR queue */
    BDSP_Raaga_P_MsgQueueParams    sDSQueue;     /* DS queue */

    /* Video Encode Task related Queues. Could be done away with video task
    itself but then video decode and encode can't co-exist */
    BDSP_Raaga_P_RdbQueueParams    sRDQueue;     /* Raw picture delivery queue */
    BDSP_Raaga_P_MsgQueueParams    sRRQueue;     /* Raw picture release queue */
    BDSP_Raaga_P_MsgQueueParams    sCCDQueue;    /* CC data queue */

}BDSP_Raaga_P_TaskQueues;

/* This structure contains actual addresses & sizes per task */
typedef struct BDSP_Raaga_P_TaskMemoryInfo
{
    BDSP_Raaga_P_TaskQueues     sTaskQueue;
    BDSP_P_FwBuffer             sTaskInfo;
    BDSP_P_FwBuffer             sCitStruct;
    BDSP_P_FwBuffer             sSpareCitStruct; /* The working buffer for bdsp to populate
                                                        the updated cit on a seamless input port switch */
    BDSP_P_FwBuffer             sStackSwapBuff; /* Task stack swap space */
    BDSP_P_FwBuffer             sTaskCfgBufInfo;    /*All global task configurations come in here*/

    BDSP_P_FwBuffer             sTaskGateOpenBufInfo;    /*All Gate Open Buffer details for Configuration come in here*/

    BDSP_P_FwBuffer             sTaskSrcIoBufInfo[BDSP_MAX_INPUTS_PER_TASK];  /* 2 Input FMM/RAVE, non IS buffer   */
    BDSP_P_FwBuffer             sTaskDstIoBufInfo[BDSP_MAX_OUTPUTS_PER_TASK];  /* 8 Outputs FMM/RAVE, non IS buffer   */

    BDSP_P_FwBuffer             sTaskISIoBufInfo[BDSP_RAAGA_MAX_BRANCH_PER_TASK];  /* 2 Input FMM/RAVE, IS buffer   */
    BDSP_P_FwBuffer             sTaskISIoGenBufInfo[BDSP_RAAGA_MAX_BRANCH_PER_TASK];    /* 8 Outputs FMM/RAVE, IS buffer   */
}BDSP_Raaga_P_TaskMemoryInfo;




/* This structure contains iframe & cfgparams buffer memory req per task. This
   memory pool size is calculated as the worst case. This can be per DSP. This
   can be extended later to not to use the worst case size. */
typedef struct BDSP_Raaga_P_TaskMemoryRequirement
{
    uint32_t    ui32IframeCommon;   /* Size of Iframe of common exec */
    uint32_t    ui32IFramePAlgo;    /* Size of Iframe of Processing Algo */
    uint32_t    ui32CfgBufCommon;   /* Size of ConfigParams of common exec */
    uint32_t    ui32CfgBufPAlgo;    /* Size of ConfigParams of Processing Algo*/
    uint32_t    ui32StatusBufCommon;/* Size of Status buffer of common exec */
    uint32_t    ui32StatusBufPAlgo; /* Size of Status buffer of Processing Algo*/
    uint32_t    ui32CBitBuf;        /* Buffer for the CBIT Information */
    uint32_t    ui32ExtraBuf ;      /* Extra buffer required for
                                                - Stack Swap
                                                - Task Port Config
                                                - Task FMM Gate */
}BDSP_Raaga_P_TaskMemoryRequirement;



typedef struct BDSP_Raaga_P_PerDSPMemory
{
    unsigned                        InUse[BDSP_RAAGA_MAX_BRANCH];
    unsigned                        ui32GenericBufferStructAddr[BDSP_RAAGA_MAX_BRANCH];
    unsigned                        ui32IoBuffCfgStructAddr[BDSP_RAAGA_MAX_BRANCH];
    BDSP_AF_P_sIO_BUFFER            InterStageIOBuff[BDSP_RAAGA_MAX_BRANCH];
    BDSP_AF_P_sIO_GENERIC_BUFFER    InterStageIOGenericBuff[BDSP_RAAGA_MAX_BRANCH];

    BDSP_AF_P_sDRAM_BUFFER          ui32DspScratchMemGrant;

}BDSP_Raaga_P_PerDSPMemory;




/* Memory allocated information for whole Raptor device */
typedef struct BDSP_Raaga_P_MemoryGrant
{
    BDSP_Raaga_P_MsgQueueParams    cmdQueueParams[BDSP_RAAGA_MAX_DSP];
                                /* Command queue per DSP */

    BDSP_Raaga_P_MsgQueueParams    genRspQueueParams[BDSP_RAAGA_MAX_DSP];
                                /* Generic (non-task) response queue per DSP */

    void *pDSPFifoAddrStruct[BDSP_RAAGA_MAX_DSP];   /*Dram buffer to store addresses of DSP Fifo's used for message queues*/

    BDSP_Raaga_P_MemBlock pFwDebugBuf[BDSP_RAAGA_MAX_DSP][BDSP_Raaga_DebugType_eLast];
    /* Structure used to initialise the libDspControl module  */
    DSP                   sLibDsp;
    TB                    sTbTargetPrint[BDSP_RAAGA_MAX_DSP]; /* TB support structure for targetprint */
    unsigned char         *pTargetPrintBuffer[BDSP_RAAGA_MAX_DSP];
    /* DRAM buffer for raaga system to swap its data memory with */
    BDSP_Raaga_P_MemBlock   sRaagaSwapMemoryBuf[BDSP_RAAGA_MAX_DSP];

    BDSP_Raaga_P_PerDSPMemory          sScratchandISBuff[BDSP_RAAGA_MAX_DSP];
                                /* Open time scratch and IS memory info */
    BDSP_Raaga_P_DwnldMemInfo       sDwnldMemInfo;

}BDSP_Raaga_P_MemoryGrant;

typedef struct BDSP_Raaga_P_ContextMemoryGrant
{
    BDSP_P_FwBuffer            sDspScratchInfo[BDSP_RAAGA_MAX_DSP];
                                /* Scratch,interstage,interface buffers per DSP*/
                                /*will not be used with new CIT, allocated one time per DSP at Raaga_Open time*/
    BDSP_P_FwBuffer            sSpdifStatusBitBufInfo;
                                /* Extra buffer to on-the-fly program cfg params*/
    BDSP_P_FwBuffer            sVomTableInfo;
                                /* DRAM for VOM Table */
}BDSP_Raaga_P_ContextMemoryGrant;

/* Buffers required by CIT. This is to handle SW7346-598 */
typedef struct BDSP_Raaga_P_CitBuffers
{
    void            *psCitDataStructure;
    void            *psCitFwBranchInfo[BDSP_P_MAX_FW_BRANCH_PER_FW_TASK];
    void            *psCitPortCfg;
    void            *psCitTaskIsBuffCfg;
}BDSP_Raaga_P_CitBuffers;




BERR_Code BDSP_Raaga_P_AllocateInitMemory (
    void *pDeviceHandle
    );

BERR_Code BDSP_Raaga_P_FreeInitMemory(
    void *pDeviceHandle
    );

BERR_Code BDSP_Raaga_P_CalculateContextMemory(
    unsigned *pMemoryReq
    );

BERR_Code BDSP_Raaga_P_AllocateContextMemory (
    void *pContextHandle
    );

BERR_Code BDSP_Raaga_P_FreeContextMemory(
    void *pContextHandle
    );

BERR_Code BDSP_Raaga_P_CalculateInterTaskBufferMemory(
    unsigned *pMemoryReq,
    unsigned Numchannels
    );

BERR_Code BDSP_Raaga_P_CalculateTaskMemory(
    unsigned *pMemoryReq
    );

BERR_Code BDSP_Raaga_P_AllocateTaskMemory(
    void *pTaskHandle,
    const BDSP_TaskCreateSettings *pSettings
    );

BERR_Code BDSP_Raaga_P_FreeTaskMemory(
    void *pTaskHandle
    );

BERR_Code BDSP_Raaga_P_CalculateStageMemory(
    BDSP_AlgorithmType AlgorithmType,
    unsigned *pMemoryReq,
    const BDSP_RaagaUsageOptions *pUsage
    );

BERR_Code BDSP_Raaga_P_AllocateStageMemory(
    void *pStageHandle
    );

BERR_Code BDSP_Raaga_P_FreeStageMemory(
    void *pStageHandle
    );

BERR_Code BDSP_MM_P_CalcandAllocScratchISbufferReq(
    void *pDeviceHandle
    );

BERR_Code BDSP_MM_P_GetFwMemRequired(
    const BDSP_RaagaSettings  *pSettings,
    BDSP_Raaga_P_DwnldMemInfo *pDwnldMemInfo,      /*[out]*/
    void                      *pImgCache,
    bool                       UseBDSPMacro,
    const BDSP_RaagaUsageOptions *pUsage
    );

BERR_Code BDSP_Raaga_P_CalculateInitMemory (
    unsigned *pMemoryReq,
    unsigned NumDsp
);

BERR_Code BDSP_MM_P_CalcScratchAndISbufferReq_MemToolAPI(
        uint32_t *pui32ScratchMem,
        uint32_t *pui32InterstageIOMem,
        uint32_t *pui32InterstageIOGenMem,
        uint32_t *pui32Numch,
        const BDSP_RaagaUsageOptions *pUsage
);

BERR_Code BDSP_MM_P_CalcScratchAndISbufferReq(
        uint32_t *pui32ScratchMem,
        uint32_t *pui32InterstageIOMem,
        uint32_t *pui32InterstageIOGenMem,
        uint32_t *pui32Numch
    );

BERR_Code BDSP_MM_P_CalcStageMemPoolReq(
    void *pStageHandle
    );
BERR_Code BDSP_Raaga_P_AssignMem_DwnldBuf(void * pDeviceHandle,
                                            void *ptr);

void BDSP_Raaga_P_FreeFwExec(   void *pDeviceHandle);

#define BDSP_INIT_DWNLDBUF(pDwnldBuf) ((BDSP_Raaga_P_DwnldBufUsageInfo*)pDwnldBuf)->algorithm = BDSP_Algorithm_eMax;\
                               ((BDSP_Raaga_P_DwnldBufUsageInfo*)pDwnldBuf)->bIsExistingDwnldValid = false;\
                               ((BDSP_Raaga_P_DwnldBufUsageInfo*)pDwnldBuf)->numUser = 0;

#endif /*BDSP_RAAGA_MM_PRIV_H_*/