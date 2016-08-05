/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its licensors,
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
******************************************************************************/

#ifndef BSID_PLATFORM_H__
#define BSID_PLATFORM_H__

#include "bchp_sun_top_ctrl.h"

/* NOTE: Verify chip versions here - once checked here
         we no longer need to use chip ver again unless
         two different versions have different requirements
         (currently none do) */

/* NOTE: all variants of the 7344 (including 7418 and 7354) are
   supported via the 7344 BCHP_CHIP setting.
   Similarly, the 7422 is supported via the 7425 setting
   (along with all other 7425 variants) */
#if  (((BCHP_CHIP==7344)  && (BCHP_VER >= BCHP_VER_B0)) || \
      ((BCHP_CHIP==7346)  && (BCHP_VER >= BCHP_VER_B0)) || \
      ((BCHP_CHIP==73465) && (BCHP_VER >= BCHP_VER_A0)) || \
      ((BCHP_CHIP==7425)  && (BCHP_VER >= BCHP_VER_B2)))
      /* GISB/RBUS Bridge used to reset the SID Arc
         (these chips do not have a SID reset control in Sundry)
         NOTE: in theory ALL chips can reset SID via GR Bridge
         (GR bridge is part of the SID block, so exists for all
         parts, although not necessarily in the graphics core) */
#include "bchp_gfx_gr.h"
#elif (((BCHP_CHIP==7231)  && (BCHP_VER >= BCHP_VER_B0)) || \
       ((BCHP_CHIP==7429)  && (BCHP_VER >= BCHP_VER_B0)) || \
       ((BCHP_CHIP==74295) && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==7435)  && (BCHP_VER >= BCHP_VER_B0)) || \
       ((BCHP_CHIP==7445)  && (BCHP_VER >= BCHP_VER_D0)) || \
       ((BCHP_CHIP==7145)  && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==7366)  && (BCHP_VER >= BCHP_VER_B0)) || \
       ((BCHP_CHIP==7364)  && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==7250)  && (BCHP_VER >= BCHP_VER_B0)) || \
       ((BCHP_CHIP==7439)  && (BCHP_VER >= BCHP_VER_B0)) || \
       ((BCHP_CHIP==7584)  && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==75845) && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==7586)  && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==7260)  && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==7268)  && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==7271)  && (BCHP_VER >= BCHP_VER_A0)) || \
       ((BCHP_CHIP==74371) && (BCHP_VER >= BCHP_VER_A0)))
      /* sundry (sun_top_ctrl) is used to reset the SID arc */
#else
#error "Unsupported Chip"
#endif

/* Include definition for BCHP_INT_ID_SID_INTR and BCHP_INT_ID_SID_WATCHDOG_INTR,
   or the definitions necessary to create it */
#if ((BCHP_CHIP==7435) || \
     (BCHP_CHIP==7445) || \
     (BCHP_CHIP==7145) || \
     (BCHP_CHIP==7366) || \
     (BCHP_CHIP==7364) || \
     (BCHP_CHIP==7250) || \
     (BCHP_CHIP==7439) || \
     (BCHP_CHIP==7260) || \
     (BCHP_CHIP==7268) || \
     (BCHP_CHIP==7271) || \
     (BCHP_CHIP==74371))
#include "bchp_sid_l2.h"
#elif ((BCHP_CHIP==7584)  || \
       (BCHP_CHIP==75845) || \
       (BCHP_CHIP==7586))
#include "bchp_int_id_sid_l2.h"
#else /* 7231, 7344, (7418, 7354), 7346, 7425 (7422), 7429 (74295) */
#include "bchp_int_id_gfx_l2.h"
#endif

#include "bsid_dbg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************
/////////////////////// Defines, Typedef, Structs ////////////////////////////////
*********************************************************************************/

/* sid arc cpu frequency for use by FW in setting up the UART */
#if   ((BCHP_CHIP==7231)  || \
       (BCHP_CHIP==7344)  || \
       (BCHP_CHIP==7346)  || \
       (BCHP_CHIP==73465) || \
       (BCHP_CHIP==7425)  || \
       (BCHP_CHIP==7429)  || \
       (BCHP_CHIP==74295) || \
       (BCHP_CHIP==7435)  || \
       (BCHP_CHIP==7364)  || \
       (BCHP_CHIP==7250)  || \
       (BCHP_CHIP==7584)  || \
       (BCHP_CHIP==75845) || \
       (BCHP_CHIP==7586))
#define BSID_P_ARC_CPU_FREQUENCY                                       (216*1000000)
#elif ((BCHP_CHIP==7445)  || \
       (BCHP_CHIP==7145)  || \
       (BCHP_CHIP==7366)  || \
       (BCHP_CHIP==7439)  || \
       (BCHP_CHIP==74371) || \
       (BCHP_CHIP==7260)  || \
       (BCHP_CHIP==7268)  || \
       (BCHP_CHIP==7271))
#define BSID_P_ARC_CPU_FREQUENCY                                       (324*1000000)
#endif

/* FIXME: 7440 is obsolete - in theory we can remove the
   support for reverse pixel format */
#if ((BCHP_CHIP==7440) && (BCHP_VER == BCHP_VER_A0))
/* 7440 A0 has reversed pixel format
   (e.g. ACrCbY instead of AYCbCr, or ABGR instead of ARGB) */
#define BSID_P_REVERSE_PXL_FMT
#endif

/* These chips do not define BCHP_INT_ID_SID_INTR, so we
   explicitly define them here */
#if ((BCHP_CHIP==7435) || \
     (BCHP_CHIP==7445) || \
     (BCHP_CHIP==7145) || \
     (BCHP_CHIP==7366) || \
     (BCHP_CHIP==7364) || \
     (BCHP_CHIP==7250) || \
     (BCHP_CHIP==7439) || \
     (BCHP_CHIP==7260) || \
     (BCHP_CHIP==7268) || \
     (BCHP_CHIP==7271) || \
     (BCHP_CHIP==74371))
#define BCHP_INT_ID_SID_INTR           BCHP_INT_ID_CREATE(BCHP_SID_L2_CPU_STATUS, BCHP_SID_L2_CPU_STATUS_SID_INTR_SHIFT)
#define BCHP_INT_ID_SID_WATCHDOG_INTR  BCHP_INT_ID_CREATE(BCHP_SID_L2_CPU_STATUS, BCHP_SID_L2_CPU_STATUS_SID_WATCHDOG_INTR_SHIFT)
#endif

#ifdef BSID_P_DEBUG_ENABLE_ARC_UART
#define BSID_P_EnableUart(x)  BSID_P_PlatformEnableUart(x)
#else
#define BSID_P_EnableUart(x)
#endif

/* ARC status register value that sets the "halt" flag (bit 25) */
#define BSID_FW_ARC_HALT                                      (uint32_t)0x02000000
#define BSID_FW_ARC_RESET_ADDRESS                             (uint32_t)0

void BSID_P_ArcSoftwareReset(BREG_Handle hReg);
void BSID_P_HaltArc(BREG_Handle hReg);
void BSID_P_ReadSIDStatus_isr(BREG_Handle hReg, uint32_t *puiSidStatus, uint32_t *puiArcPC);
void BSID_P_ChipEnable(BREG_Handle hReg, uint32_t uiBasePhysAddress);

#ifdef BSID_P_DEBUG_ENABLE_ARC_UART
void BSID_P_PlatformEnableUart(BREG_Handle hReg);
#endif

/*********************************************************************************
//////////////////////////////////////////////////////////////////////////////////
*********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSID_PICTURE_H__ */

/* end of file */
