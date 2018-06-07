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
 * Utility function/macros for ADDR union type management.
 */


#ifndef _UTIL_ADDR_H_
#define _UTIL_ADDR_H_

#include "libdspcontrol/CHIP.h"
#include "libdspcontrol/DSP.h"
#include "libdspcontrol/UTIL_c.h"

#include "libfp/src/c_utils_internal.h"



/** Utility macro to perform inline operations on ADDR structures. */
#define UTIL_addr_op(the_ADDR, op)                          \
{                                                           \
    if(the_ADDR.addr_space == ADDR_SPACE_SYSTEM) {          \
        op(the_ADDR.addr.system, SYSTEM_ADDR)               \
    } else if(the_ADDR.addr_space == ADDR_SPACE_SHARED) {   \
        op(the_ADDR.addr.shared, SHARED_ADDR)               \
    } else if(the_ADDR.addr_space == ADDR_SPACE_DSP) {      \
        op(the_ADDR.addr.dsp, DSP_ADDR)                     \
    }                                                       \
}


/**
 * @def UTIL_addr_assign_to(to_type, to, from_ADDR)
 *
 * Assign the content of the ADDR union "as-is" to an integral type,
 * narrowing of zero-extending as needed.
 */
#define UTIL_addr_assign_to(to_type, to, from_ADDR)         \
    UTIL_reinterpret_cast(to_type, to, (to_type) 0, union ADDR_UNION, from_ADDR.addr)


/**
 * @def UTIL_addr_assign_from(dest_ADDR, from_type, from)
 *
 * Emulation for the "union cast" operation that works also when the from_type
 * is not compatible with the types in the ADDR union. In pseudo-code:
 *   dest_ADDR.addr = (union ADDR_UNION) from;
 */
#define UTIL_addr_assign_from(dest_ADDR, from_type, from)           \
{                                                                   \
    union __attribute__((packed)) UTIL_addr_union {                 \
        union ADDR_UNION x;                                         \
        from_type        y;                                         \
    } UTIL_addr_assign_from_temp;                                   \
    COMPILE_TIME_ASSERT(offsetof(union UTIL_addr_union, x) == 0);   \
    COMPILE_TIME_ASSERT(offsetof(union UTIL_addr_union, y) == 0);   \
    UTIL_addr_assign_from_temp.x.system = SYSTEM_ADDR_CAST(0);      \
    UTIL_addr_assign_from_temp.x.shared = SHARED_ADDR_CAST(0);      \
    UTIL_addr_assign_from_temp.x.dsp = DSP_ADDR_CAST(0);            \
    UTIL_addr_assign_from_temp.y = from;                            \
    dest_ADDR.addr = UTIL_addr_assign_from_temp.x;                  \
}


/**
 * @def UTIL_addr_size_assign_from(dest_ADDR_SIZE, from_type, from)
 *
 * Emulation for the "union cast" operation that works also when the from_type
 * is not compatible with the types in the ADDR union. In pseudo-code:
 *   dest_ADDR_SIZE.addr = (ADDR_SIZE) from;
 */
#define UTIL_addr_size_assign_from(dest_ADDR_SIZE, from_type, from)     \
{                                                                       \
    union __attribute__((packed)) UTIL_addr_size_union {                \
        ADDR_SIZE x;                                                    \
        from_type y;                                                    \
    } UTIL_addr_size_assign_from_temp;                                  \
    COMPILE_TIME_ASSERT(offsetof(union UTIL_addr_size_union, x) == 0);  \
    COMPILE_TIME_ASSERT(offsetof(union UTIL_addr_size_union, y) == 0);  \
    UTIL_addr_size_assign_from_temp.x.system = (SYSTEM_ADDR_SIZE) 0;    \
    UTIL_addr_size_assign_from_temp.x.shared = (SHARED_ADDR_SIZE) 0;    \
    UTIL_addr_size_assign_from_temp.x.dsp = (DSP_ADDR_SIZE) 0;          \
    UTIL_addr_size_assign_from_temp.y = from;                           \
    dest_ADDR_SIZE = UTIL_addr_size_assign_from_temp.x;                 \
}


/**
 * Minimum length of the char array to pass to the UTIL_addr_2_hex macro.
 */
#define UTIL_ADDR_2_HEX_MIN_SIZE    (sizeof(union ADDR_UNION) * 2 + 1)


/**
 * @def UTIL_addr_2_hex(the_ADDR, string_var)
 * Converts an ADDR in an hexadecimal string (%0x format) in a system-independent way.
 *
 * @param the_ADDR    the ADDR instance to convert into string
 * @param string_var  buffer of length at least UTIL_ADDR_2_HEX_FUNC_STR_MIN_SIZE bytes
 */
#define UTIL_addr_2_hex(the_ADDR, string_var)                                       \
{                                                                                   \
    COMPILE_TIME_ASSERT(sizeof(string_var) >= UTIL_ADDR_2_HEX_FUNC_STR_MIN_SIZE);   \
    UTIL_addr_2_hex_func(the_ADDR, string_var);                                     \
}


/* Internal function */
void UTIL_addr_2_hex_func(ADDR addr, char *string_var);


/* Set a MAPPED_ADDR structure to invalid values */
#define MAPPED_ADDR_MAKE_INVALID(locked_addr)   \
{                                               \
    (locked_addr).length = 0;                   \
    (locked_addr).addr = (void *) UINTPTR_MAX;  \
}


#endif  /* _UTIL_ADDR_H_ */
