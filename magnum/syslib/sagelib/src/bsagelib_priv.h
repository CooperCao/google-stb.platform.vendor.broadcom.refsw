 /******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#ifndef BSAGELIB_PRIV_H_
#define BSAGELIB_PRIV_H_

#include "breg_mem.h"
#include "bchp.h"
#include "bint.h"
#include "btmr.h"
#include "bhsi.h"

#include "bchp_pwr.h"
#include "blst_slist.h"
#include "blst_squeue.h"
#include "bsagelib.h"
#include "priv/bsagelib_rpc_shared.h"
#include "bsagelib_types.h"
#include "bsagelib_boot.h"
#include "bsagelib_tools.h"
#include "bsagelib_client.h"
#include "bsagelib_management.h"

#if (BHSM_API_VERSION==1)
#include "bsp_s_otp_common.h" /* provide BCMD_Otp_CmdMsp_e */
#include "bsp_s_keycommon.h" /* provide BCMD_VKLID_e */
#else
#include "bhsm_keyladder.h" /* BHSM_KeyLadderHandle */
#if (BHSM_ZEUS_VERSION < BHSM_ZEUS_VERSION_CALC(5,0))
#include "bsp_s_otp_common.h"
#else
typedef uint32_t BCMD_Otp_CmdMsp_e;
#endif
#endif

#ifdef SAGE_KO
#include "bsage.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct BSAGElib_P_CallbackItem {
    BLST_SQ_ENTRY(BSAGElib_P_CallbackItem) link; /* member of a linked list */
    BSAGElib_RpcRemoteHandle remote;
    uint64_t containerOffset;
    BERR_Code rc;
    uint32_t async_id;
} BSAGElib_CallbackItem;

BDBG_OBJECT_ID_DECLARE(BSAGElib_P_RpcRemote);
struct BSAGElib_P_RpcRemote {
    BDBG_OBJECT(BSAGElib_P_RpcRemote)
    BLST_S_ENTRY(BSAGElib_P_RpcRemote) link; /* member of a linked list */
    BSAGElib_ClientHandle hSAGElibClient;
    uint32_t platformId;
    uint32_t moduleId;
    void *async_arg;
    BSAGElib_RpcMessage *message;
    BSAGElib_CallbackItem *callbackItem;
    uint8_t open;
#ifdef SAGE_KO
    BSAGE_RpcRemoteHandle hRemote;
#else
    BSAGElib_InOutContainer *container;
    uint8_t valid;
    uint8_t terminated;
    /* Resources for callbacks comming from SAGE */
    struct {
        uint32_t sequence;
        BSAGElib_RpcMessage *message;
        BSAGElib_InOutContainer *container;
    } callbacks;
#endif
};

BDBG_OBJECT_ID_DECLARE(BSAGElib_P_Client);
struct BSAGElib_P_Client {
    BDBG_OBJECT(BSAGElib_P_Client)
    BLST_S_ENTRY(BSAGElib_P_Client) link; /* member of a linked list */
    BSAGElib_Handle hSAGElib;
    BSAGElib_ClientSettings settings;
    BLST_S_HEAD(BSAGElib_P_RpcRemoteList, BSAGElib_P_RpcRemote) remotes;
    BLST_SQ_HEAD(BSAGElib_P_RpcResponseCallbackList, BSAGElib_P_CallbackItem) responseCallbacks;
    BLST_SQ_HEAD(BSAGElib_P_RpcResponseCallbackCache, BSAGElib_P_CallbackItem) responseCallbackCache;
    BSAGElib_Tools_ContainerCacheHandle hContainerCache;
    uint16_t platformNum;
    uint16_t moduleNum;
    BSAGElib_RpcRemoteHandle system_platform;
    BSAGElib_RpcRemoteHandle system_module;
    BSAGElib_RpcRemoteHandle antirollback_platform;
    BSAGElib_RpcRemoteHandle antirollback_module;
};

typedef struct BSAGElib_Management_CallbackItem {
    BSAGElib_Management_WatchdogCallback watchdog_isr;
    BLST_SQ_ENTRY(BSAGElib_Management_CallbackItem) link; /* member of a linked list */
} BSAGElib_Management_CallbackItem;


BDBG_OBJECT_ID_DECLARE(BSAGElib_P_Instance);
struct BSAGElib_P_Instance {
    BDBG_OBJECT(BSAGElib_P_Instance)
    BLST_S_HEAD(BSAGElib_P_ClientList, BSAGElib_P_Client) clients;
    struct {
        BREG_Handle hReg;
        BCHP_Handle hChp;
#ifndef SAGE_KO
        BINT_Handle hInt;
        BTMR_Handle hTmr;
#endif
        BHSM_Handle hHsm;
    } core_handles;

    BSAGElib_MemoryMapInterface i_memory_map;
    BSAGElib_MemorySyncInterface i_memory_sync;
    BSAGElib_MemorySyncInterface i_memory_sync_isrsafe;
    BSAGElib_MemoryAllocInterface i_memory_alloc;
    BSAGElib_SyncInterface i_sync_hsm;

    BSAGElib_ClientSettings defaultClientSettings;

    BSAGElib_ChipInfo chipInfo;
#ifdef SAGE_KO
    BSAGE_Interface  bsage;
#else
    BTMR_TimerHandle hTimer;
    /* Rpc */
    uint32_t instanceIdGen;
    uint32_t seqIdGen;
    BHSI_Handle hHsi;
    uint8_t *hsi_buffers;

    /* Management */
    BLST_SQ_HEAD(BSAGElib_Management_WatchdogCallbackList, BSAGElib_Management_CallbackItem) watchdog_callbacks;
    BINT_CallbackHandle watchdogIntCallback; /* watchdog timeout Interrupt Callback */

    bool bBootPostCalled;
#endif

    uint8_t resetPending;
    uint8_t enablePinmux;
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    BSAGElib_RpcRemoteHandle hStandbyRemote; /* remote used to sent S2 request */
#endif
    BSAGElib_RpcRemoteHandle securelog_module; /* for auto attach TAs to secure_log */
    BSAGElib_InOutContainer *securelogContainer;
    BKNI_EventHandle         securelog_response;

    /* Standby mode */
    BSAGElib_eStandbyMode currentMode;

#if (BHSM_API_VERSION==1)
    BCMD_VKLID_e vkl1;
    BCMD_VKLID_e vkl2;
#else
    BHSM_KeyLadderHandle vklHandle1;
    BHSM_KeyLadderHandle vklHandle2;
#endif

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    char BootImage_BlVerStr[SIZE_OF_BOOT_IMAGE_VERSION_STRING]; /* To hold the string of Boot Loader version */
    char BootImage_OSVerStr[SIZE_OF_BOOT_IMAGE_VERSION_STRING]; /* To hold the string of OS/APP version */
#else
    BSAGElib_ImageInfo bootloaderInfo;
    BSAGElib_ImageInfo frameworkInfo;
#endif

};

/* common interfaces */
#define BSAGElib_iMalloc             hSAGElib->i_memory_alloc.malloc
#define BSAGElib_iMallocRestricted   hSAGElib->i_memory_alloc.malloc_restricted
#define BSAGElib_iFree               hSAGElib->i_memory_alloc.free
#define BSAGElib_iAddrToOffset       hSAGElib->i_memory_map.addr_to_offset
#define BSAGElib_iOffsetToAddr       hSAGElib->i_memory_map.offset_to_addr
#define BSAGElib_iFlush_isrsafe      hSAGElib->i_memory_sync_isrsafe.flush
#define BSAGElib_iInvalidate_isrsafe hSAGElib->i_memory_sync_isrsafe.invalidate

#define BSAGElib_iLockHsm            hSAGElib->i_sync_hsm.lock
#define BSAGElib_iUnlockHsm          hSAGElib->i_sync_hsm.unlock

/* instance specifics */
#define BSAGElib_iRpcResponseRecv_isr   hSAGElibClient->settings.i_rpc.responseRecv_isr
#define BSAGElib_iRpcIndicationRecv_isr hSAGElibClient->settings.i_rpc.indicationRecv_isr
#define BSAGElib_iRpcTATerminate_isr    hSAGElibClient->settings.i_rpc.taTerminate_isr
#define BSAGElib_iRpcResponse_isr       hSAGElibClient->settings.i_rpc.response_isr
#define BSAGElib_iRpcCallbackRequest_isr hSAGElibClient->settings.i_rpc.callbackRequest_isr

#define _SAGElib_call(FCT) if (FCT) { FCT(); }
#define BSAGElib_iLockSecurity    _SAGElib_call(hSAGElibClient->settings.i_sync_security.lock)
#define BSAGElib_iUnlockSecurity  _SAGElib_call(hSAGElibClient->settings.i_sync_security.unlock)
#define BSAGElib_iLockSage        _SAGElib_call(hSAGElibClient->settings.i_sync_sage.lock)
#define BSAGElib_iUnlockSage      _SAGElib_call(hSAGElibClient->settings.i_sync_sage.unlock)

/* friendly functions */
BERR_Code BSAGElib_P_Rpc_Init(BSAGElib_Handle hSAGElib);
void BSAGElib_P_Rpc_Uninit(BSAGElib_Handle hSAGElib);
void BSAGElib_P_Rpc_Reset(BSAGElib_Handle hSAGElib);
void BSAGElib_P_Rpc_Reset_isrsafe(BSAGElib_Handle hSAGElib);
void BSAGElib_P_Rpc_RemoveRemote(BSAGElib_RpcRemoteHandle remote);
BERR_Code BSAGElib_P_Management_Initialize(BSAGElib_Handle hSAGElib);
void BSAGElib_P_Management_Uninitialize(BSAGElib_Handle hSAGElib);
void BSAGElib_P_Rpc_DispatchResponse_isr(BSAGElib_RpcRemoteHandle remote, BERR_Code response_rc);
BERR_Code BSAGElib_P_Rpc_ConsumeResponseCallbacks(BSAGElib_ClientHandle hSAGElibClient, int fire);
BERR_Code BSAGElib_P_Rpc_GetResponse( BSAGElib_ClientHandle hSAGElibClient, BSAGElib_ResponseData *data);
void BSAGElib_P_Rpc_ResponseCallbackCacheFree(BSAGElib_ClientHandle hSAGElibClient);
BERR_Code BSAGElib_P_Init_Serial(BSAGElib_Handle hSAGElib);
BERR_Code BSAGElib_P_GetOtp(BSAGElib_Handle hSAGElib, BCMD_Otp_CmdMsp_e msp_enum, uint32_t *out, uint32_t *outLock, const char *dbg_name);
void BSAGElib_P_Standby_Reset_isrsafe(BSAGElib_Handle hSAGElib);
BERR_Code BSAGElib_P_SageVklsInit(BSAGElib_Handle hSAGElib);
void BSAGElib_P_SageVklsUninit(BSAGElib_Handle hSAGElib);

/* Macros */
#define _EndianSwap(PVAL) (((PVAL)[0] << 24) | ((PVAL)[1] << 16) | ((PVAL)[2] << 8) | ((PVAL)[3]))
#ifdef __cplusplus
}
#endif


#endif /* BSAGELIB_PRIV_H_ */
