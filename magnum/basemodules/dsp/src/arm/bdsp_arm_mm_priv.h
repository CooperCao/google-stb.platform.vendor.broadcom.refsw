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


#ifndef BDSP_ARM_MM_PRIV_H_
#define BDSP_ARM_MM_PRIV_H_

#define BDSP_ARM_MAX_MSGS_PER_QUEUE           10
#define BDSP_ARM_MAX_ASYNC_MSGS_PER_QUEUE     40
#define BDSP_ARM_MAX_DECODE_CTXT 4
#define BDSP_ARM_MAX_ENCODE_CTXT 2
#define BDSP_ARM_MAX_ZERO_CTXT 0
#define BDSP_ARM_MAX_BRANCH   1

/* MAX_DWNLD_BUFS will be the max of the above defines.I have kept it slightly more  */
#define BDSP_ARM_MAX_DWNLD_BUFS BDSP_ARM_MAX_DECODE_CTXT + BDSP_ARM_MAX_ENCODE_CTXT

#define IF_ERR_GOTO_error if ( errCode ){\
                            errCode = BERR_TRACE(errCode);\
                            goto error;\
                        }
#define BDSP_ARM_INIT_DWNLDBUF(pDwnldBuf) ((BDSP_Arm_P_DwnldBufUsageInfo*)pDwnldBuf)->algorithm = BDSP_Algorithm_eMax;\
                               ((BDSP_Arm_P_DwnldBufUsageInfo*)pDwnldBuf)->bIsExistingDwnldValid = false;\
                               ((BDSP_Arm_P_DwnldBufUsageInfo*)pDwnldBuf)->numUser = 0;


typedef struct BDSP_ArmImgCacheEntry
{
    size_t size;            /* Image Size in bytes.  Populated once at startup time. */
    void  *pMemory;         /* Allocated memory (NULL if not resident) */
    uint32_t offset;        /* Physical address of allocated memory */
} BDSP_ArmImgCacheEntry;

typedef struct BDSP_Arm_MapTableEntry
{
    uint32_t offset;        /* Physical address of allocated memory */
    uint32_t size;          /* Size of the Allocated memory */
    bool     entry_valid;   /* Place Holder to Keep Track of old/New Entry */
    bool     map_valid;     /* False - FW doesn't hold the entry*/
    BDSP_ARM_AF_P_MemoryMapType eMemMapType;
} BDSP_Arm_MapTableEntry;

typedef struct BDSP_Arm_P_CommonStageMemory
{
    BDSP_P_IOBuffer                 InterStageIOBuff[BDSP_ARM_MAX_BRANCH];
    BDSP_P_IOGenBuffer              InterStageIOGenericBuff[BDSP_ARM_MAX_BRANCH];
    BDSP_P_FwBuffer                 DspScratchMemGrant;
}BDSP_Arm_P_CommonStageMemory;

/* SR_TBD:  This structure needs to be modified as per ARM requirement */
typedef struct BDSP_Arm_P_MsgQueueParams
{
    BDSP_MMA_Memory Queue;
    uint32_t uiMsgQueueSize;
} BDSP_Arm_P_MsgQueueParams;


typedef struct BDSP_Arm_P_DwnldBufUsageInfo
{
	BDSP_MMA_Memory Memory;
    BDSP_Algorithm algorithm;
    int32_t numUser;
    bool bIsExistingDwnldValid;
}BDSP_Arm_P_DwnldBufUsageInfo;



typedef struct BDSP_Arm_P_AlgoTypeImgBuf
{
    uint32_t                ui32Size;
    /* Max of existing defines is enough, but keeping 4+2 = 6 buffers for now. */
    BDSP_Arm_P_DwnldBufUsageInfo DwnldBufUsageInfo[BDSP_ARM_MAX_DWNLD_BUFS];
}BDSP_Arm_P_AlgoTypeImgBuf;


/***************************************************************************
Summary:
    This structure contains the Buffer Properties.
***************************************************************************/
typedef struct BDSP_Arm_P_DwnldMemInfo
{
    bool IsImagePreLoaded;              /* To distinguish between all binary download & the one limited by max number of instances*/
    uint32_t ui32TotalSupportedBinSize; /* Sum of all binary sizes */
    uint32_t ui32AllocatedBinSize;      /* Memory allocated for heap in this run */
    uint32_t ui32AllocwithGuardBand;        /* Memory allocated for heap in this run */
    BDSP_MMA_Memory ImgBuf;				/* Start address of the complete image buf */
    BDSP_Arm_P_AlgoTypeImgBuf AlgoTypeBufs[BDSP_AlgorithmType_eMax];
}BDSP_Arm_P_DwnldMemInfo;



typedef struct BDSP_Arm_P_TaskQueues
{
    BDSP_Arm_P_MsgQueueParams    sTaskSyncQueue;     /* Synchronous queue */
    BDSP_Arm_P_MsgQueueParams    sTaskAsyncQueue;    /* Asynchronous queue */
    BDSP_P_HostBuffer            sAsyncMsgBufmem;    /* Memory for Async */
}BDSP_Arm_P_TaskQueues;

/* This structure contains actual addresses & sizes per task */
typedef struct BDSP_Arm_P_TaskMemoryInfo
{
    BDSP_Arm_P_TaskQueues     sTaskQueue;
    BDSP_P_FwBuffer           sTaskInfo;
    BDSP_P_FwBuffer           sCitStruct;
    BDSP_P_FwBuffer           sSpareCitStruct; /* The working buffer for bdsp to populate
                                                        the updated cit on a seamless input port switch */
    BDSP_P_FwBuffer           sStackSwapBuff; /* Task stack swap space */
    BDSP_P_FwBuffer           sTaskCfgBufInfo;  /*All global task configurations come in here*/

    BDSP_P_FwBuffer           sTaskGateOpenBufInfo;    /*All Gate Open Buffer details for Configuration come in here*/

    BDSP_P_FwBuffer           sTaskSrcIoBufInfo[BDSP_MAX_INPUTS_PER_TASK];  /* 2 Input FMM/RAVE, non IS buffer   */
    BDSP_P_FwBuffer           sTaskDstIoBufInfo[BDSP_MAX_OUTPUTS_PER_TASK];  /* 8 Outputs FMM/RAVE, non IS buffer   */

    BDSP_P_FwBuffer           sTaskISIoBufInfo[BDSP_ARM_MAX_BRANCH];  /* 2 Input FMM/RAVE, IS buffer   */
    BDSP_P_FwBuffer           sTaskISIoGenBufInfo[BDSP_ARM_MAX_BRANCH];    /* 8 Outputs FMM/RAVE, IS buffer   */

}BDSP_Arm_P_TaskMemoryInfo;

BERR_Code BDSP_Arm_P_GetFwMemRequired(
        const BDSP_ArmSettings  *pSettings,
        BDSP_Arm_P_DwnldMemInfo *pDwnldMemInfo,      /*[out]*/
        void                    *pImg,
        bool                     UseBDSPMacro,
        const BDSP_UsageOptions *pUsage
);
void BDSP_Arm_P_CalculateInitMemory(
	unsigned *pMemoryReq
);
BERR_Code BDSP_Arm_P_AllocateInitMemory (
    void *pDeviceHandle
    );
BERR_Code BDSP_Arm_P_CalcandAllocScratchISbufferReq(void *pDeviceHandle);

void BDSP_Arm_P_FreeScratchISbuffer(
            void *pDeviceHandle
            );
BERR_Code BDSP_Arm_P_FreeInitMemory(
    void *pDeviceHandle
    );

BERR_Code BDSP_Arm_P_AllocateContextMemory(
    void *pContextHandle
    );

BERR_Code BDSP_Arm_P_FreeContextMemory(
    void *pContextHandle
    );

void BDSP_Arm_P_CalculateTaskMemory(
    unsigned *pMemoryReq
);

BERR_Code BDSP_Arm_P_AllocateTaskMemory(
    void *pTaskHandle,
    const BDSP_TaskCreateSettings *pSettings
    );

BERR_Code BDSP_Arm_P_FreeTaskMemory(
    void *pTaskHandle
    );
void BDSP_Arm_P_CalculateStageMemory(
    BDSP_AlgorithmType AlgorithmType,
    unsigned          *pMemoryReq,
    const BDSP_UsageOptions *pUsage
    );
BERR_Code BDSP_Arm_P_AllocateStageMemory(
    void *pStageHandle
    );

BERR_Code BDSP_Arm_P_FreeStageMemory(
    void *pStageHandle
    );

BERR_Code  BDSP_Arm_P_InsertEntry_MapTable(
	BDSP_Arm_MapTableEntry *pMapTable,
	BDSP_MMA_Memory *pMemory,
    uint32_t size,
    BDSP_ARM_AF_P_MemoryMapType eMapType,
    uint32_t ui32MaxEntry
    );

BERR_Code  BDSP_Arm_P_DeleteEntry_MapTable(
	BDSP_Arm_MapTableEntry *pMapTable,
	BDSP_MMA_Memory *pMemory,
    uint32_t ui32MaxEntry
    );

BERR_Code BDSP_Arm_P_RetrieveEntriesToMap(
    BDSP_Arm_MapTableEntry *pMapTable,
    BDSP_MAP_Table_Entry   *pUnMapArray,
    uint32_t *pui32NumEntries,
    uint32_t ui32MaxEntry
    );

BERR_Code BDSP_Arm_P_RetrieveEntriesToUnMap(
    BDSP_Arm_MapTableEntry *pMapTable,
    BDSP_MAP_Table_Entry   *pUnMapArray,
    uint32_t *pui32NumEntries,
    uint32_t ui32MaxEntry
    );

#endif  /* BDSP_ARM_MM_PRIV_H_ */
