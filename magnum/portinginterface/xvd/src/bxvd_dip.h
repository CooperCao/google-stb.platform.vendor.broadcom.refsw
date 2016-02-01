/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
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
                                                                          BXDM_DisplayInterruptHandler_isr fCallback,
                                                                          void *pPrivateContext );

BERR_Code BXVD_DisplayInterruptProvider_GetDisplayInterruptInfo_isr( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh,
                                                                     BXDM_DisplayInterruptInfo *pstXvdDisplayInterruptInfo );

BERR_Code BXVD_DisplayInterruptProvider_P_EnableInterrupts( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh );

BERR_Code BXVD_DisplayInterruptProvider_P_DisableInterrupts( BXVD_DisplayInterruptProvider_P_ChannelHandle hXvdDipCh );

#ifdef __cplusplus
}
#endif

#endif /* BXVD_DIP_H_ */
