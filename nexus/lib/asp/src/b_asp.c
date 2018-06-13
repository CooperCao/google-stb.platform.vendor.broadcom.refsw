/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include "b_asp.h"
#include "b_asp_class.h"
#include "b_asp_priv.h"

BDBG_MODULE(b_asp);
B_ASP_CLASS_DEFINE(B_Asp_Output);
B_ASP_CLASS_DEFINE(B_Asp_Input);

/* Global ASP Mgr Context. */
typedef struct B_Asp
{
    int initialized;        /* 0 => not initialized */
    B_AspInitSettings settings;
    char *pAspProxyServerIp;
} B_Asp;
static B_Asp g_aspMgr;      /* Initialized to zero at compile time. */

static pthread_mutex_t g_initMutex = PTHREAD_MUTEX_INITIALIZER;

void B_Asp_Uninit(void)
{
    pthread_mutex_lock(&g_initMutex);

    if (g_aspMgr.initialized)
    {
        B_ASP_CLASS_UNINIT(B_Asp_Output);
        B_ASP_CLASS_UNINIT(B_Asp_Input);
        g_aspMgr.initialized = false;

        if (g_aspMgr.pAspProxyServerIp)
        {
            free(g_aspMgr.pAspProxyServerIp);
            g_aspMgr.pAspProxyServerIp = NULL;
        }

        BKNI_Uninit();
    }

    pthread_mutex_unlock(&g_initMutex);
    BDBG_WRN(( B_ASP_MSG_PRE_FMT "ASP Lib is un-initialized!" B_ASP_MSG_PRE_ARG ));
    return;
}

void B_Asp_GetDefaultInitSettings(
    B_AspInitSettings                       *pSettings
    )
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_Error B_Asp_Init(
    const B_AspInitSettings                 *pSettings
    )
{

    NEXUS_Error errCode = NEXUS_SUCCESS;
    int rc;

    rc = pthread_mutex_lock(&g_initMutex);
    if ( rc )
    {
        return NEXUS_OUT_OF_SYSTEM_MEMORY;
    }

    /* Check for first open */
    if ( g_aspMgr.initialized )
    {
        BDBG_ERR(("Error, B_Asp_Init() has already been called."));
        errCode = NEXUS_NOT_SUPPORTED;
    }

    if (errCode == NEXUS_SUCCESS)
    {
        errCode = BKNI_Init();
    }

    if ( errCode == NEXUS_SUCCESS)
    {
        if (!pSettings)
        {
            B_Asp_GetDefaultInitSettings(&g_aspMgr.settings);
        }
        else
        {
            g_aspMgr.settings = *pSettings;
        }
    }

    if ( errCode == NEXUS_SUCCESS)
    {
        rc = B_ASP_CLASS_INIT(B_Asp_Input, NULL);
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "B_ASP_CLASS_INIT failed!" B_ASP_MSG_PRE_ARG));
        }
    }

    if ( errCode == NEXUS_SUCCESS)
    {
        rc = B_ASP_CLASS_INIT(B_Asp_Output, NULL);
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "B_ASP_CLASS_INIT failed!" B_ASP_MSG_PRE_ARG));
        }
    }

    if ( errCode == NEXUS_SUCCESS)
    {
        g_aspMgr.initialized = true;
    }

    pthread_mutex_unlock(&g_initMutex);
    BDBG_WRN(( B_ASP_MSG_PRE_FMT "ASP Lib is initialized!" B_ASP_MSG_PRE_ARG ));
    return errCode;
}
