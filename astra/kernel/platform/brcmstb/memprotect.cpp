/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include "platform.h"
#include "brcmstb.h"
#include "lib_printf.h"

#define RANGE_NUM_MAX           8
#define RANGE_REG_INC           0x10
#define RANGE_ADDRESS_SHIFT     12

enum {
    RANGE_ADDRESS_ULIMIT        = 0x0,
    RANGE_ADDRESS_LLIMIT        = 0x4,
    RANGE_PERMISSIONS           = 0x8,
    RANGE_ACCESS_RIGHTS_CONTROL = 0xc
};

/* HIF_CPUBIUARCH :: ADDRESS_RANGE0_ULIMIT :: ULIMIT [31:04] */
#define RANGE_ADDRESS_ULIMIT_ULIMIT_MASK                0xfffffff0
#define RANGE_ADDRESS_ULIMIT_ULIMIT_SHIFT               4

/* HIF_CPUBIUARCH :: ADDRESS_RANGE0_LLIMIT :: LLIMIT [31:04] */
#define RANGE_ADDRESS_LLIMIT_LLIMIT_MASK                0xfffffff0
#define RANGE_ADDRESS_LLIMIT_LLIMIT_SHIFT               4

/* HIF_CPUBIUARCH :: PERMISSIONS_RANGE0 :: S_BIT [06:06] */
#define RANGE_PERMISSIONS_S_BIT_MASK                    0x00000040
#define RANGE_PERMISSIONS_S_BIT_SHIFT                   6

/* HIF_CPUBIUARCH :: PERMISSIONS_RANGE0 :: WEB_EN_S_Bit [05:05] */
#define RANGE_PERMISSIONS_WEB_EN_S_Bit_MASK             0x00000020
#define RANGE_PERMISSIONS_WEB_EN_S_Bit_SHIFT            5

/* HIF_CPUBIUARCH :: PERMISSIONS_RANGE0 :: STB_EN_S_Bit [04:04] */
#define RANGE_PERMISSIONS_STB_EN_S_Bit_MASK             0x00000010
#define RANGE_PERMISSIONS_STB_EN_S_Bit_SHIFT            4

/* HIF_CPUBIUARCH :: PERMISSIONS_RANGE0 :: WEBCORES_ENABLE [02:02] */
#define RANGE_PERMISSIONS_WEBCORES_ENABLE_MASK          0x00000004
#define RANGE_PERMISSIONS_WEBCORES_ENABLE_SHIFT         2

/* HIF_CPUBIUARCH :: PERMISSIONS_RANGE0 :: STBCORES_ENABLE [00:00] */
#define RANGE_PERMISSIONS_STBCORES_ENABLE_MASK          0x00000001
#define RANGE_PERMISSIONS_STBCORES_ENABLE_SHIFT         0

/* HIF_CPUBIUARCH :: ACCESS_RIGHTS_CONTROL_RANGE0 :: ARCH_EN [00:00] */
#define RANGE_ACCESS_RIGHTS_CONTROL_ARCH_EN_MASK        0x00000001
#define RANGE_ACCESS_RIGHTS_CONTROL_ARCH_EN_SHIFT       0

void Platform::memProtect(void *physStartAddr, void *physStopAddr)
{
    uintptr_t rangeBase;
    int i;

    // Look for an unused ARCH range
    rangeBase = STB_REG_ADDR(STB_HIF_CPUBIUARCH_ADDRESS_RANGE0_ULIMIT);

    if (!rangeBase) {
        info_msg("CPU BIU ARCH not found, memory unprotected");
        return;
    }

    for (i = 0; i < RANGE_NUM_MAX; i++) {
        if (REG_RD(rangeBase + RANGE_ACCESS_RIGHTS_CONTROL) == 0)
            break;
        rangeBase += RANGE_REG_INC;
    }

    if (i == RANGE_NUM_MAX) {
        info_msg("CPU BIU ARCH has no spare range, memory unprotected");
        return;
    }

    // Set up ARCH range for protection
    REG_WR(rangeBase + RANGE_ADDRESS_ULIMIT,
           ((uintptr_t)physStopAddr >> RANGE_ADDRESS_SHIFT) <<
           RANGE_ADDRESS_ULIMIT_ULIMIT_SHIFT);

    REG_WR(rangeBase + RANGE_ADDRESS_LLIMIT,
           ((uintptr_t)physStartAddr >> RANGE_ADDRESS_SHIFT) <<
           RANGE_ADDRESS_LLIMIT_LLIMIT_SHIFT);

    REG_WR(rangeBase + RANGE_PERMISSIONS,
           RANGE_PERMISSIONS_S_BIT_MASK |
           RANGE_PERMISSIONS_STBCORES_ENABLE_MASK |
           RANGE_PERMISSIONS_WEBCORES_ENABLE_MASK |
           RANGE_PERMISSIONS_STB_EN_S_Bit_MASK |
           RANGE_PERMISSIONS_WEB_EN_S_Bit_MASK);

    REG_WR(rangeBase + RANGE_ACCESS_RIGHTS_CONTROL,
           RANGE_ACCESS_RIGHTS_CONTROL_ARCH_EN_MASK);
}
