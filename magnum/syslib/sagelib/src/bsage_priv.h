/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/

#ifndef BSAGE_PRIV_H_
#define BSAGE_PRIV_H_

#include "breg_mem.h"
#include "bchp.h"
#include "bint.h"
#include "btmr.h"
#include "bhsi.h"

#include "bchp_pwr.h"
#include "blst_slist.h"
#include "blst_squeue.h"
#include "priv/bsagelib_rpc_shared.h"

#include "bsp_s_otp_common.h" /* provide BCMD_Otp_CmdMsp_e */
#include "bsp_s_keycommon.h" /* provide BCMD_VKLID_e */

#include "bsage.h"
#include "bsage_management.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BSAGE_P_RpcRemote {
    BLST_S_ENTRY(BSAGE_P_RpcRemote) link; /* member of a linked list */
    BSAGE_Handle hSAGE;
    uint32_t platformId;
    uint32_t moduleId;
    void *async_arg;
    BSAGElib_RpcMessage *message;
    BSAGElib_InOutContainer *container;
    uint8_t valid;
    uint8_t terminated;
    /* Resources for callbacks comming from SAGE */
    struct {
        uint32_t sequence;
        BSAGElib_RpcMessage *message;
        BSAGElib_InOutContainer *container;
    } callbacks;

    BSAGE_RpcInterface rpc;
};

typedef struct BSAGE_Management_CallbackItem {
    BSAGE_ManagementInterface watchdog;
    BLST_SQ_ENTRY(BSAGE_Management_CallbackItem) link; /* member of a linked list */
} BSAGE_Management_CallbackItem;

typedef struct BSAGE_P_Instance
{
    BLST_S_HEAD(BSAGE_P_RpcRemoteList, BSAGE_P_RpcRemote) remotes;
    BREG_Handle hReg;
    BINT_Handle hInt;
    BTMR_Handle hTmr;

    BTMR_TimerHandle hTimer;

    BSAGElib_MemoryMapInterface i_memory_map;
    BSAGElib_MemorySyncInterface i_memory_sync;
    BSAGElib_SyncInterface i_sync_sage;
    BSAGElib_SyncInterface i_sync_hsm;

    /* Rpc */
    uint32_t instanceIdGen;
    uint32_t seqIdGen;
    BHSI_Handle hHsi;
    uint8_t *hsi_buffers;

    /* Management */
    BLST_SQ_HEAD(BSAGE_Management_WatchdogCallbackList, BSAGE_Management_CallbackItem) watchdog_callbacks;
    BINT_CallbackHandle watchdogIntCallback; /* watchdog timeout Interrupt Callback */

    uint8_t resetPending;

    BSAGElib_ChipInfo chipInfo;

    uint8_t enablePinmux;

    BCMD_VKLID_e vkl1;
    BCMD_VKLID_e vkl2;

    BSAGElib_ImageInfo bootloaderInfo;
    BSAGElib_ImageInfo frameworkInfo;

    bool bBootPostCalled;

} BSAGE_P_Instance;

/* common interfaces */
#define BSAGE_iAddrToOffset          hSAGE->i_memory_map.addr_to_offset
#define BSAGE_iOffsetToAddr          hSAGE->i_memory_map.offset_to_addr
#define BSAGE_iFlush                 hSAGE->i_memory_sync.flush
#define BSAGE_iInvalidate            hSAGE->i_memory_sync.invalidate
#define BSAGE_iLockHsm               hSAGE->i_sync_hsm.lock
#define BSAGE_iUnlockHsm             hSAGE->i_sync_hsm.unlock

#define _SAGE_call(FCT) if (FCT) { FCT(); }
#define BSAGE_iLockSage        _SAGE_call(hSAGE->i_sync_sage.lock)
#define BSAGE_iUnlockSage      _SAGE_call(hSAGE->i_sync_sage.unlock)

extern BSAGE_Handle hSAGE_Global;

/* friendly functions */
void BSAGE_P_Rpc_Uninit(BSAGE_Handle hSAGE);
void BSAGE_P_Rpc_Reset(BSAGE_Handle hSAGE);
void BSAGE_P_Rpc_Reset_isrsafe(BSAGE_Handle hSAGE);
BERR_Code BSAGE_P_Management_Initialize(BSAGE_Handle hSAGE);
void BSAGE_P_Management_Uninitialize(BSAGE_Handle hSAGE);
BERR_Code BSAGE_P_Init_Serial(BSAGE_Handle hSAGE);
BERR_Code BSAGE_P_GetOtp(BSAGE_Handle hSAGE,BHSM_Handle hHsm, BCMD_Otp_CmdMsp_e msp_enum, uint32_t *out, const char *dbg_name);
BERR_Code BSAGElib_P_SageVklsInit(BSAGElib_Handle hSAGElib);

#ifdef __cplusplus
}
#endif


#endif /* BSAGE_PRIV_H_ */
