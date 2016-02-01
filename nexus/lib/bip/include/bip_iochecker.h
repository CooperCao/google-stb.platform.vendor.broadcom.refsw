/******************************************************************************
 * (c) 2007-2014 Broadcom Corporation
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
#ifndef BIP_IOCHECKER_H
#define BIP_IOCHECKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/poll.h>
/*#include "bip.h"*/
typedef struct BIP_IoChecker *BIP_IoCheckerHandle;

#define BIP_MAX_IOWORKER    16

#define BIP_DEFAULT_IOWORKER 4

/**
 * Summary:
 * BIP_IoCheckerFactoryInitSettings: it specifies the
 * number of FactoryWorker threads to be created.
 *
 * Description:
 * IoChecker can only support upto BIP_MAX_IOWORKER number of
 * threads. It also specifies the  B_ThreadSettings.
 **/
typedef struct BIP_IoCheckerFactoryInitSettings
{
    int workerThreads; /* number of the I/O worker threads, defaults to BIP_DEFAULT_IOWORKER */
    B_ThreadSettings threadSettings; /* Priority & Stack size settings for IoChecker Thread */
} BIP_IoCheckerFactoryInitSettings;

/**
 * Summary:
 * IoChecker events (eonditions) that can be monitored.
 **/
typedef enum BIP_IoCheckerEvent
{
    /* There is data to read */
    BIP_IoCheckerEvent_ePollIn = POLLIN,

     /* There is urgent data to read */
    BIP_IoCheckerEvent_ePollPri = POLLPRI,

     /* writing will not block */
    BIP_IoCheckerEvent_ePollOut = POLLOUT,

    /* Stream socket peer closed connection,
       or shut down writing ,half offf connection. */
    BIP_IoCheckerEvent_ePollRdHup = POLLRDHUP,

    /* PollERR condition : Output Only */
    BIP_IoCheckerEvent_ePollError = POLLERR,

    /* Poll Hang Up : Output Only */
    BIP_IoCheckerEvent_ePollHup = POLLHUP,

    /* Poll Invalid request: Output only */
    BIP_IoCheckerEvent_ePollNvall = POLLNVAL

}BIP_IoCheckerEvent;

typedef struct BIP_IoCheckerStatus
{
    /* number of fd's added to IoCheckerFactory till now */
    int totalNumFdsAdded;

    /* number of Fds removed till now from IoCheckerFactory */
    int totalNumFdsRemoved;

    /* number of Fds currently polled.*/
    int currentNumFdsPolled;

    /* max number of Fds polled */
    int maxNumFdsPolled;

    /* number of worker threads being used by IoChecker */
    int numWorkerThreads;

    /* number of active fds not polled since they are either busy or disabled.*/
    int numActiveFdsNotPolled;

} BIP_IoCheckerStatus;


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
    );

/**
 * Summary:
 * API to initialize BIP IO Checker object.
 *
 * Description:
 *
 * This API initializes the factory that creates (and monitors) IoChecker objects.
 * Each IoChecker can be monitored for various events
 * (conditions) on a single file descriptor (fd).  When a
 * monitored condition occurs on an fd, the IoChecker's callback
 * is called.
 **/
BIP_Status BIP_IoCheckerFactory_Init(
    const BIP_IoCheckerFactoryInitSettings *pSettings /* Pass NULL for default setting.*/
    );

/**
 * Summary:
 * API to shut down the IoCheckerFactory any existing IoCheckers will
 * be destroyed and their handles will become invalid.
 *
 * Description:
 **/
void BIP_IoCheckerFactory_Uninit(void);

/**
 * Summary:
 * API to to get IoCheckerFactory status.
 *
 * Description:
 **/
void BIP_IoCheckerFactory_GetStatus(
    BIP_IoCheckerStatus *pStatus
    );

/**
 * Summary:
 * When an enabled IoCheckerEvent occurs, IoChecker will call ioCheckerCallback.
 *
 * Description:
 * In this callBack, eventMask will return the set of IoEvents.
 **/
typedef void (*BIP_IoCheckerCallback)(void *context, int param, BIP_IoCheckerEvent eventMask);

typedef struct BIP_IoCheckerSettings
{
    BIP_IoCheckerCallback     callBackFunction;
    int                       callBackParam;
    void                     *callBackContext;
}BIP_IoCheckerSettings;

typedef struct BIP_IoCheckerCreateSetting
{
    int fd;
    BIP_IoCheckerSettings  settings;
}BIP_IoCheckerCreateSetting;

void BIP_IoChecker_GetDefaultCreateSettings(
    BIP_IoCheckerCreateSetting *pSettings
    );

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
    );

/**
 * Summary:
 * API to Get IoChecker Settings
 *
 * Description:
 **/
BIP_Status BIP_IoChecker_GetSettings(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerSettings *pSettings
    );

/**
 * Summary:
 * API to Set IoChecker Settings
 *
 * Description:
 **/
BIP_Status BIP_IoChecker_SetSettings(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerSettings *pSettings
    );

/**
 * Summary:
 * API to enable IoChecker.
 *
 * Description:
 * API enables event/events specified in eventMask .
 * BIP_IoChecker_Enable will enable event/events for IoCheckers
 * created using a common Fd.
 **/
BIP_Status BIP_IoChecker_Enable(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerEvent eventMask
    );

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
    );

/**
 * Summary:
 * API to destroy hIoChecker.
 *
 * Description:
 * */
void BIP_IoChecker_Destroy(
    BIP_IoCheckerHandle hIoChecker
    );

/**
 * Summary:
 * This API instantly check event/events specified in eventMask
 * for an Fd. It returns a bit mask for all the set IO events.
 **/
int BIP_Fd_CheckNow(
    int fd,
    BIP_IoCheckerEvent eventMask
    );

/**
 * Summary:
 * This API instantly check event/events specified in
 * eventMask for a hIoChecker. It returns a bit mask for all the
 * set IO events.
 **/
int BIP_IoChecker_CheckNow(
    BIP_IoCheckerHandle hIoChecker,
    BIP_IoCheckerEvent eventMask
    );

/**
 * Summary:
 * API to retrieve IoChecker Status.
 **/
#if 0
BIP_Status BIP_IoChecker_GetStatus( BIP_IoCheckerHandle fd, BIP_IoCheckerStatus *pStatus );
#endif

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_IOCHECKER_H */
