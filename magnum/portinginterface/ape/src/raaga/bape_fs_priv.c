/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"

BDBG_MODULE(bape_fs_priv);

#if BAPE_CHIP_MAX_FS > 0

#ifdef BCHP_AUD_FMM_OP_CTRL_REG_START
#include "bchp_aud_fmm_op_ctrl.h"
#include "bchp_aud_fmm_iop_ctrl.h"
#endif

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

    BDBG_MSG(("Updated fs %u MCLK_CFG (0x%08X) to 0x%08X", fsIndex, regAddr, regVal));
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
