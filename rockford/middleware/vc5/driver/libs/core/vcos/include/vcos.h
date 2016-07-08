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
VideoCore OS Abstraction Layer - public header file
=============================================================================*/

/**
  * \mainpage OS Abstraction Layer
  *
  * \section intro Introduction
  *
  * This abstraction layer is here to allow the underlying OS to be easily changed (e.g. from
  * Nucleus to ThreadX) and to aid in porting host applications to new targets.
  *
  * \subsection error Error handling
  *
  * Wherever possible, VCOS functions assert internally and return void. The only exceptions
  * are creation functions (which might fail due to lack of resources) and functions that
  * might timeout or fail due to lack of space. Errors that might be reported by the underlying
  * OS API (e.g. invalid mutex) are treated as a programming error, and are merely asserted on.
  *
  * \section thread_synch Threads and synchronisation
  *
  * \subsection thread Threads
  *
  * The thread API is somewhat different to that found in Nucleus. In particular, threads
  * cannot just be destroyed at arbitrary times and nor can they merely exit. This is so
  * that the same API can be implemented across all interesting platforms without too much
  * difficulty. See vcos_thread.h for details. Thread attributes are configured via
  * the VCOS_THREAD_ATTR_T structure, found in vcos_thread_attr.h.
  *
  * \subsection sema Semaphores
  *
  * Counted semaphores (c.f. Nucleus NU_SEMAPHORE) are created with VCOS_SEMAPHORE_T.
  * Under ThreadX on VideoCore, semaphores are implemented using VideoCore spinlocks, and
  * so are quite a lot faster than ordinary ThreadX semaphores. See vcos_semaphore.h.
  *
  * \subsection mtx Mutexes
  *
  * Mutexes are used for locking. Attempts to take a mutex twice, or to unlock it
  * in a different thread to the one in which it was locked should be expected to fail.
  * Mutexes are not re-entrant (see vcos_reentrant_mutex.h for a slightly slower
  * re-entrant mutex). VCOS_MUTEX_T may not be fair. If you want a fair mutex, use
  * VCOS_FAIR_MUTEX_T.
  *
  * \subsection event Events
  *
  * A VCOS_EVENT_T is a bit like a saturating semaphore. No matter how many times it
  * is signalled, the waiter will only wake up once. See vcos_event.h. You might think this
  * is useful if you suspect that the cost of reading the semaphore count (perhaps via a
  * system call) is expensive on your platform.
  *
  * \subsection tls Thread local storage
  *
  * Thread local storage is supported using vcos_tls.h. This is emulated on Nucleus
  * and ThreadX.
  *
  */

/**
  * \file vcos.h
  *
  * This is the top level header file. Clients include this. It pulls in the platform-specific
  * header file (vcos_platform.h) together with header files defining the expected APIs, such
  * as vcos_mutex.h, vcos_semaphore.h, etc. It is also possible to include these header files
  * directly.
  *
  */

#ifndef VCOS_H
#define VCOS_H

#include "libs/util/assert_helpers.h"
#include "vcos_types.h"
#include "vcos_platform.h"

#ifndef VCOS_INIT_H
#include "vcos_init.h"
#endif

#ifndef VCOS_SEMAPHORE_H
#include "vcos_semaphore.h"
#endif

#ifndef VCOS_THREAD_H
#include "vcos_thread.h"
#endif

#ifndef VCOS_MUTEX_H
#include "vcos_mutex.h"
#endif

#ifndef VCOS_FAIR_MUTEX_H
#include "vcos_fair_mutex.h"
#endif

#ifndef VCOS_RWLOCK_H
#include "vcos_rwlock.h"
#endif

#ifndef VCOS_PROP_H
#include "vcos_properties.h"
#endif

#ifndef VCOS_STRING_H
#include "vcos_string.h"
#endif

#ifndef VCOS_EVENT_H
#include "vcos_event.h"
#endif

#ifndef VCOS_THREAD_ATTR_H
#include "vcos_thread_attr.h"
#endif

#ifndef VCOS_TLS_H
#include "vcos_tls.h"
#endif

#ifndef VCOS_REENTRANT_MUTEX_H
#include "vcos_reentrant_mutex.h"
#endif

#ifndef VCOS_HAVE_MODULE_H
#include "vcos_module.h"
#endif

/* Headers with predicates */

#if VCOS_HAVE_TIMER
#include "vcos_timer.h"
#endif

#if VCOS_HAVE_ATOMIC_FLAGS
#include "vcos_atomic_flags.h"
#endif

#if VCOS_HAVE_ONCE
#include "vcos_once.h"
#endif

#endif /* VCOS_H */
