/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#include "bip_priv.h"


BDBG_MODULE( bip_iochecker );

BDBG_OBJECT_ID( BIP_IoCheckerFactory );
BDBG_OBJECT_ID( BIP_IoChecker);
BDBG_OBJECT_ID( BIP_IoWorker);


static pthread_mutex_t g_initIoCheckerFactoryMutex = PTHREAD_MUTEX_INITIALIZER;
static int g_refIoCheckerFactoryCount;

#define BIP_NUM_POLL_FD_ALLOCATE 32

#define BIP_IOCHECKER_DISABLE_EVENTS(x,y)  ((x) &= ~(y))


/* This enum is used to Enable, Disable
 * and Destroy IoCheckers.*/
typedef enum BIP_IoCheckerPipeMsg
{
    BIP_IoCheckerPipeMsg_eEnableIoChecker,
    BIP_IoCheckerPipeMsg_eDisableIoChecker,
    BIP_IoCheckerPipeMsg_eStopThread,
    BIP_IoCheckerPipeMsg_eWakeupPollingThread,
}BIP_IoCheckerPipeMsg;

typedef struct BIP_IoWorker
{
    BDBG_OBJECT( BIP_IoWorker )
    B_ThreadHandle    hIoCheckerThread;

    /* This is useful for debugging. */
    int threadIndex;
    /* Every worker will have its own stopThread flag .
     * This will help us to increase and decrease the number
     * of worker threads dynamically.*/
    bool stopThread;

    /* This will be used to synchronize worker thread
     * stop operation before destroying the threads.*/
    B_EventHandle      workerThreadStoppedEvent;
} BIP_IoWorker;

/* Globacl IoChecker object */
typedef struct BIP_IoCheckerFactory
{
    BDBG_OBJECT( BIP_IoCheckerFactory )

    /* ioCheckerLock - Used to protect the object data structures.
     *  Hold this mutex when:
     *      Changing any object data.
     *      When reading multiple object fields that must be in a consistent state.
     *      When navigating any of the object's linked lists. */
    B_MutexHandle  ioCheckerLock;

    /* apiLock - Used to serialize api execution and completion event(s).
     *  Hold this mutex during entire execution of blocking APIs.  This will
     *  prevent such things as trying to create two ioCheckers at the same time,
     *  or trying to delete and create at the same time. */
    B_MutexHandle  apiLock;

    /* pollLock - This mutex serializes the symetric worker threads, so that
     *  only thread will build the poll list and call poll.  When the active
     *  worker thread returns from poll() and releases this mutex, the next
     *  worker thread will acquire the poll lock, build the poll list, and
     *  then call poll(). */
    B_MutexHandle  pollLock;

    BIP_IoCheckerFactoryInitSettings initSettings;
    BLST_Q_HEAD( ioCheckerListHead, BIP_IoChecker ) ioCheckerListHead;
    /* TODO: Currently we maintain array of workers,
     * we need to check whether we may have a need to remove
     * or add workers based on stats. Then we may need to maintain a list.*
     * Later if we support dynamic worker creation and
     * deletion then we need to have a worker lock .*/
    BIP_IoWorker     *pIoWorker[BIP_MAX_IOWORKER];

    /** allocate based on BIP_MAX_NUM_FD only for listener , later
     *  if bigger size is required then allocate a bigger memory */
    struct pollfd *pfd;
    int            numPfdAllocated;
    int            numFdPolled;

    /* pipeFd[2] - is used to synchronize Enable, Disable and Destroy of IoCheckers
     *  with any IoWorker threads which is in polling state.
     *  Pipe read (pipe[0]) is the first IoChecker in the IoChecker list.
     *  Pipe write (pipe[1]) will wake up the polling thread to update its.
     *  polling list.*/
    int            pipeFd[2];

    /* Following are for staus information. */
    /* number of fd's added to IoCheckerFactory till now */
    int totalNumFdsAdded;

    /* number of Fds removed till now from IoCheckerFactory */
    int totalNumFdsRemoved;

    /* number of Fds currently polled.*/
    int currentNumFdsPolled;

    /* number of active fds not polled since they are either busy or disabled.*/
    int numActiveFdsNotPolled;

    /* max number of Fds polled */
    int maxNumFdsPolled;

} BIP_IoCheckerFactory;

struct BIP_IoChecker
{
    BDBG_OBJECT( BIP_IoChecker )
    BLST_Q_ENTRY( BIP_IoChecker ) ioCheckerNext; /* list of active BIP_Socket objects */
    int     fd;
    /*Requested events */
    BIP_IoCheckerEvent     eventMask;

    BIP_IoCheckerSettings settings;

    /* busy - Set right before a worker thread is invoking callback
     *  for this IoChecker. This allows other workers to skip
     *  this IoChecker. */
    int              refCount;

    /* deleteMe - This flag is used when a ioChecker is waiting for a CB to be finished before it can be destryed. */
    bool deleteMe;

    int               pollIndex;
} BIP_IoChecker;

/* Do I need to add any BDBG_OBJECT_SET( hSocket, BIP_Socket ); and OBJECT_DESTROY for the global.*/
static BIP_IoCheckerFactory gBipIoCheckerFactoryCtx;

static void removeIoCheckerFromIoCheckerFactory(
    BIP_IoCheckerFactory *pIoCheckerFactory,
    BIP_IoCheckerHandle hIoChecker
    );
static void destroyIoChecker(
    BIP_IoCheckerHandle hIoChecker
    );
/**
 * Summary:
 * Get Default IoCheckerFactoryInitsettings.
 *
 * Description:
 * Number of workerThreads default value is set to
 * BIP_DEFAULT_IOWORKER.
 **/
void BIP_IoCheckerFactory_GetDefaultInitSettings(
    BIP_IoCheckerFactoryInitSettings *pSettings
    )
{
    BDBG_ENTER( BIP_IoCheckerFactory_GetDefaultInitSettings );
    pSettings->workerThreads = BIP_DEFAULT_IOWORKER;
    /* TODO get defalt threadSettings from B_Os */
    B_Thread_GetDefaultSettings( &pSettings->threadSettings );
    BDBG_LEAVE( BIP_IoCheckerFactory_GetDefaultInitSettings );
}

/**
 * Summary:
 * API to instantly check one or set of events specified in
 * eventMask for an Fd . It returns a bit mask for all the set
 * events.
 **/
int BIP_Fd_CheckNow(
    int fd,
    BIP_IoCheckerEvent eventMask
    )
{
    struct pollfd pollFds;
    int readyFd = 0;

    BDBG_ENTER( BIP_Fd_CheckNow );

    pollFds.events = eventMask;
    pollFds.fd = fd;
    pollFds.revents = 0;

    do {
        readyFd = poll(&pollFds, 1, 0);
    } while (readyFd ==-1 && errno == EINTR);   /* EINTR => interrupted by signal. Not sure if this can happen with timeout of zero. */

    BDBG_LEAVE( BIP_Fd_CheckNow );
    return (readyFd ? pollFds.revents : 0 );
}

/**
 * Summary:
 * API to instantly check one or set of events specified in
 * eventMask for a hIoChecker . It returns a bit mask for all
 * the set events.
 **/
int BIP_IoChecker_CheckNow(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerEvent eventMask
    )
{
    struct pollfd pollFds;
    int readyFd = 0;

    BDBG_ENTER( BIP_IoChecker_CheckNow );

    pollFds.events = eventMask;
    pollFds.fd = hIoChecker->fd;
    pollFds.revents = 0;

    /* EINTR => interrupted by signal. Not sure if this can happen with timeout of zero. */
    do {
        readyFd = poll(&pollFds, 1, 0);
    } while (readyFd ==-1 && errno == EINTR);

    BDBG_LEAVE( BIP_IoChecker_CheckNow );
    return (readyFd ? pollFds.revents : 0 );
}

/**
 * Summary: This function adds all the fds with any Eventmask
 * set to be added into the Polling List. It expects caller to
 * have the PollLock and ioCheckerLock.
 * */
static int addFdToMonitorList( void )
{
    BIP_IoCheckerHandle hIoChecker;
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    int pollIndex       = 0;
    struct pollfd *pfds;
    BIP_Status               rc = BIP_SUCCESS;
    int numActiveFdsNotPolled = 0;

    BDBG_ENTER( addFdToMonitorList );

    B_MUTEX_ASSERT_LOCKED(pIoCheckerFactory->pollLock );
    B_MUTEX_ASSERT_LOCKED(pIoCheckerFactory->ioCheckerLock );

#if 1
    if (pIoCheckerFactory->numFdPolled  > pIoCheckerFactory->numPfdAllocated)
    {
        int newSize;
        struct pollfd *tempPfd = NULL;

        /* Adding NumFdPolled with BIP_NUM_POLL_FD_ALLOCATE */
        newSize = pIoCheckerFactory->numFdPolled + BIP_NUM_POLL_FD_ALLOCATE;

        tempPfd = (struct pollfd *)B_Os_Malloc( newSize * sizeof( struct pollfd));

        /* I need to add this since BIP_CHECK_GOTO always expect rc having a value, if i derectly pass 0 it fails.*/
        if(tempPfd == NULL)
        {
            rc = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
        }
        /*BIP_CHECK_PTR_GOTO( tempPfd, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );*/
        BIP_CHECK_GOTO((tempPfd != NULL), ( "Memory Allocation failed " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

        if (pIoCheckerFactory->pfd) {B_Os_Free( pIoCheckerFactory->pfd ); }
        /*everytime we increase the allocation by BIP_NUM_POLL_FD_ALLOCATE*/
        pIoCheckerFactory->pfd = tempPfd;
        BDBG_MSG(( "%s Pfd ReAllocated   oldSize = %d and newSize = %d ", __FUNCTION__, pIoCheckerFactory->numPfdAllocated, newSize ));
        pIoCheckerFactory->numPfdAllocated = newSize;

    }
#endif

    pfds = pIoCheckerFactory->pfd;
    /* I need to add this since BIP_CHECK_GOTO always expect rc having a value, if i derectly pass 0 it fails.*/
    if(pfds == NULL)
    {
        rc = BIP_ERR_OUT_OF_SYSTEM_MEMORY;
    }
    BIP_CHECK_GOTO((pfds != NULL), ( "pfds NULL " ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    numActiveFdsNotPolled = 0;
    for (hIoChecker = BLST_Q_FIRST( &pIoCheckerFactory->ioCheckerListHead );
         hIoChecker ;
         hIoChecker = BLST_Q_NEXT( hIoChecker, ioCheckerNext ))
    {
        if(hIoChecker->eventMask)
        {
            pfds[pollIndex].fd        = hIoChecker->fd;

            /* Requested events set .*/
            pfds[pollIndex].events    = hIoChecker->eventMask;
            hIoChecker->pollIndex     = pollIndex;
            pfds[pollIndex++].revents = 0;
        }
        else
        {
            /* staus information */
            numActiveFdsNotPolled++;
        }
    }

    /* staus information */
    pIoCheckerFactory->currentNumFdsPolled = pollIndex ;

    /* staus information */
    if(pIoCheckerFactory->maxNumFdsPolled < pIoCheckerFactory->currentNumFdsPolled)
    {
       pIoCheckerFactory->maxNumFdsPolled =  pIoCheckerFactory->currentNumFdsPolled;
    }

    /* staus information */
    pIoCheckerFactory->numActiveFdsNotPolled = numActiveFdsNotPolled;

    BDBG_LEAVE( addFdToMonitorList );
    return pollIndex;

error:
    BDBG_LEAVE( addFdToMonitorList );
    /* return number of fds polled as 0 in error case */
    return 0;
}

static void pipeReadCallback(
    void *context,
    int param,
    BIP_IoCheckerEvent eventMask
    )
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_IoCheckerPipeMsg msg;
    ssize_t  readCount;
    BSTD_UNUSED( param );
    BSTD_UNUSED( context );
    BSTD_UNUSED( eventMask );

    BDBG_ENTER( pipeReadCallback );

    readCount = read(pIoCheckerFactory->pipeFd[0], &msg, sizeof(BIP_IoCheckerPipeMsg));
    if (readCount < (ssize_t)sizeof(BIP_IoCheckerPipeMsg)) {
        BDBG_WRN(("%s: Partial read from pipeFd, count = %zd", __FUNCTION__, readCount));
    }
    else {
        switch(msg)
        {
            /* Though this 3 mesasge does the same thing they have been
             * maintained as 3 different message for debugging and redability purpose.*/
            case BIP_IoCheckerPipeMsg_eEnableIoChecker:
            case BIP_IoCheckerPipeMsg_eDisableIoChecker:
            break;

            case BIP_IoCheckerPipeMsg_eStopThread:
            case BIP_IoCheckerPipeMsg_eWakeupPollingThread:
            /*void*/
            break;
        }
        BDBG_MSG(("%s: Call back executed for a message = %d", __FUNCTION__, msg));
    }
    BDBG_LEAVE( pipeReadCallback );
}

static void ioWorkerThread ( void *context )
{
    BIP_IoWorker  *pIoWorker = context;
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;

    while(!pIoWorker->stopThread)
    {
        int timeout = -1; /* right now we set poll timeout -1 , so it will wait untill there is any event.*/
        int numFdsPolled = 0;
        int numReadyFds = 0;
        BIP_IoCheckerHandle hIoChecker = NULL;
        BIP_IoCheckerEvent eventMask = 0;
        BIP_IoCheckerHandle hIoCheckerReady = NULL;
        struct pollfd *pfds;

        B_Mutex_Lock( pIoCheckerFactory->pollLock );

        B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );

        numFdsPolled = addFdToMonitorList();

        /*Initialize pfds after addFdToMonitorList, since the array may get recreated inside this function based on number of IoCheckers.*/
        pfds = pIoCheckerFactory->pfd;

        B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

        if(numFdsPolled)
        {
            /*BDBG_ERR(("%s: ---------------------- Start Polling -----------------------",__FUNCTION__));*/
            numReadyFds = poll( pfds, numFdsPolled, timeout );
            /*BDBG_ERR(("%s: ---------------------- End Polling -----------------------",__FUNCTION__));*/
        }

        if(numReadyFds)
        {
            B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );

            /* No Callback is required to call on Handle hIoCheckerReady->fd == pIoCheckerFactory->pipeFd[0], since this is our iochecker
               which is used for iochecker internal communication. */
            hIoChecker = BLST_Q_FIRST( &pIoCheckerFactory->ioCheckerListHead );

            /*First hIoChecker is always pipeFd[0], since we add it at the time of IoCheckerFactory creation.*/
            if (pfds[hIoChecker->pollIndex].revents == POLLIN)
            {
                /* This hIoChecker for pipeFd[0] will never be set with busy, since we want it to always present in polling list.*/
                eventMask = pfds[hIoChecker->pollIndex].revents ;
                hIoChecker->settings.callBackFunction( hIoChecker->settings.callBackContext,hIoChecker->settings.callBackParam, eventMask);

                hIoCheckerReady = NULL;/*hIoCheckerReady code should not get executed */
            }
            else /* check all other iocheckers */
            {

                /*First hIoChecker is always pipeFd[0], since we add it at the time of IoCheckerFactory creation. */
                hIoChecker = BLST_Q_NEXT( hIoChecker, ioCheckerNext );
                while(hIoChecker)
                {
                   /* REVIEW: PollIndex based validation can't be removed.
                      Reason: Suppose a hIoChecker is not enabled at a point when a workerThread is polling.At that point
                      a new hIoChecker(say X) is enabled(acquire ioCheckerLock,Enable,  Relerase ioCheckerLock).
                      At this time the polling thread wakeup. In this case the hIoChecker (X)
                      valid but still we can't use it since the pollingThread's poll list was not formed with this hIoChecker(X).
                      PollIndex(-1) based validation will help to handle this issue.*/
                    if((hIoChecker->eventMask) && (hIoChecker->pollIndex != -1))
                    {
                        if( pfds[hIoChecker->pollIndex].revents )
                        {
                            hIoCheckerReady = hIoChecker;
                            eventMask = pfds[hIoChecker->pollIndex].revents ;
                            /* RefCount will keep track of the Iochecker being in use .
                               The same Iochecker object can be executing mutiple call backs for different events.
                               We will Destroy the Iochecker object only when refCount will be 0.*/
                            hIoCheckerReady->refCount++;
                            /*The IoChecker object that is created by caller will enable the IoChecker for one of multiple events.
                              Enable is like a request to the IoChecker object to get a Callback when the event occurs.
                              IoChecker object will disable  the event once the event is occurred and ready to raise the callback for that.
                              Caller has to call Enable api again when it wants to observe occurrence of the same event.
                              This applies for all Iochecker object other than the pipe based IoChecker object that is created by IoChecker                           .
                              for internal communication.*/
                            BIP_IOCHECKER_DISABLE_EVENTS(hIoCheckerReady->eventMask,pfds[hIoChecker->pollIndex].revents);

                            break;
                        }
                    }

                    hIoChecker = BLST_Q_NEXT( hIoChecker, ioCheckerNext );
                }

            }
            B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );
        }


        B_Mutex_Unlock( pIoCheckerFactory->pollLock );

        /* this code will only come into picture for all other hIoChecker other than hIoChecker for pipeFd[0] */
        if(hIoCheckerReady)
        {
            BIP_IoCheckerPipeMsg msg = BIP_IoCheckerPipeMsg_eWakeupPollingThread;
            /*TODO: What to do if some one is trying to stop the thread at this point .*/
            if(hIoCheckerReady->settings.callBackFunction)
            {
                /*TODO:Call the call back*/
                /*hIoCheckerReady->settings.dataReadyCallback */
                BDBG_MSG(("%s: Entering Call Back for hIoChecker = %p , fd = %d in threadIndex = %d", __FUNCTION__, (void *)hIoCheckerReady, hIoCheckerReady->fd,pIoWorker->threadIndex ));
                hIoCheckerReady->settings.callBackFunction( hIoCheckerReady->settings.callBackContext,hIoCheckerReady->settings.callBackParam, eventMask);
                BDBG_MSG(("%s: Exiting Call Back for hIoChecker = %p , fd = %d in threadIndex = %d", __FUNCTION__, (void *)hIoCheckerReady, hIoCheckerReady->fd,pIoWorker->threadIndex ));
            }

            B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );

            hIoCheckerReady->refCount--;

            if((hIoCheckerReady->deleteMe)&&(!hIoCheckerReady->refCount))
            {
                /* once call back is executed delete this iochecker element.*/
                /*For an example this happens in a  case where  server is running in the callback mode, client connects to the server and then closes
                  the socket w/o sending any requests. Http socket object is created, it gets dataReadyCallback from BIP_Socket when it detects socket
                  error, so it calls errorCallback. In this callback, I am just destroying the HttpSocket (may not be right thing to do, but we should
                  be able to handle this!).
                  Http_Socket_Destroy calls BIP_Socket_Destroy, which is trying to Destroy Io_Checker and gets stuck.
                  In this case IoChecker has invoked the dataReadyCB into BIP_Socket and thus can?t Destroy itself. We will need to handle this!
                */

                /*IoChecker already removed from the factory list, this function will free the memory for ioChecker.*/
                destroyIoChecker(hIoCheckerReady);
            }
            B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

            /*This is just to wakeup the other polling thread so that it includes this Fd in the polling list.
             * Consider a case where a single Fd is present, once it has a event a CB on that is raised and the next worker
             * will poll without this Fd, now this Fd immidiately came out of CB , say within few microseccond and got another event.
             * This second event can only be handled once the other worker thread comes out of poll, in worst case that will be the
             * "timeout = 900" duration.
             * But we shouldn't do this if the poll thread has wakeup because of pipefd[0].
             * Else it will go into an infinite loop of pipeFd[0] waking up poll threads backto back.
             * Since this loop is for all other ioCheckers than the pipFd[0] , so it will never get into that situation.*/
            write(pIoCheckerFactory->pipeFd[1], &msg, sizeof(BIP_IoCheckerPipeMsg));
        }
    }

    BDBG_MSG(("%s: Exiting thread for threadIndex = %d ", __FUNCTION__, pIoWorker->threadIndex));
}

static void stopWorkerThreads(void)
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    int i;

    BDBG_ENTER( stopWorkerThreads );

    for(i = 0; i < pIoCheckerFactory->initSettings.workerThreads ; i++ )
    {
        /*TODO: Do I need to add some delay between stopThread and THread_Destroy call. */
        BIP_IoWorker *pIoWorker = NULL;
        pIoWorker = pIoCheckerFactory->pIoWorker[i];

        if(pIoWorker)
        {
            BDBG_ASSERT( pIoWorker );
            if(pIoWorker->hIoCheckerThread)
            {
                /*TODO: Do I need to add some delay between stopThread and THread_Destroy call. */
                pIoWorker->stopThread = true;
                /*set the pipeFd to inform that stopThread is set for that worker thread */
            }
        }
    }

    for(i = 0; i < pIoCheckerFactory->initSettings.workerThreads ; i++ )
    {
        BIP_IoCheckerPipeMsg stop = BIP_IoCheckerPipeMsg_eStopThread;
        write(pIoCheckerFactory->pipeFd[1], &stop, sizeof(BIP_IoCheckerPipeMsg));
    }
    BDBG_LEAVE( stopWorkerThreads );
}

static void destroyWorkers(void)
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_IoWorker *pIoWorker = NULL;
    int i;

    BDBG_ENTER( destroyWorkers );

    for(i = 0; i < pIoCheckerFactory->initSettings.workerThreads ; i++ )
    {
        pIoWorker = pIoCheckerFactory->pIoWorker[i];
        if(pIoWorker)
        {
            BDBG_ASSERT( pIoWorker );
            if(pIoWorker->hIoCheckerThread)
            {
                /*set the pipeFd to inform that stopThread is set for that worker thread */
                /*BDBG_ERR(("Writing pollFd to stop the worker thread"));*/
                /*write(pIoCheckerFactory->pipeFd[1], &stop, sizeof(stop));*/


                B_Thread_Destroy( pIoWorker->hIoCheckerThread);
            }
            BDBG_OBJECT_DESTROY( pIoWorker, BIP_IoWorker );
            B_Os_Free( pIoWorker );
        }
    }
    BDBG_LEAVE( destroyWorkers );
}

static void removeIoCheckerFromIoCheckerFactory(
    BIP_IoCheckerFactory *pIoCheckerFactory,
    BIP_IoCheckerHandle hIoChecker
    )
{
    /* first remove the ioChecker Element from the Factory list.*/
    BLST_Q_REMOVE( &pIoCheckerFactory->ioCheckerListHead , hIoChecker, ioCheckerNext );
    /* reduce num fd polled */
    pIoCheckerFactory->numFdPolled--;
    pIoCheckerFactory->totalNumFdsRemoved++;
}

static void destroyIoChecker(
    BIP_IoCheckerHandle hIoChecker
    )
{
    BDBG_ENTER( destroyIoChecker );
    BDBG_ASSERT(hIoChecker);
    BDBG_OBJECT_ASSERT( hIoChecker, BIP_IoChecker );

    if(hIoChecker)
    {
        BDBG_MSG(("%s: Deleting hIoChecker = %p for fd = %d", __FUNCTION__, (void *)hIoChecker, hIoChecker->fd));

        /* IoChecker element is already removed from the Factory list and */

        /* free iochecker*/
        B_Os_Free( hIoChecker );
    }
    BDBG_LEAVE( destroyIoChecker );
}

/*This has to be handled at the end since , it can't be done with destroyAllIoCheckersInFactory().
  ThreadStop and ioElement deletion all has dependency on this since a pipe write is used for synchronization.
  */
static void destroyIoCheckerForPipeRead(void)
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_IoCheckerHandle hIoChecker = NULL;

    BDBG_ENTER( destroyIoCheckerForPipeRead );

    /*This api Lock is acquired to keep a synargy of this function with IoChecklerDestroy Function.Later we ma have pipe Fd dynamic addition and removal.*/
    B_Mutex_Lock( pIoCheckerFactory->apiLock );

    B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );

    /* WakeUp Fd or Pipe[0] is always the first ioChecker element created at the init time. */
    hIoChecker = BLST_Q_FIRST( &pIoCheckerFactory->ioCheckerListHead );

    /* This will take care of the case when even IoChecker based on a pipeFd is not valid.
     * Some reason PipeFd based IoChecker creation fails and eventually InitFails. */

    if(hIoChecker)
    {
        /* first remove the ioChecker Element from the Factory list and reduce num fd polled.*/
        removeIoCheckerFromIoCheckerFactory(pIoCheckerFactory, hIoChecker);
        destroyIoChecker(hIoChecker);
    }

    B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

    B_Mutex_Unlock( pIoCheckerFactory->apiLock );
    BDBG_LEAVE( destroyIoCheckerForPipeRead );
}


static void destroyAllIoCheckersInFactory(void)
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_IoCheckerHandle hIoChecker;

    BDBG_ENTER( destroyAllIoCheckersInFactory );

    BDBG_ASSERT(pIoCheckerFactory);

    /* ioElement[0] - We need to skip the first element which is pipFd.
     *  since the ioElement holding pIoCheckerFactory->pipeFd[1] need to be
     *  deleted at the end once all elements are destroyed and threads are stopped.
     *  Everyone has a dependency on this since thread stop and ioElement
     *  deletion is synchronized using pipFd.
     *  It is the main control Fd it need to be handled separately.*/

    B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
    hIoChecker = BLST_Q_FIRST( &pIoCheckerFactory->ioCheckerListHead );

    /* This will take care of the case when even IoChecker based on a pipeFd is not valid.
     * Some reason, PipeFd based IoChecker creation fails and eventually InitFails. */
    if(hIoChecker)
    {
        hIoChecker = BLST_Q_NEXT( hIoChecker , ioCheckerNext);
    }
    B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );


    while(hIoChecker)
    {
        /* If any IoChecker is busy then BIP_IoChecker_Destroy will remove them from Factory and mark them for later deletion. */
        /* at the end of stop thread they will automaticcally be destroyed so need to explicitely check whether they are deleted or not.*/
        BIP_IoChecker_Destroy( hIoChecker );

        B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
        hIoChecker = BLST_Q_FIRST( &pIoCheckerFactory->ioCheckerListHead );
        hIoChecker = BLST_Q_NEXT( hIoChecker , ioCheckerNext);
        B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );
    }
    BDBG_LEAVE( destroyAllIoCheckersInFactory );
}

BIP_IoCheckerHandle createIoChecker(
    int fd
    )
{
    BIP_IoCheckerHandle hIoChecker = NULL;

    BDBG_ENTER( createIoChecker );

    hIoChecker = B_Os_Malloc( sizeof( BIP_IoChecker));
    /*BIP_CHECK_GOTO(( hIoChecker !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_INTERNAL, rc );*/
    if(hIoChecker == NULL)
    {
        BDBG_ERR(("Memory Allocation Failed"));

        BDBG_LEAVE( createIoChecker );
        return NULL;
    }
    else
    {
        B_Os_Memset( hIoChecker, 0, sizeof( BIP_IoChecker ));
        BDBG_OBJECT_SET( hIoChecker, BIP_IoChecker );

        hIoChecker->fd = fd;
        hIoChecker->pollIndex = -1;

        BDBG_LEAVE( createIoChecker );
        return (hIoChecker);
    }
}

static void ioCheckerUninit(void)
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;

    BDBG_ENTER( ioCheckerUninit );

    BDBG_ASSERT( pIoCheckerFactory );

    BDBG_MSG(("%s:",__FUNCTION__));
    /* ioCheckerLock is the first element , if it is NULL, that means it is
     *  failed to create ioCheckerLock mutex in Init,same with ApiLock.
     *  If IoCheckerLock and API lock creation fails then we can't do anything just return,
     *  since all the other cleanUp module like Destroy, eventually acquire this two locks before
     *  performing any operation.
     *  TODO: Checke better way to handle it. */
    if(pIoCheckerFactory->ioCheckerLock == NULL)
    {
        BDBG_LEAVE( ioCheckerUninit );
        return;
    }
    if(pIoCheckerFactory->apiLock == NULL)
    {
        if(pIoCheckerFactory->ioCheckerLock) { B_Mutex_Destroy( pIoCheckerFactory->ioCheckerLock ); }
        BDBG_LEAVE( ioCheckerUninit );
        return;
    }

    /* This will clean up if any ioCheckers reamin in the list.
     * This internallly calls ioChecker_Destroy for all ramining ioCheckers in the list.*/
    destroyAllIoCheckersInFactory();
    /*B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );*/

    /* Stop all worker threads.  */
    stopWorkerThreads();

    /* Destroy all worker thraeds.*/
    destroyWorkers();

    /* Destroy the ioChecker for wakeup Fd.*/
    destroyIoCheckerForPipeRead();

    if(pIoCheckerFactory->pfd) { B_Os_Free(pIoCheckerFactory->pfd); }
    if(pIoCheckerFactory->pipeFd[0]) { close(pIoCheckerFactory->pipeFd[0]); }
    if(pIoCheckerFactory->pipeFd[1]) { close(pIoCheckerFactory->pipeFd[1]); }

    if(pIoCheckerFactory->pollLock) { B_Mutex_Destroy( pIoCheckerFactory->pollLock ); }
    if(pIoCheckerFactory->apiLock) { B_Mutex_Destroy( pIoCheckerFactory->apiLock ); }
    if(pIoCheckerFactory->ioCheckerLock) { B_Mutex_Destroy( pIoCheckerFactory->ioCheckerLock ); }
    BDBG_LEAVE( ioCheckerUninit );
}

void BIP_IoChecker_GetDefaultCreateSettings(
    BIP_IoCheckerCreateSetting *pSettings
    )
{
    BDBG_ENTER( BIP_IoChecker_GetDefaultCreateSettings );
    B_Os_Memset( pSettings, 0, sizeof( BIP_IoCheckerCreateSetting));
    BDBG_LEAVE( BIP_IoChecker_GetDefaultCreateSettings );
}

static void updateApiEvent(
    BIP_IoCheckerPipeMsg msg
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;

    BDBG_ENTER( updateApiEvent );

    B_MUTEX_ASSERT_LOCKED(pIoCheckerFactory->apiLock );

    /* Pipe write */
    write(pIoCheckerFactory->pipeFd[1], &msg, sizeof(BIP_IoCheckerPipeMsg));

    BIP_CHECK_GOTO((rc == B_ERROR_SUCCESS), ( "B_Event_Wait failed, rc:0x%X", rc ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    BDBG_LEAVE( updateApiEvent );
    return;

error:
    BDBG_LEAVE( updateApiEvent );
    return;
}

static BIP_Status validateIoChecker(
    BIP_IoCheckerHandle hIoChecker
    )
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_IoCheckerHandle hIoCheckerInList = NULL;
    BIP_Status rc;

    BDBG_ENTER( validateIoChecker );

    B_MUTEX_ASSERT_LOCKED(pIoCheckerFactory->ioCheckerLock );

    /*TODO: Later we will check whether this list search is an extra kill , can I avoid it.*/
    for (hIoCheckerInList = BLST_Q_FIRST( &pIoCheckerFactory->ioCheckerListHead );
         hIoCheckerInList;
         hIoCheckerInList = BLST_Q_NEXT( hIoCheckerInList, ioCheckerNext ))
    {
        if((hIoCheckerInList->fd == hIoChecker->fd) )
        {
            /* Found the ioChecker */
            break;
        }
    }

    if(hIoCheckerInList)
    {
        rc = BIP_SUCCESS;
    }
    else
    {
        rc = BIP_ERR_INVALID_PARAMETER;;
    }

    BDBG_LEAVE( validateIoChecker );
    return rc;
}

/**
 * Summary:
 * API to disable hIoChecker
 *
 * Description:
 * This API disables event/events that have their respective bits set
 * in eventMask. This won't verify whether the event/events are
 * already enabled or not.
 * */
BIP_Status BIP_IoChecker_Disable(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerEvent eventMask
    )
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_Status rc;
    BIP_IoCheckerPipeMsg disable ;

    BDBG_ENTER( BIP_IoChecker_Disable );

    B_Mutex_Lock( pIoCheckerFactory->apiLock );

    B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
    /*TODO: Later we will check whether this list search is an extra kill , can I avoid it.*/
    rc = validateIoChecker( hIoChecker );
    B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

    if(rc == BIP_SUCCESS)
    {
        B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
        BDBG_MSG(("%s: disabling fd = %d", __FUNCTION__,hIoChecker->fd ));

        /*this will make sure we only remove the set events that uper laywer wants to remove.*/
        BIP_IOCHECKER_DISABLE_EVENTS(hIoChecker->eventMask,eventMask);
        B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

        /* B_Mutex_Unlock of ioCheckerLock should always be done before calling updateApiEvent else it will deadlock.*/
        disable = BIP_IoCheckerPipeMsg_eDisableIoChecker;
        updateApiEvent(disable);
    }
    else
    {
        BDBG_WRN(("%s: Can't disable the ioChecker object %p , it is not present in the ioCheckerFactoryList", __FUNCTION__, (void *)hIoChecker));
    }

    rc = BIP_SUCCESS;

    B_Mutex_Unlock( pIoCheckerFactory->apiLock );
    BDBG_LEAVE( BIP_IoChecker_Disable );
    return(rc);
}

/**
 * Summary:
 * API to enable hIoChecker in IoChecker list for one
 * or set of events specified in eventMask.
 **/
BIP_Status BIP_IoChecker_Enable(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerEvent eventMask
    )
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_Status rc;
    BIP_IoCheckerPipeMsg enable ;

    BDBG_ENTER( BIP_IoChecker_Enable );

    B_Mutex_Lock( pIoCheckerFactory->apiLock );

    B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
    /*TODO: Later we will check whether this list search and validation is an extra kill , can I avoid it.*/
    rc = validateIoChecker( hIoChecker );
    B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

    if(rc == BIP_SUCCESS)
    {
        B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
        hIoChecker->eventMask |= eventMask;
        B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );
        BDBG_MSG(("%s: hIoChecker = %p, fd = %d, Enabled for Events = %d and refCount = %d \n",__FUNCTION__, (void *)hIoChecker, hIoChecker->fd,hIoChecker->eventMask, hIoChecker->refCount));

        /* B_Mutex_Unlock of ioCheckerLock should always be done before calling updateApiEvent else it will deadlock.*/
        enable = BIP_IoCheckerPipeMsg_eEnableIoChecker;
        updateApiEvent(enable);
        BDBG_MSG(("%s: enabled hIoChecker for fd = %d", __FUNCTION__, hIoChecker->fd));
        rc = BIP_SUCCESS;
    }
    else
    {
        /*TODO check whether this need to be returned as rc = BIP_ERR_INVALID_PARAMETER;*/
        rc = BIP_SUCCESS;
        BDBG_WRN(("%s: ioChecker:%p can't be enabled since it is not in the ioChecker list",__FUNCTION__, (void *)hIoChecker));
    }

    B_Mutex_Unlock( pIoCheckerFactory->apiLock );

    BDBG_LEAVE( BIP_IoChecker_Enable );
    return(rc);
}

/**
 * Summary:
 * API to Get IoChecker Settings
 *
 * Description:
 **/
BIP_Status BIP_IoChecker_GetSettings(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerSettings *pSettings
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_ENTER( BIP_IoChecker_GetSettings );
    BDBG_OBJECT_ASSERT( hIoChecker, BIP_IoChecker );
    BDBG_ASSERT( pSettings );
    *pSettings = hIoChecker->settings;
    BDBG_LEAVE( BIP_IoChecker_GetSettings );
    return( errCode );
}

/**
 * Summary:
 * API to Set IoChecker Settings
 *
 * Description:
 **/
BIP_Status BIP_IoChecker_SetSettings(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerSettings *pSettings
    )
{
    BIP_Status errCode = BIP_SUCCESS;

    BDBG_ENTER( BIP_IoChecker_SetSettings );
    BDBG_OBJECT_ASSERT( hIoChecker, BIP_IoChecker );
    BDBG_ASSERT( pSettings );

    /* validate parameters */
    hIoChecker->settings = *pSettings;
    BDBG_LEAVE( BIP_IoChecker_SetSettings );

    return( errCode );
}

/**
 * Summary:
 * API to delete hIoChecker from IoCheckerFactory list.
 **/
void BIP_IoChecker_Destroy(
    BIP_IoCheckerHandle hIoChecker
    )
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_Status rc;

    BDBG_ENTER( BIP_IoChecker_Destroy );

    B_Mutex_Lock( pIoCheckerFactory->apiLock );

    B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
    /*TODO: Later we will check whether this list search is an extra kill , can I avoid it.*/
    rc = validateIoChecker( hIoChecker );
    B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

    if(rc == BIP_SUCCESS)
    {
        /* we are acquring the lock here since for eventMask check and refCount flag check we need to have the lock */
        B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );

        /* This is to disable all the event so that ,the next worker thread by no chance
         * add this in to its list when it is going through deletion process*/
        if(hIoChecker->eventMask)
        {
            BIP_IoCheckerPipeMsg disable ;

            BDBG_MSG(("%s: disabling hIoChecker for fd = %d", __FUNCTION__,hIoChecker->fd ));

            /*  this will make sure we remove all the events.*/
            hIoChecker->eventMask = 0;
            /*we need to release the ioCheckerLock otherwise updateApiEvent will get into deadlock*/
            B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

            disable = BIP_IoCheckerPipeMsg_eDisableIoChecker;
            updateApiEvent(disable);

            /*Acquire the ioCheckerLock , so rest of the code is in same lock condition for non if case.*/
            B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );

        }

        if( hIoChecker->refCount)
        {
            /* code sequence already has the pIoCheckerFactory->ioCheckerLock */
            /* mark deleteMe , so that once it comes out of the call back it will be deleted , Right now it will be called in the workerThread function.*/
            hIoChecker->deleteMe = true;

            /* Now remove the ioChecker Element from the Factory list and reduce num fd polled.*/
            removeIoCheckerFromIoCheckerFactory(pIoCheckerFactory, hIoChecker);

            BDBG_MSG(("%s: hIoChecker for fd = %d is busy, marked it to delete later", __FUNCTION__, hIoChecker->fd));
            /* This unlock will allow the workerThread to acquire the lock
               and set the Callback done event based on deleteMe flag.*/
            B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );
        }
        else
        {
            /* code sequence already has the pIoCheckerFactory->ioCheckerLock */

            /* Now remove the ioChecker Element from the Factory list and reduce num fd polled.*/
            removeIoCheckerFromIoCheckerFactory(pIoCheckerFactory, hIoChecker);

            destroyIoChecker(hIoChecker);

            B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );
        }

        BDBG_MSG(("%s: IoCheckerDestroyed",__FUNCTION__));

    }
    else
    {
        BDBG_WRN(("%s: Can't destroy the ioChecker object for fd =%d, it is not present in the ioCheckerFactoryList", __FUNCTION__, hIoChecker->fd));
    }

    B_Mutex_Unlock( pIoCheckerFactory->apiLock );
    BDBG_LEAVE( BIP_IoChecker_Destroy );
}

/**
 * Summary: API creates an IoChecker to monitor a file descriptor.
 *
 * Description:
 * When created, the IoChecker will not monitor any IoEvents.
 * Events can only be enabled by BIP_IoChecker_Enable api.
 * Multiple IoCheckers can be created using same Fd.
 * BIP_IoChecker_Enable will prevent and send a warning message
 * incase one tries to enable same event (if any event bits are
 * same) for the second IoChecker created using a common Fd.
 **/
BIP_IoCheckerHandle BIP_IoChecker_Create(
    BIP_IoCheckerCreateSetting *pSettings
    )
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_IoCheckerHandle hIoChecker = NULL;
    BIP_Status rc;

    BDBG_ENTER( BIP_IoChecker_Create );

    B_Mutex_Lock( pIoCheckerFactory->apiLock );

    BIP_CHECK_GOTO(( pSettings !=NULL ), ( "BIP_IoCheckerCreateSetting is NULL" ), error, BIP_ERR_INVALID_PARAMETER, rc );

    hIoChecker = createIoChecker(pSettings->fd);
    BIP_CHECK_GOTO(( hIoChecker !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hIoChecker->settings = pSettings->settings;
#if 0
    /* NOTE:This is moved to addFdToMonitorList function */
    /* Here we should not acquire ioCheckerLock since we may need to acquire POLLLock,
     * and ioChecker Lock can't be acquired before acquiring poll lock , that will lead to deadlock ,
     * since in ioWorkerThraed we acquire PollLock first and then before releasing POllLock we mutiple ,
     * time acquire and release ioCheckerLock for various operation.*/
    if (pIoCheckerFactory->numFdPolled  >= pIoCheckerFactory->numPfdAllocated)
    {
        int newSize;
        struct pollfd *tempPfd;

        newSize = pIoCheckerFactory->numPfdAllocated + BIP_NUM_POLL_FD_ALLOCATE;
        tempPfd = (struct pollfd *)B_Os_Malloc( newSize * sizeof( struct pollfd));
        BIP_CHECK_PTR_GOTO( tempPfd, "Memory Allocation Failed", error, BIP_ERR_OUT_OF_SYSTEM_MEMORY );

        /* This is the only place outside of pllling thread we need to acquire the pollLock,
         * we are changing the pfd structure and it can't be done when pfd structure
         * is in use by a ioWorkerThread.This is protected using PollLock.*/
        BDBG_ERR(("---------------------------------------------"));
        B_Mutex_Lock( pIoCheckerFactory->pollLock);
        BDBG_ERR(("+++++++++++++++++++++++++++++++++++++++++++++"));
        /*B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );*/
        /*BDBG_ERR(("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"));*/

        if (pIoCheckerFactory->pfd) {B_Os_Free( pIoCheckerFactory->pfd ); }
        /*everytime we increase the allocation by BIP_NUM_POLL_FD_ALLOCATE*/
        pIoCheckerFactory->pfd = tempPfd;
        BDBG_MSG(( "%s Pfd ReAllocated   oldSize = %d and newSize = %d ", __FUNCTION__, pIoCheckerFactory->numPfdAllocated, newSize ));
        pIoCheckerFactory->numPfdAllocated = newSize;
        /*B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );*/
        B_Mutex_Unlock( pIoCheckerFactory->pollLock);

        BDBG_ERR(("****************************************************"));
    }
#endif
     /* Once a ioElement is created update the num pfds count  and based on that reallocate
      * the pfd array if required.The reallocation happens in the addToMonitorList function
      * since it needs to be done in sync with all workerThreads by acquring the PollFd.
      * Also reduce the count once a fd is deleted.*/
    B_Mutex_Lock( pIoCheckerFactory->ioCheckerLock );
    BLST_Q_INSERT_TAIL( &pIoCheckerFactory->ioCheckerListHead , hIoChecker, ioCheckerNext );
    BDBG_MSG(("%s: hIoChecker = %p,fd = %d, Enable for Events = %d and refCount = %d \n",__FUNCTION__, (void *)hIoChecker, hIoChecker->fd,hIoChecker->eventMask, hIoChecker->refCount));
    pIoCheckerFactory->numFdPolled++;

    pIoCheckerFactory->totalNumFdsAdded++;
    B_Mutex_Unlock( pIoCheckerFactory->ioCheckerLock );

    B_Mutex_Unlock( pIoCheckerFactory->apiLock );

    BDBG_MSG(("%s: Created hIoChecker=%p for fd = %d", __FUNCTION__, (void *)hIoChecker, hIoChecker->fd));

    BDBG_LEAVE( BIP_IoChecker_Create );
    return(hIoChecker);
error:
    B_Mutex_Unlock( pIoCheckerFactory->apiLock );
    BDBG_LEAVE( BIP_IoChecker_Create );
    return(NULL);
}

void BIP_IoCheckerFactory_GetStatus(BIP_IoCheckerStatus *pStatus)
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;

    BDBG_ENTER( BIP_IoCheckerFactory_GetStatus );

    B_Mutex_Lock( pIoCheckerFactory->apiLock );


    pStatus->totalNumFdsAdded = pIoCheckerFactory->totalNumFdsAdded - 1;/* -1 since we don't want to inform the internal pipe fd which is always present */
    pStatus->totalNumFdsRemoved = pIoCheckerFactory->totalNumFdsRemoved;
    pStatus->currentNumFdsPolled = pIoCheckerFactory->currentNumFdsPolled - 1; /* -1 since we don't want to inform the internal pipe fd which is always polled */
    pStatus->maxNumFdsPolled = pIoCheckerFactory->maxNumFdsPolled;
    pStatus->numActiveFdsNotPolled = pIoCheckerFactory->numActiveFdsNotPolled;
    pStatus->numWorkerThreads = pIoCheckerFactory->initSettings.workerThreads;

    B_Mutex_Unlock( pIoCheckerFactory->apiLock );
    BDBG_LEAVE( BIP_IoCheckerFactory_GetStatus );
}

void BIP_IoCheckerFactory_Uninit( void )
{
    BIP_Status rc;

    BDBG_ENTER( BIP_IoCheckerFactory_Uninit );

    rc = pthread_mutex_lock(&g_initIoCheckerFactoryMutex);
    if ( rc )
    {
        BDBG_ERR(("%s:Can't Acquire GlobalInitLock", __FUNCTION__));
        BDBG_LEAVE( BIP_IoCheckerFactory_Uninit );
        return;
    }

    if (g_refIoCheckerFactoryCount == 0) {
        pthread_mutex_unlock(&g_initIoCheckerFactoryMutex);
        BDBG_ERR(("%s:IoCheckerFactory Not Initialized", __FUNCTION__));
        BDBG_LEAVE( BIP_IoCheckerFactory_Uninit );
        return;
    }

    if ( 0 == --g_refIoCheckerFactoryCount )
    {
        BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;

        BDBG_ASSERT( pIoCheckerFactory );

        BDBG_OBJECT_ASSERT( pIoCheckerFactory, BIP_IoCheckerFactory );

        BDBG_MSG(( "%s: pIoCheckerFactory %p", __FUNCTION__, (void *)pIoCheckerFactory ));

        ioCheckerUninit();

        BDBG_OBJECT_UNSET( &gBipIoCheckerFactoryCtx, BIP_IoCheckerFactory );
    }
    pthread_mutex_unlock(&g_initIoCheckerFactoryMutex);
    BDBG_LEAVE( BIP_IoCheckerFactory_Uninit );
}

/**
 * Summary:
 * API to initialize BIP IO Checker object.
 *
 * Description:
 *
 * This API initializes the factory that creates (and monitors) IoChecker objects.
 * Each IoChecker can monitor for various events (conditions) on a single
 * file descriptor (fd).  When a monitored condition occurs on an fd, the IoChecker's
 * callback is called.
 **/
BIP_Status BIP_IoCheckerFactory_Init(
    const BIP_IoCheckerFactoryInitSettings *pSettings /* Pass NULL for default setting.*/
    )
{
    BIP_IoCheckerFactory *pIoCheckerFactory = &gBipIoCheckerFactoryCtx;
    BIP_IoWorker *pIoWorker = NULL;
    BIP_Status rc = BIP_SUCCESS;
    BIP_IoCheckerHandle hIoChecker = NULL;
    int i;
    BDBG_ENTER( BIP_IoCheckerFactory_Init );

    rc = pthread_mutex_lock(&g_initIoCheckerFactoryMutex);
    if ( rc )
    {
        BDBG_LEAVE( BIP_IoCheckerFactory_Init );
        return B_ERROR_OS_ERROR;
    }
    if ( g_refIoCheckerFactoryCount > 0 )
    {
        g_refIoCheckerFactoryCount++;
        pthread_mutex_unlock(&g_initIoCheckerFactoryMutex);
        BDBG_LEAVE( BIP_IoCheckerFactory_Init );
        return(BIP_SUCCESS);
    }
    else
    {
        g_refIoCheckerFactoryCount++;
    }

    B_Os_Memset( pIoCheckerFactory, 0, sizeof( BIP_IoCheckerFactory ));

    BDBG_OBJECT_SET( pIoCheckerFactory, BIP_IoCheckerFactory );

    if(pSettings == NULL)
    {
        BIP_IoCheckerFactory_GetDefaultInitSettings(&pIoCheckerFactory->initSettings);
    }
    else
    {
        pIoCheckerFactory->initSettings = *pSettings;
        /* Validate psetting fields*/
        if(pSettings->workerThreads > BIP_MAX_IOWORKER)
        {
             pIoCheckerFactory->initSettings.workerThreads = BIP_MAX_IOWORKER;
            BDBG_WRN(("%s: Max number of worker threads can be %d, restricting to that value",__FUNCTION__, BIP_MAX_IOWORKER));
        }
    }

    pIoCheckerFactory->ioCheckerLock = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( pIoCheckerFactory->ioCheckerLock !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    pIoCheckerFactory->apiLock = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( pIoCheckerFactory->apiLock !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    pIoCheckerFactory->pollLock = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( pIoCheckerFactory->pollLock !=NULL ), ( "B_Mutex_Create Failed" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    /* Initially we allocate for BIP_NUM_POLL_FD_ALLOCATE(32)  size of poll fds*/
    pIoCheckerFactory->pfd = (struct pollfd *)B_Os_Malloc( BIP_NUM_POLL_FD_ALLOCATE * sizeof( struct pollfd));
    BIP_CHECK_GOTO(( pIoCheckerFactory->pfd ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    pIoCheckerFactory->numPfdAllocated = BIP_NUM_POLL_FD_ALLOCATE;
    pIoCheckerFactory->numFdPolled = 0;

    /* create ioWorker thread */
    for(i = 0; i < pIoCheckerFactory->initSettings.workerThreads ; i++ )
    {
        BIP_StringHandle hThreadName = NULL;
         /* Create the worker objects */
        pIoWorker = B_Os_Malloc( sizeof( BIP_IoWorker));
        BIP_CHECK_GOTO(( pIoWorker !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );
        BDBG_OBJECT_SET( pIoWorker, BIP_IoWorker );
        pIoWorker->stopThread = false;
        pIoWorker->threadIndex = i;
        hThreadName = BIP_String_CreateFromPrintf("BipIoWrkr-%d", i);
        pIoWorker->hIoCheckerThread = B_Thread_Create( BIP_String_GetString(hThreadName), (B_ThreadFunc)ioWorkerThread, pIoWorker, &pIoCheckerFactory->initSettings.threadSettings );
        BIP_String_Destroy(hThreadName);
        BIP_CHECK_GOTO(( pIoWorker->hIoCheckerThread  ), ( "Thread creation Failed" ), error, BIP_ERR_INTERNAL, rc );

        pIoCheckerFactory->pIoWorker[i] = pIoWorker;
    }

    /*TODO:Check whether we need to set any flag with pipe */
    rc = pipe(pIoCheckerFactory->pipeFd);
    BIP_CHECK_GOTO(( rc == 0 ), ( "Pipe creation Failed %d",rc ), error, BIP_ERR_OS_CHECK_ERRNO, rc );

    {
        BIP_IoCheckerCreateSetting pipeSetting;

        B_Os_Memset(&pipeSetting, 0, sizeof(BIP_IoCheckerCreateSetting));
        pipeSetting.fd =  pIoCheckerFactory->pipeFd[0];
        pipeSetting.settings.callBackFunction = pipeReadCallback;/* no callback is required for pipFd[0] */
        pipeSetting.settings.callBackContext = NULL;
        pipeSetting.settings.callBackParam = 0;

        /*Set pipe write nonBlocking */
        if(fcntl(pIoCheckerFactory->pipeFd[1], F_SETFL, O_NONBLOCK) == -1)
        {
            BDBG_ERR(("Call to fcntl failes while setting pipFd[1] to non_blocking mode"));
            rc = -1;
            goto error;
        }

        /* setting pepeFd[1] (write) to non_blocking mode since there is no check wther previous earlier writes are consumed or not ,
           we don't care whether it is overflowing, it is just for comminication, so we will always write to pipeFd irespective whether it is full or not ,
           by anychance if it is full we don't want it to block.*/
        rc = fcntl(pIoCheckerFactory->pipeFd[1], F_SETFL, O_NONBLOCK);
        BIP_CHECK_GOTO(( rc == 0 ), ( "fcntl failes while setting pipFd[1] to non_blocking mode" ), error, BIP_ERR_OS_CHECK_ERRNO, rc );


        hIoChecker = BIP_IoChecker_Create( &pipeSetting);
        BIP_CHECK_GOTO(( hIoChecker != NULL ), ( "BIP_IoCheckerFactory_Init Failed. Not able to create hIoChecker for pipFd" ), error, BIP_ERR_INTERNAL, rc );

        /* enable pipe for POLLIN Never call Enable api to enable pipe, since enable api writes into this pipe to enable an IoChecker.*/
        hIoChecker->eventMask = POLLIN;
    }

    rc = BIP_SUCCESS;
    pthread_mutex_unlock(&g_initIoCheckerFactoryMutex);
    BDBG_LEAVE( BIP_IoCheckerFactory_Init );
    return (rc);
error:
    pthread_mutex_unlock(&g_initIoCheckerFactoryMutex);
    BIP_IoCheckerFactory_Uninit();
    BDBG_LEAVE( BIP_IoCheckerFactory_Init );
    return(rc);
}
