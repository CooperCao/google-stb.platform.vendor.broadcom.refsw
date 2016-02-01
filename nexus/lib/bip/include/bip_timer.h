/******************************************************************************
 * (c) 2014 Broadcom Corporation
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

#ifndef BIP_TIMER_H
#define BIP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bip_types.h"

typedef struct B_SchedulerTimer *  BIP_TimerHandle;
/**
 * Summary:
 * BIP Timer APIs
 *
 * Description:
 * Abstraction and simplification of the B_Scheduler_XxxTimer() APIs.
 **/

/**
 * Summary:
 * API to initialize the BIP_TimerFactory that creates and destroys the BIP_Timers.
 *
 * Description:
 *
 * Called once at BIP_Initialization.
 **/
typedef struct BIP_TimerFactoryInitSettings
{
    int unused;
}BIP_TimerFactoryInitSettings;

void BIP_TimerFactory_GetDefaultInitSettings(
    BIP_TimerFactoryInitSettings *pSettings
    );

BIP_Status  BIP_TimerFactory_Init(BIP_TimerFactoryInitSettings *pSettings);

/**
 * Summary:
 * API to uninitialize (shutdown) the BIP_TimerFactory.
 *
 * Description:
 *
 * Called once at BIP_Uninitialization (shutdown).
 **/

void BIP_TimerFactory_Uninit(void);

/**
 * Summary:
 * BIP_TimerCallback definition.
 *
 * Description:
 *
 * The callback that is called when the BIP_Timer expires.
 **/

/* Timer callback */
typedef void (*BIP_TimerCallback)(void *pContext);

/**
 * Summary:
 * API to create a BIP_Timer.
 *
 * Description:
 *
 * A BIP_Timer is a one-shot timer that is started when it is created.  After it's time
 * has expired, it's callback is called, then the timer is self-destructed. Applications
 * should insure that they do not try to Destroy a BIP_Timer after it has expired.
 **/
typedef struct
{
    struct
    {
        int                 timeoutInMs;
        BIP_TimerCallback   callback;
        void               *pContext;
    } input;

} BIP_TimerCreateSettings;

void BIP_Timer_GetDefaultCreateSettings(BIP_TimerCreateSettings *pSettings);

BIP_TimerHandle BIP_Timer_Create(BIP_TimerCreateSettings *pSettings);

/**
 * Summary:
 * API to destroy a BIP_Timer.
 *
 * Description:
 *
 * If a BIP_Timer is running but is no longer needed, the following API can be used to
 * destroy it.  Timers that expire self-destruct and should not be destroyed.
 **/
void BIP_Timer_Destroy(BIP_TimerHandle  hTimer);

/* ********************************************************************************************** */
#ifdef __cplusplus
}
#endif

#endif /* BIP_TIMER_H */
