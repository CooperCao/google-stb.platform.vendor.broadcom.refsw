/***************************************************************************
*     (c)2004-2010 Broadcom Corporation
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

#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "priv/nexus_core.h"
#include "bchp_sun_top_ctrl.h"

BDBG_MODULE(nexus_platform_pinmux);

/***************************************************************************
Summary:
    Configure pin muxes for the 97420 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* Writing pinmuxes in order from pg. 14 of the schematic */

    /* GPIO 000..001 set by OS        */
    /* -------------------------------*/
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
     * GPIO_000    : ENET_ACTIVITY(1) 
     * GPIO_001    : ENET_LINK(1)
     */

    /* GPIO 002..015 set by OS        */
    /* -------------------------------*/
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
     * GPIO_002    : RGMII_RX_CLK(1) 
     * GPIO_003    : RGMII_RX_CTL(1)
     * GPIO_004    : RGMII_RXD_00(1)
     * GPIO_005    : RGMII_RXD_01(1)
     * GPIO_006    : RGMII_RXD_02(1)
     * GPIO_007    : RGMII_RXD_03(1)
     
     * BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
     * GPIO_008    : UART_TXD_0(1)
     * GPIO_009    : RGMII_TX_CLK(1)
     * GPIO_010    : RGMII_TX_CTL(1)
     * GPIO_011    : RGMII_TXD_00(1)
     * GPIO_012    : RGMII_TXD_01(1)
     * GPIO_013    : RGMII_TXD_02(1)
     * GPIO_014    : RGMII_TXD_03(1)
     * GPIO_015    : UART_RXD_0(1)
     */


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
     * GPIO_016    : POD2CHIP_MCLK(1)
     * GPIO_017    : AUD_FS_CLK0(3)
     * GPIO_018    : AUD_FS_CLK1(3)
     * GPIO_019    : MOCA_ACTIVITY(1)
     * GPIO_020    : NDS_SC_AUX_0(2)
     * GPIO_021    : NDS_SC_AUX_1(2)
     * GPIO_022    : TP_IN_22(7)
     * GPIO_023    : TP_OUT_23(7)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_017)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_019)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_020)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_021)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_022)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_023)
            );

#if NEXUS_HAS_MPOD
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016, 0) | /* POD2CHIP_MCLKI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_017, 3) | /* AUD_FS_CLK0 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018, 0) | /* AUD_FS_CLK1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_019, 1) | /* MOCA_ACTIVITY */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_020, 2) | /* NDS_SC_AUX_0 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_021, 2) | /* NDS_SC_AUX_1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_022, 7) | /* UART_DEBUG3_RXD_2 -> TP_IN_22  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_023, 7);  /* UART_DEBUG3_TXD_2 -> TP_OUT_23 */
#else
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_016, 1) | /* POD2CHIP_MCLKI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_017, 3) | /* AUD_FS_CLK0 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_018, 0) | /* AUD_FS_CLK1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_019, 1) | /* MOCA_ACTIVITY */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_020, 2) | /* NDS_SC_AUX_0 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_021, 2) | /* NDS_SC_AUX_1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_022, 7) | /* UART_DEBUG3_RXD_2 -> TP_IN_22  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_023, 7);  /* UART_DEBUG3_TXD_2 -> TP_OUT_23 */
#endif

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);
             
    
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10
     * GPIO_024    : EXT_IRQB_10(1)
     * GPIO_025    : CHIP2POD_SCLK(1)
     * GPIO_026    : POD2CHIP_SDI(1)
     * GPIO_027    : CHIP2POD_SDO(1)
     * GPIO_028    : CHIP2POD_SCTL(1)
     * GPIO_029    : POD2CHIP_MISTRT(1)
     * GPIO_030    : POD_MDET(1)
     * GPIO_031    : POD2CHIP_MDI0(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_024 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_025 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_026 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_027 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_028 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_029 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_030 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_031 ) 
            );

#if NEXUS_HAS_MPOD
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_024, 1 ) | /* EXT_IRQB_10 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_025, 0 ) | /* CHIP2POD_SCLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_026, 0 ) | /* POD2CHIP_SDI */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_027, 0) | /* CHIP2POD_SDO */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_028, 0 ) | /* CHIP2POD_SCTL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_029, 0 ) | /* POD2CHIP_MISTRT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_030, 0 ) | /* POD_MDET */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_031, 0 )   /* POD2CHIP_MDI0 */
           );
#else
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_024, 1 ) | /* EXT_IRQB_10 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_025, 1 ) | /* CHIP2POD_SCLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_026, 1 ) | /* POD2CHIP_SDI */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_027, 1 ) | /* CHIP2POD_SDO */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_028, 1 ) | /* CHIP2POD_SCTL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_029, 1 ) | /* POD2CHIP_MISTRT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_030, 1 ) | /* POD_MDET */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_031, 1 )   /* POD2CHIP_MDI0 */
           );
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
     * GPIO_032    : POD2CHIP_MDI1(1)
     * GPIO_033    : POD2CHIP_MDI2(1)
     * GPIO_034    : POD2CHIP_MDI3(1)
     * GPIO_035    : POD2CHIP_MDI4(1)
     * GPIO_036    : POD2CHIP_MDI5(1)
     * GPIO_037    : POD2CHIP_MDI6(1)
     * GPIO_038    : POD2CHIP_MDI7(1)
     * GPIO_039    : POD2CHIP_MDICLK(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_037 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_038 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_039 )  
            );

#if NEXUS_HAS_MPOD
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032, 0 ) | /* POD2CHIP_MDI1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033, 0 ) | /* POD2CHIP_MDI2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034, 0 ) | /* POD2CHIP_MDI3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035, 0 ) | /* POD2CHIP_MDI4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036, 0 ) | /* POD2CHIP_MDI5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_037, 0 ) | /* POD2CHIP_MDI6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_038, 0 ) | /* POD2CHIP_MDI7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_039, 0 )   /* POD2CHIP_MDICLK */
           );
#else
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_032, 1 ) | /* POD2CHIP_MDI1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_033, 1 ) | /* POD2CHIP_MDI2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_034, 1 ) | /* POD2CHIP_MDI3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_035, 1 ) | /* POD2CHIP_MDI4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_036, 1 ) | /* POD2CHIP_MDI5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_037, 1 ) | /* POD2CHIP_MDI6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_038, 1 ) | /* POD2CHIP_MDI7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_039, 1 )   /* POD2CHIP_MDICLK */
           );
#endif

#if NEXUS_HAS_DVB_CI
    reg &=~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_039) /* CD1 */
           ); 
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);
    
    
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12
     * GPIO_040    : CHIP2POD_MOVAL(1)
     * GPIO_041    : CHIP2POD_MOSTRT(1)
     * GPIO_042    : CHIP2POD_MDO0(1)
     * GPIO_043    : CHIP2POD_MDO1(1)
     * GPIO_044    : CHIP2POD_MDO2(1)
     * GPIO_045    : CHIP2POD_MDO3(1)
     * GPIO_046    : CHIP2POD_MDO4(1)
     * GPIO_047    : CHIP2POD_MDO5(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_045 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_046 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_047 )  
            );

#if NEXUS_HAS_MPOD
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040, 0 ) | /* CHIP2POD_MOVAL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, 0 ) | /* CHIP2POD_MOSTRT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042, 0 ) | /* CHIP2POD_MDO0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043, 0 ) | /* CHIP2POD_MDO1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044, 0 ) | /* CHIP2POD_MDO2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_045, 0 ) | /* CHIP2POD_MDO3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_046, 0 ) | /* CHIP2POD_MDO4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_047, 0 )   /* CHIP2POD_MDO5 */
           );
#else
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_040, 1 ) | /* CHIP2POD_MOVAL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_041, 1 ) | /* CHIP2POD_MOSTRT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_042, 1 ) | /* CHIP2POD_MDO0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_043, 1 ) | /* CHIP2POD_MDO1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_044, 1 ) | /* CHIP2POD_MDO2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_045, 1 ) | /* CHIP2POD_MDO3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_046, 1 ) | /* CHIP2POD_MDO4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_047, 1 )   /* CHIP2POD_MDO5 */
           );
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);
    

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13
     * GPIO_048    : CHIP2POD_MDO6(1)
     * GPIO_049    : CHIP2POD_MDO7(1)
     * GPIO_050    : CHIP2POD_MOCLK(1)
     * GPIO_051    : VO0_656_CLK(4)
     * GPIO_052    : PKT_ERROR1(1)
     * GPIO_053    : EXT_IRQB_1(3)
     * GPIO_054    : PKT_ERROR3(1)
     * GPIO_055    : VO0_656_0(2)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_053 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_054 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_055 )  
            );

#if NEXUS_HAS_MPOD
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048, 0 ) | /* CHIP2POD_MDO6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049, 0 ) | /* CHIP2POD_MDO7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050, 0 ) | /* CHIP2POD_MOCLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051, 4 ) | /* VO0_656_CLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052, 1 ) | /* PKT_ERROR1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_053, 3 ) | /* EXT_IRQB_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_054, 1 ) | /* PKT_ERROR3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_055, 2 )   /* VO0_656_0 */
           );
#else
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_048, 1 ) | /* CHIP2POD_MDO6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_049, 1 ) | /* CHIP2POD_MDO7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050, 1 ) | /* CHIP2POD_MOCLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051, 4 ) | /* VO0_656_CLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052, 1 ) | /* PKT_ERROR1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_053, 3 ) | /* EXT_IRQB_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_054, 1 ) | /* PKT_ERROR3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_055, 2 )   /* VO0_656_0 */
           );
#endif

#if NEXUS_HAS_DVB_CI
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_050 ) | /* IRQ/Ready */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_055 )  /* Vpp En0 */
            ); 
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);
    
    
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14
     * GPIO_056    : VO0_656_1(2)
     * GPIO_057    : VO0_656_2(2)
     * GPIO_058    : VO0_656_3(2)
     * GPIO_059    : VO0_656_4(2)
     * GPIO_060    : VO0_656_5(2)
     * GPIO_061    : VO0_656_6(2)
     * GPIO_062    : VO0_656_7(2)
     * GPIO_063    : PKT_CLK0(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_061 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_062 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_063 )  
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056, 2 ) | /* VO0_656_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057, 2 ) | /* VO0_656_2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058, 2 ) | /* VO0_656_3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059, 2 ) | /* VO0_656_4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060, 2 ) | /* VO0_656_5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_061, 2 ) | /* VO0_656_6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_062, 2 ) | /* VO0_656_7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_063, 1 )   /* PKT_CLK0 */
           );

#if NEXUS_HAS_DVB_CI
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056 ) | /* Vpp En1 */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_061 ) |  /* Vcc En0 */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_062 )  /* Vcc En1 */
            ); 
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,reg);
    

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15
     * GPIO_064    : PKT_CLK1(1)
     * GPIO_065    : PKT_CLK2(1)
     * GPIO_066    : PKT_CLK3(1)
     * GPIO_067    : I2S_CLK0_OUT(4)
     * GPIO_068    : PKT_CLK5(1)
     * GPIO_069    : PKT_DATA0(1)
     * GPIO_070    : PKT_DATA1(1)
     * GPIO_071    : PKT_DATA2(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_064 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_066 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_069 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_070 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_071 )  
            );
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_064, 1 ) | /* PKT_CLK1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_065, 1 ) | /* PKT_CLK2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_066, 1 ) | /* PKT_CLK3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067, 4 ) | /* I2S_CLK0_OUT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068, 1 ) | /* PKT_CLK5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_069, 1 ) | /* PKT_DATA0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_070, 1 ) | /* PKT_DATA1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_071, 1 )   /* PKT_DATA2 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15,reg);




    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16
     * GPIO_072    : PKT_DATA3(1)
     * GPIO_073    : I2S_DATA0_OUT(4)
     * GPIO_074    : PKT_DATA5(1)
     * GPIO_075    : PKT_SYNC0(1)
     * GPIO_076    : PKT_SYNC1(1)
     * GPIO_077    : PKT_SYNC2(1)
     * GPIO_078    : PKT_SYNC3(1)
     * GPIO_079    : I2S_LR0_OUT(4)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_072) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_073) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_074) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_075) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_076) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_077) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_078) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_079) 
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_072, 1) |  /* PKT_DATA3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_073, 4) |  /* I2S_DATA0_OUT */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_074, 1) |  /* PKT_DATA5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_075, 1) |  /* PKT_SYNC0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_076, 1) |  /* PKT_SYNC1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_077, 1) |  /* PKT_SYNC2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_078, 1) |  /* PKT_SYNC3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_079, 4)    /* I2S_LR0_OUT */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16,reg);
    


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17
     * GPIO_080    : PKT_SYNC5(1)
     * GPIO_081    : ALT_TP_IN_26(7)
     * GPIO_082    : ALT_TP_OUT_27(7)
     * GPIO_083    : SC_RST_1(1)
     * GPIO_084    : SC_PRES_1(1)
     * GPIO_085    : SC_VCC_1(1)
     * GPIO_086    : LED_KD_0(1)
     * GPIO_087    : LED_KD_1(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_080) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_081) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_082) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_083) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_084) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_085) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_086) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_087) 
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_080, 1) |  /* PKT_SYNC5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_081, 7) |  /* UART_DEBUG7_RXD_1 -> ALT_TP_IN_26 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_082, 7) |  /* UART_DEBUG7_TXD_1 -> ALT_TP_OUT_27 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_083, 1) |  /* SC_RST_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_084, 1) |  /* SC_PRES_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_085, 1) |  /* SC_VCC_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_086, 1) |  /* LED_KD_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_087, 1)    /* LED_KD_1 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17,reg);
    
    

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18
     * GPIO_088    : LED_KD_2(1)
     * GPIO_089    : LED_KD_3(1)
     * GPIO_090    : LED_LS_0(1)
     * GPIO_091    : LED_LS_1(1)
     * GPIO_092    : LED_LS_2(1)
     * GPIO_093    : LED_LS_3(1)
     * GPIO_094    : LED_LS_4(1)
     * GPIO_095    : LED_LD_0(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_090) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_091) |

            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_093) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_094) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_095) 
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_088, 1) |  /* LED_KD_2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_089, 1) |  /* LED_KD_3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_090, 1) |  /* LED_LS_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_091, 1) |  /* LED_LS_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_092, 1) |  /* LED_LS_2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_093, 1) |  /* LED_LS_3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_094, 1) |  /* LED_LS_4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, gpio_095, 1)    /* LED_LD_0 */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18,reg);
    


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19
     * GPIO_096    : LED_LD_1(1)
     * GPIO_097    : LED_LD_2(1)
     * GPIO_098    : LED_LD_3(1)
     * GPIO_099    : LED_LD_4(1)
     * GPIO_100   : LED_LD_5(1)
     * GPIO_101   : LED_LD_6(1)
     * GPIO_102   : LED_LD_7(1)
     * GPIO_103   : EXT_IRQB_4(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_096) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_097) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_098) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_099) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_100) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_101) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_102) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_103) 
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_096, 1) |  /* LED_LD_1 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_097, 1) |  /* LED_LD_2 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_098, 1) |  /* LED_LD_3 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_099, 1) |  /* LED_LD_4 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_100, 1) | /* LED_LD_5 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_101, 1) | /* LED_LD_6 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_102, 1) | /* LED_LD_7 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_103, 1)   /* EXT_IRQB_4 */
           );

#if NEXUS_HAS_DVB_CI
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_19, gpio_103 )  /* Reset */
            ); 
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_19,reg);



    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20
     * GPIO_104   : SC_IO_0(1)
     * GPIO_105   : SC_CLK_0(1)
     * GPIO_106   : SC_RST_0(1)
     * GPIO_107   : SC_PRES_0(1)
     * GPIO_108   : MDIO_ENET(4)
     * GPIO_109   : MDC_ENET(5)
     * GPIO_110   : CODEC_SCLK(1)
     * GPIO_111   : CODEC_FSYNCB(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_104) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_105) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_106) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_107) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_108) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_109) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_110) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_111) 
            );
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_104, 1) |  /* SC_IO_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_105, 1) |  /* SC_CLK_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_106, 1) |  /* SC_RST_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_107, 1) |  /* SC_PRES_0 */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_108, 4) |  /* MDIO_ENET */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_109, 5) |  /* MDC_ENET */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_110, 1) |  /* CODEC_SCLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_20, gpio_111, 1)    /* CODEC_FSYNCB */
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_20,reg);
    
   

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21
     * GPIO_112   : CODEC_MCLK(1)
     * SGPIO_00    : BSC_M0_SCL(1)
     * SGPIO_01    : BSC_M0_SDA(1)
     * SGPIO_02    : BSC_M1_SCL(1)
     * SGPIO_03    : BSC_M1_SDA(1)
     * SGPIO_04    : BSC_M2_SCL(1)
     * SGPIO_05    : BSC_M2_SDA(1)
     * SGPIO_06    : BSC_M3_SCL(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_112 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_00 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_01 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_02 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_03 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_04 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_05 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_06 ) 
            );

#if NEXUS_HAS_MPOD
    reg |=(
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_112, 1 ) |  /* CODEC_MCLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_00, 1 ) |  /* BSC_M0_SCL  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_01, 1 ) |  /* BSC_M0_SDA  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_02, 1 ) |  /* BSC_M1_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_03, 1 ) |  /* BSC_M1_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_04, 0 ) |  /* BSC_M2_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_05, 0 ) |  /* BSC_M2_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_06, 0 )    /* BSC_M3_SCL */
           );
#else
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, gpio_112, 1 ) |  /* CODEC_MCLK */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_00, 1 ) |  /* BSC_M0_SCL  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_01, 1 ) |  /* BSC_M0_SDA  */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_02, 1 ) |  /* BSC_M1_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_03, 1 ) |  /* BSC_M1_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_04, 1 ) |  /* BSC_M2_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_05, 1 ) |  /* BSC_M2_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_06, 1 )    /* BSC_M3_SCL */
           );
#endif

#if NEXUS_HAS_DVB_CI
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_04 ) | /* EBI bus enable */
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_21, sgpio_05 )  /* VS1 */
            ); 
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_21,reg);



    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_22
     * SGPIO_07    : BSC_M3_SDA(1)
     * SGPIO_08    : BSC_M4_SCL(1)
     * SGPIO_09    : BSC_M4_SDA(1)
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_22);
    reg &=~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_07 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_08 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_09 ) 
            );

#if NEXUS_HAS_MPOD
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_07, 0 ) |  /* BSC_M3_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_08, 1 ) |  /* BSC_M4_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_09, 1 )    /* BSC_M4_SDA */
           );
#else
    reg |=( 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_07, 1 ) |  /* BSC_M3_SDA */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_08, 1 ) |  /* BSC_M4_SCL */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_22, sgpio_09, 1 )    /* BSC_M4_SDA */
           );
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_22,reg);

    
    /* Configure the AVD UARTS to debug mode.  AVD0_OL -> UART1, AVD1_OL -> UART2. */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_3_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_7_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_7_cpu_sel, AVD0_OL);
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_3_cpu_sel, AUDIO_ZSP);
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SUN);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

#if NEXUS_DVO_DVI_LOOPBACK_SUPPORT
    /* hd_dvi0_clk_p and clkn */
    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052)); 
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_051, 5);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_052, 4);
 	BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

    /* hd_dvi0_de */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_061)); 
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_061, 4);
 	BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);

    /* hd_dvi0_hsync */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_080)); 
	reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_080, 2);
 	BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);

    /* hd_dvi0_vsync */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_079)); 
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_079, 2);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);


    /* hd_dvi0_00 and hd_dvi0_01 */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068)); 
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_067, 2);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_068, 2);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);

    /* hd_dvi0_02 and hd_dvi0_03 */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_073)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_074)); 
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_073, 2);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_074, 2);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);


    /* 
     * hd_dvi0_04, hd_dvi0_05, hd_dvi0_06, hd_dvi0_07, hd_dvi0_08 and hd_dvi0_09
     */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_061)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_062)); 
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_056, 4);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_057, 4);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_058, 4);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_059, 4);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_060, 4);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_061, 4);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_062, 4);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);


    /* hd_dvi0_10 */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_055)); 
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_055, 4);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

    /* hd_dvi0_11 */

    reg = BREG_Read32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_078)); 
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_079)); 
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_078, 2);
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_079, 2);
    BREG_Write32(g_pCoreHandles->reg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);

    /*
    SUN_TOP_CTRL.PIN_MUX_CTRL_13.gpio_051 = 5		''hd_dvi0_clk_p			
    SUN_TOP_CTRL.PIN_MUX_CTRL_13.gpio_052 = 4		'' clkn
    SUN_TOP_CTRL.PIN_MUX_CTRL_14.gpio_061 = 4		''hd_dvi0_de		
    SUN_TOP_CTRL.PIN_MUX_CTRL_17.gpio_080 = 2		''hd_dvi0_hsync
    SUN_TOP_CTRL.PIN_MUX_CTRL_16.gpio_079 = 2		''hd_dvi0_vsync		
    SUN_TOP_CTRL.PIN_MUX_CTRL_15.gpio_067 = 2		''hd_dvi0_00		
    SUN_TOP_CTRL.PIN_MUX_CTRL_15.gpio_068 = 2		''hd_dvi0_01		
    SUN_TOP_CTRL.PIN_MUX_CTRL_16.gpio_073 = 2		''hd_dvi0_02		
    SUN_TOP_CTRL.PIN_MUX_CTRL_16.gpio_074 = 2		''hd_dvi0_03		
    SUN_TOP_CTRL.PIN_MUX_CTRL_14.gpio_056 = 4		''hd_dvi0_04				
    SUN_TOP_CTRL.PIN_MUX_CTRL_14.gpio_057 = 4		''hd_dvi0_05		
    SUN_TOP_CTRL.PIN_MUX_CTRL_14.gpio_058 = 4		''hd_dvi0_06		
    SUN_TOP_CTRL.PIN_MUX_CTRL_14.gpio_059 = 4		''hd_dvi0_07		
    SUN_TOP_CTRL.PIN_MUX_CTRL_14.gpio_060 = 4		''hd_dvi0_08		
    SUN_TOP_CTRL.PIN_MUX_CTRL_14.gpio_062 = 4		''hd_dvi0_09		
    SUN_TOP_CTRL.PIN_MUX_CTRL_13.gpio_055 = 4		''hd_dvi0_10		
    SUN_TOP_CTRL.PIN_MUX_CTRL_16.gpio_078 = 2		''hd_dvi0_11		
    */

#endif
    return BERR_SUCCESS;
}


