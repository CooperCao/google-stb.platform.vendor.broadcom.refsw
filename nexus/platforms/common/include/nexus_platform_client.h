/***************************************************************************
*  Copyright (C) 2004-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_PLATFORM_CLIENT_H__
#define NEXUS_PLATFORM_CLIENT_H__

#include "nexus_platform_common.h"
#include "nexus_core_file_init.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
Summary:
Get default settings for NEXUS_Platform_AuthenticatedJoin
**/
void NEXUS_Platform_GetDefaultClientAuthenticationSettings( /* attr{local=true}  */
    NEXUS_ClientAuthenticationSettings *pSettings /* [out] */
    );

/**
Summary:
Join a client process to the Nexus server

Description:
A nexus client is a separate process or application which connects to the single Nexus server using
some form of inter-process communication (IPC).

The Nexus server must already be started in another process with NEXUS_Platform_Init. There can only be one
server in a system. There can be zero, one or more clients.

If you have already called NEXUS_Platform_Init from your process, then you cannot also join as a client.
If you have already called NEXUS_Platform_AuthenticatedJoin, then you cannot join again until calling NEXUS_Platform_Uninit.
Only one join at a time is allowed per process.

If pSettings is NULL, the client is unauthenticated. The server must explicitly allow unauthenticated clients it starts.

The NEXUS_Platform_Join macro is provided for backward compatibility.

When your client process wants to disconnect from Nexus it should call NEXUS_Platform_Uninit. Any connection
with the server will be terminated and all client nexus state will be destroyed. You can then join again
if you wish.

Clients should call NEXUS_Platform_GetClientConfiguration after joining to learn what permissions
and heaps they have access to.

See nexus/docs/Nexus_MultiProcess.pdf for more documentation.
**/
NEXUS_Error NEXUS_Platform_AuthenticatedJoin( /* attr{local=true}  */
    const NEXUS_ClientAuthenticationSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Join a trusted client process (with default settings) to the Nexus server
**/
#define NEXUS_Platform_Join() NEXUS_Platform_AuthenticatedJoin(NULL)

/***************************************************************************
Summary:
For clients, disconnect from nexus, paired with NEXUS_Platform_AuthenticatedJoin.
For the server, uninitialized nexus, pair with NEXUS_Platform_Init.

Description:
The user is responsible for closing handles before calling NEXUS_Platform_Uninit.
Some handles can be automatically closed, but some cannot. It is best to explicitly
close all handles that you have opened.

After calling NEXUS_Platform_Uninit, the client may call NEXUS_Platform_AuthenticatedJoin again.
After calling NEXUS_Platform_Uninit, the server may call NEXUS_Platform_Init again.
***************************************************************************/
void NEXUS_Platform_Uninit( /* attr{thunk=false} */
    void
);
/**
Summary:
Get client configuration settings provided by the server
**/
void NEXUS_Platform_GetClientConfiguration(
    NEXUS_ClientConfiguration *pSettings /* [out] */
    );

/* Returns everything except NEXUS_MemoryStatus.addr, which can be locally obtained with NEXUS_OffsetToCacheAddr. See NEXUS_Heap_GetStatus. */
NEXUS_Error NEXUS_Platform_GetHeapStatus_driver( NEXUS_HeapHandle heap, NEXUS_MemoryStatus *pStatus);

/* Returns FileModuleSettings passed into NEXUS_Platform_Init, used by client in NEXUS_Platform_Join */
void NEXUS_Platform_GetFileModuleSettings_driver( NEXUS_FileModuleSettings *pFileModuleSettings );

#ifdef __cplusplus
}
#endif

#endif
