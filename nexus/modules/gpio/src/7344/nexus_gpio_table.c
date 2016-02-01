/***************************************************************************
*     (c)2008-2011 Broadcom Corporation
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

static const NEXUS_GpioTable g_gpioTable[] = {
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_00_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_00_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_01_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_01_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_02_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_02_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_03_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_03_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_04_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_04_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_05_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_05_SHIFT},
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_06_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_06_SHIFT},
#endif

#if BCHP_VER >= BCHP_VER_B0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_06_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_06_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_07_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_07_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_08_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_08_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_09_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_09_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_10_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_10_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_11_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_11_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_12_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_12_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_13_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_13_SHIFT},
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_14_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7_gpio_14_SHIFT},
#endif

#if BCHP_VER >= BCHP_VER_B0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_14_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_14_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_15_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_15_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_16_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_16_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_17_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_17_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_18_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_18_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_19_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_19_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_20_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_20_SHIFT},
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_21_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_21_SHIFT},
#endif 

#if BCHP_VER >= BCHP_VER_B0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_21_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_21_SHIFT},
#endif 
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_22_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13_gpio_22_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_23_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_23_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_24_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_24_SHIFT},
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_25_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8_gpio_25_SHIFT},
#endif

#if BCHP_VER >= BCHP_VER_B0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_25_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_25_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_26_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_26_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_27_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_27_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_28_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_28_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_29_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_29_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_30_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_30_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_31_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_31_SHIFT},
    
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_32_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_32_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_33_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_33_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_34_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_34_SHIFT},
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_35_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9_gpio_35_SHIFT},
#endif

#if BCHP_VER >= BCHP_VER_B0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_35_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_35_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_36_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_36_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_37_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_37_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_38_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_38_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_39_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_39_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_40_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_40_SHIFT},
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_41_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10_gpio_41_SHIFT},
#endif

#if BCHP_VER >= BCHP_VER_B0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_41_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_41_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_42_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_42_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_43_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_43_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_44_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_44_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_45_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_45_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_46_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_46_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_47_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_47_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_48_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_48_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_49_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_49_SHIFT},
#if BCHP_VER == BCHP_VER_A0
	{BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_50_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11_gpio_50_SHIFT},
#endif

#if BCHP_VER >= BCHP_VER_B0
	{BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_50_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_50_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_51_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_51_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_52_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_52_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_53_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_53_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_54_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_54_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_55_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_55_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_56_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12_gpio_56_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_57_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_57_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_58_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_58_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_59_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_59_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_60_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_60_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_61_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_61_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_62_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_62_SHIFT},
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_63_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14_gpio_63_SHIFT},
#endif

#if BCHP_VER >= BCHP_VER_B0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_63_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_63_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_64_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_64_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_65_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_65_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_66_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_66_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_67_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_67_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_68_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_68_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_69_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_69_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_70_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_70_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_71_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_71_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_72_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_72_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_73_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_73_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_74_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_74_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_75_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0_gpio_75_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_76_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_76_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_77_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_77_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_78_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1_gpio_78_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_79_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_79_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_80_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_gpio_80_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_81_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_81_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_82_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_82_SHIFT},
#if BCHP_VER == BCHP_VER_A0
	{BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_83_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5_gpio_83_SHIFT},
#endif
#if BCHP_VER >= BCHP_VER_B0
	{BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_83_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_83_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_84_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6_gpio_84_SHIFT},

#if 0 
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_099_MASK,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_099_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_100_MASK,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_100_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_101_MASK,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_101_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_102_MASK,  BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_gpio_102_SHIFT},
#endif
};

static const NEXUS_GpioTable g_sgpioTable[] = {
#if BCHP_VER == BCHP_VER_A0
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_sgpio_00_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15_sgpio_00_SHIFT},

    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_01_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_01_SHIFT},
#endif
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_02_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_02_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_03_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_03_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_04_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_04_SHIFT},
    {BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_05_MASK, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_sgpio_05_SHIFT},
};

static const NEXUS_GpioTable g_aonGpioTable[] = {
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_00_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_00_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_01_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_01_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_02_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_02_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_03_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_03_SHIFT},
    
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_04_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_04_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_05_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_05_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_06_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_06_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_07_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_07_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_08_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_08_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_09_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_09_SHIFT},
    
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_gpio_10_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_gpio_10_SHIFT},

    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_11_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_11_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_12_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_12_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_13_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_13_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_14_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0_aon_gpio_14_SHIFT},

    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_15_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_15_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_16_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1_aon_gpio_16_SHIFT},

    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3_aon_gpio_17_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3_aon_gpio_17_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3_aon_gpio_18_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3_aon_gpio_18_SHIFT},
};

static const NEXUS_GpioTable g_aonSgpioTable[] = {
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_sgpio_00_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_sgpio_00_SHIFT},
    {BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_sgpio_01_MASK, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2_aon_sgpio_01_SHIFT},
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
    BDBG_ASSERT(type <= NEXUS_GpioType_eAonSpecial);

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
            if ( pin >= NEXUS_NUM_SGPIO_PINS)
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
            break;
    }

    *pAddr = pEntry->addr;
    *pMask = pEntry->mask;
    *pShift = pEntry->shift;
    return 0;
}

NEXUS_Error NEXUS_Gpio_P_CheckPinmux(NEXUS_GpioType type, unsigned pin)
{
    const NEXUS_GpioTable *pEntry=NULL;
    uint32_t val;

    BDBG_ASSERT(type <= NEXUS_GpioType_eAonSpecial);

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
            if ( pin >= NEXUS_NUM_SGPIO_PINS)
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
            break;
    }

    val = BREG_Read32(g_pCoreHandles->reg, pEntry->addr);
    if ( val & pEntry->mask )
    {
        /* Pin is not configured as GPIO */
        BDBG_ERR(("Pin mux register for %u is not properly configured - value %u should be 0",
                   pin, val>>pEntry->shift));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

#if NEXUS_GPIO_REGISTER_ABSTRACTION
NEXUS_Error NEXUS_Gpio_P_GetPinData(NEXUS_GpioHandle gpio)
{
    BDBG_ASSERT(gpio->type <= NEXUS_GpioType_eAonSpecial); 
    switch (gpio->type)
    {
    	case NEXUS_GpioType_eStandard:
        	if ( gpio->pin < 32 )
        	{
            	/* GPIO Pins 31..0 are in ODEN_LO */
            	gpio->shift = gpio->pin;
		        gpio->addr.iodir=BCHP_GIO_IODIR_LO;
		        gpio->addr.data=BCHP_GIO_DATA_LO;
		        gpio->addr.oden=BCHP_GIO_ODEN_LO;
		        gpio->addr.mask=BCHP_GIO_MASK_LO;
		        gpio->addr.ec= BCHP_GIO_EC_LO;
		        gpio->addr.ei= BCHP_GIO_EI_LO;
		        gpio->addr.level=BCHP_GIO_LEVEL_LO;
		        gpio->addr.stat=BCHP_GIO_STAT_LO;
		
        	}
        	else if ( gpio->pin < 64 )
        	{
            	/* GPIO Pins 63..32 are in ODEN_HI */
             	gpio->shift = gpio->pin-32;
				gpio->addr.iodir=BCHP_GIO_IODIR_HI;
				gpio->addr.data=BCHP_GIO_DATA_HI;
				gpio->addr.oden=BCHP_GIO_ODEN_HI;
				gpio->addr.mask=BCHP_GIO_MASK_HI;
				gpio->addr.ec= BCHP_GIO_EC_HI;
				gpio->addr.ei= BCHP_GIO_EI_HI;
				gpio->addr.level=BCHP_GIO_LEVEL_HI;
				gpio->addr.stat=BCHP_GIO_STAT_HI;
        	}
        	else if ( gpio->pin < 90 )
        	{
            	/* GPIO Pins 89..64 are in ODEN_EXT, above sgpio bits */
            	
            	gpio->shift = (gpio->pin-64)+NEXUS_NUM_SGPIO_PINS;
				gpio->addr.iodir=BCHP_GIO_IODIR_EXT;
				gpio->addr.data=BCHP_GIO_DATA_EXT;
				gpio->addr.oden=BCHP_GIO_ODEN_EXT;
				gpio->addr.mask=BCHP_GIO_MASK_EXT;
				gpio->addr.ec= BCHP_GIO_EC_EXT;
				gpio->addr.ei= BCHP_GIO_EI_EXT;
				gpio->addr.level=BCHP_GIO_LEVEL_EXT;
				gpio->addr.stat=BCHP_GIO_STAT_EXT;
        	}
#if 0 /* Does not apply to 734X chips */
        	else if ( pin < 113 )
        	{
            	/* GPIO Pins 110..90 are in ODEN_EXT_HI */
            	*pAddress = BCHP_GIO_ODEN_EXT_HI;
            	*pShift = (pin-90);
        	}
#endif
        	else
        	{
            	return BERR_TRACE(BERR_INVALID_PARAMETER);
        	}
            break;
        case NEXUS_GpioType_eSpecial:
            if ( gpio->pin >= NEXUS_NUM_SGPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        	else
        	{
            	/* SGPIO Pins 5..0 are in ODEN_EXT */
            
            	gpio->shift = gpio->pin;
	       		gpio->addr.iodir=BCHP_GIO_IODIR_EXT;
				gpio->addr.data=BCHP_GIO_DATA_EXT;
				gpio->addr.oden=BCHP_GIO_ODEN_EXT;
				gpio->addr.mask=BCHP_GIO_MASK_EXT;
				gpio->addr.ec= BCHP_GIO_EC_EXT;
				gpio->addr.ei= BCHP_GIO_EI_EXT;
				gpio->addr.level=BCHP_GIO_LEVEL_EXT;
				gpio->addr.stat=BCHP_GIO_STAT_EXT;
        	}
            break; 
        case NEXUS_GpioType_eAonStandard:
            if ( gpio->pin >= NEXUS_NUM_AON_GPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        	else
        	{
            	/* AON GPIO Pins 17..0 are in AON_ODEN_LO */ 
            	gpio->shift = gpio->pin;
				gpio->addr.iodir=BCHP_GIO_AON_IODIR_LO;
				gpio->addr.data=BCHP_GIO_AON_DATA_LO;
				gpio->addr.oden=BCHP_GIO_AON_ODEN_LO;
				gpio->addr.mask=BCHP_GIO_AON_MASK_LO;
				gpio->addr.ec= BCHP_GIO_AON_EC_LO;
				gpio->addr.ei= BCHP_GIO_AON_EI_LO;
				gpio->addr.level=BCHP_GIO_AON_LEVEL_LO;
				gpio->addr.stat=BCHP_GIO_AON_STAT_LO; 
        	}
            break; 
        case NEXUS_GpioType_eAonSpecial:
            if ( gpio->pin >= NEXUS_NUM_AON_SGPIO_PINS)
            {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        	else
        	{
            	/* AON SGPIO Pins 3..0 are in AON_ODEN_EXT */
            	gpio->shift = gpio->pin;
		        gpio->addr.iodir=BCHP_GIO_AON_IODIR_EXT;
				gpio->addr.data=BCHP_GIO_AON_DATA_EXT;
				gpio->addr.oden=BCHP_GIO_AON_ODEN_EXT;
				gpio->addr.mask=BCHP_GIO_AON_MASK_EXT;
				gpio->addr.ec= BCHP_GIO_AON_EC_EXT;
				gpio->addr.ei= BCHP_GIO_AON_EI_EXT;
				gpio->addr.level=BCHP_GIO_AON_LEVEL_EXT;
				gpio->addr.stat=BCHP_GIO_AON_STAT_EXT;
        	}
            break; 
        default:
            break;
    }

    return BERR_SUCCESS;
}
#else
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
        	else if ( pin < 90 )
        	{
            	/* GPIO Pins 89..64 are in ODEN_EXT, above sgpio bits */
            	*pAddress = BCHP_GIO_ODEN_EXT;
            	*pShift = (pin-64)+NEXUS_NUM_SGPIO_PINS;
        	}
#if 0 /* Does not apply to 734X chips */
        	else if ( pin < 113 )
        	{
            	/* GPIO Pins 110..90 are in ODEN_EXT_HI */
            	*pAddress = BCHP_GIO_ODEN_EXT_HI;
            	*pShift = (pin-90);
        	}
#endif
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
#endif
