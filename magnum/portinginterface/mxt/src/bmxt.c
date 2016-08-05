/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bstd.h"
#include "bchp.h"
#include "bkni.h"
#include "bmxt_priv.h"
#include "bmxt.h"
#include "bmxt_rdb.h"
#include "bmxt_rdb_wakeup.h"
#include "bmxt_rdb_dcbg.h"

BDBG_MODULE(bmxt);

void BMXT_P_SetPlatform(
    /* you specify one or the other */
    BMXT_Handle mxt,
    BMXT_Chip chip, BMXT_ChipRev rev, unsigned **pNumElem
    )
{
    const uint32_t *pNum = 0, *pOff = 0, *pStep = 0;
    const uint32_t *pOffWakeup = 0;
    const uint32_t *pNumDcbg = 0, *pOffDcbg = 0, *pStepDcbg = 0;
    uint32_t base = 0, baseWakeup = 0, baseDcbg = 0;
    BMXT_P_PlatformType type;

    if (mxt) {
        chip = mxt->settings.chip;
        rev = mxt->settings.chipRev;
    }

    switch (chip) {
        /* 3128-family */
        case BMXT_Chip_e3128:  pNum = BMXT_NUMELEM_3128;  base = BMXT_REGBASE_3128;  pOff = BMXT_REGOFFSETS_3128; pStep = BMXT_STEPSIZE_3128; break;
        case BMXT_Chip_e3383:  pNum = BMXT_NUMELEM_3383;  base = BMXT_REGBASE_3383;  pOff = BMXT_REGOFFSETS_3128; pStep = BMXT_STEPSIZE_3128; break;
        case BMXT_Chip_e4517:  /* same as 4528 */
        case BMXT_Chip_e4528:  pNum = BMXT_NUMELEM_4528;  base = BMXT_REGBASE_4528;  pOff = BMXT_REGOFFSETS_3128; pStep = BMXT_STEPSIZE_3128; break;
        case BMXT_Chip_e3472:  pNum = BMXT_NUMELEM_3472;  base = BMXT_REGBASE_3472;  pOff = BMXT_REGOFFSETS_3128; pStep = BMXT_STEPSIZE_3128; break;

        /* 4538-family */
        case BMXT_Chip_e4538:  pNum = BMXT_NUMELEM_4538;  base = BMXT_REGBASE_4538;  pOff = BMXT_REGOFFSETS_4538; pStep = BMXT_STEPSIZE_4538;
            baseWakeup = BMXT_REGBASE_WAKEUP_4538; pOffWakeup = BMXT_REGOFFSETS_WAKEUP; break;
        case BMXT_Chip_e3384:  pNum = BMXT_NUMELEM_3384;  base = BMXT_REGBASE_3384;  pOff = BMXT_REGOFFSETS_4538; pStep = BMXT_STEPSIZE_4538; break;
        case BMXT_Chip_e4548:  /* same as 7366 */
        case BMXT_Chip_e7364:  /* same as 7366 */
        case BMXT_Chip_e7366:  pNum = BMXT_NUMELEM_7366;  base = BMXT_REGBASE_7366;  pOff = BMXT_REGOFFSETS_4538; pStep = BMXT_STEPSIZE_4538; break;
        case BMXT_Chip_e7145:  pNum = BMXT_NUMELEM_7145;  base = BMXT_REGBASE_7145;  pOff = BMXT_REGOFFSETS_4538; pStep = BMXT_STEPSIZE_4538; break;
        case BMXT_Chip_e45216: pNum = BMXT_NUMELEM_45216; base = BMXT_REGBASE_45216; pOff = BMXT_REGOFFSETS_4538; pStep = BMXT_STEPSIZE_4538; break;

        /* 45308-family */
        case BMXT_Chip_e45308: pNum = BMXT_NUMELEM_45308; base = BMXT_REGBASE_45308; pOff = BMXT_REGOFFSETS_45308; pStep = BMXT_STEPSIZE_45308;
            pNumDcbg = BMXT_NUMELEM_DCBG_45308; baseDcbg = BMXT_REGBASE_DCBG_45308; pOffDcbg = BMXT_REGOFFSETS_DCBG_45308; pStepDcbg = BMXT_STEPSIZE_DCBG_45308; break;

        /* 3158-family */
        case BMXT_Chip_e3158:  pNum = BMXT_NUMELEM_3158;  base = BMXT_REGBASE_3158;  pOff = BMXT_REGOFFSETS_3158;  pStep = BMXT_STEPSIZE_3158;
            baseWakeup = BMXT_REGBASE_WAKEUP_3158; pOffWakeup = BMXT_REGOFFSETS_WAKEUP; break;
        case BMXT_Chip_e3390:
            if (rev < BMXT_ChipRev_eB0) {
                pNum = BMXT_NUMELEM_3384;  base = BMXT_REGBASE_3384;  pOff = BMXT_REGOFFSETS_4538; pStep = BMXT_STEPSIZE_4538; break; /* 4538-family */
            }
            else {
                pNum = BMXT_NUMELEM_3158;  base = BMXT_REGBASE_3384;  pOff = BMXT_REGOFFSETS_3158; pStep = BMXT_STEPSIZE_3158; break; /* 3158-family, but assume previous regbase */
            }
        default: BERR_TRACE(BERR_INVALID_PARAMETER); break;
    }

    /* overrides */
    if ((chip==BMXT_Chip_e7145) && (rev<BMXT_ChipRev_eB0)) { pNum = BMXT_NUMELEM_NULL; } /* 7145 A0 has no DEMOD_XPT_FE block */

    switch (chip) {
        case BMXT_Chip_e3383:
        case BMXT_Chip_e3384:
        case BMXT_Chip_e3390:
        case BMXT_Chip_e7145: /* 7145A0 has no DEMOD_XPT_FE block, but 7145B0 does. but 7145 has always gone through RPC for slave device reg access. so keep assuming RPC for now */
            type = BMXT_P_PlatformType_eRpc; break;
        case BMXT_Chip_e7366:
        case BMXT_Chip_e7364:
            type = BMXT_P_PlatformType_eReg; break;
        default:
            type = BMXT_P_PlatformType_eHab; break;
    }

    if (mxt) {
        mxt->platform.num = (uint32_t*)pNum;
        mxt->platform.regbase = base;
        mxt->platform.regoffsets = (uint32_t*)pOff;
        mxt->platform.stepsize = (uint32_t*)pStep;
        mxt->platform.type = type;

        mxt->platform.regbaseWakeup = baseWakeup;
        mxt->platform.regoffsetsWakeup = (uint32_t*)pOffWakeup;

        mxt->platform.numDcbg = (uint32_t*)pNumDcbg;
        mxt->platform.regbaseDcbg = baseDcbg;
        mxt->platform.regoffsetsDcbg = (uint32_t*)pOffDcbg;
        mxt->platform.stepsizeDcbg = (uint32_t*)pStepDcbg;
    }
    else {
        *pNumElem = (uint32_t*)pNum;
    }
}

unsigned BMXT_GetNumResources(BMXT_Chip chip, BMXT_ChipRev rev, BMXT_ResourceType resource)
{
    unsigned *ptr;
    if (resource >= BMXT_ResourceType_eMax) {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return 0;
    }
    BMXT_P_SetPlatform(NULL, chip, rev, &ptr);
    return ptr[resource];
}

/* these are DEPRECATED */
void BMXT_3128_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e3128;
}

void BMXT_3383_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e3383;
}

void BMXT_3472_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e3472;
}

void BMXT_4517_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e4517;
}

void BMXT_4528_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e4528;
}

void BMXT_4538_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e4538;
}

void BMXT_3384_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e3384;
}

void BMXT_7366_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e7366;
}

void BMXT_7145_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e7145;
}

void BMXT_4548_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e4548;
}

void BMXT_45216_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e45216;
}

void BMXT_7364_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e7364;
}

void BMXT_45308_GetDefaultSettings(BMXT_Settings *pSettings)
{
    BMXT_GetDefaultSettings(pSettings);
    pSettings->chip = BMXT_Chip_e45308;
}
