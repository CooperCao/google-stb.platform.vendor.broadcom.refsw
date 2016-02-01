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
*   This file contains pinmux initialization for the 97400 reference board.
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
	Configure pin muxes for a 97401 reference platform
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

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
     * GPIO31: IR_INT
     * GPIO4-GPIO05   :  SDS Data 0 -1
     * GPIO0-GPIO3    : LED_KD0-3
     */
     /* GPIO 00 ...05 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_05)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_04)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_03)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_02)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_01)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_00)
            );
    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_05, 3) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_04, 3) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_03, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_02, 1)
            );

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);
    

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3
     * GPIO15: SDS OUT Clock
     * GPIO14: Ext_SC_CLK
     * GPIO13 : SDS Valid
     * GPIO12    :  SDS Sync
     * GPIO06-GPIO11: SDS Data 2-7
     */
    /* GPIO 06..15 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_15 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_14 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_13 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_12 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_11 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_10 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_09 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_08 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_07 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_06 ) 
            );
    reg |= (
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_15, 3 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_14, 2 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_13, 3 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_12, 3 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_11, 5 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_10, 5 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_09, 3 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_08, 3 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_07, 3 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_06, 3 ) 
        );


    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3,reg);


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4
     * GPIO 016...27
     *  GPIO27: LED_LS_0
     * GPIO24-GPIO26: GPIO
     *  GPIO23: SPI_S_MISO
     *  GPIO22: SPI_S_SS0B
     * GPIO21: I2S_0_SYNC
     * GPIO20: I2S_0_DATA
     * GPIO19: PPKT_I_CLK
     * GPIO18: PPKT_I_ERROR
     *  GPIO17: PPKT_I_VALID
     * GPIO16: PPKT_I_SYNC
     */
    /* If DVB-CI is enabled, GPIO16..23 are GPIO */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_16 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_17 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_18 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_19 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_20 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_21 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_22 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_23 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_24 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_25 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_26 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_27 )  
            );
    reg |=( 
           #if !NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_16, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_17, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_18, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_19, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_20, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_21, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_22, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_23, 1 ) | 
           #endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_24, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_25, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_26, 0 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_27, 2 )  
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4,reg);

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5
    *  GPIO28-GPIO30: LED_LS_1-3
    * GPIO31: IR_INT
    * GPIO32: MII_RXD_03 Daughter card connector
    * GPIO33: MII_RXD_02
    * GPIO34: MII_RXD_01
    * GPIO35: MII_RXD_00
    * GPIO36: MII_RXD_ER
    * GPIO37: MII_RXD_EN
    * 
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_28 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_29 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_30 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_31 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_32 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_33 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_34 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_35 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_36 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_37 )
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_28, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_29, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_30, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_31, 0) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_32, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_33, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_34, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_35, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_36, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_37, 1 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5,reg);



    /* GPIO 38-47 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6
     * GPIO38: MII_CRS Daughter card continued
     * GPIO39: MII_COL
     * GPIO40: MII_RX_CLK
     * GPIO41: MII_MDIO
     * GPIO42: MII_MDC
     * GPIO43: MII_RX_CLK
     * GPIO44: MII_TX_EN
     * GPIO45: MII_TX_ER
     * GPIO46: MII_TXD_03
     * GPIO47: MII_TXD_02
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_38 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_39 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_40 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_41 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_42 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_43 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_44 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_45 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_46 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_47 )
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_38, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_39, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_40, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_41, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_42, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_43, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_44, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_45, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_46, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_47, 1 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,reg);
    
    /* GPIO 48-51 & GPIO 53-58 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
    * GPIO48: MII_TXD_01
    * GPIO49: MII_TXD_00
   * GPIO50: PWM_0
   * GPIO51: PWM_1
   * GPIO53: PPKT_I_DATA_0
   * GPIO54: PPKT_I_DATA_1
   * GPIO55: PPKT_I_DATA_2
   * GPIO56: PPKT_I_DATA_3
   * GPIO57: PPKT_I_DATA_4
   * GPIO58: PPKT_I_DATA_5
    */
    /* With DVB-CI enabled, GPIO 53..58 become GPIO pins */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_48 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_49 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_50 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_51 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_53 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_54 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_55 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_56 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_57 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58 ) 
            );
    reg |=( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_48, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_49, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_50, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_51, 1 ) |
           #if NEXUS_HAS_DVB_CI
           0
           #else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_53, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_54, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_55, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_56, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_57, 3 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58, 3 ) 
           #endif
           );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
    * GPIO59: PPKT_I_DATA_6
    * GPIO60: PPKT_I_DATA_7
    * GPIO61: SC_IO_1
    * GPIO62: SC_CLK_OUT_1
    * GPIO63: SC_RAT_1
    * GPIO64: SC_PRES_1
    * GPIO65: SC_VCC_1
    * GPIO66: PKT_CLK1
    * GPIO67: PKT_DATA1
    * GPIO68: PKT_SYNC1
    * GPIO69: PKT_VALID1
    */
    /* GPIO 59-60 become GPIO with DVB-CI enabled */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_59 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_60 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_61 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_62 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_63 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_64 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_65 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_68 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_69 )
            );
    reg |=( 
           #if !NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_59, 3 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_60, 3 ) | 
           #endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_61, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_62, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_63, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_64, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_65, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_68, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_69, 1 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
     * GPIO70: PKT_ERROR1
     * GPIO52: EPHY_LINK
     *  GPIO77: EPHY_ACTIVITY
     */
    /* GPIO 70-77 && GPIO 52  */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_70 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_71 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_73 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_74 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_52 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_77 )  
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_70, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_71, 1 ) | /* ir_in0 */ 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_72, 1 ) | /* ir_in1*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_73, 1 ) | /* qpsk_if_agc */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_74, 1 ) | /* qpsk_rf_agc */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_52, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_77, 1 )  
          );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

   /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
    * SGPIO00: BSC_M0_SCL 
    * SGPIO01:  BSC_M0_SDA
    * SGPIO02: BSC_M1_SCL
    * SGPIO03: BSC_M1_SDA
    * SGPIO04: BSC_M2_SCL
    * SGPIO05: BSC_M2_SDA
    * SGPIO06: BSC_M3_SCL
    * SGPIO07: BSC_M3_SDA
   
   */
    /* SGPIO 00-07 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_00 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_01 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_02 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_03 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_04 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_05 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_06 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_07 ) 
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_00, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_01, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_02, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_03, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_04, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_05, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_06, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, sgpio_07, 1 ) 
            );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

#ifdef SOAP
	reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, uart_txdb) | BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, uart_rxdb));
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, uart_txdb, 0);
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, uart_rxdb, 0);
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);
#else
    /* Configure the AVD UARTS to debug mode.  AVD0_OL -> UART1, AVD1_IL -> UART2. */
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, uart_rxdb, 2 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, uart_txdb, 2 ));

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

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

