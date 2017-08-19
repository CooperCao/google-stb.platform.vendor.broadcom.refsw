/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
  * re-entrant mutex). VCOS_MUTEX_T may not be fair!
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

#ifndef VCOS_PROP_H
#include "vcos_properties.h"
#endif

#ifndef VCOS_STRING_H
#include "vcos_string.h"
#endif

#ifndef VCOS_EVENT_H
#include "vcos_event.h"
#endif

#ifndef VCOS_TLS_H
#include "vcos_tls.h"
#endif

/* Headers with predicates */

#if VCOS_HAVE_ONCE
#include "vcos_once.h"
#endif

#endif /* VCOS_H */
