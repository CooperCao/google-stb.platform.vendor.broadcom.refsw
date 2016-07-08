/***************************************************************************
*     (c)2008-2012 Broadcom Corporation
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
 * API Description:
 *   API name: Gpio
 *    Specific APIs related to Gpio Control.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
/* This functions must be implemented per-chip */
NEXUS_Error NEXUS_Gpio_P_GetPinData(NEXUS_GpioType type, unsigned pin, uint32_t *pAddress, uint32_t *pShift)
{
    BDBG_ASSERT(type <= NEXUS_GpioType_eAonSpecial);

    switch (type)
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
                else if ( pin < 96 )
                {
                /* GPIO Pins 95..64 are in ODEN_EXT, above sgpio bits */
                *pAddress = BCHP_GIO_ODEN_EXT;
                *pShift = (pin-64);
                }
                else if ( pin < 125 )
                {
                /* GPIO Pins 124..96 are in ODEN_EXT_HI */
                *pAddress = BCHP_GIO_ODEN_EXT_HI;
                *pShift = (pin-96);
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
                /* SGPIO Pins 5..0 are in ODEN_EXT */
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
                /* AON GPIO Pins 17..0 are in AON_ODEN_LO */
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
                /* AON SGPIO Pins 3..0 are in AON_ODEN_EXT */
                *pAddress = BCHP_GIO_AON_ODEN_EXT;
                *pShift = pin;
                }
            break;
        default:
            break;
    }

    return BERR_SUCCESS;
}
