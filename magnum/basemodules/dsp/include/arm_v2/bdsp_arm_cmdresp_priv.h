/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef BDSP_ARM_CMDRESP_PRIV_H_
#define BDSP_ARM_CMDRESP_PRIV_H_
#if 0
#include "bdsp_raaga_cmdresp_priv.h"

#define BDSP_ARM_SYSTEM_OPEN_COMMAND_ID ((uint64_t)0X524141474141524d )
#define BDSP_MAX_LIB_NAME_SIZE      48
#define BDSP_ARM_MAX_NUM_ROMFS_LIB      12

#define BDSP_ARM_MAX_NUM_PREEMPTION_LEVELS  3 /* No.of Preemption levels */
#define BDSP_ARM_MAX_NUM_SYSTEM_LEVELS      2 /* Maximum no.of system scheduling levels , Idle and RP*/
#define BDSP_ARM_MAX_NUM_TASKS              2 /* Maximum no.of Tasks to run on DSP */
#define BDSP_ARM_NUM_PROCESS_PER_TASK       2 /* Maximum no.of user process per task */

#define BDSP_ARM_MAX_NUM_SCHED_LEVELS (BDSP_ARM_MAX_NUM_PREEMPTION_LEVELS + BDSP_ARM_MAX_NUM_SYSTEM_LEVELS) /* Total scheduling levels */
#define BDSP_ARM_MAX_NUM_USER_PROCESS BDSP_ARM_MAX_NUM_TASKS * BDSP_ARM_NUM_PROCESS_PER_TASK /* No.of user processs */

#define BDSP_ARM_PERIODIC_TIMER         2000 /*in micro seconds*/
#define BDSP_ARM_WATCHDOG_TIMER          400 /*in milli seconds*/

typedef enum BDSP_ARM_P_eRdbVarIndices
{
    BDSP_ARM_P_eRdbVarIndices_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR = 0,
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

typedef struct BDSP_Arm_P_LibDescriptor
{
    dramaddr_t  ui64StartAddress;
    uint64_t    ui64Size;
    char        ui8LibName[BDSP_MAX_LIB_NAME_SIZE];
}BDSP_Arm_P_LibDescriptor;

typedef enum BDSP_ArmDspCommandType
{
    BDSP_ArmDspCommandType_Open  = 1,  /* Arm System Open*/
    BDSP_ArmDspCommandType_Close = 2,  /* Arm System Close*/
    BDSP_ArmDspCommandType_Max,
    BDSP_ArmDspCommandType_Invalid = 0x7fffffff
}BDSP_ArmDspCommandType;

typedef enum BDSP_ArmDspAck_Type
{
    BDSP_ArmDspAck_Type_eDevice,
    BDSP_ArmDspAck_Type_eTask,
    BDSP_ArmDspAck_Type_eLast,
    BDSP_ArmDspAck_Type_eMax = 0x7FFFFFFF
}BDSP_ArmDspAck_Type;

typedef enum BDSP_ArmDspResp_Type
{
    BDSP_ArmDspResp_Type_eCmdAck,
    BDSP_ArmDspResp_Type_eEvent,
    BDSP_ArmDspResp_Type_eLast,
    BDSP_ArmDspResp_Type_eMax = 0x7FFFFFFF
}BDSP_ArmDspResp_Type;


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

typedef struct BDSP_Arm_P_SchedulingInfo
{
    uint32_t  ui32NumCores;             /* number of cores in the DSP */
    uint32_t  ui32NumUserProcess;       /* number of user processes */
    uint32_t  ui32NumPreemptionLevels;  /* number of pre-emption levels */
    uint32_t  ui32NumSchedulingLevels;  /* number of scheduling levels */

    uint32_t  ui32PreemptiveThreshold[BDSP_ARM_MAX_NUM_SCHED_LEVELS]; /* preempt threshold level for each scheduling level*/
    uint32_t  ui32Dummy0;             /* Element added to make structure size 64 bit aligned*/
}BDSP_Arm_P_SchedulingInfo;

typedef struct BDSP_Arm_P_TimerInfo
{
    uint32_t  ui32PeriodicTimerInUs;  /* Periodicity of Timer interrupt in microseconds */
    uint32_t  ui32WatchdogTimerinMs;  /* Watchdog timer value in ms*/
}BDSP_Arm_P_TimerInfo;

typedef struct BDSP_Arm_P_InitCommand {
    BDSP_Arm_P_CommonMemory         sCommonMemory;      /* RO image sizes */
    BDSP_Arm_P_SchedulingInfo       sSchedulingInfo;    /* Scheduler info */ /* CDN UNIFY */
    BDSP_Arm_P_TimerInfo            sTimerInfo;         /* Timer info for scheduler *//* CDN UNIFY */
    uint64_t                        ui64NumResidentLib;
    BDSP_Arm_P_LibDescriptor        sResidentLibDescriptor[BDSP_ARM_MAX_NUM_ROMFS_LIB];
} BDSP_Arm_P_InitCommand;

typedef struct BDSP_P_Command{
    BDSP_P_CommandHeader  sCommandHeader;
    union{
       BDSP_Arm_P_InitCommand              sInitCommand;
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
}BDSP_P_Command;

typedef struct BDSP_P_Response{
    BDSP_P_ResponseHeader     sResponseHeader;
}BDSP_P_Response;

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
#endif
#endif /*BDSP_ARM_CMDRESP_PRIV_H_*/
