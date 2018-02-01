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

#ifndef BXDM_DIH_H_
#define BXDM_DIH_H_

#include "bxdm.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

typedef struct BXDM_DisplayInterruptHandler_P_Context *BXDM_DisplayInterruptHandler_Handle;

typedef struct BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings
{
   uint32_t uiVDCRectangleNumber;
}  BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings;

/* BXDM_DisplayInterruptHandler_Create creates an instance
 * of a Display Interrupt Handle (DIH). Typically, there is
 * one DIH for each Display Interrupt Provider (DIP) module
 */
BERR_Code
BXDM_DisplayInterruptHandler_Create(
         BXDM_DisplayInterruptHandler_Handle *phXdmDih
         );

BERR_Code
BXDM_DisplayInterruptHandler_Destroy(
         BXDM_DisplayInterruptHandler_Handle hXdmDih
         );

BERR_Code
BXDM_DIH_AddPictureProviderInterface_GetDefaultSettings (
         BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings *pstDefSettings
         );

/* BXDM_DisplayInterruptHandler_AddPictureProviderInterface is used by the application
 * to register a Picture Provider (PP) with the Display Interrupt Handler (DIH). The DIH
 * will call into the PP on each Picture Data Ready ISR and provide it the
 * Display Interrupt Info.   The PP will provide a linked list of BAVC_MFD_Picture structs
 */
BERR_Code
BXDM_DisplayInterruptHandler_AddPictureProviderInterface(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback_isr,
         void * pPrivateContext,
         BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings *patDefSettings
         );

BERR_Code
BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback_isr,
         void *pPrivateContext
         );

/* BXDM_DisplayInterruptHandler_Callback_isr is passed to the
 * Display Interrupt Provider (DIP).  In this callback, the DIH
 * will cycle through each Picture Provider (PP) that is registered
 * and create a super-linked list of BAVC_MFD_Picture structs that
 * is then provided to the installed PictureDataReady callback handler
 */
BERR_Code
BXDM_DisplayInterruptHandler_Callback_isr(
         void *pPrivateContext,
         BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo );

/* BXDM_DisplayInterruptHandler_PictureDataReady_isr is the typedef for
 * the function that needs to receive the the BAVC_MFD_Picture linked list.
 * This typedef matches the prototype for BVDC_Source_MpegDataReady()
 */
typedef BERR_Code (*BXDM_DisplayInterruptHandler_PictureDataReady_isr)(
         void *pPrivateContext,
         int32_t iPrivateParam,
         BAVC_MFD_Picture *pMFDPicture );

/* BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt is used
 * by the app to register it's MAVC_MFD_Picture handler.
 * Typically, this will be BVDC_Source_MpegDataReady()
 */
BERR_Code
BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_PictureDataReady_isr fCallback_isr,
         void *pPrivateContext,
         int32_t iPrivateParam
         );

BERR_Code
BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(
         BXDM_DisplayInterruptHandler_Handle hXdmDih
         );

/* BXDM_DisplayInterruptHandler_Display_isr is the typedef for
 * the function that needs to receive a callback for each display interrupt.
 */
typedef void (*BXDM_DisplayInterruptHandler_Display_isr)(
   void *pPrivateContext,
   int32_t iPrivateParam,
   void *Unused
   );

/* BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt is used
 * by the app to register a callback for the each display interupt.
 */
BERR_Code
BXDM_DisplayInterruptHandler_InstallCallback_DisplayInterrupt(
   BXDM_DisplayInterruptHandler_Handle hXdmDih,
   BXDM_DisplayInterruptHandler_Display_isr fCallback,
   void *pPrivateContext,
   int32_t iPrivateParam
   );

BERR_Code
BXDM_DisplayInterruptHandler_UnInstallCallback_DisplayInterrupt(
   BXDM_DisplayInterruptHandler_Handle hXdmDih
   );

/* SW7405-3984: the Set/Get mode API's provide a way to disable
 * a PictureProvide (XDM) without unregistering it from the DIP.
 * When a PictureProvider is "disabled", the associated callback
 * will not be executed by in BXDM_DisplayInterruptHandler_Callback_isr.
 */
typedef enum BXDM_DisplayInterruptHandler_PictureProviderMode
{
      BXDM_DisplayInterruptHandler_PictureProviderMode_eDisabled=0,
      BXDM_DisplayInterruptHandler_PictureProviderMode_eEnabled,

      BXDM_DisplayInterruptHandler_PictureProviderMode_eMax
} BXDM_DisplayInterruptHandler_PictureProviderMode;


BERR_Code
BXDM_DisplayInterruptHandler_SetPictureProviderMode_isr(
   BXDM_DisplayInterruptHandler_Handle hXdmDih,
   BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback_isr,
   void * pPrivateContext,
   BXDM_DisplayInterruptHandler_PictureProviderMode eMode
   );

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXDM_DisplayInterruptHandler_GetPictureProviderMode_isr(
   BXDM_DisplayInterruptHandler_Handle hXdmDih,
   BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback,
   void * pPrivateContext,
   BXDM_DisplayInterruptHandler_PictureProviderMode * peMode
   );
#endif

#ifdef __cplusplus
}
#endif

#endif /* BXDM_DIH_H_ */
