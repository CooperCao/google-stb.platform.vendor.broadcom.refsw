/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2003-2016 Broadcom. All rights reserved.
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
 *
 * Module Description:
 *
 ***************************************************************************/
#ifndef BERR_ID_H__
#define BERR_ID_H__

/* The standard ID. This ID is present when the standard error codes are
   being used without a module specific ID */
#define BERR_STD_ID     0x00

/* The application ID. This ID is reserved for use by top level applications
   so they can extend our error codes if they wish. This ID cannot be used
   in any module or library */
#define BERR_APP_ID     0x01

#define BERR_START_ID   0x02

/* porting interfaces */
#define BERR_AUD_ID     (BERR_START_ID + 0)
#define BERR_HDM_ID     (BERR_START_ID + 1)
#define BERR_ICM_ID     (BERR_START_ID + 2)
#define BERR_IFD_ID     (BERR_START_ID + 3)
#define BERR_MVD_ID     (BERR_START_ID + 4)
#define BERR_QDS_ID     (BERR_START_ID + 5)
#define BERR_QOB_ID     (BERR_START_ID + 6)
#define BERR_QUS_ID     (BERR_START_ID + 7)
#define BERR_RDC_ID     (BERR_START_ID + 8)
#define BERR_RFM_ID     (BERR_START_ID + 9)
#define BERR_SCD_ID     (BERR_START_ID + 10)
#define BERR_TNR_ID     (BERR_START_ID + 11)
#define BERR_VBI_ID     (BERR_START_ID + 12)
#define BERR_VDC_ID     (BERR_START_ID + 13)
#define BERR_XPT_ID     (BERR_START_ID + 14)
#define BERR_I2C_ID     (BERR_START_ID + 15)
#define BERR_SPI_ID     (BERR_START_ID + 16)
#define BERR_ICP_ID     (BERR_START_ID + 17)
#define BERR_IRB_ID     (BERR_START_ID + 18)
#define BERR_KIR_ID     (BERR_START_ID + 19)
#define BERR_KPD_ID     (BERR_START_ID + 20)
#define BERR_LED_ID     (BERR_START_ID + 21)
#define BERR_PWM_ID     (BERR_START_ID + 22)
#define BERR_URT_ID     (BERR_START_ID + 23)
#define BERR_SDS_ID     (BERR_START_ID + 24)
#define BERR_VSB_ID     (BERR_START_ID + 25)
#define BERR_ENC_ID     (BERR_START_ID + 26)
#define BERR_DMA_ID     (BERR_START_ID + 27)
#define BERR_GIO_ID     (BERR_START_ID + 28)
#define BERR_LNA_ID     (BERR_START_ID + 29)
#define BERR_GRC_ID     (BERR_START_ID + 30)
#define BERR_P3D_ID     (BERR_START_ID + 31)
#define BERR_XVD_ID     (BERR_START_ID + 32)
#define BERR_ARC_ID     (BERR_START_ID + 33)
#define BERR_RAP_ID     (BERR_START_ID + 34)
#define BERR_MRC_ID     (BERR_START_ID + 35)
#define BERR_AST_ID     (BERR_START_ID + 36)
#define BERR_TMR_ID     (BERR_START_ID + 37)
#define BERR_RPC_ID     (BERR_START_ID + 38)
#define BERR_MEM_ID     (BERR_START_ID + 39)
#define BERR_INT_ID     (BERR_START_ID + 40)
#define BERR_ADS_ID     (BERR_START_ID + 41)
#define BERR_AOB_ID     (BERR_START_ID + 42)
#define BERR_AUS_ID     (BERR_START_ID + 43)
#define BERR_XCD_ID     (BERR_START_ID + 44)
#define BERR_THD_ID     (BERR_START_ID + 45)
#define BERR_XCU_ID     (BERR_START_ID + 46)
#define BERR_NET_ID     (BERR_START_ID + 47)
#define BERR_USB_ID     (BERR_START_ID + 48)
#define BERR_ATA_ID     (BERR_START_ID + 49)
#define BERR_HDR_ID     (BERR_START_ID + 50)
#define BERR_UHF_ID     (BERR_START_ID + 51)
#define BERR_LVD_ID     (BERR_START_ID + 52)
#define BERR_UPG_ID     (BERR_START_ID + 53)
#define BERR_MDM_ID     (BERR_START_ID + 54)
#define BERR_VIF_ID     (BERR_START_ID + 55)
#define BERR_AST1_ID    (BERR_START_ID + 56)
#define BERR_QPSK_ID    (BERR_START_ID + 57)
#define BERR_FTM_ID     (BERR_START_ID + 58)
#define BERR_RLE_ID     (BERR_START_ID + 59)
#define BERR_CTK_ID     (BERR_START_ID + 60)
#define BERR_TMN_ID     (BERR_START_ID + 61)
#define BERR_ELF_ID     (BERR_START_ID + 62)
#define BERR_APE_ID     (BERR_START_ID + 63)
#define BERR_TNF_ID     (BERR_START_ID + 64)
#define BERR_TFE_ID     (BERR_START_ID + 65)
#define BERR_ANV_ID     (BERR_START_ID + 66)
#define BERR_UDP_ID     (BERR_START_ID + 67)
#define BERR_ANA_ID     (BERR_START_ID + 68)
#define BERR_SID_ID     (BERR_START_ID + 69)
#define BERR_V3D_ID     (BERR_START_ID + 70)
#define BERR_DSP_ID     (BERR_START_ID + 71)
#define BERR_TC2_ID     (BERR_START_ID + 72)
#define BERR_MMD_ID     (BERR_START_ID + 73)
#define BERR_WFE_ID     (BERR_START_ID + 74)
#define BERR_SCS_ID     (BERR_START_ID + 75)
#define BERR_CEC_ID		(BERR_START_ID + 76)
#define BERR_VCE_ID     (BERR_START_ID + 77)
#define BERR_BCP_ID     (BERR_START_ID + 78)
#define BERR_ODS_ID     (BERR_START_ID + 79)
#define BERR_HAB_ID     (BERR_START_ID + 80)
#define BERR_SAT_ID     (BERR_START_ID + 81)
#define BERR_DSQ_ID     (BERR_START_ID + 82)
#define BERR_FSK_ID     (BERR_START_ID + 83)
#define BERR_BOX_ID     (BERR_START_ID + 84)
#define BERR_MMA_ID     (BERR_START_ID + 85)
#define BERR_DBG_ID     (BERR_START_ID + 86)


/* syslibs */
#define BERR_START_LIB_ID     0x100

#define BERR_TNRlib_ID        (BERR_START_LIB_ID + 0)
#define BERR_VBIlib_ID        (BERR_START_LIB_ID + 1)
#define BERR_BTSlib_ID        (BERR_START_LIB_ID + 2)
#define BERR_MUXlib_ID        (BERR_START_LIB_ID + 3)
#define BERR_XUDlib_ID        (BERR_START_LIB_ID + 4)
#define BERR_SAGElib_ID       (BERR_START_LIB_ID + 5)

/* common utils */
#define BERR_START_COMMON_UTILS_ID     0x110

#define BERR_MRC_MONITOR_ID     (BERR_START_COMMON_UTILS_ID + 1)

/* 0x180 .. 0x200 used by nexus */

#endif /* #ifndef BERR_ID_H__ */

/* end of file */
