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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/HAL/src/bbHalSystemTimer.c $
*
* DESCRIPTION:
*   Hardware System Timer implementation.
*
* $Revision: 3357 $
* $Date: 2014-08-21 05:04:58Z $
*
*****************************************************************************************/

/************************* INCLUDES *****************************************************/
#include "bbSysTaskScheduler.h"
#include "bbSysTimeoutTask.h"
#include "private/bbHalTask.h"
#include "private/bbHalPrivateSystemTimer.h"    /* Hardware System Timer private interface. */

#if defined(__SoC__)
# include "bbSocSystemTimer.h"      /* SoC system timer interface. */
#
#elif defined(__ML507__)
# include "bbMl507SystemTimer.h"    /* ML507 system timer simulator interface. */
#
#else
# define ML507_SYSTEM_TIME_PRESCALER_LIMIT_MS  10
#
#endif


/************************* STATIC VARIABLES *********************************************/
/**//**
 * \brief Software 32-bit milliseconds system time counter.
 */
static HAL_SystemTimestamp_t halSystemTimeCounter;


/************************* INLINES ******************************************************/
/*************************************************************************************//**
  \brief
    Sets-up system clock prescaler and clears hardware system time prescaler counter.
*****************************************************************************************/
INLINE void halSystemTimeSetPrescaler(void)
{
#if defined(__SoC__)
    SOC_SystemTimeSetPrescaler();
#elif defined(__ML507__)
    ML507_SystemTimeSetPrescaler();

#endif
}


/*************************************************************************************//**
  \brief
    Returns the current timestamp according to the System Time Counter.
  \return
    The current timestamp according to the combination of the software and hardware System
    Time Counters, in milliseconds.
*****************************************************************************************/
INLINE HAL_SystemTimestamp_t halSystemTimestamp(void)
{
    HAL_SystemTimestamp_t timestamp;    /*!< System timestamp. */

    ATOMIC_SECTION_ENTER(SYS_ATOMIC_DEFAULT_UID)
    {
#if defined(__SoC__)
        timestamp = halSystemTimeCounter + SOC_SystemTimeGetPrescaler();

#elif defined(__ML507__)
        timestamp = halSystemTimeCounter + ML507_SystemTimeGetPrescaler();

#else /* __i386__ */
        timestamp = 0;

#endif
    }
    ATOMIC_SECTION_LEAVE(SYS_ATOMIC_DEFAULT_UID)

    return timestamp;
}


/************************* IMPLEMENTATION ***********************************************/
/*
 * Initializes the hardware System Timer.
 */
void HAL_SystemTimeInit(void)
{
#if defined(__SoC__)
    SOC_SystemTimer0SetHandler(halSystemTimeMatchHandler);
#endif
    halSystemTimeSetPrescaler();
    halSystemTimeCounter = 0;
}


/*
 * Returns the current timestamp according to the hardware System Time Counter.
 */
HAL_SystemTimestamp_t HAL_GetSystemTime(void)
{
    return halSystemTimestamp();
}


/*
 * Hardware System Timer Prescaler compare-match event handler.
 */
void halSystemTimeMatchHandler(void)
{
#if defined(__SoC__)
    halSystemTimeCounter += SOC_SYSTEM_TIME_PRESCALER_LIMIT_MS;
#elif defined(__ML507__)
    halSystemTimeCounter += ML507_SYSTEM_TIME_PRESCALER_LIMIT_MS;
#endif
    halPostTask(HAL_TIMER_TASK);
}


/*
 * Timer Task handler.
 */
void halSystemTimerTaskHandler(SYS_SchedulerTaskDescriptor_t *const notUsed)
{
    SYS_TimeoutTaskHandlerTick();

    (void)notUsed;
}


/* eof bbHalSystemTimer.c */