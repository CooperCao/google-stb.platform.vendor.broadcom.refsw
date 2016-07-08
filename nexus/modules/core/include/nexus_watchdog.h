/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef NEXUS_WATCHDOG_H__
#define NEXUS_WATCHDOG_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_WATCHDOG_MAX_TIMEOUT 159 /* seconds  (2^32 / 27 MHz) */

#ifndef NEXUS_WATCHDOG_MIN_TIMEOUT     /* override possible but not recommended */
#define NEXUS_WATCHDOG_MIN_TIMEOUT 10  /* seconds */
#endif

/***************************************************************************
Summary:
Sets the HW watchdog timer timeout value.

Description:
This function sets the HW watchdog timeout value, in seconds. The timeout
value should be set before calling NEXUS_Watchdog_StartTimer.
A new timeout value is ignored until the next call to NEXUS_Watchdog_StartTimer.

Timeout values less than 10s may conflict with GISB bus timeouts and are not recommended without further discussion with Broadcom.

See NEXUS_WATCHDOG_MIN_TIMEOUT and NEXUS_WATCHDOG_MAX_TIMEOUT for enforced limits.
***************************************************************************/
NEXUS_Error NEXUS_Watchdog_SetTimeout(
    unsigned timeout    /* timeout value in seconds */
    );

/***************************************************************************
Summary:
Starts the HW watchdog timer.

Description:
Aftering starting the watchdog, the application is responsible to monitor system health.
If it believes its system is good it should call NEXUS_Watchdog_StartTimer again, before the timeout, to
restart the watchdog timer and keep the system running.
If it believes its system has gone bad or if the software to monitor health is not executed, then it should not
or simply does not call NEXUS_Watchdog_StartTimer again. This will cause the watchdog timer to expire, and
then the chip will be automatically reset.

An application can use a variety of criteria to determine if its system is still alive, including
normal OS scheduling, harddrive or other peripheral communication, etc.

See Also:
NEXUS_Watchdog_SetTimeout
NEXUS_Watchdog_StopTimer
***************************************************************************/
NEXUS_Error NEXUS_Watchdog_StartTimer(void);

/***************************************************************************
Summary:
Stops the HW watchdog timer.

Description:
Stopping the timer does not reset the timeout value.
***************************************************************************/
NEXUS_Error NEXUS_Watchdog_StopTimer(void);

/***************************************************************************
Summary:
This function returns pStatus = true if the last chip reset was triggered by the watchdog timer.

Description:
This function does not work on ARM systems with the BOLT bootloader.
BOLT reads and clears reset status, so this function will always return false.
The reset status captured by BOLT can be retrieved from Device Tree as follows:

    cat /proc/device-tree/bolt/reset-list

***************************************************************************/
void NEXUS_Watchdog_GetLastResetStatus(
    bool *pStatus /* [out] */
    );

typedef struct NEXUS_WatchdogCallback *NEXUS_WatchdogCallbackHandle;

typedef struct NEXUS_WatchdogCallbackSettings
{
    NEXUS_CallbackDesc midpointCallback; /* callback is fired when watchdog is at the midpoint. chip is reset when watchdog expires. */
} NEXUS_WatchdogCallbackSettings;

void NEXUS_WatchdogCallback_GetDefaultSettings(
    NEXUS_WatchdogCallbackSettings *pSettings
    );

/**
Summary:
Create a callback which fires at the midpoint of the watchdog.
In the callback you can pet the watchdog by calling StartTimer again.
If the callback does not fire, something has gone wrong inside Nexus, the watchdog will expire and the system will reset.
**/
NEXUS_WatchdogCallbackHandle NEXUS_WatchdogCallback_Create( /* attr{destructor=NEXUS_WatchdogCallback_Destroy} */
    const NEXUS_WatchdogCallbackSettings *pSettings
    );

/**
Summary:
**/
void NEXUS_WatchdogCallback_Destroy(
    NEXUS_WatchdogCallbackHandle handle
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_WATCHDOG_H__ */

