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

#include "ENDIANESS.h"



#if defined(BFPSDK_LIBDSPCONTROL_ENDIANESS_NO_BUILTINS)
/* Original code from libfp/host_src/endian.c */

uint16_t ENDIANESS_bswap16(const uint16_t arg)
{
    return ((arg<<8) | (arg>>8));
}


uint32_t ENDIANESS_bswap32(const uint32_t arg)
{
    uint8_t res[4];

    res[0] = ((uint8_t *)(&arg))[3];
    res[1] = ((uint8_t *)(&arg))[2];
    res[2] = ((uint8_t *)(&arg))[1];
    res[3] = ((uint8_t *)(&arg))[0];

    return (*(uint32_t *)res);
}


uint64_t ENDIANESS_bswap64(const uint64_t arg)
{
    uint8_t res[8];

    res[0] = ((uint8_t *)(&arg))[7];
    res[1] = ((uint8_t *)(&arg))[6];
    res[2] = ((uint8_t *)(&arg))[5];
    res[3] = ((uint8_t *)(&arg))[4];
    res[4] = ((uint8_t *)(&arg))[3];
    res[5] = ((uint8_t *)(&arg))[2];
    res[6] = ((uint8_t *)(&arg))[1];
    res[7] = ((uint8_t *)(&arg))[0];

    return (*(uint64_t *)res);
}

#elif FEATURE_IS(SW_HOST, RAAGA_MAGNUM)

/* To avoid the "ISO C forbids an empty translation unit" warning */
void ENDIANESS_unused(void)
{
}

#endif
