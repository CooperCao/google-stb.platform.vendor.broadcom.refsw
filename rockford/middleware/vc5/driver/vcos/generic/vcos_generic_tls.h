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
VideoCore OS Abstraction Layer - generic thread local storage
=============================================================================*/

#ifndef VCOS_GENERIC_TLS_H
#define VCOS_GENERIC_TLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vcos_types.h"

/**
  * \file
  *
  * Do an emulation of Thread Local Storage. The platform needs to
  * provide a way to set and get a per-thread pointer which is
  * where the TLS data itself is stored.
  *
  *
  * Each thread that wants to join in this scheme needs to call
  * vcos_tls_thread_register().
  *
  * The platform needs to support the macros/functions
  * _vcos_tls_thread_ptr_set() and _vcos_tls_thread_ptr_get().
  */

#ifndef VCOS_WANT_TLS_EMULATION
#error Should not be included unless TLS emulation is defined
#endif

/** Number of slots to reserve per thread. This results in an overhead
  * of this many words per thread.
  */
#define VCOS_TLS_MAX_SLOTS 4

/** TLS key. Allocating one of these reserves the client one of the
  * available slots.
  */
typedef VCOS_UNSIGNED VCOS_TLS_KEY_T;

/** TLS per-thread structure. Each thread gets one of these
  * if TLS emulation (rather than native TLS support) is
  * being used.
  */
typedef struct VCOS_TLS_THREAD_T
{
   void *slots[VCOS_TLS_MAX_SLOTS];
} VCOS_TLS_THREAD_T;

/*
 * Internal APIs
 */

/** Register this thread's TLS storage area. */
void vcos_tls_thread_register(VCOS_TLS_THREAD_T *);

/** Create a new TLS key */
VCOS_STATUS_T vcos_generic_tls_create(VCOS_TLS_KEY_T *key, void(*destructor)(void *));

/** Delete a TLS key */
void vcos_generic_tls_delete(VCOS_TLS_KEY_T tls);

/** Initialise the TLS library */
VCOS_STATUS_T vcos_tls_init(void);

/** Deinitialise the TLS library */
void vcos_tls_deinit(void);

#undef VCOS_ASSERT_LOGGING_DISABLE
#define VCOS_ASSERT_LOGGING_DISABLE 1

/*
 * Implementations of public API functions
 */

/** Set the given value. Since everything is per-thread, there is no need
  * for any locking.
  */
static inline VCOS_STATUS_T vcos_tls_set(VCOS_TLS_KEY_T tls, void *v) {
   VCOS_TLS_THREAD_T *tlsdata = _vcos_tls_thread_ptr_get();
   assert(tlsdata); /* Fires if this thread has not been registered */
   if (tls<VCOS_TLS_MAX_SLOTS)
   {
      tlsdata->slots[tls] = v;
      return VCOS_SUCCESS;
   }
   else
   {
      assert(0);
      return VCOS_EINVAL;
   }
}

/** Get the given value. No locking required.
  */
static inline void *vcos_tls_get(VCOS_TLS_KEY_T tls) {
   VCOS_TLS_THREAD_T *tlsdata = _vcos_tls_thread_ptr_get();
   assert(tlsdata); /* Fires if this thread has not been registered */
   if (tls<VCOS_TLS_MAX_SLOTS)
   {
      return tlsdata->slots[tls];
   }
   else
   {
      assert(0);
      return NULL;
   }
}

static inline VCOS_STATUS_T vcos_tls_create(VCOS_TLS_KEY_T *key, void(*destructor)(void *))
{
   return vcos_generic_tls_create(key, destructor);
}

static inline void vcos_tls_delete(VCOS_TLS_KEY_T tls) {
   vcos_generic_tls_delete(tls);
}

#undef VCOS_ASSERT_LOGGING_DISABLE
#define VCOS_ASSERT_LOGGING_DISABLE 0

#ifdef __cplusplus
}
#endif

#endif
