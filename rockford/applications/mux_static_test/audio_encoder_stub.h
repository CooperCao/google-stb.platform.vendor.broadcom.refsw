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

#ifndef AUDIO_ENCODER_STUB_H_
#define AUDIO_ENCODER_STUB_H_

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* Audio Encoder Stub */
typedef struct AudioEncoderStubContext* AudioEncoderHandle;

BERR_Code
app_GetAudioBufferDescriptors(
   AudioEncoderHandle hAudioEncoder,
   const BAVC_AudioBufferDescriptor *astDescriptors0[], /* Can be NULL if uiNumDescriptors0=0. */
   size_t *puiNumDescriptors0,
   const BAVC_AudioBufferDescriptor *astDescriptors1[], /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
   size_t *puiNumDescriptors1
   );

BERR_Code
app_ConsumeAudioBufferDescriptors(
   AudioEncoderHandle hAudioEncoder,
   size_t uiNumDescriptors
   );

BERR_Code
app_GetAudioBufferStatus(
   AudioEncoderHandle hAudioEncoder,
   BAVC_AudioBufferStatus *pBufferStatus
   );

typedef struct AudioEncoderSettings
{
      BMEM_Handle hMem;
} AudioEncoderSettings;

BERR_Code
app_OpenAudioEncoder(
         AudioEncoderHandle *phAudioEncoder,
         const AudioEncoderSettings* pstSettings
         );

BERR_Code
app_CloseAudioEncoder(
         AudioEncoderHandle hAudioEncoder
         );

BERR_Code
app_StopAudioEncoder(
         AudioEncoderHandle hAudioEncoder
         );

BERR_Code
app_AudioEncoderIncrementTime(
         AudioEncoderHandle hAudiooEncoder,
         unsigned uiMilliseconds
         );

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_ENCODER_STUB_H_ */
