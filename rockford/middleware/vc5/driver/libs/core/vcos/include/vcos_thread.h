/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include "vcos_types.h"
#include <stdint.h>

#if defined(__unix__)
#include "../posix/vcos_thread_posix.h"
#else
#include "../cpp/vcos_thread_cpp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//! Thread entry point.
typedef void (*VCOS_THREAD_ENTRY_FN_T)(void*);

//! Integer thread ID.
typedef uint32_t vcos_thread_t;

/** Create a thread. It must be cleaned up by calling vcos_thread_join().
  *
  * @param thread   Filled in on return with thread
  * @param entry    Entry point.
  * @param arg      Argument passed to the entry point.
  */
VCOS_STATUS_T vcos_thread_create(
   VCOS_THREAD_T *thread,
   const char *name,
   VCOS_THREAD_ENTRY_FN_T entry,
   void *arg);

//! Wait for a thread to terminate and then clean up its resources.
void vcos_thread_join(VCOS_THREAD_T *thread);

//! Sleep for at least given number of milliseconds.
void vcos_sleep(uint32_t ms);

//! Sleep for at least given number of microseconds.
void vcos_sleep_us(uint32_t us);

//! Yield the remaining time-slice for the current thread.
void vcos_yield(void);

//! Set the name of the current thread.
void vcos_this_thread_set_name(const char* name);

//! Get the ID of the current thread.
vcos_thread_t vcos_this_thread_id(void);

//! Return the number of processes available.
unsigned vcos_thread_num_processors(void);

#ifdef __cplusplus
}
#endif
