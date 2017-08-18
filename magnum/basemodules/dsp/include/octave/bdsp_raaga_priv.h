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
#ifndef BDSP_RAAGA_PRIV_H_
#define BDSP_RAAGA_PRIV_H_

#include "bdsp_raaga_priv_include.h"

#define BDSP_RAAGA_MAX_DECODE_CTXT 		     4
#define BDSP_RAAGA_MAX_PASSTHROUGH_CTXT      4
#define BDSP_RAAGA_MAX_ENCODE_CTXT           2
#define BDSP_RAAGA_MAX_ECHOCANCELLER_CTXT    1
#define BDSP_RAAGA_MAX_AUDIO_PROCESSING_CTXT 1     /* All PP are downloaded onetime hence kept as 1 */
#define BDSP_RAAGA_MAX_MIXER_CTXT            2     /* We currently have only 2 types of mixers */

#define BDSP_RAAGA_PREBOOT_MAILBOX_PATTERN          0xA5A5

#define BDSP_RAAGA_NUM_FIFOS  ((BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)/ \
							   (BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)) /* 59 */
#define BDSP_RAAGA_DRAM_FIFO  ((BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)/ \
							   (BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)) /*59*/
#define BDSP_RAAGA_UART_FIFO  ((BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)/ \
							   (BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR))/*60*/
#define BDSP_RAAGA_CORE_FIFO  ((BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)/ \
							   (BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR))/*61*/
#define BDSP_RAAGA_TARGET_PRINT_FIFO  ((BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR)/ \
							   (BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR))/*62*/
#define BDSP_RAAGA_FIFO_INVALID         ((unsigned int)(-1))
#define BDSP_RAAGA_FIFO_0_INDEX         0


#define BDSP_STAGE_TRAVERSE_LOOP_V1_BEGIN(A, B, C, D)                           \
    {                                                                           \
        BDSP_RaagaStage *macroVarStackOfStages[BDSP_AF_P_MAX_STAGES];           \
        BDSP_RaagaStage *B;                                                     \
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
            B = macroVarStackOfStages[macroVarStackDepth];                      \
            C = macroVarBrId;                                                   \
            D = macroVarStId++;                                                 \
            BDBG_OBJECT_ASSERT(B, BDSP_RaagaStage);                             \

            /* Push all connected stage handles into the stack */
            /* For loop in reverse order as the stack is a lifo structure */
#define BDSP_STAGE_TRAVERSE_LOOP_END(B)                                         \
            macroVarNumStOps = 0;                                               \
            for(macroVari = BDSP_AF_P_MAX_OP_FORKS; macroVari >=1 ; macroVari--)\
            {                                                                   \
                if ((B->sStageConnectionInfo.sStageOutput[macroVari-1].eConnectionType               \
                        == BDSP_ConnectionType_eStage)                          \
                    && (B->sStageConnectionInfo.sStageOutput[macroVari-1].eValid ==              \
                        BDSP_AF_P_eValid))                                      \
                {                                                               \
                    macroVarNumStOps++;                                         \
                    macroVarStackOfStages[macroVarStackDepth] = 				\
                    (BDSP_RaagaStage *)B->sStageConnectionInfo.sStageOutput[macroVari-1].connectionHandle.stage.hStage->pStageHandle; \
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

#define BDSP_STAGE_TRAVERSE_LOOP_BEGIN(A, B)                                    \
    BDSP_STAGE_TRAVERSE_LOOP_V1_BEGIN(A, B, macroBrId, macroStId)

typedef struct BDSP_Raaga_P_DeviceMemoryInfo
{
	BDSP_P_MemoryPool 			sROMemoryPool;
	BDSP_P_MemoryPool 			sRWMemoryPool[BDSP_RAAGA_MAX_DSP];

    BDSP_P_FwBuffer             SharedKernalMemory[BDSP_RAAGA_MAX_DSP];
    BDSP_P_FwBuffer             DescriptorMemory[BDSP_RAAGA_MAX_DSP];
	BDSP_P_FwBuffer             WorkBufferMemory[BDSP_RAAGA_MAX_NUM_PREEMPTION_LEVELS];
    BDSP_Raaga_P_MsgQueueParams cmdQueueParams[BDSP_RAAGA_MAX_DSP];
    BDSP_Raaga_P_MsgQueueParams genRspQueueParams[BDSP_RAAGA_MAX_DSP];
	BDSP_Raaga_P_MsgQueueParams debugQueueParams[BDSP_RAAGA_MAX_DSP][BDSP_Raaga_DebugType_eLast];
}BDSP_Raaga_P_DeviceMemoryInfo;

typedef struct BDSP_P_TaskMemoryInfo
{
	BDSP_P_MemoryPool			sMemoryPool;

	BDSP_P_HostBuffer			hostAsyncQueue;
	BDSP_P_FwBuffer 	  		sCITMemory;
	BDSP_P_FwBuffer 	  		sCITReConfigMemory;
	BDSP_P_FwBuffer				sSampleRateLUTMemory;
	BDSP_P_FwBuffer				sSchedulingConfigMemory;
	BDSP_P_FwBuffer				sGateOpenConfigMemory;

	BDSP_P_FwBuffer		  		sCacheHole; /* 512 bytes of hole for cache coherency */
	BDSP_Raaga_P_MsgQueueParams syncQueueParams;
	BDSP_Raaga_P_MsgQueueParams asyncQueueParams;
	BDSP_P_FwBuffer             sMPSharedMemory;
}BDSP_P_TaskMemoryInfo;

typedef struct BDSP_P_TaskInfo
{
    bool    taskId[BDSP_RAAGA_MAX_FW_TASK_PER_DSP];
    void    *pTask[BDSP_RAAGA_MAX_FW_TASK_PER_DSP];
    unsigned numActiveTasks;
}BDSP_P_TaskInfo;

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

typedef struct BDSP_P_StageMemoryInfo
{
	BDSP_P_MemoryPool	  sMemoryPool;
	BDSP_P_FwBuffer 	  sDataSyncSettings;
	BDSP_P_FwBuffer 	  sTsmSettings;
	BDSP_P_FwBuffer       sHostAlgoUserConfig;

	BDSP_P_FwBuffer		  sCacheHole; /* 512 bytes of hole for cache coherency */
	BDSP_P_FwBuffer 	  sAlgoUserConfig;
	BDSP_P_FwBuffer       sAlgoStatus;
	BDSP_P_FwBuffer       sIdsStatus;
	BDSP_P_FwBuffer       sTsmStatus;
	BDSP_P_FwBuffer       sInterframe;
}BDSP_P_StageMemoryInfo;

typedef struct BDSP_Raaga_P_HardwareStatus
{
    bool bWatchdogTimerStatus;
	bool deviceWatchdogFlag;
	bool dspFifo[BDSP_RAAGA_MAX_DSP][BDSP_RAAGA_NUM_FIFOS];
	bool descriptor[BDSP_RAAGA_MAX_DSP][BDSP_RAAGA_MAX_DESCRIPTORS];
}BDSP_Raaga_P_HardwareStatus;

BDBG_OBJECT_ID_DECLARE(BDSP_Raaga);
typedef struct BDSP_Raaga
{
	BDBG_OBJECT(BDSP_Raaga)
	BDSP_Device device;
	BDSP_RaagaSettings deviceSettings;
    BCHP_Handle chpHandle;
    BREG_Handle regHandle;
    BMMA_Heap_Handle memHandle;
    BINT_Handle intHandle;
    BBOX_Handle boxHandle;
    DSP     sLibDsp;  /* Structure used to initialise the libDspControl module  */
    TB      sTbTargetPrint[BDSP_RAAGA_MAX_DSP]; /* TB support structure for targetprint */
    uint32_t dspOffset[BDSP_RAAGA_MAX_DSP];

	unsigned numDsp;
	unsigned numCorePerDsp;

	BDSP_Raaga_P_HardwareStatus hardwareStatus;
	BKNI_MutexHandle deviceMutex;
    BKNI_EventHandle hEvent[BDSP_RAAGA_MAX_DSP];
	BDSP_Raaga_P_DeviceCallBacks interruptCallbacks[BDSP_RAAGA_MAX_DSP];
	BDSP_P_TaskInfo  taskDetails[BDSP_RAAGA_MAX_DSP];

    BDSP_Raaga_P_MsgQueueHandle hCmdQueue[BDSP_RAAGA_MAX_DSP];
    BDSP_Raaga_P_MsgQueueHandle hGenRespQueue[BDSP_RAAGA_MAX_DSP];

    BDSP_Raaga_P_DeviceMemoryInfo memInfo;
	BDSP_Raaga_P_CodeDownloadInfo codeInfo;

    BLST_S_HEAD(BDSP_RaagaContextList, BDSP_RaagaContext) contextList;
}BDSP_Raaga;

BDBG_OBJECT_ID_DECLARE(BDSP_RaagaContext);
typedef struct BDSP_RaagaContext
{
    BDBG_OBJECT(BDSP_RaagaContext)
    BDSP_Context context;
    BDSP_Raaga *pDevice;
    BDSP_ContextCreateSettings settings;
    BDSP_ContextInterruptHandlers interruptHandlers;
    BLST_S_ENTRY(BDSP_RaagaContext) node;
    BLST_S_HEAD(BDSP_RaagaTaskList, BDSP_RaagaTask) taskList;
    bool                    contextWatchdogFlag;
}BDSP_RaagaContext;

BDBG_OBJECT_ID_DECLARE(BDSP_RaagaTask);
typedef struct BDSP_RaagaTask
{
    BDBG_OBJECT(BDSP_RaagaTask)
	BDSP_Task task;
	BDSP_RaagaContext *pContext;
	BDSP_TaskCreateSettings createSettings;
    BDSP_TaskStartSettings  startSettings;
	BDSP_P_TaskParams taskParams;

	BDSP_Raaga_P_MsgQueueHandle hSyncQueue;
	BDSP_Raaga_P_MsgQueueHandle hAsyncQueue;

	BDSP_P_TaskMemoryInfo taskMemInfo;

	BDSP_Raaga_P_TaskCallBacks interruptCallbacks;
	BDSP_AudioInterruptHandlers audioInterruptHandlers;
    BKNI_EventHandle hEvent;
	BLST_S_ENTRY(BDSP_RaagaTask) node;
}BDSP_RaagaTask;

BDBG_OBJECT_ID_DECLARE(BDSP_RaagaStage);

typedef struct BDSP_RaagaStage
{
    BDBG_OBJECT(BDSP_RaagaStage)
    BDSP_Stage stage;

    BDSP_RaagaContext *pContext;
    BDSP_RaagaTask *pRaagaTask;

    BDSP_Algorithm eAlgorithm;
    BDSP_StageCreateSettings settings;

	BDSP_P_StageMemoryInfo stageMemInfo;
	bool 	  running;
    unsigned  totalInputs;
    unsigned  totalOutputs;
	unsigned  stageID;

	BDSP_P_StageConnectionInfo sStageConnectionInfo;
}BDSP_RaagaStage;

void BDSP_Raaga_P_CalculateStageMemory(
	unsigned    *pMemReqd,
    bool        ifMemApiTool,
	const BDSP_RaagaUsageOptions *pUsage
);
void BDSP_Raaga_P_CalculateTaskMemory(
	unsigned *pMemReqd
);
void BDSP_Raaga_P_CalculateDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned   *pMemReqd
);
void BDSP_Raaga_P_CalculateDeviceROMemory(
    BDSP_Raaga *pDevice,
    unsigned *pMemReqd
);
BERR_Code BDSP_Raaga_P_AssignDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
);
BERR_Code BDSP_Raaga_P_ReleaseDeviceRWMemory(
    BDSP_Raaga *pDevice,
    unsigned dspindex
);
BERR_Code BDSP_Raaga_P_InitDeviceSettings(
	BDSP_Raaga *pRaaga
);
BERR_Code BDSP_Raaga_P_GetNumberOfDspandCores(
	BBOX_Handle boxHandle,
	unsigned *pNumDsp,
	unsigned *pNumCores
);
BERR_Code BDSP_Raaga_P_ReleaseFIFO(
	BDSP_Raaga *pDevice,
	unsigned dspIndex,
	uint32_t* ui32Fifo,
	unsigned numfifos
);
BERR_Code BDSP_Raaga_P_AssignFreeFIFO(
	BDSP_Raaga *pDevice,
	unsigned dspIndex,
	uint32_t *pui32Fifo,
	unsigned numfifosreqd
);
BERR_Code BDSP_Raaga_P_CheckDspAlive(
	BDSP_Raaga *pDevice
);
BERR_Code BDSP_Raaga_P_Open(
	BDSP_Raaga *pDevice
);
void BDSP_Raaga_P_Close(
	void *pDeviceHandle
);
void BDSP_Raaga_P_GetDefaultContextSettings(
	void 					   *pDeviceHandle,
    BDSP_ContextType 			contextType,
    BDSP_ContextCreateSettings *pSettings
);
BERR_Code BDSP_Raaga_P_CreateContext(
	void 							 *pDeviceHandle,
	const BDSP_ContextCreateSettings *pSettings,
	BDSP_ContextHandle 				 *pContextHandle
);
void BDSP_Raaga_P_DestroyContext(
	void *pContextHandle
);
void BDSP_Raaga_P_GetDefaultTaskSettings(
	void *pContextHandle,
	BDSP_TaskCreateSettings *pSettings
);
BERR_Code BDSP_Raaga_P_CreateTask(
	void *pContextHandle,
	const BDSP_TaskCreateSettings *pSettings,
	BDSP_TaskHandle *pTaskHandle
);
void BDSP_Raaga_P_DestroyTask(
	void *pTaskHandle
);
void BDSP_Raaga_P_GetDefaultTaskStartSettings(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pSettings    /* [out] */
);
BERR_Code BDSP_Raaga_P_StartTask(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pStartSettings
);
BERR_Code BDSP_Raaga_P_StopTask(
	void *pTaskHandle
);
void BDSP_Raaga_P_GetDefaultStageCreateSettings(
	BDSP_AlgorithmType algoType,
	BDSP_StageCreateSettings *pSettings /* [out] */
);
BERR_Code BDSP_Raaga_P_CreateStage(
	void *pContextHandle,
	const BDSP_StageCreateSettings *pSettings,
	BDSP_StageHandle *pStageHandle /* [out] */
);
void BDSP_Raaga_P_DestroyStage(
	void *pStageHandle
);
BERR_Code BDSP_Raaga_P_SetAlgorithm(
	void *pStageHandle,
	BDSP_Algorithm algorithm
);
BERR_Code BDSP_Raaga_P_GetStageSettings(
	void *pStageHandle,
	void *pSettingsBuffer,
	size_t settingsSize
);
BERR_Code BDSP_Raaga_P_SetStageSettings(
	void *pStageHandle,
	const void *pSettingsBuffer,
	size_t settingsSize
);
BERR_Code BDSP_Raaga_P_GetDatasyncSettings(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
);
BERR_Code BDSP_Raaga_P_GetDatasyncSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
);
BERR_Code BDSP_Raaga_P_SetDatasyncSettings(
	void *pStageHandle,
	const BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
);
BERR_Code BDSP_Raaga_P_GetTsmSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmSettings  *pSettingsBuffer
);
BERR_Code BDSP_Raaga_P_SetTsmSettings_isr(
	void *pStageHandle,
	const BDSP_AudioTaskTsmSettings *pSettingsBuffer
);
BERR_Code BDSP_Raaga_P_GetStageStatus(
	void *pStageHandle,
	void *pStatusBuffer,
	size_t statusSize
);
BERR_Code BDSP_Raaga_P_GetDatasyncStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncStatus *pStatusBuffer
);
BERR_Code BDSP_Raaga_P_GetTsmStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmStatus *pStatusBuffer
);
void BDSP_Raaga_P_GetAlgorithmInfo(
	BDSP_Algorithm algorithm,
	BDSP_AlgorithmInfo *pInfo /* [out] */
);
void BDSP_Raaga_P_GetStatus(
	void *pDeviceHandle,
	BDSP_Status *pStatus
);
BERR_Code BDSP_Raaga_P_Pause(
	void *pTaskHandle
);
BERR_Code BDSP_Raaga_P_Resume(
	void *pTaskHandle
);
BERR_Code BDSP_Raaga_P_AudioGapFill(
	void *pTaskHandle
);
BERR_Code BDSP_Raaga_P_Advance(
	void *pTaskHandle,
	unsigned timeInMs
);
BERR_Code BDSP_Raaga_P_Freeze(
	void *pTaskHandle,
	const BDSP_AudioTaskFreezeSettings *pSettings
);
BERR_Code BDSP_Raaga_P_UnFreeze(
	void *pTaskHandle,
	const BDSP_AudioTaskUnFreezeSettings *pSettings
);
#endif /*BDSP_RAAGA_PRIV_H_*/
