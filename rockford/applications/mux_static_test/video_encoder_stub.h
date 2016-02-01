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

#ifndef VIDEO_ENCODER_STUB_H_
#define VIDEO_ENCODER_STUB_H_

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* Video Encoder Stub */
typedef struct VideoEncoderStubContext* VideoEncoderHandle;

BERR_Code
app_GetVideoBufferDescriptors(
         VideoEncoderHandle hVideoEncoder,
         const BAVC_VideoBufferDescriptor *astDescriptors0[], /* Can be NULL if uiNumDescriptors0=0. */
         size_t *puiNumDescriptors0,
         const BAVC_VideoBufferDescriptor *astDescriptors1[], /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
         size_t *puiNumDescriptors1
         );

BERR_Code
app_ConsumeVideoBufferDescriptors(
         VideoEncoderHandle hVideoEncoder,
         size_t uiNumDescriptors
         );

BERR_Code
app_GetVideoBufferStatus(
         VideoEncoderHandle hVideoEncoder,
         BAVC_VideoBufferStatus *pBufferStatus
         );

typedef struct VideoEncoderSettings
{
      BMEM_Handle hMem;
} VideoEncoderSettings;

BERR_Code
app_OpenVideoEncoder(
         VideoEncoderHandle *phVideoEncoder,
         const VideoEncoderSettings* pstSettings
         );

BERR_Code
app_CloseVideoEncoder(
         VideoEncoderHandle hVideoEncoder
         );

BERR_Code
app_StopVideoEncoder(
         VideoEncoderHandle hVideoEncoder
         );

BERR_Code
app_VideoEncoderIncrementTime(
         VideoEncoderHandle hVideoEncoder,
         unsigned uiMilliseconds
         );

bool
app_IsInputDone(
   VideoEncoderHandle hVideoEncoder
   );

#ifdef __cplusplus
}
#endif

#endif /* VIDEO_ENCODER_STUB_H_ */
