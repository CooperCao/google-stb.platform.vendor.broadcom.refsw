/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if BCHP_CLKGEN_REG_START
#include "bchp_clkgen.h"
#endif

BDBG_MODULE(bape_pll);

typedef struct {
    uint32_t baseFs;
    uint32_t freqCh1;
    uint32_t ndivInt;
    uint32_t MdivCh0;
    uint32_t MdivCh1;
    uint32_t MdivCh2;
} BAPE_PllDescriptor;

static BERR_Code BAPE_P_ProgramPll_isr(BAPE_Handle handle, BAPE_Pll pll, BAPE_PllDescriptor *pDescriptor);
static void BAPE_Pll_UpdateDividers_isr(BAPE_Handle handle, BAPE_Pll pll, uint32_t ndivInt, uint32_t MdivCh0, uint32_t MdivCh1, uint32_t MdivCh2);

#define BAPE_P_USE_PLL_MACROS       1
#define BAPE_PLL_MAX_NDIV           255
#define BAPE_PLL_MAX_MDIV           255

#if BCHP_AUD_FMM_PLL0_REG_START

/* Code below is for non-DTV chips that use the standard PLL macros */

#include "bchp_aud_fmm_pll0.h"
#if BAPE_CHIP_MAX_PLLS > 1
#include "bchp_aud_fmm_pll1.h"
#define BAPE_PLL_STRIDE (BCHP_AUD_FMM_PLL1_MACRO-BCHP_AUD_FMM_PLL0_MACRO)
#if BAPE_CHIP_MAX_PLLS > 2
#include "bchp_aud_fmm_pll2.h"
#endif
#else
#define BAPE_PLL_STRIDE 0
#endif

#if BAPE_P_USE_PLL_MACROS
static void BAPE_Pll_UpdateMacro_isr(BAPE_Handle handle, BAPE_Pll pll, uint32_t baseRate)
{
    uint32_t regAddr, regVal;
    
    /* AUD_FMM_PLL1_MACRO.MACRO_SELECT = User */
    regAddr = BCHP_AUD_FMM_PLL0_MACRO + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT);
    switch ( baseRate )
    {
        case  32000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT,  Mult_of_32000);
            break;
        case  44100:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT,  Mult_of_44100);
            break;
        case  48000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT,  Mult_of_48000);
            break;
        case  96000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT,  Mult_of_96000);
            break;
        case 192000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_192000);
            break;
        default:
            BDBG_ERR(("%s : Invalid baseRate %u", __FUNCTION__, baseRate));
            return;
            break;
    }
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}
#endif /* BAPE_P_USE_PLL_MACROS */

static void BAPE_Pll_UpdateDividers_isr(BAPE_Handle handle, BAPE_Pll pll, uint32_t ndivInt, uint32_t MdivCh0, uint32_t MdivCh1, uint32_t MdivCh2)
{
    uint32_t regAddr, regVal;
    
    /* AUD_FMM_PLL1_MACRO.MACRO_SELECT = User */
    regAddr = BCHP_AUD_FMM_PLL0_MACRO + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_PLL0_MACRO, MACRO_SELECT, User);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_CONTROL_0.USER_UPDATE_DIVIDERS = 0 */
    regAddr = BCHP_AUD_FMM_PLL0_CONTROL_0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_CONTROL_0, USER_UPDATE_DIVIDERS);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_USER_NDIV.NDIV_INT = ndivInt */
    regAddr = BCHP_AUD_FMM_PLL0_USER_NDIV + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_NDIV, NDIV_INT);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_USER_NDIV, NDIV_INT, ndivInt);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_CONTROL_0.USER_UPDATE_DIVIDERS = 1 */
    regAddr = BCHP_AUD_FMM_PLL0_CONTROL_0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_CONTROL_0, USER_UPDATE_DIVIDERS);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_CONTROL_0, USER_UPDATE_DIVIDERS, 1 );
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch0.MDIV = MdivCh0 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch0, MDIV);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_USER_MDIV_Ch0, MDIV, MdivCh0);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch0.LOAD_EN = 1 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_USER_MDIV_Ch0, LOAD_EN, 1 );
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch0.LOAD_EN = 0 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch0, LOAD_EN);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch1.MDIV = MdivCh1 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch1 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch1, MDIV);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_USER_MDIV_Ch1, MDIV, MdivCh1);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch1.LOAD_EN = 1 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch1 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch1, LOAD_EN);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_USER_MDIV_Ch1, LOAD_EN, 1 );
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch1.LOAD_EN = 0 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch1 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch1, LOAD_EN);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch2.MDIV = MdivCh2 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch2 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch2, MDIV);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_USER_MDIV_Ch2, MDIV, MdivCh2);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch2.LOAD_EN = 1 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch2 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch2, LOAD_EN);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_PLL0_USER_MDIV_Ch2, LOAD_EN, 1 );
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
    
    /* AUD_FMM_PLL0_USER_MDIV_Ch2.LOAD_EN = 0 */
    regAddr = BCHP_AUD_FMM_PLL0_USER_MDIV_Ch2 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_PLL0_USER_MDIV_Ch2, LOAD_EN);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}

#else

#include "bchp_aud_fmm_iop_pll_0.h"

#ifdef BCHP_AUD_FMM_IOP_PLL_1_REG_START
#include "bchp_aud_fmm_iop_pll_1.h"
#define BAPE_PLL_STRIDE (BCHP_AUD_FMM_IOP_PLL_1_REG_START-BCHP_AUD_FMM_IOP_PLL_0_REG_START)
#else
#define BAPE_PLL_STRIDE 0
#endif

#if BAPE_P_USE_PLL_MACROS
static void BAPE_Pll_UpdateMacro_isr(BAPE_Handle handle, BAPE_Pll pll, uint32_t baseRate)
{
    uint32_t regAddr, regVal;
    
    /* AUD_FMM_PLL1_MACRO.MACRO_SELECT = User */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_MACRO + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT);
    switch ( baseRate )
    {
        case  32000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT,  Mult_of_32000);
            break;
        case  44100:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT,  Mult_of_44100);
            break;
        case  48000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT,  Mult_of_48000);
            break;
        case  96000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT,  Mult_of_96000);
            break;
        case 192000:
            regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT, Mult_of_192000);
            break;
        default:
            BDBG_ERR(("%s : Invalid baseRate %u", __FUNCTION__, baseRate));
            return;
            break;
    }
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
}
#endif /* BAPE_P_USE_PLL_MACROS */

static void BAPE_Pll_UpdateDividers_isr(BAPE_Handle handle, BAPE_Pll pll, uint32_t ndivInt, uint32_t MdivCh0, uint32_t MdivCh1, uint32_t MdivCh2)
{
    uint32_t regAddr, regVal;
    
    /* AUD_FMM_PLL1_MACRO.MACRO_SELECT = User */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_MACRO + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT);
    regVal |= BCHP_FIELD_ENUM(AUD_FMM_IOP_PLL_0_MACRO, MACRO_SELECT, User);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    /* AUD_FMM_IOP_PLL_0_CONTROL_0.USER_UPDATE_DIVIDERS = 0 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_CONTROL_0, USER_UPDATE_DIVIDERS);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    /* AUD_FMM_IOP_PLL_0_USER_NDIV.NDIV_INT = ndivInt */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_NDIV + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_NDIV, NDIV_INT);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_USER_NDIV, NDIV_INT, ndivInt);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    /* AUD_FMM_IOP_PLL_0_CONTROL_0.USER_UPDATE_DIVIDERS = 1 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_CONTROL_0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_CONTROL_0, USER_UPDATE_DIVIDERS);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_CONTROL_0, USER_UPDATE_DIVIDERS, 1 );
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);


    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0.MDIV = MdivCh0 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0, MDIV);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0, MDIV, MdivCh0);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

#if BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0_LOAD_EN_MASK
    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0.LOAD_EN = 1 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0, LOAD_EN, 1 );
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);

    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0.LOAD_EN = 0 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0, LOAD_EN);
    BREG_Write32_isr(handle->regHandle, regAddr, regVal);
#endif

    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1.MDIV = MdivCh1 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1, MDIV);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1, MDIV, MdivCh1);
    BREG_Write32(handle->regHandle, regAddr, regVal);

#if BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0_LOAD_EN_MASK
    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1.LOAD_EN = 1 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1, LOAD_EN);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1, LOAD_EN, 1 );
    BREG_Write32(handle->regHandle, regAddr, regVal);

    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1.LOAD_EN = 0 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch1, LOAD_EN);
    BREG_Write32(handle->regHandle, regAddr, regVal);
#endif

    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2.MDIV = MdivCh2 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2, MDIV);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2, MDIV, MdivCh2);
    BREG_Write32(handle->regHandle, regAddr, regVal);

#if BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch0_LOAD_EN_MASK
    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2.LOAD_EN = 1 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2, LOAD_EN);
    regVal |= BCHP_FIELD_DATA(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2, LOAD_EN, 1 );
    BREG_Write32(handle->regHandle, regAddr, regVal);

    /* AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2.LOAD_EN = 0 */
    regAddr = BCHP_AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2 + (BAPE_PLL_STRIDE * pll);
    regVal = BREG_Read32(handle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(AUD_FMM_IOP_PLL_0_USER_MDIV_Ch2, LOAD_EN);
    BREG_Write32(handle->regHandle, regAddr, regVal);
#endif
}

#endif

static BERR_Code BAPE_P_ProgramPll_isr(BAPE_Handle handle, BAPE_Pll pll, BAPE_PllDescriptor *pDescriptor)
{

#if BAPE_P_USE_PLL_MACROS
    BAPE_Pll_UpdateMacro_isr(handle, pll, pDescriptor->baseFs);
#else
    BAPE_Pll_UpdateDividers_isr(handle, pll, pDescriptor->ndivInt, pDescriptor->MdivCh0, pDescriptor->MdivCh1, pDescriptor->MdivCh2);
#endif

    return BERR_SUCCESS;
}

/* Common code */
void BAPE_Pll_GetSettings(
    BAPE_Handle handle,
    BAPE_Pll pll,
    BAPE_PllSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    if ( pll >= BAPE_CHIP_MAX_PLLS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
        return;
    }
    *pSettings = handle->audioPlls[pll].settings;
}

BERR_Code BAPE_Pll_SetSettings(
    BAPE_Handle handle,
    BAPE_Pll pll,
    const BAPE_PllSettings *pSettings
    )
{
    uint32_t data;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    if ( pll >= BAPE_CHIP_MAX_PLLS )
    {
        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_ASSERT(NULL != pSettings);
    handle->audioPlls[pll].settings = *pSettings;

#ifdef BCHP_CLKGEN_INTERNAL_MUX_SELECT
    /* VCXO source */
    switch (pSettings->vcxo)
    {
    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo0
    case 0:
        data = BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo0;
        break;
    #endif

    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo1
    case 1:
        data = BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo1;
        break;
    #endif

    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo2
    case 2:
        data = BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo2;
        break;
    #endif

    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo3
    case 3:
        data = BCHP_CLKGEN_INTERNAL_MUX_SELECT_REFERENCE_CLOCK_Vcxo3;
        break;
    #endif

    default:
        BDBG_ERR(("Invalid or unsupported audio VCXO: %u", pSettings->vcxo));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_EnterCriticalSection();
    /*regAddr = BCHP_CLKGEN_INTERNAL_MUX_SELECT;
    regVal = BREG_Read32_isr(handle->regHandle, regAddr);*/
    switch ( pll )
    {
#if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_PLLAUDIO0_REFERENCE_CLOCK_SHIFT
    case BAPE_Pll_e0:
        BAPE_Reg_P_UpdateField(handle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, CLKGEN_INTERNAL_MUX_SELECT, PLLAUDIO0_REFERENCE_CLOCK, data);
        break;
    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_PLLAUDIO1_REFERENCE_CLOCK_SHIFT && (BAPE_CHIP_MAX_PLLS>1)
    case BAPE_Pll_e1:
        BAPE_Reg_P_UpdateField(handle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, CLKGEN_INTERNAL_MUX_SELECT, PLLAUDIO1_REFERENCE_CLOCK, data);
        break;
    #endif
    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_PLLAUDIO2_REFERENCE_CLOCK_SHIFT && (BAPE_CHIP_MAX_PLLS>2)
    case BAPE_Pll_e2:
        BAPE_Reg_P_UpdateField(handle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, CLKGEN_INTERNAL_MUX_SELECT, PLLAUDIO2_REFERENCE_CLOCK, data);
        break;
    #endif
#elif defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_AUDIO0_OSCREF_CMOS_CLOCK_SHIFT
    case BAPE_Pll_e0:
        BAPE_Reg_P_UpdateField(handle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, CLKGEN_INTERNAL_MUX_SELECT, AUDIO0_OSCREF_CMOS_CLOCK, data);
        break;
    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_AUDIO1_OSCREF_CMOS_CLOCK_SHIFT && (BAPE_CHIP_MAX_PLLS>1)
    case BAPE_Pll_e1:
        BAPE_Reg_P_UpdateField(handle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, CLKGEN_INTERNAL_MUX_SELECT, AUDIO1_OSCREF_CMOS_CLOCK, data);
        break;
    #endif
    #if defined BCHP_CLKGEN_INTERNAL_MUX_SELECT_AUDIO2_OSCREF_CMOS_CLOCK_SHIFT && (BAPE_CHIP_MAX_PLLS>2)
    case BAPE_Pll_e2:
        BAPE_Reg_P_UpdateField(handle, BCHP_CLKGEN_INTERNAL_MUX_SELECT, CLKGEN_INTERNAL_MUX_SELECT, AUDIO2_OSCREF_CMOS_CLOCK, data);
        break;
    #endif
#else
    #warning "UNSUPPORTED CHIP - update this code"
#endif
    default:
        break;
    }
    BKNI_LeaveCriticalSection();
#endif

    if ( handle->audioPlls[pll].settings.mode == BAPE_PllMode_eCustom )
    {
        if ( handle->audioPlls[pll].settings.nDivInt > BAPE_PLL_MAX_NDIV )
        {
            BDBG_WRN(("%s: Warning : nDivInt %u is greater than BAPE_PLL_MAX_NDIV %u", __FUNCTION__, handle->audioPlls[pll].settings.nDivInt, BAPE_PLL_MAX_NDIV));
            handle->audioPlls[pll].settings.nDivInt = BAPE_PLL_MAX_NDIV;
        }

        if ( handle->audioPlls[pll].settings.mDivCh0 > BAPE_PLL_MAX_MDIV )
        {
            BDBG_WRN(("%s: Warning : nDivInt %u is greater than BAPE_PLL_MAX_MDIV %u", __FUNCTION__, handle->audioPlls[pll].settings.mDivCh0, BAPE_PLL_MAX_MDIV));
            handle->audioPlls[pll].settings.mDivCh0 = BAPE_PLL_MAX_MDIV;
        }

        BKNI_EnterCriticalSection();
        BAPE_Pll_UpdateDividers_isr(handle,
                                    pll,
                                    handle->audioPlls[pll].settings.nDivInt,
                                    handle->audioPlls[pll].settings.mDivCh0,
                                    handle->audioPlls[pll].settings.mDivCh0,
                                    handle->audioPlls[pll].settings.mDivCh0);
        BKNI_LeaveCriticalSection();
    }

    return BERR_SUCCESS;
}

void BAPE_P_AttachMixerToPll(BAPE_MixerHandle mixer, BAPE_Pll pll)
{
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    if ( pll >= BAPE_CHIP_MAX_PLLS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
        return;
    }
    BLST_S_INSERT_HEAD(&mixer->deviceHandle->audioPlls[pll].mixerList, mixer, pllNode);
    /* Update MCLK source for attached outputs */
    BKNI_EnterCriticalSection();
    BAPE_P_UpdatePll_isr(mixer->deviceHandle, pll);
    BKNI_LeaveCriticalSection();
}

void BAPE_P_DetachMixerFromPll(BAPE_MixerHandle mixer, BAPE_Pll pll)
{
    BDBG_OBJECT_ASSERT(mixer, BAPE_Mixer);
    if ( pll >= BAPE_CHIP_MAX_PLLS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
        return;
    }
    BLST_S_REMOVE(&mixer->deviceHandle->audioPlls[pll].mixerList, mixer, BAPE_Mixer, pllNode);
}

void BAPE_P_AttachInputPortToPll(BAPE_InputPort input, BAPE_Pll pll)
{
    BKNI_EnterCriticalSection();
    BAPE_P_AttachInputPortToPll_isrsafe(input, pll);
    BKNI_LeaveCriticalSection();
}

void BAPE_P_AttachInputPortToPll_isrsafe(BAPE_InputPort input, BAPE_Pll pll)
{
    unsigned pllIndex = pll - BAPE_Pll_e0;
    BAPE_Handle deviceHandle;
    BAPE_I2sInputHandle i2sHandle;
    BAPE_MaiInputHandle maiHandle;
    BAPE_SpdifInputHandle spdifHandle;

    BDBG_OBJECT_ASSERT(input, BAPE_InputPort);

    if ( pllIndex >= BAPE_CHIP_MAX_PLLS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(pllIndex < BAPE_CHIP_MAX_PLLS);
        return;
    }

    BDBG_MSG(("Attaching InputPort(%s) %p to PLL:%u", input->pName,(void *)input, pllIndex ));
    switch (input->type)
    {
    case BAPE_InputPortType_eI2s:
        i2sHandle = input->pHandle;
        deviceHandle = i2sHandle->deviceHandle;
        break;
    case BAPE_InputPortType_eMai:
        maiHandle = input->pHandle;
        deviceHandle = maiHandle->deviceHandle;
        break;
    case BAPE_InputPortType_eSpdif:
        spdifHandle = input->pHandle;
        deviceHandle = spdifHandle->deviceHandle;
        break;
    default:
        BDBG_ERR(("Invalid InputPort Type (%d)", input->type));
        return;
    }

    BLST_S_INSERT_HEAD(&deviceHandle->audioPlls[pllIndex].inputList, input, pllNode);
    /* Update MCLK source for attached outputs */
    BAPE_P_UpdatePll_isr(deviceHandle, pll);
}

void BAPE_P_DetachInputPortFromPll_isrsafe(BAPE_InputPort input, BAPE_Pll pll)
{

    unsigned pllIndex = pll - BAPE_Pll_e0;
    BAPE_Handle deviceHandle;
    BAPE_I2sInputHandle i2sHandle;
    BAPE_MaiInputHandle maiHandle;
    BAPE_SpdifInputHandle spdifHandle;

    BDBG_OBJECT_ASSERT(input, BAPE_InputPort);
    if ( pllIndex >= BAPE_CHIP_MAX_PLLS )
    {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ASSERT(pllIndex < BAPE_CHIP_MAX_PLLS);
        return;
    }
    BDBG_MSG(("Detaching InputPort(%s) %p from PLL:%u", input->pName, (void *)input, pllIndex ));
    switch (input->type)
    {
    case BAPE_InputPortType_eI2s:
        i2sHandle = input->pHandle;
        deviceHandle = i2sHandle->deviceHandle;
        break;
    case BAPE_InputPortType_eMai:
        maiHandle = input->pHandle;
        deviceHandle = maiHandle->deviceHandle;
        break;
    case BAPE_InputPortType_eSpdif:
        spdifHandle = input->pHandle;
        deviceHandle = spdifHandle->deviceHandle;
        break;
    default:
        BDBG_ERR(("Invalid InputPort Type (%d)", input->type));
        return;
    }

    BLST_S_REMOVE(&deviceHandle->audioPlls[pllIndex].inputList, input, BAPE_InputPortObject, pllNode);
}

static BERR_Code BAPE_P_GetPllBaseSampleRate_isr(unsigned sampleRate, unsigned *pBaseRate)
{
    switch ( sampleRate )
    {
    case 32000:    /* 32K Sample rate */
    case 64000:      /* 64K Sample rate */
    case 128000:     /* 128K Sample rate */
        *pBaseRate = 32000;
        return BERR_SUCCESS;
    case 44100:    /* 44.1K Sample rate */
    case 88200:    /* 88.2K Sample rate */
    case 176400:   /* 176.4K Sample rate */
        *pBaseRate = 44100;
        return BERR_SUCCESS;
    case 48000:      /* 48K Sample rate */
    case 96000:      /* 96K Sample rate */
    case 192000:     /* 192K Sample rate */
        *pBaseRate = 48000;
        return BERR_SUCCESS;
    default:
        BDBG_ERR(("Invalid sampling rate %u", sampleRate));
        *pBaseRate = 0;
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
}

static BERR_Code BAPE_P_SetPllFreq_isr( BAPE_Handle handle, BAPE_Pll pll, unsigned baseRate )
{
    int i;

    /* 40 nm values clock values - note that these are only used when programming
       the clocks manually using "BAPE_Pll_UpdateDividers_isr".  When using 
       "BAPE_Pll_UpdateMacro_isr", the hardware calculates and programs internally */
    BAPE_PllDescriptor pllInfo[] = 
    {
        #if BAPE_BASE_PLL_TO_FS_RATIO == 128    /* Run PLL Ch0 at 128 BaseFS */
            {  32000,               4096000,         64,          180,          180,       90 },
            {  44100,               5644800,         49,          100,          100,       50 }, 
            {  48000,               6144000,         64,          120,          120,       60 }, 
    
        #elif BAPE_BASE_PLL_TO_FS_RATIO == 256  /* Run PLL Ch0 at 256 BaseFS */
            {  32000,               8192000,         64,           90,           90,       45 },
            {  44100,              11289600,         49,           50,           50,       25 }, 
            {  48000,              12288000,         64,           60,           60,       30 }, 
    
        #elif BAPE_BASE_PLL_TO_FS_RATIO == 512  /* Run PLL Ch0 at 512 BaseFS */
            {  32000,              16384000,        128,           90,           90,       45 },
            {  44100,              22579200,         98,           50,           50,       25 }, 
            {  48000,              24576000,        128,           60,           60,       30 }
        #else
            #error "BAPE_BASE_PLL_TO_FS_RATIO is invalid or not defined"
        #endif
    };
    int numElems = sizeof pllInfo/sizeof pllInfo[0];

    for ( i=0 ; i<numElems ; i++  )
    {
        if ( pllInfo[i].baseFs ==  baseRate )
        {
            break;
        }
    }

    if ( i >= numElems )
    {
        BDBG_ERR(("Invalid sampling rate %u", baseRate));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(pllInfo[i].baseFs ==  baseRate);

    BDBG_MSG(("Setting PLL %u frequency to %u Hz (%u * BaseFs)", pll,  pllInfo[i].freqCh1, BAPE_BASE_PLL_TO_FS_RATIO));

    BAPE_P_ProgramPll_isr(handle, pll, &(pllInfo[i]));
    handle->audioPlls[pll].baseSampleRate   = baseRate;
    handle->audioPlls[pll].freqCh1          = pllInfo[i].freqCh1;

    return BERR_SUCCESS;
}


void BAPE_P_VerifyPllCallback_isr(void *pParam1, int param2)
{
    BAPE_Handle handle = pParam1;
    BAPE_Pll pll = param2;
    BAPE_Mixer *pMixer;
    BAPE_InputPortObject *pInputPort;
    BERR_Code errCode;
    unsigned baseRate = 0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);

    /* Walk through each mixer and make sure we have no conflicts */
    for ( pMixer = BLST_S_FIRST(&handle->audioPlls[pll].mixerList);
          pMixer != NULL;
          pMixer = BLST_S_NEXT(pMixer, pllNode) )
    {        
        unsigned mixerRate;

        if ( BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) == 0 )
        {
            continue;
        }

        errCode = BAPE_P_GetPllBaseSampleRate_isr(BAPE_Mixer_P_GetOutputSampleRate(pMixer), &mixerRate);
        if ( errCode )
        {
            break;
        }
        if ( pMixer->running )
        {
            if ( baseRate == 0 )
            {
                baseRate = mixerRate;
            }
            else if ( baseRate != mixerRate )
            {
                BDBG_WRN(("Sample rate conflict on PLL %d.  One mixer requests %u another requests %u", pll, baseRate, mixerRate));
            }
        }
    }

    /* Walk through each InputPort and make sure we have no conflicts */
    for ( pInputPort = BLST_S_FIRST(&handle->audioPlls[pll].inputList);
          pInputPort != NULL;
          pInputPort = BLST_S_NEXT(pInputPort, pllNode) )
    {
        unsigned mixerRate;

        if (pInputPort->format.sampleRate == 0)
        {
            continue;
        }

        mixerRate = pInputPort->format.sampleRate;

        if ( baseRate == 0 )
        {
            baseRate = mixerRate;
        }
        else if ( baseRate != mixerRate )
        {
            BDBG_WRN(("Sample rate conflict on PLL %d.  One mixer requests %u, and InputPort(%s) %u", pll, baseRate, pInputPort->pName, mixerRate));
        }
    }
}

BERR_Code BAPE_P_UpdatePll_isr(BAPE_Handle handle, BAPE_Pll pll)
{
    unsigned baseRate = 0;
    unsigned idleRate = 0;
    BAPE_Mixer *pMixer;
    BAPE_InputPortObject *pInputPort;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    if ( pll >= BAPE_CHIP_MAX_PLLS )
    {
        BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Walk through each mixer and make sure we have no conflicts */
    for ( pMixer = BLST_S_FIRST(&handle->audioPlls[pll].mixerList);
          pMixer != NULL;
          pMixer = BLST_S_NEXT(pMixer, pllNode) )
    {
        unsigned mixerRate;

        if ( BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) == 0 )
        {
            continue;
        }

        errCode = BAPE_P_GetPllBaseSampleRate_isr(BAPE_Mixer_P_GetOutputSampleRate(pMixer), &mixerRate);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        if (pMixer->pathNode.subtype == BAPE_MixerType_eDsp)
        {
            mixerRate = 48000;
        }

        if ( pMixer->running )
        {
            if ( baseRate == 0 )
            {
                baseRate = mixerRate;
            }
            else if ( baseRate != mixerRate )
            {
                errCode = BTMR_StartTimer_isr(handle->pllTimer[pll], 100*100);
            }
        }
        else if ( idleRate == 0 )
        {
            idleRate = mixerRate;
        }
    }

    if ( baseRate == 0 )
    {
        baseRate = idleRate;
    }

    /* Walk through each InputPort and make sure we have no conflicts */
    for ( pInputPort = BLST_S_FIRST(&handle->audioPlls[pll].inputList);
          pInputPort != NULL;
          pInputPort = BLST_S_NEXT(pInputPort, pllNode) )
    {
        unsigned mixerRate;

        if (pInputPort->format.sampleRate == 0)
        {
            continue;
        }

        errCode = BAPE_P_GetPllBaseSampleRate_isr(pInputPort->format.sampleRate, &mixerRate);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        if ( baseRate == 0 )
        {
            baseRate = mixerRate;
        }
        else if ( baseRate != mixerRate )
        {
            errCode = BTMR_StartTimer_isr(handle->pllTimer[pll], 100*100);
        }
    }

    if ( baseRate != 0 )
    {
        BDBG_MSG(("Updating PLL %u for base sample rate of %u Hz", pll, baseRate));

        errCode = BAPE_P_SetPllFreq_isr( handle, pll, baseRate );
        if ( errCode )  return BERR_TRACE(errCode);

        /* For each output, set it's mclk appropriately */
        for ( pMixer = BLST_S_FIRST(&handle->audioPlls[pll].mixerList);
              pMixer != NULL;
              pMixer = BLST_S_NEXT(pMixer, pllNode) )
        {
            BAPE_OutputPort output;
            unsigned rateNum = BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer);
            unsigned pllChan;
            BAPE_MclkSource mclkSource;

            BDBG_ASSERT( BAPE_MCLKSOURCE_IS_PLL(pMixer->mclkSource));

            if ( BAPE_Mixer_P_GetOutputSampleRate_isr(pMixer) == 0 )
            {
                /* Skip this mixer if it doesn't have a sample rate yet */
                continue;
            }

            switch ( pll )
            {
            default:
            case BAPE_Pll_e0:
                mclkSource = BAPE_MclkSource_ePll0;
                break;
            case BAPE_Pll_e1:
                mclkSource = BAPE_MclkSource_ePll1;
                break;
            case BAPE_Pll_e2:
                mclkSource = BAPE_MclkSource_ePll2;
                break;
            }
            BDBG_ASSERT( mclkSource == pMixer->mclkSource);

            pllChan =  rateNum  <= 48000 ? 0 :        /* Channel 0 runs at 32 KHz, 44.1 KHz, or 48 KHz    */
                       rateNum  <= 96000 ? 1 :        /* Channel 1 runs at 64 KHz, 88.2 KHz, or 96 KHz    */
                                           2 ;        /* Channel 2 runs at 128 KHz, 176.4 KHz, or 192 KHz */
                                                             
            for ( output = BLST_S_FIRST(&pMixer->outputList);
                  output != NULL;
                  output = BLST_S_NEXT(output, node) )
            {
                if ( output->setMclk_isr )
                {
                    BDBG_MSG(("Setting output mclk for '%s' to source:%s channel:%u ratio:%u",
                               output->pName, BAPE_Mixer_P_MclkSourceToText_isrsafe(mclkSource), pllChan, BAPE_BASE_PLL_TO_FS_RATIO));
                    output->setMclk_isr(output, mclkSource, pllChan, BAPE_BASE_PLL_TO_FS_RATIO);
                }
            }

            if ( pMixer->fs != BAPE_FS_INVALID )
            {
                BDBG_MSG(("Setting FS mclk for FS %u to source:%s ratio:%u",
                           pMixer->fs, BAPE_Mixer_P_MclkSourceToText_isrsafe(mclkSource), BAPE_BASE_PLL_TO_FS_RATIO));
                BAPE_P_SetFsTiming_isr(handle, pMixer->fs, mclkSource, pllChan, BAPE_BASE_PLL_TO_FS_RATIO);
            }
        }
    }
    else
    {
        BDBG_MSG(("Not updating PLL %u rate (unknown)", pll));
    }

    return BERR_SUCCESS;
}



BERR_Code BAPE_Pll_EnableExternalMclk(
    BAPE_Handle     handle,
    BAPE_Pll        pll,
    unsigned        mclkIndex,
    BAPE_MclkRate   mclkRate
    )
{
    unsigned            pllChannel;
    unsigned            pll0ToBaseFsRatio;   
    unsigned            requestedPllToBaseFsRatio;
    uint32_t            pllclksel;
    uint32_t            regAddr;
    uint32_t            regVal;

    BDBG_OBJECT_ASSERT(handle, BAPE_Device);
    BDBG_ASSERT(pll < BAPE_CHIP_MAX_PLLS);
    BDBG_ASSERT(mclkIndex < BAPE_CHIP_MAX_EXT_MCLKS);

    pll0ToBaseFsRatio = BAPE_BASE_PLL_TO_FS_RATIO;

    switch (mclkRate)
    {
    case BAPE_MclkRate_e128Fs:
        requestedPllToBaseFsRatio = 128;
        break;

    case BAPE_MclkRate_e256Fs:
        requestedPllToBaseFsRatio = 256;
        break;

    case BAPE_MclkRate_e384Fs:
        requestedPllToBaseFsRatio = 384;
        break;

    case BAPE_MclkRate_e512Fs:
        requestedPllToBaseFsRatio = 512;
        break;

    default:
        BDBG_ERR(("Requested mclkRate is invalid"));
        return(BERR_TRACE(BERR_INVALID_PARAMETER));
    }
    
    if (requestedPllToBaseFsRatio == pll0ToBaseFsRatio)
    {
        pllChannel = 0;
    }
    else if (requestedPllToBaseFsRatio == 2 * pll0ToBaseFsRatio)
    {
        pllChannel = 1;
    }
#if BAPE_BASE_PLL_TO_FS_RATIO < 256
    else if (requestedPllToBaseFsRatio == 4 * pll0ToBaseFsRatio)
    {
        pllChannel = 2;
    }
#endif
    else
    {
        BDBG_ERR(("Requested mclkRate:%dFs is invalid", requestedPllToBaseFsRatio));
        if ( requestedPllToBaseFsRatio < pll0ToBaseFsRatio )
        {
            BDBG_ERR(("Current minimum MCLK rate is %dFs... you may need to reduce BAPE_BASE_PLL_TO_FS_RATIO", pll0ToBaseFsRatio ));
        }
        return(BERR_TRACE(BERR_INVALID_PARAMETER));
    }
            
#if defined BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_BASE
    switch ( pll )
    {
    /* PLL Timing */
    #if BAPE_CHIP_MAX_PLLS > 0
    case BAPE_Pll_e0:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_PLLCLKSEL_PLL0_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 1
    case BAPE_Pll_e1:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_PLLCLKSEL_PLL1_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 2
    case BAPE_Pll_e2:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_PLLCLKSEL_PLL2_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 3
    case BAPE_Pll_e3:
        pllclksel = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_PLLCLKSEL_PLL3_ch1 + pllChannel;
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 4
        #error "Need to add support for more PLLs"
    #endif
    
    /* Should never get here */
    default:
        BDBG_ERR(("PLL is invalid"));
        BDBG_ASSERT(false);     /* something went wrong somewhere! */
        return(BERR_INVALID_PARAMETER);
    }

    /* Read the register. */
    regAddr = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_BASE + ((BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_ELEMENT_SIZE * mclkIndex)/8);
    regVal = BREG_Read32(handle->regHandle, regAddr);

    /* Clear the field that we're going to fill in. */
    regVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, PLLCLKSEL));

    /* Fill in the PLLCLKSEL field. */
    regVal |= BCHP_FIELD_DATA(AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, PLLCLKSEL, pllclksel);

    BREG_Write32(handle->regHandle, regAddr, regVal);

#elif defined BCHP_AUD_FMM_IOP_MISC_MCLK_CFG_i_ARRAY_BASE
    BSTD_UNUSED(regVal);
    BSTD_UNUSED(pllclksel);
    {
        BAPE_Reg_P_FieldList regFieldList;

        regAddr = BAPE_Reg_P_GetArrayAddress(AUD_FMM_IOP_MISC_MCLK_CFG_i, mclkIndex);
        BAPE_Reg_P_InitFieldList(handle, &regFieldList);
        switch ( pll ) 
        {
    #if BAPE_CHIP_MAX_PLLS > 0
    case BAPE_Pll_e0:
        if ( 0 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL0_ch1);
        }
        else if ( 1 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL0_ch2);
        }
        #if BAPE_BASE_PLL_TO_FS_RATIO < 256
        else if ( 2 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL0_ch3);
        }
        #endif        
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 1
    case BAPE_Pll_e1:
        if ( 0 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL1_ch1);
        }
        else if ( 1 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL1_ch2);
        }
        #if BAPE_BASE_PLL_TO_FS_RATIO < 256
        else if ( 2 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL1_ch3);
        }
        #endif        
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 2
    case BAPE_Pll_e2:
        if ( 0 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL2_ch1);
        }
        else if ( 1 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL2_ch2);
        }
        #if BAPE_BASE_PLL_TO_FS_RATIO < 256
        else if ( 2 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL2_ch3);
        }
        #endif        
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 3
    case BAPE_Pll_e3:
        if ( 0 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL3_ch1);
        }
        else if ( 1 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL3_ch2);
        }
        #if BAPE_BASE_PLL_TO_FS_RATIO < 256
        else if ( 2 == pllChannel )
        {
            BAPE_Reg_P_AddEnumToFieldList(&regFieldList, AUD_FMM_IOP_MISC_MCLK_CFG_i, PLLCLKSEL, PLL3_ch3);
        }
        #endif        
        break;
    #endif
    #if BAPE_CHIP_MAX_PLLS > 4
        #error "Need to add support for more PLLs"
    #endif
        /* Should never get here */
        default:
            BDBG_ERR(("PLL is invalid"));
            BDBG_ASSERT(false);     /* something went wrong somewhere! */
            return(BERR_INVALID_PARAMETER);
        }
        BAPE_Reg_P_ApplyFieldList(&regFieldList, regAddr);

        switch ( mclkIndex )
        {
        #ifdef BCHP_AUD_MISC_SEROUT_OE_EXTMCLK0_OE_MASK
        case 0:
            BAPE_Reg_P_UpdateEnum(handle, BCHP_AUD_MISC_SEROUT_OE, AUD_MISC_SEROUT_OE, EXTMCLK0_OE, Drive);
            break;
        #endif
        #ifdef BCHP_AUD_MISC_SEROUT_OE_EXTMCLK1_OE_MASK
        case 1:
            BAPE_Reg_P_UpdateEnum(handle, BCHP_AUD_MISC_SEROUT_OE, AUD_MISC_SEROUT_OE, EXTMCLK1_OE, Drive);
            break;
        #endif
        default:
            break;
        }
    }
#endif

    return(BERR_SUCCESS);
    
}

BERR_Code BAPE_Pll_P_ResumeFromStandby(BAPE_Handle bapeHandle)
{
    BERR_Code   errCode = BERR_SUCCESS;
    unsigned    pllIndex;

    BDBG_OBJECT_ASSERT(bapeHandle, BAPE_Device);

    /* For each pll, call the functions necessary to restore the hardware to it's appropriate state. */
    for ( pllIndex=0 ; pllIndex<BAPE_CHIP_MAX_PLLS ; pllIndex++ )
    {
        BAPE_AudioPll *pPll = &bapeHandle->audioPlls[pllIndex];

        /* Now apply changes for the settings struct. */
        errCode = BAPE_Pll_SetSettings(bapeHandle, pllIndex, &pPll->settings );
        if ( errCode ) return BERR_TRACE(errCode);

        /* Now restore the dynamic stuff from the values saved in the device struct. */
        if (pPll->baseSampleRate != 0)
        {
            BKNI_EnterCriticalSection();
                errCode = BAPE_P_SetPllFreq_isr( bapeHandle, pllIndex, pPll->baseSampleRate );
            BKNI_LeaveCriticalSection();
            if ( errCode ) return BERR_TRACE(errCode);
        }
    }
    return errCode;
}
