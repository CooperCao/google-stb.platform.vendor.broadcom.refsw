/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*=============================================================================
VideoCore OS Abstraction Layer - initialization routines
=============================================================================*/


#include "vcos_types.h"
#include "vcos_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \file
  *
  * Some OS support libraries need some initialization. To support this, call
  * vcos_init() function at the start of day; vcos_deinit() at the end.
  */

/**
 * vcos initialization. Call this function before using other vcos functions.
 * Calls can be nested within the same process; they are reference counted so
 * that only a call from uninitialized state has any effect.
 * @note On platforms/toolchains that support it, gcc's constructor attribute or
 *       similar is used to invoke this function before main() or equivalent.
 * @return Status of initialisation.
 */
VCOS_STATUS_T vcos_init(void);

/**
 * vcos deinitialization. Call this function when vcos is no longer required,
 * in order to free resources.
 * Calls can be nested within the same process; they are reference counted so
 * that only a call that decrements the reference count to 0 has any effect.
 * @note On platforms/toolchains that support it, gcc's destructor attribute or
 *       similar is used to invoke this function after exit() or equivalent.
 * @return Status of initialisation.
 */
void vcos_deinit(void);

/**
 * Acquire global lock. This must be available independent of vcos_init()/vcos_deinit().
 */
void vcos_global_lock(void);

/**
 * Release global lock. This must be available independent of vcos_init()/vcos_deinit().
 */
void vcos_global_unlock(void);

/** Pass in the argv/argc arguments passed to main() */
void vcos_set_args(int argc, const char **argv);

/** Return argc. */
int vcos_get_argc(void);

/** Return argv. */
const char ** vcos_get_argv(void);

/**
 * Platform-specific initialisation.
 * VCOS internal function, not part of public API, do not call from outside
 * vcos. vcos_init()/vcos_deinit() reference count calls, so this function is
 * only called from an uninitialized state, i.e. there will not be two
 * consecutive calls to vcos_platform_init() without an intervening call to
 * vcos_platform_deinit().
 * This function is called with vcos_global_lock held.
 * @return Status of initialisation.
 */
VCOS_STATUS_T vcos_platform_init(void);

/**
 * Platform-specific de-initialisation.
 * VCOS internal function, not part of public API, do not call from outside
 * vcos.
 * See vcos_platform_init() re reference counting.
 * This function is called with vcos_global_lock held.
 */
void vcos_platform_deinit(void);

#ifdef __cplusplus
}
#endif
