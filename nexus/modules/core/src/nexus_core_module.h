/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Coree
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_CORE_MODULE_H__
#define NEXUS_CORE_MODULE_H__

#include "nexus_base.h"
#include "nexus_core_thunks.h"
#include "nexus_core_priv.h"
#include "nexus_memory.h"
#include "nexus_watchdog.h"
#include "nexus_core_utils.h"
#include "nexus_avs.h"
#include "nexus_power_management.h"
#if NEXUS_AVS_EXTENSION
#include "nexus_avs_extension.h"
#endif
#include "priv/nexus_core.h"
#include "priv/nexus_core_video.h"
#include "priv/nexus_core_audio.h"
#include "nexus_platform_features.h"
#include "nexus_memory_priv.h"
#include "bmma_pool.h"

struct NEXUS_FileNameNode;
BLST_AA_TREE_HEAD(NEXUS_P_FileNameTree, NEXUS_FileNameNode);

typedef struct NEXUS_Core_P_State {
    NEXUS_Core_MagnumHandles publicHandles;
    NEXUS_Core_Settings cfg;
    NEXUS_TimerHandle timer;
    NEXUS_ModuleHandle module;
#if NEXUS_AVS_MONITOR
    NEXUS_TimerHandle pvtTimer;
#endif
    BLST_D_HEAD(NEXUS_P_MemoryBlock_List,  NEXUS_MemoryBlock) allocatedBlocks;
    bool standby;
    BMMA_PoolAllocator_Handle memoryBlockPool;
#if BDBG_DEBUG_BUILD
    struct NEXUS_P_FileNameTree fileNameTree;
#endif
} NEXUS_Core_P_State;


extern NEXUS_Core_P_State g_NexusCore;

#define NEXUS_MODULE_SELF g_NexusCore.module

NEXUS_Error NEXUS_Core_P_Profile_Init(void);
void NEXUS_Core_P_Profile_Uninit(void);

void NEXUS_PowerManagement_Init(void);
void NEXUS_PowerManagement_Uninit(void);

void NEXUS_VideoInput_P_Destroy(NEXUS_VideoInputHandle videoInput);
void NEXUS_VideoOutput_P_Destroy(NEXUS_VideoOutputHandle videoOutput);
void NEXUS_AudioOutput_P_Destroy(NEXUS_AudioOutputHandle audioOutput);

void NEXUS_MemoryBlock_P_Print(void);

NEXUS_Error NEXUS_Watchdog_P_Init(void);
void NEXUS_Watchdog_P_Uninit(void);


/* #define B_MODULE_OBJDB_EXTRA B_OBJDB_TABLE_ENTRY(NEXUS_VideoInput, NULL, NEXUS_VideoInput_P_Destroy) */

#endif /* NEXUS_CORE_MODULE_H__ */

