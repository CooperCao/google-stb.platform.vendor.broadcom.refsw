/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Host <-> Decoder message descriptions
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BAPE_DSP_IPC_PRIV_H_
#define BAPE_DSP_IPC_PRIV_H_

#include "bape_types.h"
#include "bape_codec_types.h"
#include "bkernel_ipc.h"
#include "bape_chip_priv.h"

/***************************************************************************
Summary:
Message ID's
***************************************************************************/
typedef enum BAPE_MessageId
{
    BAPE_MessageId_eNone,
    BAPE_MessageId_eRegisterModule,
    BAPE_MessageId_eUnregisterModule,
    BAPE_MessageId_eCreateTask,
    BAPE_MessageId_eDestroyTask,
    BAPE_MessageId_eGetTaskSettings,
    BAPE_MessageId_eSetTaskSettings,
    BAPE_MessageId_eGetTaskStatus,
    BAPE_MessageId_eStartTask,
    BAPE_MessageId_eStopTask,
    BAPE_MessageId_ePauseTask,
    BAPE_MessageId_eAdvanceTask,
    BAPE_MessageId_eGetStageSettings,
    BAPE_MessageId_eSetStageSettings,
    BAPE_MessageId_eGetStageStatus,
    BAPE_MessageId_eHalt,
    BAPE_MessageId_eMax
} BAPE_MessageId;

/***************************************************************************
Summary:
Register Module Message
***************************************************************************/
typedef struct BAPE_RegisterModuleMessage
{
    void *pEntry;
    void *pCodeSegmentBase;
    uint32_t codeSegmentLength;
} BAPE_RegisterModuleMessage;

/***************************************************************************
Summary:
Register Module Reply
***************************************************************************/
typedef struct BAPE_RegisterModuleReply
{
    uint32_t moduleId;
} BAPE_RegisterModuleReply;

/***************************************************************************
Summary:
Unregister Module Message
***************************************************************************/
typedef struct BAPE_UnregisterModuleMessage
{
    uint32_t moduleId;
} BAPE_UnregisterModuleMessage;

/***************************************************************************
Summary:
Task Stages
***************************************************************************/
typedef struct BAPE_TaskStage
{
    uint32_t moduleId;
    uint16_t algorithmId;   /* BAPE_ProcessorType */
    uint16_t numOutputs;    /* Number of connected outputs */
} BAPE_TaskStage;

/***************************************************************************
Summary:
Task Processing Branch
***************************************************************************/
#define BAPE_MAX_BRANCH_STAGES (4)
typedef struct BAPE_TaskBranch
{
    /* Input to the branch */
    uint32_t inputBranchId;   /* 0xffffffff is the decode branch */
    uint32_t inputStageId;    /* if decode branch, 0 indicates stereo, 1 indicates multichannel, 2 indicates compressed */

    /* Stages */
    uint32_t numStages;
    BAPE_TaskStage stages[BAPE_MAX_BRANCH_STAGES];
} BAPE_TaskBranch;

/***************************************************************************
Summary:
Task Processing Branch
***************************************************************************/
typedef struct BAPE_TaskOutput
{
    /* Data passed to the output */
    uint32_t inputBranchId; /* 0xffffffff is the decode branch */
    uint32_t inputStageId;  /* if decode branch, 0 indicates stereo, 1 indicates multichannel, 2 indicates compressed */

    /* Output Properties */
    uint8_t master;            /* If true, this is a master source channel */
    uint8_t spdifFormat;       /* If true, compressed data should be SPDIF formatted */
    uint16_t masterId;          /* If this is a slave output, ID of the master output for independent delay.  Otherwise, equal to output ID */
    uint16_t delayMs;           /* output delay in ms */
    uint16_t numChannelPairs;   /* Number of output channel pairs */
    uint32_t rbufBase[BAPE_ChannelPair_eMax];   /* Ringbuffer base for channel pair (RDADDR) */
    uint32_t gateReg[BAPE_ChannelPair_eMax];    /* AUD_FMM_BF_CTRL_SOURCECH_CTRLn */ 
    uint32_t ppmReg[BAPE_ChannelPair_eMax];     /* AUD_FMM_BF_CTRL_ADAPTRATE_REPEATRDOP_WRCNT_n (0=no ppm adjustments) */
} BAPE_TaskOutput;

/***************************************************************************
Summary:
TSM Threshold
***************************************************************************/
typedef struct BAPE_TaskTsmRange
{
    uint32_t minus; /* in 45KHz units */
    uint32_t plus;  /* in 45KHz units */    
} BAPE_TaskTsmRange;

/***************************************************************************
Summary:
TSM Settings Container
***************************************************************************/
typedef struct BAPE_TaskTsmSettings
{
    uint8_t tsmEnabled;
    uint8_t astmEnabled;
    uint8_t playback;
    uint8_t dummy;
    uint32_t ptsOffset;     /* PTS offset value in 45 kHz ticks. */

    BAPE_TaskTsmRange discardThreshold; /* if PTS-STC is outside this range, then frames are dropped without further consideration */
    BAPE_TaskTsmRange trackingRange;    /* if PTS-STC is within this range, then no frames are dropped/repeated, however samples could be added removed to reduce PTS-STC difference  */
    BAPE_TaskTsmRange inSyncRange;      /* if PTS-STC is within this range, then no samples or frames are dropped/repeated */
    BAPE_TaskTsmRange startRange;       /* if during the start sequence PTS-STC can't be made within this range, frame dropped and start sequence repeated */
} BAPE_TaskTsmSettings;

/***************************************************************************
Summary:
TSM Status Container
***************************************************************************/
typedef struct BAPE_TaskTsmStatus
{
    BAVC_PTSInfo ptsInfo;
    int32_t ptsStcDifference;   /* PTS - STC value */
} BAPE_TaskTsmStatus;

/***************************************************************************
Summary:
Task Status Container
***************************************************************************/
typedef struct BAPE_TaskStatus
{
    uint32_t sampleRate;
    uint32_t framesDecoded;
    uint32_t frameErrors;
    uint32_t dummyFrames;
} BAPE_TaskStatus;

/***************************************************************************
Summary:
Task Settings
***************************************************************************/
typedef struct BAPE_TaskSettings
{
    BAPE_DualMonoMode dualMonoMode;
    BAPE_ChannelMode outputMode;
    uint32_t outputLfe;
} BAPE_TaskSettings;

/***************************************************************************
Summary:
Task Description Structure
***************************************************************************/
#define BAPE_MAX_TASK_BRANCHES (4)
#define BAPE_MAX_TASK_OUTPUTS  (8)
typedef struct BAPE_TaskDescriptor
{
    /* Shared memory information */
    volatile uint32_t *pTsmSettingsBegin;   /* Updated when host begins writing settings */
    const BAPE_TaskTsmSettings *pTsmSettings;
    volatile uint32_t *pTsmSettingsEnd;     /* Updated when host finishes writing settings */

    volatile uint32_t *pTsmStatusBegin;     /* Updated when DSP begins writing status */
    BAPE_TaskTsmStatus *pTsmStatus;
    volatile uint32_t *pTsmStatusEnd;       /* Updated when DSP finishes writing status */

    volatile bool *pPlaybackStcValid;    /* Updated by host when playback STC is valid */

    volatile const uint32_t *pSampleRateBuffer;

    /* Input Information */
    uint32_t stcReg;        /* STC shadow register (AUD_FMM_MISC_STC_UPPERn), gives upper 32-bits of STC */
    uint32_t inputRaveBase; /* Input RAVE context base register (AV_CDB_READ_PTR) */
    uint32_t inputRbufBase; /* Input ringbuffer base register (RDADDR) */

    /* Decode information */
    uint32_t decodeModuleId;    /* Module ID of decoder */
    uint32_t decodeCodec;       /* BAVC_AudioCompressionStd */
    uint32_t decodeStreamType;  /* BAVC_StreamType, used for TSM */
    uint32_t numDecodeOutputs;  /* Number of decode outputs */

    /* Post-processing */
    uint32_t numBranches;

    /* Post-processing */
    BAPE_TaskBranch branches[BAPE_MAX_TASK_BRANCHES];

    /* Outputs */
    BAPE_TaskOutput outputs[BAPE_MAX_TASK_OUTPUTS];
    
    /* Task Settings */
    BAPE_TaskSettings taskSettings;
} BAPE_TaskDescriptor;

/***************************************************************************
Summary:
Create Task Message
***************************************************************************/
typedef struct BAPE_CreateTaskMessage
{
    BAPE_TaskDescriptor descriptor;
} BAPE_CreateTaskMessage;

/***************************************************************************
Summary:
Create Task Reply
***************************************************************************/
typedef struct BAPE_CreateTaskReply
{
    uint32_t taskId;
} BAPE_CreateTaskReply;

/***************************************************************************
Summary:
Destroy Task Message
***************************************************************************/
typedef struct BAPE_DestroyTaskMessage
{
    uint32_t taskId;
} BAPE_DestroyTaskMessage;

/***************************************************************************
Summary:
Get Task Settings Message
***************************************************************************/
typedef struct BAPE_GetTaskSettingsMessage
{
    uint32_t taskId;    
} BAPE_GetTaskSettingsMessage;

/***************************************************************************
Summary:
Get Task Settings Reply
***************************************************************************/
typedef struct BAPE_GetTaskSettingsReply
{
    BAPE_TaskSettings settings;    
} BAPE_GetTaskSettingsReply;

/***************************************************************************
Summary:
Set Task Settings Message
***************************************************************************/
typedef struct BAPE_SetTaskSettingsMessage
{
    uint32_t taskId;    
    BAPE_TaskSettings settings;    
} BAPE_SetTaskSettingsMessage;

/***************************************************************************
Summary:
Get Task Status Message
***************************************************************************/
typedef struct BAPE_GetTaskStatusMessage
{
    uint32_t taskId;    
} BAPE_GetTaskStatusMessage;

/***************************************************************************
Summary:
Get Task Status Reply
***************************************************************************/
typedef struct BAPE_GetTaskStatusReply
{
    BAPE_TaskStatus status;    
} BAPE_GetTaskStatusReply;

/***************************************************************************
Summary:
Start Task Message
***************************************************************************/
typedef struct BAPE_StartTaskMessage
{
    uint32_t taskId;    
} BAPE_StartTaskMessage;

/***************************************************************************
Summary:
Stop Task Message
***************************************************************************/
typedef struct BAPE_StopTaskMessage
{
    uint32_t taskId;    
} BAPE_StopTaskMessage;

/***************************************************************************
Summary:
Pause Task Message
***************************************************************************/
typedef struct BAPE_PauseTaskMessage
{
    uint32_t taskId;    
    uint32_t pauseEnabled;
} BAPE_PauseTaskMessage;

/***************************************************************************
Summary:
Advance Task Message
***************************************************************************/
typedef struct BAPE_AdvanceTaskMessage
{
    uint32_t taskId;    
    uint32_t ptsUnits;  /* number of PTS units to advance */
} BAPE_AdvanceTaskMessage;

/***************************************************************************
Summary:
Get Stage Settings Message
***************************************************************************/
typedef struct BAPE_GetStageSettingsMessage
{
    uint32_t taskId;    
    uint32_t branchId;  /* 0xFFFFFFFF indicated decode stage/branch */
    uint32_t stageId;   /* Ignored if branch ID is 0xffffffff */
    uint32_t settingsSize;
    void *pSettings;
} BAPE_GetStageSettingsMessage;

/***************************************************************************
Summary:
Set Stage Settings Message
***************************************************************************/
typedef struct BAPE_SetStageSettingsMessage
{
    uint32_t taskId;    
    uint32_t branchId;  /* 0xFFFFFFFF indicated decode stage/branch */
    uint32_t stageId;   /* Ignored if branch ID is 0xffffffff */
    uint32_t settingsSize;
    const void *pSettings;
} BAPE_SetStageSettingsMessage;

/***************************************************************************
Summary:
Get Stage Status Message
***************************************************************************/
typedef struct BAPE_GetStageStatusMessage
{
    uint32_t taskId;    
    uint32_t branchId;  /* 0xFFFFFFFF indicates decode stage/branch */
    uint32_t stageId;   /* Ignored if branch ID is 0xffffffff */
    uint32_t statusSize;
    void *pStatus;
} BAPE_GetStageStatusMessage;

/***************************************************************************
Summary:
Message Structure
***************************************************************************/
typedef struct BAPE_Message
{
    BAPE_MessageId id;
    union
    {
        BAPE_RegisterModuleMessage registerModule;
        BAPE_UnregisterModuleMessage unregisterModule;
        BAPE_CreateTaskMessage createTask;
        BAPE_DestroyTaskMessage destroyTask;
        BAPE_GetTaskSettingsMessage getTaskSettings;
        BAPE_SetTaskSettingsMessage setTaskSettings;
        BAPE_GetTaskStatusMessage getTaskStatus;
        BAPE_StartTaskMessage startTask;
        BAPE_StopTaskMessage stopTask;
        BAPE_PauseTaskMessage pauseTask;
        BAPE_AdvanceTaskMessage advanceTask;
        BAPE_GetStageSettingsMessage getStageSettings;
        BAPE_SetStageSettingsMessage setStageSettings;
        BAPE_GetStageStatusMessage getStageStatus;
    } data;
} BAPE_Message;

/***************************************************************************
Summary:
Message Reply Structure
***************************************************************************/
typedef struct BAPE_Reply
{
    BAPE_MessageId id;
    BERR_Code errCode;
    union
    {
        BAPE_RegisterModuleReply registerModule;
        BAPE_CreateTaskReply createTask;
        BAPE_GetTaskSettingsReply getTaskSettings;
        BAPE_GetTaskStatusReply getTaskStatus;
    } data;
} BAPE_Reply;

/***************************************************************************
Summary:
Interrupt ID's
***************************************************************************/
typedef enum BAPE_InterruptId
{
    BAPE_InterruptId_eNone,
    BAPE_InterruptId_eLock,
    BAPE_InterruptId_eUnlock,
    BAPE_InterruptId_eSampleRateChanged,
    BAPE_InterruptId_eFirstPts,
    BAPE_InterruptId_eTsmFail,
    BAPE_InterruptId_eAstmTsmPass,
    BAPE_InterruptId_eTaskStatusValid,
    BAPE_InterruptId_eMax
} BAPE_InterruptId;

/***************************************************************************
Summary:
Sample Rate Changed Interrupt Data
***************************************************************************/
typedef struct BAPE_SampleRateChangedData
{
    uint32_t sampleRate;
} BAPE_SampleRateChangedData;

/***************************************************************************
Summary:
First PTS Interrupt Data
***************************************************************************/
typedef struct BAPE_FirstPtsData
{
    BAVC_PTSInfo ptsInfo;
} BAPE_FirstPtsData;

/***************************************************************************
Summary:
TSM Failed Interrupt Data
***************************************************************************/
typedef struct BAPE_TsmFailData
{
    BAVC_PTSInfo ptsInfo;
    int32_t ptsStcDifference;
} BAPE_TsmFailData;

/***************************************************************************
Summary:
Interrupt Data Structure
***************************************************************************/
typedef struct BAPE_Interrupt
{
    BAPE_InterruptId id;
    uint32_t taskId;
    union
    {
        BAPE_SampleRateChangedData sampleRateChanged;
        BAPE_FirstPtsData firstPts;
        BAPE_TsmFailData tsmFail;
    } data;
} BAPE_Interrupt;

/***************************************************************************
Summary:
Layout of the debug log buffer 
***************************************************************************/
typedef struct BAPE_DebugLogFifo {
    size_t writeCnt; /* number of characters written by the TP1, it could go backwards */
    size_t readCnt;  /* read couunter also used by the TP1 */
    size_t bufferSize;
    char buffer[2*4096]; /* buffer for messages */
} BAPE_DebugLogFifo;

/***************************************************************************
Summary:
Structure for IPC between processors
***************************************************************************/
#define BAPE_MAX_INTERRUPTS (8)
#define BAPE_DSP_DATA_BUFFER_SIZE (1024)
typedef struct BAPE_DspIpcBlock
{
    B_Kernel_HeapInfo heapInfo;
    unsigned maxModules;
    unsigned maxTasks;
    BAPE_Message message;
    BAPE_Reply reply;
    volatile uint32_t prevMessageNum;
    volatile uint32_t messageNum;
    volatile uint32_t prevReplyNum;
    volatile uint32_t replyNum;
    volatile uint32_t interruptRead;
    volatile uint32_t interruptWrite;
    BAPE_Interrupt interrupts[BAPE_MAX_INTERRUPTS];
    volatile uint32_t hostTicks;
    volatile uint32_t targetTicks;
    BAPE_TaskTsmSettings tsmSettings[BAPE_CHIP_MAX_DECODERS];
    uint32_t tsmSettingsSequence[BAPE_CHIP_MAX_DECODERS][2];
    BAPE_TaskTsmStatus tsmStatus[BAPE_CHIP_MAX_DECODERS];
    uint32_t tsmStatusSequence[BAPE_CHIP_MAX_DECODERS][2];
    BAPE_TaskDescriptor descriptors[BAPE_CHIP_MAX_DECODERS];
    uint32_t fmmSampleRate[BAPE_CHIP_MAX_DECODERS];
    uint8_t playbackStcValid[BAPE_CHIP_MAX_DECODERS];
    BAPE_DebugLogFifo *debugLog; /* not NULL if debug log is avaliable */
    uint8_t dataBuffer[BAPE_DSP_DATA_BUFFER_SIZE];
    unsigned size;          /* Structure size.  Better match on both sides or we have a big problem.  */
} BAPE_DspIpcBlock;


#ifdef BAPE_EMULATION
/***************************************************************************
Summary:
Handle an interrupt (Applications must call this)
 
Description: 
Only used in emulation environments for testing.  Call whenever a message 
reply is ready or an interrupt has been raised. 
***************************************************************************/
void BAPE_P_DspIpcInterrupt_isr(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Announce a message is ready (Applications must implement this) 
 
Description: 
Only used in emulation environments for testing 
***************************************************************************/
BERR_Code BAPE_P_DspMessageReady(
    BAPE_Handle handle
    );

/***************************************************************************
Summary:
Boot the DSP (Applications must implement this)
 
Description: 
Only used in emulation environments for testing 
***************************************************************************/
BERR_Code BAPE_P_DspBoot(
    BAPE_Handle handle,
    BAPE_DspIpcBlock *pIpc
    );

/***************************************************************************
Summary:
Halt the DSP (Applications must implement this)
 
Description: 
Only used in emulation environments for testing 
***************************************************************************/
void BAPE_P_DspHalt(
    BAPE_Handle handle
    );
#endif

#endif /* #ifndef BAPE_DSP_IPC_PRIV_H_ */


