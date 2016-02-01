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
VideoCore OS Abstraction Layer - common postamble code
=============================================================================*/

/** \file
  *
  * Postamble code included by the platform-specific header files
  */

#define VCOS_THREAD_PRI_DEFAULT VCOS_THREAD_PRI_NORMAL

#if !defined(VCOS_THREAD_PRI_INCREASE)
#error Which way to thread priorities go?
#endif

#if VCOS_THREAD_PRI_INCREASE < 0
/* smaller numbers are higher priority */
#define VCOS_THREAD_PRI_LESS(x) ((x)<VCOS_THREAD_PRI_MAX?(x)+1:VCOS_THREAD_PRI_MAX)
#define VCOS_THREAD_PRI_MORE(x) ((x)>VCOS_THREAD_PRI_MIN?(x)-1:VCOS_THREAD_PRI_MIN)
#else
/* bigger numbers are lower priority */
#define VCOS_THREAD_PRI_MORE(x) ((x)<VCOS_THREAD_PRI_MAX?(x)+1:VCOS_THREAD_PRI_MAX)
#define VCOS_THREAD_PRI_LESS(x) ((x)>VCOS_THREAD_PRI_MIN?(x)-1:VCOS_THREAD_PRI_MIN)
#endif

/* Convenience for Brits: */
#define VCOS_APPLICATION_INITIALISE VCOS_APPLICATION_INITIALIZE

/*
 * Check for constant definitions
 */
#ifndef VCOS_TICKS_PER_SECOND
#error VCOS_TICKS_PER_SECOND not defined
#endif

#if !defined(VCOS_THREAD_PRI_MIN) || !defined(VCOS_THREAD_PRI_MAX)
#error Priority range not defined
#endif

#if !defined(VCOS_THREAD_PRI_HIGHEST) || !defined(VCOS_THREAD_PRI_LOWEST) || !defined(VCOS_THREAD_PRI_NORMAL)
#error Priority ordering not defined
#endif

#if !defined(VCOS_CAN_SET_STACK_ADDR)
#error Can stack addresses be set on this platform? Please set this macro to either 0 or 1.
#endif

#if (_VCOS_AFFINITY_CPU0|_VCOS_AFFINITY_CPU1) & (~_VCOS_AFFINITY_MASK)
#error _VCOS_AFFINITY_CPUxxx values are not consistent with _VCOS_AFFINITY_MASK
#endif

#ifndef VCOS_HAVE_TIMER
void vcos_timer_init(void);
#endif
