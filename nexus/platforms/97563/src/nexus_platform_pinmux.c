/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*  $brcm_Workfile: $
*  $brcm_Revision: $
*  $brcm_Date: $
*
*  Module Description:
*
*  Revision History:
*
*  $brcm_Log: $
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
    Configure pin muxes for the 97358 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
        BREG_Handle hReg = g_pCoreHandles->reg;
        uint32_t reg;

        /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6
          * GPIO_18    :
          * GPIO_20    :
          * GPIO_23    :
          * GPIO_24    :
          * GPIO_25    :
          */

        reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);

        reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_18) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_20) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_23) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_24) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_25)
                );

        reg |=(
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_18, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_20, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_23, 3) | /* uart tx 1*/
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_24, 3) | /* uart rx 1*/
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_25, 1)
                );

        BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1
          * GPIO_26    :
          * GPIO_27    :
          * GPIO_28    :
          * GPIO_29    :
          * GPIO_30    :
          * GPIO_31    :
          * GPIO_32    :
          * GPIO_41    :
          */

        reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);

        reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_26) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_27) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_28) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_29) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_30) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_31) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_32) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_41)
                );

        reg |=(
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_26, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_27, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_28, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_29, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_30, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_31, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_32, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_41, 1)
                );

        BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);



        /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2
          * GPIO_42    :
          * GPIO_43    :
          * GPIO_44    :
          * GPIO_45    :
          * GPIO_79    :
          * GPIO_80    :
          * GPIO_81    :
          * GPIO_82    :
          */

        reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);

        reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_42) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_43) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_44) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_45) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_79) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_80) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_81) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_82)
                );

        reg |=(
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_42, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_43, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_44, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_45, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_79, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_80, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_81, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_82, 1)
                );

        BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

        /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3
          * GPIO_83    :
          * GPIO_84    :
          * GPIO_85    :
          * GPIO_86    :
          * GPIO_87    :
          * GPIO_88    :
          * GPIO_101    :
          * GPIO_102    :
          */

        reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);

        reg &= ~(
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_83) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_84) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_85) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_86) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_87) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_88) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_101) |
                BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_102)
                );

        reg |=(
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_83, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_84, 0) |   /* needed to be zero for smartcard*/
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_85, 0) |       /* needed to be zero for smartcard*/
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_86, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_87, 2) |   /* uart tx 0*/
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_88, 2) |  /* uart rx 0*/
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_101, 1) |
                BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_102, 1)
                );

        BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);

        /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4
                  * GPIO_105    :
                  */

                reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);

                reg &= ~(
                        BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_105)
                        );

                reg |=(
                        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_105, 1)
                        );

                BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);


        /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0
        * AON_IR_IN0     : AON_IR_IN0(0)
        * AON_S3_STANDBYB: AON_S3_STANDBYB(0)
        * AON_HDMI_HTPLG : AON_HDMI_HTPLG(0)
        * AON_GPIO_00    : AUD_SPDIF(1)
        * AON_GPIO_01    : ENET_LINK(1)
        * AON_GPIO_03    : LED_OUT(1)
        * AON_GPIO_04    : LED_LD0(1)
        * AON_GPIO_05    : SDS0_DSEC_SELVTOP(6)
    */

        reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);

        reg &=~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_ir_in0 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_s3_standbyb ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_hdmi_htplg ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05 )
                );

        reg |=(
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_ir_in0, 0 ) |      /* AON_IR_IN0 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_s3_standbyb, 0 ) | /* AON_S3_STANDBYB */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_hdmi_htplg, 0 )  | /* AON_HDMI_HTPLG */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 1 ) |     /* AUD_SPDIF */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 1 ) |     /* ENET_LINK */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 1 ) |     /* LED_OUT */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 0 ) |     /* AON_GPIO_04  */
#if NEXUS_PLATFORM_7563_DGL
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 0 )       /* AON_GPIO_05 */
#else
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 6 )       /* SDS0_DSEC_SELVTOP */
#endif
                );

        BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

        /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
        * AON_GPIO_06    : SDS0_DSEC_VCTL(6)
        * AON_GPIO_07    : RMX0_PAUSE(3)
        * AON_GPIO_08    : RMX0_CLK(3)
        * AON_GPIO_09    : RMX0_DATA(3)
        * AON_GPIO_10    : RMX0_SYNC(3)
        * AON_GPIO_11    : RMX0_VALID(3)
        * AON_GPIO_12    : LED_LS0(1)
        * AON_GPIO_13    : LED_LS1(1)
    */

        reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);

        reg &=~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13 )
                );
#if NEXUS_PLATFORM_75639_SFF
        reg |=(
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06, 0 ) |  /* AON_GPIO_6 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07, 0 ) |  /* AON_GPIO_7*/
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 6 ) |  /* SDIO0_PRES */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 0 ) |  /* AON_GPIO_9 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10, 0 ) |  /* AON_GPIO_10 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11, 0 ) |  /* AON_GPIO_11 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 5 ) |  /* SDIO0_PWR0 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 5 )    /* SDIO0_CLK */
                );
#else
        reg |=(
#if NEXUS_PLATFORM_7563_DGL
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06, 0 ) |  /* AON_GPIO_6 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07, 6 ) |  /* SF_WPb*/
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 6 ) |  /* SF_HOLDb */
#else
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06, 6 ) |  /* SDS0_DSEC_VCTL */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07, 3 ) |  /* RMX0_PAUSE*/
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 3 ) |  /* RMX0_DATA */
#endif
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 3 ) |  /* RMX0_CLK */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10, 3 ) |  /* RMX0_SYNC */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11, 3 ) |  /* RMX0_VALID */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 2 ) |  /* PKT2_CLK */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 2 )    /* PKT2_DATA */
                );
#endif
        BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

        /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2
        * AON_GPIO_14    : LED_LS2(1)
        * AON_GPIO_15    : LED_LS3(1)
        * AON_GPIO_16    : LED_LS4(1)
        * AON_GPIO_17    : LED_KD0(1)
        * AON_GPIO_18    : LED_KD1(1)
        * AON_GPIO_19    : LED_KD2(1)
        * AON_GPIO_20    : LED_KD3(1)
        * AON_SGPIO_00   : BSC_M3_SCL(1)
        */

        reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);

        reg &=~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00 )
                );

#if NEXUS_PLATFORM_75639_SFF
        reg |=(
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14, 5 ) |  /* SDIO0_LED */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 5 ) |  /* SDIO0_DAT0 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 5 ) |  /* SDIO0_DAT1 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 5 ) |  /* SDIO0_DAT2 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18, 5 ) |  /* SDIO0_DAT3 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 5 ) |  /* SDIO0_CMD */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20, 5 ) |  /* SDIO0_WPROT */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00,1 )    /* BSC_M3_SCL */
                );
#elif NEXUS_PLATFORM_7563_DGL
        reg |=(
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14, 2 ) |  /* PKT2_SYNC */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 0 ) |  /* SPI_M_CS1 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 4 ) |  /* SPI_M_SCK*/
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 4 ) |  /* SPI_M_MOSI */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18, 0 ) |  /* SPI_M_CS2 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 4 ) |  /* SPI_M_MISO */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20, 1 ) |  /* LED_KD3 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00,1 )    /* BSC_M3_SCL */
                );
#else
        reg |=(
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14, 2 ) |  /* PKT2_SYNC */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 0 ) |  /* LED_LS3 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 1 ) |  /* LED_LS4 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 1 ) |  /* LED_KD0 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18, 1 ) |  /* LED_KD1 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 1 ) |  /* LED_KD2 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20, 1 ) |  /* LED_KD3 */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00,1 )    /* BSC_M3_SCL */
                );
#endif

        BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);

        /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3
        * AON_SGPIO_01   : BSC_M3_SDA(1)
        */

        reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);

        reg &=~(
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01 ) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00) |
                BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01)
                );

        reg |=(
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_sgpio_01,1 ) |  /* BSC_M3_SDA */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00,1 ) |      /* BSC_M0_SCL */
                BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01,1 )                /* BSC_M0_SDA */
                );

        BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);

        /* Configure the AVD UARTS to debug mode.  AVD0_OL -> UART1, AVD1_OL -> UART2. */
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
        reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, AVD0_OL);
        reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, AVD0_IL);
        BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
        reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

#if 0
        /* Configure the Input Band source select options */
        /* DS->IB1; PKT2->IB2 */
        reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0);
        reg &= ~(BCHP_MASK(SUN_TOP_CTRL_GENERAL_CTRL_0, ib2_source));
        reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_GENERAL_CTRL_0,ib2_source, 1); /* PKT2 */
        BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_GENERAL_CTRL_0, reg);
#endif

    return BERR_SUCCESS;
}


