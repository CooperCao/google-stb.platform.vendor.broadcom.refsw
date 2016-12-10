/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/
#ifndef NEXUS_PLATFORM_LOCAL_PRIV_H__
#define NEXUS_PLATFORM_LOCAL_PRIV_H__

#include "nexus_platform.h"

/* nexus_platform_local.c */
NEXUS_Error NEXUS_Platform_P_Magnum_Init(void);
void NEXUS_Platform_P_Magnum_Uninit(void);
NEXUS_Error NEXUS_Platform_P_ModulesInit(const NEXUS_PlatformSettings *pSettings);
void NEXUS_Platform_P_ModulesUninit(void);

/* nexus_platform_privilege.c */
/* Change from privileged user (root) to unprivileged user (non-root). */
NEXUS_Error NEXUS_Platform_P_DropPrivilege(const NEXUS_PlatformSettings *pSettings);
/* read box mode from device tree or env variable */
unsigned NEXUS_Platform_P_ReadBoxMode(void);
/* read board id from proc */
unsigned NEXUS_Platform_P_ReadBoardId(void);
/* read pmap id from device tree or env variable */
unsigned NEXUS_Platform_P_ReadPMapId(void);

NEXUS_Error NEXUS_Platform_P_InitWakeupDriver(void);
void NEXUS_Platform_P_UninitWakeupDriver(void);
NEXUS_Error NEXUS_Platform_P_InitThermalMonitor(void);
void NEXUS_Platform_P_UninitThermalMonitor(void);

/* Allows system to set in-line OS thread priority for scheduler threads.
Called once per scheduler thread at system init.
For the server process, there is one thread for each module priority.
For client processes, there is only one thread; the 'priority' param passed is just NEXUS_ModulePriority_eMax.
See NEXUS_PLATFORM_PROXY_INTF build variable for file-replacement.
 */
void nexus_p_set_scheduler_thread_priority(NEXUS_ModulePriority priority);

#endif /* #ifndef NEXUS_PLATFORM_LOCAL_PRIV_H__ */
