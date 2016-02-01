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
VideoCore OS Abstraction Layer - public header file
=============================================================================*/

#ifndef VCOS_THREAD_H
#define VCOS_THREAD_H

#include "vcos_types.h"
#include "vcos_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file vcos_thread.h
 *
 * \section thread Threads
 *
 * Under Nucleus, a thread is created by NU_Create_Task, passing in the stack
 * and various other parameters. To stop the thread, NU_Terminate_Thread() and
 * NU_Delete_Thread() are called.
 *
 * Unfortunately it's not possible to emulate this API under some fairly common
 * operating systems. Under Windows you can't pass in the stack, and you can't
 * safely terminate a thread.
 *
 * Therefore, an API which is similar to the pthreads API is used instead. This
 * API can (mostly) be emulated under all interesting operating systems.
 *
 * Obviously this makes the code somewhat more complicated on VideoCore than it
 * would otherwise be - we end up with an extra mutex per thread, and some code
 * that waits for it. The benefit is that we have a single way of creating
 * threads that works consistently on all platforms (apart from stack supplying).
 *
 * \subsection stack Stack
 *
 * It's still not possible to pass in the stack address, but this can be made
 * much more obvious in the API: the relevant function is missing and the
 * CPP symbol VCOS_CAN_SET_STACK_ADDR is zero rather than one.
 *
 * \subsection thr_create Creating a thread
 *
 * The simplest way to create a thread is with vcos_thread_create() passing in a
 * NULL thread parameter argument. To wait for the thread to exit, call
 * vcos_thread_join().
 *
 * \subsection back Backward compatibility
 *
 * To ease migration, a "classic" thread creation API is provided for code
 * that used to make use of Nucleus, vcos_thread_create_classic(). The
 * arguments are not exactly the same, as the PREEMPT parameter is dropped.
 *
 */

#define VCOS_AFFINITY_CPU(n)    _VCOS_AFFINITY_CPU(n)
/* Define explicit CPU0/1 defines for backward compatibility.  Newer code should
 * use VCOS_AFFINITY_CPU(n) to better support multiple cores. */
#define VCOS_AFFINITY_CPU0    _VCOS_AFFINITY_CPU0
#define VCOS_AFFINITY_CPU1    _VCOS_AFFINITY_CPU1

#define VCOS_AFFINITY_MASK    _VCOS_AFFINITY_MASK
#define VCOS_AFFINITY_DEFAULT _VCOS_AFFINITY_DEFAULT
#define VCOS_AFFINITY_THISCPU _VCOS_AFFINITY_THISCPU

/** Report whether or not we have an RTOS at all, and hence the ability to
  * create threads.
  */
int vcos_have_rtos(void);

/** Create a thread. It must be cleaned up by calling vcos_thread_join().
  *
  * @param thread   Filled in on return with thread
  * @param name     A name for the thread. May be the empty string.
  * @param attrs    Attributes; default attributes will be used if this is NULL.
  * @param entry    Entry point.
  * @param arg      Argument passed to the entry point.
  */
VCOS_STATUS_T vcos_thread_create(VCOS_THREAD_T *thread,
                                                    const char *name,
                                                    VCOS_THREAD_ATTR_T *attrs,
                                                    VCOS_THREAD_ENTRY_FN_T entry,
                                                    void *arg);

/** Exit the thread from within the thread function itself.
  * Resources must still be cleaned up via a call to thread_join().
  *
  * The thread can also be terminated by simply exiting the thread function.
  *
  * @param data Data passed to thread_join. May be NULL.
  */
void vcos_thread_exit(void *data);

/** Wait for a thread to terminate and then clean up its resources.
  *
  * @param thread Thread to wait for
  * @param pData  Updated to point at data provided in vcos_thread_exit or exit
  * code of thread function.
  */
void vcos_thread_join(VCOS_THREAD_T *thread,
                             void **pData);


/**
  * \brief Create a thread using an API similar to the one "traditionally"
  * used under Nucleus.
  *
  * This creates a thread which must be cleaned up by calling vcos_thread_join().
  * The thread cannot be simply terminated (as in Nucleus and ThreadX) as thread
  * termination is not universally supported.
  *
  * @param thread       Filled in with thread instance
  * @param name         An optional name for the thread. NULL or "" may be used (but
  *                     a name will aid in debugging).
  * @param entry        Entry point
  * @param arg          A single argument passed to the entry point function
  * @param stack        Pointer to stack address
  * @param stacksz      Size of stack in bytes
  * @param priaff       Priority of task, between VCOS_PRI_LOW and VCOS_PRI_HIGH, ORed with the CPU affinity
  * @param autostart    If non-zero the thread will start immediately.
  * @param timeslice    Timeslice (system ticks) for this thread.
  *
  * @sa vcos_thread_terminate vcos_thread_delete
  */
VCOS_STATUS_T vcos_thread_create_classic(VCOS_THREAD_T *thread,
                                                            const char *name,
                                                            void *(*entry)(void *arg),
                                                            void *arg,
                                                            void *stack,
                                                            VCOS_UNSIGNED stacksz,
                                                            VCOS_UNSIGNED priaff,
                                                            VCOS_UNSIGNED timeslice,
                                                            VCOS_UNSIGNED autostart);

/**
  * \brief Set a thread's priority
  *
  * Set the priority for a thread.
  *
  * @param thread  The thread
  * @param pri     Thread priority in VCOS_PRI_MASK bits; affinity in VCOS_AFFINITY_MASK bits.
  */
static inline void vcos_thread_set_priority(VCOS_THREAD_T *thread, VCOS_UNSIGNED pri);

/**
  * \brief Return the currently executing thread.
  *
  */
static inline VCOS_THREAD_T *vcos_thread_current(void);

/**
  * \brief Return the thread's priority.
  */
static inline VCOS_UNSIGNED vcos_thread_get_priority(VCOS_THREAD_T *thread);

/**
  * \brief Return the thread's cpu affinity.
  */
static inline VCOS_UNSIGNED vcos_thread_get_affinity(VCOS_THREAD_T *thread);

/**
  * \brief Set the thread's cpu affinity.
  */

static inline void vcos_thread_set_affinity(VCOS_THREAD_T *thread, VCOS_UNSIGNED affinity);

/**
  * \brief Query whether we are in an interrupt.
  *
  * @return 1 if in interrupt context.
  */
static inline int vcos_in_interrupt(void);

/**
  * \brief Sleep a while.
  *
  * @param ms Number of milliseconds to sleep for
  *
  * This may actually sleep a whole number of ticks.
  */
static inline void vcos_sleep(uint32_t ms);

/**
  * \brief Sleep a while.
  *
  * @param us Number of microseconds to sleep for
  *
  * Depending on the platform, this may not be microsecond accurate.
  */
static inline void vcos_sleep_us(uint32_t us);

/** Just like pthread_yield: http://man7.org/linux/man-pages/man3/pthread_yield.3.html */
static inline void vcos_yield(void);

/**
  * \brief Return the value of the hardware microsecond counter.
  *
  */
static inline uint32_t vcos_getmicrosecs(void);

static inline uint64_t vcos_getmicrosecs64(void);

#define vcos_get_ms() (vcos_getmicrosecs()/1000)

/**
  * \brief Return a unique identifier for the current process
  *
  */
static inline VCOS_UNSIGNED vcos_process_id_current(void);

/** Return the name of the given thread.
  */
const char * vcos_thread_get_name(const VCOS_THREAD_T *thread);

/** Set the name of the current thread.
  */
void vcos_this_thread_set_name(const char* name);

/** Return the number of processes available
  */
int vcos_thread_num_processors(void);

/*
 * Internal APIs - may not always be present and should not be used in
 * client code.
 */

extern void _vcos_task_timer_set(void (*pfn)(void*), void *, VCOS_UNSIGNED ms);
extern void _vcos_task_timer_cancel(void);

#ifdef __cplusplus
}
#endif
#endif
