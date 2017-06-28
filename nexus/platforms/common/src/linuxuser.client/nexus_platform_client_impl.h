/***************************************************************************
* Copyright (C) 2004-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
***************************************************************************/
#ifndef NEXUS_PLATFORM_CLIENT_IMPL_H__
#define NEXUS_PLATFORM_CLIENT_IMPL_H__

#include "nexus_platform_common.h"

typedef struct NEXUS_P_Client *NEXUS_P_ClientHandle;

void NEXUS_P_Client_StopCallbacks(NEXUS_P_ClientHandle client, void *interfaceHandle);
void NEXUS_P_Client_StartCallbacks(NEXUS_P_ClientHandle client, void *interfaceHandle);

NEXUS_P_ClientHandle NEXUS_P_Client_Init( const NEXUS_ClientAuthenticationSettings *pSettings);
void NEXUS_P_Client_Disconnect(NEXUS_P_ClientHandle client);
void NEXUS_P_Client_Uninit( NEXUS_P_ClientHandle client );
NEXUS_P_ClientModuleHandle NEXUS_P_Client_InitModule( NEXUS_P_ClientHandle client, unsigned module_id, unsigned data_size );
void NEXUS_P_Client_UninitModule( NEXUS_P_ClientModuleHandle module );

struct NEXUS_Platform_P_State
{
    NEXUS_ModuleHandle module;
    int mem_fd;
    int uncached_mem_fd;
    int fake_mem_fd;
    struct {
        NEXUS_HeapHandle heap;
        void *addr;
        void *uncached_addr;
        unsigned long length;
        bool dynamic;
        NEXUS_AddrType memoryMapType;
    } mmaps[NEXUS_MAX_HEAPS];
    bool init;
} NEXUS_Platform_P_State;

#define NEXUS_MODULE_SELF NEXUS_Platform_P_State.module

#endif
