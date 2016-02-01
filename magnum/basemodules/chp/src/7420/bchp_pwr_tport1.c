/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 *   See Module Overview below.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
***************************************************************************/

#include "bchp_ctk.h"

static void BCHP_PWR_P_HW_TSP_Control(BCHP_Handle handle, bool activate)
{
    uint32_t val, mask;
    BDBG_MSG(("HW_CTK: %s", activate?"on":"off"));

    val = BREG_Read32(handle->regHandle, BCHP_CTK_AFE_CTRL1);
    mask = BCHP_CTK_AFE_CTRL1_biaspwrdn_MASK |
           BCHP_CTK_AFE_CTRL1_rxpwrdn_MASK |
           BCHP_CTK_AFE_CTRL1_txpwrdn_MASK;
    if (activate) {
        val &= ~mask;
    }
    else {
        val |= mask;
    }
    BREG_Write32(handle->regHandle, BCHP_CTK_AFE_CTRL1, val);

    if (activate) {
        val = BREG_Read32(handle->regHandle, BCHP_CTK_AFE_CTRL3);
        val |= BCHP_CTK_AFE_CTRL3_clkenable_MASK;
        BREG_Write32(handle->regHandle, BCHP_CTK_AFE_CTRL3, val);

        BKNI_Delay(1); /* wait at least 300ns */

        val = BREG_Read32(handle->regHandle, BCHP_CTK_AFE_CTRL3);
        val |= BCHP_CTK_AFE_CTRL3_clkresetb_MASK;
        BREG_Write32(handle->regHandle, BCHP_CTK_AFE_CTRL3, val);
    }
    else {
        val = BREG_Read32(handle->regHandle, BCHP_CTK_AFE_CTRL3);
        val &= (~BCHP_CTK_AFE_CTRL3_clkenable_MASK) &
               (~BCHP_CTK_AFE_CTRL3_clkresetb_MASK);
        BREG_Write32(handle->regHandle, BCHP_CTK_AFE_CTRL3, val);
    }
}

