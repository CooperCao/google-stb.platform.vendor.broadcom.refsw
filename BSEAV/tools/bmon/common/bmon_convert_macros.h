/******************************************************************************
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
 *****************************************************************************/

#ifndef __BMON_CONVERT_MACROS_H__
#define __BMON_CONVERT_MACROS_H__

#include <stdio.h>
#include <string.h>
#include "bmon_defines.h"

#define STRING_MAP_START(map)               static const char * map[] = {
#define STRING_MAP_ENTRY(nvalue, strvalue)  [nvalue] = strvalue,
#define STRING_MAP_END()                    };

/***************************************************************************
 * Summary:
 * converts given enum to associated string based on the given map.
 * If the given eunm does not exist map, then error is printed and "unknown"
 * string is returned.
 ***************************************************************************/
#define ENUM_TO_STRING_FUNCTION(func, ntype, map)                                                             \
    const char * func(ntype var)                                                                              \
    {                                                                                                         \
        const char * pStr = map[var];                                                                         \
        if (NULL == pStr)                                                                                     \
        {                                                                                                     \
            fprintf(stderr, "%s enum:%d is missing corresponding valid string value.\n", __FUNCTION__, var); \
            return ("unknown");                                                                               \
        }                                                                                                     \
        return (pStr);                                                                                        \
    }

/***************************************************************************
 * Summary:
 * Converts given string to associated enum based on given map. if given
 * string does not exist in map, then error is printed and the last map
 * entry enum is returned (typically an invalid eMax value)
 ***************************************************************************/
#define STRING_TO_ENUM_FUNCTION(func, ntype, map)                                                        \
    const ntype func(const char * pStr)                                                                  \
    {                                                                                                    \
        int i          = 0;                                                                              \
        int mapMaxSize = sizeof(map) / sizeof(map[0]);                                                   \
        if (NULL != pStr)                                                                                \
        {                                                                                                \
            for (i = 0; i < mapMaxSize; i++)                                                             \
            {                                                                                            \
                if ((NULL != map[i]) && (0 == strncmp(map[i], pStr, strlen(pStr))))                      \
                {                                                                                        \
                    return ((ntype)i);                                                                   \
                }                                                                                        \
            }                                                                                            \
        }                                                                                                \
        fprintf(stderr, "%s string:\"%s\" is missing corresponding enum value.\n", __FUNCTION__, pStr); \
        return ((ntype)mapMaxSize - 1);                                                                  \
    }

#define ENUM_TO_STRING_DECLARE(func, ntype)  const char * func(ntype var);
#define STRING_TO_ENUM_DECLARE(func, ntype)  const ntype func(const char * pStr);

#endif /* __BMON_CONVERT_MACROS_H__ */