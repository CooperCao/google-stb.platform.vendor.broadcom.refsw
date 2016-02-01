/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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

#ifndef BMUXLIB_FNRT_PRIV_H_
#define BMUXLIB_FNRT_PRIV_H_

#include "bmuxlib_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BMUXLIB_FNRT_P_SIGNATURE_CREATESETTINGS 0x464e5201
#define BMUXLIB_FNRT_P_SIGNATURE_HANDLE         0x464e5202
#define BMUXLIB_FNRT_P_SIGNATURE_INPUTSETTINGS  0x464e5203
#define BMUXLIB_FNRT_P_SIGNATURE_OUTPUTSETTINGS 0x464e5204

typedef enum BMUXlib_FNRT_P_InputType
{
   BMUXlib_FNRT_P_InputType_eVideo,
   BMUXlib_FNRT_P_InputType_eAudio,

   BMUXlib_FNRT_P_InputType_eMax
} BMUXlib_FNRT_P_InputType;

typedef enum BMUXlib_FNRT_P_InputState
{
   BMUXlib_FNRT_P_InputState_eSelectInput,
   BMUXlib_FNRT_P_InputState_eProcessInput,
   BMUXlib_FNRT_P_InputState_eDone
} BMUXlib_FNRT_P_InputState;

typedef struct BMUXlib_FNRT_P_EncoderGroup_Metadata
{
   signed iInputIndex;
   unsigned uiNumDescriptors;
} BMUXlib_FNRT_P_EncoderGroup_Metadata;

typedef struct BMUXlib_FNRT_P_EncoderGroup_Context *BMUXlib_FNRT_P_EncoderGroup_Handle;

#define BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_INPUTS 6
#define BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_DESCRIPTORS 512

typedef struct BMUXlib_FNRT_P_EncoderGroup_Context
{
   BMUXlib_FNRT_P_InputType eType;

   struct
   {
      bool bEnable;
      union
      {
         BMUXlib_VideoEncoderInterface stVideoInterface;
         BMUXlib_AudioEncoderInterface stAudioInterface;
      } interface;
      unsigned uiPendingDescriptors;
   } input[BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_INPUTS];
   unsigned uiNumActiveInputs;

   struct
   {
      unsigned uiWriteOffset;
      unsigned uiReadOffset;
      unsigned uiQueuedOffset;

      union
      {
         BAVC_VideoBufferDescriptor astVideoDescriptor[BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_DESCRIPTORS];
         BAVC_AudioBufferDescriptor astAudioDescriptor[BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_DESCRIPTORS];
      } descriptors;

      BMUXlib_FNRT_P_EncoderGroup_Metadata stMetadata[BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_DESCRIPTORS];
   } serialized;

   struct
   {
      unsigned uiWriteOffset;
      unsigned uiReadOffset;

      union
      {
         BAVC_VideoBufferDescriptor astVideoDescriptor[BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_DESCRIPTORS];
         BAVC_AudioBufferDescriptor astAudioDescriptor[BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_DESCRIPTORS];
      } descriptors;

      BMUXlib_FNRT_P_EncoderGroup_Metadata stMetadata[BMUXLIB_FNRT_P_ENCODER_GROUP_MAX_DESCRIPTORS];

      union
      {
         BAVC_VideoBufferInfo stVideoBufferInfo;
         BAVC_AudioBufferInfo stAudioBufferInfo;
      } bufferInfo;

      BMUXlib_CompressedBufferStatus stBufferStatus;
      bool bBufferStatusValid;
   } output;

   BMUXlib_FNRT_P_InputState eInputState;
   signed iActiveInputIndex; /* -1 indicates no active input */
   unsigned uiChunkId;
   BMUXlib_FNRT_P_EncoderGroup_Handle hMasterEncoderGroup; /* If NULL, then this is the master */
   struct
   {
      uint64_t uiTargetDeltaNewVsOriginalPts;
      bool bValid;
   } master;
   bool bDropUntilRAP;

   uint64_t uiPreviousPTS;
   bool bPreviousPTSValid;

   uint64_t uiPreviousDTS;
   bool bPreviousDTSValid;

   uint32_t uiPreviousOPTS;
   bool bPreviousOPTSValid;

   unsigned uiDeltaDTS;
   bool bDeltaDTSValid;

   unsigned uiEtsDtsOffset;
   bool bEtsDtsOffsetValid;

   struct
   {
      uint64_t uiTimingOffsetIn90Khz;
      bool bTimingOffsetValid;

      uint64_t uiPreviousOriginalDTS;
      bool bPreviousOriginalDTSValid;

      size_t auiNumDescriptors[2];
      union
      {
            const BAVC_CompressedBufferDescriptor *astCommon[2];
            const BAVC_VideoBufferDescriptor *astVideo[2];
            const BAVC_AudioBufferDescriptor *astAudio[2];
      } descriptors;

      BMUXlib_CompressedBufferStatus stBufferStatus;
      bool bBufferStatusValid;
      bool bEOCSeen;
   } chunk;

   struct
   {
      bool bValid;
      unsigned uiPreviousESCR;
      uint16_t uiPreviousTicksPerBit;
      int16_t iPreviousSHR;
      size_t uiPreviousLength;
   } linearizeESCR;

   struct BMUXlib_FNRT_P_Context *hMain;  /* reference back to main containing context */

#if BMUXLIB_FNRT_P_DUMP_DESC
   BMuxlib_Debug_CSV_Handle hDescDumpFile;
   unsigned *puiDescCount;
#endif
} BMUXlib_FNRT_P_EncoderGroup_Context;

BERR_Code
BMUXlib_FNRT_P_EncoderGroup_GetVideoBufferDescriptors(
   BMUXlib_FNRT_P_EncoderGroup_Handle hMuxFNRTEncoderGroup,
   const BAVC_VideoBufferDescriptor **astDescriptors0,
   size_t *puiNumDescriptors0,
   const BAVC_VideoBufferDescriptor **astDescriptors1,
   size_t *puiNumDescriptors1
   );

BERR_Code
BMUXlib_FNRT_P_EncoderGroup_GetBufferStatus(
   BMUXlib_FNRT_P_EncoderGroup_Handle hMuxFNRTEncoderGroup,
   BMUXlib_CompressedBufferStatus *pBufferStatus
   );

BERR_Code
BMUXlib_FNRT_P_EncoderGroup_GetAudioBufferDescriptors(
   BMUXlib_FNRT_P_EncoderGroup_Handle hMuxFNRTEncoderGroup,
   const BAVC_AudioBufferDescriptor **astDescriptors0,
   size_t *puiNumDescriptors0,
   const BAVC_AudioBufferDescriptor **astDescriptors1,
   size_t *puiNumDescriptors1
   );

BERR_Code
BMUXlib_FNRT_P_EncoderGroup_ConsumeBufferDescriptors(
   BMUXlib_FNRT_P_EncoderGroup_Handle hMuxFNRTEncoderGroup,
   size_t uiNumBufferDescriptors
   );

typedef struct BMUXlib_FNRT_P_Context
{
   uint32_t uiSignature;
   BMUXlib_FNRT_CreateSettings stCreateSettings;

   BMUXlib_FNRT_P_EncoderGroup_Context astEncoderGroup[BMUXlib_FNRT_P_InputType_eMax];

#if BMUXLIB_FNRT_P_DUMP_DESC
   unsigned uiDescCount;
#endif
#if BMUXLIB_FNRT_P_TEST_MODE
   BMuxlib_Debug_CSV_Handle hConfigFile;
#endif
} BMUXlib_FNRT_P_Context;

#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_FNRT_PRIV_H_ */
/* End of File */
