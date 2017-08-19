/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ****************************************************************************/

/* NOTE: this file gets exported into the Raaga Magnum host library and so it
 *       must abide by a specific strict set of rules. Please use only ANSI C
 *       constructs and include only FPSDK headers which do as well. After any
 *       edit, always make sure that the Raaga Magnum build succeeds without
 *       any warning.
 */

#include "libdspcontrol/CHIP.h"

#if !FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include <stdint.h>
#  include <stddef.h>
#else
#  include "bstd.h"
#endif

#include "libdspcontrol/DSP.h"

#include "UTIL_addr.h"



/* Workaround for "long long" being frowned upon in 32 bit Magnum builds.
 * Definition stolen from Magnum various bchp_common.h headers. */
#ifdef UINT64_C
#  undef UINT64_C
#endif
#define UINT64_C(hi, lo)    (((uint64_t)hi)<<32 | (lo)) /* C89 64-bit literal */


void UTIL_addr_2_hex_func(ADDR addr, char *string_var)
{
    const char *hex_chars = "0123456789abcdef";
    const uint64_t endianess = UINT64_C(0x07060504, 0x03020100);
    const uint8_t *endianess_bytes = (const uint8_t *) &endianess;

    size_t num_nibbles, num_bytes = 0;
    char *addr_bytes = NULL;
    if(addr.addr_space == ADDR_SPACE_SYSTEM) {
        addr_bytes = (char *)(void *) &(addr.addr.system);
        num_bytes = sizeof(ADDR_SPACE_SYSTEM);
    } else if(addr.addr_space == ADDR_SPACE_SHARED) {
        addr_bytes = (char *)(void *) &(addr.addr.shared);
        num_bytes = sizeof(ADDR_SPACE_SHARED);
    } else if(addr.addr_space == ADDR_SPACE_DSP) {
        addr_bytes = (char *)(void *) &(addr.addr.dsp);
        num_bytes = sizeof(ADDR_SPACE_DSP);
    }
    num_nibbles = num_bytes * 2;

    {
        unsigned str_i;
        for(str_i = 0; str_i < num_nibbles; str_i += 2)
        {
            unsigned byte_i = num_bytes - str_i / 2 - 1;
            uint8_t addr_byte = addr_bytes[endianess_bytes[byte_i]];
            string_var[str_i]     = hex_chars[(addr_byte & 0xf0) >> 4];
            string_var[str_i + 1] = hex_chars[(addr_byte & 0x0f) >> 0];
        }
        string_var[str_i] = 0;
    }
}
