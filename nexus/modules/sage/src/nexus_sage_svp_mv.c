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
 *
 ******************************************************************************/

#include "nexus_sage_svp_mv.h"
#include "bchp_common.h"
#include "priv/bsagelib_shared_globalsram.h"
#include "brdc.h"

#include "bdbg.h"
BDBG_MODULE(nexus_sage_svp_mv);

#ifdef BCHP_IT_0_REG_START
#include "bchp_it_0.h"
#endif
#ifdef BCHP_IT_1_REG_START
#include "bchp_it_1.h"
#endif
#ifdef BCHP_IT_2_REG_START
#include "bchp_it_2.h"
#endif
#ifdef BCHP_IT_3_REG_START
#include "bchp_it_3.h"
#endif

static bool MV_detect_sub (BREG_Handle hReg, uint32_t core_offset)
{
    uint32_t tg_config = BREG_Read32 (hReg,
        BCHP_IT_0_TG_CONFIG + (core_offset - BCHP_IT_0_REG_START));
    uint32_t mc_enables =
        BCHP_GET_FIELD_DATA (tg_config, IT_0_TG_CONFIG, MC_ENABLES);
    /* Condition is: are any of microcontrollers 3, 4, 5 enabled? */
    return ((mc_enables & 0x38) != 0);
}

static bool Display_MV_detect(BREG_Handle hReg, uint32_t core_offset, uint32_t id)
{
    bool capable;
    int i;
    /* This is the minimum wait time between setting the reset
       blocker and reading the registers */
    int block_wait = 2;
    /* This is the minimum time to give display to finish up format switch
       to avoid timeout */
    int format_switch_wait = 16;

    for(i=0; i<block_wait; i++)
        BRDC_WriteScratch(hReg, BRDC_GET_MV_BLOCK_REG(id), 1);

    capable = MV_detect_sub (hReg, core_offset);

    for(i=0; i<format_switch_wait; i++)
        BRDC_WriteScratch(hReg, BRDC_GET_MV_BLOCK_REG(id), 0);

    return capable;
}

bool MV_detect (BREG_Handle hReg)
{
    bool capable = false;

#ifdef BCHP_IT_0_REG_START
    capable |= Display_MV_detect (hReg, BCHP_IT_0_REG_START, 0);
#endif
#ifdef BCHP_IT_1_REG_START
    capable |= Display_MV_detect (hReg, BCHP_IT_1_REG_START, 1);
#endif
#ifdef BCHP_IT_2_REG_START
    capable |= Display_MV_detect (hReg, BCHP_IT_2_REG_START, 2);
#endif
#ifdef BCHP_IT_3_REG_START
    capable |= Display_MV_detect (hReg, BCHP_IT_3_REG_START, 3);
#endif

    return capable;
}

bool MV_Monitor_Check(BREG_Handle hReg)
{
    bool rc = false; /* no violation */

    if (MV_detect(hReg))
    {
        uint32_t addr, value;

        /* Macrovision is ON */

        /* Now see if we have license */
        addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eBP3HostFeatureList);
        value = BREG_Read32(hReg, addr);
        BDBG_ERR(("### BSAGElib_GlobalSram_eBP3HostFeatureList: 0x%08x", value));
        if (!(value & BP3_HOST_FL_MACROVISION))
        {
            /* Macrovision is not in the feature list, violation */
            rc = true;
        }
    }
    return rc;
}
