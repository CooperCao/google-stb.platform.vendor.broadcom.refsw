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

#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#include "bip_priv.h"

#include <sys/types.h>
#include <sys/socket.h>

BDBG_MODULE( bip_console );
BDBG_OBJECT_ID( BIP_Console );
BDBG_OBJECT_ID( BIP_ConsoleApp );
BIP_SETTINGS_ID(BIP_ConsoleCreateSettings);

typedef enum BIP_ConsoleState
{
    BIP_ConsoleState_eUninitialized = 0,     /* Initial state: object is just created but not fully initialzed */
    BIP_ConsoleState_eListening,                  /* Idle state: Server object is just created, but it hasn't yet started listening for Requests. */
    BIP_ConsoleState_eStopping,
    BIP_ConsoleState_eStopped,
    BIP_ConsoleState_eMax
} BIP_ConsoleState;

typedef enum BIP_ConsoleShutdownState
{
    BIP_ConsoleShutdownState_eNormal,         /* Normal existing (not shutdown) state.          */
    BIP_ConsoleShutdownState_eStartShutdown,  /* Starting shutdown... No further APIs accepted. */
    BIP_ConsoleShutdownState_eMax             /* eMax: last enum.                               */
} BIP_ConsoleShutdownState;


typedef struct BIP_ConsoleApp  *BIP_ConsoleAppHandle;

typedef struct BIP_ConsoleApp
{
    BDBG_OBJECT( BIP_ConsoleApp )
    BLST_Q_ENTRY(BIP_ConsoleApp) consoleAppListNext;
    BIP_ConsoleHandle     hConsole;
    BIP_SocketHandle      hSocket;
    int                   socketFd;
    B_ThreadFunc          threadFunc;
    void                 *pThreadParam;
    B_ThreadHandle        hThread;
    bool                  exited;
} BIP_ConsoleApp;


typedef struct BIP_Console
{
    BDBG_OBJECT( BIP_Console )

    BIP_ConsoleState                state;
    BIP_ConsoleShutdownState        shutdownState;

    B_MutexHandle                   hObjectLock;        /* Mutex to synchronize a API invocation with callback invocation */

    BIP_ConsoleCreateSettings       createSettings;

    BIP_ListenerHandle              hListener;
    BIP_TimerHandle                 hShutdownTimer;
    BIP_TimerHandle                 hThreadExitTimer;

    struct
    {
        BIP_ArbHandle               hArb;
    } destroyApi;

    struct
    {
        BIP_ArbHandle               hArb;
    } createApi;

    int                             consoleAppsCount;
    BLST_Q_HEAD(consoleAppList, BIP_ConsoleApp) consoleAppListHead;


} BIP_Console;










void consoleAppThread( void* pParam)
{
    BIP_ConsoleAppHandle  hConsoleApp = (BIP_ConsoleAppHandle) pParam;
    int fd = hConsoleApp->socketFd;
    BSTD_UNUSED(pParam);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry:" BIP_MSG_PRE_ARG));

    for (;;) {

        char buffer[80];
        ssize_t count;
        BIP_StringHandle  bsh;
        BIP_Status rc;

        /* GARYWASHERE (other) */  BDBG_LOG(("%s:%d: Calling recv for %d bytes", __FUNCTION__, __LINE__, sizeof(buffer)));

        count = recv(fd, buffer, sizeof(buffer)-1, 0);
        if (count==0) {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hConsoleApp %p: Got end of file on socketFd=%d" BIP_MSG_PRE_ARG, hConsoleApp, hConsoleApp->socketFd));
            goto error;
        }
        else if (count<0) {
            BIP_CHECK_GOTO((count>0),("recv failed: count=%d, errno=%d",count,errno),error,errno,rc);
        }

        buffer[count] = '\0';

        bsh = BIP_String_CreateFromPrintf("You entered: %d bytes: \"%s\"", count, buffer);

        /* GARYWASHERE (other) */  BDBG_LOG(("%s:%d: You entered: %d bytes: \"%s\"", __FUNCTION__, __LINE__, count, buffer));

        count = send(fd, BIP_String_GetString(bsh), BIP_String_GetLength(bsh), MSG_NOSIGNAL);
        BIP_String_Destroy(bsh);

    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exiting." BIP_MSG_PRE_ARG));

error:
    return;
}


/* Prototype for the "generic" processState entry. */
void processConsoleState(BIP_ConsoleHandle hConsole, int value,BIP_Arb_ThreadOrigin threadOrigin);
static void processConsoleStateFromThreadExitTimer(void *pContext);


static void consoleWrapperThread(void *data)
{
    BIP_ConsoleAppHandle    hConsoleApp = (BIP_ConsoleAppHandle) data;
    BIP_Status              rc;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry:" BIP_MSG_PRE_ARG));

    hConsoleApp->threadFunc( hConsoleApp->pThreadParam);

    /* The App thread is done, close its socketFd. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsoleApp %p: Closing socketFd=%d" BIP_MSG_PRE_ARG, hConsoleApp, hConsoleApp->socketFd));
    rc = close(hConsoleApp->socketFd);
    BIP_CHECK_LOGERR((rc==0),("close() failed on sockeFd=%d", hConsoleApp->socketFd),BIP_StatusFromErrno(errno), rc);

   {
        BIP_ConsoleHandle       hConsole = hConsoleApp->hConsole;
        BIP_TimerCreateSettings timerCreateSettings;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsoleApp %p: Setting ConsoleApp's exited flag." BIP_MSG_PRE_ARG, hConsoleApp));
        hConsoleApp->exited = true;
        /* hConsoleApp is now invalid!!! */

        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Waking up state machine" BIP_MSG_PRE_ARG, hConsole ));
        timerCreateSettings.input.callback    = processConsoleStateFromThreadExitTimer;
        timerCreateSettings.input.pContext    = hConsole;
        timerCreateSettings.input.timeoutInMs = 0;
        hConsole->hShutdownTimer = BIP_Timer_Create(&timerCreateSettings);
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsoleApp %p: exiting." BIP_MSG_PRE_ARG, hConsoleApp));
    return;
}

static BIP_Status launchConsoleThread(BIP_ConsoleHandle hConsole, BIP_SocketHandle hSocket)
{
    BIP_ConsoleAppHandle    hConsoleApp;
    BIP_SocketStatus        socketStatus;
    BIP_Status              brc;

    /* Allocate a context structure for the ConsoleApp thread.  This structure
     * is free by the consoleWrapperThread after the real ConsoleApp thread
     * returns back to it. */
    hConsoleApp = B_Os_Calloc( 1, sizeof( BIP_ConsoleApp ));
    BIP_CHECK_GOTO(( hConsoleApp != NULL ), ( "Failed to allocate memory (%d bytes) for BIP_ConsoleApp Object", sizeof(BIP_ConsoleApp) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

    BDBG_OBJECT_SET( hConsoleApp, BIP_ConsoleApp );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsoleApp %p: Allocated " BIP_MSG_PRE_ARG, hConsoleApp ));

    /* Get the Linux socket file descriptor from the BIP_Socket. */
    brc = BIP_Socket_GetStatus(hSocket, &socketStatus);
    BIP_CHECK_GOTO((brc==BIP_SUCCESS), ("BIP_Socket_GetStatus() failed"), error, brc, brc );

    BDBG_LOG(( BIP_MSG_PRE_FMT "Socket fd is %d" BIP_MSG_PRE_ARG, socketStatus.socketFd ));

    /* Now make a duplicate of the socket's fd so that the console app can use it even if
     * it gets closed by BIP.  This fd will get closed by the consoleWrapperThread.*/
    hConsoleApp->hConsole = hConsole;
    hConsoleApp->hSocket  = hSocket;
    hConsoleApp->socketFd = dup(socketStatus.socketFd);
    BIP_CHECK_GOTO((hConsoleApp->socketFd!=-1),("dup system call failed"),error, BIP_StatusFromErrno(errno), brc);

    /* Get the thread function and param pointer. */

    hConsoleApp->threadFunc = hConsole->createSettings.consoleAppThreadFunction;
    hConsoleApp->pThreadParam = hConsole->createSettings.pConsoleAppThreadParam;


#if 1  /* ==================== GARYWASHERE - Start of Original Code ==================== */
    /* Use these for testing. */
    hConsoleApp->threadFunc = consoleAppThread;
    hConsoleApp->pThreadParam = hConsoleApp;
#endif /* ==================== GARYWASHERE - End of Modified Code   ==================== */

    BLST_Q_INSERT_TAIL(&hConsole->consoleAppListHead, hConsoleApp, consoleAppListNext);

    hConsoleApp->hThread = B_Thread_Create("BIP_Cons", consoleWrapperThread, (void *)hConsoleApp, NULL);
    if (NULL == hConsoleApp->hThread) {
        BDBG_ERR(( BIP_MSG_PRE_FMT "B_Thread_Create() failed" BIP_MSG_PRE_ARG));
        goto error;
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Created hThread=%p" BIP_MSG_PRE_ARG, hConsoleApp->hThread));
    return (BIP_SUCCESS);

error:
    return (BIP_ERR_B_OS_LIB);
}



static void connectedCallbackFromListener (
    void *context,
    int   param
    )
{
    BIP_ConsoleHandle hConsole = context;

    BSTD_UNUSED(param);
    BDBG_ASSERT(hConsole);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Enter: hConsole %p" BIP_MSG_PRE_ARG, hConsole ));
    processConsoleState( hConsole, 0, BIP_Arb_ThreadOrigin_eBipCallback);
    BDBG_MSG(( BIP_MSG_PRE_FMT "Exit: hConsole %p" BIP_MSG_PRE_ARG, hConsole ));
    return;
}


static void processConsoleStateFromShutdownTimer(void *pContext)
{
    BIP_ConsoleHandle    hConsole = (BIP_ConsoleHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Entry..." BIP_MSG_PRE_ARG, hConsole ));

    B_Mutex_Lock(hConsole->hObjectLock);
    if (hConsole->hShutdownTimer) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Got BIP_Timer callback, marking timer as self-destroyed" BIP_MSG_PRE_ARG, hConsole ));
        hConsole->hShutdownTimer = NULL;   /* Indicate timer not active. */
    }
    B_Mutex_Unlock(hConsole->hObjectLock);

    processConsoleState(hConsole, 0, BIP_Arb_ThreadOrigin_eTimer);
}


static void processConsoleStateFromThreadExitTimer(void *pContext)
{
    BIP_ConsoleHandle    hConsole = (BIP_ConsoleHandle) pContext;
    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Entry..." BIP_MSG_PRE_ARG, hConsole ));

    B_Mutex_Lock(hConsole->hObjectLock);
    if (hConsole->hThreadExitTimer) {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Got BIP_Timer callback, marking timer as self-destroyed" BIP_MSG_PRE_ARG, hConsole ));
        hConsole->hThreadExitTimer = NULL;   /* Indicate timer not active. */
    }
    B_Mutex_Unlock(hConsole->hObjectLock);

    processConsoleState(hConsole, 0, BIP_Arb_ThreadOrigin_eTimer);
}


static void processConsoleStateFromArb(void *hObject, int value, BIP_Arb_ThreadOrigin threadOrigin)
{
    processConsoleState( (BIP_ConsoleHandle) hObject, value, threadOrigin);
}


void processConsoleState(
    BIP_ConsoleHandle hConsole,               /* BIP_Console object handle */
    int value,
    BIP_Arb_ThreadOrigin threadOrigin
    )
{
    BIP_ArbHandle           hArb;
    BIP_Status              brc = BIP_ERR_INTERNAL;
    bool                    reRunProcessState;

    BSTD_UNUSED(value);

    BDBG_ASSERT(hConsole);
    BDBG_OBJECT_ASSERT( hConsole, BIP_Console);

    B_Mutex_Lock( hConsole->hObjectLock );

    /******************************************************************************
     * Check for any new Arbs that have arrived.  Any new Arbs need to be either
     * Accepted (if valid request) or Rejected (if not valid).
     ******************************************************************************/
    /* Check for a new Create Arb. */
    if (BIP_Arb_IsNew(hArb = hConsole->createApi.hArb))
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: BIP_Console_Create() in progress..." BIP_MSG_PRE_ARG, hConsole ));
        if (hConsole->shutdownState != BIP_ConsoleShutdownState_eNormal)
        {
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else if (hConsole->state == BIP_ConsoleState_eUninitialized)
        {
            BIP_Arb_AcceptRequest(hArb);
        }
        else
        {
            BIP_Arb_RejectRequest(hArb, BIP_ERR_INVALID_API_SEQUENCE);
        }
    }

    /* Check for a new Destroy Arb. */
    if (BIP_Arb_IsNew(hArb = hConsole->destroyApi.hArb))
    {
        if (hConsole->shutdownState != BIP_ConsoleShutdownState_eNormal)
        {
            BIP_Arb_RejectRequest(hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
        }
        else
        {
            hConsole->shutdownState = BIP_ConsoleShutdownState_eStartShutdown;
            BIP_Arb_AcceptRequest(hArb);
        }

        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: BIP_Console_Destroy() in progress..." BIP_MSG_PRE_ARG, hConsole ));
    }

    /******************************************************************************
     *  Go through the list of ConsoleApps to look for any that might have exited.
     *  If they exited, cleanup after them.  While we are at it, if the Console
     *  is shutting down, call "shutdown()" for each of the active ConsoleApps.
     *  That will close the socket, causing any future reads to get EOF and will
     *  hopefully cause the ConsoleApp to exit.
     ******************************************************************************/
    {
        BIP_ConsoleAppHandle    hConsoleApp;
        BIP_ConsoleAppHandle    hNextConsoleApp;
        int                     consoleAppsCount = 0;

        for ( hConsoleApp = BLST_Q_FIRST(&hConsole->consoleAppListHead);
              NULL != hConsoleApp;
              hConsoleApp = hNextConsoleApp )
        {
            hNextConsoleApp = BLST_Q_NEXT(hConsoleApp, consoleAppListNext);

            if (hConsoleApp->exited)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: ConsoleApp %p has exited. Cleaning up" BIP_MSG_PRE_ARG, hConsole, hConsoleApp));

                BLST_Q_REMOVE(&hConsole->consoleAppListHead, hConsoleApp, consoleAppListNext);

                BIP_Socket_Destroy(hConsoleApp->hSocket);

                BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Destroying B_Thread %p from ConsoleApp %p" BIP_MSG_PRE_ARG, hConsole, hConsoleApp->hThread, hConsoleApp));
                B_Thread_Destroy(hConsoleApp->hThread);

                /* And free the BIP_ConsoleApp context. */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hConsoleApp %p: Freeing object data" BIP_MSG_PRE_ARG, hConsoleApp));
                B_Os_Free(hConsoleApp);
            }
            else
            {
                consoleAppsCount++;

                if (hConsole->shutdownState != BIP_ConsoleShutdownState_eNormal)
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Calling shutdown() for ConsoleApp %p: fd=%d" BIP_MSG_PRE_ARG, hConsole, hConsoleApp, hConsoleApp->socketFd));
                    shutdown(hConsoleApp->socketFd, SHUT_RDWR);
                }
            }
        }
        hConsole->consoleAppsCount = consoleAppsCount;
    }


    /******************************************************************************
     * Main state processing loop
     ******************************************************************************/
    reRunProcessState = true;
    while (reRunProcessState)
    {
        reRunProcessState = false;
        BDBG_MSG(( BIP_MSG_PRE_FMT "Top of loop: " BIP_CONSOLE_PRINTF_FMT
                    BIP_MSG_PRE_ARG, BIP_CONSOLE_PRINTF_ARG(hConsole) ));

        /**************************************************************************
         * Main state:  BIP_ConsoleState_eUninitialized
         *
         * If we're uninitialized create and start a listener.
         **************************************************************************/
        if (hConsole->state == BIP_ConsoleState_eUninitialized)
        {
            BIP_ListenerCreateSettings      listenerCreateSettings;
            BIP_ListenerSettings            listenerSettings;
            BIP_ListenerStartSettings       listenerStartSettings;
            BIP_Status                      createStatus = BIP_SUCCESS;

            if (hConsole->shutdownState != BIP_ConsoleShutdownState_eNormal)
            {
                createStatus = BIP_ERR_OBJECT_BEING_DESTROYED;
                BIP_LOGERR(("BIP_Console initialization cancelled"), createStatus);
            }

            /* Now Create the Listener Object */
            if (createStatus == BIP_SUCCESS)
            {
                BIP_Listener_GetDefaultCreateSettings( &listenerCreateSettings );
                hConsole->hListener = BIP_Listener_Create ( &listenerCreateSettings );
                if (hConsole->hListener==NULL) {
                    createStatus = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
                    BIP_LOGERR(("BIP_Listener_Create failed"), createStatus);
                }
            }

            /* Update Listener's settings: port, IP address, and interface, etc. */
            if (createStatus == BIP_SUCCESS)
            {
                BIP_Listener_GetSettings( hConsole->hListener, &listenerSettings );
                listenerSettings.connectedCallback.callback = connectedCallbackFromListener;
                listenerSettings.connectedCallback.context = hConsole;
                createStatus = BIP_Listener_SetSettings( hConsole->hListener, &listenerSettings );
                BIP_CHECK_LOGERR((createStatus==BIP_SUCCESS),("BIP_Listener_SetSettings failed"), createStatus, createStatus);
            }

            if (createStatus == BIP_SUCCESS)
            {
                BIP_Listener_GetDefaultStartSettings(&listenerStartSettings);
                listenerStartSettings.pPort = hConsole->createSettings.pPort;
                listenerStartSettings.pInterfaceName = hConsole->createSettings.pInterfaceName;
                listenerStartSettings.ipAddressType = hConsole->createSettings.ipAddressType;

                /* Successfully updated BIP_Listener's Settings, now Start it! */
                createStatus = BIP_Listener_Start( hConsole->hListener, &listenerStartSettings );
                BIP_CHECK_LOGERR((createStatus==BIP_SUCCESS),("BIP_Listener_SetSettings failed"), createStatus, createStatus);
            }

            if (createStatus == BIP_SUCCESS)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Listening for telnet on port %s"
                           BIP_MSG_PRE_ARG, hConsole, hConsole->createSettings.pPort ));

                hConsole->state = BIP_ConsoleState_eListening;
            }

            /* Do any other initialization stuff... set completions status*/

            if ( BIP_Arb_IsBusy(hConsole->createApi.hArb))  /* It really should be busy! */
            {
                BIP_Arb_CompleteRequest(hConsole->createApi.hArb, createStatus);
            }
        }  /* End if (hConsole->state == BIP_ConsoleState_eUninitialized) */

        /**************************************************************************
         * Main state:  BIP_ConsoleState_eListening
         *
         * Nothing to do, just stay here until we are destroyed.
         **************************************************************************/
        if (hConsole->state == BIP_ConsoleState_eListening  &&
            hConsole->shutdownState != BIP_ConsoleShutdownState_eNormal)
        {
                hConsole->state = BIP_ConsoleState_eStopping;
        }
        if (hConsole->state == BIP_ConsoleState_eListening)
        {
            BIP_SocketHandle   hSocket = NULL;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Trying to Accept a connection." BIP_MSG_PRE_ARG, hConsole));

            hSocket = BIP_Listener_Accept( hConsole->hListener, 0 /* timeout */); /* non-blocking accept call */
            if (hSocket)
            {
                BIP_Status  rc;

                BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Got connection on hSocket %p" BIP_MSG_PRE_ARG, hConsole, hSocket));
                rc = launchConsoleThread(hConsole, hSocket);
                BIP_CHECK_LOGERR((rc==BIP_SUCCESS), ("Failed to launch console thread"), rc, rc);
            }
            else
            {
                /* No new connection available to accept at this time, we remain in the same state! */
                BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Accept came up empty."
                            BIP_MSG_PRE_ARG, hConsole ));
            }
        } /* End if (hConsole->state == BIP_ConsoleState_eListening) */

        /**************************************************************************
         * Main state:  BIP_ConsoleState_eStopping
         *
         * Time to shut down... Stop and destroy the Listener.
         **************************************************************************/
        if (hConsole->state == BIP_ConsoleState_eStopping)
        {

            if (hConsole->hListener) {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Stopping Listener: hListener %p"
                           BIP_MSG_PRE_ARG, hConsole, hConsole->hListener));
                BIP_Listener_Destroy(hConsole->hListener);
                hConsole->hListener = NULL;
            }

            if (BIP_Arb_IsBusy(hConsole->createApi.hArb))
            {
                BIP_Arb_CompleteRequest(hConsole->createApi.hArb, BIP_ERR_OBJECT_BEING_DESTROYED);
            }

            if (hConsole->consoleAppsCount > 0) {
                BIP_TimerCreateSettings timerCreateSettings;

                BDBG_WRN(( BIP_MSG_PRE_FMT "Waiting for %d ConsoleApps to exit" BIP_MSG_PRE_ARG, hConsole->consoleAppsCount));

                if (hConsole->hShutdownTimer == NULL) {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Starting timer for a second" BIP_MSG_PRE_ARG, hConsole ));
                    timerCreateSettings.input.callback    = processConsoleStateFromShutdownTimer;
                    timerCreateSettings.input.pContext    = hConsole;
                    timerCreateSettings.input.timeoutInMs = 1000;
                    hConsole->hShutdownTimer = BIP_Timer_Create(&timerCreateSettings);
                }
            }
            else
            {
                if (BIP_Arb_IsBusy(hConsole->destroyApi.hArb))
                {
                    BIP_Arb_CompleteRequest(hConsole->destroyApi.hArb, BIP_SUCCESS);
                }
                hConsole->state = BIP_ConsoleState_eStopped;
            }

        } /* End if (hConsole->state == BIP_ConsoleState_eStopping) */

        /**************************************************************************
         * Main state:  BIP_ConsoleState_eStopped
         *
         * Nothing to do, just stay here until we are destroyed.
         **************************************************************************/
        if (hConsole->state == BIP_ConsoleState_eStopped)
        {

        } /* End if (hConsole->state == BIP_ConsoleState_eStopped) */

    } /* while reRunProcessState */

    BDBG_MSG(( BIP_MSG_PRE_FMT "Exited loop: " BIP_CONSOLE_PRINTF_FMT
                BIP_MSG_PRE_ARG, BIP_CONSOLE_PRINTF_ARG(hConsole) ));
    /*
     * Done with state processing. We have to unlock state machine before issuing callbacks!
     * Issue any direct callbacks to caller (which happens when caller hasn't yet called either _Recv & _Send API.
     * And instead state machine was run via a _SetSettings or callbacks from BIP_Socket.
     */
    B_Mutex_Unlock( hConsole->hObjectLock );

    brc = BIP_Arb_DoDeferred( hConsole->destroyApi.hArb, threadOrigin );
    BDBG_ASSERT( brc == BIP_SUCCESS );

    return;
}


static void bipConsoleDestroy(
    BIP_ConsoleHandle hConsole
    )
{
    if (!hConsole) return;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroying hConsole %p" BIP_MSG_PRE_ARG, hConsole ));

    if (hConsole->destroyApi.hArb) BIP_Arb_Destroy(hConsole->destroyApi.hArb);
    if (hConsole->createApi.hArb) BIP_Arb_Destroy(hConsole->createApi.hArb);

    if (hConsole->hObjectLock) B_Mutex_Destroy( hConsole->hObjectLock );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Destroyed" BIP_MSG_PRE_ARG, hConsole ));
    BDBG_OBJECT_DESTROY( hConsole, BIP_Console );
    B_Os_Free( hConsole );

} /* bipConsoleDestroy */


BIP_ConsoleHandle BIP_Console_Create(
    const BIP_ConsoleCreateSettings *pCreateSettings
    )
{
    BIP_Status                      brc;
    BIP_ConsoleHandle               hConsole = NULL;

    BIP_SETTINGS_ASSERT(pCreateSettings, BIP_ConsoleCreateSettings);

    /* Create the BIP_Console object */
    {
        hConsole = B_Os_Calloc( 1, sizeof( BIP_Console ));
        BIP_CHECK_GOTO(( hConsole != NULL ), ( "Failed to allocate memory (%d bytes) for BIP_Console Object", sizeof(BIP_Console) ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

        BDBG_OBJECT_SET( hConsole, BIP_Console );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Allocated " BIP_MSG_PRE_ARG, hConsole ));
    }

    /* Create mutex to synchronize state machine from being run by multiple threads. */
    {
        hConsole->hObjectLock = B_Mutex_Create(NULL);
        BIP_CHECK_GOTO(( hConsole->hObjectLock ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
        BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Created hObjectLock:%p" BIP_MSG_PRE_ARG, hConsole, hConsole->hObjectLock ));
    }

    BLST_Q_INIT(&hConsole->consoleAppListHead);

    /* Set our createSettings... either from caller or use defaults. */
    {
        BIP_ConsoleCreateSettings       defaultSettings;

        if (NULL == pCreateSettings)
        {
            BIP_Console_GetDefaultCreateSettings( &defaultSettings );
            pCreateSettings = &defaultSettings;
        }
        hConsole->createSettings = *pCreateSettings;
    }

    /* Create API ARBs: one per API */
    {
        hConsole->destroyApi.hArb = BIP_Arb_Create(NULL, NULL);
        BIP_CHECK_GOTO(( hConsole->destroyApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );

        hConsole->createApi.hArb = BIP_Arb_Create(NULL, NULL);
        BIP_CHECK_GOTO(( hConsole->createApi.hArb ), ( "BIP_Arb_Create Failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, brc );
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hConsole %p: Dispatching Create Arb" BIP_MSG_PRE_ARG, hConsole));

    /* Now issue a Create Arb to the state machine so it can do its initialization. */
    {
        BIP_ArbHandle hArb;
        BIP_ArbSubmitSettings arbSettings;

        BDBG_OBJECT_ASSERT( hConsole, BIP_Console );

        BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Creating: " BIP_CONSOLE_PRINTF_FMT
                          BIP_MSG_PRE_ARG, BIP_CONSOLE_PRINTF_ARG(hConsole)));

        /* Serialize access to Settings state among another thread calling the same API. */
        hArb = hConsole->createApi.hArb;
        brc = BIP_Arb_Acquire(hArb);
        BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

        BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
        arbSettings.hObject = hConsole;
        arbSettings.arbProcessor = processConsoleStateFromArb;
        arbSettings.waitIfBusy = true;;

        /* Invoke state machine via the Arb Submit API */
        brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
        BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Console_Create" ), error, brc, brc );
    }

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_CONSOLE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_CONSOLE_PRINTF_ARG(hConsole)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_CONSOLE_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_CONSOLE_PRINTF_ARG(hConsole)));

    return ( hConsole );

error:
    bipConsoleDestroy( hConsole );
    return ( NULL );
} /* BIP_Console_Create */


/**
 * Summary:
 * Destroy HTTP Server
 *
 * Description:
 **/
void BIP_Console_Destroy(
    BIP_ConsoleHandle hConsole
    )
{
    BIP_Status brc;
    BIP_ArbHandle hArb;
    BIP_ArbSubmitSettings arbSettings;

    BDBG_OBJECT_ASSERT( hConsole, BIP_Console );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_CONSOLE_PRINTF_FMT
                      BIP_MSG_PRE_ARG, BIP_CONSOLE_PRINTF_ARG(hConsole)));

    /* Serialize access to Settings state among another thread calling the same API. */
    hArb = hConsole->destroyApi.hArb;
    brc = BIP_Arb_Acquire(hArb);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_Acquire() Failed" ), error, brc, brc );

    BIP_Arb_GetDefaultSubmitSettings( &arbSettings );
    arbSettings.hObject = hConsole;
    arbSettings.arbProcessor = processConsoleStateFromArb;
    arbSettings.waitIfBusy = true;;

    /* Invoke state machine via the Arb Submit API */
    brc = BIP_Arb_Submit(hArb, &arbSettings, NULL);
    BIP_CHECK_GOTO((brc == BIP_SUCCESS), ( "BIP_Arb_SubmitRequest() Failed for BIP_Console_Destroy" ), error, brc, brc );

error:
    /* Now free the HttpServer's resources. */
    bipConsoleDestroy( hConsole );

} /* BIP_Console_Destroy */
