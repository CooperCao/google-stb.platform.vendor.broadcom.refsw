/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/

#ifndef NEXUS_SYSLIB_FRAMEWORK_H__
#define NEXUS_SYSLIB_FRAMEWORK_H__

#include "bsyslib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_SYSlib_ContextSettings
{
	void * module;
} NEXUS_SYSlib_ContextSettings;

typedef struct NEXUS_SYSlib_Context * NEXUS_SYSlib_ContextHandle;

void NEXUS_SYSlib_GetDefaultContextSettings_priv(
	NEXUS_SYSlib_ContextSettings * pSettings
);

NEXUS_Error NEXUS_SYSlib_CreateContext_priv(
	NEXUS_SYSlib_ContextHandle * pContext, 
	const NEXUS_SYSlib_ContextSettings * pSettings
);

void NEXUS_SYSlib_DestroyContext_priv(
	NEXUS_SYSlib_ContextHandle context
);

void      NEXUS_SYSlib_P_GetTime_isr(void * pvParm1, int iParm2, unsigned long * pulNow);
BERR_Code NEXUS_SYSlib_P_CreateTimer(void * pvParm1, int iParm2, BSYSlib_Timer_Handle *phTimer);
void      NEXUS_SYSlib_P_DestroyTimer(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);
BERR_Code NEXUS_SYSlib_P_StartTimer_isr(void * pvParm1, int iParm2,  BSYSlib_Timer_Handle hTimer, const BSYSlib_Timer_Settings * psSettings);
void      NEXUS_SYSlib_P_CancelTimer_isr(void * pvParm1, int iParm2, BSYSlib_Timer_Handle hTimer);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_SYSLIB_FRAMEWORK_H__ */

