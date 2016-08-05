/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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


#ifndef BDSP_ARM_CMDRESP_PRIV_H__
#define BDSP_ARM_CMDRESP_PRIV_H__

#include "bdsp_common_cmdresp_priv.h"

 /* The following are the various ID's given to each command
from host to any task on DSP */

#define BDSP_ARM_ASYNC_EVENT_ID_OFFSET                    ((uint32_t)0x0)

#define BDSP_ARM_INVALID_COMMAND                          ((uint32_t)-1)

#define BDSP_ARM_COMMAND_ID_OFFSET                        ((uint32_t)0x130)

#define BDSP_ARM_ACK_ID_OFFSET                            ((uint32_t)0x200) /* TO DO - CDN */

#define BDSP_ARM_RESPONSE_ID_OFFSET                       ((uint32_t)0x300)

#define BDSP_ARM_DEFAULT_EVENTS_ENABLE_MASK               ((uint32_t)0x3)

    /** This is the starting Task ID offset which can be
        used by Host to create any    new tasks in future **/
#define BDSP_ARM_TASK_ID_START_OFFSET                     ((uint32_t)115)

    /** This is the maximum value of taskID which can be
        used by host in future **/
#define BDSP_ARM_TASK_MAX_ID_AVAILABLE                    ((uint32_t)156)


/*** Following are the various Event Ids used  ***/

#define BDSP_ARM_DUMMY_FRAME_GENERATION_EVENT_ID  \
        ((uint32_t)((BDSP_ARM_ASYNC_EVENT_ID_OFFSET) )  )                             /** 0x00 **/

#define BDSP_ARM_FRAME_REPEAT_EVENT_ID    \
        ((uint32_t)((BDSP_ARM_DUMMY_FRAME_GENERATION_EVENT_ID) + 1)   )               /** 0x01 **/

#define BDSP_ARM_FRAME_DROP_FROM_CDB_EVENT_ID \
        ((uint32_t)((BDSP_ARM_FRAME_REPEAT_EVENT_ID) + 1) )                           /** 0x02 **/

#define BDSP_ARM_TSM_FAIL_EVENT_ID    \
        ((uint32_t)((BDSP_ARM_FRAME_DROP_FROM_CDB_EVENT_ID) + 1))                     /** 0x03 **/

#define BDSP_ARM_CDB_ITB_UNDERFLOW_EVENT_ID   \
        ((uint32_t)((BDSP_ARM_TSM_FAIL_EVENT_ID) + 1) )                               /** 0x04 **/

#define BDSP_ARM_CDB_ITB_OVERFLOW_EVENT_ID    \
        ((uint32_t)((BDSP_ARM_CDB_ITB_UNDERFLOW_EVENT_ID) + 1)    )                   /** 0x05 **/

#define BDSP_ARM_CRC_ERROR_EVENT_ID       \
        ((uint32_t)((BDSP_ARM_CDB_ITB_OVERFLOW_EVENT_ID) + 1) )                       /** 0x06 **/

#define BDSP_ARM_AUDIO_MODE_CHANGE_EVENT_ID   \
        ((uint32_t)((BDSP_ARM_CRC_ERROR_EVENT_ID) + 1)    )                           /** 0x07 **/

#define BDSP_ARM_BIT_RATE_CHANGE_EVENT_ID \
        ((uint32_t)((BDSP_ARM_AUDIO_MODE_CHANGE_EVENT_ID) + 1)    )                   /** 0x08 **/

#define BDSP_ARM_SAMPLING_RATE_CHANGE_EVENT_ID    \
        ((uint32_t)((BDSP_ARM_BIT_RATE_CHANGE_EVENT_ID) + 1)  )                       /** 0x09 **/

#define BDSP_ARM_FRAME_SYNC_LOCK_EVENT_ID \
        ((uint32_t)((BDSP_ARM_SAMPLING_RATE_CHANGE_EVENT_ID) + 1))                    /** 0x0A */

#define BDSP_ARM_FRAME_SYNC_LOCK_LOST_EVENT_ID    \
        ((uint32_t)((BDSP_ARM_FRAME_SYNC_LOCK_EVENT_ID) + 1))                         /** 0x0B **/

#define BDSP_ARM_STC_PTS_DIFF_LOWER_THRESHOLD_EVENT_ID    \
        ((uint32_t)((BDSP_ARM_FRAME_SYNC_LOCK_LOST_EVENT_ID) + 1))                    /** 0x0C */

#define BDSP_ARM_STC_PTS_DIFF_UPPER_THRESHOLD_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_STC_PTS_DIFF_LOWER_THRESHOLD_EVENT_ID) + 1))                /** 0x0D **/

#define BDSP_ARM_DESIRED_CDB_BUFFER_LEVEL_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_STC_PTS_DIFF_UPPER_THRESHOLD_EVENT_ID) + 1))                /** 0x0E **/

#define BDSP_ARM_FIRST_PTS_RECEIVED_FROM_ITB_EVENT_ID \
    ((uint32_t)((BDSP_ARM_DESIRED_CDB_BUFFER_LEVEL_EVENT_ID) + 1))                    /** 0x0F **/

#define BDSP_ARM_PTS_ERR_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_FIRST_PTS_RECEIVED_FROM_ITB_EVENT_ID) + 1))                 /** 0x10 **/

#define BDSP_ARM_TSM_LOCK_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_PTS_ERR_EVENT_ID) + 1))                                     /** 0x11 **/

#define BDSP_ARM_START_PTS_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_TSM_LOCK_EVENT_ID) + 1))                                    /** 0x12 **/

#define BDSP_ARM_STOP_PTS_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_START_PTS_EVENT_ID) + 1))                                   /** 0x13 **/

#define BDSP_ARM_ASTMTSM_PASS_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_STOP_PTS_EVENT_ID) + 1))                                    /** 0x14 **/

#define BDSP_ARM_RAMP_ENABLE_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_ASTMTSM_PASS_EVENT_ID) + 1))                                /** 0x15 **/

#define BDSP_ARM_CDB_DATA_AVAIL_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_RAMP_ENABLE_EVENT_ID) + 1))                                 /** 0x16 **/

#define BDSP_ARM_STREAM_INFO_AVAIL_EVENT_ID \
    ((uint32_t)((BDSP_ARM_CDB_DATA_AVAIL_EVENT_ID) + 1))                              /** 0x17 **/

#define BDSP_ARM_UNLICENSED_ALGO_EVENT_ID    \
    ((uint32_t)((BDSP_ARM_STREAM_INFO_AVAIL_EVENT_ID) + 1))                           /** 0x18 **/

#define BDSP_ARM_ENCODER_OVERFLOW_EVENT_ID \
    ((uint32_t)((BDSP_ARM_UNLICENSED_ALGO_EVENT_ID) + 1))                             /** 0x19 **/

#define BDSP_ARM_ANCDATA_EVENT_ID \
    ((uint32_t)((BDSP_ARM_ENCODER_OVERFLOW_EVENT_ID) + 1))                            /** 0x1A **/

#define BDSP_ARM_CHANGE_IN_DIALNORM_EVENT_ID \
    ((uint32_t)((BDSP_ARM_ANCDATA_EVENT_ID) + 1))                                     /** 0x1B **/

#define BDSP_ARM_BSP_SCM_RESPONSE_EVENT_ID  \
    ((uint32_t)((BDSP_ARM_CHANGE_IN_DIALNORM_EVENT_ID) + 1))                          /** 0x1C**/

#define BDSP_ARM_VENC_DATA_DISCARDED_EVENT_ID \
    ((uint32_t)((BDSP_ARM_BSP_SCM_RESPONSE_EVENT_ID) + 1))                            /** 0x1D **/


#define BDSP_ARM_ONDEMAND_AUDIO_FRAME_DELIVERED_EVENT_ID \
    ((uint32_t)((BDSP_ARM_VENC_DATA_DISCARDED_EVENT_ID) + 1))                           /** 0x1E **/

#define BDSP_ARM_VOLUME_LEVEL_REACHED_EVENT_ID \
    ((uint32_t)((BDSP_ARM_ONDEMAND_AUDIO_FRAME_DELIVERED_EVENT_ID) + 1))               /** 0x1F **/

#define BDSP_ARM_LAST_EVENT_ID \
    ((uint32_t)BDSP_ARM_VOLUME_LEVEL_REACHED_EVENT_ID)                                /** 0x20 **/

#define BDSP_ARM_MAX_ASYNC_EVENT   \
    ((uint32_t)(BDSP_ARM_LAST_EVENT_ID - BDSP_ARM_ASYNC_EVENT_ID_OFFSET))           /** 0x20 **/


#if 0
/*** These are unique Command Ids used by Host ***/

#define BDSP_ARM_START_TASK_COMMAND_ID   \
        ((uint32_t)((BDSP_ARM_COMMAND_ID_OFFSET) + 1))                                /** 0x101 **/

#define BDSP_ARM_STOP_TASK_COMMAND_ID    \
        ((uint32_t)((BDSP_ARM_START_TASK_COMMAND_ID) + 1))                            /** 0x102 **/

#define BDSP_ARM_ALGO_PARAMS_CFG_COMMAND_ID  \
        ((uint32_t)((BDSP_ARM_STOP_TASK_COMMAND_ID) + 1))                             /** 0x103 **/

#define BDSP_ARM_PAUSE_COMMAND_ID        \
        ((uint32_t)((BDSP_ARM_ALGO_PARAMS_CFG_COMMAND_ID) + 1))                       /** 0x104 **/

#define BDSP_ARM_FRAME_ADVANCE_COMMAND_ID        \
        ((uint32_t)((BDSP_ARM_PAUSE_COMMAND_ID) + 1))                                 /** 0x105 **/

#define BDSP_ARM_RESUME_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_FRAME_ADVANCE_COMMAND_ID) + 1))                         /** 0x106 **/

#define BDSP_ARM_EVENT_NOTIFICATION_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_RESUME_COMMAND_ID ) + 1))                               /** 0x107 **/

#define BDSP_ARM_FMMPORT_RECFG_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_EVENT_NOTIFICATION_COMMAND_ID ) + 1))                   /** 0x108 **/

#define BDSP_ARM_PING_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_FMMPORT_RECFG_COMMAND_ID ) + 1))                        /** 0x109 **/

#define BDSP_ARM_GET_VOM_TABLE_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_PING_COMMAND_ID ) + 1))                                 /** 0x10A **/

#define BDSP_ARM_NUM_PIC_TO_DROP_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_GET_VOM_TABLE_COMMAND_ID ) + 1))                        /** 0x10B **/

#define BDSP_ARM_RECONFIGURATION_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_NUM_PIC_TO_DROP_COMMAND_ID ) + 1))                      /** 0x10C **/
#define BDSP_ARM_GET_SYSTEM_SWAP_MEMORY_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_RECONFIGURATION_COMMAND_ID ) + 1))                      /** 0x10D **/
/* Support to handle audio gaps in NRT xcode case */
#define BDSP_ARM_AUDIO_GAP_FILL_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_GET_SYSTEM_SWAP_MEMORY_COMMAND_ID ) + 1))               /** 0x10E **/
#define BDSP_ARM_BSP_SCM_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_AUDIO_GAP_FILL_COMMAND_ID ) + 1))                       /** 0x10F **/

#define BDSP_ARM_AUDIO_OUTPUT_FREEZE_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_BSP_SCM_COMMAND_ID ) + 1))                              /** 0x110 **/

#define BDSP_ARM_AUDIO_OUTPUT_UNFREEZE_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_AUDIO_OUTPUT_FREEZE_COMMAND_ID ) + 1))                  /** 0x111 **/

#define BDSP_ARM_MAP_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_AUDIO_OUTPUT_UNFREEZE_COMMAND_ID ) + 1))                /** 0x112 **/

#define BDSP_ARM_UNMAP_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_MAP_COMMAND_ID ) + 1))                                    /** 0x113 **/
#endif

#define BDSP_ARM_MAP_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_COMMAND_ID_OFFSET ) + 1))                             /** 0x131 **/

#define BDSP_ARM_UNMAP_COMMAND_ID       \
        ((uint32_t)((BDSP_ARM_MAP_COMMAND_ID ) + 1))                                   /** 0x132 **/


/*** Following are the Ack ids for different commands ***/

#define BDSP_ARM_START_TASK_ACK_ID   \
        ((uint32_t)((BDSP_ARM_ACK_ID_OFFSET) + 1))                                    /** 0x201 **/

#define BDSP_ARM_STOP_TASK_ACK_ID        \
        ((uint32_t)((BDSP_ARM_START_TASK_ACK_ID) + 1))                                /** 0x202 **/

#define BDSP_ARM_ALGO_PARAMS_CFG_COMMAND_ACK_ID            \
        ((uint32_t)((BDSP_ARM_STOP_TASK_ACK_ID) + 1))                                 /** 0x203 **/

#define BDSP_ARM_PAUSE_ACK_ID            \
        ((uint32_t)((BDSP_ARM_ALGO_PARAMS_CFG_COMMAND_ACK_ID) + 1))                   /** 0x204 **/

#define BDSP_ARM_FRAME_ADVANCE_ACK_ID    \
        ((uint32_t)((BDSP_ARM_PAUSE_ACK_ID ) + 1))                                    /** 0x205 **/

#define BDSP_ARM_RESUME_ACK_ID           \
        ((uint32_t)((BDSP_ARM_FRAME_ADVANCE_ACK_ID) + 1))                             /** 0x206 **/

#define BDSP_ARM_EVENT_NOTIFICATION_ACK_ID       \
        ((uint32_t)((BDSP_ARM_RESUME_ACK_ID ) + 1))                                   /** 0x207 **/

#define BDSP_ARM_FMMPORT_RECFG_ACK_ID       \
        ((uint32_t)((BDSP_ARM_EVENT_NOTIFICATION_ACK_ID ) + 1))                       /** 0x208 **/

#define BDSP_ARM_PING_ACK_ID       \
        ((uint32_t)((BDSP_ARM_FMMPORT_RECFG_ACK_ID ) + 1))                            /** 0x209 **/

/* Support for Video Picture Drop */
#define BDSP_ARM_NUM_PIC_TO_DROP_ACK_ID     \
        ((uint32_t)((BDSP_ARM_PING_ACK_ID ) + 1))                                     /** 0x20A **/

/* Support to handle audio gaps in NRT xcode case */
#define BDSP_ARM_AUDIO_GAP_FILL_ACK_ID       \
        ((uint32_t)((BDSP_ARM_NUM_PIC_TO_DROP_ACK_ID ) + 1))                          /** 0x20B **/

#define BDSP_ARM_BSP_SCM_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_ARM_AUDIO_GAP_FILL_ACK_ID ) + 1))                          /** 0x20C **/

#define BDSP_ARM_AUDIO_OUTPUT_FREEZE_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_ARM_BSP_SCM_COMMAND_ACK_ID ) + 1))                         /** 0x20D **/

#define BDSP_ARM_AUDIO_OUTPUT_UNFREEZE_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_ARM_AUDIO_OUTPUT_FREEZE_COMMAND_ACK_ID ) + 1))             /** 0x20E **/


 /*** The following are the various Response Ids used for different commands  ***/

#define BDSP_ARM_START_TASK_RESPONSE_ID   \
        ((uint32_t)((BDSP_ARM_RESPONSE_ID_OFFSET) + 1))                               /** 0x301 **/

#define BDSP_ARM_STOP_TASK_RESPONSE_ID        \
        ((uint32_t)((BDSP_ARM_START_TASK_RESPONSE_ID) + 1))                           /** 0x302 **/

#define BDSP_ARM_ALGO_PARAMS_CFG_RESPONSE_ID            \
        ((uint32_t)((BDSP_ARM_STOP_TASK_RESPONSE_ID) + 1))                            /** 0x303 **/

#define BDSP_ARM_PAUSE_RESPONSE_ID            \
        ((uint32_t)((BDSP_ARM_ALGO_PARAMS_CFG_RESPONSE_ID) + 1))                      /** 0x304 **/

#define BDSP_ARM_FRAME_ADVANCE_RESPONSE_ID    \
        ((uint32_t)((BDSP_ARM_PAUSE_RESPONSE_ID ) + 1))                               /** 0x305 **/

#define BDSP_ARM_RESUME_RESPONSE_ID           \
        ((uint32_t)((BDSP_ARM_FRAME_ADVANCE_RESPONSE_ID) + 1))                        /** 0x306 **/

#define BDSP_ARM_EVENT_NOTIFICATION_RESPONSE_ID       \
        ((uint32_t)((BDSP_ARM_RESUME_RESPONSE_ID ) + 1))                              /** 0x307 **/

#define BDSP_ARM_FMMPORT_RECFG_RESPONSE_ID       \
        ((uint32_t)((BDSP_ARM_EVENT_NOTIFICATION_RESPONSE_ID ) + 1))                  /** 0x308 **/

#define BDSP_ARM_RECONFIGURATION_RESPONSE_ID       \
        ((uint32_t)((BDSP_ARM_FMMPORT_RECFG_RESPONSE_ID ) + 1))                       /** 0x309 **/

#define BDSP_ARM_BSP_SCM_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_ARM_RECONFIGURATION_RESPONSE_ID ) + 1))                    /** 0x30A **/

#define BDSP_ARM_AUDIO_OUTPUT_FREEZE_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_ARM_BSP_SCM_COMMAND_RESPONSE_ID ) + 1))                    /** 0x30B **/

#define BDSP_ARM_AUDIO_OUTPUT_UNFREEZE_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_ARM_AUDIO_OUTPUT_FREEZE_COMMAND_RESPONSE_ID ) + 1))        /** 0x30C **/

#define BDSP_ARM_HBC_SET_WATCHDOG_RESPONSE_ID       \
         ((uint32_t)((BDSP_ARM_AUDIO_OUTPUT_UNFREEZE_COMMAND_RESPONSE_ID ) + 1))        /** 0x30D **/
/***************************************************************************
Summary:
    Enum indicating Mask bit for Enabling/Disabling specific event for a task.

See Also:
****************************************************************************/

typedef enum BDSP_Arm_P_EventIdMask
{
    BDSP_Arm_P_EventIdMask_eNone                          = 0x00000000,
    BDSP_Arm_P_EventIdMask_eTargetVolumeLevelReached      = 0x00000001,
    BDSP_Arm_P_EventIdMask_eFrameRepeat                   = 0x00000002,
    BDSP_Arm_P_EventIdMask_eFrameDropFromCdb              = 0x00000004,
    BDSP_Arm_P_EventIdMask_eTsmFail                       = 0x00000008,
    BDSP_Arm_P_EventIdMask_eCdbItbUnderflow               = 0x00000010,
    BDSP_Arm_P_EventIdMask_eCdbItbOverflow                = 0x00000020,
    BDSP_Arm_P_EventIdMask_eCrcError                      = 0x00000040,
    BDSP_Arm_P_EventIdMask_eAudioModeChange               = 0x00000080,
    BDSP_Arm_P_EventIdMask_eBitRateChange                 = 0x00000100,
    BDSP_Arm_P_EventIdMask_eSampleRateChange              = 0x00000200,
    BDSP_Arm_P_EventIdMask_eFrameSyncLock                 = 0x00000400,
    BDSP_Arm_P_EventIdMask_eFrameSyncLockLost             = 0x00000800,
    BDSP_Arm_P_EventIdMask_eSTC_PTS_DiffLowerThreshold    = 0x00001000,
    BDSP_Arm_P_EventIdMask_eSTC_PTS_DiffUpperThreshold    = 0x00002000,
    BDSP_Arm_P_EventIdMask_eDesiredCDB_BufferLevel        = 0x00004000,
    BDSP_Arm_P_EventIdMask_eFirstPTS_Received             = 0x00008000,
    BDSP_Arm_P_EventIdMask_ePTS_error                     = 0x00010000,
    BDSP_Arm_P_EventIdMask_eTSM_Lock                      = 0x00020000,
    BDSP_Arm_P_EventIdMask_eStartOnPTS                    = 0x00040000,
    BDSP_Arm_P_EventIdMask_eStopOnPTS                     = 0x00080000,
    BDSP_Arm_P_EventIdMask_eAstmTsmPass                   = 0x00100000,
    BDSP_Arm_P_EventIdMask_eRampEnable                    = 0x00200000,
    BDSP_Arm_P_EventIdMask_eCDBDataAvail                  = 0x00400000,
    BDSP_Arm_P_EventIdMask_eStreamInfoAvail               = 0x00800000,
    BDSP_Arm_P_EventIdMask_eUnlicensedAlgo                = 0x01000000,
    BDSP_Arm_P_EventIdMask_eEncoderOverflow               = 0x02000000,
    BDSP_Arm_P_EventIdMask_eAncData                       = 0x04000000,
    BDSP_Arm_P_EventIdMask_eChangeInDialnorm              = 0x08000000,
    BDSP_Arm_P_EventIdMask_eBspScmResponse                = 0x10000000,
    BDSP_Arm_P_EventIdMask_eVencDataDiscarded             = 0x20000000,
    BDSP_Arm_P_EventIdMask_eOnDemandAudioFrameDelivered   = 0x40000000,
    BDSP_Arm_P_EventIdMask_eLast,
    BDSP_Arm_P_EventIdMask_eAll                           = 0x7FFFFFFF   /* Keep updating this All */
} BDSP_Arm_P_EventIdMask;

/***************************************************************************
Summary:
     Get MAP table command structure.

Description:

See Also:
****************************************************************************/

typedef struct  BDSP_Arm_P_MapTableCommand
{
#if 0 /*SR_TBD Need the max element in the entries to be optimised */
    uint32_t                    ui32HostMapTableAddr;        /*Address of table in the host*/
    uint32_t                    ui32NumEntries;              /*Number of valid entries in the table*/
#else
    uint32_t                    ui32NumEntries;              /*Number of valid entries in the table*/
    BDSP_MAP_Table_Entry        sMapEntries[BDSP_ARM_MAX_MAP_TABLE_ENTRY];
#endif
} BDSP_Arm_P_MapTableCommand;


/***************************************************************************
Summary:
     Get UNMAP table command structure.

Description:

See Also:
****************************************************************************/

typedef struct  BDSP_Arm_P_UnMapTableCommand
{
#if 0 /*SR_TBD Need the max element in the entries to be optimised */
    uint32_t                    ui32HostUnMapTableAddr;      /*Address of table in the host*/
    uint32_t                    ui32NumEntries;              /*Number of valid entries in the table*/
#else
    uint32_t                    ui32NumEntries;              /*Number of valid entries in the table*/
    BDSP_MAP_Table_Entry        sMapEntries[BDSP_ARM_MAX_MAP_TABLE_ENTRY];
#endif
} BDSP_Arm_P_UnMapTableCommand;

/***************************************************************************
Summary:
     Common structure for all firmware commands

Description: Currently, it is assumed that all the commands posted by Host will be
             of fixed size, which is sizeof(BDSP_Arm_P_Command) data structure.
             It will be easier in future if we need to skip specific commands
             from the queue, which can be easily achieved by moving with a
             fixed size offset for each command in the queue.
See Also:
****************************************************************************/

typedef struct BDSP_Arm_P_Command
{
    BDSP_P_CommandHeader   sCommandHeader;
    union
    {
        BDSP_P_StartTaskCommand             sStartTask;
        BDSP_P_StopTaskCommand              sStopTask;
        BDSP_P_CfgChangeCommand             sCfgChange;
        BDSP_P_FrameAdvanceCommand          sFrameAdvance;
        BDSP_P_EventEnableDisable           sEnableDisableEvent;
        BDSP_P_FmmPortReconfigCommand       sFmmPortReconfig;
        BDSP_P_PingCommand                  sPing;
        BDSP_P_CitReconfigCommand           sCitReconfigCommand;
        BDSP_P_AudioOutputFreezeCommand     sFreezeCommand;
        BDSP_P_AudioOutputUnFreezeCommand   uUnFreezeCommand;
        BDSP_Arm_P_MapTableCommand          sMapCommand;
        BDSP_Arm_P_UnMapTableCommand        sUnMapCommand;
    } uCommand;
} BDSP_Arm_P_Command;

/***************************************************************************
Summary:
     Structure for Ack/Response message in sync queue

Description: Currently, it is assumed that all the sync responses posted by DSP will
             be of fixed size, which is sizeof(BDSP_Raaga_P_Response) data structure.
             It will be easier in future if Host needs to skip specific response
             message    in the sync queue for processing, which can be easily achieved
             by moving with a fixed size offset for each response in the sync queue.

            Only, header needs to be posted by DSP for any Ack to the Host.
See Also:
****************************************************************************/

typedef struct BDSP_Arm_P_Response
{
    BDSP_P_CommonAckResponseHeader     sCommonAckResponseHeader;
    union
    {
        BDSP_P_FrameAdvanceResponse    sFrameAdvance;
    } uResponse;
} BDSP_Arm_P_Response;


/***************************************************************************
Summary:
     Structure for unlicensed Algo event

Description:
See Also:
****************************************************************************/
typedef struct BDSP_Arm_P_UnsupportedAlgoInfo
{
    uint32_t        ui32AudioAlgorithm; /* Algorithm Name */

} BDSP_Arm_P_UnsupportedAlgoInfo;


/***************************************************************************
Summary:
     Structure for Async message events

Description: Currently, it is assumed that all the async message events posted by
             DSP will be of fixed size, which is sizeof(BDSP_Arm_P_AsynEventMsg) data
             structure. It will be easier in future if Host needs to skip specific event
             message from async queue for processing, which can be easily achieved by
             moving with a fixed size offset for each async event message in the queue.
See Also:
****************************************************************************/
/*TODO: UNify BDSP_Arm_P_PtsInfo and BDSP_AudioTaskTsmStatus to single structure.*/
typedef struct BDSP_Arm_P_PtsInfo
{
    uint32_t        ui32RunningPts;       /* The PTS value of the frame going
                                                                              out the next vsync
                                                                              MPEG/DirecTv HD - 45KHz domain
                                                                              DirecTv SD- 27MHz domain */
    int32_t         i32Pts2StcPhase;      /* Difference between current PTS
                                                                              and the STC */
    int32_t         i32TSMUpperThreshold; /*TSM Upper threshold*/
    BDSP_PtsType    ePtsType;             /* The PTS type tag */
    uint32_t        ui32NumContinuousFail;/* Fail count from TSM */

} BDSP_Arm_P_PtsInfo;


typedef struct BDSP_Arm_P_AsynEventMsg
{
    BDSP_P_AsynMSGHeader  sMsgHeader;
    union
    {
        BDSP_Arm_P_PtsInfo                sPtsInfo;
        BDSP_Arm_P_UnsupportedAlgoInfo    sUnsupportedAlgoInfo;
    }uFWMessageInfo;
}BDSP_Arm_P_AsynEventMsg;


typedef enum BDSP_Arm_P_AckType
{
    BDSP_Arm_P_AckType_eDevice,
    BDSP_Arm_P_AckType_eTask,
    BDSP_Arm_P_AckType_eLast,
    BDSP_Arm_P_AckType_eMax = 0x7FFFFFFF
}BDSP_Arm_P_AckType;

/***************************************************************************
Summary:
    This structure is used while sending ack to BDSP.

Description:
    This is the header for Ack by Astra

See Also:
****************************************************************************/
typedef struct BDSP_Arm_P_AckInfo
{
    BDSP_Arm_P_AckType          eAckType;           /*  type of Ack from Arm */
	union {
		uint32_t                    ui32TaskID;         /*  Task ID */
		uint32_t                    ui32DeviceCmdResp;     /*  Device Resp */
	}params;
} BDSP_Arm_P_AckInfo;


/* This define return the fixed size value for any command
   This SIZEOF() macro return the value in bytes */
#define BDSP_ARM_P_COMMAND_SIZE_IN_BYTES             SIZEOF(BDSP_Arm_P_Command)

/* This define return the fixed size value for any Ack */
#define BDSP_ARM_ACK_SIZE_IN_BYTES                   SIZEOF(BDSP_Arm_P_Response)

/* This define return the fixed size value for any Response */
#define BDSP_ARM_RESPONSE_SIZE_IN_BYTES              SIZEOF(BDSP_Arm_P_Response)

/* This define return the fixed size value for any Async Event */
#define BDSP_ARM_ASYNC_RESPONSE_SIZE_IN_BYTES        SIZEOF(BDSP_Arm_P_AsynEventMsg)


typedef enum BDSP_ArmDspMsg
{
    BDSP_ARM_DSP_MSG_INIT_PARAMS = 1,           /* ArmDsp intialization parameters */
    BDSP_ARM_DSP_MSG_CLOSE_APP = 2,
    BDSP_ARM_DSP_MSG_HBC_INFO = 3,
    BDSP_ARM_DSP_MSG_LAST
}BDSP_ArmDspMsg;

typedef struct BDSP_ArmDspSystemInitParams
{
    uint32_t QueueHandleArryPhyAddr;
    uint32_t ui32NumQueueHandle;
    uint32_t cmdQueueHandlePhyAddr;
    uint32_t genRspQueueHandlePhyAddr;
}BDSP_ArmDspSystemInitParams;

typedef struct BDSP_ArmDspSystemInitParamsResp
{
   uint32_t checksum;
}BDSP_ArmDspSystemInitParamsResp;

typedef struct BDSP_ArmDspHbcParams
{
	uint32_t *hbcValidDramAddr;
	uint32_t *hbcDramAddr;
}BDSP_ArmDspHbcParams;

typedef struct BDSP_ArmDspSystemCmd
{
    BDSP_ArmDspMsg eArmSysMsg;
    union{
        BDSP_ArmDspSystemInitParams sInitParams;
		BDSP_ArmDspHbcParams sHbcParams;
    }uCommand;
}BDSP_ArmDspSystemCmd;

#endif /* BDSP_ARM_CMDRESP_PRIV_H__ */
