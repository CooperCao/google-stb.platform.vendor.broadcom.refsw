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
#include "bchp_aon_pin_ctrl.h"

/* Define GOLd_BOARD to setup SHVD UARTS on GOLD BOARD, default is silver board */
#define GOLD_BOARD 0

BDBG_MODULE(nexus_platform_pinmux);

#if 0
ifeq ($(NEXUS_PLATFORM),97445)
ifeq ($(NEXUS_USE_7445_SV),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7445_SV=1
endif
ifeq ($(NEXUS_USE_7445_VMS_SFF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7445_VMS_SFF=1
endif
ifeq ($(NEXUS_USE_7445_C),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7445_C=1
endif
ifeq ($(NEXUS_USE_7445_DBS),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7445_DBS=1
endif
endif
#endif


/***************************************************************************
Summary:
    Configure pin muxes for the 97425 reference platform
Description:
    The core module must be initialized for this to be called
 ***************************************************************************/
NEXUS_Error NEXUS_Platform_P_InitPinmux(void)
{

    BREG_Handle hReg = g_pCoreHandles->reg;
    uint32_t reg;


#if 0
    /*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002)
            );


    reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_000, 1) | /*EBI_DATA_8*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_001, 1) | /*EBI_DATA_9*/
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, gpio_002, 1)   /*EBI_DATA_10*/
           );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);


    /*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    reg &= ~(
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_000)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_000)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_000)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_000)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_000)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_000)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_001)  |
            BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, gpio_002)
            );

    reg |= (
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_sgpio_04, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_sgpio_05, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_000, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_001, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_002, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_003, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_004, 0) |
           BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_2, onoff_gpio_005, 0)
           );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_2, reg);

    /*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_006)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_007)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_008)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_009)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_010)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_011)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_012)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_013)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_006, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_007, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_008, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_009, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_010, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_011, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_012, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_3, onoff_gpio_013, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_3, reg);

    /*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_014)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_015)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_016)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_017)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_018) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_019)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_020) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_021)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_014, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_015, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_016, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_017, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_018, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_019, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_020, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_4, onoff_gpio_021, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_4, reg);

    /*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_022)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_023)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_024)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_025)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_026) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_027)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_028) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_029)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_022, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_023, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_024, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_025, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_026, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_027, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_028, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_5, onoff_gpio_029, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_5, reg);


    /*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_030)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_031)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_032)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_033)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_034) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_035)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_036) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_037)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_030, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_031, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_032, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_033, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_034, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_035, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_036, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_6, onoff_gpio_037, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_6, reg);


    /*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_038)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_039)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_040)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_041)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_042) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_043)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_044) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_045)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_038, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_039, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_040, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_041, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_042, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_043, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_044, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_7, onoff_gpio_045, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_7, reg);


/*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_046)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_047)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_048)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_049)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_050) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_051)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_052) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_053)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_046, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_047, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_048, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_049, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_050, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_051, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_052, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_8, onoff_gpio_053, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_8, reg);

 /* */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_054)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_055)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_056)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_057)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_058) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_059)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_060) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_061)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_054, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_055, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_056, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_057, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_058, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_059, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_060, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_9, onoff_gpio_061, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_9, reg);

 /* */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_062)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_063)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_064)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_065)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_066) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_067)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_068) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_069)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_062, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_063, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_064, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_065, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_066, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_067, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_068, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_10, onoff_gpio_069, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_10, reg);

 /* */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_070)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_071)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_072)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_073)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_074) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_075)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_076) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_077)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_070, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_071, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_072, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_073, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_074, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_075, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_076, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_11, onoff_gpio_077, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_11, reg);

 /* */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_078)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_079)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_080)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_081)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_082) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_083)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_084) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_085)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_078, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_079, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_080, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_081, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_082, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_083, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_084, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_12, onoff_gpio_085, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_12, reg);


 /* */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_086)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_087)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_088)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_089)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_090) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_091)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_092) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_093)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_086, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_087, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_088, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_089, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_090, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_091, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_092, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_13, onoff_gpio_093, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_13, reg);

 /* */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_094)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_095)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_096)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_097)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_098)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_099)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_100)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_101)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_094, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_095, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_096, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_097, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_098, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_099, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_100, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_14, onoff_gpio_101, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_14, reg);

/*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_102)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_103)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_104)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_105)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_106) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_107)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_108) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_109)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_102, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_103, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_104, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_105, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_106, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_107, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_108, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_15, onoff_gpio_109, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_15, reg);


/*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_110)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_111)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_112)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_113)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_114) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_115)  |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_116) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_117)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_110, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_111, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_112, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_113, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_114, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_115, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_116, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_16, onoff_gpio_117, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, reg);

/*  */
    reg = BREG_Read32(hReg,BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17);
    reg &= ~(
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_118) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_119) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_120) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_121) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_122) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_123) |
              BCHP_MASK(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_124)
              );


    reg |= (
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_118, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_119, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_120, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_121, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_122, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_123, 0) |
             BCHP_FIELD_DATA(SUN_TOP_CTRL_PIN_MUX_CTRL_17, onoff_gpio_124, 0)
             );
    BREG_Write32 (hReg, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, reg);



    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0);
    reg &=~(
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07 )
           );

    reg |=(
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_00, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_01, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_02, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_03, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_04, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_05, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_06, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_0, aon_gpio_07, 0 )
          );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_0, reg);


    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1);
    reg &=~(
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15 )
           );

    reg |=(
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_08, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_09, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_10, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_11, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_12, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_13, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_14, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_1, aon_gpio_15, 0 )
          );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_1, reg);

    reg = BREG_Read32(hReg,BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2);
    reg &=~(
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_01 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_02 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_03 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_04 ) |
           BCHP_MASK(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_05 )
           );

    reg |=(
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_00, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_01, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_02, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_03, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_04, 0 ) |
          BCHP_FIELD_DATA(AON_PIN_CTRL_PIN_MUX_CTRL_2, aon_sgpio_05, 0 )
          );
    BREG_Write32 (hReg, BCHP_AON_PIN_CTRL_PIN_MUX_CTRL_2, reg);

#endif


    /* Configure the AVD UARTS to debug mode.  SHVD0_OL -> UART1, SHVD_IL -> UART2. */
    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0);
 /*
  * Other possible values keeping it handy here for quick update:
  * AUDIO_FP0,AUDIO_FP1,SHVD0_OL,SHVD0_IL, HVD1_OL, HVD1_IL, HVD2_OL, HVD2_IL, VICE20_ARC0, VICE20_ARC1,, VICE21_ARC0, VICE21_ARC1
  */

    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel) | BCHP_MASK(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel));
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_1_cpu_sel, HVD0_IL);
    reg |= BCHP_FIELD_ENUM(SUN_TOP_CTRL_UART_ROUTER_SEL_0, port_2_cpu_sel, HVD0_OL);


    BREG_Write32(hReg,BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0,reg);

    reg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL);
    reg &= ~(BCHP_MASK(SUN_TOP_CTRL_TEST_PORT_CTRL, encoded_tp_enable));
    reg |= BCHP_FIELD_DATA(SUN_TOP_CTRL_TEST_PORT_CTRL,encoded_tp_enable, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS);
    BREG_Write32(hReg, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, reg);

    return BERR_SUCCESS;
}
