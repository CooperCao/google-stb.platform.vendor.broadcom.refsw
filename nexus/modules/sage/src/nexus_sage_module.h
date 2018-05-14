/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/

#ifndef NEXUS_SAGE_MODULE_H__
#define NEXUS_SAGE_MODULE_H__

#include "nexus_sage_thunks.h"
#include "nexus_base.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"
#include "nexus_base.h"
#include "nexus_sage_init.h"
#include "nexus_sage_types.h"
#include "nexus_sage.h"
#include "priv/nexus_sage_priv.h"
#include "nexus_sage_message_types.h"

#include "btmr.h"
#include "bavc_types.h"

#include "blst_list.h"
#include "blst_squeue.h"

#include "bsagelib.h"
#include "bsagelib_rpc.h"

#include "nexus_sage_image.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME sage
#define NEXUS_MODULE_SELF g_NEXUS_sageModule.moduleHandle

typedef struct NEXUS_SageIndicationContext_s {
    BLST_SQ_ENTRY(NEXUS_SageIndicationContext_s) link;
    /* struct { struct SAGE_P_IndicationContext_s *sq_next; } link; */
    uint32_t id;
    uint32_t value;
} NEXUS_SageIndicationContext;

/* SageChannel context */
typedef BLST_SQ_HEAD(NEXUS_SageIndicationQueueHead_s, NEXUS_SageIndicationContext_s) NEXUS_SageIndicationQueueHead;
typedef struct NEXUS_SageChannel {
    NEXUS_OBJECT(NEXUS_SageChannel);
    BLST_D_ENTRY(NEXUS_SageChannel) link; /* member of a linked list */

    /* refer to the upper sage instance for fast access */
    NEXUS_SageHandle sage;

    NEXUS_SageChannelStatus status;
    bool valid;

    BSAGElib_RpcRemoteHandle sagelib_remote;

    /* response callbacks */
    NEXUS_IsrCallbackHandle successCallback;
    NEXUS_IsrCallbackHandle errorCallback;

    /* callbacks from SAGE */
    NEXUS_IsrCallbackHandle callbackRequestRecvCallback;

    /* TA termination indication callback */
    NEXUS_IsrCallbackHandle taTerminateCallback;

    /* SAGE indication [id, value] callback */
    NEXUS_IsrCallbackHandle indicationCallback;

    /* Queue of indications to consume.
       See NEXUS_SageChannel_GetNextIndication() */
    NEXUS_SageIndicationQueueHead indications;
} NEXUS_SageChannel;

/* Sage instance context */
typedef struct NEXUS_Sage {
    NEXUS_OBJECT(NEXUS_Sage);
    BLST_S_ENTRY(NEXUS_Sage) link; /* member of a linked list */
    unsigned index;
    BSAGElib_ClientHandle hSAGElibClient;
    bool valid;

    /* SageChannel linked list */
    BLST_D_HEAD(NEXUS_SageChannelList, NEXUS_SageChannel) channels;

    NEXUS_TaskCallbackHandle watchdogCallback;
} NEXUS_Sage;

typedef enum NEXUS_SageSWState
{
    NEXUS_SageSWState_eUnknown,
    NEXUS_SageSWState_eRunning,
    NEXUS_SageSWState_eError
} NEXUS_SageSWState;

typedef struct NEXUS_SageModule_P_Handle
{
    NEXUS_ModuleHandle moduleHandle; /* module handle */
    NEXUS_ModuleHandle moduleSecurityHandle;

    /* Timer */
    NEXUS_Time initTime;
    unsigned initTimeUs;     /* Initialization timestamp in micro sec. */

    BSAGElib_ChipInfo chipInfo;

    BKNI_EventHandle hEventReceive; /* receive event to sync on channel destroy */

    NEXUS_SageSWState SWState;      /* Indicates SW state over boot process */

    NEXUS_SageChannelHandle waitingChannel; /* currently in a WaitForEvent on hEventReceive */
    BLST_S_HEAD(NEXUS_SageList, NEXUS_Sage) instances;

    BSAGElib_Handle hSAGElib;
#define NEXUS_SAGE_MAX_WATCHDOG_EVENTS 4
    BKNI_EventHandle watchdogEvent[NEXUS_SAGE_MAX_WATCHDOG_EVENTS];

    BKNI_EventHandle sageReadyEvent;
} NEXUS_SageModule_P_Handle;


typedef enum NEXUS_SageFirmwareType
{
    NEXUS_SageFirmwareType_eKernel,
    NEXUS_SageFirmwareType_eBl
} NEXUS_SageFirmwareType;


#ifndef NEXUS_PLATFORM_DEFAULT_HEAP
#define SAGE_FULL_HEAP NEXUS_MEMC0_MAIN_HEAP
#else
#define SAGE_FULL_HEAP NEXUS_PLATFORM_DEFAULT_HEAP
#endif


extern NEXUS_SageModule_P_Handle g_NEXUS_sageModule;

/* private init and cleanup functions for the Host to SAGE communication */
NEXUS_Error NEXUS_Sage_P_VarInit(void);
void NEXUS_Sage_P_VarCleanup(void);

/* utility functions */
void * NEXUS_Sage_P_Malloc(size_t size);
void * NEXUS_Sage_P_MallocRestricted(size_t size);
NEXUS_Error NEXUS_Sage_P_ConfigureAlloc(void);
int NEXUS_Sage_P_IsHeapValid(NEXUS_HeapHandle heap, NEXUS_MemoryType memoryType);
int NEXUS_Sage_P_IsMemoryRestricted_isrsafe(const void *address);

/* SAGE boot verification */
int NEXUS_Sage_P_CheckSageBooted(void);

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_Sage);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_SageChannel);

void NEXUS_Sage_P_GetSAGElib_ClientSettings(BSAGElib_ClientSettings *pSAGElibClientSettings);

/* watchdog related */
NEXUS_Error NEXUS_Sage_P_WatchdogInit(void);
void NEXUS_Sage_P_WatchdogUninit(void);
NEXUS_Error NEXUS_Sage_P_WatchdogLink(NEXUS_SageHandle sage, const NEXUS_CallbackDesc *watchdogCallback);
void NEXUS_Sage_P_WatchdogUnlink(NEXUS_SageHandle sage);

/* Init/Cleanup contexts */
void NEXUS_Sage_P_Init(void);
void NEXUS_Sage_P_Cleanup(void);

/* Start/kick-off SAGE */
NEXUS_Error NEXUS_SageModule_P_Start(void);

/* *_priv functions cleanup */
void NEXUS_SageModule_P_PrivClean(void);

/* when transiting into S3, hsm is closed, on resume the handle needs to be refreshed */
NEXUS_Error NEXUS_Sage_P_SAGElibUpdateHsm(bool set);

NEXUS_Error NEXUS_Sage_P_Status(NEXUS_SageStatus *pStatus);

/*pBuff size should be equal to CRRBuffsize + 16bytes for AES enc alignment*/
NEXUS_Error NEXUS_Sage_P_GetLogBuffer(uint8_t *pBuff,  uint32_t inputBufSize,
                                      uint32_t *pBufSize, uint32_t *pWrapBufSize,
                                      uint32_t *pActualBufSize, uint32_t *pActualWrapBufSize);

/*pKeyBuff Size should be 256 bytes to hold RSA2048 encrypted key*/
NEXUS_Error NEXUS_Sage_P_GetEncKey(uint8_t *pKeyBuff,uint32_t inputKeyBufSize, uint32_t *pOutKeySize);

void NEXUS_Sage_P_SAGELogUninit(void);

NEXUS_Error Nexus_SageModule_P_Img_Create(const char *id, void **ppContext, BIMG_Interface  *pInterface);
void Nexus_SageModule_P_Img_Destroy(void *pContext);
NEXUS_Error NEXUS_SageModule_P_Load(NEXUS_SageImageHolder *holder, BIMG_Interface *img_interface, void *img_context);
void NEXUS_Sage_P_PrintSvp(void);

extern const struct NEXUS_SageSvpHwBlock {
    const char *achName;
} g_NEXUS_SvpHwBlockTbl[];

#define SAGE_ALIGN_SIZE (4096)
#define RoundDownP2(VAL, PSIZE) ((VAL) & (~(PSIZE-1)))
#define RoundUpP2(VAL, PSIZE)   RoundDownP2((VAL) + PSIZE-1, PSIZE)

NEXUS_Error NEXUS_SageModule_P_GetHeapBoundaries(int heapid, NEXUS_Addr *offset, uint32_t *len, uint32_t *max_free);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SAGE_MODULE_H__ */
