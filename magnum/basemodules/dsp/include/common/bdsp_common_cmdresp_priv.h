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


#ifndef BDSP_COMMON_CMDRESP_PRIV_H__
#define BDSP_COMMON_CMDRESP_PRIV_H__

#define BDSP_COMMAND_ID_OFFSET                        ((uint32_t)0x100)

/*** These are unique Command Ids used by Host ***/

#define BDSP_START_TASK_COMMAND_ID   \
        ((uint32_t)((BDSP_COMMAND_ID_OFFSET) + 1))                             /** 0x101 **/

#define BDSP_STOP_TASK_COMMAND_ID    \
        ((uint32_t)((BDSP_START_TASK_COMMAND_ID) + 1))                         /** 0x102 **/

#define BDSP_ALGO_PARAMS_CFG_COMMAND_ID  \
        ((uint32_t)((BDSP_STOP_TASK_COMMAND_ID) + 1))                          /** 0x103 **/

#define BDSP_PAUSE_COMMAND_ID        \
        ((uint32_t)((BDSP_ALGO_PARAMS_CFG_COMMAND_ID) + 1))                    /** 0x104 **/

#define BDSP_RESUME_COMMAND_ID       \
        ((uint32_t)((BDSP_PAUSE_COMMAND_ID) + 1))                               /** 0x105 **/

#define BDSP_FRAME_ADVANCE_COMMAND_ID        \
        ((uint32_t)((BDSP_RESUME_COMMAND_ID) + 1))                              /** 0x106 **/

#define BDSP_EVENT_NOTIFICATION_COMMAND_ID       \
        ((uint32_t)((BDSP_FRAME_ADVANCE_COMMAND_ID ) + 1))                     /** 0x107 **/

#define BDSP_FMMPORT_RECFG_COMMAND_ID       \
        ((uint32_t)((BDSP_EVENT_NOTIFICATION_COMMAND_ID ) + 1))                /** 0x108 **/

#define BDSP_PING_COMMAND_ID       \
        ((uint32_t)((BDSP_FMMPORT_RECFG_COMMAND_ID ) + 1))                     /** 0x109 **/

#define BDSP_RECONFIGURATION_COMMAND_ID       \
        ((uint32_t)((BDSP_PING_COMMAND_ID ) + 1))                               /** 0x10A **/

#define BDSP_AUDIO_OUTPUT_FREEZE_COMMAND_ID       \
        ((uint32_t)((BDSP_RECONFIGURATION_COMMAND_ID ) + 1))                    /** 0x10B **/

#define BDSP_AUDIO_OUTPUT_UNFREEZE_COMMAND_ID       \
        ((uint32_t)((BDSP_AUDIO_OUTPUT_FREEZE_COMMAND_ID ) + 1))               /** 0x10C **/

/***************************************************************************
Summary:
    Enum indicating the Algo type.

See Also:
****************************************************************************/

typedef enum BDSP_P_AlgoType
{
    BDSP_P_AlgoType_eEncode = 10,
    BDSP_P_AlgoType_eDecode,
    BDSP_P_AlgoType_eTranscode,
    BDSP_P_AlgoType_ePassThru,
    BDSP_P_AlgoType_eVideoDecode,
    BDSP_P_AlgoType_eVideoEncode,
    BDSP_P_AlgoType_eScm,
    BDSP_P_AlgoType_eLast,
    BDSP_P_AlgoType_eInvalid = 0x7FFFFFFF

} BDSP_P_AlgoType;


/***************************************************************************
Summary:
    Enum indicating the task type.

See Also:
****************************************************************************/

typedef enum BDSP_P_TaskType
{
    BDSP_P_TaskType_eNonRealtime = 0,
    BDSP_P_TaskType_eSoftRealtime,
    BDSP_P_TaskType_eRealtime,
    BDSP_P_TaskType_eOnDemand,

    BDSP_P_TaskType_eLast,
    BDSP_P_TaskType_eInvalid = 0x7FFFFFFF

} BDSP_P_TaskType;

/***************************************************************************
Summary:
    Enum indicating the Scheduling mode for the task.

See Also:
****************************************************************************/

typedef enum BDSP_P_SchedulingMode
{
    BDSP_P_SchedulingMode_eMaster = 0,
    BDSP_P_SchedulingMode_eSlave,

    BDSP_P_SchedulingMode_eLast,
    BDSP_P_SchedulingMode_eInvalid = 0x7FFFFFFF

} BDSP_P_SchedulingMode;

/***************************************************************************
Summary:
    Enum indicating the type of the deadline computation function to be used
    for the task.

See Also:
****************************************************************************/

typedef enum BDSP_P_DeadLineComputeFuncType
{
    BDSP_P_DeadLineComputeFuncType_eRealtimeDecode = 0,
    BDSP_P_DeadLineComputeFuncType_eRealtimeEncode,
    BDSP_P_DeadLineComputeFuncType_eNonRealtimeDecode,
    BDSP_P_DeadLineComputeFuncType_eNonRealtimeEncode,
    BDSP_P_DeadLineComputeFuncType_ePassthrough,
    BDSP_P_DeadLineComputeFuncType_eScmTask,
    BDSP_P_DeadLineComputeFuncType_eOnDemand,
    BDSP_P_DeadLineComputeFuncType_eMax,
    BDSP_P_DeadLineComputeFuncType_eInvalid = 0x7FFFFFFF

} BDSP_P_DeadLineComputeFuncType;

/***************************************************************************
Summary:
    Enum indicating whether Ack or Response required by Host

See Also:
****************************************************************************/

typedef enum BDSP_P_ResponseType
{
    BDSP_P_ResponseType_eAckRequired = 0,
    BDSP_P_ResponseType_eResponseRequired,
    BDSP_P_ResponseType_eAckAndResponseBothRequired,
    BDSP_P_ResponseType_eNone,
    BDSP_P_ResponseType_eLast,
    BDSP_P_ResponseType_eInvalid = 0x7FFFFFFF

} BDSP_P_ResponseType;

/***************************************************************************
Summary:
    Structure holding the common header for all commands between the host
    and the DSP.

Description:
    This is the common header for all Commands from the host.

See Also:
****************************************************************************/

typedef struct BDSP_P_CommandHeader
{
    uint32_t                    ui32CommandID;              /*  Command ID */
    uint32_t                    ui32CommandSizeInBytes;     /*  Command Size in Bytes */
    uint32_t                    ui32CommandCounter;         /*  Command Counter */
    uint32_t                    ui32TaskID;                 /*  Task ID*/
    uint32_t                    ui32CommandTimeStamp;       /*  CommandTimeStamp */
    BDSP_P_ResponseType         eResponseType;              /*  Ack or response required */
} BDSP_P_CommandHeader;

/***************************************************************************
Summary:
    This structure holds common header for both Ack and responses sent by
    the DSP to the host in synchronous queue.
    Only header is sufficient for sending any specifc Acks to the Host.

Description:
    This is the common header for Ack and Response messages sent by DSP;

See Also:
****************************************************************************/
typedef struct BDSP_P_CommonAckResponseHeader
{
    uint32_t                    ui32ResponseID;             /*  Response ID */
    uint32_t                    ui32ResponseSizeInBytes;    /*  Response Size in Bytes */
    uint32_t                    ui32ResponseCounter;        /*  Response Counter */
    uint32_t                    ui32TaskID;                 /*  Task ID */
    uint32_t                    eStatus;                    /*  Status of completion */
} BDSP_P_CommonAckResponseHeader;

/***************************************************************************
Summary:
     Structure for async message header for any event

Description:

See Also:
****************************************************************************/

typedef struct BDSP_P_AsynMSGHeader
{
    uint32_t    ui32EventID;                /* Event ID */
    uint32_t    ui32MsgSize;                /* Size of Message in bytes */
    uint32_t    ui32TaskID;                 /* Task ID for which the message occurs */
}BDSP_P_AsynMSGHeader;

/***************************************************************************
Summary:
    This structure holds all the information required by DSP for scheduling.

     This structure is derived from the BDSP_P_TaskParamInfo by ATM.
     DSP uses this structure internally

See Also:
****************************************************************************/

typedef union BDSP_P_DspSchedulingBuffInfo
{
    dramaddr_t                          ui32DramSchedulingBuffCfgAddr;  /*   Handle for the Scheduling Buffer.
                                                                                                                                    Used only for BDSP_AF_P_BufferType_eDRAM*/
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER     sRdbBasedSchedulingBuffer;      /*   Used only for buffer of type FMM/CDB/RDB */

}BDSP_P_DspSchedulingBuffInfo;

typedef struct BDSP_P_NonRTTaskParamInfo
{
    uint32_t    ui32MinTimeThreshold;
    uint32_t    ui32MaxTimeSlice;
}BDSP_P_NonRTTaskParamInfo;

/***************************************************************************
Summary:
    This structure holds all the information required by DSP for scheduling
    the newly created task by Host

See Also:
****************************************************************************/

typedef struct BDSP_P_TaskParamInfo
{
   uint32_t                            ui32SamplingFrequency;
   uint32_t                            ui32FrameSize;
   uint32_t                            ui32SoftRealTimeThreshold;
   uint32_t                            ui32SchedulingBufferThreshold;
   uint32_t                            ui32BlockTime;

   /*  Following Circ buffer details are used for task scheduling by FW CPT,
    also this is used by TSM for Buffer delay computation.

    PI will populates the 'sRdbBasedSchedulingBuffer' for RDB based buffers.

    Incase of DRAM intertask buffer as the scheduling buffer,
    PI will take Buffer Handle of the intertask Dram buffer and hook to ui32DramSchedulingBuffCfgAddr
    */

    BDSP_AF_P_BufferType                eBufferType;
    BDSP_P_DspSchedulingBuffInfo        sDspSchedulingBuffInfo;

    /*
    Feedback buffer from the Master Task to the Slave Tasks.

    Currently we support only one feedback buffer from the Master Task to all its slave tasks.

    The concept of Feedback path exists only in the Master Slave configuration and the feed back path
    becomes invalid in the single decode usage modes. PI has to make Valid/Invalid field according.

    PI allocates the Feedback buffer when a master task is started and allocates a DRAM memory of
    type BDSP_AF_P_sSINGLE_CIRC_BUFFER to populate the circullar buffer details.  PI passes the base address
    of this DRAM memory to FW as Handle so that FW can take care of all kinds of buffers including
    the BDSP_AF_P_BufferType_eDRAM.

    PI gives the same handle for all the slave tasks of a master task..

    The 'ui32MasterTaskFeedbackBuffCfgAddr' is the converted address...

    */
   uint32_t                             ui32MasterTaskFeedbackBuffValid;    /* Valid =1; Invalid 0 */
   dramaddr_t                           ui32MasterTaskFeedbackBuffCfgAddr;

   uint32_t                             ui32MaxIndepDelay ;
   uint32_t                             ui32IndepDelay ;
   BDSP_P_NonRTTaskParamInfo            sNonRealTimeTaskParams;


   BDSP_AF_P_FmmDstFsRate               eFmmDstFsRate; /* The sinking rate of Scheduling buffer.
                                                                                            The sinking buffer should be selected as the fastest sinking buffer in the task.
                                                                                            This will ensure the jitter in 4X rate FMM buffers getting accounted for scheduling */
}BDSP_P_TaskParamInfo;

/***************************************************************************
Summary:
    Start task command structure.

Description:
    This is the command structure of the start task command to be issued by
    the host on start decode.

See Also:
****************************************************************************/
typedef struct BDSP_P_StartTaskCommand
{
    BDSP_P_AlgoType                     eTaskAlgoType;                     /*  Algorithm type enum */
    BDSP_P_TaskType                     eTaskType;                         /*  Task type:Real Time,
                                                                                                                                        Non real time */
    BDSP_P_SchedulingMode               eSchedulingMode;
    uint32_t                            ui32MasterTaskId;

    BDSP_P_DeadLineComputeFuncType      eDeadlineComputationFuncType;
    dramaddr_t                          ui32DramDeadlineConfigStructAddr;  /* This is pointer to this structure
                                                                                                                                       BDSP_P_TaskParamInfo in DRAM */
    dramaddr_t                          ui32DramTaskConfigAddr;            /* Task config data structure address
                                                                                                                                       from CIT module */
    uint32_t                            ui32SyncQueueId;                   /* Task Sync queue Id */
    uint32_t                            ui32AsyncQueueId;                  /* Task Async queue Id */
    BDSP_AF_P_sDRAM_BUFFER              sDramStackBuffer;                  /* Stack DRAM address for stack swapping for each task */
    uint32_t                            ui32EventEnableMask;

    BDSP_AF_P_EnableDisable             ePPMCorrEnable;                    /* PPM correction enable or not */
    BDSP_AF_P_EnableDisable             eOpenGateAtStart;                  /* Open gate at start task, do not wait for the first valid frame
                                                                                                                                        to be found before starting kick starting the FMM data consumption */
    BDSP_AF_P_EnableDisable             eZeroPhaseCorrEnable;              /*Flag to disable zero phase audio for low delay cases*/
    BDSP_AF_P_eSchedulingGroup          eSchedulingGroup;                  /* Indicate the scheduling group in which CPT needs to schedule the task*/

} BDSP_P_StartTaskCommand;

/***************************************************************************
Summary:
    Start task command structure.

Description:
    This is the command structure of the stop task command to be issued by
    the host on stoping a task.

See Also:
****************************************************************************/
typedef struct BDSP_P_StopTaskCommand
{
    BDSP_P_TaskType                     eTaskType;                         /* Task Type*/
    BDSP_P_SchedulingMode               eSchedulingMode;
    uint32_t                            ui32MasterTaskId;
}BDSP_P_StopTaskCommand;

/***************************************************************************
Summary:
    Generic configuration change command structure.

Description:
    Any configuration change command will have a generic structure. The
    configuration change will only have three parameters. The DRAM address
    location where the host has changed the configuration. The DRAM location
    currently being used by the DSP and the size of the configuration
    structure.

See Also:
****************************************************************************/

typedef struct BDSP_P_CfgChangeCommand
{
    dramaddr_t                  ui32HostConfigParamBuffAddr;
    dramaddr_t                  ui32DspConfigParamBuffAddr;
    uint32_t                    ui32SizeInBytes;

} BDSP_P_CfgChangeCommand;

/***************************************************************************
Summary:
     Frame Advance command structure.

Description:

See Also:
****************************************************************************/

typedef struct BDSP_P_FrameAdvanceCommand
{
    BDSP_TIME_45KHZ_TICKS        ui32DurationOfFrameAdv; /*  Duration for the frame advance */
} BDSP_P_FrameAdvanceCommand;

/***************************************************************************
Summary:
     Event enable/disable command structure.

Description:

See Also:
****************************************************************************/

typedef struct  BDSP_P_EventEnableDisable
{
    uint32_t ui32EnableEvent; /* Use  BDSP_P_EventIdMask for enabling or
                                                         disabling specfic events by Host, We need to
                                                         take care of starting 0x301 offset for event Ids */
} BDSP_P_EventEnableDisable;

/***************************************************************************
Summary:
    Fmm-Port Reconfig Command configuration change command structure.

Description:

    This structure contain two DRAM Addresses.
        a) ui32NewFMMPortBuffAdd :    Buffer that contains new FMM Buffer
                                    port cfg.
                                    This buffer is a temporary buffer
                                    allocated in PI.

        b) ui32TaskFMMPortCfgBuffAddr : The task FMM port cfg addr that was
                                    allocated by CIT.


    Upon the FMM port Reconfiguration command, the FW is expected to copy
    the data in ui32NewFMMPortBuffAdd to ui32TaskFMMPortCfgBuffAddr and
    perform the FMM port Reconfiguration.

    FW will send a response once the Command operations are done....

See Also:
****************************************************************************/

typedef struct BDSP_P_FmmPortReconfigCommand
{
    uint32_t                    ui32NewFMMPortBuffAdd;
    uint32_t                    ui32TaskFMMPortCfgBuffAddr;
    uint32_t                    ui32SizeInBytes;

} BDSP_P_FmmPortReconfigCommand;

/***************************************************************************
Summary:
     Ping command structure.

Description:

See Also:
****************************************************************************/

typedef struct  BDSP_P_PingCommand
{
    uint32_t    ui32AckQueueId; /* Ack queue Id for ping command */
} BDSP_P_PingCommand;

/***************************************************************************
Summary:
    Cit Reconfiguration Command

Description:
    This structure contain two DRAM Addresses
        a) ui32ModifiedCitAddr     :Buffer that contains new CIT data structure
                                  This buffer is a temporary buffer
                                  allocated in PI.

        b) ui32RunningTaskCitAddr: The CIT data structure of the running task

        All these addresses should be Converted Address.

        and

        c) CIT Datastructure Size

See Also:
****************************************************************************/

typedef struct BDSP_P_CitReconfigCommand
{
    dramaddr_t              ui32ModifiedCitAddr;
    dramaddr_t              ui32RunningTaskCitAddr;
    uint32_t                ui32SizeInBytes;

} BDSP_P_CitReconfigCommand;



/***************************************************************************
Summary:
     Frame Advance Response structure.

Description:

See Also:
****************************************************************************/

typedef struct BDSP_P_FrameAdvanceResponse
{
    BDSP_TIME_45KHZ_TICKS    ui32DurationOfFrmAdvCompleted;  /*  Duration for the frame
                                                                                                            advance completed */
} BDSP_P_FrameAdvanceResponse;

/***************************************************************************
Summary:
    Command to Freeze the FMM output consumption and snapshot STC

Description:

    This is the command structure sent from the host for the DSP to snapshot
    the STC value and the stop FMM consumption.

See Also: BDSP_P_AudioOutputUnFreezeCommand
****************************************************************************/
typedef struct BDSP_P_AudioOutputFreezeCommand
{
    uint32_t            ui32StcHiAddress ;                  /*STC Hi Address*/
    uint32_t            ui32StcLoAddress ;                  /*STC Low Address*/
    dramaddr_t          ui32AudFmmOutputPortAddr;           /*RDB address to read modify*/
                                                    /*write to stop FMM consumption*/
    uint32_t            ui32AudFmmOutputPortMask;           /*Mask value for the above address.*/
                                                    /*FW will mask these bits and write*/
                                                    /*into the remaining bits a value*/
                                                    /*to stop FMM consumption */
    uint32_t            ui32AudFmmOutputPortValue;          /*Value to be written in the ~Mask*/
                                                    /*in the above address */

} BDSP_P_AudioOutputFreezeCommand;

typedef struct BDSP_P_AudioOutputUnFreezeHostToDspCommand
{
    uint32_t        ui32StcHiAddress ;                  /*STC Hi Address*/
    uint32_t        ui32StcLoAddress ;                  /*STC Low Address*/
    dramaddr_t      ui32AudFmmOutputPortAddr;           /*RDB address to read modify*/
                                                    /*write to stop FMM consumption*/
    uint32_t        ui32AudFmmOutputPortMask;           /*Mask value for the above address.*/
                                                    /*FW will mask these bits and write*/
                                                    /*into the remaining bits a value*/
                                                    /*to stop FMM consumption */
    uint32_t        ui32AudFmmOutputPortValue;          /*Value to be written in the ~Mask*/
                                                    /*in the above address */
    uint32_t        ui32NumDummySinkBuffers;            /* Number of buffers */
    BDSP_AF_P_sDRAM_CIRCULAR_BUFFER  sCircBuffer[1];/* Circular Buffer Address for dummy sink buffer 0.
                                                                                            FW will add 24 bytes for other associated buffers */
}BDSP_P_AudioOutputUnFreezeHostToDspCommand;

/***************************************************************************
Summary:
    Command to UnFreeze the FMM output consumption and snapshot STC

Description:

    This is the command structure sent from the host for the DSP to snapshot
    the STC value and the resume FMM consumption.

See Also: BDSP_P_AudioOutputFreezeCommand
****************************************************************************/
typedef union BDSP_P_AudioOutputUnFreezeCommand
{
    BDSP_P_AudioOutputUnFreezeHostToDspCommand sUnFreezeHostToDspCmd;

} BDSP_P_AudioOutputUnFreezeCommand;


#endif /*BDSP_COMMON_CMDRESP_PRIV_H__*/
