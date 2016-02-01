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
 * $brcm_Revision:
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BTNR_DEF_H__
#define BTNR_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BTNR_P_BCHP_CORE_V(MAJOR,MINOR) (((MAJOR)<<8)+MINOR)

#define BTNR_P_BCHP_CORE_V_1_0 (BTNR_P_BCHP_CORE_V(1,0))
#define BTNR_P_BCHP_CORE_V_1_1 (BTNR_P_BCHP_CORE_V(1,1))
#define BTNR_P_BCHP_CORE_V_1_2 (BTNR_P_BCHP_CORE_V(1,2))
#define BTNR_P_BCHP_CORE_V_1_3 (BTNR_P_BCHP_CORE_V(1,3))
#define BTNR_P_BCHP_CORE_V_1_4 (BTNR_P_BCHP_CORE_V(1,4))


#define EXT_LNA_ENABLE 0

/***************************************************************************
 *  BTNR CORE Defines
 ****************************************************************************/
#if (BCHP_VER == BCHP_VER_A0)
        #if ((BCHP_CHIP==7552) || (BCHP_CHIP==35233) || (BCHP_CHIP==3461))
          #define BTNR_P_BCHP_TNR_CORE_VER     BTNR_P_BCHP_CORE_V_1_0
      #define BTNR_P_BCHP_TNR_NUM_CORES   (1)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_OFFSET (0x0)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_TM_SYS_PLL_OFFSET (0x0)
        #elif (BCHP_CHIP==3462)
          #define BTNR_P_BCHP_TNR_CORE_VER     BTNR_P_BCHP_CORE_V_1_2
      #define BTNR_P_BCHP_TNR_NUM_CORES   (1)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_OFFSET (0x0)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_TM_SYS_PLL_OFFSET (0x0)
        #elif (BCHP_CHIP==3472)
          #define BTNR_P_BCHP_TNR_CORE_VER     BTNR_P_BCHP_CORE_V_1_3
      #define BTNR_P_BCHP_TNR_NUM_CORES   (2)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_OFFSET (0x20000)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_TM_SYS_PLL_OFFSET (0x4)
        #elif (BCHP_CHIP==7563) || (BCHP_CHIP==75635)
          #define BTNR_P_BCHP_TNR_CORE_VER     BTNR_P_BCHP_CORE_V_1_4
      #define BTNR_P_BCHP_TNR_NUM_CORES   (1)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_OFFSET (0x0)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_TM_SYS_PLL_OFFSET (0x0)
        #endif
#elif (BCHP_VER == BCHP_VER_B0)
        #if ((BCHP_CHIP==7552) || (BCHP_CHIP==3461))
          #define BTNR_P_BCHP_TNR_CORE_VER     BTNR_P_BCHP_CORE_V_1_1
      #define BTNR_P_BCHP_TNR_NUM_CORES   (1)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_OFFSET (0x0)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_TM_SYS_PLL_OFFSET (0x0)
        #elif (BCHP_CHIP==3472)
          #define BTNR_P_BCHP_TNR_CORE_VER     BTNR_P_BCHP_CORE_V_1_3
      #define BTNR_P_BCHP_TNR_NUM_CORES   (2)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_OFFSET (0x20000)
      #define BTNR_P_BCHP_TNR_MULTI_CORE_TM_SYS_PLL_OFFSET (0x4)
        #endif
#endif


#ifdef SMART_TUNE_ENABLED
        #define SmartTuneEnabled
#endif


#ifndef BTNR_P_BCHP_TNR_CORE_VER
  #error TNR core NOT DEFINED in TNR PI
#endif

#if (BCHP_CHIP==7552)   /* Requires for any PI with single Channel support */
#define BTNR_3x7x_ChnHandle BTNR_3x7x_Handle
#define BTNR_P_3x7x_ChnHandle BTNR_P_3x7x_Handle
#endif

#ifdef __cplusplus
}
#endif

#endif
