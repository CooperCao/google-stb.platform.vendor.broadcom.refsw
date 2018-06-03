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
 * Module Description:
 *
 **************************************************************************/

#ifndef NEXUS_SCM_MODULE_H__
#define NEXUS_SCM_MODULE_H__

#include "nexus_scm_thunks.h"
#include "nexus_base.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#include "nexus_base.h"
#include "nexus_scm_init.h"
#include "nexus_scm.h"

#include "btmr.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME scm
#define NEXUS_MODULE_SELF g_NEXUS_scmModule.moduleHandle

/* ScmChannel context */
typedef struct NEXUS_ScmChannel {
    NEXUS_OBJECT(NEXUS_ScmChannel);
    NEXUS_ScmHandle scm;
    NEXUS_ScmType type;
    /* version stored in OTP */
    uint32_t otp_version[4];
    /* version stored in the image */
    uint32_t version;
} NEXUS_ScmChannel;
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_ScmChannel);

/* Scm instance context */
typedef struct NEXUS_Scm {
    NEXUS_OBJECT(NEXUS_Scm);
    NEXUS_ScmChannel channel;
    int channel_cnt;
} NEXUS_Scm;


typedef struct NEXUS_ScmModule_P_Handle
{
    NEXUS_ModuleHandle moduleHandle; /* module handle */
    NEXUS_ModuleHandle moduleSecurityHandle;

    /* Timer */
    BTMR_TimerHandle hTimer; /* timer handle to read timestamps */
    unsigned timerMax;       /* value retrieve by BTMR_ReadTimerMax() */
    unsigned initTimeUs;     /* Initialization timestamp in micro sec. */

    BKNI_EventHandle hEventReceive; /* receive event to sync on channel destroy */

    uint8_t reset;                  /* 0= no, SCM CPU is running. ;  1= yes, SCM CPU is in reset */
    uint8_t booted;                 /* 0= no, SCM-side is not ready.;1= yes, SCM-side is ready   */

    NEXUS_ScmChannelHandle waitingChannel; /* currently in a WaitForEvent on hEventReceive */
    NEXUS_Scm * instance;
    /* scm communication buffers */
    void * sendBuf;
    void * recvBuf;
} NEXUS_ScmModule_P_Handle;


typedef enum NEXUS_ScmFirmwareType
{
    NEXUS_ScmFirmwareType_eKernel,
    NEXUS_ScmFirmwareType_eBl
} NEXUS_ScmFirmwareType;


#ifndef NEXUS_PLATFORM_DEFAULT_HEAP
#define SCM_FULL_HEAP NEXUS_MEMC0_MAIN_HEAP
#else
#define SCM_FULL_HEAP NEXUS_PLATFORM_DEFAULT_HEAP
#endif


extern NEXUS_ScmModule_P_Handle g_NEXUS_scmModule;

/* private init and cleanup functions for the Host to SCM communication */
NEXUS_Error NEXUS_Scm_P_VarInit(void);
void NEXUS_Scm_P_VarCleanup(void);

/* utility functions */
void * NEXUS_Scm_P_Malloc(size_t size);
void * NEXUS_Scm_P_MallocRestricted(size_t size);
NEXUS_Error NEXUS_Scm_P_ConfigureAlloc(void);
int NEXUS_Scm_P_IsHeapValid(NEXUS_HeapHandle heap, NEXUS_MemoryType memoryType);
int NEXUS_Scm_P_IsMemoryRestricted_isrsafe(const void *address);
void * NEXUS_Scm_P_MallocAligned(size_t size, size_t alignment);

/* SCM boot verification */
int NEXUS_Scm_P_CheckScmBooted(void);
/* (re)Start SCM */
NEXUS_Error NEXUS_ScmModule_P_Start(void);
NEXUS_Error NEXUS_Scm_P_DtaModeSelect(uint32_t mode);

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Scm);

/* watchdog related */
NEXUS_Error NEXUS_Scm_P_WatchdogInit(void);
void NEXUS_Scm_P_WatchdogUninit(void);
NEXUS_Error NEXUS_Scm_P_WatchdogLink(NEXUS_ScmHandle scm, const NEXUS_CallbackDesc *watchdogCallback);
void NEXUS_Scm_P_WatchdogUnlink(NEXUS_ScmHandle scm);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SCM_MODULE_H__ */
