/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 **************************************************************************/

#include "bchp_common.h"
#include "bchp_clkgen.h"

/* Determine VCXO-PLL register layout */
#ifdef BCHP_VCXO_RM_REG_START
#include "bchp_vcxo_rm.h"

#if NEXUS_NUM_VCXOS != 1
#error NEXUS_NUM_VCXOS must be 0 or 1 for this chipset
#endif
#define NEXUS_VCXO_RESET_ADDR(i) (BCHP_CLKGEN_PLL_VCXO_PLL_RESET)

#define NEXUS_VCXO_RESET_MASK BCHP_MASK(CLKGEN_PLL_VCXO_PLL_RESET, RESETD)
#define NEXUS_VCXO_RESET_SET BCHP_FIELD_DATA(CLKGEN_PLL_VCXO_PLL_RESET, RESETD, 1)
#define NEXUS_VCXO_RESET_CLEAR BCHP_FIELD_DATA(CLKGEN_PLL_VCXO_PLL_RESET, RESETD, 0)

#define NEXUS_VCXO_GET_REG_ADDR(i,name) (BCHP_VCXO_RM_##name)
#define NEXUS_VCXO_MASK(regname,fieldname) (BCHP_MASK(VCXO_RM_##regname,fieldname))
#define NEXUS_VCXO_FIELD_DATA(regname,fieldname,val) (BCHP_FIELD_DATA(VCXO_RM_##regname,fieldname,val))
#define NEXUS_VCXO_FIELD_ENUM(regname,fieldname,val) (BCHP_FIELD_ENUM(VCXO_RM_##regname,fieldname,val))
#define NEXUS_VCXO_GET_FIELD_DATA(val,regname,fieldname) (BCHP_GET_FIELD_DATA(val,VCXO_RM_##regname,fieldname))

#else
#if NEXUS_NUM_VCXOS > 3
#error Support not available for > 3 VCXO-PLLs
#elif NEXUS_NUM_VCXOS == 3
#include "bchp_vcxo_0_rm.h"
#include "bchp_vcxo_1_rm.h"
#include "bchp_vcxo_2_rm.h"
#define NEXUS_VCXO_BASE(i) ((i)==2?BCHP_VCXO_2_RM_REG_START:(i)==1?BCHP_VCXO_1_RM_REG_START:BCHP_VCXO_0_RM_REG_START)
#define NEXUS_VCXO_RESET_ADDR(i) ((i)==2?BCHP_CLKGEN_PLL_VCXO2_PLL_RESET:(i)==1?BCHP_CLKGEN_PLL_VCXO1_PLL_RESET:BCHP_CLKGEN_PLL_VCXO0_PLL_RESET)
#elif NEXUS_NUM_VCXOS == 2
#include "bchp_vcxo_0_rm.h"
#include "bchp_vcxo_1_rm.h"
#define NEXUS_VCXO_BASE(i) ((i)==1?BCHP_VCXO_1_RM_REG_START:BCHP_VCXO_0_RM_REG_START)
#define NEXUS_VCXO_RESET_ADDR(i) (((i)==1)?BCHP_CLKGEN_PLL_VCXO1_PLL_RESET:BCHP_CLKGEN_PLL_VCXO0_PLL_RESET)
#elif NEXUS_NUM_VCXOS == 1
#include "bchp_vcxo_0_rm.h"
#define NEXUS_VCXO_BASE(i) (BCHP_VCXO_0_RM_REG_START)
#define NEXUS_VCXO_RESET_ADDR(i) (BCHP_CLKGEN_PLL_VCXO0_PLL_RESET)
#endif

#define NEXUS_VCXO_RESET_MASK BCHP_MASK(CLKGEN_PLL_VCXO0_PLL_RESET, RESETD)
#define NEXUS_VCXO_RESET_SET BCHP_FIELD_DATA(CLKGEN_PLL_VCXO0_PLL_RESET, RESETD, 1)
#define NEXUS_VCXO_RESET_CLEAR BCHP_FIELD_DATA(CLKGEN_PLL_VCXO0_PLL_RESET, RESETD, 0)

#define NEXUS_VCXO_GET_REG_ADDR(i,name) (((BCHP_VCXO_0_RM_##name)-BCHP_VCXO_0_RM_REG_START) + NEXUS_VCXO_BASE(i))
#define NEXUS_VCXO_MASK(regname,fieldname) (BCHP_MASK(VCXO_0_RM_##regname,fieldname))
#define NEXUS_VCXO_FIELD_DATA(regname,fieldname,val) (BCHP_FIELD_DATA(VCXO_0_RM_##regname,fieldname,val))
#define NEXUS_VCXO_FIELD_ENUM(regname,fieldname,val) (BCHP_FIELD_ENUM(VCXO_0_RM_##regname,fieldname,val))
#define NEXUS_VCXO_GET_FIELD_DATA(val,regname,fieldname) (BCHP_GET_FIELD_DATA(val,VCXO_0_RM_##regname,fieldname))
#endif

static void NEXUS_Vcxo_P_Init(void)
{
    unsigned i;

    for ( i = 0; i < NEXUS_NUM_VCXOS; i++ )
    {
        uint32_t regAddr, regVal;

        /* Assert VCXO Reset Bit and clear all others */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,CONTROL);
        regVal = NEXUS_VCXO_FIELD_DATA(CONTROL,RESET,1);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);

        /* Apply CLKGEN reset for this VCXO */
        regAddr = NEXUS_VCXO_RESET_ADDR(i);
        BREG_AtomicUpdate32(g_pCoreHandles->reg, regAddr, NEXUS_VCXO_RESET_MASK, NEXUS_VCXO_RESET_SET);

        /* Set Denominator */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,RATE_RATIO);
        regVal = NEXUS_VCXO_FIELD_DATA(RATE_RATIO,DENOMINATOR,32);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);

        /* Setup numerator and sample increment */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,SAMPLE_INC);
        regVal = NEXUS_VCXO_FIELD_DATA(SAMPLE_INC,NUMERATOR,29) |
                 NEXUS_VCXO_FIELD_DATA(SAMPLE_INC,SAMPLE_INC,3);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);

        /* Setup Phase Increment */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,PHASE_INC);
        regVal = NEXUS_VCXO_FIELD_DATA(PHASE_INC,PHASE_INC,0x7d634);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);

#ifdef BCHP_VCXO_0_RM_INTEGRATOR_HI
        /* setup integrator */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,INTEGRATOR_LO);
        regVal = 0x0;
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);
        /* setup integrator*/
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,INTEGRATOR_HI);
        regVal = 0x0;
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);
        /* atomic 64bit  write */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,CONTROL);
        regVal = NEXUS_VCXO_FIELD_DATA(CONTROL,LOAD_INTEGRATOR,1);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);
        /* disable rewrite  */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,CONTROL);
        regVal = NEXUS_VCXO_FIELD_DATA(CONTROL,LOAD_INTEGRATOR,0);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);
#else
        /* Setup Phase Increment */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,INTEGRATOR);
        regVal = 0x0;
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);
#endif
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,FORMAT);
        regVal = NEXUS_VCXO_FIELD_DATA(FORMAT,SHIFT,2) |
                 NEXUS_VCXO_FIELD_DATA(FORMAT,STABLE_COUNT,10000);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);

        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,OFFSET);
        regVal = NEXUS_VCXO_FIELD_DATA(OFFSET,OFFSET_ONLY,0) |
                 NEXUS_VCXO_FIELD_DATA(OFFSET,OFFSET,0x10000000);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);

        /* Update control and de-assert reset */
        regAddr = NEXUS_VCXO_GET_REG_ADDR(i,CONTROL);
        regVal =
            NEXUS_VCXO_FIELD_DATA(CONTROL,TIMEBASE,0) |   /* Timebase 0 */
            NEXUS_VCXO_FIELD_DATA(CONTROL,FREE_RUN,0) |
            NEXUS_VCXO_FIELD_DATA(CONTROL,DITHER,1) |
            NEXUS_VCXO_FIELD_DATA(CONTROL,DIRECT_GAIN,2) |
            NEXUS_VCXO_FIELD_DATA(CONTROL,INT_GAIN,4) |
            NEXUS_VCXO_FIELD_DATA(CONTROL,RESET,0);
        BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);

        /* Apply CLKGEN reset for this VCXO */
        regAddr = NEXUS_VCXO_RESET_ADDR(i);
        BREG_AtomicUpdate32(g_pCoreHandles->reg, regAddr, NEXUS_VCXO_RESET_MASK, NEXUS_VCXO_RESET_CLEAR);
    }
}

#if 0
static unsigned NEXUS_Vcxo_P_GetTimebase(NEXUS_Vcxo vcxo)
{
    uint32_t regAddr, regVal;
    BSTD_UNUSED(vcxo);
    regAddr = NEXUS_VCXO_GET_REG_ADDR(vcxo,CONTROL);
    regVal = BREG_Read32(g_pCoreHandles->reg, regAddr);
    return (unsigned)NEXUS_VCXO_GET_FIELD_DATA(regVal,CONTROL,TIMEBASE);
}
#endif

static void NEXUS_Vcxo_P_SetTimebase(NEXUS_Vcxo vcxo, unsigned timebase)
{
    uint32_t regAddr, regVal;
    BSTD_UNUSED(vcxo);
    regAddr = NEXUS_VCXO_GET_REG_ADDR(vcxo,CONTROL);
    regVal = BREG_Read32(g_pCoreHandles->reg, regAddr);
    regVal &= ~NEXUS_VCXO_MASK(CONTROL,TIMEBASE);
    regVal |= NEXUS_VCXO_FIELD_DATA(CONTROL,TIMEBASE,timebase);
    BREG_Write32(g_pCoreHandles->reg, regAddr, regVal);    
}


