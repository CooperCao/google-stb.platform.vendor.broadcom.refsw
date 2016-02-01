/***************************************************************************
 *     (c)2004-2011 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ************************************************************/
#include "nexus_base.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "nexus_platform_server.h"
#include "bstd.h"
#include "bkni.h"

#if NEXUS_SERVER_SUPPORT
#include "nexus_driver.h"
#include "server/nexus_ipc_api.h"
#include "server/nexus_server_prologue.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h> /* see SIGPIPE below */
#endif

BDBG_MODULE(nexus_platform_server);

#if NEXUS_SERVER_SUPPORT

#else /* NEXUS_SERVER_SUPPORT */

/* stub the public API */
NEXUS_Error NEXUS_Platform_P_InitServer(void)
{
    return 0;
}
void NEXUS_Platform_P_UninitServer(void)
{
}
void NEXUS_Platform_GetDefaultStartServerSettings( NEXUS_PlatformStartServerSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_Platform_StartServer(const NEXUS_PlatformStartServerSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
void NEXUS_Platform_GetDefaultClientSettings( NEXUS_ClientSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
}
void NEXUS_Platform_StopServer(void)
{
}
NEXUS_ClientHandle NEXUS_Platform_RegisterClient( const NEXUS_ClientSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}
void NEXUS_Platform_UnregisterClient( NEXUS_ClientHandle client )
{
    BSTD_UNUSED(client);
}
#endif /* NEXUS_SERVER_SUPPORT */

/**
The following client functions only have meaning in the client, not in the server.
They exist here because the server is the superset of the client.
**/
NEXUS_Error NEXUS_Platform_AuthenticatedJoin(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    BDBG_ERR(("You are calling client api's with a server-only Nexus library. Have you set NEXUS_MODE=client?"));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_Platform_GetDefaultClientAuthenticationSettings( NEXUS_ClientAuthenticationSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
}

void NEXUS_Platform_GetClientConfiguration( NEXUS_ClientConfiguration *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

}

