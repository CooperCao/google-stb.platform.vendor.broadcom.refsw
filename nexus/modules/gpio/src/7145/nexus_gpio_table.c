/***************************************************************************
*     (c)2008-2013 Broadcom Corporation
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
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_gpio_module.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"
#include "bchp_gio.h"
#include "bchp_gio_aon.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_gpio_table);

typedef struct NEXUS_GpioTable
{
    uint32_t addr;
    unsigned mask;
    unsigned shift;
} NEXUS_GpioTable;

#define NEXUS_STD_GPIO(mux_index,gpio_index) \
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_##mux_index, \
     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_##mux_index##_gpio_##gpio_index##_MASK, \
     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_##mux_index##_gpio_##gpio_index##_SHIFT}

#define NEXUS_S_GPIO(mux_index,gpio_index) \
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_##mux_index, \
     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_##mux_index##_sgpio_##gpio_index##_MASK, \
     BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_##mux_index##_sgpio_##gpio_index##_SHIFT}

#define NEXUS_AON_STD_GPIO(mux_index,gpio_index) \
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_##mux_index, \
     BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_##mux_index##_aon_gpio_##gpio_index##_MASK, \
     BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_##mux_index##_aon_gpio_##gpio_index##_SHIFT}

#define NEXUS_AON_S_GPIO(mux_index,gpio_index) \
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_##mux_index, \
     BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_##mux_index##_aon_sgpio_##gpio_index##_MASK, \
     BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_##mux_index##_aon_sgpio_##gpio_index##_SHIFT}

static const NEXUS_GpioTable g_gpioTable[] = {

    NEXUS_STD_GPIO (1, 000),

    NEXUS_STD_GPIO (2, 001),
    NEXUS_STD_GPIO (2, 002),
    NEXUS_STD_GPIO (2, 003),
    NEXUS_STD_GPIO (2, 004),
    NEXUS_STD_GPIO (2, 005),
    NEXUS_STD_GPIO (2, 006),
    NEXUS_STD_GPIO (2, 007),
    NEXUS_STD_GPIO (2, 008),

    NEXUS_STD_GPIO (3, 009),
    NEXUS_STD_GPIO (3, 010),
    NEXUS_STD_GPIO (3, 011),
    NEXUS_STD_GPIO (3, 012),
    NEXUS_STD_GPIO (3, 013),
    NEXUS_STD_GPIO (3, 014),
    NEXUS_STD_GPIO (3, 015),
    NEXUS_STD_GPIO (3, 016),

    NEXUS_STD_GPIO (4, 017),
    NEXUS_STD_GPIO (4, 018),
    NEXUS_STD_GPIO (4, 019),
    NEXUS_STD_GPIO (4, 020),
    NEXUS_STD_GPIO (4, 021),
    NEXUS_STD_GPIO (4, 022),
    NEXUS_STD_GPIO (4, 023),
    NEXUS_STD_GPIO (4, 024),

    NEXUS_STD_GPIO (5, 025),
    NEXUS_STD_GPIO (5, 026),
    NEXUS_STD_GPIO (5, 027),
    NEXUS_STD_GPIO (5, 028),
    NEXUS_STD_GPIO (5, 029),
    NEXUS_STD_GPIO (5, 030),
    NEXUS_STD_GPIO (5, 031),
    NEXUS_STD_GPIO (5, 032),

    NEXUS_STD_GPIO (6, 033),
    NEXUS_STD_GPIO (6, 034),
    NEXUS_STD_GPIO (6, 035),
    NEXUS_STD_GPIO (6, 036),
    NEXUS_STD_GPIO (6, 037),
    NEXUS_STD_GPIO (6, 038),
    NEXUS_STD_GPIO (6, 039),
    NEXUS_STD_GPIO (6, 040),

    NEXUS_STD_GPIO (7, 041),
    NEXUS_STD_GPIO (7, 042),
    NEXUS_STD_GPIO (7, 043),
    NEXUS_STD_GPIO (7, 044),
    NEXUS_STD_GPIO (7, 045),
    NEXUS_STD_GPIO (7, 046),
    NEXUS_STD_GPIO (7, 047),
    NEXUS_STD_GPIO (7, 048),

    NEXUS_STD_GPIO (8, 049),
    NEXUS_STD_GPIO (8, 050),
    NEXUS_STD_GPIO (8, 051),
    NEXUS_STD_GPIO (8, 052),
    NEXUS_STD_GPIO (8, 053),
    NEXUS_STD_GPIO (8, 054),
    NEXUS_STD_GPIO (8, 055),
    NEXUS_STD_GPIO (8, 056),

    NEXUS_STD_GPIO (9, 057),
    NEXUS_STD_GPIO (9, 058),
    NEXUS_STD_GPIO (9, 059),
    NEXUS_STD_GPIO (9, 060),
    NEXUS_STD_GPIO (9, 061),
    NEXUS_STD_GPIO (9, 062),
    NEXUS_STD_GPIO (9, 063),
    NEXUS_STD_GPIO (9, 064),

    NEXUS_STD_GPIO (10, 065),
    NEXUS_STD_GPIO (10, 066),
    NEXUS_STD_GPIO (10, 067),
    NEXUS_STD_GPIO (10, 068),
    NEXUS_STD_GPIO (10, 069),
    NEXUS_STD_GPIO (10, 070),
    NEXUS_STD_GPIO (10, 071),
    NEXUS_STD_GPIO (10, 072),

    NEXUS_STD_GPIO (11, 073),
    NEXUS_STD_GPIO (11, 074),
    NEXUS_STD_GPIO (11, 075),
    NEXUS_STD_GPIO (11, 076),
    NEXUS_STD_GPIO (11, 077),
    NEXUS_STD_GPIO (11, 078),
    NEXUS_STD_GPIO (11, 079),
    NEXUS_STD_GPIO (11, 080),

    NEXUS_STD_GPIO (12, 081),
    NEXUS_STD_GPIO (12, 082),
    NEXUS_STD_GPIO (12, 083),
    NEXUS_STD_GPIO (12, 084),
    NEXUS_STD_GPIO (12, 085),
    NEXUS_STD_GPIO (12, 086),
    NEXUS_STD_GPIO (12, 087),
    NEXUS_STD_GPIO (12, 088),

    NEXUS_STD_GPIO (13, 089),
    NEXUS_STD_GPIO (13, 090),
    NEXUS_STD_GPIO (13, 091),
    NEXUS_STD_GPIO (13, 092),
    NEXUS_STD_GPIO (13, 093),
    NEXUS_STD_GPIO (13, 094),
    NEXUS_STD_GPIO (13, 095),
    NEXUS_STD_GPIO (13, 096),

    NEXUS_STD_GPIO (14, 097),
    NEXUS_STD_GPIO (14, 098),
    NEXUS_STD_GPIO (14, 099),
    NEXUS_STD_GPIO (14, 100),
    NEXUS_STD_GPIO (14, 101),
    NEXUS_STD_GPIO (14, 102)
};

NEXUS_GpioTable g_sgpioTable[] = {
    NEXUS_S_GPIO (14, 00),
    NEXUS_S_GPIO (14, 01),

    NEXUS_S_GPIO (15, 02),
    NEXUS_S_GPIO (15, 03)
};

static const NEXUS_GpioTable g_aonGpioTable[] = {
    NEXUS_AON_STD_GPIO(0, 00),
    NEXUS_AON_STD_GPIO(0, 01),
    NEXUS_AON_STD_GPIO(0, 02),
    NEXUS_AON_STD_GPIO(0, 03),
    NEXUS_AON_STD_GPIO(0, 04),
    NEXUS_AON_STD_GPIO(0, 05),
    NEXUS_AON_STD_GPIO(0, 06),
    NEXUS_AON_STD_GPIO(0, 07),

    NEXUS_AON_STD_GPIO(1, 08),
    NEXUS_AON_STD_GPIO(1, 09),
    NEXUS_AON_STD_GPIO(1, 10),
    NEXUS_AON_STD_GPIO(1, 11),
    NEXUS_AON_STD_GPIO(1, 12),
    NEXUS_AON_STD_GPIO(1, 13),
    NEXUS_AON_STD_GPIO(1, 14),
    NEXUS_AON_STD_GPIO(1, 15),

    NEXUS_AON_STD_GPIO(2, 16),
    NEXUS_AON_STD_GPIO(2, 17),
#if BCHP_VER >= BCHP_VER_B0
    NEXUS_AON_STD_GPIO(2, 18),
    NEXUS_AON_STD_GPIO(2, 19),
#endif
};

static const NEXUS_GpioTable g_aonSgpioTable[] = {
    NEXUS_AON_S_GPIO(2, 00),
    NEXUS_AON_S_GPIO(2, 01),
    NEXUS_AON_S_GPIO(2, 02),
    NEXUS_AON_S_GPIO(2, 03),
#if BCHP_VER == BCHP_VER_A0
    NEXUS_AON_S_GPIO(2, 04),
    NEXUS_AON_S_GPIO(2, 05)
#endif
};

/* if these NEXUS_NUM gpio macros are defined in nexus_platform_features.h, it is only for application convenience.
this internal definition is more reliable for implementation. */
#undef NEXUS_NUM_GPIO_PINS
#define NEXUS_NUM_GPIO_PINS (sizeof(g_gpioTable)/sizeof(g_gpioTable[0]))
#undef NEXUS_NUM_SGPIO_PINS
#define NEXUS_NUM_SGPIO_PINS (sizeof(g_sgpioTable)/sizeof(g_sgpioTable[0]))
#undef NEXUS_NUM_AON_GPIO_PINS
#define NEXUS_NUM_AON_GPIO_PINS (sizeof(g_aonGpioTable)/sizeof(g_aonGpioTable[0]))
#undef NEXUS_NUM_AON_SGPIO_PINS
#define NEXUS_NUM_AON_SGPIO_PINS (sizeof(g_aonSgpioTable)/sizeof(g_aonSgpioTable[0]))

/* These functions must be implemented per-chip */
NEXUS_Error NEXUS_Gpio_P_GetPinMux(NEXUS_GpioType type, unsigned pin, uint32_t *pAddr, uint32_t *pMask, unsigned *pShift )
{
    const NEXUS_GpioTable *pEntry=NULL;

    switch (type)
    {
        case NEXUS_GpioType_eStandard:
            if ( pin >= NEXUS_NUM_GPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            pEntry = g_gpioTable+pin;
            break;
        case NEXUS_GpioType_eSpecial:
            if ( pin >= NEXUS_NUM_SGPIO_PINS )
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            pEntry = g_sgpioTable+pin;
            break;
        case NEXUS_GpioType_eAonStandard:
            if ( pin >= NEXUS_NUM_AON_GPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            pEntry = g_aonGpioTable+pin;
            break;
        case NEXUS_GpioType_eAonSpecial:
            if ( pin >= NEXUS_NUM_AON_SGPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            pEntry = g_aonSgpioTable+pin;
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *pAddr = pEntry->addr;
    *pMask = pEntry->mask;
    *pShift = pEntry->shift;

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Gpio_P_CheckPinmux(NEXUS_GpioType type, unsigned pin)
{
    uint32_t addr, mask, shift, val;
    NEXUS_Error rc;

    rc = NEXUS_Gpio_P_GetPinMux(type, pin, &addr, &mask, &shift);
    if (rc) return BERR_TRACE(rc);

    val = BREG_Read32(g_pCoreHandles->reg, addr);
    if ( val & mask )
    {
        /* Pin is not configured as GPIO */
        BDBG_ERR(("Pin mux register for %u is not properly configured - value %u should be 0",
                   pin, val>>shift));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_Gpio_P_GetPinData(NEXUS_GpioType type, unsigned pin, uint32_t *pAddress, uint32_t *pShift)
{

    switch (type)
            /* SGPIO Pins 9..0 are in ODEN_EXT */
    {
        case NEXUS_GpioType_eStandard:
        if ( pin < 32 )
        {
            /* GPIO Pins 31..0 are in ODEN_LO */
            *pAddress = BCHP_GIO_ODEN_LO;
            *pShift = pin;
        }
        else if ( pin < 64 )
        {
            /* GPIO Pins 63..32 are in ODEN_HI */
            *pAddress = BCHP_GIO_ODEN_HI;
            *pShift = pin-32;
        }
            else if ( pin < 90 )
        {
            /* GPIO Pins 85..64 are in ODEN_EXT, above sgpio bits */
            *pAddress = BCHP_GIO_ODEN_EXT;
            *pShift = (pin-64)+NEXUS_NUM_SGPIO_PINS;
        }
        else if ( pin < 113 )
        {
            /* GPIO Pins 112..86 are in ODEN_EXT_HI */
            *pAddress = BCHP_GIO_ODEN_EXT_HI;
                *pShift = (pin-90);
        }
        else
        {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
            break;
        case NEXUS_GpioType_eSpecial:
            if ( pin >= NEXUS_NUM_SGPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
            else
            {
                *pAddress = BCHP_GIO_ODEN_EXT;
                *pShift = pin;
            }
            break;
        case NEXUS_GpioType_eAonStandard:
            if ( pin >= NEXUS_NUM_AON_GPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            else
            {
                *pAddress = BCHP_GIO_AON_ODEN_LO;
                *pShift = pin;
            }
            break;
        case NEXUS_GpioType_eAonSpecial:
            if ( pin >= NEXUS_NUM_AON_SGPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            else
            {
                *pAddress = BCHP_GIO_AON_ODEN_EXT;
                *pShift = pin;
            }
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BERR_SUCCESS;
}

