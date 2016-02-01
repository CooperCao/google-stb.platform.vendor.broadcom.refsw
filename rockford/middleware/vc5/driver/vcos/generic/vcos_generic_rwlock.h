/*
 * Copyright (c) 2013 Broadcom. All rights reserved.
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
VideoCore OS Abstraction Layer - read/write lock implemented via semaphores
=============================================================================*/

#ifndef VCOS_GENERIC_RWLOCK_H
#define VCOS_GENERIC_RWLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VCOS_RWLOCK_T
{
   /* When zero the reader or writer lock is held */
   VCOS_SEMAPHORE_T rw_lock;
   /* Lock to hold to access readers */
   VCOS_SEMAPHORE_T read_lock;
   /* The number of read locks held */
   unsigned int readers;
} VCOS_RWLOCK_T;

VCOS_STATUS_T vcos_generic_rwlock_create(VCOS_RWLOCK_T *rw,
   const char *name);
void vcos_generic_rwlock_delete(VCOS_RWLOCK_T *rw);
VCOS_STATUS_T vcos_generic_rwlock_read_lock(
   VCOS_RWLOCK_T *rw);
VCOS_STATUS_T vcos_generic_rwlock_write_lock(
   VCOS_RWLOCK_T *rw);
void vcos_generic_rwlock_read_unlock(VCOS_RWLOCK_T *rw);
void vcos_generic_rwlock_write_unlock(VCOS_RWLOCK_T *rw);
VCOS_STATUS_T vcos_generic_rwlock_read_trylock(
   VCOS_RWLOCK_T *rw);
VCOS_STATUS_T vcos_generic_rwlock_write_trylock(
   VCOS_RWLOCK_T *rw);

static inline VCOS_STATUS_T vcos_rwlock_create(VCOS_RWLOCK_T *rw, const char *name) {
   return vcos_generic_rwlock_create(rw, name);
}

static inline void vcos_rwlock_delete(VCOS_RWLOCK_T *rw) {
   vcos_generic_rwlock_delete(rw);
}

static inline VCOS_STATUS_T vcos_rwlock_read_lock(VCOS_RWLOCK_T *rw) {
   return vcos_generic_rwlock_read_lock(rw);
}

static inline VCOS_STATUS_T vcos_rwlock_write_lock(VCOS_RWLOCK_T *rw) {
   return vcos_generic_rwlock_write_lock(rw);
}

static inline void vcos_rwlock_read_unlock(VCOS_RWLOCK_T *rw) {
   vcos_generic_rwlock_read_unlock(rw);
}

static inline void vcos_rwlock_write_unlock(VCOS_RWLOCK_T *rw) {
   vcos_generic_rwlock_write_unlock(rw);
}

static inline VCOS_STATUS_T vcos_rwlock_read_trylock(VCOS_RWLOCK_T *rw) {
   return vcos_generic_rwlock_read_trylock(rw);
}

static inline VCOS_STATUS_T vcos_rwlock_write_trylock(VCOS_RWLOCK_T *rw) {
   return vcos_generic_rwlock_write_trylock(rw);
}

#ifdef __cplusplus
}
#endif
#endif /* VCOS_GENERIC_RWLOCK_H */
