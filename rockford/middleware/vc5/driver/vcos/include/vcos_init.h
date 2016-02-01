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
