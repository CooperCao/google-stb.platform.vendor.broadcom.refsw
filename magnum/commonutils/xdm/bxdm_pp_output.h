/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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

#ifndef bxdm_pp_OUTPUT_H__
#define bxdm_pp_OUTPUT_H__


#include "bavc.h"
#include "bxdm_pp_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
}
#endif

void BXDM_PPOUT_P_CalculateStaticVdcData_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_Picture_Context *pPictureContext
   );

void BXDM_PPOUT_P_CalculateVdcData_isr(
   BXDM_PictureProvider_Handle hXdmPP,
   BXDM_PictureProvider_P_LocalState* pLocalState,
   BAVC_MFD_Picture** pMFDPicture
   );

void BXDM_PPOUT_P_OpenChannel(
   BXDM_PictureProvider_Handle hXdmPP
   );

#if 0
BERR_Code BXDM_PPOUT_P_ComputeAspectRatio(
   BAVC_VideoCompressionStd eProtocol,
   BXDM_Picture_AspectRatioInfo stCodedAspectRatio,
   uint32_t uiCustomAspectRatioWidthHeight,
   BFMT_AspectRatio *peAspectRatio,
   uint16_t *puiSampleAspectRatioX,
   uint16_t *puiSampleAspectRatioY
   );
#endif

BERR_Code BXDM_PPOUT_P_ComputeSARScaling_isr(
   uint32_t uiOriginalSourceSizeX,
   uint32_t uiOriginalSourceSizeY,
   uint16_t uiOriginalSampleAspectRatioX,
   uint16_t uiOriginalSampleAspectRatioY,
   uint32_t uiEffectiveSourceSizeX,
   uint32_t uiEffectiveSourceSizeY,
   uint16_t *puiEffectiveSampleAspectRatioX,
   uint16_t *puiEffectiveSampleAspectRatioY
   );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef bxdm_pp_OUTPUT_H__ */
/* End of File */
