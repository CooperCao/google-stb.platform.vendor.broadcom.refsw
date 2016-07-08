/*
 * Broadcom Proprietary and Confidential. (c)2013 Broadcom.  All rights reserved.
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
VideoCore OS Abstraction Layer - rwlock public header file
=============================================================================*/

#ifndef VCOS_RWLOCK_H
#define VCOS_RWLOCK_H

#include "vcos_types.h"
#include "vcos_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#if VCOS_HAVE_RWLOCK

/**
 * \file vcos_rwlock.h
 *
 * RWLock API. RWLocks are not re-entrant, as supporting this adds extra code
 * that slows down clients which have been written sensibly.
 *
 */

/** Create an rwlock.
  *
  * @param rw     Filled in with an rwlok on return
  * @param name   A non-null name for the rwlock, used for diagnostics.
  *
  * @return VCOS_SUCCESS if an rwlock was created, or error code.
  */
static inline VCOS_STATUS_T vcos_rwlock_create(VCOS_RWLOCK_T *rw, const char *name);

/** Delete the rwlock.
  */
static inline void vcos_rwlock_delete(VCOS_RWLOCK_T *rw);

/**
  * \brief Wait to claim the rwlock for reading.
  *
  * On most platforms this always returns VCOS_SUCCESS, and so would ideally be
  * a void function, however some platforms allow a wait to be interrupted so
  * it remains non-void.
  *
  * Try to obtain the rwlock for reading.
  * @param rw  rwlock to wait on
  * @return VCOS_SUCCESS - rwlock was taken.
  *         VCOS_EAGAIN  - could not take rwlock.
  */
#ifndef vcos_rwlock_read_lock
static inline VCOS_STATUS_T vcos_rwlock_read_lock(VCOS_RWLOCK_T *rw);

/**
  * \brief Wait to claim the rwlock for writing.
  *
  * On most platforms this always returns VCOS_SUCCESS, and so would ideally be
  * a void function, however some platforms allow a wait to be interrupted so
  * it remains non-void.
  *
  * Try to obtain the rwlock for writing.
  * @param rw  rwlock to wait on
  * @return VCOS_SUCCESS - rwlock was taken.
  *         VCOS_EAGAIN  - could not take rwlock.
  */
static inline VCOS_STATUS_T vcos_rwlock_write_lock(VCOS_RWLOCK_T *rw);

/** Release the read rwlock.
  */
static inline void vcos_rwlock_read_unlock(VCOS_RWLOCK_T *rw);

/** Release the write rwlock.
  */
static inline void vcos_rwlock_write_unlock(VCOS_RWLOCK_T *rw);
#endif

/** Obtain the rwlock for reading if possible.
  *
  * @param rw  the rwlock to try to obtain
  *
  * @return VCOS_SUCCESS if the rwlock is successfully obtained, or VCOS_EAGAIN
  * if it is held for writing by another thread.
  */
#ifndef vcos_rwlock_read_trylock
static inline VCOS_STATUS_T vcos_rwlock_read_trylock(VCOS_RWLOCK_T *rw);

/** Obtain the rwlock for writing if possible.
  *
  * @param rw  the rwlock to try to obtain
  *
  * @return VCOS_SUCCESS if the rwlock is succesfully obtained, or VCOS_EAGAIN
  * if it is already in use by another thread.
  */
static inline VCOS_STATUS_T vcos_rwlock_write_trylock(VCOS_RWLOCK_T *rw);
#endif

#endif

#ifdef __cplusplus
}
#endif
#endif
