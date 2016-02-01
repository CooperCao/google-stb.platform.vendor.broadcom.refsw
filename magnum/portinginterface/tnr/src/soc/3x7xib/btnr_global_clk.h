/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 * USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 * ANY LIMITED REMEDY.
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
 *****************************************************************************/

#ifndef _BTNR_GLOBAL_CLK_H__
#define _BTNR_GLOBAL_CLK_H__

#ifdef __cplusplus
extern "C" {
#endif

/*The following offsets need to be set board to board, this set was done with the BCM93461DC1_01*/
/*Gain offsets for board to get channel power, values are offsets in db*256*/
#define GAIN_OFFSET  20*256  /*Calibrate at 500 MHz and 0 dbmv*/


#define REF_FREQ 54000000

/*Choose 1 REFPLL Rate*/
/*0 for 1080000000, 1 for 1350000000*/
#define REFPLL_FREQ_SEL 0

/*Assumes REF_FREQ = 54000000 and that
 *BTNR_P_TunerInit() makes the #define true*/
#if (REFPLL_FREQ_SEL == 0)
#define REFPLL_FREQ    1080000000UL
#endif
#if (REFPLL_FREQ_SEL == 1)
#define REFPLL_FREQ    1350000000UL
#endif

/*It is assumed that the BTNR_P_TunerInit()
 * and BTNR_P_Program_CIC_HB_SAW()
 *makes the following #defines true       */
#define TERR_PHYPLL1_FREQ  2700000000UL    
#define TERR_PHYPLL2_FREQ  1350000000UL            
#define TERR_PHYPLL3_FREQ   225000000UL              
#define TERR_PHYPLL4_FREQ   100000000UL              
#define TERR_PHYPLL5_FREQ    10546875UL  					 
#define TERR_PHYPLL6_FREQ   540000000UL
#define F_TER                54000000UL  
#define CABLE_PHYPLL1_FREQ 2632500000UL 
#define CABLE_PHYPLL2_FREQ 1316250000UL            
#define CABLE_PHYPLL3_FREQ  219375000UL              
#define CABLE_PHYPLL4_FREQ   97500000UL              
#define CABLE_PHYPLL5_FREQ   10283203UL                                          
#define CABLE_PHYPLL6_FREQ  540000000UL /* 526500000UL */
#define F_HS		             15000000UL /* 14625000UL */
#define F_1S                 30000000UL /* 29250000UL */


#define REF_FREQ_D128 421875
#define DPM_FREQ_MAX 1012600000
#define DPM_FREQ_MIN 31640625

#ifdef LEAP_BASED_CODE
#define SPUR_TBL_SIZE 11 
static uint32_t SPUR_TBL_u32[SPUR_TBL_SIZE] =
{
 0, 54000000, 675000000,  99900000, 432000000,
810000000, 864000000, 486000000, 594000000,
648000000, 540000000
};
#endif
#ifdef __cplusplus
}
#endif

#endif

 
