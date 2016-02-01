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
	Configure pin muxes for a 97344 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{
    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;

#if BCHP_VER == BCHP_VER_A0
    /* Writing pinmuxes in order from pg. 10 of the schematic */
    /* GPIO PIN MUX CTRL 0 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_68)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_69)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_70)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_71)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_72)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_73)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_74)	 |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_75)
            );
    reg |= ( 
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_68, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_69, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_70, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_71, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_72, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_73, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_74, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_0, gpio_75, 1)
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_0, reg);
#endif


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1
     * GPIO76: IR_INT
     * GPIO4-GPIO05   :  SDS Data 0 -1
     * GPIO0-GPIO3    : LED_KD0-3
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1);
    reg &= ~(
#if BCHP_VER == BCHP_VER_A0
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_76)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_77)  |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_78)  
            );
    reg |= ( 
#if BCHP_VER == BCHP_VER_A0
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_76, 1) |
#endif
 
#if NEXUS_HAS_DVB_CI        
#if BCHP_VER == BCHP_VER_A0
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_77, 0) |
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_78, 0) 
#else  
#if BCHP_VER == BCHP_VER_A0
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_77, 1) |
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_1, gpio_78, 1) 
#endif
            );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_1, reg);


    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5
     * GPIO31: IR_INT
     * GPIO4-GPIO05   :  SDS Data 0 -1
     * GPIO0-GPIO3    : LED_KD0-3
     */
     /* GPIO 00 ...05 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
	reg &= ~(
#if BCHP_VER == BCHP_VER_A0
	 		BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_83)  |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_81)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_82)  
            );
    reg |= ( 
#if BCHP_VER == BCHP_VER_A0
	 		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_83, 1) |
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_81, 1) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, gpio_82, 1) 
            );
	

    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);
    

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
	reg &= ~(
#if BCHP_VER == BCHP_VER_A0
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_06 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_83 ) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_00 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_01 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_02 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_03 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_04 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_05 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_84 )
            );
    reg |= (
#if BCHP_VER == BCHP_VER_A0
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_06, 1 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_83, 1 ) |
#endif
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_00, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_01, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_02, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_03, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_04, 1 ) | 
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_05, 1 ) |
        BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, gpio_84, 1 )
        );	

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6,reg);


    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7
    *
    * 
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
	reg &=~(
#if BCHP_VER == BCHP_VER_A0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_14) | 
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_06) | 
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_07) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_08) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_09) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_10) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_11) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_12) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_13) 
            );
    reg |=( 
#if BCHP_VER == BCHP_VER_A0
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_14, 1 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_06, 1 ) |
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_07, 4 ) |  /* AVD outer uart */
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_08, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_09, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_10, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_11, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_12, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, gpio_13, 1 ) 
           );	

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7,reg);



    /* GPIO 38-47 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_25) |
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_14) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_15) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_16) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_17) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_18) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_19) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_23) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_24)
            );
    reg |=( 
#if BCHP_VER == BCHP_VER_A0		
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_25, 1 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0		
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_14, 1 ) |
#endif
#if !NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_15, 1 ) | 
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_16, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_17, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_18, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_19, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_23, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, gpio_24, 1 )       
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8,reg);
    
    /* GPIO 48-51 & GPIO 53-58 */
    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9
     *
    */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_35 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_25 ) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_26 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_27 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_28 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_29 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_30 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_31 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_34 ) 
            );
    reg |=( 
#if BCHP_VER == BCHP_VER_A0
     		BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_35, 1 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_25, 1 ) |
#endif
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_26, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_27, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_28, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_29, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_30, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_31, 1 ) |
            BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, gpio_34, 3 )  /* AVD outer uart */
            );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10
    *
    */
   /* Not 52 */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0
		    BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_41 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_35 ) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_36 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_37 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_38 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_39 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_40 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_32 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_33 ) 
            );
    reg |=( 
#if BCHP_VER == BCHP_VER_A0
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_41, 0 )  |
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_41, 1 )  |
#endif
#endif
#if BCHP_VER >= BCHP_VER_B0
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_35, 1 )  |
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_36, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_37, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_38, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_39, 1 )  |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_40, 1 )  | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_32, 1 )  | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, gpio_33, 1 )  
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10,reg);

    /*  BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11
     *
     */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0	
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_50 )  |
#endif
#if BCHP_VER >= BCHP_VER_B0	
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_41 )  |
#endif

            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_42 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_44 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_45 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_46 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_47 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_48 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_49 ) 
            );
    reg |=( 
#if BCHP_VER == BCHP_VER_A0
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_50, 2 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
#if NEXUS_HAS_DVB_CI
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_41, 0 )  |
#else
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_41, 1 )  |
#endif
#endif
#if !NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_42, 1 ) | 
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_44, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_45, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_46, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_47, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_48, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, gpio_49, 2 ) 
          );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11,reg);

   /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12
    *
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0
		    BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_21 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_50 ) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_51 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_52 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_53 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_54 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_55 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_56 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_20 ) 
            );
    reg |=( 
#if NEXUS_HAS_DVB_CI
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_20, 0 ) | 
#if BCHP_VER == BCHP_VER_A0
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_21, 0 ) |
#endif
#else
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_20, 1 ) |
#if BCHP_VER == BCHP_VER_A0
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_21, 1 ) |
#endif
#endif
#if BCHP_VER >= BCHP_VER_B0
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_50, 2 ) |
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_51, 2 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_52, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_53, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_54, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_55, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, gpio_56, 1 ) 
            );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12,reg);

       /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13
    *includes 52
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &=~(
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_21 ) |
#endif		
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_22 ) 
            );
#if !NEXUS_HAS_DVB_CI
    reg |=( 
#if BCHP_VER >= BCHP_VER_B0 
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_21, 1 ) |
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, gpio_22, 1 ) 
            );
#endif

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13,reg);

     /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14
    * sgpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0
		    BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_63 ) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_57 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_58 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_59 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_60 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_61 ) |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_62 ) 
            );
    reg |=(
#if BCHP_VER == BCHP_VER_A0
     	   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_63, 1 ) |
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_57, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_58, 1 ) | 
#if !NEXUS_HAS_DVB_CI        
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_59, 1 ) | 
#endif 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_60, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_61, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, gpio_62, 1 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15
    * gpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0
		    BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, sgpio_00 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
			BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_63) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_64 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_65 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_66 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_67 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_43 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_79 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_80 ) 
            );
    reg |=( 
#if BCHP_VER == BCHP_VER_A0
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, sgpio_00, 1 ) |
#endif
#if BCHP_VER >= BCHP_VER_B0
		   BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_63, 1 ) | 
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_64, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_65, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_66, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_67, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_43, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_79, 1 ) | 
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, gpio_80, 1 ) 
           );

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15,reg);

    /* BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16
    * sgpio 0-7
   */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &=~(
#if BCHP_VER == BCHP_VER_A0
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_01 ) |
#endif
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_02 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_03 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_04 ) | 
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_05 ) 
             );
    reg |=( 
#if BCHP_VER == BCHP_VER_A0
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_01, 1 ) |
#endif
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_02, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_03, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_04, 1 ) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, sgpio_05, 1 ) 
           );
    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16,reg);

    /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0
     *
     */
    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_11 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_12 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_13 ) |
            BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_14 ) 
            );

    reg |=( 
	#if NEXUS_PLATFORM_7418SFF_H /*for RF4CE remote set RESET/WK/INTOUT pin to AON_GPIO  */
		   BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 0 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 0 ) | 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02, 0 ) |
	#else 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 4 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 4 ) | 
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02, 4 ) |
    #endif
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 0 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_11, 1 ) |  
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_12, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_13, 1 ) |
           BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_14, 1 ) 
           );

    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);

    /*
    * BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1
    *
   */
   reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
   reg &=~(
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_04 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_05 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_16 ) 
           );

   reg |=( 
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_04, 4 ) |
#if NEXUS_PLATFORM_7418SFF_H
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_05, 0 ) |  
#else 
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_05, 4 ) | 
#endif           
		/*Change value for aon gpio 6/7 for NDS. */
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_06, 0 ) |  
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_07, 0 ) | 
#if NEXUS_PLATFORM_7418SFF_H
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 1 ) |  /*EXT_IRQB_10*/
#else 
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 4 ) |  
#endif
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 1 ) |  
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15, 0 ) |  
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_16,  0)   
          );

   BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);


   /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2
    *
    */
   reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
   reg &=~(
#if BCHP_VER >= BCHP_VER_B0
		   BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_02 ) |
		   BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_03 ) |
#endif 
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_10 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_01 ) 
           );

   reg |=( 
#if BCHP_VER >= BCHP_VER_B0
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_02, 1 ) |  
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_03, 1 ) | 
#endif
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_gpio_10, 0 ) | /* IR INT*/  
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00, 1 ) |  
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_01, 1 )   
          );

   BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);


   /* BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3
    *
    */
   reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3);
   reg &=~(
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_17 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_18 ) 
           );

   reg |=( 
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_17, 0 ) | 
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_3, aon_gpio_18, 0 ) 
          );

   BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_3, reg);


    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL);

   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL, port_4_cpu_sel));
   reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_4_cpu_sel, SVD0_OL);
   /* reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL, port_2_cpu_sel, SVD0_IL); */

    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL,reg);

   /* dont know if this is needed as this is done inside XVD */
   reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
   reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
   reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable,16);
   BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}

