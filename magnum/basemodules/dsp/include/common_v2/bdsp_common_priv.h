/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BDSP_COMMON_PRIV_H_
#define BDSP_COMMON_PRIV_H_

#include "bdsp_common_priv_include.h"

#define BDSP_P_INVALID_TASK_ID            ((unsigned)(-1))
#define BDSP_MAX_INDEPENDENT_DELAY_IN_MS      500
#define BDSP_EVENT_TIMEOUT_IN_MS              500

#define BDSP_MAX_DECODE_CTXT           4
#define BDSP_MAX_PASSTHROUGH_CTXT      4
#define BDSP_MAX_ENCODE_CTXT           2
#define BDSP_MAX_ECHOCANCELLER_CTXT    1
#define BDSP_MAX_AUDIO_PROCESSING_CTXT 1     /* All PP are downloaded onetime hence kept as 1 */
#define BDSP_MAX_MIXER_CTXT            2     /* We currently have only 2 types of mixers */

#define BDSP_FIFO_0_INDEX              0
#define BDSP_FIFO_INVALID             ((unsigned int)(-1))

#define BDSP_P_SET_BIT(A,B)   (A |= ((uint32_t)0x00000001 << B))
#define BDSP_P_CLEAR_BIT(A,B) (A &= ~((uint32_t)0x00000001 << B))

typedef struct BDSP_P_TaskInfo
{
    bool    taskId[BDSP_MAX_FW_TASK_PER_DSP];
    void    *pTask[BDSP_MAX_FW_TASK_PER_DSP];
    unsigned numActiveTasks;
}BDSP_P_TaskInfo;

typedef struct BDSP_P_TaskSchedulingInfo{
	bool     supported;
	unsigned schedulingLevel;
	unsigned schedulingThreshold;
}BDSP_P_TaskSchedulingInfo;

typedef struct BDSP_P_SystemSchedulingInfo{
	unsigned numSchedulingLevels;
	unsigned numPreemptionLevels;
	BDSP_P_TaskSchedulingInfo sTaskSchedulingInfo[BDSP_P_TaskType_eLast];
}BDSP_P_SystemSchedulingInfo;

typedef struct BDSP_P_InterTaskBufferDetails{
    BDSP_P_MsgQueueParams   BuffersDetails;
    BDSP_P_MsgQueueHandle   hQueueHandle; /* will be used in case of RDB*/
    BDSP_P_BufferPointer    BufferPointer;/* will be used incase of Dram*/
}BDSP_P_InterTaskBufferDetails;

BDBG_OBJECT_ID_DECLARE(BDSP_P_InterTaskBuffer);
typedef struct BDSP_P_InterTaskBuffer
{
    BDBG_OBJECT(BDSP_P_InterTaskBuffer)
    BDSP_InterTaskBuffer interTaskBuffer;

    BDSP_Context            *pContext;
    bool                     inUse; /* Flag to indicate if the intertask buffer is in use */
    BDSP_AF_P_BufferType     ebufferType;
    BDSP_DataType            dataType; /* Type of data in the inter task buffer */
    BDSP_AF_P_DistinctOpType distinctOp; /* Distinct output type of the intertask buffer */
    BDSP_Stage              *srcHandle; /* Source Stage handle */
    BDSP_Stage              *dstHandle; /* Destination Stage Handle */
    int32_t                  srcIndex; /* Index of the src Stage output to which the inter task buffer is connected */
    int32_t                  dstIndex; /* Index of the dst Stage input to which the inter task buffer is conneceted */
    unsigned                 dspIndex;

    unsigned                 numChannels; /* Number of channels in the intertask buffer */
    unsigned                 numTocData;
    unsigned                 numMetaData;
    unsigned                 numObjectData;
    BDSP_P_InterTaskBufferDetails  PcmDetails[BDSP_AF_P_MAX_CHANNELS];
    BDSP_P_InterTaskBufferDetails  TocDetails[BDSP_AF_P_MAX_TOC];
    BDSP_P_InterTaskBufferDetails  MetaDataDetails[BDSP_AF_P_MAX_METADATA];
    BDSP_P_InterTaskBufferDetails  ObjectDataDetails[BDSP_AF_P_MAX_OBJECTDATA];
    BDSP_P_MemoryPool        MemoryPool;
    dramaddr_t               bufferDescriptorAddr[BDSP_AF_P_MAX_PORT_BUFFERS][BDSP_AF_P_MAX_CHANNELS];
    bool                     descriptorAllocated;
} BDSP_P_InterTaskBuffer;

typedef struct BDSP_P_InterStagePortInfo{
    BDSP_AF_P_DistinctOpType ePortDataType;
	dramaddr_t bufferDescriptorAddr[BDSP_AF_P_MAX_PORT_BUFFERS][BDSP_AF_P_MAX_CHANNELS];
	BDSP_AF_P_Port_sDataAccessAttributes dataAccessAttributes;
	unsigned branchFromPort;
	unsigned tocIndex;
}BDSP_P_InterStagePortInfo;

typedef struct BDSP_P_StageConnectionInfo{
    unsigned                     numOutputs[BDSP_ConnectionType_eMax]; /* Outputs of each particular type */
    unsigned                     numInputs[BDSP_ConnectionType_eMax]; /* Inputs of each particular type */

	BDSP_P_ConnectionDetails 	 sStageInput[BDSP_AF_P_MAX_IP_FORKS];
	BDSP_P_ConnectionDetails 	 sStageOutput[BDSP_AF_P_MAX_OP_FORKS];

	BDSP_AF_P_DistinctOpType     eStageOpDataType[BDSP_AF_P_MAX_OP_FORKS];
	BDSP_P_InterStagePortInfo    sInterStagePortInfo[BDSP_AF_P_MAX_OP_FORKS];
}BDSP_P_StageConnectionInfo;

typedef struct BDSP_P_TaskParams
{
	bool 	 isRunning;
	bool 	 paused;
	bool	 frozen;
	unsigned taskId;
	unsigned masterTaskId;
	unsigned commandCounter;
	unsigned lastCommand;
	uint32_t eventEnabledMask;
}BDSP_P_TaskParams;

typedef struct BDSP_P_TaskMemoryInfo
{
	BDSP_P_MemoryPool			sMemoryPool;

	BDSP_P_HostBuffer			hostAsyncQueue;
	BDSP_P_FwBuffer 	  		sCITMemory;
	BDSP_P_FwBuffer 	  		sHostCITMemory;
	BDSP_P_FwBuffer				sSampleRateLUTMemory;
	BDSP_P_FwBuffer				sSchedulingConfigMemory;
	BDSP_P_FwBuffer				sGateOpenConfigMemory;
    BDSP_P_FwBuffer             sStcTriggerConfigMemory;

	BDSP_P_FwBuffer		  		sCacheHole; /* 512 bytes of hole for cache coherency */
	BDSP_P_MsgQueueParams       syncQueueParams;
	BDSP_P_MsgQueueParams       asyncQueueParams;
	BDSP_P_FwBuffer             sMPSharedMemory;
}BDSP_P_TaskMemoryInfo;

typedef struct BDSP_P_StageMemoryInfo
{
	BDSP_P_MemoryPool	  sMemoryPool;
	BDSP_P_FwBuffer       sInterframe;
	BDSP_P_FwBuffer 	  sAlgoUserConfig;
	BDSP_P_FwBuffer       sAlgoStatus;
	BDSP_P_FwBuffer       sIdsStatus;
	BDSP_P_FwBuffer       sTsmStatus;

	BDSP_P_FwBuffer		  sCacheHole; /* 512 bytes of hole for cache coherency */
	BDSP_P_FwBuffer 	  sDataSyncSettings;
	BDSP_P_FwBuffer 	  sTsmSettings;
	BDSP_P_FwBuffer       sHostAlgoUserConfig;
}BDSP_P_StageMemoryInfo;

BERR_Code BDSP_P_PopulateSystemSchedulingDeatils(
	BDSP_P_SystemSchedulingInfo *pSystemSchedulingInfo
);
void BDSP_P_ValidateCodeDownloadSettings(
    unsigned *pmaxAlgorithms
);
unsigned BDSP_P_GetFreeTaskId(
    BDSP_P_TaskInfo *pTaskInfo
);
void BDSP_P_ReleaseTaskId(
    BDSP_P_TaskInfo *pTaskInfo,
    unsigned        *taskId
);
void BDSP_P_GetDistinctOpTypeAndNumChans(
    BDSP_DataType dataType, /* [in] */
    unsigned *numChans, /* [out] */
    BDSP_AF_P_DistinctOpType *distinctOp /* [out] */
);
void BDSP_P_GetFreeInputPortIndex(
	BDSP_P_ConnectionDetails *psStageInput,
	unsigned *index
);
void BDSP_P_GetFreeOutputPortIndex(
	BDSP_P_ConnectionDetails *psStageOutput,
	unsigned *index
);
BERR_Code BDSP_P_InterframeRunLengthDecode(
	void *pDst,
	void *pSrc,
	uint32_t ui32EncodedSize,
	uint32_t ui32AllocatedBufferSize
);
void BDSP_P_GetInterStagePortIndex(
	BDSP_P_StageConnectionInfo *pStageConnectionInfo,
	BDSP_AF_P_DistinctOpType    eDistinctOpType,
	unsigned                   *pIndex
);
void BDSP_P_GetInterTaskPortIndex(
	BDSP_Stage                 *pStage,
	BDSP_P_StageConnectionInfo *pStageConnectionInfo,
	BDSP_AF_P_DistinctOpType    eDistinctOpType,
	unsigned                   *pIndex
);
#if !B_REFSW_MINIMAL
BERR_Code BDSP_P_GetDefaultDatasyncSettings(
        void *pDeviceHandle,
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
);
#endif /*!B_REFSW_MINIMAL*/
BERR_Code BDSP_P_GetDefaultTsmSettings(
        void *pDeviceHandle,
        void *pSettingsBuffer,        /* [out] */
        size_t settingsBufferSize   /*[In]*/
);
BERR_Code BDSP_P_CopyStartTaskSettings(
    BDSP_ContextType        contextType,
	BDSP_TaskStartSettings *pBDSPStartSettings,
	BDSP_TaskStartSettings *pAPPStartSettings
);
void BDSP_P_DeleteStartTaskSettings(
	BDSP_TaskStartSettings *pStartSettings
);
BERR_Code BDSP_P_PopulateSchedulingInfo(
    BDSP_TaskStartSettings  	*pTaskStartSettings,
	BDSP_ContextType         	 contextType,
	BDSP_P_SystemSchedulingInfo *pSystemSchedulingLevel,
	BDSP_P_StartTaskCommand 	*psCommand
);
void BDSP_P_InitBufferDescriptor(
    BDSP_P_BufferPointer    *pBufferPointer,
    BDSP_MMA_Memory         *pMemory,
    unsigned                 size
);
void BDSP_P_InitDramBuffer(
    BDSP_P_FwBuffer   *pDescriptorMemory,
    dramaddr_t         bufferDescriptorAddr,
    dramaddr_t         BaseOffset
);
#endif /*BDSP_COMMON_PRIV_H_*/
