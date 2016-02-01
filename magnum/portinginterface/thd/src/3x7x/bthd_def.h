/***************************************************************************
 *     (c)2005-2013 Broadcom Corporation
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
#ifndef BTHD_DEF_H__
#define BTHD_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BTHD_P_BCHP_CORE_V(MAJOR,MINOR) (((MAJOR)<<8)+MINOR)

#define BTHD_P_BCHP_CORE_V_4_0 (BTHD_P_BCHP_CORE_V(4,0))
#define BTHD_P_BCHP_CORE_V_5_0 (BTHD_P_BCHP_CORE_V(5,0))
#define BTHD_P_BCHP_CORE_V_5_1 (BTHD_P_BCHP_CORE_V(5,1))
#define BTHD_P_BCHP_CORE_V_5_21 (BTHD_P_BCHP_CORE_V(5,0x21))
#define BTHD_P_BCHP_CORE_V_5_22 (BTHD_P_BCHP_CORE_V(5,0x22))

/***************************************************************************
 *  BTHD CORE Defines
 ****************************************************************************/
#if (BCHP_VER == BCHP_VER_A0)
	#if ((BCHP_CHIP==7552) || (BCHP_CHIP==35233) || (BCHP_CHIP==3461))
	  #define BTHD_P_BCHP_THD_CORE_VER     BTHD_P_BCHP_CORE_V_4_0
      #define BTHD_P_BCHP_THD_NUM_CORES   (1)
      #define BTHD_P_BCHP_THD_MULTI_CORE_OFFSET (0x0)
      #define BTHD_P_BCHP_THD_MULTI_CORE_TM_SYS_PLL_OFFSET (0x0)
	#elif (BCHP_CHIP==3462)
	  #define BTHD_P_BCHP_THD_CORE_VER     BTHD_P_BCHP_CORE_V_5_1
      #define BTHD_P_BCHP_THD_NUM_CORES   (1)     
      #define BTHD_P_BCHP_THD_MULTI_CORE_OFFSET (0x0)
      #define BTHD_P_BCHP_THD_MULTI_CORE_TM_SYS_PLL_OFFSET (0x0)
	#elif (BCHP_FAMILY == 3472)
      #define BTHD_P_BCHP_THD_CORE_VER   BTHD_P_BCHP_CORE_V_5_21
      #define BTHD_P_BCHP_THD_NUM_CORES   (2)     
      #define BTHD_P_BCHP_THD_MULTI_CORE_OFFSET (0x20000)
      #define BTHD_P_BCHP_THD_MULTI_CORE_TM_SYS_PLL_OFFSET (0x4)
	#endif
#elif (BCHP_VER == BCHP_VER_B0)
	#if ((BCHP_CHIP==7552) || (BCHP_CHIP==3461))
	  #define BTHD_P_BCHP_THD_CORE_VER     BTHD_P_BCHP_CORE_V_5_1
      #define BTHD_P_BCHP_THD_NUM_CORES   (1)     
      #define BTHD_P_BCHP_THD_MULTI_CORE_OFFSET (0x0)
      #define BTHD_P_BCHP_THD_MULTI_CORE_TM_SYS_PLL_OFFSET (0x0)
	#elif (BCHP_FAMILY == 3472)
      #define BTHD_P_BCHP_THD_CORE_VER   BTHD_P_BCHP_CORE_V_5_22
      #define BTHD_P_BCHP_THD_NUM_CORES   (2)
      #define BTHD_P_BCHP_THD_MULTI_CORE_OFFSET (0x20000)
      #define BTHD_P_BCHP_THD_MULTI_CORE_TM_SYS_PLL_OFFSET (0x4)
	#endif
#endif

#if ((BCHP_CHIP==7552) || (BCHP_CHIP==3461) || (BCHP_CHIP==3462))
	#define SmartNotchEnabled
#endif

/* Added for DirecTV spur test */
#if ((BCHP_CHIP==7552) || (BCHP_CHIP==3472))
	#define SpurNotchEnabled
#endif

#ifdef SMART_TUNE_ENABLED
	#define SmartTuneEnabled
#endif


#ifndef BTHD_P_BCHP_THD_CORE_VER
  #error THD core NOT DEFINED in THD PI
#endif

#if (BCHP_CHIP==7552)   /* Requires for any PI with single Channel support */
#define BTHD_3x7x_P_ChnHandle BTHD_3x7x_P_Handle
#define BTHD_3x7x_ChnHandle BTHD_3x7x_Handle
#endif


#ifdef __cplusplus
}
#endif

#endif