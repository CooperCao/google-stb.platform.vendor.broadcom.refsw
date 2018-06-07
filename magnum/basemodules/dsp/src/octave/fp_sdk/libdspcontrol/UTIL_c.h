/****************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ****************************************************************************/

/**
 * Utility function/macros for handling C language compatibility between
 * targets.
 */


#ifndef _UTIL_C_H_
#define _UTIL_C_H_

#include "libdspcontrol/CHIP.h"
#include "fp_sdk_config.h"


/* Some toolchains choke on "inline" - how nice! */
#if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include "bstd_defs.h"
#  if defined(BSTD_INLINE)
/* On some targets (and only on some of them!) the Magnum BSTD_INLINE macro
 * contains "static" in its expansion (#$@^%$%"????!!!!) so we have to try be
 * clever here (sigh!).
 * Note to future self: to understand who defines what, use the following:
 *   grep -R BSTD_INLINE nexus/base/include/
 */
#    if defined(NEXUS_BASE_OS_linuxuser)   || \
        defined(NEXUS_BASE_OS_linuxkernel) || \
        defined(NEXUS_BASE_OS_ucos)        || \
        defined(NEXUS_BASE_OS_bare)
#      define BFPSDK_INLINE               BSTD_INLINE
#      define BFPSDK_ALWAYS_INLINE        BSTD_INLINE
#      define BFPSDK_STATIC_INLINE        BSTD_INLINE
#      define BFPSDK_STATIC_ALWAYS_INLINE BSTD_INLINE
#    else
#      define BFPSDK_INLINE               BSTD_INLINE
#      define BFPSDK_ALWAYS_INLINE        BSTD_INLINE
#      define BFPSDK_STATIC_INLINE        static BSTD_INLINE
#      define BFPSDK_STATIC_ALWAYS_INLINE static BSTD_INLINE
#    endif
#  else
#    define BFPSDK_INLINE
#    define BFPSDK_ALWAYS_INLINE
#    define BFPSDK_STATIC_INLINE        static
#    define BFPSDK_STATIC_ALWAYS_INLINE static
#  endif
#  define BFPSDK_NONNULL                __attribute__((nonnull))
#  define BFPSDK_NONNULL_ARGS(args)     __attribute__((nonnull args))
#  define BFPSDK_WARN_UNUSED_RESULT     __attribute__((warn_unused_result))
#elif __GNUC__ < 3
#  define BFPSDK_INLINE                 inline
#  define BFPSDK_ALWAYS_INLINE          inline
#  define BFPSDK_STATIC_INLINE          static inline
#  define BFPSDK_STATIC_ALWAYS_INLINE   static inline
#  define BFPSDK_NONNULL
#  define BFPSDK_NONNULL_ARGS(args)
#  define BFPSDK_WARN_UNUSED_RESULT
#else
#  define BFPSDK_INLINE                 inline
#  define BFPSDK_ALWAYS_INLINE          inline __attribute__((always_inline))
#  define BFPSDK_STATIC_INLINE          static inline
#  define BFPSDK_STATIC_ALWAYS_INLINE   static inline __attribute__((always_inline))
#  define BFPSDK_NONNULL                __attribute__((nonnull))
#  define BFPSDK_NONNULL_ARGS(args)     __attribute__((nonnull args))
#  define BFPSDK_WARN_UNUSED_RESULT     __attribute__((warn_unused_result))
#endif


/**
 * @def UTIL_reinterpret_cast(dest_type, dest_var, dest_var_init, src_type, src_var)
 *
 * C equivalent of the C++ reinterpret_cast<> construct. In pseudo-language:
 *   dest_var = dest_var_init;
 *   dest_var = reinterpret_cast<dest_type>((src_type) src_var);
 */
#define UTIL_reinterpret_cast(dest_type, dest_var, dest_var_init, src_type, src_var)    \
{                                                                                       \
    union __attribute__((packed)) UTIL_reinterpret_cast_union {                         \
    src_type src_field;                                                                 \
    dest_type dest_field;                                                               \
    } UTIL_reinterpret_cast_temp;                                                       \
    UTIL_reinterpret_cast_temp.dest_field = dest_var_init;                              \
    UTIL_reinterpret_cast_temp.src_field = src_var;                                     \
    dest_var = UTIL_reinterpret_cast_temp.dest_field;                                   \
}


#endif  /* _UTIL_C_H_ */
