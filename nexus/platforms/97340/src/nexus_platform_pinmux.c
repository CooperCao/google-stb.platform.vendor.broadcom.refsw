/***************************************************************************
*     (c)2004-2009 Broadcom Corporation
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

BDBG_MODULE(nexus_platform_pinmux);


/***************************************************************************
Summary:
	Configure pin muxes for a 97342 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* Writing pinmuxes in order from pg. 10 of the schematic */
    /* GPIO 000..001 set by OS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4
     * GPIO31: IR_INT
     * GPIO4-GPIO05   :  SDS Data 0 -1
     * GPIO0-GPIO3    : LED_KD0-3
     */
     /* GPIO 00 ...05 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_05)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_04)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_03)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_02)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_01)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_00)
            );
    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_05, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_04, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_03, 0) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_02, 0) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_01, 0) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_00,  0)
            );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);
    

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_13 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_12 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_11 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_10 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_09 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_08 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_07 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_06 ) 
            );
    reg |= (
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_13, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_12, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_11, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_10, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_09, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_08, 1  ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_07, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_06, 1 ) 
        );


    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,reg);


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6
      *
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_14 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_15 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_16 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_17) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_18 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_19 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_20 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_21 ) 
            );
    reg |=( 
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_14, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_15, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_16, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_17, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_18, 1 ) | 
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_14, 2 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_15, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_16, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_17, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_18, 2 ) | 
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_19, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_20, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_21, 0 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,reg);

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
    *
    * 
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_22) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_23) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_24) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_25) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_26) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_27) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_28) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_29) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_22, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_23, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_24, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_25, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_26, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_27, 5 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_28, 6 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_29, 3 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);



    /* GPIO 38-47 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_30) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_31) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_32) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_33) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_34) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_35) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_36) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_37) 
            );
    reg |=( 
           /* Adding MII ENET not MII MOCA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_30, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_31, 0 ) | /* SPI */ 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_32, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_33, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_34, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_35, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_36, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_37, 1 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);
    
    /* GPIO 48-51 & GPIO 53-58 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
     *
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_38 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_39 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_40 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_41 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_42 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_43 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_44 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_45 ) 

            );
    reg |=( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_38, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_39, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_40, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_41, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_42, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_43, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_44, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_45, 1 ) 
            );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10
    *
    */
   /* Not 52 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_46 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_47 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_48 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_49 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_50 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_51 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_53 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_54 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_46, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_47, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_48, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_49, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_50, 1 )  | /* PWM or Moca link*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_51, 1 )  | /* PWM or Moca link*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_53, 2 )  | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_54, 2 )  
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
     *
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_55 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_56 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_57 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_58 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_59 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_60 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_61 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_62 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_55, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_56, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_57, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_58, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_59, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_60, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_61, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_62, 1 ) 
          );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

   /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12
    *
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_63 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_64 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_65 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_66 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_67 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_68 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_69 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_70 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_63, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_64, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_65, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_66, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_67, 1 ) | 
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_68, 0 ) | 
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_68, 1 ) | 
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_69, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_70, 0 ) /* SPI Interrupt? and PKT_1? */

            );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);

       /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13
    *includes 52
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_71 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_72 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_73 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_74 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_75 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_76 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_52 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_77 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_71, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_72, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_73, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_74, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_75, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_76, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_52, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_77, 1 ) 

            );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);

     /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14
    * sgpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_00 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_01 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_02 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_03 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_04 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_05 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_06 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_07 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_00, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_01, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_02, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_03, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_04, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_05, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_06, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, sgpio_07, 1 ) 

            );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,reg);

#ifdef SOAP
	reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, uart_txdb) | BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, uart_rxdb));
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, uart_txdb, 0);
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, uart_rxdb, 0);
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18,reg);
#else
    /* Configure the AVD UARTS to debug mode.  AVD0_OL -> UART1, AVD1_IL -> UART2. */
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, uart_rxdb, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, uart_txdb, 1 ));

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18,reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_OL);
   /* reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_IL); */

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);
#endif
   /* dont know if this is needed as this is done inside XVD */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,11);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}

