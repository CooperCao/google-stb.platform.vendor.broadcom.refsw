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

#include "bdsp_arm_priv_include.h"

#define BDSP_ARM_FMM_WRAP_BIT             40
#define BDSP_ARM_FMM_WRAP_MASK           ((dramaddr_t)0x1<<BDSP_ARM_FMM_WRAP_BIT)
#define BDSP_ARM_FMM_ADDR_MASK           (BDSP_ARM_FMM_WRAP_MASK-1)

#define BDSP_ARM_NUM_FIFOS 59

#define BDSP_ARM_STAGE_TRAVERSE_LOOP_V1_BEGIN(A, B, C, D)                           \
    {                                                                           \
        BDSP_ArmStage *macroVarStackOfStages[BDSP_AF_P_MAX_STAGES];           \
        BDSP_ArmStage *B;                                                     \
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
            BDBG_OBJECT_ASSERT(B, BDSP_ArmStage);                             \

            /* Push all connected stage handles into the stack */
            /* For loop in reverse order as the stack is a lifo structure */
#define BDSP_ARM_STAGE_TRAVERSE_LOOP_END(B)                                         \
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
                    (BDSP_ArmStage *)B->sStageConnectionInfo.sStageOutput[macroVari-1].connectionHandle.stage.hStage->pStageHandle; \
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

typedef enum BDSP_ARM_P_eRdbVarIndices
{
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_RDB_VARS_ADDR = 0,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID = 1,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_HOST2DSPRESPONSE_FIFO_ID = 2,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_BASE_ADDR = 3,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_END_ADDR = 4,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_WRITE_ADDR = 5,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_0_READ_ADDR = 6,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_FIFO_1_BASE_ADDR = 7,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_STATUS = 8,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_RCV_DATA = 9,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_XMIT_DATA = 10,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_DBG_CTRL_UART_CTRL = 11,
    BDSP_ARM_P_eRdbVarIndices_DSP_TIMERS_TSM_TIMER = 12,
    BDSP_ARM_P_eRdbVarIndices_DSP_TIMERS_TSM_TIMER_VALUE = 13,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_STATUS_0 = 14,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_SET_0 = 15,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_CLEAR_0 = 16,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_STATUS_1 = 17,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_SET_1 = 18,
    BDSP_ARM_P_eRdbVarIndices_DSP_PERI_SW_MSG_BITS_CLEAR_1 = 19,
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_INTH_HOST_SET = 20,
    BDSP_ARM_P_eRdbVarIndices_Max,
    BDSP_ARM_P_eRdbVarIndices_Invalid = 0x7fffffff
}BDSP_ARM_P_eRdbVarIndices;

typedef struct BDSP_Arm_P_DeviceMemoryInfo
{
    BDSP_P_MemoryPool           sROMemoryPool;
    BDSP_P_MemoryPool           sIOMemoryPool[BDSP_ARM_MAX_DSP];
    BDSP_P_MemoryPool           sRWMemoryPool[BDSP_ARM_MAX_DSP];

    BDSP_P_FwBuffer             DescriptorMemory[BDSP_ARM_MAX_DSP];
    BDSP_P_FwBuffer             WorkBufferMemory[BDSP_ARM_MAX_DSP][BDSP_MAX_NUM_SCHED_LEVELS];
    BDSP_P_FwBuffer             CfgRegisters[BDSP_ARM_MAX_DSP];
    BDSP_P_FwBuffer             softFifo[BDSP_ARM_MAX_DSP];
    BDSP_P_MsgQueueParams       cmdQueueParams[BDSP_ARM_MAX_DSP];
    BDSP_P_MsgQueueParams       genRspQueueParams[BDSP_ARM_MAX_DSP];
}BDSP_Arm_P_DeviceMemoryInfo;

typedef struct BDSP_Arm_P_HardwareStatus
{
    bool deviceWatchdogFlag;
    bool softFifo[BDSP_ARM_MAX_DSP][BDSP_ARM_NUM_FIFOS];
	bool descriptor[BDSP_ARM_MAX_DSP][BDSP_MAX_DESCRIPTORS];
}BDSP_Arm_P_HardwareStatus;

typedef struct BDSP_ArmDspApp{
    BTEE_InstanceHandle hBteeInstance;
    BTEE_ClientHandle hClient;
    BTEE_ApplicationHandle hApplication;
    BTEE_ConnectionHandle hConnection;
}BDSP_ArmDspApp;

BDBG_OBJECT_ID_DECLARE(BDSP_Arm);
typedef struct BDSP_Arm
{
    BDBG_OBJECT(BDSP_Arm)
    BDSP_Device device;
    BDSP_ArmDspApp   armDspApp;
    BDSP_ArmSettings deviceSettings;
    BCHP_Handle chpHandle;
    BREG_Handle regHandle;
    BMMA_Heap_Handle memHandle;
    BINT_Handle intHandle;
    BBOX_Handle boxHandle;

    unsigned numDsp;
    unsigned numCorePerDsp;

    BDSP_Arm_P_HardwareStatus hardwareStatus;
    BKNI_MutexHandle deviceMutex;
    BKNI_EventHandle hEvent[BDSP_ARM_MAX_DSP];
#if 0
    BDSP_Raaga_P_DeviceCallBacks interruptCallbacks[BDSP_ARM_MAX_DSP];
#endif
    BDSP_P_TaskInfo  taskDetails[BDSP_ARM_MAX_DSP];

    BDSP_P_MsgQueueHandle hCmdQueue[BDSP_ARM_MAX_DSP];
    BDSP_P_MsgQueueHandle hGenRespQueue[BDSP_ARM_MAX_DSP];

    BDSP_Arm_P_DeviceMemoryInfo memInfo;
    BDSP_Arm_P_CodeDownloadInfo codeInfo;
	BDSP_P_SystemSchedulingInfo systemSchedulingInfo;

    BLST_S_HEAD(BDSP_ArmContextList, BDSP_ArmContext) contextList;
}BDSP_Arm;

BDBG_OBJECT_ID_DECLARE(BDSP_ArmContext);
typedef struct BDSP_ArmContext
{
    BDBG_OBJECT(BDSP_ArmContext)
    BDSP_Context context;
    BDSP_Arm    *pDevice;
    BDSP_ContextCreateSettings settings;
    BDSP_ContextInterruptHandlers interruptHandlers;
    BLST_S_ENTRY(BDSP_ArmContext) node;
    BLST_S_HEAD(BDSP_ArmTaskList, BDSP_ArmTask) taskList;
    bool                    contextWatchdogFlag;
}BDSP_ArmContext;

BDBG_OBJECT_ID_DECLARE(BDSP_ArmTask);
typedef struct BDSP_ArmTask
{
    BDBG_OBJECT(BDSP_ArmTask)
	BDSP_Task task;
	BDSP_ArmContext *pContext;
	BDSP_TaskCreateSettings createSettings;
    BDSP_TaskStartSettings  startSettings;
	BDSP_P_TaskParams taskParams;

	BDSP_P_MsgQueueHandle hSyncQueue;
	BDSP_P_MsgQueueHandle hAsyncQueue;

	BDSP_P_TaskMemoryInfo taskMemInfo;

	/*BDSP_Arm_P_TaskCallBacks interruptCallbacks;*/
	BDSP_AudioInterruptHandlers audioInterruptHandlers;
    BKNI_EventHandle hEvent;
	BLST_S_ENTRY(BDSP_ArmTask) node;
}BDSP_ArmTask;

BDBG_OBJECT_ID_DECLARE(BDSP_ArmStage);
typedef struct BDSP_ArmStage
{
    BDBG_OBJECT(BDSP_ArmStage)
    BDSP_Stage stage;

    BDSP_ArmContext *pContext;
    BDSP_ArmTask    *pArmTask;

    BDSP_Algorithm eAlgorithm;
    BDSP_StageCreateSettings settings;

	BDSP_P_StageMemoryInfo stageMemInfo;
	bool 	  running;
    unsigned  totalInputs;
    unsigned  totalOutputs;
	unsigned  stageID;

	BDSP_P_StageConnectionInfo sStageConnectionInfo;

    /* Capture information for the stage */
    BLST_S_HEAD(BDSP_ArmCaptureList, BDSP_ArmCapture) captureList;
}BDSP_ArmStage;

/***************************************************************************
Summary:
Capture pointers structure
***************************************************************************/
typedef struct BDSP_ArmCapturePointerInfo
{
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER outputBufferPtr; /* Structure containing pointers to the output buffers
                                                   from which the data has to be captured */
    dramaddr_t ui32StartWriteAddr;
    BDSP_P_BufferDescriptor captureBufferPtr; /* Structure containing the pointers to the intermediate
                                                    buffer into which the captured data is written */
    dramaddr_t shadowRead; /* The shadow read pointer for the output buffer */
    dramaddr_t lastWrite; /* The last value of the write pointer; will be used for capture error detection*/
    BDSP_MMA_Memory CaptureBufferMemory; /* Memory for Each channel  where captured Data is written into*/
	BDSP_MMA_Memory OutputBufferMemory;  /* Memory provided by APE(FMM/RDB) which was allocated for the Port to read from*/
} BDSP_ArmCapturePointerInfo;

BDBG_OBJECT_ID_DECLARE(BDSP_ArmCapture);
typedef struct BDSP_ArmCapture
{
    BDBG_OBJECT(BDSP_ArmCapture)
    BDSP_AudioCapture    capture;
    BDSP_ArmStage *pArmStage;
    BDSP_ArmContext *pArmContext;

    BMMA_Heap_Handle hHeap; /* Heap from which the capture buffers need to be allocated */
    BDSP_MMA_Memory  captureBuffer;
    uint8_t maxBuffers;             /* Maximum number of buffers */
    bool enabled;                   /* Flag to indicate whether the capture is enabled or disabled */
    bool updateRead;               /* If true then the read pointers of the output buffer are updated in the capture
                                                                    thread. This can be set to true when there is not consumer for the output data */

    BDSP_AF_P_BufferType eBuffType; /* The buffer type of the the output buffer (RAVE, FMM, DRAM etc ...) */
    bool StartCapture;
    uint8_t numBuffers;             /* Number of valid buffers */
    BDSP_ArmCapturePointerInfo CapturePointerInfo[BDSP_AF_P_MAX_CHANNELS]; /* Capture pointer info for all the output capture */
    BLST_S_ENTRY(BDSP_ArmCapture) node;
} BDSP_ArmCapture;

BERR_Code BDSP_Arm_P_InitDeviceSettings(
    BDSP_Arm *pArm
);
BERR_Code BDSP_Arm_P_ReleaseFIFO(
    BDSP_Arm *pDevice,
    unsigned dspIndex,
    uint32_t* ui32Fifo,
    unsigned numfifos
);
BERR_Code BDSP_Arm_P_AssignFreeFIFO(
    BDSP_Arm *pDevice,
    unsigned  dspIndex,
    uint32_t *pui32Fifo,
    unsigned  numfifosreqd
);
void BDSP_Arm_P_CalculateStageMemory(
	unsigned           *pMemReqd,
	BDSP_AlgorithmType  algoType,
    bool                ifMemApiTool,
	const BDSP_ArmUsageOptions *pUsage
);
void BDSP_Arm_P_CalculateTaskMemory(
	unsigned *pMemReqd
);
void BDSP_Arm_P_CalculateDeviceRWMemory(
    BDSP_Arm   *pDevice,
    unsigned    dspIndex,
    unsigned   *pMemReqd
);
void BDSP_Arm_P_CalculateDeviceROMemory(
    BDSP_Arm *pDevice,
    unsigned *pMemReqd
);
void BDSP_Arm_P_CalculateDeviceIOMemory(
    BDSP_Arm *pDevice,
    unsigned *pMemReqd
);
BERR_Code BDSP_Arm_P_AssignDeviceRWMemory(
    BDSP_Arm *pDevice,
    unsigned  dspindex
);
BERR_Code BDSP_Arm_P_ReleaseDeviceRWMemory(
    BDSP_Arm *pDevice,
    unsigned  dspindex
);
BERR_Code BDSP_Arm_P_ValidateVersion(
    BDSP_ArmSettings *pSettings
);
BERR_Code BDSP_Arm_P_CheckDspAlive(
    BDSP_Arm *pDevice
);
BERR_Code BDSP_Arm_P_Open(
    BDSP_Arm *pDevice
);
BERR_Code BDSP_Arm_P_OpenUserApp(
    BDSP_Arm *pDevice
);
void BDSP_Arm_P_Close(
    void *pDeviceHandle
);
void BDSP_Arm_P_GetDefaultContextSettings(
	void 					   *pDeviceHandle,
    BDSP_ContextType 			contextType,
    BDSP_ContextCreateSettings *pSettings
);
BERR_Code BDSP_Arm_P_CreateContext(
	void 							 *pDeviceHandle,
	const BDSP_ContextCreateSettings *pSettings,
	BDSP_ContextHandle 				 *pContextHandle
);
void BDSP_Arm_P_DestroyContext(
	void *pContextHandle
);
void BDSP_Arm_P_GetDefaultTaskSettings(
	void *pContextHandle,
	BDSP_TaskCreateSettings *pSettings
);
BERR_Code BDSP_Arm_P_CreateTask(
	void *pContextHandle,
	const BDSP_TaskCreateSettings *pSettings,
	BDSP_TaskHandle *pTaskHandle
);
void BDSP_Arm_P_DestroyTask(
	void *pTaskHandle
);
void BDSP_Arm_P_GetDefaultTaskStartSettings(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pSettings    /* [out] */
);
BERR_Code BDSP_Arm_P_StartTask(
	void *pTaskHandle,
	BDSP_TaskStartSettings *pStartSettings
);
BERR_Code BDSP_Arm_P_StopTask(
	void *pTaskHandle
);
BERR_Code BDSP_Arm_P_Initialize(
    void *pDeviceHandle
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
void BDSP_Arm_P_GetAlgorithmInfo(
	BDSP_Algorithm algorithm,
	BDSP_AlgorithmInfo *pInfo /* [out] */
);
BERR_Code BDSP_Arm_P_SetAlgorithm(
	void *pStageHandle,
	BDSP_Algorithm algorithm
);
BERR_Code BDSP_Arm_P_GetAudioDelay_isrsafe(
    BDSP_CTB_Input   *pCtbInput,
    void             *pStageHandle,
    BDSP_CTB_Output  *pCTBOutput
);
BERR_Code BDSP_Arm_P_GetStageSettings(
	void *pStageHandle,
	void *pSettingsBuffer,
	size_t settingsSize
);
BERR_Code BDSP_Arm_P_GetTsmSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmSettings  *pSettingsBuffer
);
BERR_Code BDSP_Arm_P_GetStageStatus(
	void *pStageHandle,
	void *pStatusBuffer,
	size_t statusSize
);
BERR_Code BDSP_Arm_P_GetStageSettings(
	void *pStageHandle,
	void *pSettingsBuffer,
	size_t settingsSize
);
BERR_Code BDSP_Arm_P_SetStageSettings(
	void *pStageHandle,
	const void *pSettingsBuffer,
	size_t settingsSize
);
BERR_Code BDSP_Arm_P_GetDatasyncSettings(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
);
BERR_Code BDSP_Arm_P_GetDatasyncSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
);
BERR_Code BDSP_Arm_P_SetDatasyncSettings(
	void *pStageHandle,
	const BDSP_AudioTaskDatasyncSettings *pSettingsBuffer
);
BERR_Code BDSP_Arm_P_GetTsmSettings_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmSettings  *pSettingsBuffer
);
BERR_Code BDSP_Arm_P_SetTsmSettings_isr(
	void *pStageHandle,
	const BDSP_AudioTaskTsmSettings *pSettingsBuffer
);
BERR_Code BDSP_Arm_P_GetStageStatus(
	void *pStageHandle,
	void *pStatusBuffer,
	size_t statusSize
);
BERR_Code BDSP_Arm_P_GetDatasyncStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskDatasyncStatus *pStatusBuffer
);
BERR_Code BDSP_Arm_P_GetTsmStatus_isr(
	void *pStageHandle,
	BDSP_AudioTaskTsmStatus *pStatusBuffer
);
void BDSP_Arm_P_GetStatus(
    void *pDeviceHandle,
    BDSP_Status *pStatus
);
BERR_Code BDSP_Arm_P_Pause(
	void *pTaskHandle
);
BERR_Code BDSP_Arm_P_Resume(
	void *pTaskHandle
);
BERR_Code BDSP_Arm_P_AudioGapFill(
	void *pTaskHandle
);
BERR_Code BDSP_Arm_P_Advance(
	void *pTaskHandle,
	unsigned timeInMs
);
BERR_Code BDSP_Arm_P_Freeze(
	void *pTaskHandle,
	const BDSP_AudioTaskFreezeSettings *pSettings
);
BERR_Code BDSP_Arm_P_UnFreeze(
	void *pTaskHandle,
	const BDSP_AudioTaskUnFreezeSettings *pSettings
);
#endif /*BDSP_ARM_PRIV_H_*/
