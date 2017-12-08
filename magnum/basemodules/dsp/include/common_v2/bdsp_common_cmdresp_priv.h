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

#ifndef BDSP_COMMON_CMDRESP_PRIV_H_
#define BDSP_COMMON_CMDRESP_PRIV_H_

#define BDSP_ARM_SYSTEM_OPEN_COMMAND_ID ((uint64_t)0X524141474141524d )
#define BDSP_ARM_MAX_NUM_ROMFS_LIB           13
#define BDSP_ARM_MAX_REGISTER_REGIONS         4

#define BDSP_MAX_NUM_TASKS                    5 /* Maximum no.of Tasks to run on DSP */
#define BDSP_NUM_PROCESS_PER_TASK             2 /* Maximum no.of user process per task */

#define BDSP_MAX_NUM_SCHED_LEVELS (3) /* Total scheduling levels */
#define BDSP_MAX_NUM_USER_PROCESS BDSP_MAX_NUM_TASKS * BDSP_NUM_PROCESS_PER_TASK /* No.of user processs */

#define BDSP_PERIODIC_TIMER			2000 /*in micro seconds*/
#define BDSP_WATCHDOG_TIMER			 400 /*in milli seconds*/


typedef enum BDSP_ArmDspResp_Type
{
    BDSP_ArmDspResp_Type_eCmdAck,
    BDSP_ArmDspResp_Type_eEvent,
    BDSP_ArmDspResp_Type_eLast,
    BDSP_ArmDspResp_Type_eMax = 0x7FFFFFFF
}BDSP_ArmDspResp_Type;

typedef enum BDSP_ArmDspAck_Type
{
    BDSP_ArmDspAck_Type_eDevice,
    BDSP_ArmDspAck_Type_eTask,
    BDSP_ArmDspAck_Type_eLast,
    BDSP_ArmDspAck_Type_eMax = 0x7FFFFFFF
}BDSP_ArmDspAck_Type;

typedef enum BDSP_ArmDspCommandType
{
    BDSP_ArmDspCommandType_Open  = 1,  /* Arm System Open*/
    BDSP_ArmDspCommandType_Close = 2,  /* Arm System Close*/
    BDSP_ArmDspCommandType_Max,
    BDSP_ArmDspCommandType_Invalid = 0x7fffffff
}BDSP_ArmDspCommandType;

typedef enum BDSP_P_EventID{
   /* Audio IDS Events */
    BDSP_P_EventID_CDB_ITB_UNDERFLOW = 0,
    BDSP_P_EventID_CDB_ITB_OVERFLOW,
    BDSP_P_EventID_SAMPLING_RATE_CHANGE,
    BDSP_P_EventID_FRAME_SYNC_LOCK,
    BDSP_P_EventID_FRAME_SYNC_UNLOCK,
    BDSP_P_EventID_FIRST_PTS_RECEIVED,
    BDSP_P_EventID_TSM_FAIL,
    BDSP_P_EventID_ASTM_TSM_PASS,

    /* Video Encode IDS Events */
    BDSP_P_EventID_VIDEO_ENCODER_DATA_DISCARD,

    /* Algorithm (codec) Events */
    BDSP_P_EventID_ACMODE_CHANGE,
    BDSP_P_EventID_BIT_RATE_CHANGE,
    BDSP_P_EventID_STREAM_INFO_AVAILABLE,
    BDSP_P_EventID_UNLICENSED_ALGO,
    BDSP_P_EventID_ANCILLARY_DATA_AVAILABLE,
    BDSP_P_EventID_DIAL_NORM_CHANGE,
    BDSP_P_EventID_VOLUME_LEVEL_REACHED,

    /* Process specific Events */
    BDSP_P_EventID_ENCODER_OVERFLOW_EVENT,
    BDSP_P_EventID_ON_DEMAND_AUDIO_FRAME_DELIVERED,

    BDSP_P_EventID_MAX,
    BDSP_P_EventID_INVALID = 0x7FFFFFFF
}BDSP_P_EventID;


typedef enum BDSP_P_CommandID{
    BDSP_P_CommandID_INIT = 100,
    BDSP_P_CommandID_PING,
    BDSP_P_CommandID_START_TASK,
    BDSP_P_CommandID_STOP_TASK,
    BDSP_P_CommandID_PAUSE,
    BDSP_P_CommandID_RESUME,
    BDSP_P_CommandID_FRAME_ADVANCE,
    BDSP_P_CommandID_EVENT_NOTIFICATION,
    BDSP_P_CommandID_TSM_RECONFIG,
    BDSP_P_CommandID_DATASYNC_RECONFIG,
    BDSP_P_CommandID_ALGO_RECONFIG,
    BDSP_P_CommandID_CIT_RECONFIG,
    BDSP_P_CommandID_AUDIO_GAP_FILL_ENABLE,
    BDSP_P_CommandID_AUDIO_OUTPUT_FREEZE,
    BDSP_P_CommandID_AUDIO_OUTPUT_UNFREEZE,
    BDSP_P_CommandID_MAX,
    BDSP_P_CommandID_INVALID = 0x7FFFFFFF
}BDSP_P_CommandID;

typedef enum BDSP_P_ResponseType{
    BDSP_P_ResponseType_eNone = 0,
    BDSP_P_ResponseType_eRequired,
    BDSP_P_ResponseType_eLast,
    BDSP_P_ResponseType_eInvalid = 0x7FFFFFFF
} BDSP_P_ResponseType;

typedef enum BDSP_P_TaskType{
	BDSP_P_TaskType_eInterruptBased,
	BDSP_P_TaskType_eRealtime,
	BDSP_P_TaskType_eAssuredRate,
	BDSP_P_TaskType_eOnDemand,
	BDSP_P_TaskType_eAFAP,
    BDSP_P_TaskType_eLast,
    BDSP_P_TaskType_eInvalid = 0x7FFFFFFF
}BDSP_P_TaskType;

typedef enum BDSP_P_SchedulingMode
{
    BDSP_P_SchedulingMode_eMaster = 0,
    BDSP_P_SchedulingMode_eSlave,
    BDSP_P_SchedulingMode_eLast,
    BDSP_P_SchedulingMode_eInvalid = 0x7FFFFFFF
} BDSP_P_SchedulingMode;

typedef struct BDSP_P_CommandHeader
{
    BDSP_P_CommandID            eCommandID;                 /*  Command ID */
    uint32_t                    ui32CommandSizeInBytes;     /*  Command Size in Bytes */
    uint32_t                    ui32CommandCounter;         /*  Command Counter */
    uint32_t                    ui32TaskID;                 /*  Task ID*/
    uint32_t                    ui32CommandTimeStamp;       /*  CommandTimeStamp */
    BDSP_P_ResponseType         eResponseType;              /*  Response type */
} BDSP_P_CommandHeader;

typedef struct BDSP_Raaga_P_ROImageSizes
{
    uint32_t  ui32RORdbVarSize;       /* RDBVars size */
    uint32_t  ui32RORomfsSize;        /* Size of the initial ROMFS in the RO memory */
    uint32_t  ui32ROLoadableImgSize;  /* Size of loadable images */
    uint32_t  ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
} BDSP_Raaga_P_ROImageSizes;

typedef struct BDSP_Raaga_P_RWImageSizes
{
    uint32_t  ui32RWCommonMemSize;        /* Shared memory for CMD/RESP Q and Debug buffer */
    uint32_t  ui32RWSystemProcessMemSize; /* Memory for all top level Process  Custom MM, Default MM, SP, RP, Idle, Debug, File server, Process Manager */
    uint32_t  ui32RWDefaultMMMemSize;     /* Memory reserved for Default MM server */
    uint32_t  ui32RWCustomMMMemSize;      /* Total Work buffer, Num of User process * Mem for each user process */
} BDSP_Raaga_P_RWImageSizes;

typedef struct BDSP_Raaga_P_HeapLimitsInfo
{
    dramaddr_t HeapBaseAddr; /* Heap Start address */
    uint64_t   ui64HeapSize;     /* Heap size */
}BDSP_Raaga_P_HeapLimitsInfo;

typedef struct BDSP_Raaga_P_HeapInfo
{
    uint32_t ui32NumHeap; /* Number of heap ranges*/
    uint32_t ui32Dummy0; /* Number of heap ranges*/
    BDSP_Raaga_P_HeapLimitsInfo sHeapLimits[BDSP_RAAGA_MAX_NUM_HEAPS]; /* Entry for each heap range */
} BDSP_Raaga_P_HeapInfo;

typedef struct BDSP_P_CustomMMInfo
{
    uint64_t ui64WorkBufferStartAddr;
    uint32_t ui32TotalWorkBufferSize; /* Total work buffer memory required for all the processes */
    uint32_t ui32WorkBufferBlockSizePerLevel[BDSP_MAX_NUM_SCHED_LEVELS]; /* Working buffer block size */
    uint64_t ui64ProcessSpawnMemStartAddr;
    uint32_t ui32UserProcessSpawnMemSize;    /* Memory required to spawn a single user process */
    uint32_t ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
}BDSP_P_CustomMMInfo;

typedef struct BDSP_P_SchedulingInfo
{
    uint32_t  ui32NumCores;             /* number of cores in the DSP */
    uint32_t  ui32NumUserProcess;       /* number of user processes */
    uint32_t  ui32NumSchedulingLevels;  /* number of scheduling levels */

    uint32_t  ui32PreemptiveThreshold[BDSP_MAX_NUM_SCHED_LEVELS]; /* preempt threshold level for each scheduling level*/
}BDSP_P_SchedulingInfo;

typedef struct BDSP_P_TimerInfo
{
    uint32_t  ui32PeriodicTimerInUs;  /* Periodicity of Timer interrupt in microseconds */
    uint32_t  ui32WatchdogTimerinMs;  /* Watchdog timer value in ms*/
}BDSP_P_TimerInfo;

typedef struct BDSP_P_Raaga_InitCommand {
    BDSP_Raaga_P_ROImageSizes       sROImageSizeInfo;      /* RO image sizes */
    BDSP_Raaga_P_RWImageSizes       sRWImageSizeInfo;      /* RW image sizes */
    BDSP_Raaga_P_HeapInfo           sHeapInfo;             /* Nexus Heap Info */
    BDSP_P_CustomMMInfo             sCustomMMInfo;         /* Custom MM info */
    BDSP_P_SchedulingInfo           sSchedulingInfo;       /* Scheduler info */
    BDSP_P_TimerInfo                sTimerInfo;            /* Timer info for scheduler */
} BDSP_P_Raaga_InitCommand;

typedef struct BDSP_Arm_P_MemoryDescriptor{
    dramaddr_t  ui64StartAddress;
    uint64_t    ui64Size;
}BDSP_Arm_P_MemoryDescriptor;

typedef struct BDSP_Arm_P_CommonMemory
{
    BDSP_Arm_P_MemoryDescriptor sRODescriptor; /*sRdbVarsDescritor will be the first 1024 byte of RO memory, rest will be executable images*/
    BDSP_Arm_P_MemoryDescriptor sRWDescriptor;
    BDSP_Arm_P_MemoryDescriptor sIoMemoryDescriptor;
}BDSP_Arm_P_CommonMemory;

typedef struct BDSP_Arm_P_LibDescriptor
{
    dramaddr_t  ui64StartAddress;
    uint64_t    ui64Size;
    char        ui8LibName[BDSP_AF_P_MAX_LIB_NAME_SIZE];
}BDSP_Arm_P_LibDescriptor;

typedef struct BDSP_P_Arm_InitCommand {
    BDSP_Arm_P_CommonMemory         sCommonMemory;      /* RO image sizes */
    BDSP_P_CustomMMInfo             sCustomMMInfo;         /* Custom MM info */
    BDSP_P_SchedulingInfo           sSchedulingInfo;    /* Scheduler info */
    BDSP_P_TimerInfo                sTimerInfo;         /* Timer info for scheduler */
    uint64_t                        ui64NumRegRegions;  /* Number of register regions to Map */
    BDSP_Arm_P_MemoryDescriptor     RegisterRegions[BDSP_ARM_MAX_REGISTER_REGIONS];  /* Details of the Register Regions */
} BDSP_P_Arm_InitCommand;



typedef struct BDSP_P_StartTaskCommand{
	BDSP_P_SchedulingMode          eSchedulingMode;		   	/* Master/Slave*/
	BDSP_P_TaskType                eTaskType;				/*Interrupt/RT/Assured/OnDemand/AFAP*/
	uint32_t					   ui32SchedulingLevel;     /* Scheduling level*/
	uint32_t					   ui32TaskId;				/*Task ID*/

	uint32_t                       ui32MasterTaskId;        /*ID of master task*/
    uint32_t                       ui32SyncQueueFifoId;		/* Task Sync queue Id */
    uint32_t                       ui32AsyncQueueFifoId;   	/* Task Async queue Id */
	uint32_t                       ui32EventEnableMask;     /* Event Mask Enable */

	BDSP_P_MemoryInfo              sConfigMemoryInfo;       /* CIT information*/
	BDSP_P_MemoryInfo			   sTaskMemoryInfo;			/* Info of the memory allocated for Task*/
	BDSP_P_MemoryInfo			   sPrimaryStageMemoryInfo; /* Info of the memory allocated for Primary Stage*/
	BDSP_P_MemoryInfo			   sSharedMemoryInfo;	    /* Info of the memory allocated for MP/AP sharing*/
}BDSP_P_StartTaskCommand;

/*TODO Clean this command structure as and when it is implemented*/
typedef struct BDSP_P_FrameAdvanceCommand{
	uint32_t				   ui32DurationOfFrameAdv;
    uint32_t  ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
}BDSP_P_FrameAdvanceCommand;

typedef struct BDSP_P_EventEnableDisableCommand{
	uint32_t					   ui32EnableEvent;
    uint32_t  ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
}BDSP_P_EventEnableDisableCommand;

typedef struct BDSP_P_TsmReconfigCommand{
	BDSP_AudioTaskTsmSettings      sTsmSettings;
}BDSP_P_TsmReconfigCommand;

typedef struct BDSP_P_DataSyncReconfigCommand{
	BDSP_AudioTaskDatasyncSettings	sDataSyncSettings;
}BDSP_P_DataSyncReconfigCommand;

typedef struct BDSP_P_AlgoReconfigCommand{
	BDSP_Algorithm				   eAlgorithm;
    uint32_t  ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
	BDSP_P_MemoryInfo			   sStageMemoryInfo;
	BDSP_P_MemoryInfo			   sHostConfigMemoryInfo;
	BDSP_P_MemoryInfo			   sFwConfigMemoryInfo;
}BDSP_P_AlgoReconfigCommand;

typedef struct BDSP_P_CitReconfigCommand{
	BDSP_P_MemoryInfo				sHostConfigMemoryInfo;
	BDSP_P_MemoryInfo				sFwConfigMemoryInfo;
}BDSP_P_CitReconfigCommand;

typedef struct BDSP_P_AudioOutputFreezeCommand{
    uint32_t        ui32StcHiAddress ;                  /* STC Hi Address*/
    uint32_t        ui32StcLoAddress ;                  /* STC Low Address*/
    dramaddr_t      ui32AudFmmOutputPortAddr;           /* RDB address to read modify
                                                           write to stop FMM consumption*/
    uint32_t        ui32AudFmmOutputPortMask;           /* Mask value for the above address.
                                                           FW will mask these bits and write
                                                           into the remaining bits a value
                                                           to stop FMM consumption */
    uint32_t        ui32AudFmmOutputPortValue;          /* Value to be written in the ~Mask
                                                           in the above address */

}BDSP_P_AudioOutputFreezeCommand;

typedef struct BDSP_P_AudioOutputUnFreezeCommand{
    uint32_t        ui32StcHiAddress ;                  /* STC Hi Address*/
    uint32_t        ui32StcLoAddress ;                  /* STC Low Address*/
    dramaddr_t      ui32AudFmmOutputPortAddr;           /* RDB address to read modify
                                                           write to stop FMM consumption*/
    uint32_t        ui32AudFmmOutputPortMask;           /* Mask value for the above address.
                                                           FW will mask these bits and write
                                                           into the remaining bits a value
                                                           to stop FMM consumption */
    uint32_t        ui32AudFmmOutputPortValue;          /* Value to be written in the ~Mask
                                                           in the above address */
    uint32_t        ui32NumDummySinkBuffers;            /* Number of buffers */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER  sCircBuffer[1];    /* Circular Buffer Address for dummy sink buffer 0.
                                                           FW will add 24 bytes for other associated buffers */

    uint32_t  ui32Dummy1;			                    /* Element added to make structure size 64 bit aligned*/

}BDSP_P_AudioOutputUnFreezeCommand;

typedef struct BDSP_P_Command
{
    BDSP_P_CommandHeader   sCommandHeader;
    union{
        BDSP_P_Raaga_InitCommand            sRaagaInitCommand;
		BDSP_P_Arm_InitCommand              sArmInitCommand;
		BDSP_P_StartTaskCommand             sStartTask;
		BDSP_P_FrameAdvanceCommand          sFrameAdvance;
		BDSP_P_EventEnableDisableCommand    sEventEnableDisable;
		BDSP_P_TsmReconfigCommand           sTsmReconfig;
		BDSP_P_DataSyncReconfigCommand      sDataSyncReconfig;
		BDSP_P_AlgoReconfigCommand          sAlgoReconfig;
		BDSP_P_CitReconfigCommand           sCitReconfigure;
		BDSP_P_AudioOutputFreezeCommand     sAudioOutputFreeze;
		BDSP_P_AudioOutputUnFreezeCommand   sAudioOutputUnFreeze;
    }uCommand;
} BDSP_P_Command;

typedef struct BDSP_P_ResponseHeader
{
    BDSP_P_CommandID	eCommandID;               /*  Response for Command ID */
    uint32_t            ui32ResponseSizeInBytes;  /*  Response Size in Bytes */
    uint32_t            ui32ResponseCounter;      /*  Response Counter */
    uint32_t            ui32TaskID;               /*  Task ID */

    uint32_t            eStatus;                  /*  Status of completion */
    uint32_t            ui32Dummy0;			      /* Element added to make structure size 64 bit aligned*/
} BDSP_P_ResponseHeader;

typedef struct BDSP_P_Response
{
    BDSP_P_ResponseHeader     sResponseHeader;
} BDSP_P_Response;

typedef struct BDSP_P_AsyncHeader
{
    BDSP_P_EventID    eEventID;             /* Event ID */
    uint32_t          ui32MsgSize;          /* Size of Message in bytes */
    uint32_t          ui32TaskID;           /* Task ID for which the message occurs */
    uint32_t		  ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
}BDSP_P_AsynHeader;

typedef struct BDSP_P_SampleRateChangeInfo
{
	uint32_t	ui32BaseSampleRate;
	uint32_t    ui32StreamSampleRate;
}BDSP_P_SampleRateChangeInfo;

typedef struct BDSP_P_PtsInfo
{
    uint32_t        ui32RunningPts;       /* The PTS value of the frame going
                                                                              out the next vsync
                                                                              MPEG/DirecTv HD - 45KHz domain
                                                                              DirecTv SD- 27MHz domain */
    int32_t         i32Pts2StcPhase;      /* Difference between current PTS
                                                                              and the STC */
    int32_t         i32TSMUpperThreshold; /*TSM Upper threshold*/
    BDSP_PtsType    ePtsType;             /* The PTS type tag */

}BDSP_P_PtsInfo;

typedef struct BDSP_P_AcmodeChangeInfo
{
    uint32_t    ui32ModeValue;
    uint32_t	ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
} BDSP_P_AcmodeChangeInfo;

typedef struct BDSP_P_BitrateChangeInfo
{
    uint32_t    ui32BitRate;            /* New Bit Rate value*/
    uint32_t    ui32BitRateIndex;       /* This has the Bit rate index
                                           as given in the standard. This value
                                           is zero for audio type AAC-HE*/
}BDSP_P_BitrateChangeInfo;

typedef struct BDSP_P_UnlicensedAlgoInfo
{
    uint32_t        ui32AudioAlgorithm; /* Algorithm Name */
    uint32_t		ui32Dummy0;			  /* Element added to make structure size 64 bit aligned*/
} BDSP_P_UnlicensedAlgoInfo;

typedef struct BDSP_P_AsynMsg
{
	BDSP_P_AsynHeader sAsynHeader;
	union{
		BDSP_P_SampleRateChangeInfo     sSampleRateChangeInfo;
		BDSP_P_PtsInfo	                sPtsInfo;
		BDSP_P_AcmodeChangeInfo         sAcmodeChangeInfo;
		BDSP_P_BitrateChangeInfo        sBitrateChangeInfo;
		BDSP_P_UnlicensedAlgoInfo       sUnlicensedAlgoInfo;
	}uInfo;
}BDSP_P_AsynMsg;

typedef struct BDSP_ArmSystemOpenCommand
{
    uint64_t     ui64identifier;/*0X52414147_4141524d -> ARM*/
    dramaddr_t   host2DspCommandQueueOffset;
    dramaddr_t   genericResponseQueueOffset;
}BDSP_ArmSystemOpenCommand;

typedef struct BDSP_ArmDspCommand{
    BDSP_ArmDspCommandType     eCommandType;
    BDSP_ArmSystemOpenCommand  sOpenCommand;
}BDSP_ArmDspCommand;


/***************************************************************************
Summary:
    This structure is used while sending ack to BDSP.

Description:
    This is the header for Ack by Astra

See Also:
****************************************************************************/
typedef struct BDSP_ArmDspAck
{
    BDSP_ArmDspAck_Type    eAckType;           /*  type of Ack from Arm */
    BDSP_ArmDspResp_Type   ui32RespType;       /* Additional Data */
    uint32_t               ui32DspIndex;
    uint32_t               ui32TaskID;         /*  Task ID */
} BDSP_ArmDspAck;

#endif /*BDSP_COMMON_CMDRESP_PRIV_H_*/
