/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* API Description:
*   This file contains pinmux initialization for the 97340 reference board.
*
* Revision History:
* 
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"
#include "bchp_aon_pin_ctrl.h"

BDBG_MODULE(nexus_platform_pinmux);


/***************************************************************************
Summary:
    Configure pin muxes for a 97346 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
#if NEXUS_PLATFORM_97346_H43
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* Writing pinmuxes in order from pg. 10 of the schematic */
    /* GPIO PIN MUX CTRL 0 -07 not GPIO*/

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
    *
    *
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_000) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_001) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_002) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_003) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_004)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_000, 2 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_001, 2 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_002, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_003, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_004, 1 )
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);



    /* GPIO 38-47 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_005) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_008) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_009) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_010) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_011) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_012)
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_005, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_008, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_009, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_010, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_011, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_012, 0 )
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);

    /* GPIO 48-51 & GPIO 53-58 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
     *
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_013 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_014 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_015 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_017 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_019 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_020 )

            );
    reg |=(
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_013, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_014, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_015, 0 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016, 0 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_017, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_019, 0 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_020, 0 )
            );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10
    *
    */
   /* Not 52 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_021 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_022 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_023 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_024 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_025 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_026 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_027 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_028 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_021, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_022, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_023, 0 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_024, 0 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_025, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_026, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_027, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_028, 1 )
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
     *
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_029 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_030 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_031 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_029, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_030, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_031, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033, 2 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034, 3 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035, 4 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036, 0 )
          );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

   /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12
    *
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_038 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_039 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_038, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_039, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044, 0 )

            );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);

       /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13
    *includes 52
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_045 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_046 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_047 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_045, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_046, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_047, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052, 0 )
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);

     /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14
    * sgpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_053 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_054 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_055 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_053, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_054, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_055, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058, 3 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060, 0 )
            );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15
    * gpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_061 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_063 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_064 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_066 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068 )
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_061, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_063, 7 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_064, 7 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065, 7 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_066, 7 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067, 6 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068, 5 )
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16
    * sgpio 0-4
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_069 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_070 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_071 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_00 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_01 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_02 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_03 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_04 )
             );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_069, 3 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_070, 5 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_071, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_00, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_01, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_02, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_03, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_04, 1 )
           );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17
    * sgpio 05
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &=~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_05));
    reg |=( BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_05, 1 ));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17,reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_000 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_001 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_004 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_005 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_sgpio_02 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_sgpio_03 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_000, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_001, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_004, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_005, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_sgpio_02, 2 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_sgpio_03, 2 )
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
     *
    */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_006 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_007 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_008 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_009 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_010 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_011 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_012 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_013 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_006, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_007, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_008, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_009, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_010, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_011, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_012, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_013, 0 )
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);


    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_014 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_015 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_016 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_017 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_018 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_019 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_020 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_021 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_014, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_015, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_016, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_017, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_018, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_019, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_020, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_021, 0 )
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);


    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_022 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_023 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_024 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_025 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_026 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_00 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01 )
            );

    reg |=(
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_022, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_023, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_024, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_025, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_026, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_00, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01, 1 )
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);


    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, SVD0_OL);

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,16);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}
#else
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* Writing pinmuxes in order from pg. 10 of the schematic */
    /* GPIO PIN MUX CTRL 0 -07 not GPIO*/
    
    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
    *
    * 
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_000) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_001) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_002) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_003) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_004)  
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_000, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_001, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_002, 1 ) |
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_003, 2 ) |
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_003, 1 ) |
#endif
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_004, 2 )
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_004, 1 )
#endif
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);



    /* GPIO 38-47 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_005) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_008) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_009) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_010) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_011) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_012) 
            );
    reg |=(
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_005, 2 ) |
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_005, 1 ) |
#endif 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_006, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_007, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_008, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_009, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_010, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_011, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_012, 1 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);
    
    /* GPIO 48-51 & GPIO 53-58 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
     *
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_013 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_014 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_015 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_017 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_019 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_020 ) 

            );
    reg |=( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_013, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_014, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_015, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_017, 5 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018, 5 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_019, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_020, 1 ) 
            );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10
    *
    */
   /* Not 52 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_021 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_022 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_023 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_024 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_025 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_026 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_027 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_028 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_021, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_022, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_023, 0 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_024, 0 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_025, 1 )  | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_026, 1 )  | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_027, 1 )  | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_028, 1 )  
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
     *
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_029 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_030 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_031 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_029, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_030, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_031, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032, 1 ) |
#if NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035, 4 ) | 
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035, 3 ) |
#endif
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036, 5 ) 
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036, 3 )
#endif
          );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

   /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12
    *
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_038 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_039 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044 ) 
            );
    reg |=(
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037, 6 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_038, 6 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_039, 6 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040, 6 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, 5 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042, 5 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043, 5 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044, 5 ) 
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_037, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_038, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_039, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044, 3 ) 
#endif      

            );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);

       /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13
    *includes 52
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_045 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_046 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_047 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052 ) 
            );
    reg |=( 
#if !NEXUS_HAS_DVB_CI
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_045, 5 ) |
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_045, 3 ) |
#endif
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_046, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_047, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049, 0 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050, 4 ) | /* HMC need value 4*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051, 0 ) |
#if defined(NEXUS_PLATFORM_97346_HR44)              
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052, 3 )   /* PKT_CLK3 */
#else           
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052, 0 ) 
#endif           
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);

     /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14
    * sgpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_053 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_054 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_055 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_053, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_054, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_055, 3 ) | 
#if !NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056, 3 ) | 
#endif          
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057, 2 ) | 
#if NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058, 3 ) | /* MTSIF_RX0_SYNC */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058, 2 ) | 
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059, 4 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060, 4 )  
            );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15
    * gpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_061 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_063 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_064 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_066 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_061, 6 ) | 
#if (defined(NEXUS_PLATFORM_97346_HR44) || defined(NEXUS_PLATFORM_97346_SV) || defined(NEXUS_PLATFORM_97346_I2SFF))
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062, 7 ) | 
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_062, 3 ) | 
#endif
#if NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_063, 7 ) | /* MTSIF_RX0_DATA_6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_064, 7 ) | /* MTSIF_RX0_DATA_7 */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_063, 6 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_064, 6 ) |
#endif
#if defined(NEXUS_PLATFORM_97346_HR44)            
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065, 0 ) | 
#elif NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065, 7 ) | /* MTSIF_RX0_DATA_3 */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065, 6 ) |
#endif
#if NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_066, 7 ) | /* MTSIF_RX0_DATA_4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067, 6 ) | /* MTSIF_RX0_DATA_5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068, 5 )   /* MTSIF_RX0_CLK */
#else           
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_066, 6 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067, 5 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068, 0 ) 
#endif           
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16
    * sgpio 0-4
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_069 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_070 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_071 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_00 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_01 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_02 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_03 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_04 ) 
             );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_069, 3 ) | /* MTSIF_RX0_DATA_1 */
#if !NEXUS_HAS_DVB_CI        
#if NEXUS_PLATFORM_97346_I2SFF || NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_070, 5 ) | /* MTSIF_RX0_DATA_2 */
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_070, 4 ) |
#endif
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_071, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_00, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_01, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_02, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_03, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_04, 1 ) 
           );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17
    * sgpio 05
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &=~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_05));
    reg |=( BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_05, 1 ));
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17,reg);
            
    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_000 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_001 ) |
#if (BCHP_CHIP==7346) && (BCHP_VER==BCHP_VER_A0)
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_002 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_003 ) |
#endif
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_004 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_005 ) 
            );

    reg |=( 
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_000, 0 ) |
#else
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_000, 1 ) |
#endif
#if NEXUS_PLATFORM_97346_SHR44
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_001, 0 ) |
#else
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_001, 1 ) |
#endif
#if (BCHP_CHIP==7346) && (BCHP_VER==BCHP_VER_A0)
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_002, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_003, 1 ) |  
#endif
#if NEXUS_PLATFORM_97346_SHR44
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_004, 0 ) |
#else
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_004, 1 ) |
#endif
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_005, 1 ) 
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
     *
    */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_006 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_007 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_008 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_009 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_010 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_011 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_012 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_013 ) 
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_006, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_007, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_008, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_009, 1 ) | 
#if NEXUS_HAS_DVB_CI 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_010, 0 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_011, 0 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_012, 0 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_013, 0 )   
#else
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_010, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_011, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_012, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_013, 1 )   
#endif         
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);


    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_014 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_015 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_016 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_017 )    |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_018 )    |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_019 )    |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_020 )    |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_021 ) 
            );

    reg |=( 
#if !NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_014, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_015, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_016, 1 ) |  
#endif         
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_017, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_018, 0 ) | /* IR_INT */  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_019, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_020, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_021, 1 ) 
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);


    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_022 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_023 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_024 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_025 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_026 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_00 ) | 
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01 ) 
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_022, 1 ) | 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_023, 1 ) | 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_024, 1 ) | 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_025, 0 ) |  /* KEYPAD Interrupt */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_026, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_00, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01, 1 ) 
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);


    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, SVD0_OL);

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,16);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}
#endif
