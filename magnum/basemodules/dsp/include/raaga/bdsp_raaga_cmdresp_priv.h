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
 ******************************************************************************/


#ifndef _BRAAGA_CMDRESP_PRIV_H__
#define _BRAAGA_CMDRESP_PRIV_H__

#include "bdsp_common_cmdresp_priv.h"

 /* The following are the various ID's given to each command
from host to any task on DSP */

#define BDSP_RAAGA_ASYNC_EVENT_ID_OFFSET                    ((uint32_t)0x0)

#define BDSP_RAAGA_INVALID_COMMAND                          ((uint32_t)-1)

#define BDSP_RAAGA_COMMAND_ID_OFFSET                        ((uint32_t)0x150)

#define BDSP_RAAGA_ACK_ID_OFFSET                            ((uint32_t)0x200)

#define BDSP_RAAGA_RESPONSE_ID_OFFSET                       ((uint32_t)0x300)

#define BDSP_RAAGA_DEFAULT_EVENTS_ENABLE_MASK               ((uint32_t)0x3)

    /** This is the starting Task ID offset which can be
        used by Host to create any    new tasks in future **/
#define BDSP_RAAGA_TASK_ID_START_OFFSET                     ((uint32_t)15)

    /** This is the maximum value of taskID which can be
        used by host in future **/
#define BDSP_RAAGA_TASK_MAX_ID_AVAILABLE                    ((uint32_t)56)


/*** Following are the various Event Ids used  ***/

#define BDSP_RAAGA_DUMMY_FRAME_GENERATION_EVENT_ID  \
        ((uint32_t)((BDSP_RAAGA_ASYNC_EVENT_ID_OFFSET) )  )                             /** 0x00 **/

#define BDSP_RAAGA_FRAME_REPEAT_EVENT_ID    \
        ((uint32_t)((BDSP_RAAGA_DUMMY_FRAME_GENERATION_EVENT_ID) + 1)   )               /** 0x01 **/

#define BDSP_RAAGA_FRAME_DROP_FROM_CDB_EVENT_ID \
        ((uint32_t)((BDSP_RAAGA_FRAME_REPEAT_EVENT_ID) + 1) )                           /** 0x02 **/

#define BDSP_RAAGA_TSM_FAIL_EVENT_ID    \
        ((uint32_t)((BDSP_RAAGA_FRAME_DROP_FROM_CDB_EVENT_ID) + 1))                     /** 0x03 **/

#define BDSP_RAAGA_CDB_ITB_UNDERFLOW_EVENT_ID   \
        ((uint32_t)((BDSP_RAAGA_TSM_FAIL_EVENT_ID) + 1) )                               /** 0x04 **/

#define BDSP_RAAGA_CDB_ITB_OVERFLOW_EVENT_ID    \
        ((uint32_t)((BDSP_RAAGA_CDB_ITB_UNDERFLOW_EVENT_ID) + 1)    )                   /** 0x05 **/

#define BDSP_RAAGA_CRC_ERROR_EVENT_ID       \
        ((uint32_t)((BDSP_RAAGA_CDB_ITB_OVERFLOW_EVENT_ID) + 1) )                       /** 0x06 **/

#define BDSP_RAAGA_AUDIO_MODE_CHANGE_EVENT_ID   \
        ((uint32_t)((BDSP_RAAGA_CRC_ERROR_EVENT_ID) + 1)    )                           /** 0x07 **/

#define BDSP_RAAGA_BIT_RATE_CHANGE_EVENT_ID \
        ((uint32_t)((BDSP_RAAGA_AUDIO_MODE_CHANGE_EVENT_ID) + 1)    )                   /** 0x08 **/

#define BDSP_RAAGA_SAMPLING_RATE_CHANGE_EVENT_ID    \
        ((uint32_t)((BDSP_RAAGA_BIT_RATE_CHANGE_EVENT_ID) + 1)  )                       /** 0x09 **/

#define BDSP_RAAGA_FRAME_SYNC_LOCK_EVENT_ID \
        ((uint32_t)((BDSP_RAAGA_SAMPLING_RATE_CHANGE_EVENT_ID) + 1))                    /** 0x0A */

#define BDSP_RAAGA_FRAME_SYNC_LOCK_LOST_EVENT_ID    \
        ((uint32_t)((BDSP_RAAGA_FRAME_SYNC_LOCK_EVENT_ID) + 1))                         /** 0x0B **/

#define BDSP_RAAGA_STC_PTS_DIFF_LOWER_THRESHOLD_EVENT_ID    \
        ((uint32_t)((BDSP_RAAGA_FRAME_SYNC_LOCK_LOST_EVENT_ID) + 1))                    /** 0x0C */

#define BDSP_RAAGA_STC_PTS_DIFF_UPPER_THRESHOLD_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_STC_PTS_DIFF_LOWER_THRESHOLD_EVENT_ID) + 1))                /** 0x0D **/

#define BDSP_RAAGA_DESIRED_CDB_BUFFER_LEVEL_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_STC_PTS_DIFF_UPPER_THRESHOLD_EVENT_ID) + 1))                /** 0x0E **/

#define BDSP_RAAGA_FIRST_PTS_RECEIVED_FROM_ITB_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_DESIRED_CDB_BUFFER_LEVEL_EVENT_ID) + 1))                    /** 0x0F **/

#define BDSP_RAAGA_PTS_ERR_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_FIRST_PTS_RECEIVED_FROM_ITB_EVENT_ID) + 1))                 /** 0x10 **/

#define BDSP_RAAGA_TSM_LOCK_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_PTS_ERR_EVENT_ID) + 1))                                     /** 0x11 **/

#define BDSP_RAAGA_START_PTS_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_TSM_LOCK_EVENT_ID) + 1))                                    /** 0x12 **/

#define BDSP_RAAGA_STOP_PTS_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_START_PTS_EVENT_ID) + 1))                                   /** 0x13 **/

#define BDSP_RAAGA_ASTMTSM_PASS_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_STOP_PTS_EVENT_ID) + 1))                                    /** 0x14 **/

#define BDSP_RAAGA_RAMP_ENABLE_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_ASTMTSM_PASS_EVENT_ID) + 1))                                /** 0x15 **/

#define BDSP_RAAGA_CDB_DATA_AVAIL_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_RAMP_ENABLE_EVENT_ID) + 1))                                 /** 0x16 **/

#define BDSP_RAAGA_STREAM_INFO_AVAIL_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_CDB_DATA_AVAIL_EVENT_ID) + 1))                              /** 0x17 **/

#define BDSP_RAAGA_UNLICENSED_ALGO_EVENT_ID    \
    ((uint32_t)((BDSP_RAAGA_STREAM_INFO_AVAIL_EVENT_ID) + 1))                           /** 0x18 **/

#define BDSP_RAAGA_ENCODER_OVERFLOW_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_UNLICENSED_ALGO_EVENT_ID) + 1))                             /** 0x19 **/

#define BDSP_RAAGA_ANCDATA_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_ENCODER_OVERFLOW_EVENT_ID) + 1))                            /** 0x1A **/

#define BDSP_RAAGA_CHANGE_IN_DIALNORM_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_ANCDATA_EVENT_ID) + 1))                                     /** 0x1B **/

#define BDSP_RAAGA_BSP_SCM_RESPONSE_EVENT_ID  \
    ((uint32_t)((BDSP_RAAGA_CHANGE_IN_DIALNORM_EVENT_ID) + 1))                          /** 0x1C**/

#define BDSP_RAAGA_VENC_DATA_DISCARDED_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_BSP_SCM_RESPONSE_EVENT_ID) + 1))                            /** 0x1D **/


#define BDSP_RAAGA_ONDEMAND_AUDIO_FRAME_DELIVERED_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_VENC_DATA_DISCARDED_EVENT_ID) + 1))                         /** 0x1E **/

#define BDSP_RAAGA_VOLUME_LEVEL_REACHED_EVENT_ID \
    ((uint32_t)((BDSP_RAAGA_ONDEMAND_AUDIO_FRAME_DELIVERED_EVENT_ID) + 1))               /** 0x1F **/

#define BDSP_RAAGA_LAST_EVENT_ID \
    ((uint32_t)BDSP_RAAGA_VOLUME_LEVEL_REACHED_EVENT_ID)                                /** 0x20 **/

#define BDSP_RAAGA_MAX_ASYNC_EVENT   \
    ((uint32_t)(BDSP_RAAGA_LAST_EVENT_ID - BDSP_RAAGA_ASYNC_EVENT_ID_OFFSET))           /** 0x20 **/



#if 0

/*** These are unique Command Ids used by Host ***/

#define BDSP_RAAGA_START_TASK_COMMAND_ID   \
        ((uint32_t)((BDSP_RAAGA_COMMAND_ID_OFFSET) + 1))                                /** 0x101 **/

#define BDSP_RAAGA_STOP_TASK_COMMAND_ID    \
        ((uint32_t)((BDSP_RAAGA_START_TASK_COMMAND_ID) + 1))                            /** 0x102 **/

#define BDSP_RAAGA_ALGO_PARAMS_CFG_COMMAND_ID  \
        ((uint32_t)((BDSP_RAAGA_STOP_TASK_COMMAND_ID) + 1))                             /** 0x103 **/

#define BDSP_RAAGA_PAUSE_COMMAND_ID        \
        ((uint32_t)((BDSP_RAAGA_ALGO_PARAMS_CFG_COMMAND_ID) + 1))                       /** 0x104 **/

#define BDSP_RAAGA_FRAME_ADVANCE_COMMAND_ID        \
        ((uint32_t)((BDSP_RAAGA_PAUSE_COMMAND_ID) + 1))                                 /** 0x105 **/

#define BDSP_RAAGA_RESUME_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_FRAME_ADVANCE_COMMAND_ID) + 1))                         /** 0x106 **/

#define BDSP_RAAGA_EVENT_NOTIFICATION_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_RESUME_COMMAND_ID ) + 1))                               /** 0x107 **/

#define BDSP_RAAGA_FMMPORT_RECFG_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_EVENT_NOTIFICATION_COMMAND_ID ) + 1))                   /** 0x108 **/

#define BDSP_RAAGA_PING_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_FMMPORT_RECFG_COMMAND_ID ) + 1))                        /** 0x109 **/

#define BDSP_RAAGA_GET_VOM_TABLE_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_PING_COMMAND_ID ) + 1))                                 /** 0x10A **/

#define BDSP_RAAGA_NUM_PIC_TO_DROP_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_GET_VOM_TABLE_COMMAND_ID ) + 1))                        /** 0x10B **/

#define BDSP_RAAGA_RECONFIGURATION_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_NUM_PIC_TO_DROP_COMMAND_ID ) + 1))                      /** 0x10C **/
#define BDSP_RAAGA_GET_SYSTEM_SWAP_MEMORY_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_RECONFIGURATION_COMMAND_ID ) + 1))                      /** 0x10D **/
/* Support to handle audio gaps in NRT xcode case */
#define BDSP_RAAGA_AUDIO_GAP_FILL_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_GET_SYSTEM_SWAP_MEMORY_COMMAND_ID ) + 1))               /** 0x10E **/
#define BDSP_RAAGA_BSP_SCM_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_AUDIO_GAP_FILL_COMMAND_ID ) + 1))                       /** 0x10F **/

#define BDSP_RAAGA_AUDIO_OUTPUT_FREEZE_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_BSP_SCM_COMMAND_ID ) + 1))                              /** 0x110 **/

#define BDSP_RAAGA_AUDIO_OUTPUT_UNFREEZE_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_AUDIO_OUTPUT_FREEZE_COMMAND_ID ) + 1))                  /** 0x111 **/
#endif

/*** These are unique Command Ids used by Host only for Raaga ***/

#define BDSP_RAAGA_GET_SYSTEM_SWAP_MEMORY_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_COMMAND_ID_OFFSET ) + 1))                               /** 0x151 **/

#define BDSP_RAAGA_GET_VOM_TABLE_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_GET_SYSTEM_SWAP_MEMORY_COMMAND_ID ) + 1))               /** 0x152 **/

#define BDSP_RAAGA_NUM_PIC_TO_DROP_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_GET_VOM_TABLE_COMMAND_ID ) + 1))                        /** 0x153 **/

/* Support to handle audio gaps in NRT xcode case */
#define BDSP_RAAGA_AUDIO_GAP_FILL_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_NUM_PIC_TO_DROP_COMMAND_ID ) + 1))                       /** 0x154 **/

#define BDSP_RAAGA_BSP_SCM_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_AUDIO_GAP_FILL_COMMAND_ID ) + 1))                       /** 0x155**/

#define BDSP_RAAGA_PROCESS_PAK_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_BSP_SCM_COMMAND_ID ) + 1))                              /** 0x156 **/

#define BDSP_RAAGA_GET_AUDIOLICENSE_STATUS_COMMAND_ID       \
        ((uint32_t)((BDSP_RAAGA_PROCESS_PAK_COMMAND_ID ) + 1))                          /** 0x157 **/

/*** Following are the Ack ids for different commands ***/

#define BDSP_RAAGA_START_TASK_ACK_ID   \
        ((uint32_t)((BDSP_RAAGA_ACK_ID_OFFSET) + 1))                                    /** 0x201 **/

#define BDSP_RAAGA_STOP_TASK_ACK_ID        \
        ((uint32_t)((BDSP_RAAGA_START_TASK_ACK_ID) + 1))                                /** 0x202 **/

#define BDSP_RAAGA_ALGO_PARAMS_CFG_COMMAND_ACK_ID            \
        ((uint32_t)((BDSP_RAAGA_STOP_TASK_ACK_ID) + 1))                                 /** 0x203 **/

#define BDSP_RAAGA_PAUSE_ACK_ID            \
        ((uint32_t)((BDSP_RAAGA_ALGO_PARAMS_CFG_COMMAND_ACK_ID) + 1))                   /** 0x204 **/

#define BDSP_RAAGA_FRAME_ADVANCE_ACK_ID    \
        ((uint32_t)((BDSP_RAAGA_PAUSE_ACK_ID ) + 1))                                    /** 0x205 **/

#define BDSP_RAAGA_RESUME_ACK_ID           \
        ((uint32_t)((BDSP_RAAGA_FRAME_ADVANCE_ACK_ID) + 1))                             /** 0x206 **/

#define BDSP_RAAGA_EVENT_NOTIFICATION_ACK_ID       \
        ((uint32_t)((BDSP_RAAGA_RESUME_ACK_ID ) + 1))                                   /** 0x207 **/

#define BDSP_RAAGA_FMMPORT_RECFG_ACK_ID       \
        ((uint32_t)((BDSP_RAAGA_EVENT_NOTIFICATION_ACK_ID ) + 1))                       /** 0x208 **/

#define BDSP_RAAGA_PING_ACK_ID       \
        ((uint32_t)((BDSP_RAAGA_FMMPORT_RECFG_ACK_ID ) + 1))                            /** 0x209 **/

/* Support for Video Picture Drop */
#define BDSP_RAAGA_NUM_PIC_TO_DROP_ACK_ID     \
        ((uint32_t)((BDSP_RAAGA_PING_ACK_ID ) + 1))                                     /** 0x20A **/

/* Support to handle audio gaps in NRT xcode case */
#define BDSP_RAAGA_AUDIO_GAP_FILL_ACK_ID       \
        ((uint32_t)((BDSP_RAAGA_NUM_PIC_TO_DROP_ACK_ID ) + 1))                          /** 0x20B **/

#define BDSP_RAAGA_BSP_SCM_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_RAAGA_AUDIO_GAP_FILL_ACK_ID ) + 1))                          /** 0x20C **/

#define BDSP_RAAGA_AUDIO_OUTPUT_FREEZE_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_RAAGA_BSP_SCM_COMMAND_ACK_ID ) + 1))                         /** 0x20D **/

#define BDSP_RAAGA_AUDIO_OUTPUT_UNFREEZE_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_RAAGA_AUDIO_OUTPUT_FREEZE_COMMAND_ACK_ID ) + 1))             /** 0x20E **/

#define BDSP_RAAGA_PROCESS_PAK_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_RAAGA_AUDIO_OUTPUT_UNFREEZE_COMMAND_ACK_ID ) + 1))           /** 0x20F **/

#define BDSP_RAAGA_GET_AUDIOLICENSE_STATUS_COMMAND_ACK_ID     \
         ((uint32_t)((BDSP_RAAGA_PROCESS_PAK_COMMAND_ACK_ID ) + 1))                     /** 0x210 **/

 /*** The following are the various Response Ids used for different commands  ***/

#define BDSP_RAAGA_START_TASK_RESPONSE_ID   \
        ((uint32_t)((BDSP_RAAGA_RESPONSE_ID_OFFSET) + 1))                               /** 0x301 **/

#define BDSP_RAAGA_STOP_TASK_RESPONSE_ID        \
        ((uint32_t)((BDSP_RAAGA_START_TASK_RESPONSE_ID) + 1))                           /** 0x302 **/

#define BDSP_RAAGA_ALGO_PARAMS_CFG_RESPONSE_ID            \
        ((uint32_t)((BDSP_RAAGA_STOP_TASK_RESPONSE_ID) + 1))                            /** 0x303 **/

#define BDSP_RAAGA_PAUSE_RESPONSE_ID            \
        ((uint32_t)((BDSP_RAAGA_ALGO_PARAMS_CFG_RESPONSE_ID) + 1))                      /** 0x304 **/

#define BDSP_RAAGA_FRAME_ADVANCE_RESPONSE_ID    \
        ((uint32_t)((BDSP_RAAGA_PAUSE_RESPONSE_ID ) + 1))                               /** 0x305 **/

#define BDSP_RAAGA_RESUME_RESPONSE_ID           \
        ((uint32_t)((BDSP_RAAGA_FRAME_ADVANCE_RESPONSE_ID) + 1))                        /** 0x306 **/

#define BDSP_RAAGA_EVENT_NOTIFICATION_RESPONSE_ID       \
        ((uint32_t)((BDSP_RAAGA_RESUME_RESPONSE_ID ) + 1))                              /** 0x307 **/

#define BDSP_RAAGA_FMMPORT_RECFG_RESPONSE_ID       \
        ((uint32_t)((BDSP_RAAGA_EVENT_NOTIFICATION_RESPONSE_ID ) + 1))                  /** 0x308 **/

#define BDSP_RAAGA_RECONFIGURATION_RESPONSE_ID       \
        ((uint32_t)((BDSP_RAAGA_FMMPORT_RECFG_RESPONSE_ID ) + 1))                       /** 0x309 **/

#define BDSP_RAAGA_BSP_SCM_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_RAAGA_RECONFIGURATION_RESPONSE_ID ) + 1))                    /** 0x30A **/

#define BDSP_RAAGA_AUDIO_OUTPUT_FREEZE_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_RAAGA_BSP_SCM_COMMAND_RESPONSE_ID ) + 1))                    /** 0x30B **/

#define BDSP_RAAGA_AUDIO_OUTPUT_UNFREEZE_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_RAAGA_AUDIO_OUTPUT_FREEZE_COMMAND_RESPONSE_ID ) + 1))        /** 0x30C **/
#define BDSP_RAAGA_PROCESS_PAK_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_RAAGA_AUDIO_OUTPUT_UNFREEZE_COMMAND_RESPONSE_ID ) + 1))      /** 0x30D **/

#define BDSP_RAAGA_GET_AUDIOLICENSE_STATUS_COMMAND_RESPONSE_ID       \
         ((uint32_t)((BDSP_RAAGA_PROCESS_PAK_COMMAND_RESPONSE_ID ) + 1))                /** 0x30E **/
/***************************************************************************
Summary:
    Enum indicating Mask bit for Enabling/Disabling specific event for a task.

See Also:
****************************************************************************/

typedef enum BDSP_Raaga_P_EventIdMask
{
    BDSP_Raaga_P_EventIdMask_eNone                          = 0x00000000,
    BDSP_Raaga_P_EventIdMask_eTargetVolumeLevelReached      = 0x00000001,
    BDSP_Raaga_P_EventIdMask_eFrameRepeat                   = 0x00000002,
    BDSP_Raaga_P_EventIdMask_eFrameDropFromCdb              = 0x00000004,
    BDSP_Raaga_P_EventIdMask_eTsmFail                       = 0x00000008,
    BDSP_Raaga_P_EventIdMask_eCdbItbUnderflow               = 0x00000010,
    BDSP_Raaga_P_EventIdMask_eCdbItbOverflow                = 0x00000020,
    BDSP_Raaga_P_EventIdMask_eCrcError                      = 0x00000040,
    BDSP_Raaga_P_EventIdMask_eAudioModeChange               = 0x00000080,
    BDSP_Raaga_P_EventIdMask_eBitRateChange                 = 0x00000100,
    BDSP_Raaga_P_EventIdMask_eSampleRateChange              = 0x00000200,
    BDSP_Raaga_P_EventIdMask_eFrameSyncLock                 = 0x00000400,
    BDSP_Raaga_P_EventIdMask_eFrameSyncLockLost             = 0x00000800,
    BDSP_Raaga_P_EventIdMask_eSTC_PTS_DiffLowerThreshold    = 0x00001000,
    BDSP_Raaga_P_EventIdMask_eSTC_PTS_DiffUpperThreshold    = 0x00002000,
    BDSP_Raaga_P_EventIdMask_eDesiredCDB_BufferLevel        = 0x00004000,
    BDSP_Raaga_P_EventIdMask_eFirstPTS_Received             = 0x00008000,
    BDSP_Raaga_P_EventIdMask_ePTS_error                     = 0x00010000,
    BDSP_Raaga_P_EventIdMask_eTSM_Lock                      = 0x00020000,
    BDSP_Raaga_P_EventIdMask_eStartOnPTS                    = 0x00040000,
    BDSP_Raaga_P_EventIdMask_eStopOnPTS                     = 0x00080000,
    BDSP_Raaga_P_EventIdMask_eAstmTsmPass                   = 0x00100000,
    BDSP_Raaga_P_EventIdMask_eRampEnable                    = 0x00200000,
    BDSP_Raaga_P_EventIdMask_eCDBDataAvail                  = 0x00400000,
    BDSP_Raaga_P_EventIdMask_eStreamInfoAvail               = 0x00800000,
    BDSP_Raaga_P_EventIdMask_eUnlicensedAlgo                = 0x01000000,
    BDSP_Raaga_P_EventIdMask_eEncoderOverflow               = 0x02000000,
    BDSP_Raaga_P_EventIdMask_eAncData                       = 0x04000000,
    BDSP_Raaga_P_EventIdMask_eChangeInDialnorm              = 0x08000000,
    BDSP_Raaga_P_EventIdMask_eBspScmResponse                = 0x10000000,
    BDSP_Raaga_P_EventIdMask_eVencDataDiscarded             = 0x20000000,
    BDSP_Raaga_P_EventIdMask_eOnDemandAudioFrameDelivered   = 0x40000000,
    BDSP_Raaga_P_EventIdMask_eLast,
    BDSP_Raaga_P_EventIdMask_eAll                           = 0x7FFFFFFF   /* Keep updating this All */
} BDSP_Raaga_P_EventIdMask;

/***************************************************************************
Summary:
     Get System swap memory address structure.

Description:

See Also:
****************************************************************************/

typedef struct  BDSP_Raaga_P_GetSystemSwapMemCommand
{
    dramaddr_t              ui32SystemSwapDramMemAddr;   /* System Swap memory in Dram Address */
} BDSP_Raaga_P_GetSystemSwapMemCommand;

/***************************************************************************
Summary:
     Get VOM table command structure.

Description:

See Also:
****************************************************************************/

typedef struct  BDSP_Raaga_P_GetVomTableCommand
{
    dramaddr_t              ui32HostVomTableAddr;        /*Address of VOM table in the host*/
    uint32_t                ui32NumEntries;              /*Number of valid entries in the table*/
} BDSP_Raaga_P_GetVomTableCommand;

/***************************************************************************
Summary:
    Number of picture to drop at decoder command structure.

Description:

    This is the command structure of the NUM_PIC_TO_DROP command to be
    issued by the host to video decoder to update the decoder whenever DM
    increments it's picture drop counter.

See Also:
****************************************************************************/
typedef struct BDSP_Raaga_P_NumPicToDropCommand
{
    uint32_t      ui32NumPicToDrop;

} BDSP_Raaga_P_NumPicToDropCommand;

/***************************************************************************
Summary:
    BSP Command: Send SCM Commands to DSP

Description:

    Host sends this command structure to DSP for SCM operations.

****************************************************************************/

typedef struct BDSP_Raaga_P_SCM_CmdOperation
{
    BDSP_Raaga_P_SCM_CmdPayload  sCmdBufferInfo;
}BDSP_Raaga_P_SCM_CmdOperation;

/***************************************************************************
Summary:
    License evaluation through PAK(Packet Authorization Key) method.

Description:

    This is the command structure of the PROCESS_PAK command to be
    issued by the host to DSP to decrypt the PAK buffer and then return
    the PAK output.

See Also:
****************************************************************************/
typedef struct BDSP_Raaga_P_ProcessPakCommand
{
    dramaddr_t              pakBufAddr;         /*Address of the packet to be parsed*/
    uint32_t                ui32PakBufSize;               /*Size of the packet*/
    dramaddr_t              drmBufAddr;         /*Address of the DRM packet*/
    uint32_t                ui32DrmBufSize;               /*Size of the DRM packet*/
    dramaddr_t              pakOpBufAddr;       /*Address of the packet to be parsed*/
    dramaddr_t              pakDecryptTableAddr; /* Physical Addr of the PAK decrypt info table downloaded by BDSP */
    uint32_t                ui32PakDecryptTableSize;             /* Size of the PAK Table  */
    uint32_t                ui32Dummy;                /*Required for Size Alignment */
} BDSP_Raaga_P_ProcessPakCommand;

typedef struct BDSP_Raaga_P_AudioLicenseStatus{
    uint32_t ui32AllAudioLicense;
    uint32_t ui32LicenseEnabledInBP3;
    uint32_t ui32LicenseEnabledInPAK;
    uint32_t ui32LicenseEnabledInPAKUnmasked;
    BDSP_eDolbyOTP eDolbyOTP;
    uint32_t ui32BondOption;
    uint32_t ui32Isit28nm;
}BDSP_Raaga_P_AudioLicenseStatus ;

/***************************************************************************
Summary:
     Common structure for all firmware commands

Description: Currently, it is assumed that all the commands posted by Host will be
             of fixed size, which is sizeof(BDSP_Raaga_P_Command) data structure.
             It will be easier in future if we need to skip specific commands
             from the queue, which can be easily achieved by moving with a
             fixed size offset for each command in the queue.
See Also:
****************************************************************************/

typedef struct BDSP_Raaga_P_Command
{
    BDSP_P_CommandHeader   sCommandHeader;
    union
    {
        BDSP_P_StartTaskCommand                 sStartTask;
        BDSP_P_StopTaskCommand                  sStopTask;
        BDSP_P_CfgChangeCommand                 sCfgChange;
        BDSP_P_FrameAdvanceCommand              sFrameAdvance;
        BDSP_P_EventEnableDisable               sEnableDisableEvent;
        BDSP_P_FmmPortReconfigCommand           sFmmPortReconfig;
        BDSP_P_PingCommand                      sPing;
        BDSP_P_CitReconfigCommand               sCitReconfigCommand;
        BDSP_P_AudioOutputFreezeCommand         sFreezeCommand;
        BDSP_P_AudioOutputUnFreezeCommand       uUnFreezeCommand;
        BDSP_Raaga_P_GetSystemSwapMemCommand    sSystemSwapMemCommand;
        BDSP_Raaga_P_SCM_CmdOperation           sScmCmd;
        BDSP_Raaga_P_GetVomTableCommand         sGetVomTable;
        BDSP_Raaga_P_NumPicToDropCommand        sNumPicToDropCommand;
        BDSP_Raaga_P_ProcessPakCommand          sProcessPakCommand;
    } uCommand;
} BDSP_Raaga_P_Command;

/***************************************************************************
Summary:
     PAK Response structure.

Description:

See Also:
****************************************************************************/

typedef struct BDSP_Raaga_P_PAKResponse
{
	uint32_t	ui32LicenseBits;
} BDSP_Raaga_P_PAKResponse;



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

typedef struct BDSP_Raaga_P_Response
{
    BDSP_P_CommonAckResponseHeader     sCommonAckResponseHeader;
    union
    {
        BDSP_P_FrameAdvanceResponse    sFrameAdvance;
        BDSP_Raaga_P_PAKResponse             sPAK;
        BDSP_Raaga_P_AudioLicenseStatus sAudioLicenseStatus;
    } uResponse;
} BDSP_Raaga_P_Response;


/***************************************************************************
Summary:
     Structure for getting the TSM buffer ID info from FW
Description:

See Also:
****************************************************************************/
typedef struct BDSP_DSPCN_P_TsmFwInfo
{
    uint32_t    uiBufId;
}BDSP_DSPCN_P_TsmFwInfo;

/***************************************************************************
Summary:
     Structure for getting the Sample rate info from FW

    There are three parts to the payload
            1) Base Rate : For Configuring the SRC blocks
            2) Stream Sampling Rate : For Configuring the outputs at native
                                      sample rate. Ex: DD convert outputs
            3) HDMI Sampling Rate : Configuring the HDMI block
                - Required only for RAP PI; not required for APE
Description:

See Also:
****************************************************************************/
typedef struct BDSP_DSPCHN_P_FsToHost
{
    uint32_t      ui32SamplingRate;
    uint32_t      ui32ValidFlag;       /* 1: valid and 0: Invalid */

} BDSP_DSPCHN_P_FsToHost;

typedef struct BDSP_DSPCHN_P_FwSampleinfo
{
    BDSP_DSPCHN_P_FsToHost        sBaseSamplingRate;
    BDSP_DSPCHN_P_FsToHost        sHdmiSamplingRate;
    BDSP_DSPCHN_P_FsToHost        sStreamSamplingRate;

}BDSP_DSPCHN_P_FwSampleinfo;

/***************************************************************************
Summary:
     Structure for getting the Bit rate change info from FW
Description:

See Also:
****************************************************************************/
typedef struct BDSP_DSPCHN_P_FwBitRateChangeInfo
{
    uint32_t    ui32BitRate;            /* New Bit Rate value*/
    uint32_t     ui32BitRateIndex;      /* This has the Bit rate index
                                                                        as given in the standard. This value
                                                                        is zero for audio type AAC-HE*/
}BDSP_DSPCHN_P_FwBitRateChangeInfo;

/***************************************************************************
Summary:
     Structure for getting the mode change info from FW
Description:

See Also:
****************************************************************************/
typedef struct BDSP_DSPCHN_P_FwModeChnageInfo
{
    uint32_t    ui32ModeValue;
} BDSP_DSPCHN_P_FwModeChangeInfo;


/***************************************************************************
Summary:
     Structure for ramp enable event

Description:
See Also:
****************************************************************************/
typedef struct BDSP_DSPCHN_RampEnableInfo
{
 uint32_t    ui32TimeDelay;   /* PI should enable the ramp after ui32TimeDelay(ms) */

} BDSP_DSPCHN_RampEnableInfo;

/***************************************************************************
Summary:
     Structure for unlicensed Algo event

Description:
See Also:
****************************************************************************/
typedef struct BDSP_Raaga_P_UnsupportedAlgoInfo
{
    uint32_t        ui32AudioAlgorithm; /* Algorithm Name */

} BDSP_Raaga_P_UnsupportedAlgoInfo;


/***************************************************************************
Summary:
     Structure for Async message events

Description: Currently, it is assumed that all the async message events posted by
             DSP will be of fixed size, which is sizeof(BDSP_Raaga_P_AsynEventMsg) data
             structure. It will be easier in future if Host needs to skip specific event
             message from async queue for processing, which can be easily achieved by
             moving with a fixed size offset for each async event message in the queue.
See Also:
****************************************************************************/
/*TODO: UNify BDSP_Raaga_P_PtsInfo and BDSP_AudioTaskTsmStatus to single structure.*/
typedef struct BDSP_Raaga_P_PtsInfo
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

} BDSP_Raaga_P_PtsInfo;


/***************************************************************************
Summary:
     This structure is used for Response from DSP for SCM operations

Description:
1 - Authentication failure

****************************************************************************/

typedef enum BDSP_Raaga_P_BspScmResponseType
{
    BDSP_Raaga_P_BspScmResponseType_Algorithm_Authentication_Failure = 0x1,
    BDSP_Raaga_P_BspScmResponseType_eLast,

    BDSP_Raaga_P_BspScmResponseType_eINVALID             =   0x7FFFFFFF
} BDSP_Raaga_P_BspScmResponseType;

/***************************************************************************
Summary:
    This structure is used for Response from DSP for SCM operations

Description:

See Also: BDSP_Raaga_P_BspScmResponseType

****************************************************************************/

typedef struct BDSP_Raaga_P_BspScmResponseInfo
{
    BDSP_Raaga_P_BspScmResponseType         eBspScmResponseType;

}BDSP_Raaga_P_BspScmResponseInfo;



typedef struct BDSP_Raaga_P_AsynEventMsg
{
    BDSP_P_AsynMSGHeader  sMsgHeader;
    union
    {
        BDSP_Raaga_P_PtsInfo                sPtsInfo;
        BDSP_DSPCN_P_TsmFwInfo              sTsmInfo;
        BDSP_DSPCHN_P_FwSampleinfo          sFwSampleInfo;
        BDSP_DSPCHN_P_FwModeChangeInfo      sFwModeChange;
        BDSP_DSPCHN_P_FwBitRateChangeInfo   sFwBitRateChange;
        BDSP_DSPCHN_RampEnableInfo          sRampEnableInfo;
        BDSP_Raaga_P_UnsupportedAlgoInfo    sUnsupportedAlgoInfo;
    BDSP_Raaga_P_BspScmResponseInfo         sBspScmResponseInfo;
    }uFWMessageInfo;
}BDSP_Raaga_P_AsynEventMsg;


/* This define return the fixed size value for any command
   This SIZEOF() macro return the value in bytes */
#define BDSP_RAAGA_P_COMMAND_SIZE_IN_BYTES             SIZEOF(BDSP_Raaga_P_Command)

/* This define return the fixed size value for any Ack */
#define BDSP_RAAGA_ACK_SIZE_IN_BYTES                   SIZEOF(BDSP_Raaga_P_Response)

/* This define return the fixed size value for any Response */
#define BDSP_RAAGA_RESPONSE_SIZE_IN_BYTES              SIZEOF(BDSP_Raaga_P_Response)

/* This define return the fixed size value for any Async Event */
#define BDSP_RAAGA_ASYNC_RESPONSE_SIZE_IN_BYTES        SIZEOF(BDSP_Raaga_P_AsynEventMsg)

#endif
