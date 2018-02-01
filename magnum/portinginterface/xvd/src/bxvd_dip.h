/***************************************************************************
 * Copyright (C) 2003-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#ifndef BXVD_DIP_H_
#define BXVD_DIP_H_

#include "bxdm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BXVD_DisplayInterruptProvider_P_ChannelContext *BXVD_DisplayInterruptProvider_P_ChannelHandle;

typedef struct BXVD_DisplayInterruptProvider_P_ChannelSettings
{
      BXVD_Handle hXvd;
      BINT_Handle hInterrupt;
      BREG_Handle hRegister;
      BINT_Id interruptId;
      uint32_t uiInterruptClearRegister;
      uint32_t uiInterruptMaskRegister;
      uint32_t uiInterruptClearRegister_1;
      uint32_t uiInterruptMaskRegister_1;
      BXVD_DisplayInterrupt eDisplayInterrupt;
#if BXVD_P_FW_HIM_API
      uint32_t  uiDisplayInfoOffset;
#else
      BXVD_P_DisplayInfo *pstDisplayInfo;
#endif
} BXVD_DisplayInterruptProvider_P_ChannelSettings;

typedef struct BXVD_DisplayInterruptProvider_P_RULIDMasks
{
  uint32_t ui32TopFieldRULIDMask;
  uint32_t ui32BottomFieldRULIDMask;
  uint32_t ui32ProgressiveFieldRULIDMask;
} BXVD_DisplayInterruptProvider_P_RULIDMasks;

typedef struct BXVD_DisplayInterruptProvider_P_InterruptSettings
{
      BXVD_DisplayInterruptProvider_P_RULIDMasks stRULIDMasks_0;
      BXVD_DisplayInterruptProvider_P_RULIDMasks stRULIDMasks_1;
} BXVD_DisplayInterruptProvider_P_InterruptSettings;

BERR_Code BXVD_DisplayInterruptProvider_P_GetDefaultChannelSettings( BXVD_DisplayInterruptProvider_P_ChannelSettings *pstXvdDipChSettings );

BERR_Code BXVD_DisplayInterruptProvider_P_OpenChannel( BXVD_DisplayInterruptProvider_P_ChannelHandle *phXvdDipCh,
                                                       const BXVD_DisplayInterruptProvider_P_ChannelSettings *pstXvdDipChSettings );

BERR_Code BXVD_DisplayInterruptProvider_P_CloseChannel( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh );

BERR_Code BXVD_DisplayInterruptProvider_P_ProcessWatchdog( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code BXVD_DisplayInterruptProvider_P_GetDefaultInterruptSettings( BXVD_DisplayInterruptProvider_P_InterruptSettings *pstXvdDipIntSettings );
#endif

BERR_Code BXVD_DisplayInterruptProvider_P_SetInterruptConfiguration( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
                                                                     const BXVD_DisplayInterruptProvider_P_InterruptSettings *pstXvdDipIntSettings);

BERR_Code BXVD_DisplayInterruptProvider_P_GetInterruptConfiguration( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
                                                                     BXVD_DisplayInterruptProvider_P_InterruptSettings *pstXvdDipIntSettings);

BERR_Code BXVD_DisplayInterruptProvider_InstallCallback_DisplayInterrupt( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
                                                                          BXDM_DisplayInterruptHandler_isr fCallback_isr,
                                                                          void *pPrivateContext );

BERR_Code BXVD_DisplayInterruptProvider_GetDisplayInterruptInfo_isr( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
                                                                     BXDM_DisplayInterruptInfo *pstXvdDisplayInterruptInfo );

BERR_Code BXVD_DisplayInterruptProvider_P_EnableInterrupts( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh );

BERR_Code BXVD_DisplayInterruptProvider_P_DisableInterrupts( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh );

#ifdef __cplusplus
}
#endif

#endif /* BXVD_DIP_H_ */
