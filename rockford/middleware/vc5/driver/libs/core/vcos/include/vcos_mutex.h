/*
 * Broadcom Proprietary and Confidential. (c)2010-2011 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or
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
VideoCore OS Abstraction Layer - mutex public header file
=============================================================================*/

#ifndef VCOS_MUTEX_H
#define VCOS_MUTEX_H

#include "vcos_types.h"
#include "vcos_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file vcos_mutex.h
 *
 * Mutex API. Mutexes are not re-entrant, as supporting this adds extra code
 * that slows down clients which have been written sensibly.
 *
 * \sa vcos_reentrant_mutex.h
 *
 */

/** Create a mutex.
  *
  * @param m      Filled in with mutex on return
  * @param name   A non-null name for the mutex, used for diagnostics.
  *
  * @return VCOS_SUCCESS if mutex was created, or error code.
  */
static inline VCOS_STATUS_T vcos_mutex_create(VCOS_MUTEX_T *m, const char *name);

/** Delete the mutex.
  */
static inline void vcos_mutex_delete(VCOS_MUTEX_T *m);

/**
  * \brief Wait to claim the mutex.
  *
  * On most platforms this always returns VCOS_SUCCESS, and so would ideally be
  * a void function, however some platforms allow a wait to be interrupted so
  * it remains non-void.
  *
  * Try to obtain the mutex.
  * @param m   Mutex to wait on
  * @return VCOS_SUCCESS - mutex was taken.
  *         VCOS_EAGAIN  - could not take mutex.
  */
#ifndef vcos_mutex_lock
static inline VCOS_STATUS_T vcos_mutex_lock(VCOS_MUTEX_T *m);

/** Release the mutex.
  */
static inline void vcos_mutex_unlock(VCOS_MUTEX_T *m);
#endif

/** Test if the mutex is already locked.
  *
  * @return 1 if mutex is locked, 0 if it is unlocked.
  */
static inline int vcos_mutex_is_locked(VCOS_MUTEX_T *m);

/** Obtain the mutex if possible.
  *
  * @param m  the mutex to try to obtain
  *
  * @return VCOS_SUCCESS if mutex is succesfully obtained, or VCOS_EAGAIN
  * if it is already in use by another thread.
  */
#ifndef vcos_mutex_trylock
static inline VCOS_STATUS_T vcos_mutex_trylock(VCOS_MUTEX_T *m);
#endif


#ifdef __cplusplus
}
#endif

#endif
