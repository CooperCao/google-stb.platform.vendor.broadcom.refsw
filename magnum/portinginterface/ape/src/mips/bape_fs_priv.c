/***************************************************************************
 *     Copyright (c) 2006-2010, Broadcom Corporation
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
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_iop_ctrl.h"

BDBG_MODULE(bape_fs_priv);

unsigned BAPE_P_AllocateFs(BAPE_Handle handle)
{
    unsigned i;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    for ( i = 0; i < BAPE_CHIP_MAX_FS; i++ )
    {
        if ( !handle->fsAllocated[i] )
        {
            handle->fsAllocated[i] = true;
            return i;
        }
    }

    return BAPE_FS_INVALID;
}

void BAPE_P_FreeFs(BAPE_Handle handle, unsigned fs)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(fs < BAPE_CHIP_MAX_FS);
    BDBG_ASSERT(handle->fsAllocated[fs] == true);
    handle->fsAllocated[fs] = false;
}

void BAPE_P_SetFsTiming_isr(BAPE_Handle handle, unsigned fs, BAPE_MclkSource clkSource, unsigned multiplier)
{
    uint32_t regVal, regAddr;
    uint32_t pllclksel=0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(fs < BAPE_CHIP_MAX_FS);
    BDBG_ASSERT(handle->fsAllocated[fs] == true);
    BDBG_ASSERT(clkSource < BAPE_MclkSource_eMax);
    BDBG_ASSERT(multiplier == 1 || multiplier == 2 || multiplier == 4);

    regAddr = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_BASE + ((BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_ELEMENT_SIZE * fs)/8);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, PLLCLKSEL)|BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, MCLK_RATE));
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, MCLK_RATE, multiplier);        
    if ( clkSource >= BAPE_MclkSource_ePll0 && clkSource <= BAPE_MclkSource_ePll2 )
    {
        unsigned pll = clkSource - BAPE_MclkSource_ePll0;

        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);

        switch ( multiplier )
        {
        case 1:
            pllclksel = 3*pll;
            break;
        case 2:
            pllclksel = 1+(3*pll);
            break;
        case 4:
            pllclksel = 2+(3*pll);
            break;
        default:
            BDBG_ASSERT(multiplier == 1 || multiplier == 2 || multiplier == 4);
            return;
        }
    }
    else
    {
        /* TODO: Expand with more DACs */
        BDBG_ASSERT(clkSource == BAPE_MclkSource_eHifidac0);
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_Hifidac0;
    }
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, PLLCLKSEL, pllclksel);
    BDBG_MSG(("Updated fs %u MCLK_CFG to 0x%x", fs, regVal));
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}

