/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*    This file contains pinmux initialization for the 97231 reference board.
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


#if NEXUS_PLATFORM == 97231

/***************************************************************************
Summary:
    Configure pin muxes for the 97231 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0:gpio_00-gpio_07 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1:gpio_08-gpio_15 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2:gpio_16-gpio_25 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3:gpio_26-gpio_33 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4:gpio_34-gpio_41 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5:gpio_42-gpio_45 programmed by CFE/OS 
    */
    
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_16)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_23)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_24) 
            );
    
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_16, 1) | /* EBI_WE1B */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_23, 1) | /* EBI_WE1B */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_24, 2);  /* SPI_SSb     */
        
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);
    
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
    
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_38)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_39)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_40) 
            );
    
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_38, 2) | /* SPI_MOSI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_39, 2) | /* SPI_MISO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, gpio_40, 2);  /* SPI_SCK   */
        
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);

    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_46)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_47)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_48)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_49) 
            );


    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_46, 3) |  /* EBI_ADDR14       */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_47, 1) |  /* POD2CHIP_MDI0    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_48, 1) |  /* POD2CHIP_MDI1    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_49, 1);   /* POD2CHIP         */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_50)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_51)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_52)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_53)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_54)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_55)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_56)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_57) 
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_50, 1) |  /* POD2CHIP_MDI3    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_51, 1) |  /* POD2CHIP_MDI4    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_52, 1) |  /* POD2CHIP_MDI5    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_53, 1) |   /* POD2CHIP_MDI6   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_54, 1) |  /* POD2CHIP_MDI7    */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_55, 1) |  /* POD2CHIP_MISTRT  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_56, 1) |  /* POD2CHIP_MIVAL   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_57, 1);   /* CHIP2POD_MCLKO   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);



    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_59)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_60)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_61)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_62)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_63)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_64)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_65)
            );  

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_58, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_59, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_60, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_61, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_62, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_63, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_64, 1) |  /* POD2CHIP_MDI3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_65, 1) ;  /* POD2CHIP_MDI3   */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_68)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_69)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_70)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_71)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_72)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_73)
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_66, 1) |  /* CHIP2POD_MOSTRT */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_67, 1) |  /* CHIP2POD_MOVAL  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_68, 3) |  /* EBI_ADDR13      */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_69, 3) |  /* EBI_ADDR12      */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_70, 2) |  /* EBI_ADDR2       */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_71, 2) |  /* EBI_ADDR1       */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_72, 2) |  /* EBI_ADDR0       */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_73, 1) ;  /* MPOD SDI        */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_74)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_75)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_76)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_77)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_78)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_79)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_80)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_81)  
            );

    reg |=  
#if !NEXUS_HAS_DVB_CI    
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_74, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_75, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_76, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_77, 1) |  /* RMX0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_78, 1) |  /* RMX0   */
#endif            
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_79, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_80, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_81, 1) ;  /* NDS-SC0   */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_82)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_83)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_84)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_85)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_86)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_87)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_88)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_89)  
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_82, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_83, 1) |  /* NDS-SC0   */
#if NEXUS_PLATFORM_7231_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_87, 3) |  /* MTSIF   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_88, 3) |  /* MTSIF   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_89, 3) ;  /* MTSIF   */
#else            
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_87, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_88, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_89, 1) ;  /* PKT1   */
#endif
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);


    /* gpio_092-gpio_093 uart0 set by CFE */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_94)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_95)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_96)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_97)
            );
#if NEXUS_PLATFORM_7231_EUSFF || NEXUS_PLATFORM_7231_EUSFF_V20
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90, 1) |  /* Legacy mode transport PKT1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91, 1) |  /* Legacy mode transport PKT1 */
#elif NEXUS_PLATFORM_7231_FBTSFF || NEXUS_PLATFORM_7231_DCSFBTSFF
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90, 3) |  /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91, 3) |  /* MTSIF */
#else            
    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90, 2) |  /* PKT3   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91, 2) |  /* PKT3   */
#endif          
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_94, 3) |  /* POD   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_95, 4) |  /* POD   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_96, 4) |  /* POD   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_97, 4) ;  /* POD2CHIP_MCLKI   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);


    /* gpio_106 set by OS/CFE */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_98)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_99)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105)  
            );

    reg |= ( 
#if NEXUS_PLATFORM_7231_EUSFF || NEXUS_PLATFORM_7231_EUSFF_V20
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_98, 2) |  /* EXT_IRQ2 for 3461 & hmirx_nxp   */
#else            
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_98, 1) |  /* GP98_SF_HOLDb/(GP98_BCM3128_IRQb/GP98_DCARD1_IRQb)   */
#endif
#if !NEXUS_HAS_DVB_CI               
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_99, 1) |  /* GP99_SF_WPb/(GP99_POD_IRQb) */
#endif
#if NEXUS_PLATFORM_7231_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 7)|  /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 7)|  /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 4)|  /* MTSIF */
#else            
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1)|  /* PKT 2 */
#endif
#if NEXUS_HAS_DVB_CI               
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1)  /* PKT3_SYNC */
#elif NEXUS_PLATFORM_7231_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 7)| /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 6)  /* MTSIF */
#else                   
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1)|  /* PKT3_SYNC */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 1)  /* PKT2_ERR */
#endif            
            );
    
    if(NEXUS_StrCmp(NEXUS_GetEnv("hddvi_width"), "30")==0)
    {
        BDBG_WRN(("enabling 30 bit hd-dvi"));
        reg |= ( BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 2)|  /* HD_DVI0_0 */
                 BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 2)  /* HD_DVI0_10 */
                );
    }
    else
    {
        reg |= ( BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1)|  /* PKT 2 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1)  /* PKT 2 */
            );
    }
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);


    /* gpio_107,gpio_108,gpio_109,gpio_110,gpio_111,gpio_112,gpio_113,gpio_114 
       set by CFE/OS for USB and ENET */


    /*  gpio_122 -  gpio_126 is TDB AON set by OS???? */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117)  |
#if NEXUS_HAS_DVB_CI             
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_122)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_123)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_124)              
#else            
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118)
#endif            
            );


    reg |= (
#if NEXUS_PLATFORM_7231_FBTSFF || NEXUS_PLATFORM_7231_DCSFBTSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 7) | /* MTSIF */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 7) | /* MTSIF */            
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 7)   /* MTSIF */
#elif NEXUS_PLATFORM_7231_EUSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 5) | /* HD_DVI0_21 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 5) | /* HD_DVI0_20 */              
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 5)   /* HD_DVI0_11 */
#elif NEXUS_PLATFORM_7231_EUSFF_V20
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 4) | /* HD_DVI0_21 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 4)   /* HD_DVI0_20 */
#elif NEXUS_PLATFORM_7231_CSFF
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 4) | /* EXT_IRQ1 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 4)   /* EXT_IRQ2*/
#else
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 1) | /* PKT_4 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 1) | /* PKT_4 */            
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 1)   /* PKT_4 */
#endif 
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);

/* this is going to break SDIO , memc error from kernel 
    build the kernel with CONFIG_MMC  option or use
    following commands in CFE to disable SDIO in the kernel 
    e 0xb04136fc 0x00000001
    e 0xb04138fc 0x00000001
*/
    
#if NEXUS_PLATFORM_7231_EUSFF  || NEXUS_PLATFORM_7231_EUSFF_V20
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_128)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_129)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_130)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_131)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_132)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_133)  |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_134)
            );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_128, 2) |   /* i2s input */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_129, 2) |   /* i2s input */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_130, 2) |   /* i2s input */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_132, 3) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_133, 4) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_134, 3)    /* HD_DVIO */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);

    /* gpio_139 - gpio_147 MII set by OS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_135)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_136)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_138)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_139)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_140)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_141)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_142)  
            );

    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_135, 4) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_136, 5) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_138, 4) |  /* HD_DVI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_139, 0) |  /* tbd gpio */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_140, 3) |  /* HD_DVI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_141, 3) |  /* HD_DVI */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_142, 4)    /* HD_DVI */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_143) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_144) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_145) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_146) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_148) 
             );
    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_143, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_144, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_145, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_146, 4) |   /*  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_148, 3)     /*  */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);

#endif

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_02)
             );
    reg |= (
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_02, 1)    /* BSC_M1_SCL */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);


    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_04) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_05) 
             );

    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03, 1) |  /* BSC_M1_SDA */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_04, 1) |  /* BSC_M2_SCL,nxp on eusffv10 */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_05, 1)    /* BSC_M2_SDA nxp on eusffv10*/
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, reg);
    
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05) 
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 1 ) | /* aud spdif o/p */
#if NEXUS_PLATFORM_7231_DCSFBTSFF
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 4)  |  /* HD_DVI0_17 */
#else
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 7)  |  /* HD_DVI0_17 */
#endif
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 7)     /* HD_DVI0_16 */
          );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);
    
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00 ) | /* hdmi tx BSC_M0_SCL */  
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01 )   /* hdmi tx BSC_M0_SDA */
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00, 1 ) | 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01, 1 )   
           );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);


    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
     *
    */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13 ) 
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06, 7 ) |  /* HD_DVI0_15 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07, 6 ) |  /* HD_DVI0_14 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 6 ) |  /* HD_DVI0_13 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 6 ) |  /* HD_DVI0_12 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 6 ) |  /* HD_DVI0_9 */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 6 )    /* HD_DVI0_8 */
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);


    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20 ) 
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_14, 6 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_15, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_16, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_17, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_18, 7 ) |  /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_19, 7 ) |   /* HD_DVI */
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_20, 7 )    /* HD_DVI */
           );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);

#if 1
#if !NEXUS_HAS_DVB_CI
    /* AVD UARTS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_94)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_95)  
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_94, 4) |  /* 4 = ALT_TP_OUT_0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_95, 5) ;  /* 5 = ALT_TP_IN_0   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);
#endif
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, SVD0_OL);
    /* reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, SVD0_IL);  */
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,16);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

#endif

    return BERR_SUCCESS;
}

#elif NEXUS_PLATFORM == 97230
/***************************************************************************
Summary:
    Configure pin muxes for the 97231 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

    /* 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0:gpio_00-gpio_07 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1:gpio_08-gpio_15 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2:gpio_16-gpio_25 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3:gpio_26-gpio_33 programmed by CFE/OS 
       BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4:gpio_34-gpio_41 programmed by CFE/OS 
    */
#if 0
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &= ~(
           BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_79)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_80)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_81)  
            );

    reg |=  
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_79, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_80, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_81, 1) ;  /* NDS-SC0   */
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);
#endif

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &= ~(

            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_82)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_83)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_84)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_85)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_86)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_87)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_88)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_89)  
            );

    reg |= 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_82, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_83, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_84, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_85, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_86, 1) |  /* NDS-SC0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_87, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_88, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_89, 1) ;  /* PKT1   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);


    /* gpio_092-gpio_093 uart0 set by CFE */

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91) 
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_90, 1) |  /* PKT1   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_91, 1);

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);



    /* gpio_106 set by OS/CFE */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_98)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_99)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105)  
            );

    reg |= ( 
            /* TDB */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_98, 1) |  /* GP98_SF_HOLDb/(GP98_BCM3128_IRQb/GP98_DCARD1_IRQb)   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_99, 1) |  /* GP99_SF_WPb/(GP99_POD_IRQb) */

            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_101, 1)|  /* PKT 2 (97230)*/
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_102, 1)|  /* PKT 2 (97230)*/
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_103, 1)|  /* PKT 2 (97230)*/
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_104, 1)|  /* PKT2_VALID (97230)*/
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_105, 1)   /* PKT2_ERR (97230)*/
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);


    /* gpio_107,gpio_108,gpio_109,gpio_110,gpio_111,gpio_112,gpio_113,gpio_114 
       set by CFE/OS for USB and ENET */
    
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118) 
            );

    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_116, 1) |  /* PKT_4   (same for 97230)*/
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_117, 1) |  /* PKT_4   (same for 97230)*/
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_118, 1)   /* PKT_4   (same for 97230) */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);

    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_131)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_132)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_133)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_134)
            );

    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_131, 1) |  /* TBD Schematics say GPIO_131/CARD_2/SDIO0_LED/VEC_HSYNC---> GP123_POD_VS1  */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_132, 3) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_133, 4) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_134, 3)    /* HD_DVIO */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);


    /* gpio_139 - gpio_147 MII set by OS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_135)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_136)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_138)  
            );


    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_135, 4) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_136, 5) |  /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, gpio_138, 4) 
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);


  /* set by OS GP149_MDC_ENET */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_148) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_02)
             );

    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, gpio_148, 3)  | /* HD_DVIO */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, sgpio_02, 1)   /* BSC_M1_SCL */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);



    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18);
    reg &= ~(
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_04) |
             BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_05) 
             );

    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_03, 1) |  /* bsc */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_04, 1) |  /* bsc */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_18, sgpio_05, 1)    /* bsc */
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_18, reg);


    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00 ) 
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 1 )  /* aud spdif o/p */
          );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);


    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    reg &=~(
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13 )
            );

    reg |=( 
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 6 )
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);


    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00 ) | /* hdmi tx BSC_M0_SCL */  
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01 )   /* hdmi tx BSC_M0_SDA */
            );

    reg |=( 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_00, 1 ) | 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, sgpio_01, 1 )  
           );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);

    /* AVD UARTS */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_94)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_95)  
            );

    reg |=  BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_94, 4) |  /* 4 = ALT_TP_OUT_0   */
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_95, 5) ;  /* 5 = ALT_TP_IN_0   */

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_1_cpu_sel, SVD0_OL);
    /* reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, SVD0_IL);  */
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,16);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}
#else
#error "Please define the NEXUS_PLATFORM variable"
#endif


