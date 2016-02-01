/***************************************************************************
 *         (c)2007-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").   Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.      This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *      2.         TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.      TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
 ***************************************************************************/

#ifndef BCEC_CONFIG_H__
#define BCEC_CONFIG_H__

#include "bchp_common.h"

#include "bchp.h"
#include "bcec.h"


#ifdef __cplusplus
extern "C" {
#endif


/* The following configuration SHOULD NOT be enable in standard releases */
#define BCEC_CONFIG_DEBUG_CEC 0
#define BCEC_CONFIG_DEBUG_INTERRUPTS 0
#define BCEC_CONFIG_DEBUG_CEC_TIMING 0
#define BCEC_CONFIG_DEBUG_MESSAGE_TX 0
#define BCEC_CONFIG_DEBUG_MESSAGE_RX 0
#define BCEC_CONFIG_DEBUG_OPCODE 0

#define BCEC_CONFIG_P_MAX_MESSAGE_BUFFER 16

/***************************************
        DO NOT MODIFY THE BELOW CODE
***************************************/
#if ((BCHP_CHIP == 7400) && (BCHP_VER >= BCHP_VER_B0)) || (BCHP_CHIP == 7405) || (BCHP_CHIP == 7325) \
                || (BCHP_CHIP == 7335) || (BCHP_CHIP == 7336) || (BCHP_CHIP == 7340) || (BCHP_CHIP == 7342) \
                || (BCHP_CHIP == 7420) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7550) || (BCHP_CHIP == 7208) \
                || (BCHP_CHIP == 7408) || (BCHP_CHIP == 7468) || (BCHP_CHIP == 7601) || (BCHP_CHIP == 7630) \
                || (BCHP_CHIP == 7635)
#define BCEC_CONFIG_65NM_SUPPORT 1


/*****************************
 * 40nm platforms:
 * + Rev. 1 corresponds to Ax
 * + Rev. 2 corresponds to
                * 40nm B0 and later (except 7552)
                * 7429 and later platforms
 ******************************/
#elif (BCHP_CHIP == 7422)  || (BCHP_CHIP == 7425)  || (BCHP_CHIP == 7231)  \
   || (BCHP_CHIP == 7358)  || (BCHP_CHIP == 7344)  || (BCHP_CHIP == 7346)  \
   || (BCHP_CHIP == 7640)  || (BCHP_CHIP == 7552)  || (BCHP_CHIP == 7429)  \
   || (BCHP_CHIP == 7435)  || (BCHP_CHIP == 7360)  || (BCHP_CHIP == 7584)  \
   || (BCHP_CHIP == 7563)  || (BCHP_CHIP == 7543)  || (BCHP_CHIP == 7362)  \
   || (BCHP_CHIP == 7228)  || (BCHP_CHIP == 75635) || (BCHP_CHIP == 73625) \
   || (BCHP_CHIP == 75845) || (BCHP_CHIP == 74295) || (BCHP_CHIP == 75525) \
   || (BCHP_CHIP == 73465)
#define BCEC_CONFIG_40NM_SUPPORT 1

#else

/*****************************
 * all 28nm and beyond platforms
 ******************************/
#define BCEC_CONFIG_28NM_SUPPORT 1
#define BCEC_CONFIG_ENABLE_COMPLIANCE_TEST_WORKAROUND 1
#endif


/**********************************
        40nm Rev1 platforms have only 1 CEC
        interrupt       for both send and receive
***********************************/
#if ((BCHP_VER < BCHP_VER_B0) \
        && ((BCHP_CHIP == 7422) || (BCHP_CHIP == 7425) || (BCHP_CHIP == 7231) \
        || (BCHP_CHIP == 7358) || (BCHP_CHIP == 7344) || (BCHP_CHIP == 7346) \
        || (BCHP_CHIP == 7640))) \
        || (BCHP_CHIP == 7552) || (BCHP_CHIP == 7543)
#define BCEC_CONFIG_SINGLE_INTERRUPT 1
#endif


/**********************************
        40nm Rev2 platforms have 2 separate
        CEC interrupts: send and receive
***********************************/
#if (((BCHP_VER >= BCHP_VER_B0)                                                 \
        && ((BCHP_CHIP == 7422) || (BCHP_CHIP == 7425)  || (BCHP_CHIP == 7231)  \
        || (BCHP_CHIP == 7358)  || (BCHP_CHIP == 7344)  || (BCHP_CHIP == 7346)  \
        || (BCHP_CHIP == 7640)  || (BCHP_CHIP == 7552)))                        \
        || (BCHP_CHIP == 7429)  || (BCHP_CHIP == 7435)  || (BCHP_CHIP == 7360)  \
        || (BCHP_CHIP == 7584)  || (BCHP_CHIP == 7563)  || (BCHP_CHIP == 7543)  \
        || (BCHP_CHIP == 7362)  || (BCHP_CHIP == 7228)  || (BCHP_CHIP == 75635) \
        || (BCHP_CHIP == 73625) || (BCHP_CHIP == 75845) || (BCHP_CHIP == 74295) \
        || (BCHP_CHIP == 75525) || (BCHP_CHIP == 73465)\
        || BCEC_CONFIG_28NM_SUPPORT)
#define BCEC_CONFIG_DUAL_INTERRUPT 1
#endif


/***********************************
        Platforms support AUTO_ON Features
***********************************/
#if BCEC_CONFIG_40NM_SUPPORT || BCEC_CONFIG_28NM_SUPPORT
#define BCEC_CONFIG_AUTO_ON_SUPPORT 1

#include "bchp_aon_hdmi_tx.h"
#ifdef BCHP_AON_HDMI_TX_AUTO_CEC_GIVE_VERSION_CFG
#define BCEC_CONFIG_AUTO_ON_CEC_20_SUPPORT 1
#endif

#endif


/****************************************
        Platforms with both HDMI_TX and HDMI_Rx
****************************************/
#if BCHP_AON_HDMI_RX_REG_START
#define BCEC_CONFIG_HAS_HDMI_RX 1
#endif


/***************************************
        Older platforms use CEC PAD_SW_RESET register
        7601/7420 and newer platforms no longer use this.
*****************************************/
#if (BCHP_CHIP==7038) || (BCHP_CHIP==7438) || (BCHP_CHIP==7440) || \
        (BCHP_CHIP==7400) || (BCHP_CHIP==7401) || (BCHP_CHIP==7405) || \
        (BCHP_CHIP==7335) || (BCHP_CHIP==7336) || (BCHP_CHIP==7325)
#define BCEC_CONFIG_CEC_USE_PAD_SW_RESET 1
#endif


/***************************************
        Uninitialized CEC logical address
**************************************/
#define BCEC_CONFIG_UNINITIALIZED_LOGICAL_ADDR 0xFF


/* HDMI Tx DVD/Bluray */
#if (BCHP_CHIP==7438) || (BCHP_CHIP==7440) || (BCHP_CHIP==7601) || \
        (BCHP_CHIP==7635) || (BCHP_CHIP==7630) || (BCHP_CHIP==7640)
#define BCEC_CONFIG_DEVICE_TYPE 0x04
#else
#define BCEC_CONFIG_DEVICE_TYPE 0x03
#endif


#ifdef __cplusplus
}
#endif

#endif /* BCEC_CONFIG_H__ */
/* End of File */

