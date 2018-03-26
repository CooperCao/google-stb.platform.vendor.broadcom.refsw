/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bchp_common.h"
#ifdef BCHP_AON_CTRL_REG_START
#include "bchp_aon_ctrl.h"
#endif

/* TEMP code to set address map */
static void BDTU_P_ReadMappingInfo(BREG_Handle reg, unsigned memcIndex, BDTU_MappingInfo *pInfo)
{
    BSTD_UNUSED(reg);
    BKNI_Memset(pInfo, 0, sizeof(*pInfo));
    pInfo->memcIndex = memcIndex;

#if BCHP_CHIP==7260 || BCHP_CHIP==7268 || BCHP_CHIP==7271
    pInfo->region[0].addr = 0x00000000;
    pInfo->region[0].size = 0xC0000000;
#if 0
    /* region above 4GB has issues for HVD, so avoid */
    pInfo->region[1].addr = BCHP_UINT64_C(0x1, 0x00000000);
    pInfo->region[1].size = 0x40000000;
#endif
#elif BCHP_CHIP==7278
    if (BCHP_GET_FIELD_DATA(BREG_Read32(reg, BCHP_AON_CTRL_GLOBAL_ADDRESS_MAP_VARIANT),
        AON_CTRL_GLOBAL_ADDRESS_MAP_VARIANT, map_variant))
    {
        /* v7-64 */
        if (memcIndex == 0) {
            pInfo->region[0].addr = 0x040000000;                    /* 8GB@1GB */
            pInfo->region[0].size = BCHP_UINT64_C(0x2, 0x00000000);
        }
        else {
            pInfo->region[0].addr = BCHP_UINT64_C(0x3, 0x00000000); /* 8GB@12GB */
            pInfo->region[0].size = BCHP_UINT64_C(0x2, 0x00000000);
        }
    }
    else {
        /* v7-32 */
        if (memcIndex == 0) {
            pInfo->region[0].addr = 0x040000000;
            pInfo->region[0].size = 0x080000000;
            pInfo->region[1].addr = BCHP_UINT64_C(0x1, 0x00000000); /* 6GB@4GB */
            pInfo->region[1].size = BCHP_UINT64_C(0x1, 0x80000000);
        }
        else {
            pInfo->region[0].addr = 0x0C0000000;
            pInfo->region[0].size = 0x040000000;
            pInfo->region[1].addr = BCHP_UINT64_C(0x3, 0x00000000); /* 7GB@12GB */
            pInfo->region[1].size = BCHP_UINT64_C(0x1, 0xC0000000);
        }
    }
#endif
}
