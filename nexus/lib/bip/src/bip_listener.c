/******************************************************************************
 * (c) 2016 Broadcom Corporation
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/poll.h>

#include <stdlib.h>
#include "bip_priv.h"
#include "bip_listener.h"
#include "bip_socket.h"

BDBG_MODULE( bip_listener );
BDBG_OBJECT_ID( BIP_Listener );
BDBG_FILE_MODULE( bip_listener_list );
#define PRINTMSG_LIST( bdbg_args ) BDBG_MODULE_MSG( bip_listener_list, bdbg_args );

/*  # of incoming connection requests to queue up in the kernel before apps gets to accept them */
#define LISTENER_BACKLOG_LENGTH 16

typedef enum
{
    BIP_ListenerState_eIdle,           /* state after create when there are no pending messages on this socket, Listener Stopped */
    BIP_ListenerState_eListening,      /* socket is in listening state, Listener Started*/
    BIP_ListenerState_eAccepting,      /* In proces of accepting a connection */
    BIP_ListenerState_eAcceptDone,    /*  Accept has completed typically from a CheckNow */
    BIP_ListenerState_eStopping,      /*  in the middle of an of calling a Listener stop*/
    BIP_ListenerState_eStopDone,      /*  Todo: currentlt not used */
    BIP_ListenerState_eError,         /* error has occured on the socket: peer closed or network error */
    /* TODO: need to define different socket states: listening, accepted, closing, etc. */
    BIP_ListenerState_eMax
} BIP_ListenerState;

typedef enum
{
    BIP_ListenerProcessStateEvent_eStart,
    BIP_ListenerProcessStateEvent_eDataReady,
    BIP_ListenerProcessStateEvent_eAccept,
    BIP_ListenerProcessStateEvent_eAcceptTimeout,
    BIP_ListenerProcessStateEvent_eStop
} BIP_ListenerProcessStateEvent;

typedef enum
{
    BIP_ListenerIoCheckerState_eNotInUse, /* Listener has disabled the Ioelement for the Listeners's socket Fd*/
    BIP_ListenerIoCheckerState_eIdle,     /* Listener has disabled the Ioelement for the Listeners's socket Fd*/
    BIP_ListenerIoCheckerState_eChecking  /* Listener has enabled the Ioelement for the Listeners's socket Fd*/
} BIP_ListenerIoCheckerState;

typedef enum
{
    BIP_ListenerCallbackState_eNotInUse,  /* Conncected Callback has not been set */
    BIP_ListenerCallbackState_eEnable,    /* Conncected Callback has been disabled by customer */
    BIP_ListenerCallbackState_eDisable    /* Conncected Callback has been enabled by customer */
} BIP_ListenerCallbackState;

struct BIP_Listener
{
    BDBG_OBJECT( BIP_Listener )
    BIP_ListenerCreateSettings createSettings;  /* dummy for now */
    BIP_ListenerStartSettings startSettings;
    BIP_ListenerSettings settings;

    B_MutexHandle listenerLock; /* mutex for object lock */
    B_MutexHandle apiLock;      /* mutex for other functions other than accept */
    B_MutexHandle acceptLock;   /* mutex specifc for the Accept function */

    BIP_ListenerState          state;
    BIP_ListenerIoCheckerState ioCheckerState;
    BIP_ListenerCallbackState  callbackState;

    int              socketFd;        /* Listeners's  socket Fd */
    BIP_SocketHandle hAcceptedSocket; /* Accepted BIPSocketHandle, Not listeners socket handle */

    BIP_IoCheckerHandle hIoElement;
    struct sockaddr_in  remoteIpAddress; /* TODO: convert this to ipv6 compliant way! */
    struct sockaddr_in  localIpAddress;  /* TODO: convert this to ipv6 compliant way! */
    /* TODO: Need to add localIpAddress & remoteIpAddress fields here, these should be ipv6 compliant, is it easier to keep them as string to be v4 or v6 agnostic ? */

    char *pInterfaceName;           /* Interface Name over which this socket connection is established */

    BLST_Q_ENTRY( BIP_Listener ) listenerListNext; /*name of elements link field*/

    bool          waitingOnDataReadyEvent; /* Flag indicating that there is a waitforEvent; Only then will callback setEvent, Only modifiable from Accept*/
    B_EventHandle dataReadyEvent;          /* this is the event, blocking and timeout versions of accept wait on */
    bool          waitingOnStopEvent;
    B_EventHandle stopEvent;  /* this is the stopEvent that will serialize the exit out of Accept before the finish of Listener_Stop */
 #if 0
    /* listener socket specific data */
    BLST_Q_HEAD( socketListHead, BIP_Socket ) socketListHead;
    B_SchedulerHandle socketScheduler;
    B_ThreadHandle    schedulerThread;
    B_ThreadHandle    monitorSocketEvents;
    bool              loopExit;
    bool              threadDone;
    bool              eventsMonitored;
    int               pollIndex;
    /** alloca based on BIP_MAX_NUM_FD only for listener , later
     *  if bigger size is required then allocate a bigger memory */
    int            numPfdAllocated;
    struct pollfd *pfd;
    int            numFdPolled;
#endif /* if 0 */
} BIP_Listener;

#define BIP_LISTENER_PRINTF_FMT  \
    "[hListener=%p Port=%s Iface=%s Type=%s qDepth=%d fd=%d]"

#define BIP_LISTENER_PRINTF_ARG(pObj)                                                                 \
    (pObj),                                                                                           \
    (pObj)->startSettings.pPort           ? (pObj)->startSettings.pPort :          "NULL",            \
    (pObj)->startSettings.pInterfaceName  ? (pObj)->startSettings.pInterfaceName : "NULL",            \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV4           ? "IpV4"          :   \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV6           ? "IpV6"          :   \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV6_and_IpV4  ? "IpV6_and_IpV4" :   \
    (pObj)->startSettings.ipAddressType==BIP_NetworkAddressType_eIpV6_over_IpV4 ? "IpV6_over_IpV4":   \
                                                                                  "<undefined>",      \
    (pObj)->startSettings.queueDepth,                                                                 \
    (pObj)->socketFd


/* maintain list of all Listeners */
static BLST_Q_HEAD( g_ListenerListHead, BIP_Listener ) g_ListenerListHead = BLST_Q_INITIALIZER( BIP_Listener );

/* Internal Function Headers */
static BIP_Status ProcessState(
    BIP_ListenerHandle            hListener,
    BIP_ListenerProcessStateEvent event
    );

static BIP_Status mapInterfaceNametoIpV4Address(
    BIP_ListenerHandle hListener,
    const char *pInterfaceName,
    struct sockaddr_in *pInAddr /* out */
    )
{
    int osErrno = 0;
    int osRc = 0;
    int interfaceNameLength;
    struct ifreq ifreq;
    int socketFd = -1;
    struct sockaddr_in *pSockAddrIn;
    BIP_Status bipStatus = BIP_ERR_INVALID_PARAMETER;

    BDBG_ASSERT(pInterfaceName);

    /*
     * This code assumes that we are getting IPv4 address for a given interface.
     * I think we should take an option from App to state whether we should use IPv6 address.
     * This way apps using IPv6 can set it and thus allow us to get the IPv6 address.
     */
    interfaceNameLength = strlen(pInterfaceName);
    if (interfaceNameLength > IFNAMSIZ-1)
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "hListener %p: interface name: %s, is too long, MAX allowed IFNAMSIZ-1: %d"
                    BIP_MSG_PRE_ARG, hListener, pInterfaceName, interfaceNameLength, IFNAMSIZ-1));
        return BIP_ERR_INVALID_PARAMETER;
    }

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) osErrno = errno;
    BIP_CHECK_GOTO( ( socketFd >= 0), ("hListener %p: System call: socket returned %d, errno=%d", hListener, socketFd, osErrno ), error, BIP_StatusFromErrno(osErrno), bipStatus );

    B_Os_Memset(&ifreq, 0, sizeof(struct ifreq));
    strncpy( ifreq.ifr_name, pInterfaceName, IFNAMSIZ-1);
    ifreq.ifr_addr.sa_family = AF_INET;
    osRc = ioctl(socketFd, SIOCGIFADDR, (char *)&ifreq);
    if (osRc != 0) osErrno = errno;
    BIP_CHECK_GOTO( ( osRc == 0), ("hListener %p: System call: ioctl SIOCGIFADDR returned %d, errno=%d", hListener, osRc, osErrno ), error, BIP_StatusFromErrno(osErrno), bipStatus );

    pSockAddrIn = (struct sockaddr_in *)&ifreq.ifr_addr;
    *pInAddr = *pSockAddrIn;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hListener %p: interface name: %s, ip Address: 0x%x" BIP_MSG_PRE_ARG, hListener, pInterfaceName, pInAddr->sin_addr.s_addr ));
    bipStatus = BIP_SUCCESS;

error:
    if (socketFd != -1)
    {
        shutdown(socketFd, SHUT_RDWR);
        close(socketFd);
    }
    return (bipStatus);

} /* mapInterfaceNametoIpV4Address */

/*
static void IoChecker_ReadyCallBack(
    void *context,
    int   param,
    BIP_IoCheckerEvent eventMask
    );
*/
static BIP_Status printBIPListenerList(
    void
    )
{
    BIP_ListenerHandle hElem;
    int                i = 0;

    BDBG_ENTER( printBIPListenerList );
    PRINTMSG_LIST(( "--Printing List of Bip_Listeners --" ));

    /*Remove the hSocket from BIP socket list */
    for (hElem = BLST_Q_FIRST( &g_ListenerListHead );
         hElem != NULL;
         hElem = BLST_Q_NEXT( hElem, listenerListNext ))
    {
        PRINTMSG_LIST(( "ListIndex %d \n ", i++ ));
        PRINTMSG_LIST(( "Listener state %d \n ", hElem->state ));
        PRINTMSG_LIST(( "Listener's socketFd %d \n ", hElem->socketFd ));
        PRINTMSG_LIST(( "Listener's pPort %d \n ", hElem->startSettings.pPort ));
    }
    PRINTMSG_LIST(( "----------End of List----------\n" ));

    BDBG_LEAVE( printBIPListenerList );

    return( BIP_SUCCESS );
} /* printBIPListenerList */

/* Function prints the current state, event, ioCheckerState, CallbackState*/
static BIP_Status printListenerStates(
    char                         *str,
    BIP_ListenerHandle            hListener,
    BIP_ListenerProcessStateEvent event
    )
{
    char* stateStr;
    char* eventStr;
    char* ioCheckerStateStr;
    char* callbackStateStr;

   stateStr =
    (  hListener->state== BIP_ListenerState_eIdle ) ?  "Idle" :
    (  hListener->state == BIP_ListenerState_eListening ) ?  "Listening" :
    (  hListener->state == BIP_ListenerState_eAccepting ) ? "Accepting" :
    (  hListener->state == BIP_ListenerState_eAcceptDone ) ?  "AcceptDone" :
    (  hListener->state == BIP_ListenerState_eStopping ) ? "Stopping" :
    (  hListener->state == BIP_ListenerState_eStopDone ) ?  "Stopdone" :
    (  hListener->state == BIP_ListenerState_eError ) ? "Error" :
                                                        "UNKNOWN";
    eventStr =
    ( event == BIP_ListenerProcessStateEvent_eStart ) ? "Start" :
    ( event == BIP_ListenerProcessStateEvent_eDataReady ) ?"DataReady" :
    ( event == BIP_ListenerProcessStateEvent_eAccept ) ? "Accept" :
    ( event == BIP_ListenerProcessStateEvent_eAcceptTimeout ) ?  "AcceptTimeout" :
    ( event == BIP_ListenerProcessStateEvent_eStop ) ? "Stop" :
                                                    "UNKNOWN";

    ioCheckerStateStr =
    ( hListener->ioCheckerState == BIP_ListenerIoCheckerState_eNotInUse ) ? "NotInUse" :
    ( hListener->ioCheckerState == BIP_ListenerIoCheckerState_eIdle ) ?  "Idle" :
    ( hListener->ioCheckerState == BIP_ListenerIoCheckerState_eChecking ) ?  "Checking" :
     "UNKNOWN";

    callbackStateStr =
    ( hListener->callbackState== BIP_ListenerCallbackState_eNotInUse ) ?  "NotInUse" :
    ( hListener->callbackState == BIP_ListenerCallbackState_eEnable ) ?  "Enable" :
    ( hListener->callbackState == BIP_ListenerCallbackState_eDisable ) ?  "Disable" :
    "UNKNOWN";

    BDBG_MSG(( "%s: Listener(fd=%d) State: %s, Event: %s, ioCheckerState: %s, callbackState: %s. ",
               str, hListener->socketFd, stateStr, eventStr, ioCheckerStateStr, callbackStateStr ));

    return( BIP_SUCCESS );
} /* printListenerStates */

/* Destructor function: This will go through the list of Listeners and clean up any remaining  */
BIP_Status BIP_Listener_Uninit(
    void
    )
{
    BIP_ListenerHandle hElem, hIter;
    /*Remove the hSocket from BIP socket list */

   if (!BLST_Q_EMPTY( &g_ListenerListHead ))
   {
       hIter =BLST_Q_FIRST( &g_ListenerListHead );
       while (hIter) {
           hElem = hIter;
           hIter = BLST_Q_NEXT( hIter, listenerListNext );
           BDBG_WRN(("leftover listener handle %p deleteing ",hElem));
           BLST_Q_REMOVE( &g_ListenerListHead, hElem, listenerListNext );
           BIP_Listener_Destroy( hElem );
       }
   }

    return( BIP_SUCCESS );
}

/*  Does a BIP_IoChecker_CheckNow and will accept(). Storing the BIP socket handle  */
static BIP_Status CheckNowAndAccept(
    BIP_ListenerHandle hListener
    )
{
    /* Check immediately if there is avaiable FD befoere you wait for an event*/
    BIP_Status rc;
    int       acceptedSocketFd;

    rc = BIP_IoChecker_CheckNow( hListener->hIoElement, BIP_IoCheckerEvent_ePollIn | BIP_IoCheckerEvent_ePollPri );
    if (rc)
    {
        BDBG_MSG(( "%s: accept() socket fd %d", __FUNCTION__, hListener->socketFd ));
        /* accept connection; don't care about information form remoteAddr*/
        acceptedSocketFd = accept( hListener->socketFd, NULL, NULL );
        BIP_CHECK_GOTO(( acceptedSocketFd >= 0 ), ( "listener accept failed: errno %d", errno ), error, BIP_ERR_INTERNAL, rc );
        hListener->hAcceptedSocket = BIP_Socket_CreateFromFd( acceptedSocketFd );
        rc = BIP_SUCCESS;
    }
    else
    {rc = BIP_ERR_NOT_AVAILABLE; }

    return( rc );

error:
    return( rc );
} /* CheckNowAndAccept */

/*
The reset state is:
 Enable IoChecker for   BIP_IoCheckerEvent_ePollIn  and  BIP_IoCheckerEvent_ePollPri
 Set Io Checker to  Checking
 Set Connection Callback to Enable
 Set Listener state to Listeneing
*/
static void ResetStates(
    BIP_ListenerHandle hListener
    )
{
    /* This makes me nervous to see that you are blindly setting all states to specific values.
     * I would expect the state transitions to be handled indpendently for each state, */



    BIP_IoChecker_Enable( hListener->hIoElement, BIP_IoCheckerEvent_ePollIn | BIP_IoCheckerEvent_ePollPri );
    hListener->callbackState  = BIP_ListenerCallbackState_eEnable;
    hListener->ioCheckerState = BIP_ListenerIoCheckerState_eChecking;
    hListener->state          = BIP_ListenerState_eListening;
    /* In case it has finished  and there its there was a set event without a waitforEvent.  See cases where DataReady comes in right after
     * the waitforEvent in Listener Accept.
     */
    B_Event_Reset(hListener->dataReadyEvent);
}

/*
This is the callback called by IoElement when there is an element ready. This will look for
BIP_IoCheckerEvent_ePollIn, BIP_IoCheckerEvent_ePollPri, BIP_IoCheckerEvent_ePollRdHup

*/
static void IoChecker_ReadyCallBack(
    void *context,
    int   param,
    BIP_IoCheckerEvent eventMask
    )
{
    BIP_ListenerHandle hListener    = context;
    unsigned           read_bitmask = BIP_IoCheckerEvent_ePollIn | BIP_IoCheckerEvent_ePollPri;
    BSTD_UNUSED(param);
    BDBG_MSG(( "%s: In Listener Callback: Listener state %d param %d ", __FUNCTION__, hListener->state, param ));
    if (( eventMask & BIP_IoCheckerEvent_ePollRdHup ) == BIP_IoCheckerEvent_ePollRdHup)
    {
        BDBG_ERR(( "Stream socket peer closed connection, or shut down writing ,half of connection.  " ));
        /* Todo: What action to take */
    }
    else if (eventMask & read_bitmask)
    {
        /*
               * if (listening state) => Disable io checker, call ConnectedCB (if exist and enabled)
               * or if( accepting state ) => CheckNow, set Event data ready event if accept waiting.
               */
        B_Mutex_Lock( hListener->listenerLock );
        ProcessState( hListener, BIP_ListenerProcessStateEvent_eDataReady );
        B_Mutex_Unlock( hListener->listenerLock );
    }
    else
    {
        BDBG_WRN(( "Ignore this poll event. %x", eventMask ));
    }
} /* IoChecker_ReadyCallBack */

/*
 2 layers of Switch statements, First check the State you are in. Second check the event/ function you are in and set the appropriate state.

*/
/*
  Pseudo Code another way of doing it
  Main differences: SS only calls process state once per function.  He also moved a lot of the logic(timeout into the Process State). Also some processing done outside
  of Process state.  SS changes state before going into  process State.  I change the state inside Process State.
  Do I need a new state?

   ===========================================================================
    =  The objectMutex must be locked by the caller.
    =  Can be invoked either on behalf of user calls (_Accept or Stop, start and stop ) or
    =  callbacks from IO Checker  (dataRead or error).
    ===========================================================================

    ===========================================================================
    =  Start by handling the startState  to see if we need to start Listening.
    ===========================================================================



    ===========================================================================
    =  Now try to process Aceepting.  Callback or caller has called Accept
    ===========================================================================


===========================================================================
=  Now try to process what happens when there is Listening.
===========================================================================

===========================================================================
=  Now try to process a read that's already in progress... and
=  handle both receiving a complete message & requested payload bytes case!
===========================================================================

*/
static BIP_Status ProcessState(
    BIP_ListenerHandle            hListener,
    BIP_ListenerProcessStateEvent event
    )
{
    BIP_Status rc = BIP_SUCCESS;
    printListenerStates( "Begin of ProcessState", hListener, event );
/*    B_Mutex_Lock( hListener->listenerLock ); */
    BDBG_MSG(( " not stuck on Process State listenerLock" ));
    switch (hListener->state) /*Check the Protocol and do accordingly... */
    {
        case BIP_ListenerState_eIdle:
        {
            switch (event)
            {
                case BIP_ListenerProcessStateEvent_eDataReady:
                case BIP_ListenerProcessStateEvent_eAccept:
                case BIP_ListenerProcessStateEvent_eAcceptTimeout:
                {
                    BDBG_ERR(( "%s:Listener State = Idle, Event=DataReady,Accept all not allowed  ", __FUNCTION__ ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
                case BIP_ListenerProcessStateEvent_eStop:
                {
                    BDBG_MSG(("%s: Listener State = Idle, Event = Stop. Already Idle, do nothing for Stop Event",__FUNCTION__));
                    break;
                }
                case BIP_ListenerProcessStateEvent_eStart:
                {
                    int                        lrc;
                    struct sockaddr_in         localAddr;
                    int                        reuse_flag = 1;
                    BIP_IoCheckerCreateSetting ioCreateSettings;
                    char                       ipAddress[INET6_ADDRSTRLEN];
                    const char                *pIpAddress = NULL;

                    /*socket () */
                    hListener->socketFd = socket( AF_INET, SOCK_STREAM, 0 );
                    BIP_CHECK_GOTO(( hListener->socketFd >= 0 ), ( "listener creation Failed: errno %d", errno ), error, BIP_ERR_INTERNAL, rc );

                    lrc = setsockopt( hListener->socketFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse_flag, sizeof( reuse_flag ));
                    BIP_CHECK_GOTO(( lrc >= 0 ), ( "setsockopt failed to set SO_REUSEADDR: errno %d", errno ), error, BIP_ERR_INTERNAL, rc );

                    if ( hListener->startSettings.pInterfaceName && strlen(hListener->startSettings.pInterfaceName) )
                    {
                        localAddr.sin_family = AF_INET;
                        if (hListener->startSettings.ipAddressType == BIP_NetworkAddressType_eIpV4)
                        {
                            BDBG_ERR(("hListener->startSettings.pInterfaceName -------------> %s", hListener->startSettings.pInterfaceName));
                            rc = mapInterfaceNametoIpV4Address( hListener, hListener->startSettings.pInterfaceName, &localAddr);
                            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "hListener %p: mapInterfaceNametoIpV4Address() Failed", hListener ), error, BIP_ERR_INTERFACE_NAME_TO_IP_ADDR, rc );
                            pIpAddress = inet_ntop(AF_INET, &localAddr.sin_addr, ipAddress, INET6_ADDRSTRLEN);
                        }
                        /* TODO: Add code for IPv6 & other enums! */
                    }
                    else
                    {
                        localAddr.sin_family = AF_INET;
                        localAddr.sin_addr.s_addr = htonl( INADDR_ANY );
                    }
                    localAddr.sin_port   = htons( atoi( hListener->startSettings.pPort ));

                    lrc = bind(  hListener->socketFd, (struct sockaddr *) &localAddr, sizeof( localAddr ));
                    BIP_CHECK_GOTO(( lrc >= 0 ), ( "listener bind failed: errno %d", errno ), error, BIP_ERR_INTERNAL, rc );

                    lrc = listen(  hListener->socketFd, hListener->startSettings.queueDepth );
                    BIP_CHECK_GOTO(( lrc >= 0 ), ( "socket listen failed: errno %d", errno ), error, BIP_ERR_INTERNAL, rc );

                    BDBG_MSG(( "%s: done, ip:port:socketFd %s:%s:%d", __FUNCTION__, pIpAddress,  hListener->startSettings.pPort,  hListener->socketFd ));

                    /* Add to internal static list of Listeners */
                    BLST_Q_INSERT_TAIL( &g_ListenerListHead, hListener, listenerListNext );

                    /* Add listener to IOChecker Todo*/

                    BIP_IoChecker_GetDefaultCreateSettings( &ioCreateSettings );
                    ioCreateSettings.fd =  hListener->socketFd;
                    ioCreateSettings.settings.callBackFunction = IoChecker_ReadyCallBack;
                    ioCreateSettings.settings.callBackContext  =  hListener;
                    hListener->hIoElement = BIP_IoChecker_Create( &ioCreateSettings );

                    ResetStates( hListener );
                    break;

error:
                    break;
                }
                default:
                {
                    BDBG_ERR(( "Invalid Event " ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
            } /* switch */
            break;
        }
        case BIP_ListenerState_eListening:
        {
            /* Depending on event  */
            switch (event)
            {
                case BIP_ListenerProcessStateEvent_eStart:
                {
                    BDBG_ERR(( "%s: State Listening: You are already Started.  " ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
                case BIP_ListenerProcessStateEvent_eDataReady:
                {
                    /* IoChecker automatically disables after firing the callback, so mark it disabled. */
                    hListener->ioCheckerState = BIP_ListenerIoCheckerState_eIdle;

                    /*Send conncected callback to upper level,  Need to handle when connectedCallback is Null or App directly calls Bip_Listener_Accept */
                    if (hListener->settings.connectedCallback.callback && ( hListener->callbackState == BIP_ListenerCallbackState_eEnable ))
                    {
                        BDBG_MSG(( "%s:Listener Data Ready CB calling connected callback  %p", __FUNCTION__, hListener->hIoElement ));

                        hListener->callbackState = BIP_ListenerCallbackState_eDisable;

                        B_Mutex_Unlock( hListener->listenerLock ); /* must releast lock cause callback can call into Process state in this CB */
                        ( *hListener->settings.connectedCallback.callback )( hListener->settings.connectedCallback.context, hListener->settings.connectedCallback.param );

                        printListenerStates( "End of ProcessState", hListener, event );
                        B_Mutex_Lock( hListener->listenerLock ); /* must maintin the state of lock coming out of ProcessState */
                        return( BIP_SUCCESS ); /* return here because you already unlocked */
                    }

                    break;
                }
                case BIP_ListenerProcessStateEvent_eAccept:
                {
                    hListener->state = BIP_ListenerState_eAccepting;

                    /* This code is common to  both non blocking and blocking */
                    /* Check immediately if there is avaiable FD befoere you wait for an event*/
                    if (CheckNowAndAccept( hListener ) == BIP_SUCCESS)
                    {
                        hListener->state = BIP_ListenerState_eAcceptDone;
                    }
                    else
                    {
                        BDBG_MSG(("%s: No New Connection available at this time.If no timeout, go to listening, else wait till timeout", __FUNCTION__));
                    }

                    break;
                }
                case BIP_ListenerProcessStateEvent_eStop:
                {
                    hListener->state = BIP_ListenerState_eStopping;
                    /* disable ListenerFd  from IOChecker, this may not be needed*/
                    BIP_IoChecker_Disable( hListener->hIoElement, BIP_IoCheckerEvent_ePollIn |BIP_IoCheckerEvent_ePollPri );
                    hListener->ioCheckerState = BIP_ListenerIoCheckerState_eIdle;

                    break;
                }
                default:
                {
                    BDBG_ERR(( "Invalid Event " ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
            } /* switch */

            break;
        }

        case BIP_ListenerState_eAccepting:
        {
            /* Depending on event  */
            switch (event)
            {
                case BIP_ListenerProcessStateEvent_eStart:
                {
                    BDBG_MSG(( "%s: State: Accepting and Event: Start, do nothing " ));
                    break;
                }
                /* Accept is waiting, data ready callback has come to set event */
                case BIP_ListenerProcessStateEvent_eDataReady:
                {
                    /* Check immediately if there is avaiable FD befoere you wait for an event*/
                    if (CheckNowAndAccept( hListener )== BIP_SUCCESS)
                    {
                        /* Don't set state yet ending state yet, Process State after the wait in Accept will do so.  */
                         hListener->state = BIP_ListenerState_eAcceptDone;
                    }

                    if (hListener->waitingOnDataReadyEvent)
                    {
                        B_Event_Set((BKNI_EventHandle) hListener->dataReadyEvent );
                    }
                    else
                    {
                        BDBG_WRN(( " ??Got a Data Ready Callback while Accepting, but there was no one waiting for it?? " ));
                        printListenerStates( " Check States that causes this error", hListener, event );
                        ResetStates( hListener );
                    }

                    break;
                }
                /*After  the waitForEvent in Accept, no proces */
                case BIP_ListenerProcessStateEvent_eAccept:
                {
                    /* Should have been accepted alredy from the check now */
                    ResetStates( hListener );
                    /* Async support  */
 #if 0
                      if (hListener->settings.asyncAcceptedCallback.callback)
                      {

                          B_Mutex_Unlock( hListener->listenerLock ); /* must releast lock cause callback can call into Process state in this CB */
                          ( *hListener->settings.asyncAcceptedCallback.callback )( hListener->settings.asyncAcceptedCallback.context, hListener->settings.asyncAcceptedCallback.param );
                          B_Mutex_Lock( hListener->listenerLock );
                      }
#endif /* if 0 */

                    break;
                }
                case BIP_ListenerProcessStateEvent_eAcceptTimeout:
                {
                    /* Accept done go back to Listening */
                    ResetStates( hListener );
                    /* Async support  */
 #if 0
                      if (hListener->settings.asyncAcceptedCallback.callback)
                      {

                          B_Mutex_Unlock( hListener->listenerLock ); /* must releast lock cause callback can call into Process state in this CB */
                          ( *hListener->settings.asyncAcceptedCallback.callback )( hListener->settings.asyncAcceptedCallback.context, hListener->settings.asyncAcceptedCallback.param );
                          B_Mutex_Lock( hListener->listenerLock );
                      }
#endif /* if 0 */

                    break;
                }
                case BIP_ListenerProcessStateEvent_eStop:
                {
                    hListener->state = BIP_ListenerState_eStopping;

                    /* disable ListenerFd  from IOChecker */
                    BIP_IoChecker_Disable( hListener->hIoElement, BIP_IoCheckerEvent_ePollIn |BIP_IoCheckerEvent_ePollPri );
                    hListener->ioCheckerState = BIP_ListenerIoCheckerState_eIdle;

                    break;
                }

                default:
                {
                    BDBG_ERR(( "Invalid Event " ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
            } /* switch */

            break;
        }

        case BIP_ListenerState_eAcceptDone:
        {
            switch (event)
            {
                case BIP_ListenerProcessStateEvent_eDataReady:
                case BIP_ListenerProcessStateEvent_eStop:
                case BIP_ListenerProcessStateEvent_eStart:

                {
                    printListenerStates( " Check States that causes this error", hListener, event );
                    BDBG_ERR(( "%s:Need to handle this !!!   ", __FUNCTION__ ));

                    break;
                }

                /*just timed out, data ready before lock after waitEvent, does a check now then try to setEvent, but there is no Event  what happens?? */
                case BIP_ListenerProcessStateEvent_eAcceptTimeout:
                case BIP_ListenerProcessStateEvent_eAccept:
                {
                    ResetStates( hListener );
                    break;
                }
                default:
                {
                    BDBG_ERR(( "Invalid Event " ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
            } /* switch */
            break;
        }
        case BIP_ListenerState_eStopping:
        {
            switch (event)
            {
                case BIP_ListenerProcessStateEvent_eDataReady:
                case BIP_ListenerProcessStateEvent_eStart:
                {
                    BDBG_ERR(( "%s:Listener State = Stopping, Event=DataReady,Start  not allowed  ", __FUNCTION__ ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
                case BIP_ListenerProcessStateEvent_eAcceptTimeout:
                case BIP_ListenerProcessStateEvent_eAccept:
                {
                    /* Stop has set event so accept to go further now Accept must fire event back */
                    /*In the middle of shutting down */
                    B_Event_Set((BKNI_EventHandle) hListener->stopEvent );
                   #if 0
                    if (hListener->settings.asyncAcceptedCallback.callback)
                    {

                        B_Mutex_Unlock( hListener->listenerLock ); /* must releast lock cause callback can call into Process state in this CB */
                        ( *hListener->settings.asyncAcceptedCallback.callback )( hListener->settings.asyncAcceptedCallback.context, hListener->settings.asyncAcceptedCallback.param );
                        B_Mutex_Lock( hListener->listenerLock );
                    }
                  #endif /* if 0 */

                    break;
                }
                case BIP_ListenerProcessStateEvent_eStop:
                {
                    /*  has come back from accept Finish    Stop */
                    if (hListener->socketFd >= 0)
                    {
                        /* remove from IoChecker */
                        BIP_IoChecker_Destroy( hListener->hIoElement );

                        /* Close socket, so kernel doesn't get any more notifications */
                        shutdown(hListener->socketFd, SHUT_RDWR);
                        close( hListener->socketFd );
                    }

                    hListener->callbackState = BIP_ListenerCallbackState_eDisable;
                    hListener->state         = BIP_ListenerState_eIdle;
                    /* Reset the Events right after things get to Idle or right Into Listening*/
                    B_Event_Reset(hListener->stopEvent);
                    B_Event_Reset(hListener->dataReadyEvent);
                    break;
                }
                default:
                {
                    BDBG_ERR(( "Invalid Event " ));
                    rc = BIP_ERR_INTERNAL;
                    break;
                }
            } /* switch */
            break;
        }
        case BIP_ListenerState_eStopDone:
        {
            break;
        }

        default:
        {
            BDBG_ERR(( "Invalid State " ));
            rc = BIP_ERR_INTERNAL;
            break;
        }
    } /* switch */
/*    B_Mutex_Unlock( hListener->listenerLock ); */
    printListenerStates( "End of ProcessState", hListener, event );
    return( rc );
} /* ProcessState */

void BIP_Listener_GetDefaultCreateSettings(
    BIP_ListenerCreateSettings *pSettings
    )
{
    BDBG_ENTER( BIP_Listener_GetDefaultListenerCreateSettings );

    B_Os_Memset( pSettings, 0, sizeof( BIP_ListenerCreateSettings ));

    BDBG_LEAVE( BIP_Listener_GetDefaultListenerCreateSettings );
}

BIP_ListenerHandle BIP_Listener_Create(
    const BIP_ListenerCreateSettings *pSettings
    )
{
    BIP_Status          rc = BIP_SUCCESS;
    BIP_ListenerHandle hListener;

    /*malloc struct */
    hListener = (BIP_ListenerHandle) B_Os_Malloc( sizeof( *hListener ));
    B_Os_Memset( hListener, 0, sizeof( *hListener ));
    BDBG_OBJECT_SET( hListener, BIP_Listener );
    BDBG_ENTER( BIP_Listener_Create );

    /* create mutex */
    hListener->listenerLock = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hListener->listenerLock !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hListener->apiLock = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hListener->apiLock !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    hListener->acceptLock = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hListener->acceptLock !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    /* Create Events */
    hListener->dataReadyEvent = B_Event_Create( NULL );
    BIP_CHECK_GOTO(( hListener->dataReadyEvent ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hListener->stopEvent = B_Event_Create( NULL );
    BIP_CHECK_GOTO(( hListener->stopEvent ), ( "Event Creation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    if (NULL == pSettings)
    {
        BIP_Listener_GetDefaultCreateSettings( &hListener->createSettings );
    }
    else
    {
        /* If caller passed CreateSettings, use those. */
        BKNI_Memcpy( &hListener->createSettings, pSettings, sizeof( hListener->createSettings ));
    }
    pSettings = &hListener->createSettings;  /* Now we can use our own copy of the CreateSettings to access callbacks */

    /* Set Default for ListenerSettings */
    B_Os_Memset( &hListener->settings, 0, sizeof( BIP_ListenerSettings ));
    hListener->settings.connectedCallback.context     = hListener;
    hListener->settings.errorCallback.context         = hListener;
    hListener->settings.asyncAcceptedCallback.context = hListener;
    hListener->startSettings.queueDepth = LISTENER_BACKLOG_LENGTH;

    /* Set Initial State */
    hListener->state          = BIP_ListenerState_eIdle;
    hListener->ioCheckerState = BIP_ListenerIoCheckerState_eNotInUse;
    hListener->callbackState  = BIP_ListenerCallbackState_eNotInUse;

    BDBG_MSG((    BIP_MSG_PRE_FMT "Created: " BIP_LISTENER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_LISTENER_PRINTF_ARG(hListener)));
    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Created: " BIP_LISTENER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_LISTENER_PRINTF_ARG(hListener)));


    BDBG_LEAVE( BIP_Listener_Create );
    return(  hListener );

error:
    B_Os_Free( hListener );

    return( NULL );
} /* BIP_Listener_Create */

/**
 * Summary:
 * Default StartSettings
 **/
void BIP_Listener_GetDefaultStartSettings(
    BIP_ListenerStartSettings *pSettings
    )
{
    BDBG_ENTER( BIP_Listener_GetDefaultStartSettings );

    BDBG_ASSERT( pSettings );
    if (pSettings)
    {
        B_Os_Memset( pSettings, 0, sizeof( BIP_ListenerStartSettings ));
        pSettings->pInterfaceName = NULL;
        pSettings->pPort =  "80";
        pSettings->queueDepth = 16;
    }

    BDBG_LEAVE( BIP_Listener_GetDefaultStartSettings );
    return;
}

static int compare_string(const char *first, const char *second)
{
   if(first == NULL && second ==NULL)
    return 0;
   else if(first ==NULL && second != NULL)
     return 1;
   else if(first !=NULL && second == NULL)
    return 1;

   while(*first==*second)
   {
      if ( *first == '\0' || *second == '\0' )
         break;

      first++;
      second++;
   }
   if( *first == '\0' && *second == '\0' )
      return 0;
   else
      return 1;
}

BIP_Status BIP_Listener_Start(
    BIP_ListenerHandle hListener,
    BIP_ListenerStartSettings *pSettings
    )
{
    BIP_Status rc = BIP_SUCCESS;
    int settingCheck= 0;


    BDBG_ASSERT( hListener );

    B_Mutex_Lock( hListener->apiLock );
    B_Mutex_Lock( hListener->listenerLock );
    settingCheck = (compare_string(hListener->startSettings.pPort, pSettings->pPort) ||
         compare_string(hListener->startSettings.pInterfaceName,pSettings->pInterfaceName));

    if(hListener->state != BIP_ListenerState_eIdle && (
        hListener->startSettings.queueDepth != pSettings->queueDepth  ||
        settingCheck
        )
        ) {

        BDBG_WRN(("BIP_Listener_SetSettings: %lx can't change settings when started", (unsigned long)hListener));
        rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto error;
    }

    hListener->startSettings = *pSettings;

    /* socket, bind, listen, create and enable this  io element, initial state*/
    rc = ProcessState( hListener, BIP_ListenerProcessStateEvent_eStart );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "failure during Listener Start %d", errno ), error, rc, rc );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Started: " BIP_LISTENER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_LISTENER_PRINTF_ARG(hListener)));

error:
    /* todo */
    B_Mutex_Unlock( hListener->listenerLock );
    B_Mutex_Unlock( hListener->apiLock );
    return( rc );
} /* BIP_Listener_Start */


/**
 * Summary:
 * Default StartSettings
 **/
void BIP_Listener_GetDefaultSettings(
    BIP_ListenerSettings *pSettings
    )
{
    BIP_Status brc;
    BDBG_ASSERT( pSettings );
    BIP_CHECK_GOTO(( pSettings ), ( "pSettings can't be NULL" ), error, BIP_ERR_INVALID_PARAMETER, brc );
    B_Os_Memset( pSettings, 0, sizeof( BIP_ListenerSettings ));
error:
    return;
}

BIP_Status BIP_Listener_SetSettings(
    BIP_ListenerHandle    hListener,
    BIP_ListenerSettings *pSettings
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BDBG_OBJECT_ASSERT( hListener, BIP_Listener );
    BDBG_ASSERT( pSettings );
    B_Mutex_Lock( hListener->apiLock );
    B_Mutex_Lock( hListener->listenerLock );

    hListener->settings = *pSettings;

    B_Mutex_Unlock( hListener->listenerLock );
    B_Mutex_Unlock( hListener->apiLock );

    return( rc );
} /* BIP_Listener_SetSettings */

void BIP_Listener_GetSettings(
    BIP_ListenerHandle    hListener,
    BIP_ListenerSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT( hListener, BIP_Listener );
    BDBG_ASSERT( pSettings );

    B_Mutex_Lock( hListener->apiLock );

    *pSettings = hListener->settings;

    B_Mutex_Unlock( hListener->apiLock );
}

BIP_Status BIP_Listener_Stop(
    BIP_ListenerHandle hListener
    )
{
    int rc;

    BDBG_ASSERT( hListener );

    B_Mutex_Lock( hListener->apiLock );
    B_Mutex_Lock( hListener->listenerLock );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Stopping: " BIP_LISTENER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_LISTENER_PRINTF_ARG(hListener)));

    /* Disable ioElement, change state */
    ProcessState( hListener, BIP_ListenerProcessStateEvent_eStop );  /*Current State = Listening/Accepting  to  Next State = Stopping*/

    /* First check if the Accept is waiting. break out*/
    BDBG_MSG(( "%s: Stopping is there a waiting on DataReadyEvent %d ", __FUNCTION__, hListener->waitingOnDataReadyEvent ));
    if (hListener->waitingOnDataReadyEvent)
    {
        /* If accept actually waiting Should not take longer than 1 second to close out Accept */
        B_Event_Set((BKNI_EventHandle) hListener->dataReadyEvent );

        hListener->waitingOnStopEvent = true;  /*This variable isn't used right now */
        B_Mutex_Unlock( hListener->listenerLock );
        rc = B_Event_Wait( hListener->stopEvent, 1000 );
        B_Mutex_Lock( hListener->listenerLock );
        hListener->waitingOnStopEvent = false;
        if (( rc == BERR_TIMEOUT ) || ( rc != 0 ))
        {
            BDBG_ERR(( "%s: Has Timeout of %d reached, rc %d: %s", __FUNCTION__, 1000, rc == BERR_TIMEOUT ? "event timeout" : "event failure" ));
        }
    }

    /* destroy io element, close socket, change state */
    ProcessState( hListener, BIP_ListenerProcessStateEvent_eStop );   /*Current State =Stopping   to  Next State = Idle*/

    B_Mutex_Unlock( hListener->listenerLock );
    B_Mutex_Unlock( hListener->apiLock );
    return( BIP_SUCCESS );
} /* BIP_Listener_Stop */

/*
  Don't call the Blocking version perfrom Polling on a a non blocking version  of this function from a callback.  This can cause a deadlock.
*/
BIP_SocketHandle BIP_Listener_Accept(
    BIP_ListenerHandle hListener,
    int                timeout
    )
{
    BIP_Status rc = BIP_SUCCESS;
    BDBG_OBJECT_ASSERT( hListener, BIP_Listener );
    BDBG_MSG(( "%s: Start of ", __FUNCTION__ ));
    B_Mutex_Lock( hListener->acceptLock );
    B_Mutex_Lock( hListener->listenerLock );

    /* Default  value from accept should be NULL */
    hListener->hAcceptedSocket=NULL;

    /*Check Now,  change state   */
    ProcessState( hListener, BIP_ListenerProcessStateEvent_eAccept );  /* Current State = Listening to Next State = Accepting/AcceptDone */

    if (hListener->state== BIP_ListenerState_eAcceptDone)  /* check now returned something */
    {
        /*change states*/
        ProcessState( hListener, BIP_ListenerProcessStateEvent_eAccept );  /*Current State = Accept done to  Next State = Listening*/

        B_Mutex_Unlock( hListener->listenerLock );
        B_Mutex_Unlock( hListener->acceptLock );

        if (hListener->hAcceptedSocket) {
            BIP_MSG_TRC(( BIP_MSG_PRE_FMT "New connect on BipSocket=%p: " BIP_LISTENER_PRINTF_FMT
                          BIP_MSG_PRE_ARG, hListener->hAcceptedSocket, BIP_LISTENER_PRINTF_ARG(hListener)));
        }
        return( hListener->hAcceptedSocket );
    }

    /* non blocking: */
    if (timeout == 0)
    {
        /* Was not able to get  anything from CheckNow, use acceptTimeout to get back to listening*/
        ProcessState( hListener, BIP_ListenerProcessStateEvent_eAcceptTimeout ); /*Current State = Accepting/AcceptDone/Stopping  to  Next State = Listening Return NULL*/
    }
    /*blocking  *//* blocking with timeout */
    else if ((( timeout == -1 ) || ( timeout >0 )) && (( hListener->state == BIP_ListenerState_eListening ) || ( hListener->state == BIP_ListenerState_eAccepting )))
    {
        BDBG_MSG(( "%s: wait for Event: set by  dataReady or stop\n", __FUNCTION__ ));


        hListener->waitingOnDataReadyEvent = true;


        B_Mutex_Unlock( hListener->listenerLock );
        if (timeout == -1)
        {
            rc = B_Event_Wait( hListener->dataReadyEvent, B_WAIT_FOREVER);
        }
        else
        {
            rc = B_Event_Wait( hListener->dataReadyEvent, timeout );
        }
        /*just timed out, data ready, does a check now then try to setEvent, but there is no Event  what happens?? */
        B_Mutex_Lock( hListener->listenerLock );
        hListener->waitingOnDataReadyEvent = false;


        if (( rc == BERR_TIMEOUT ) || ( rc != 0 ))
        {
            BDBG_WRN(( "%s: Has Timeout of %d reached, rc %d: %s", __FUNCTION__, timeout, rc == BERR_TIMEOUT ? "event timeout" : "event failure" ));
            if (rc == BERR_TIMEOUT)
            {
                /* Change State */
                ProcessState( hListener, BIP_ListenerProcessStateEvent_eAcceptTimeout ); /*Current State = Accepting/AcceptDone/Stopping  to  Next State = Listening Return NULL*/
            }
        }
        else
        {
            /* process event from either DataReady or  Stop;
                       * if (Stopping State) =>  then set event stopEvent to allow stop to finish, and return NULL
                       * or if ( Accepting state)  =>   reset State; return  socket handle.
                       */
            ProcessState( hListener, BIP_ListenerProcessStateEvent_eAccept );    /*Current State = Accepting/AcceptDone/Stopping  to  Next State = Listening/Stopping*/
        }
    }

    B_Mutex_Unlock( hListener->listenerLock );
    B_Mutex_Unlock( hListener->acceptLock );

    if (hListener->hAcceptedSocket) {
        BIP_MSG_TRC(( BIP_MSG_PRE_FMT "New connect on BipSocket=%p: " BIP_LISTENER_PRINTF_FMT
                      BIP_MSG_PRE_ARG, hListener->hAcceptedSocket, BIP_LISTENER_PRINTF_ARG(hListener)));
    }

    return( hListener->hAcceptedSocket );
} /* BIP_Listener_Accept */

void BIP_Listener_Destroy(
    BIP_ListenerHandle hListener
    )
{
    BDBG_ASSERT( hListener );

    BIP_MSG_TRC(( BIP_MSG_PRE_FMT "Destroying: " BIP_LISTENER_PRINTF_FMT
                  BIP_MSG_PRE_ARG, BIP_LISTENER_PRINTF_ARG(hListener)));

    BIP_Listener_Stop( hListener );

    /* remove from  intneral static list */
    if (hListener->listenerLock)
    {
        B_Mutex_Lock( hListener->listenerLock );

        if (!BLST_Q_EMPTY( &g_ListenerListHead ))
        {
            BLST_Q_REMOVE( &g_ListenerListHead, hListener, listenerListNext );
        }
        B_Mutex_Unlock( hListener->listenerLock );
        B_Mutex_Destroy( hListener->listenerLock );
    }

    if (hListener->pInterfaceName) {B_Os_Free( hListener->pInterfaceName ); }
    if (hListener->dataReadyEvent) {B_Event_Destroy( hListener->dataReadyEvent ); }
    if (hListener->stopEvent) {B_Event_Destroy( hListener->stopEvent ); }
    if (hListener->acceptLock) {B_Mutex_Destroy( hListener->acceptLock ); }
    if (hListener->apiLock) {B_Mutex_Destroy( hListener->apiLock ); }

    BDBG_OBJECT_DESTROY( hListener, BIP_Listener );
    B_Os_Free( hListener );
} /* BIP_Listener_Destroy */

BIP_Status BIP_Listener_GetStatus(
    BIP_ListenerHandle  hListener,
    BIP_ListenerStatus *pStatus
    )
{
#if 1

    BDBG_MSG(( "%s: printing Listeners internal list ", __FUNCTION__ ));
    printBIPListenerList();

#endif /* if 1 */

    BDBG_ASSERT( pStatus );
    BDBG_OBJECT_ASSERT( hListener, BIP_Listener );
    pStatus->socketFd = hListener->socketFd;

    /* go through global list and find this  Todo*/
#if 0
    pStatus->numConnectionsPending  =;
    pStatus->numConnectionsAccepted =;
#endif
    return( BIP_SUCCESS );
} /* BIP_Listener_GetStatus */
