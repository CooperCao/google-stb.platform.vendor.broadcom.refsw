/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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

#ifndef BXVD_DECODER_H_
#define BXVD_DECODER_H_

#include "bxvd.h"
/*#include "bxvd_decoder_timer.h"*/

#ifdef __cplusplus
extern "C" {
#endif

/* A bit of overkill for one counter, perhaps more
 * will be added later.
 */
typedef struct BXVD_Decoder_Counters
{
   uint32_t uiDecoderInputOverflow;  /* Number of times the decoder input has gotten into an overflow state. */
   uint32_t uiVsyncCount;           /* The number times BXVD_Decoder_DisplayInterruptEvent_isr has
                                     * been called.  Conditionally reset in BXVD_Decoder_StartDecode_isr.
                                     */
}  BXVD_Decoder_Counters;


/*
 * Public functions.
 */
BERR_Code
BXVD_Decoder_GetPictureCount_isr(
         BXVD_ChannelHandle hXvdCh,
         uint32_t *puiPictureCount
         );

BERR_Code
BXDM_PP_PP_Decoder_PeekAtPicture_isr(
         BXVD_ChannelHandle hXvdCh,
         uint32_t uiIndex,
         BXDM_Picture **pUnifiedPicture
         );

BERR_Code
BXVD_Decoder_GetNextPicture_isr(
         BXVD_ChannelHandle hXvdCh,
         BXDM_Picture **pUnifiedPicture
         );

BERR_Code
BXVD_Decoder_ReleasePicture_isr(
         BXVD_ChannelHandle hXvdCh,
         BXDM_Picture *pUnifiedPicture,
         const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo
         );

BERR_Code
BXVD_Decoder_GetPictureDropPendingCount_isr(
         BXVD_ChannelHandle hXvdCh,
         uint32_t *puiPictureDropPendingCount
         );

BERR_Code
BXVD_Decoder_RequestPictureDrop_isr(
         BXVD_ChannelHandle hXvdCh,
         uint32_t *puiPictureDropRequestCount
         );

BERR_Code
BXVD_Decoder_DisplayInterruptEvent_isr(
         BXVD_ChannelHandle hXvdCh
         );

BERR_Code
BXVD_Decoder_OpenChannel(
      BXVD_ChannelHandle hXvdCh
      );

BERR_Code
BXVD_Decoder_CloseChannel(
      BXVD_ChannelHandle hXvdCh
      );

BERR_Code
BXVD_Decoder_GetDMInterface(
         BXVD_ChannelHandle hXvdCh,
         BXDM_Decoder_Interface *pstDecoderInterface,
         void **pPrivateContext
         );

BERR_Code
BXVD_Decoder_GetPPBParameterQueueInfo_isr(
         BXVD_ChannelHandle hXvdCh,
         const BXVD_PPBParameterInfo* apstPPBParameterInfo[],
         uint32_t uiPPBParameterInfoCount,
         uint32_t *puiValidPPBParameterInfoCount
   );

BERR_Code
BXVD_Decoder_SetCRCMode_isr(
         BXVD_ChannelHandle hXvdCh,
         bool bEnable);

#if !B_REFSW_MINIMAL /* SWSTB-461 */
BERR_Code
BXVD_Decoder_GetCRCMode_isr(
         BXVD_ChannelHandle hXvdCh,
         bool *pbEnable);
#endif

BERR_Code
BXVD_Decoder_StartDecode_isr(
         BXVD_ChannelHandle hXvdCh
         );

BERR_Code
BXVD_Decoder_StopDecode_isr(
         BXVD_ChannelHandle hXvdCh
         );

BERR_Code
BXVD_Decoder_WatchdogReset_isr(
         BXVD_ChannelHandle hXvdCh
         );

BERR_Code
BXVD_Decoder_GetCounters_isr(
      BXVD_ChannelHandle hXvdCh,
      BXVD_Decoder_Counters * pstCounters
      );


#ifdef __cplusplus
}
#endif

#endif /* BXVD_DECODER_H_ */
