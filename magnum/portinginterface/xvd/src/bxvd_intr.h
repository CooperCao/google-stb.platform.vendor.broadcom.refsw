/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *   See Module Overview below.
 *
  ***************************************************************************/
/*=************************ Module Overview ********************************
<verbatim>

Overview:

This module contains the funtion prototypes for the XVD interrupt service
routines. These are now separate from both bxvd.c and bxvd_priv.c to facilitate
easier modification and maintainence.

</verbatim>
****************************************************************************/
#ifndef BXVD_INTR_H__
#define BXVD_INTR_H__

#include "bxvd.h"

/* This value clear the whole low order word except interrupts 1 thru 4 */
#define BXVD_INTR_INTGEN_CLEAR_VALUE 0x0000ffe1

#ifdef __cplusplus
extern "C" {
#endif

void BXVD_P_AVD_MBX_isr(void *pvXvd,
            int iParam2);

void BXVD_P_AVD_PicDataRdy_isr(void *pvXvd,
                   int iParam2);

void BXVD_P_PictureDataReady_isr(BXVD_Handle hXvd,
                 BXVD_ChannelHandle hXvdCh,
                 BAVC_XVD_Picture *pPicItem);

void BXVD_P_PictureDataRdy_NoDecode_isr(BXVD_Handle hXvd,
                    BXVD_DisplayInterrupt eDisplayInterrupt,
                    BAVC_XVD_Picture *pPicItem);

void BXVD_P_AVD_StillPictureRdy_isr(void *pvXvd,
                    int iParam2);

void BXVD_P_WatchdogInterrupt_isr(void *pvXvd,
                  int param2);

void BXVD_P_VidInstrChkr_isr(void *pvXvd,
                             int param2);

void BXVD_P_StereoSeqError_isr(void *pvXvd,
                               int param2);

void BXVD_P_ClockBoost_isr(
   void *pvXvd,
   int param2
   );

#ifdef __cplusplus
}
#endif

#endif /* BXVD_INTR_H__ */
