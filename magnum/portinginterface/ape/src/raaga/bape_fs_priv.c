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

BDBG_MODULE(bape_fs_priv);

#ifdef BCHP_AUD_FMM_OP_CTRL_REG_START
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_iop_ctrl.h"

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
    if ( fs >= BAPE_CHIP_MAX_FS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(fs < BAPE_CHIP_MAX_FS);
        return;
    }
    BDBG_ASSERT(handle->fsAllocated[fs] == true);
    handle->fsAllocated[fs] = false;
}

void BAPE_P_SetFsTiming_isr(BAPE_Handle handle, unsigned fsIndex, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    uint32_t    regVal, regAddr;
    uint32_t    pllclksel=0;
    uint32_t    mclkRate=0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    if ( fsIndex >= BAPE_CHIP_MAX_FS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(fsIndex < BAPE_CHIP_MAX_FS);
        return;
    }
    BDBG_ASSERT(handle->fsAllocated[fsIndex] == true);
    BDBG_ASSERT(mclkSource < BAPE_MclkSource_eMax);
    BDBG_ASSERT(pllChannel < BAPE_CHIP_MAX_PLLS );

    switch ( mclkSource )
    {
        /* PLL Timing */
    #if BAPE_CHIP_MAX_PLLS > 0
        case BAPE_MclkSource_ePll0:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_PLL0_ch1 + pllChannel;
            break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 1
        case BAPE_MclkSource_ePll1:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_PLL1_ch1 + pllChannel;
            break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 2
        case BAPE_MclkSource_ePll2:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_PLL2_ch1 + pllChannel;
            break;
    #endif

        /* DAC Timing (pllChannel doesn't apply for DACs) */
    #if BAPE_CHIP_MAX_DACS > 0
        case BAPE_MclkSource_eHifidac0:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_Hifidac0;
            break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 1
        case BAPE_MclkSource_eHifidac1:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_Hifidac1;
            break;
    #endif
    #if BAPE_CHIP_MAX_DACS > 2
        case BAPE_MclkSource_eHifidac2:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_Hifidac2;
            break;
    #endif

        /* NCO (Mclkgen) Timing (pllChannel doesn't apply for NCOs) */
    #if BAPE_CHIP_MAX_NCOS > 0
        case BAPE_MclkSource_eNco0:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_Mclk_gen0;
            break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 1
        case BAPE_MclkSource_eNco1:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_Mclk_gen1;
            break;
    #endif
    #if BAPE_CHIP_MAX_NCOS > 2
        case BAPE_MclkSource_eNco2:
            pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_PLLCLKSEL_Mclk_gen2;
            break;
    #endif

        /* Should never get here */
        default:
            BDBG_ERR(("mclkSource (%u) doesn't refer to a valid PLL or DAC", mclkSource));
            BDBG_ASSERT(false);     /* something went wrong somewhere! */
            return;
    }

    mclkRate = mclkFreqToFsRatio / 128;

    regAddr = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_BASE + ((BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_ELEMENT_SIZE * fsIndex)/8);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);

    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, PLLCLKSEL));
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, MCLK_RATE));

    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, PLLCLKSEL, pllclksel);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_FSi, MCLK_RATE, mclkRate);        

    BDBG_MSG(("Updated fs %u MCLK_CFG (0x%08X) to 0x%08X", fsIndex, regVal));
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}

#else
/* Stubs for newer chips that don't have the FS clock mux */
unsigned BAPE_P_AllocateFs(BAPE_Handle handle)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ERR(("This chipset does not support Fs timing"));
    BDBG_ASSERT(false);
    return (unsigned)-1;
}

void BAPE_P_FreeFs(BAPE_Handle handle, unsigned fs)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BSTD_UNUSED(fs);
    BDBG_ERR(("This chipset does not support Fs timing"));
    BDBG_ASSERT(false);
}

void BAPE_P_SetFsTiming_isr(BAPE_Handle handle, unsigned fsIndex, BAPE_MclkSource mclkSource, unsigned pllChannel, unsigned mclkFreqToFsRatio)
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BSTD_UNUSED(fsIndex);
    BSTD_UNUSED(mclkSource);
    BSTD_UNUSED(pllChannel);
    BSTD_UNUSED(mclkFreqToFsRatio);
    BDBG_ERR(("This chipset does not support Fs timing"));
    BDBG_ASSERT(false);
}
#endif

