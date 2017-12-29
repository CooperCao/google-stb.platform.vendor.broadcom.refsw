/******************************************************************************
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
 ******************************************************************************/

#ifndef NEXUS_SAGE_PRIV_H__
#define NEXUS_SAGE_PRIV_H__

#include "nexus_base_types.h"
#include "bsagelib.h"
#include "priv/nexus_core.h"
#include "bavc_types.h"
#include "bsagelib_types.h"
#include "nexus_sage_types.h"
#include "nexus_sage_init.h"


#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
private API

For any of the following 'private' API, it is caller responsibility to lock Sage module:
in order to lock/unlock the Sage Module, caller must use NEXUS_Lock_Module(sageModuleHandle)
prior the NEXUS_Sage_*_priv() call and NEXUS_Unlock_Module(sageModuleHandle) after
***************************************************************************/

void NEXUS_Sage_GetSageLib_priv(BSAGElib_Handle *pSAGElibHandle);
NEXUS_Error NEXUS_Sage_WaitSage_priv(void);

/* allocate memory with SAGE constrains */
void * NEXUS_Sage_Malloc_priv(size_t size);
void * NEXUS_Sage_MallocRestricted_priv(size_t size);

NEXUS_Error NEXUS_Sage_AddWatchdogEvent_priv(BKNI_EventHandle event);
void NEXUS_Sage_RemoveWatchdogEvent_priv(BKNI_EventHandle event);

/* Standby/Resume */
NEXUS_Error NEXUS_SageModule_Standby_priv( bool enabled, const NEXUS_StandbySettings *pSettings);

NEXUS_Error NEXUS_Sage_SecureRemap(unsigned memcIndex, const BDTU_RemapSettings *pSettings);
NEXUS_Error NEXUS_Sage_UpdateHeaps(void);
NEXUS_Error NEXUS_Sage_AddSecureCores(const BAVC_CoreList *pCoreList);
void NEXUS_Sage_RemoveSecureCores(const BAVC_CoreList *pCoreList);
NEXUS_Error NEXUS_Sage_P_SvpEnterS3(void);
NEXUS_Error NEXUS_Sage_P_SvpInit(NEXUS_SageModuleInternalSettings *internalSettings);
NEXUS_Error NEXUS_Sage_P_SvpStart(void);
void NEXUS_Sage_P_SvpStop(bool reset);
void NEXUS_Sage_P_SvpInitDelayed(void *pParam);
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
void NEXUS_Sage_P_SvpUninit(bool reset);
#else
void NEXUS_Sage_P_SvpUninit(void);
#endif
NEXUS_Error NEXUS_Sage_P_BP3Init(NEXUS_SageModuleSettings *pSettings);
void NEXUS_Sage_P_BP3Uninit(void);
NEXUS_Error NEXUS_Sage_P_ARInit(NEXUS_SageModuleSettings *pSettings);
void NEXUS_Sage_P_ARUninit(BSAGElib_eStandbyMode standbyMode);
NEXUS_Error NEXUS_Sage_P_SystemCritRestartCheck(void *pSettings);
NEXUS_Error NEXUS_SageModule_P_AddRegion(uint32_t id, NEXUS_Addr offset, uint32_t size);
NEXUS_Error NEXUS_Sage_P_SvpSetRegions(void);
NEXUS_Error NEXUS_Sage_P_SARMInit(NEXUS_SageModuleSettings *pSettings);
void NEXUS_Sage_P_SARMUninit(void);

#ifdef NEXUS_SAGE_SVP_TEST
NEXUS_Error NEXUS_Sage_P_SecureCores_test(const BAVC_CoreList *pCoreList, bool add);
#endif

typedef struct NEXUS_SageMemoryBlock {
    size_t len;
    void *buf;
} NEXUS_SageMemoryBlock;

typedef struct NEXUS_SageImageHolder {
    const char *name;         /* printable name */
    SAGE_IMAGE_FirmwareID id; /* SAGE_IMAGE_FirmwareID_eFramework or SAGE_IMAGE_FirmwareID_eBootLoader */
    NEXUS_SageMemoryBlock *raw;
} NEXUS_SageImageHolder;

NEXUS_Error NEXUS_Sage_LoadImage_priv(NEXUS_SageImageHolder *holder);

NEXUS_Error NEXUS_Sage_P_SecureLog_Init(const NEXUS_SageModuleSettings *pSettings);
void NEXUS_Sage_P_SecureLog_Uninit(void);
void NEXUS_Sage_P_PrintSecureLog(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_SAGE_PRIV_H__ */
