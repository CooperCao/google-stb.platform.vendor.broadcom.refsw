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
VideoCore OS Abstraction Layer - low level thread support
=============================================================================*/

#ifndef VCOS_LOWLEVEL_THREAD_H
#define VCOS_LOWLEVEL_THREAD_H

#include "vcos_types.h"
#ifndef VCOS_PLATFORM_H
#include "vcos_platform.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 *
 * This defines a low level thread API that is supported by *some* operating systems
 * and can be used to construct the regular "joinable thread" API on those operating
 * systems.
 *
 * Most clients will not need to use this code.
 *
 * \sa vcos_joinable_thread.h
 */

/**
  * \brief Create a thread.
  *
  * This creates a thread which can be stopped either by returning from the
  * entry point function or by calling vcos_llthread_exit from within the entry
  * point function. The thread must be cleaned up by calling
  * vcos_llthread_delete. vcos_llthread_delete may or may not terminate the
  * thread.
  *
  * The preemptible parameter familiar from Nucleus is removed, as it is unused in
  *  VideoCore code. Affinity is added, since we do use this.
  *
  * @param thread       Filled in with thread instance
  * @param name         An optional name for the thread. "" may be used (but
  *                     a name will aid in debugging).
  * @param entry        Entry point
  * @param arg          A single argument passed to the entry point function
  * @param stack        Pointer to stack address
  * @param stacksz      Size of stack in bytes
  * @param priority     Priority of task, between VCOS_PRI_LOW and VCOS_PRI_HIGH
  * @param affinity     CPU affinity
  *
  * @sa vcos_llthread_terminate vcos_llthread_delete
  */
VCOS_STATUS_T vcos_llthread_create(VCOS_LLTHREAD_T *thread,
                                                      const char *name,
                                                      VCOS_LLTHREAD_ENTRY_FN_T entry,
                                                      void *arg,
                                                      void *stack,
                                                      VCOS_UNSIGNED stacksz,
                                                      VCOS_UNSIGNED priority,
                                                      VCOS_UNSIGNED affinity,
                                                      VCOS_UNSIGNED timeslice,
                                                      VCOS_UNSIGNED autostart);

/**
  * \brief Exits the current thread.
  */
void vcos_llthread_exit(void);

/**
  * \brief Delete a thread. This must be called to cleanup after
  * vcos_llthread_create. This may or may not terminate the thread.
  * It does not clean up any resources that may have been
  * allocated by the thread.
  */
void vcos_llthread_delete(VCOS_LLTHREAD_T *thread);

/**
  * \brief Return current lowlevel thread pointer.
  */
static inline VCOS_LLTHREAD_T *vcos_llthread_current(void);

/**
  * \brief Create a VCOS_LLTHREAD_T for the current thread. This is so we can
  * have VCOS_LLTHREAD_Ts even for threads not originally created by VCOS (eg
  * the thread that calls vcos_init).
  */
extern VCOS_STATUS_T _vcos_llthread_create_attach(VCOS_LLTHREAD_T *thread);

#ifdef __cplusplus
}
#endif
#endif
