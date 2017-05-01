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


#ifndef BDSP_ARM_PRIV_H_
#define BDSP_ARM_PRIV_H_

#include "blst_slist.h"

BDBG_OBJECT_ID_DECLARE(BDSP_Arm);
BDBG_OBJECT_ID_DECLARE(BDSP_ArmContext);
BDBG_OBJECT_ID_DECLARE(BDSP_ArmStage);
BDBG_OBJECT_ID_DECLARE(BDSP_ArmTask);

#define BDSP_ARM_NUM_DSP                          1

#define BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE         ((BDSP_ARM_MAX_FW_TASK_PER_DSP*2)+2) /* max 4 task with 2 queues each and 1 each cmd and Resp Q for system */

#define BDSP_ARM_EVENT_TIMEOUT_IN_MS               400
#define BDSP_ARM_START_STOP_EVENT_TIMEOUT_IN_MS    400

#define BDSP_ARM_GET_TASK_INDEX(taskId) (taskId - BDSP_ARM_TASK_ID_START_OFFSET)

#define BDSP_Arm_P_LookupAlgorithmInfo BDSP_Arm_P_LookupAlgorithmInfo_isrsafe

#define BDSP_ARM_P_ALGORITHM_SUPPORTED(a) (BDSP_Arm_P_LookupAlgorithmInfo_isrsafe(a)->supported)
#define BDSP_ARM_P_ALGORITHM_NAME(a)      (BDSP_Arm_P_LookupAlgorithmInfo_isrsafe(a)->pName)
#define BDSP_ARM_P_ALGORITHM_TYPE(a)      (BDSP_Arm_P_LookupAlgorithmInfo_isrsafe(a)->type)

typedef struct BDSP_ArmDspApp{
    BTEE_InstanceHandle hBteeInstance;
    BTEE_ClientHandle hClient;
    BTEE_ApplicationHandle hApplication;
    BTEE_ConnectionHandle hConnection;

    /* Handle for HBC */
    BTEE_ApplicationHandle hHbcApplication;
    BTEE_ConnectionHandle hHbcConnection;

    /* msg lock and count */
    BKNI_MutexHandle    msgLock;

}BDSP_ArmDspApp;

/*************************************************************************
Summary:
       Data structure details for the message queue in the system memory

Description:
    Parmeters passed in this structure:-
        Handle for the heap of memory  to be allocated
        Base and End Addresses of the message queue (local copy)
        Read pointer and write pointer addresses (local copy)required for operations like writing a message and reading a message
        The message queue attribute address containing the base address of the message queue needed to be passed to the shared copy in DRAM
        The MSB toggle bits for both the read and write pointers to determine wrap around conditions in the message queue
***************************************************************************/
typedef struct BDSP_Arm_P_MsgQueue
{
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *psQueuePointer;
    int32_t  MsgQueueHandleIndex;     /*Message queue Handle Index for this message queue */
    BDSP_MMA_Memory Memory;
    uint32_t ui32Size;                /* Size of the Buffer */
} BDSP_Arm_P_MsgQueue;

typedef struct BDSP_Arm_P_MsgQueue *BDSP_Arm_P_MsgQueueHandle;

typedef struct BDSP_Arm_P_TaskInfo
{
    bool    taskId[BDSP_ARM_MAX_FW_TASK_PER_DSP];
    struct BDSP_ArmTask    *pArmTask[BDSP_ARM_MAX_FW_TASK_PER_DSP];
    BKNI_MutexHandle taskIdMutex;
    unsigned numActiveTasks;
}BDSP_Arm_P_TaskInfo;

typedef struct BDSP_Arm_P_MemoryGrant
{
    BDSP_Arm_P_MsgQueueParams    cmdQueueParams; /* Command queue per DSP */
    BDSP_Arm_P_MsgQueueParams    genRspQueueParams;  /* Generic (non-task) response queue per DSP */
    BDSP_Arm_P_CommonStageMemory sScratchandISBuff; /* Open time scratch and IS memory info */
    BDSP_P_FwBuffer              sMapTable;
    BDSP_Arm_P_DwnldMemInfo       sDwnldMemInfo;
}BDSP_Arm_P_MemoryGrant;

typedef struct BDSP_Arm_P_HbcInfo
{
    uint32_t hbcValid;
    uint32_t hbc;

}BDSP_Arm_P_HbcInfo;

typedef struct BDSP_Arm_P_InterfaceQueue{
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER *parmInterfaceQHndl; /*[BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE];*/ /* Stores the physical address of Interface Queue Handle */
    BDSP_MMA_Memory  Memory; /*[BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE];*/ /* Stores the physical address of Interface Queue Handle */
}BDSP_Arm_P_InterfaceQueue;

typedef struct BDSP_Arm_P_Hbc{
    BDSP_Arm_P_HbcInfo  *psHbcInfo;
    BDSP_MMA_Memory     Memory;
}BDSP_Arm_P_Hbc;

typedef struct BDSP_Arm
{
    BDBG_OBJECT(BDSP_Arm)
    BDSP_Device device;
    BDSP_ArmDspApp armDspApp;
    BDSP_ArmSettings settings;
    BCHP_Handle chpHandle;
    BREG_Handle regHandle;
    BMMA_Heap_Handle memHandle;
    BINT_Handle intHandle;
    BKNI_EventHandle hDeviceEvent;
    size_t fwHeapSize;
    void *pFwHeapMemory;
    dramaddr_t FwHeapOffset;
    BDSP_ArmImgCacheEntry imgCache[BDSP_ARM_IMG_ID_MAX];
    BLST_S_HEAD(BDSP_ArmContextList, BDSP_ArmContext) contextList;
    BDSP_Arm_P_TaskInfo taskDetails;
    BDSP_Arm_P_MemoryGrant memInfo;
    bool armIntrfcQHndlFlag[BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE];
    BDSP_Arm_P_InterfaceQueue  sArmInterfaceQ; /*[BDSP_ARM_NUM_INTERFACE_QUEUE_HANDLE];*/ /* Stores the physical address of Interface Queue Handle */
    BDSP_Arm_P_MsgQueueHandle hCmdQueue;         /* Cmd queue handle*/
    BDSP_Arm_P_MsgQueueHandle hGenRspQueue;      /* Generic Response queue handle*/
    bool                    deviceWatchdogFlag;
    BKNI_MutexHandle armInterfaceQHndlMutex;
    BKNI_MutexHandle captureMutex;
    BKNI_MutexHandle    watchdogMutex;
    BDSP_Arm_MapTableEntry       sDeviceMapTable[BDSP_ARM_MAX_ALLOC_DEVICE];
    BDSP_Arm_P_Hbc              HbcInfo;
}BDSP_Arm;

typedef struct BDSP_ArmContext
{
    BDBG_OBJECT(BDSP_ArmContext)
    BDSP_Context context;
    struct BDSP_Arm *pDevice;
    BDSP_ContextCreateSettings settings;
    /*BDSP_ContextInterruptHandlers interruptHandlers;*/
    BLST_S_ENTRY(BDSP_ArmContext) node;
    BLST_S_HEAD(BDSP_ArmAllocTaskList, BDSP_ArmTask) allocTaskList;
    BLST_S_HEAD(BDSP_ArmFreeTaskList, BDSP_ArmTask) freeTaskList;
    /*BDSP_Arm_P_ContextMemoryGrant  contextMemInfo;*/
    bool                    contextWatchdogFlag;
}BDSP_ArmContext;

typedef struct BDSP_Arm_P_TaskCallBacks
{
    BINT_CallbackHandle  hDSPAsync;/* DSP Async Interrupt callback */
    BINT_CallbackHandle  hDSPSync; /* DSP SYNC Interrupt callback */
}BDSP_Arm_P_TaskCallBacks;

 typedef struct BDSP_ArmTask
{
    BDBG_OBJECT(BDSP_ArmTask)
    BLST_S_ENTRY(BDSP_ArmTask) node;

    BDSP_Task task;
    struct BDSP_ArmContext *pContext;
    BDSP_TaskCreateSettings settings;
    BDSP_TaskStartSettings startSettings;

    bool allocated;
    bool isStopped;  /* TRUE : If the stop command for this task has been sent,
                                           But the Processing of stop is still under process. Keep it in true state untill the hDspCh is started*/

    unsigned int    taskId;
    unsigned int    lastEventType;
    unsigned int    masterTaskId;

    unsigned commandCounter;
    BKNI_EventHandle hEvent;
    BDSP_Arm_P_TaskCallBacks    interruptCallbacks;

    BDSP_ARM_CIT_P_Output   citOutput;
    BDSP_ARM_CTB_Output     ctbOutput;
    BDSP_Arm_P_MsgQueueHandle hAsyncMsgQueue; /* Asynchronous message queue belonging to this task */
    BDSP_Arm_P_MsgQueueHandle hSyncMsgQueue; /* Synchronous message queue belonging to this task */
    BDSP_Arm_P_TaskMemoryInfo taskMemGrants; /* Memory for contiguous Async Msgs */
    uint32_t        eventEnabledMask;        /* Contains information abt. event ids already enabled */
    BDSP_TaskSchedulingMode schedulingMode;
    BDSP_MMA_Memory FeedbackBuffer;             /* Feedback buffer between Tasks(Master writes-Slaves read) */
    BDSP_Arm_MapTableEntry       sTaskMapTable[BDSP_ARM_MAX_ALLOC_TASK];
} BDSP_ArmTask;

typedef struct BDSP_ArmStage
{
    BDBG_OBJECT(BDSP_ArmStage)
    BDSP_Stage stage;

    BDSP_ArmContext *pContext;
    BDSP_ArmTask    *pArmTask;

    BDSP_Algorithm algorithm;
    BDSP_StageCreateSettings settings;

    unsigned ui32StageId;/*stage ID in for this branch id; can be used for filling in node network*/
    unsigned ui32BranchId;/*stores the branch information during traversing; can be used for filling in node network*/

    /*Alloc these buffers during Stage create*/
    BDSP_P_FwBuffer  sDramUserConfigBuffer;
    BDSP_P_FwBuffer  sDramInterFrameBuffer;
    BDSP_P_FwBuffer  sDramStatusBuffer;

    /* The offsets to the interframe, status and user cfg buffer for the framsync node */
    BDSP_P_AlgoBufferOffsets     sFrameSyncOffset;

    /* Extra buffer to on-the-fly program cfg params */
    BDSP_P_FwBuffer  sDramUserConfigSpareBuffer;

    unsigned                     totalInputs;
    unsigned                     totalOutputs; /* Outputs of any type */
    unsigned                     numOutputs[BDSP_ConnectionType_eMax]; /* Outputs of each particular type */
    unsigned                     numInputs[BDSP_ConnectionType_eMax]; /* Inputs of each particular type */

    BDSP_StageSrcDstDetails      sStageInput[BDSP_AF_P_MAX_IP_FORKS];
    BDSP_StageSrcDstDetails      sStageOutput[BDSP_AF_P_MAX_OP_FORKS];
    BDSP_StageSrcDstDetails      sIdsStageOutput;

    BDSP_AF_P_DistinctOpType     eStageOpBuffDataType[BDSP_AF_P_MAX_OP_FORKS];

    /*uint32_t StageMemReq;*/
    BDSP_P_StageMemoryReqd       stageMemInfo;

    bool running; /* Flag to indicate if the stage is a part of running task */
    BDSP_Arm_MapTableEntry       sStageMapTable[BDSP_ARM_MAX_ALLOC_STAGE];

    /* Capture information for the stage */
    /*BLST_S_HEAD(BDSP_ArmCaptureList, BDSP_ArmCapture) captureList;*/
}BDSP_ArmStage;

/* ======= APIs ========== */
void BDSP_Arm_P_Close(
    void *pDeviceHandle);


BERR_Code BDSP_Arm_P_DownloadSystemCodeToAstra(
	BTEE_ClientHandle hClient,
	BDSP_Arm *pDevice,
	BDSP_Arm_SystemImgId ImgId);


BERR_Code BDSP_Arm_P_DownloadFwToAstra(
	BTEE_ClientHandle hClient,
	BDSP_Arm *pDevice,
	BDSP_ARM_AF_P_AlgoId AlgoId);

BERR_Code BDSP_Arm_P_StartHbcMonitor(
    BDSP_Arm *pDevice);

BERR_Code BDSP_Arm_P_Open(
    void *pDeviceHandle);

void BDSP_Arm_P_GetStatus(
    void *pDeviceHandle,
    BDSP_Status *pStatus
    );

void BDSP_Arm_P_GetAlgorithmInfo(
    BDSP_Algorithm algorithm,
    BDSP_AlgorithmInfo *pInfo /* [out] */
    );

void BDSP_Arm_P_GetAlgorithmDefaultSettings(
    BDSP_Algorithm algorithm,
    BDSP_MMA_Memory *pMemory,
    size_t settingsBufferSize
    );

void BDSP_Arm_P_GetDefaultContextSettings(
    void *pDeviceHandle,
    BDSP_ContextType contextType,
    BDSP_ContextCreateSettings *pSettings);

BERR_Code BDSP_Arm_P_CreateContext(
    void *pDeviceHandle,
    const BDSP_ContextCreateSettings *pSettings,
    BDSP_ContextHandle *pContextHandle);

void BDSP_Arm_P_DestroyContext(
    void *pContextHandle);

void BDSP_Arm_P_GetDefaultTaskSettings(
    void *pContextHandle,
    BDSP_TaskCreateSettings *pSettings);

BERR_Code BDSP_Arm_P_CreateTask(
    void *pContextHandle,
    const BDSP_TaskCreateSettings *pSettings,
    BDSP_TaskHandle *pTask);

void BDSP_Arm_P_DestroyTask(
    void *pTaskHandle);

BERR_Code BDSP_Arm_P_ProcessWatchdogInterrupt(
    void *pContextHandle);


BERR_Code BDSP_Arm_P_PopulateGateOpenFMMStages(
    void *pPrimaryStageHandle,
    BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG *sTaskFmmGateOpenConfig,
    uint32_t ui32MaxIndepDelay
    );

BERR_Code BDSP_Arm_P_RetrieveGateOpenSettings(
    void *pTaskHandle,
    BDSP_TaskGateOpenSettings *pSettings   /* [out] */
    );

void BDSP_Arm_P_GetDefaultTaskStartSettings(
    void *pTaskHandle,
    BDSP_TaskStartSettings *pSettings    /* [out] */
    );

BERR_Code BDSP_Arm_P_StartTask(
    void *pTaskHandle,
    BDSP_TaskStartSettings *pStartSettings    /* [out] */
    );

BERR_Code BDSP_Arm_P_StopTask(
    void *pTaskHandle
    );

void BDSP_Arm_P_GetDefaultStageCreateSettings(
    BDSP_AlgorithmType algoType,
    BDSP_StageCreateSettings *pSettings /* [out] */
    );

BERR_Code BDSP_Arm_P_CreateStage(
    void *pContextHandle,
    const BDSP_StageCreateSettings *pSettings,
    BDSP_StageHandle *pStageHandle /* [out] */
    );

void BDSP_Arm_P_DestroyStage(
    void *pStageHandle
    );

BERR_Code BDSP_Arm_P_GetStageSettings(
    void *pStageHandle,
    void *pSettingsBuffer,
    size_t settingsSize);

BERR_Code BDSP_Arm_P_SetStageSettings(
    void *pStageHandle,
    const void *pSettingsBuffer,
    size_t settingsSize);

BERR_Code BDSP_Arm_P_GetDatasyncSettings(
    void *pStageHandle,
    BDSP_AudioTaskDatasyncSettings *pSettings);

BERR_Code BDSP_Arm_P_GetDatasyncSettings_isr(
    void *pStageHandle,
    BDSP_AudioTaskDatasyncSettings *pSettings);

BERR_Code BDSP_Arm_P_SetDatasyncSettings_isr(
    void *pStageHandle,
    const BDSP_AudioTaskDatasyncSettings *pSettings);

BERR_Code BDSP_Arm_P_SetDatasyncSettings(
    void *pStageHandle,
    const BDSP_AudioTaskDatasyncSettings *pSettings);

BERR_Code BDSP_Arm_P_SetAlgorithm(
    void *pStageHandle,
    BDSP_Algorithm algorithm);

BERR_Code   BDSP_Arm_P_CreateTaskQueues(void *pTaskHandle);

BERR_Code   BDSP_Arm_P_DestroyTaskQueues(void *pTaskHandle);

BERR_Code BDSP_Arm_P_SendCitReconfigCommand(BDSP_ArmTask *pArmTask);

BERR_Code BDSP_Arm_P_AddInterTaskBufferInput(
    void *pStageHandle,
    BDSP_DataType dataType,
    const BDSP_InterTaskBuffer *pBufferHandle,
    unsigned *pInputIndex
    );

BERR_Code BDSP_Arm_P_AddInterTaskBufferOutput(
    void *pStageHandle,
    BDSP_DataType dataType,
    const BDSP_InterTaskBuffer *pBufferHandle,
    unsigned *pOutputIndex
    );

BERR_Code BDSP_Arm_P_AddFmmOutput(
    void *pStageHandle,
    BDSP_DataType dataType,
    const BDSP_FmmBufferDescriptor *pDescriptor,
    unsigned *pOutputIndex
    );

#ifdef BDSP_QUEUE_DEBUG
BERR_Code BDSP_Arm_P_AddQueueOutput(
    void     *pStageHandle,
    void     *pQueueHandle,
    unsigned *pOutputIndex /* [out] */
    );
#endif /* BDSP_QUEUE_DEBUG */
BERR_Code BDSP_Arm_P_AddOutputStage(
    void *pSrcStageHandle,
    BDSP_DataType dataType,
    void *pDstStageHandle,
    unsigned *pSourceOutputIndex,
    unsigned *pDestinationInputIndex
    );

void BDSP_Arm_P_RemoveInput(
    void *pStageHandle,
    unsigned inputIndex);

void BDSP_Arm_P_RemoveAllInputs(
    void *pStageHandle);

void BDSP_Arm_P_RemoveOutput(
    void *pStageHandle,
    unsigned outputIndex);

void BDSP_Arm_P_RemoveAllOutputs(
    void *pStageHandle);

BERR_Code BDSP_Arm_P_SendUnMapCommand(
    void *pDeviceHandle,
    BDSP_MAP_Table_Entry *pMapTableEntries,
    uint32_t ui32NumEntries);

BERR_Code BDSP_Arm_P_SendMapCommand(
    void *pDeviceHandle,
    BDSP_MAP_Table_Entry *pMapTableEntries,
    uint32_t ui32NumEntries);

/* Macro for traversing through the stages in a task */
#define BDSP_ARM_STAGE_TRAVERSE_LOOP_V1_BEGIN(A, B, C, D)                           \
    {                                                                           \
        BDSP_ArmStage *macroVarStackOfStages[BDSP_ARM_MAX_STAGES_PER_TASK];         \
        BDSP_ArmStage *B;                                                       \
        uint32_t    macroVarStackDepth, macroVari, macroVarNumStOps;            \
        uint32_t   macroVarBrId, macroVarStId, C, D;                            \
                                                                                \
        macroVarStackOfStages[0] = A;                                           \
        macroVarStackDepth = 1;                                                 \
        macroVarBrId = 0;                                                       \
        macroVarStId = -1;                                                      \
        while (macroVarStackDepth != 0)                                         \
        {                                                                       \
            macroVarStackDepth--;                                               \
            B = macroVarStackOfStages[macroVarStackDepth];                   \
            C = macroVarBrId;                                                   \
            D = macroVarStId++;                                                 \
            BDBG_OBJECT_ASSERT(B, BDSP_ArmStage);                             \

            /* Push all connected stage handles into the stack */
            /* For loop in reverse order as the stack is a lifo structure */
#define BDSP_ARM_STAGE_TRAVERSE_LOOP_END(B)                                         \
            macroVarNumStOps = 0;                                               \
            for(macroVari = BDSP_AF_P_MAX_OP_FORKS; macroVari >=1 ; macroVari--)\
            {                                                                   \
                if ((B->sStageOutput[macroVari-1].eConnectionType               \
                        == BDSP_ConnectionType_eStage)                          \
                    && (B->sStageOutput[macroVari-1].eNodeValid ==              \
                        BDSP_AF_P_eValid))                                      \
                {                                                               \
                    macroVarNumStOps++;                                         \
                    macroVarStackOfStages[macroVarStackDepth] = (BDSP_ArmStage *)B->sStageOutput[macroVari-1].connectionDetails.stage.hStage->pStageHandle; \
                    macroVarStackDepth++;                                       \
                    BDBG_ASSERT(macroVarStackDepth < 10);                       \
                }                                                               \
            }                                                                   \
                                                                                \
            if (macroVarNumStOps == 0)                                          \
            {                                                                   \
                macroVarBrId++;                                                 \
                macroVarStId = -1;                                              \
            }                                                                   \
        }                                                                       \
    }

#define BDSP_ARM_STAGE_TRAVERSE_LOOP_BEGIN(A, B)                                    \
BDSP_ARM_STAGE_TRAVERSE_LOOP_V1_BEGIN(A, B, macroBrId, macroStId)


/*extern  const BDSP_ARM_VOM_Algo_Start_Addr                      BDSP_ARM_sAlgoStartAddr;*/
extern  const BDSP_AF_P_sNODE_INFO                          BDSP_ARM_sNodeInfo [BDSP_ARM_AF_P_AlgoId_eMax+1];
#endif /* BDSP_ARM_PRIV_H_ */
