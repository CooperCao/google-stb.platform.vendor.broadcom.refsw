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
 * @file
 * @ingroup libsyschip
 * @brief Definition of Target Buffer structures common to the DSP and the Host.
 *
 * Target Buffers is a mechanism for transferring streams of data from the DSP to
 * the Host. It makes use of a "shared" circular buffer and a shared control structure
 * represented by the #TB_shared type. The level of visibility and the means of
 * access to the shared storage, both from the DSP and the Host point of view, vary
 * from platform to platform. Also the meaning of the storage pointer (to which
 * address space it belongs, for example) is platform-dependent.\n
 *
 * The #TB_shared structure must be placed in a memory area directly readable and
 * writable by both the DSP and the Host. Note that some platforms impose restrictions
 * on the granularity and alignment of memory accesses to the #TB_shared structure
 * (e.g. only whole 32 bit words on 32 bit boundaries can be read and written).
 * Specific internal functions are used to access the #TB_shared structure properly.
 * The size of the #TB_shared control structure is 4 words for historical reasons and
 * is unlikely to change in the near future. To avoid race conditions, only one of
 * the two parties (the DSP and the Host) should be in charge of writing to each of
 * the words; there are no known reading restrictions.
 * The #TB_shared structure setup is usually performed by one of the two parties. For
 * the other party there is no specific mean to detect if the shared structure has been
 * correctly set up, so heuristic methods are usually employed.\n
 *
 * The #TB_shared structure describes a circular buffer of storage_kbytes KiB size.
 * The write pointer indicates where any new write should happen, the read pointer
 * indicates where next data can be read from. Taking into account the appropriate
 * pointers wrap-arounds, the distance read_ptr - write_ptr represents the available
 * (not-yet-read) data in the %TB, the distance write_ptr - read_ptr represents free
 * space (or already read, overwritable data) in the %TB.
 * By convention, when read_ptr == write_ptr there is no data available and the %TB is
 * empty; when write_ptr == read_ptr - 1 the %TB is full. Given this, notice how the
 * maximum amount of data the %TB can contain is always one byte less than its nominal
 * size.
 *
 * @image html  TB_shared_scenarios.png "Some possible TB_shared scenarios"
 * @image latex TB_shared_scenarios.eps "Some possible TB_shared scenarios" width=\textwidth
 */

#ifndef _TBUF_H_
#define _TBUF_H_

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "fp_sdk_config.h"

#if defined(__FIREPATH__)
#  include <stdint.h>
#else
/* Let's assume we are being included from libdspcontrol */
#  include "libdspcontrol/CHIP.h"
#  if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#    include "bstd_defs.h"
#  else
#    include <stdint.h>
#  endif
#endif



/**
 * This pointer type macro allows the same #TB_shared structure definition
 * to be used both on the DSP and the Host.
 */
#ifdef __FIREPATH__
#  define TB_CHAR_PTR   char *
#else
#  define TB_CHAR_PTR   uint32_t
#endif


/**
 * This is the 4-words Target Buffer shared control structure. An instance of
 * this is put in a memory visible from both the DSP and the Host, and used for
 * coordination between the two.
 */
typedef struct
{
    uint8_t dummy1;             /**< usage to be defined - set to 0 */
    uint8_t dummy2;             /**< usage to be defined - set to 0 */
    uint16_t storage_kbytes;    /**< data buffer size */
    TB_CHAR_PTR storage;        /**< data buffer location */
    TB_CHAR_PTR read_ptr;       /**< where the next read should start from */
    TB_CHAR_PTR write_ptr;      /**< where the next write should occur */
} TB_shared;



#endif /* _TBUF_H_ */
