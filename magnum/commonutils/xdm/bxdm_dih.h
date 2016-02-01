/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
         BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback,
         void * pPrivateContext,
         BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings *patDefSettings
         );

BERR_Code
BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(
         BXDM_DisplayInterruptHandler_Handle hXdmDih,
         BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback,
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
         BXDM_DisplayInterruptHandler_PictureDataReady_isr fCallback,
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
   BXDM_DisplayInterruptHandler_PictureProvider_isr fCallback,
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
