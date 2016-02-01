/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bchp.h"
#include "bkni.h"
#include "bmxt_priv.h"
#include "bmxt.h"
#include "bmxt_rdb.h"
#include "bmxt_rdb_wakeup.h"

BDBG_MODULE(bmxt);

void BMXT_P_SetPlatform(
    /* you specify one or the other */
    BMXT_Handle mxt,
    BMXT_Chip chip, BMXT_ChipRev rev, unsigned **pNumElem
    )
{
    const uint32_t *pNum = 0, *pOff = 0, *pStep = 0, *pOffWakeup = 0;
    uint32_t base = 0, baseWakeup = 0;
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
        case BMXT_Chip_e45308: pNum = BMXT_NUMELEM_45308; base = BMXT_REGBASE_45308; pOff = BMXT_REGOFFSETS_45308; pStep = BMXT_STEPSIZE_45308; break;

        /* 3158-family */
        case BMXT_Chip_e3158:  pNum = BMXT_NUMELEM_3158;  base = BMXT_REGBASE_3158;  pOff = BMXT_REGOFFSETS_3158;  pStep = BMXT_STEPSIZE_3158;
            baseWakeup = BMXT_REGBASE_WAKEUP_3158; pOffWakeup = BMXT_REGOFFSETS_WAKEUP; break;
        default: BERR_TRACE(BERR_INVALID_PARAMETER); break;
    }

    /* overrides */
    if ((chip==BMXT_Chip_e7145) && (rev<BMXT_ChipRev_eB0)) { pNum = BMXT_NUMELEM_NULL; } /* 7145 A0 has no DEMOD_XPT_FE block */

    switch (chip) {
        case BMXT_Chip_e3383:
        case BMXT_Chip_e3384:
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
