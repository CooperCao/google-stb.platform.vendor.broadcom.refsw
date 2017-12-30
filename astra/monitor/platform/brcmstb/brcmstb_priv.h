/******************************************************************************
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
 *****************************************************************************/

#ifndef _BRCMSTB_PRIV_H_
#define _BRCMSTB_PRIV_H_

#include "brcmstb_params.h"

#define BRCMSTB_UART_CLK                81000000
#define BRCMSTB_BAUD_RATE               115200

/* GIC v2 memory map, from start of GIC base */
#define BRCMSTB_GIC_DIST_BASE           0x1000
#define BRCMSTB_GIC_CPUIF_BASE          0x2000
#define BRCMSTB_GIC_VIRT_CTRL_BASE      0x4000
#define BRCMSTB_GIC_VIRT_CPUIF_BASE     0x6000

#define BRCMSTB_GIC_MMAP_SIZE           0x8000

/* !!! For Bolt backward compatibility only !!! */
#define BRCMSTB_CHIP_ID_ADDR            0xf0404000

/* Macros to retrieve Brcmstb SOC register group info */
#define BRCMSTB_RGROUP_REV(rgname) \
    (brcmstb_params.rgroups[BRCMSTB_RGROUP_##rgname].rev)

#define BRCMSTB_RGROUP_BASE(rgname) \
    (brcmstb_params.rgroups[BRCMSTB_RGROUP_##rgname].base)

#define BRCMSTB_RGROUP_SIZE(rgname) \
    (brcmstb_params.rgroups[BRCMSTB_RGROUP_##rgname].size)

/* Macros to calculate Brcmstb SOC register address */
#define BRCMSTB_REG_ADDR(rgname, rname) \
    (BRCMSTB_RGROUP_BASE(rgname) + BCHP_##rgname##_##rname)

#define BRCMSTB_REG_ADDR_WITH_INDEX(rgname, rname, index) \
    (BRCMSTB_REG_ADDR(rgname, rname) + BCHP_##rgname##_##rname##_STRIDE * index)

/* Macros to contruct Brcmstb SOC register field defines */
#define BRCMSTB_FIELD_MASK(rgname, rname, fname) \
    (BCHP_##rgname##_##rname##_##fname##_MASK)

#define BRCMSTB_FIELD_SHIFT(rgname, rname, fname) \
    (BCHP_##rgname##_##rname##_##fname##_SHIFT)

/* Brcmstb global variables */
extern brcmstb_params_t brcmstb_params;

#endif /* _BRCMSTB_PRIV_H_ */
