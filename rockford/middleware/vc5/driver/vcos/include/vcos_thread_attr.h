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
VideoCore OS Abstraction Layer - thread attributes
=============================================================================*/

#ifndef VCOS_THREAD_ATTR_H
#define VCOS_THREAD_ATTR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 *
 * Attributes for thread creation.
 *
 */

/** Initialize thread attribute struct. This call does not allocate memory,
  * and so cannot fail.
  *
  */
void vcos_thread_attr_init(VCOS_THREAD_ATTR_T *attrs);

/** Set the stack address and size. If not set, a stack will be allocated automatically.
  *
  * This can only be set on some platforms. It will always be possible to set the stack
  * address on VideoCore, but on host platforms, support may well not be available.
  */
#if VCOS_CAN_SET_STACK_ADDR
static inline void vcos_thread_attr_setstack(VCOS_THREAD_ATTR_T *attrs, void *addr, VCOS_UNSIGNED sz);
#endif

/** Set the stack size. If not set, a default size will be used. Attempting to call this after having
  * set the stack location with vcos_thread_attr_setstack() will result in undefined behaviour.
  */
static inline void vcos_thread_attr_setstacksize(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED sz);

/** Set the task priority. If not set, a default value will be used.
  */
static inline void vcos_thread_attr_setpriority(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED pri);

/** Set the task cpu affinity. If not set, the default will be used.
  */
static inline void vcos_thread_attr_setaffinity(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED aff);

/** Set the timeslice. If not set the default will be used.
  */
static inline void vcos_thread_attr_settimeslice(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED ts);

/** The thread entry function takes (argc,argv), as per Nucleus, with
  * argc being 0. This may be withdrawn in a future release and should not
  * be used in new code.
  */
static inline void _vcos_thread_attr_setlegacyapi(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED legacy);

static inline void vcos_thread_attr_setautostart(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED autostart);

#ifdef __cplusplus
}
#endif
#endif
