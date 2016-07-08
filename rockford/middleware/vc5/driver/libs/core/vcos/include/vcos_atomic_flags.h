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

#ifndef VCOS_ATOMIC_FLAGS_H
#define VCOS_ATOMIC_FLAGS_H

#include "vcos_types.h"
#include "vcos_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file vcos_atomic_flags.h
 *
 * Defines atomic flags API.
 *
 * 32 flags. Atomic "or" and "get and clear" operations
 */

/**
 * Create an atomic flags instance.
 *
 * @param atomic_flags Pointer to atomic flags instance, filled in on return
 *
 * @return VCOS_SUCCESS if succeeded.
 */
static inline VCOS_STATUS_T vcos_atomic_flags_create(VCOS_ATOMIC_FLAGS_T *atomic_flags);

/**
 * Atomically set the specified flags.
 *
 * @param atomic_flags Instance to set flags on
 * @param flags        Mask of flags to set
 */
static inline void vcos_atomic_flags_or(VCOS_ATOMIC_FLAGS_T *atomic_flags, uint32_t flags);

/**
 * Retrieve the current flags and then clear them. The entire operation is
 * atomic.
 *
 * @param atomic_flags Instance to get/clear flags from/on
 *
 * @return Mask of flags which were set (and we cleared)
 */
static inline uint32_t vcos_atomic_flags_get_and_clear(VCOS_ATOMIC_FLAGS_T *atomic_flags);

/**
 * Delete an atomic flags instance.
 *
 * @param atomic_flags Instance to delete
 */
static inline void vcos_atomic_flags_delete(VCOS_ATOMIC_FLAGS_T *atomic_flags);

#ifdef __cplusplus
}
#endif

#endif
