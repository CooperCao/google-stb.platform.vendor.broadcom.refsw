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
******************************************************************************/
#ifndef __ASTRATOP_UTILS_H__
#define __ASTRATOP_UTILS_H__
#include <stdint.h>
#define MAX_NUM_CORES 32
#define MAX_NUM_TASKS 1024
#define SOCK_PATH "socket"

/*Primary command options*/
#define ASTRATOP_CMD_GET_CPUCORES 1
#define ASTRATOP_CMD_GET_TASKINFO 2
/*Secondary command options*/
#define ASTRATOP_CMD_GET_PER_CORE_TASKS_INFO 1
#define ASTRATOP_CMD_GET_TASKID_INFO 2
/*Tasktype options*/
#define RT_TASKS 1
#define NRT_TASKS 2

/*Cores request details*/
struct coresReqDetails
{
    uint32_t coresBitMask;
    uint8_t taskType;
};

struct astraTopRequest
{
    uint8_t cmd;
    uint32_t secondaryCmd;
    struct coresReqDetails coresRequest;
    uint16_t taskId;
};

struct cpuCore
{
    uint8_t cpuId;
    uint16_t noOfTasks;
    uint16_t noOfNRTTasks;
    uint16_t noOfRTTasks;
};

struct cpuCoresInformation
{
    uint8_t noOfCores;
    struct cpuCore cores[MAX_NUM_CORES];
};

struct task
{
    struct taskInfo
    {
        unsigned int moduleID            :4;
        unsigned int CPUID               :2;
        unsigned int taskID              :10;
        unsigned int taskStatus          :1;
        union
        {
            /*Valid for CFS Tasks*/
            unsigned int priority            :10;
            /*Valid for EDF Tasks*/
            unsigned int taskLoad            :10;
        };
        float CPUper;
    }taskDetails;
    uint8_t taskType;
    long long unsigned int timeStamp;
};

struct tasksInformation
{
    uint16_t noOfTasks;
    struct task tasks[MAX_NUM_TASKS + 1];
};

struct astraTopResponse
{
    uint32_t CPUCoresBitMask; /*BitMask of enabled CPU cores in the system*/
    struct cpuCoresInformation coresInfo;
    struct tasksInformation tasksInfo;
};
#endif /*__ASTRATOP_UTILS_H__ */
