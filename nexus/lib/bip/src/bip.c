/******************************************************************************
 * (c) 2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
*****************************************************************************/
#include <pthread.h>

#include "bip_priv.h"

BDBG_MODULE( bip );
BIP_SETTINGS_ID(BIP_InitSettings);
BIP_CLASS_DEFINE(BIP_Socket);
BIP_CLASS_DEFINE(BIP_HttpServer);
BIP_CLASS_DEFINE(BIP_HttpServerSocket);


static pthread_mutex_t  g_initMutex = PTHREAD_MUTEX_INITIALIZER;
static int              g_refCount;

static BIP_ConsoleHandle  g_hConsole = NULL;




void BIP_Uninit_locked(void)
{
    B_Error     rc;

    /* Shutdown any BIP "services" first. */
    if (g_hConsole)
    {
        BIP_Console_Destroy(g_hConsole);
    }

    BIP_CLASS_UNINIT(BIP_HttpServerSocket);
    BIP_CLASS_UNINIT(BIP_HttpServer);
    BIP_CLASS_UNINIT(BIP_Socket);

    /* Now do the uninitialization. */
    BIP_IoCheckerFactory_Uninit();

    BIP_TimerFactory_Uninit();

    BIP_ArbFactory_Uninit();

    rc = B_Os_Uninit();
    BIP_ERR_TRACE(rc);
}


/**
 * Summary:
 * Uninitialize (shut down) the BIP infrastructure.  Should be the last BIP API called by an application.
 **/
void BIP_Uninit(void)
{
    BIP_Status   rc = BIP_SUCCESS;

    rc = pthread_mutex_lock(&g_initMutex);
    if ( rc )
    {
        rc = BIP_StatusFromErrno(rc);
        BIP_LOGERR(("pthread_mutex_lock() failed"), rc);
        return;
    }

    if (g_refCount == 0) {
        pthread_mutex_unlock(&g_initMutex);
        BIP_LOGERR(("BIP_Init() must be called before BIP_Uninit()!"), BIP_ERR_NOT_INITIALIZED);
        return;
    }

    /* Check for last close */
    if ( 0 == --g_refCount )
    {
        BIP_Uninit_locked();
    }

    pthread_mutex_unlock(&g_initMutex);
    return;
}


/**
 * Summary:
 * Initialize the BIP infrastructure.  Should be the first BIP API called by an application.
 **/
BIP_Status BIP_Init(
    BIP_InitSettings *pSettings
    )
{
    BIP_Status   rc = BIP_SUCCESS;
    BSTD_UNUSED(pSettings);

    BIP_SETTINGS_ASSERT(pSettings, BIP_InitSettings);

    rc = pthread_mutex_lock(&g_initMutex);
    if ( rc )
    {
        rc = BIP_StatusFromErrno(rc);
        BIP_LOGERR(("pthread_mutex_lock() failed"), rc);
        return(rc);
    }

    /* Check for first open */
    if ( 0 == g_refCount++ )
    {
        rc = B_Os_Init();
        BIP_CHECK_GOTO(rc==B_ERROR_SUCCESS, ("B_Os_Init() failed"), error, rc, rc);

        rc = BIP_ArbFactory_Init(NULL);
        BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_Arb_Init() failed"), error, rc, rc);

        rc = BIP_TimerFactory_Init(NULL);
        BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_TimerFactory_Init() failed"), error, rc, rc);

        rc = BIP_IoCheckerFactory_Init(NULL);
        BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_IoCheckerFactory_Init() failed"), error, rc, rc);

        rc = BIP_CLASS_INIT(BIP_Socket, NULL);
        BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_CLASS_INIT(BIP_Socket) failed"), error, rc, rc);

        rc = BIP_CLASS_INIT(BIP_HttpServer, NULL);
        BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_CLASS_INIT(BIP_HttpServer) failed"), error, rc, rc);

        rc = BIP_CLASS_INIT(BIP_HttpServerSocket, NULL);
        BIP_CHECK_GOTO(rc==BIP_SUCCESS, ("BIP_CLASS_INIT(BIP_HttpServerSocket) failed"), error, rc, rc);

        /* Now start any BIP "services". */
#if 0  /* Don't bother starting the Console service since it's not really useful yet. */
        g_hConsole = BIP_Console_Create(NULL);
        if (g_hConsole == NULL)
        {
            BDBG_ERR((BIP_MSG_PRE_FMT "BIP_Console_Create() failed! Continuing anyway..."BIP_MSG_PRE_ARG));
        }
#endif
    }

    pthread_mutex_unlock(&g_initMutex);
    return(BIP_SUCCESS);

    /* Something went wrong during Init() calls.  Call BIP_Uninit_locked() to undo
     * any partial inits.  */
error:
    BIP_Uninit_locked();
    pthread_mutex_unlock(&g_initMutex);

    return( rc );
}


BIP_Status BIP_Init_GetStatus( BIP_InitStatus *pStatus )
{
    pStatus->refCount = g_refCount ? true : false;
    return BIP_SUCCESS;
}
