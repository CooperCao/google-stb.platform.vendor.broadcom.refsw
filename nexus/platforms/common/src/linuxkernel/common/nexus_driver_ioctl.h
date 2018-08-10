/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* API Description:
*   API name: Platform (private)
*    This file enumerates linuxkernel driver supported on the given platform
*
***************************************************************************/

#ifndef __NEXUS_DRIVER_IOCTL_H_
#define __NEXUS_DRIVER_IOCTL_H_

/* This file cannot #include linux header files. It defines a Broadcom-only API to the driver. */
#include "nexus_memory.h"
#include "nexus_base_ioctl.h"
#ifdef NEXUS_CONFIG_IMAGE
#include "nexus_img_ioctl.h"
#endif

#define PROXY_NEXUS_CALLBACK_PACKET 4

typedef struct nexus_driver_callback_desc {
    uint64_t interfaceHandle; /* interface handle that is used in NEXUS_Start/StopCallbacks */
    struct {
        uint64_t callback;
        uint64_t context;
        int param;
    } desc; /* NEXUS_CallbackDesc */
} nexus_driver_callback_desc;

#define NEXUS_PROXY_IOCTL(NUM, NAME) NEXUS_IOCTL(101, (NEXUS_IOCTL_PROXY_MODULE*NEXUS_IOCTL_PER_MODULE)+(NUM), NAME)

#define IOCTL_PROXY_NEXUS_Open NEXUS_PROXY_IOCTL(0, int)

/* PROXY_NEXUS_Scheduler is used to dequeue, not run */
typedef struct PROXY_NEXUS_Scheduler {
    struct {
        unsigned priority;
    } in;
    struct {
        unsigned count;
        nexus_driver_callback_desc callbacks[PROXY_NEXUS_CALLBACK_PACKET];
    } out;
} PROXY_NEXUS_Scheduler;
#define IOCTL_PROXY_NEXUS_Scheduler NEXUS_PROXY_IOCTL(1, PROXY_NEXUS_Scheduler)

typedef struct PROXY_NEXUS_SchedulerLock {
    unsigned priority;
    bool lock;
} PROXY_NEXUS_SchedulerLock;
#define IOCTL_PROXY_NEXUS_SchedulerLock NEXUS_PROXY_IOCTL(3, PROXY_NEXUS_SchedulerLock)

typedef struct PROXY_NEXUS_AuthenticatedJoin {
    NEXUS_Certificate certificate;
} PROXY_NEXUS_AuthenticatedJoin;
#define IOCTL_PROXY_NEXUS_AuthenticatedJoin NEXUS_PROXY_IOCTL(4, PROXY_NEXUS_AuthenticatedJoin)

#ifdef NEXUS_CONFIG_IMAGE
#define IOCTL_PROXY_NEXUS_Image NEXUS_PROXY_IOCTL(5, BIMG_Ioctl)
#endif

typedef struct PROXY_NEXUS_Log_Instance {
    unsigned instance;
} PROXY_NEXUS_Log_Instance;

typedef struct PROXY_NEXUS_Log_Activate {
    unsigned debug_log_size;
} PROXY_NEXUS_Log_Activate;

typedef struct PROXY_NEXUS_Log_Dequeue {
    PROXY_NEXUS_Log_Instance instance;
    unsigned timeout;
    uint64_t buffer;
    unsigned buffer_size;
} PROXY_NEXUS_Log_Dequeue;


#define IOCTL_PROXY_NEXUS_Log_Activate  NEXUS_PROXY_IOCTL(11, PROXY_NEXUS_Log_Activate)
#define IOCTL_PROXY_NEXUS_Log_Create    NEXUS_PROXY_IOCTL(12, PROXY_NEXUS_Log_Instance)
#define IOCTL_PROXY_NEXUS_Log_Dequeue   NEXUS_PROXY_IOCTL(13, PROXY_NEXUS_Log_Dequeue)
#define IOCTL_PROXY_NEXUS_Log_Destroy   NEXUS_PROXY_IOCTL(14, PROXY_NEXUS_Log_Instance)
#define IOCTL_PROXY_NEXUS_Log_Deactivate    NEXUS_PROXY_IOCTL(15, int)
#define IOCTL_PROXY_NEXUS_Log_Test      NEXUS_PROXY_IOCTL(16, int)

/* this PROXY uninit is used to signal clean shutdown. it is different the actual IOCTL_PLATFORM_NEXUS_Platform_Uninit */
#define IOCTL_PROXY_NEXUS_Platform_Uninit   NEXUS_PROXY_IOCTL(17, int)

/* PROXY_NEXUS_RunScheduler is used to run, not dequeue */
typedef struct PROXY_NEXUS_RunScheduler {
    struct {
        unsigned timeout;
        unsigned priority;
    } in;
    struct {
        bool has_callbacks; /* if true, call IOCTL_PROXY_NEXUS_Scheduler to get them */
    } out;
} PROXY_NEXUS_RunScheduler;
#define IOCTL_PROXY_NEXUS_RunScheduler NEXUS_PROXY_IOCTL(18, PROXY_NEXUS_RunScheduler)

#define IOCTL_PROXY_NEXUS_StopCallbacks       NEXUS_PROXY_IOCTL(19, int)
#define IOCTL_PROXY_NEXUS_StartCallbacks      NEXUS_PROXY_IOCTL(20, int)

#if NEXUS_CPU_ARM
typedef struct PROXY_NEXUS_CacheFlush {
    uint64_t address;     /* Virtual memory address */
    unsigned length;       /* Length of range (in bytes) */
} PROXY_NEXUS_CacheFlush;

#define IOCTL_PROXY_NEXUS_CacheFlush      NEXUS_PROXY_IOCTL(21, PROXY_NEXUS_CacheFlush)
#endif

#define IOCTL_PROXY_NEXUS_WakeupScheduler    NEXUS_PROXY_IOCTL(22, int)

typedef struct PROXY_NEXUS_GetOsConfig {
    bool os_64bit;
} PROXY_NEXUS_GetOsConfig;

#define IOCTL_PROXY_NEXUS_GetOsConfig     NEXUS_PROXY_IOCTL(23, PROXY_NEXUS_GetOsConfig)

#endif /* __NEXUS_DRIVER_IOCTL_H_ */

