/*
 * Copyright (c) 2010-2011 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom Corporation and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use
 *    all reasonable efforts to protect the confidentiality thereof, and to
 *    use this information only in connection with your use of Broadcom
 *    integrated circuit products.
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
 *    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
 *    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
 *    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 */

/*=============================================================================
VideoCore OS Abstraction Layer - timer support
=============================================================================*/

#ifndef VCOS_TIMER_H
#define VCOS_TIMER_H

#include "vcos_types.h"
#ifndef VCOS_PLATFORM_H
#include "vcos_platform.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** \file vcos_timer.h
  *
  * Timers are single shot.
  *
  * Timer times are in milliseconds.
  *
  * \note that timer callback functions are called from an arbitrary thread
  * context. The expiration function should do its work as quickly as possible;
  * blocking should be avoided.
  *
  * \note On Windows, the separate function vcos_timer_init() must be called
  * as timer initialization from DllMain is not possible.
  */

/** Perform timer subsystem initialization. This function is not needed
  * on non-Windows platforms but is still present so that it can be
  * called. On Windows it is needed because vcos_init() gets called
  * from DLL initialization where it is not possible to create a
  * time queue (deadlock occurs if you try).
  *
  * @return VCOS_SUCCESS on success. VCOS_EEXIST if this has already been called
  * once. VCOS_ENOMEM if resource allocation failed.
  */
VCOS_STATUS_T vcos_timer_init(void);

/** Create a timer in a disabled state.
  *
  * The timer is initially disabled.
  *
  * @param timer     timer handle
  * @param name      name for timer
  * @param expiration_routine function to call when timer expires
  * @param context   context passed to expiration routine
  *
  */
static inline VCOS_STATUS_T vcos_timer_create(VCOS_TIMER_T *timer,
                                const char *name,
                                void (*expiration_routine)(void *context),
                                void *context);



/** Start a timer running.
  *
  * Timer must be stopped.
  *
  * @param timer     timer handle
  * @param delay     Delay to wait for, in ms
  */
static inline void vcos_timer_set(VCOS_TIMER_T *timer, VCOS_UNSIGNED delay);

/** Stop an already running timer.
  *
  * @param timer     timer handle
  */
static inline void vcos_timer_cancel(VCOS_TIMER_T *timer);

/** Stop a timer and restart it.
  * @param timer     timer handle
  * @param delay     delay in ms
  */
static inline void vcos_timer_reset(VCOS_TIMER_T *timer, VCOS_UNSIGNED delay);

static inline void vcos_timer_delete(VCOS_TIMER_T *timer);

#ifdef __cplusplus
}
#endif
#endif
