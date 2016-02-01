/******************************************************************************
 * (c) 2010-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/


#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"
#include "bdbg.h"

#include "bmuxlib_file_pes_priv.h"

BDBG_MODULE(BMUXLIB_FILE_PES_PRIV);
BDBG_FILE_MODULE(BMUX_PES_IN_DESC);    /* enables input descriptor diagnostics */
BDBG_FILE_MODULE(BMUX_PES_OUT_DESC);   /* enables output descriptor diagnostics */
BDBG_FILE_MODULE(BMUX_PES_RELEASEQ);   /* enables release Q diagnostics */
BDBG_FILE_MODULE(BMUX_PES_DU);         /* enables data unit diagnostics */
BDBG_FILE_MODULE(BMUX_PES_INPUT);      /* enables input diagnostics */
BDBG_FILE_MODULE(BMUX_PES_OUTPUT);     /* enables output diagnostics */
BDBG_FILE_MODULE(BMUX_PES_STATE);      /* enables state machine diagnostics */
BDBG_FILE_MODULE(BMUX_PES_FINISH);     /* enables finish diagnostics */

/****************************
     Static Prototypes
****************************/

static const BMUXlib_File_PES_P_FrameHeader s_stDefaultPESHeader =
{
 {
  0x00,0x00,0x01, /* Start Code */
  0xE0, /* Stream ID */
  0x00, 0x00, /* Packet Length */
  0x80, 0x00, /* Extension w/ PTS+DTS invalid */
  0x0A, /* PES Header Data Length */
  0xFF, /* PTS[32:30] */
  0xFF, 0xFF, /* PTS[29:15] */
  0xFF, 0xFF, /* PTS[14:00] */
  0xFF, /* DTS[32:30] */
  0xFF, 0xFF, /* DTS[29:15] */
  0xFF, 0xFF  /* DTS[14:00] */
 },
};

static const BMUXlib_File_PES_P_VP8Header s_stDefaultVP8Header =
{
   {
      'B','C','M','V', /* Signature */
      0x00, 0x00, 0x00, 0x00, /* Frame Length + Header Length */
      0x00, 0x00 /* 0=normal picture, 0xFF=EOS */
   }
};

/*************************
* P R I V A T E   A P I  *
**************************/

/*
   Function:
      BMUXlib_File_PES_P_Start

   Initialise any settings necessary before starting the Mux process,
   create any necessary auxiliary tracks based on user settings, and create
   the necessary temporary storage for the metadata.

   NOTE: typically, the only way this fails is due to storage error
   (unable to create the metadata storage)
*/
BERR_Code BMUXlib_File_PES_P_Start(BMUXlib_File_PES_Handle hPESMux)
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BMUXlib_File_PES_P_Start);

   if (BERR_SUCCESS == rc)
   {
      BMUXlib_Input_CreateSettings stInputSettings;

      /* create the input ... */
      BMUXlib_Input_GetDefaultCreateSettings(&stInputSettings);
      stInputSettings.eType = BMUXlib_Input_Type_eVideo;
      stInputSettings.interface.stVideo = hPESMux->stStartSettings.stInterface;
      /* NOTE: PES must operate in frame mode since the frame size is required in the frame header */
      stInputSettings.eBurstMode = BMUXlib_Input_BurstMode_eFrame;
      stInputSettings.bFilterUntilMetadataSeen = true;
      rc = BMUXlib_Input_Create(&hPESMux->hInput, &stInputSettings);
   }

   if (BERR_SUCCESS != rc)
   {
      BDBG_ERR(("Unable to Create Video Input"));
   }

   if ( BERR_SUCCESS == rc )
   {
      BMUXlib_Output_CreateSettings stOutputSettings;

      /* create the output ... */
      BMUXlib_Output_GetDefaultCreateSettings(&stOutputSettings);
      stOutputSettings.stStorage = hPESMux->stStartSettings.stOutput;
      rc = BMUXlib_Output_Create(&hPESMux->hOutput, &stOutputSettings);
   }
   if (BERR_SUCCESS != rc)
   {
      BDBG_ERR(("Unable to Create Output"));
   }

   if (BERR_SUCCESS == rc)
      /* everything OK, so start the mux ... */
      hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eStartup;
   else
      /* else, cleanup anything partially created */
      BMUXlib_File_PES_P_Stop(hPESMux);

   BDBG_LEAVE(BMUXlib_File_PES_P_Start);
   return rc;
}

/*
   Function:
      BMUXlib_File_PES_P_Stop

   Shut down all muxing and release resources in use (typically
   this involves destroying the temporary storage used for metadata
   etc.).

   Also return context to post-create state.

   NOTE: this can be called at any time if the user wishes
   to perform a "hard" stop (i.e. without calling Finish())
*/
void BMUXlib_File_PES_P_Stop(BMUXlib_File_PES_Handle hPESMux)
{
   BDBG_ENTER(BMUXlib_File_PES_P_Stop);

   /* destroy output */
   if ( NULL != hPESMux->hOutput )
   {
      BMUXlib_Output_Destroy( hPESMux->hOutput );
      hPESMux->hOutput = NULL;
   }

   /* destroy input */
   if (NULL != hPESMux->hInput)
   {
      BMUXlib_Input_Destroy(hPESMux->hInput);
      hPESMux->hInput = NULL;
   }

   BDBG_LEAVE(BMUXlib_File_PES_P_Stop);
}

/*
   Function:
      BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting

   Processed descriptors that are queued up and waiting to be output to storage
   If a storage interface is busy, descriptors will remain waiting until the next iteration

   Notes:
   The current design uses a single descriptor queue for all the outgoing descriptors.
   Thus, if any single interface stops processing descriptors in a timely manner, this will
   eventually block processing completely.  If this becomes an issue separate queues could be
   maintained for each output, although this will not solve the ultimate problem, only delay it

   Returns
      Storage interface error code
   (This will only fail if an error occurs on the storage interface)
*/
BERR_Code BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting(BMUXlib_File_PES_Handle hPESMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting);
   hPESMux->stStatus.bBlockedOutput= false;

   if (NULL != hPESMux->hOutput)
   {
      rc = BMUXlib_Output_ProcessNewDescriptors(hPESMux->hOutput);
      /* if any output did not queue up all descriptors, then we are blocked ... */
      /* NOTE: it is questionable whether blocked due to lack of a resource that
         is output dependent should be treated as "blocked output" also
         (since processing the output will free the blocked resources)
         Perhaps we may need to include all these "blocked" criteria also */
      if (BMUXlib_Output_IsDescriptorPendingQueue(hPESMux->hOutput))
         hPESMux->stStatus.bBlockedOutput = true;
   }

   BDBG_LEAVE(BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting);
   return rc;
}

/*
   Function:
      BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted

   Process completed descriptors returned by the storage interface.  Release
   all resources pointed to by these descriptors (such as box buffer, metadata cache
   input descriptors, etc) and then free the descriptors

   Returns:
      BERR_UNKNOWN -    bad source type detected in descriptor
      + Other errors from storage interface
      + Other errors from input interface(s)
*/
BERR_Code BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted(BMUXlib_File_PES_Handle hPESMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted);

   rc = BMUXlib_Output_ProcessCompletedDescriptors(hPESMux->hOutput);

   if ( 0 != hPESMux->uiCompletedCount )
   {
      rc = BMUXlib_Input_ConsumeDescriptors(
               hPESMux->hInput,
               hPESMux->uiCompletedCount
               );

      hPESMux->uiCompletedCount = 0;
   }
   BDBG_LEAVE(BMUXlib_File_PES_P_ProcessOutputDescriptorsCompleted);
   return rc;
}

/*
   Function:
      BMUXlib_File_PES_P_ProcessInputDescriptorsWaiting

   Process new input descriptors that are waiting

   Returns:
      Input interface error code
*/
BERR_Code BMUXlib_File_PES_P_ProcessInputDescriptorsWaiting(BMUXlib_File_PES_Handle hPESMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_PES_P_ProcessInputDescriptorsWaiting);

   if (NULL != hPESMux->hInput)
   {
      rc = BMUXlib_Input_ProcessNewDescriptors(hPESMux->hInput);
   }

   BDBG_LEAVE(BMUXlib_File_PES_P_ProcessInputDescriptorsWaiting);
   return rc;
}

void BMUXlib_File_PES_P_Output_InputDescriptorDone(
   void *pPrivateData,
   const BMUXlib_Output_Descriptor *pOutputDescriptor
   )
{
   BMUXlib_File_PES_Handle hPESMux = (BMUXlib_File_PES_Handle) pPrivateData;

   BDBG_ASSERT( hPESMux->uiPendingCount );
   BSTD_UNUSED( pOutputDescriptor );

   hPESMux->uiCompletedCount++;
   hPESMux->uiPendingCount--;
}

void BMUXlib_File_PES_P_Output_FrameHeaderDone(
   void *pPrivateData,
   const BMUXlib_Output_Descriptor *pOutputDescriptor
   )
{
   BMUXlib_File_PES_Handle hPESMux = (BMUXlib_File_PES_Handle) pPrivateData;

   BDBG_ASSERT( hPESMux->stFrameHeader.uiReadOffset != hPESMux->stFrameHeader.uiWriteOffset );
   BSTD_UNUSED( pOutputDescriptor );

   hPESMux->stFrameHeader.uiReadOffset++;
   hPESMux->stFrameHeader.uiReadOffset %= BMUXlib_File_PES_P_MAX_FRAMES;
}

void BMUXlib_File_PES_P_Output_VP8HeaderDone(
   void *pPrivateData,
   const BMUXlib_Output_Descriptor *pOutputDescriptor
   )
{
   BMUXlib_File_PES_Handle hPESMux = (BMUXlib_File_PES_Handle) pPrivateData;

   BDBG_ASSERT( hPESMux->stVP8Header.uiReadOffset != hPESMux->stVP8Header.uiWriteOffset );
   BSTD_UNUSED( pOutputDescriptor );

   hPESMux->stVP8Header.uiReadOffset++;
   hPESMux->stVP8Header.uiReadOffset %= BMUXlib_File_PES_P_MAX_FRAMES;
}

#define BMUXlib_File_PES_P_VP8_EOS_SIZE BMUXlib_File_PES_P_VP8_HEADER_SIZE + 4

static const uint8_t s_stVP8EOS[BMUXlib_File_PES_P_VP8_EOS_SIZE] =
{
   'B','C','M','V', /* Signature */
   0x00, 0x00, 0x00, 0x06, /* Frame Length + Header Length */
   0x00, 0xFF, /* 0=normal picture, 0xFF=EOS */
   0x00, 0x00, 0x00, 0x00
};

/*
   Function:
      BMUXlib_File_PES_P_ProcessInputDescriptors

   Process new data from encoder(s) and create output descriptor lists to describe the data
   for the mdat chunks, as well as record the metadata for each sample

   NOTES:
   Descriptors are always processed in DTS-order (the ESCR is not used here, although
   it is assumed that there is a constant relationship between DTS and ESCR).
   It is expected that video frames are delivered in DTS order from the encoder.

   EVERY input descriptor *must* have a corresponding output descriptor created. The output
   descriptor is the link that causes the input descriptor to be freed when the data is
   consumed.  If the data is not required, then the output descriptor length must be zero.
   This ensures the input descriptor gets freed correctly in the order they are provided

   The only exception to this is when input descriptors are consumed *prior* to processing
   descriptors into output descriptors, whereby input descriptors can be discarded by
   simply returning them to the input.  Once samples have been processed, this can no
   longer be done (i.e. can only be done in the startup state).
*/
BERR_Code BMUXlib_File_PES_P_ProcessInputDescriptors(BMUXlib_File_PES_Handle hPESMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_PES_P_ProcessInputDescriptors);

   rc = BMUXlib_File_PES_P_ProcessInputDescriptorsWaiting(hPESMux);
   if (BERR_SUCCESS != rc)
   {
      BDBG_LEAVE(BMUXlib_File_PES_P_ProcessInputDescriptors);
      return rc;
   }

   BDBG_MODULE_MSG(BMUX_PES_STATE, ("====Processing Input Descriptors===="));

   /* keep processing as long as there are input descriptors available, and resources to process them ... */
   while ( (BERR_SUCCESS == rc) && BMUXlib_Input_IsDescriptorAvailable( hPESMux->hInput ) )
   {
      BMUXlib_File_PES_P_InputState ePreviousInputState = hPESMux->eInputState;
      BMUXlib_Input_Descriptor stInputDescriptor;
      bool bDescAvail;

      bDescAvail = BMUXlib_Input_PeekAtNextDescriptor(
         hPESMux->hInput,
         &stInputDescriptor
      );
      if ( !bDescAvail )
      {
         /* NOTE: This should not happen due to IsDescriptorAvailable() check, above */
         rc = BERR_TRACE(BERR_UNKNOWN);
         break;
      }

      switch (hPESMux->eInputState)
      {
         /* Find the SOF (start of frame) on each stream, and process any initial metadata ...
            NOTE: This is the only state in here that is permitted to consume input descriptors directly */
         case BMUXlib_File_PES_P_InputState_eStartup:
         {
            void *pMetadataBase = BMMA_Lock( BMUXLIB_INPUT_DESCRIPTOR_BLOCK( &stInputDescriptor ) );
            BAVC_VideoMetadataDescriptor *pstVideoMetadata;

            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stInputDescriptor ) );

            pstVideoMetadata = BMUXLIB_INPUT_DESCRIPTOR_VIRTUAL_ADDRESS( pMetadataBase, &stInputDescriptor );

            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stInputDescriptor ) );

            if ( 0 != ( pstVideoMetadata->uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BUFFERINFO_VALID ) )
            {
               hPESMux->eProtocol = pstVideoMetadata->stBufferInfo.eProtocol;
            }

            BMMA_Unlock( BMUXLIB_INPUT_DESCRIPTOR_BLOCK( &stInputDescriptor ), pMetadataBase );

            BDBG_MODULE_MSG(BMUX_PES_STATE, ("eStartup --> eProcessMetadata"));
            hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eProcessMetadata;
         }
         break;

         case BMUXlib_File_PES_P_InputState_eGetNextDescriptor:
         {
            hPESMux->uiBytesLeftInDescriptor = BMUXLIB_INPUT_DESCRIPTOR_LENGTH( &stInputDescriptor );

            /* Check if metadata, frame, or frame data */
            if ( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stInputDescriptor ) )
            {
               BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGetNextDescriptor --> eProcessMetadata"));
               hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eProcessMetadata;
            }
            else if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stInputDescriptor ) )
            {
               BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGetNextDescriptor --> eGenerateFrameHeader"));
               hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eGenerateFrameHeader;
            }
            else
            {
               BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGetNextDescriptor --> eProcessFrameData"));
               hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eProcessFrameData;
            }
         }
         break;
         case BMUXlib_File_PES_P_InputState_eProcessMetadata:
         {
            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stInputDescriptor ) );

            /* Queue dummy descriptor to output to skip descriptor */
            if ( true == BMUXlib_Output_IsSpaceAvailable( hPESMux->hOutput ) )
            {
               BMUXlib_Output_Descriptor stOutputDescriptor;
               BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

               BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
               BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

               stOutputDescriptor.stStorage.bWriteOperation = false;
               stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
               stOutputDescriptor.stStorage.pBufferAddress = 0;
               stOutputDescriptor.stStorage.uiLength = 0;
               stOutputDescriptor.stStorage.uiOffset = 0;

               stCompletedCallbackInfo.pCallbackData = hPESMux;
               stCompletedCallbackInfo.pCallback = BMUXlib_File_PES_P_Output_InputDescriptorDone;

               rc = BMUXlib_Output_AddNewDescriptor(
                        hPESMux->hOutput,
                        &stOutputDescriptor,
                        &stCompletedCallbackInfo
                        );

               if ( BERR_SUCCESS == rc )
               {
                  hPESMux->uiPendingCount++;

                  /* NOTE: The following should always succeed since we verified
                     descriptor existence prior to entering the loop */
                  bDescAvail = BMUXlib_Input_GetNextDescriptor(
                     hPESMux->hInput,
                     &stInputDescriptor
                  );
                  BDBG_ASSERT( true == bDescAvail );

                  BDBG_MODULE_MSG(BMUX_PES_STATE, ("eProcessMetadata --> eGetNextDescriptor"));
                  hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eGetNextDescriptor;
               }
            }
         }
         break;

         case BMUXlib_File_PES_P_InputState_eGenerateFrameHeader:
         {
            unsigned uiWriteOffsetTemp = (hPESMux->stFrameHeader.uiWriteOffset + 1) % BMUXlib_File_PES_P_MAX_FRAMES;

            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stInputDescriptor ) );
            BDBG_ASSERT( sizeof( hPESMux->stFrameHeader.astFrameHeader[0] ) == BMUXlib_File_PES_P_FRAME_HEADER_SIZE );

            /* Write Frame Header */
            if ( ( true == BMUXlib_Output_IsSpaceAvailable( hPESMux->hOutput ) )
                 && ( uiWriteOffsetTemp != hPESMux->stFrameHeader.uiReadOffset ) )
            {
               BMUXlib_File_PES_P_FrameHeader *pstFrameHeader = &hPESMux->stFrameHeader.astFrameHeader[hPESMux->stFrameHeader.uiWriteOffset];

               *pstFrameHeader = s_stDefaultPESHeader;

               hPESMux->uiFrameSize = BMUXLIB_INPUT_DESCRIPTOR_FRAMESIZE( &stInputDescriptor );
               hPESMux->uiBytesLeftInFrame = BMUXLIB_INPUT_DESCRIPTOR_FRAMESIZE( &stInputDescriptor );

               if ( BAVC_VideoCompressionStd_eVP8 == hPESMux->eProtocol )
               {
                  /* VP8 requires a VP8 header followed by the frame data.  Since VP8 doesn't have start code emulation prevention,
                   * the frame may need to be split into multiple PES packets each containaing a valid length in the PES header */
                  hPESMux->uiBytesLeftInPES = (BMUXlib_File_PES_P_FRAME_HEADER_SIZE - 6) + BMUXlib_File_PES_P_VP8_HEADER_SIZE + hPESMux->uiFrameSize;
                  if ( hPESMux->uiBytesLeftInPES > BMUXlib_File_PES_P_MAX_PES_LENGTH ) hPESMux->uiBytesLeftInPES = BMUXlib_File_PES_P_MAX_PES_LENGTH;
                  BMUXlib_File_PES_P_Set16_BE( pstFrameHeader->auiBytes, BMUXlib_File_PES_P_FrameHeader_FrameSize_OFFSET, hPESMux->uiBytesLeftInPES );
               }
               else
               {
                  /* Length-less PES */
                  hPESMux->uiBytesLeftInPES = hPESMux->uiBytesLeftInFrame;
               }

               if ( BMUXLIB_INPUT_DESCRIPTOR_IS_PTS_VALID( &stInputDescriptor ) )
               {
                  BMUXlib_File_PES_P_PESHeader_SetPTS( pstFrameHeader->auiBytes, BMUXLIB_INPUT_DESCRIPTOR_PTS( &stInputDescriptor ) );
               }

               if ( BMUXLIB_INPUT_DESCRIPTOR_IS_DTS_VALID( &stInputDescriptor ) )
               {
                  BMUXlib_File_PES_P_PESHeader_SetDTS( pstFrameHeader->auiBytes, BMUXLIB_INPUT_DESCRIPTOR_DTS( &stInputDescriptor ) );
               }

               {
                  BMUXlib_Output_Descriptor stOutputDescriptor;
                  BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

                  BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
                  BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

                  stOutputDescriptor.stStorage.bWriteOperation = true;
                  stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
                  stOutputDescriptor.stStorage.pBufferAddress = pstFrameHeader->auiBytes;
                  stOutputDescriptor.stStorage.uiLength = BMUXlib_File_PES_P_FRAME_HEADER_SIZE;
                  stOutputDescriptor.stStorage.uiOffset = 0;

                  stCompletedCallbackInfo.pCallbackData = hPESMux;
                  stCompletedCallbackInfo.pCallback = BMUXlib_File_PES_P_Output_FrameHeaderDone;

                  rc = BMUXlib_Output_AddNewDescriptor(
                           hPESMux->hOutput,
                           &stOutputDescriptor,
                           &stCompletedCallbackInfo
                           );

                  if ( BERR_SUCCESS == rc )
                  {
                     hPESMux->uiBytesLeftInPES -= (BMUXlib_File_PES_P_FRAME_HEADER_SIZE - 6);
                     hPESMux->stFrameHeader.uiWriteOffset = uiWriteOffsetTemp;
                     hPESMux->uiFrameCount++;

                     if ( BAVC_VideoCompressionStd_eVP8 == hPESMux->eProtocol )
                     {
                        /* Each frame in VP8 requires a special BRCM specific VP8 header */
                        BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGenerateFrameHeader --> eGenerateVP8Header"));
                        hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eGenerateVP8Header;
                     }
                     else
                     {
                        BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGenerateFrameHeader --> eProcessFrameData"));
                        hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eProcessFrameData;
                     }
                  }
               }
            }
         }
         break;

         case BMUXlib_File_PES_P_InputState_eGenerateVP8Header:
         {
            unsigned uiWriteOffsetTemp = (hPESMux->stVP8Header.uiWriteOffset + 1) % BMUXlib_File_PES_P_MAX_FRAMES;

            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stInputDescriptor ) );
            BDBG_ASSERT( sizeof( hPESMux->stVP8Header.astFrameHeader[0] ) == BMUXlib_File_PES_P_VP8_HEADER_SIZE );

            /* Write Frame Header */
            if ( ( true == BMUXlib_Output_IsSpaceAvailable( hPESMux->hOutput ) )
                 && ( uiWriteOffsetTemp != hPESMux->stVP8Header.uiReadOffset ) )
            {
               BMUXlib_File_PES_P_VP8Header *pstVP8Header = &hPESMux->stVP8Header.astFrameHeader[hPESMux->stVP8Header.uiWriteOffset];

               *pstVP8Header = s_stDefaultVP8Header;

               BMUXlib_File_PES_P_Set32_BE( &pstVP8Header->auiBytes, BMUXlib_File_PES_P_VP8Header_FrameSize_OFFSET, BMUXlib_File_PES_P_VP8_HEADER_SIZE + hPESMux->uiFrameSize );

               {
                  BMUXlib_Output_Descriptor stOutputDescriptor;
                  BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

                  BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
                  BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

                  stOutputDescriptor.stStorage.bWriteOperation = true;
                  stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
                  stOutputDescriptor.stStorage.pBufferAddress = pstVP8Header->auiBytes;
                  stOutputDescriptor.stStorage.uiLength = BMUXlib_File_PES_P_VP8_HEADER_SIZE;
                  stOutputDescriptor.stStorage.uiOffset = 0;

                  stCompletedCallbackInfo.pCallbackData = hPESMux;
                  stCompletedCallbackInfo.pCallback = BMUXlib_File_PES_P_Output_VP8HeaderDone;

                  rc = BMUXlib_Output_AddNewDescriptor(
                           hPESMux->hOutput,
                           &stOutputDescriptor,
                           &stCompletedCallbackInfo
                           );

                  if ( BERR_SUCCESS == rc )
                  {
                     hPESMux->uiBytesLeftInPES -= stOutputDescriptor.stStorage.uiLength;
                     hPESMux->stVP8Header.uiWriteOffset = uiWriteOffsetTemp;

                     BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGenerateVP8Header --> eProcessFrameData"));
                     hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eProcessFrameData;
                  }
               }
            }
         }
         break;

         case BMUXlib_File_PES_P_InputState_eGeneratePESHeader:
         {
            unsigned uiWriteOffsetTemp = (hPESMux->stFrameHeader.uiWriteOffset + 1) % BMUXlib_File_PES_P_MAX_FRAMES;

            BDBG_ASSERT( sizeof( hPESMux->stFrameHeader.astFrameHeader[0] ) == BMUXlib_File_PES_P_FRAME_HEADER_SIZE );

            /* Write Frame Header */
            if ( ( true == BMUXlib_Output_IsSpaceAvailable( hPESMux->hOutput ) )
                 && ( uiWriteOffsetTemp != hPESMux->stFrameHeader.uiReadOffset ) )
            {
               bool bEOS = false;
               BMUXlib_File_PES_P_FrameHeader *pstFrameHeader = &hPESMux->stFrameHeader.astFrameHeader[hPESMux->stFrameHeader.uiWriteOffset];

               *pstFrameHeader = s_stDefaultPESHeader;

               hPESMux->uiBytesLeftInPES = (BMUXlib_File_PES_P_FRAME_HEADER_SIZE - 6) + hPESMux->uiBytesLeftInFrame;
               if ( hPESMux->uiBytesLeftInPES > BMUXlib_File_PES_P_MAX_PES_LENGTH ) hPESMux->uiBytesLeftInPES = BMUXlib_File_PES_P_MAX_PES_LENGTH;

               /* Handle EOS for PES */
               if ( 0 == hPESMux->uiBytesLeftInFrame )
               {
                  bEOS = true;
                  hPESMux->uiBytesLeftInPES = (BMUXlib_File_PES_P_FRAME_HEADER_SIZE - 6) + BMUXlib_File_PES_P_VP8_EOS_SIZE;
               }

               BMUXlib_File_PES_P_Set16_BE( pstFrameHeader->auiBytes, BMUXlib_File_PES_P_FrameHeader_FrameSize_OFFSET, hPESMux->uiBytesLeftInPES );

               {
                  BMUXlib_Output_Descriptor stOutputDescriptor;
                  BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

                  BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
                  BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

                  stOutputDescriptor.stStorage.bWriteOperation = true;
                  stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
                  stOutputDescriptor.stStorage.pBufferAddress = pstFrameHeader->auiBytes;
                  stOutputDescriptor.stStorage.uiLength = BMUXlib_File_PES_P_FRAME_HEADER_SIZE;
                  stOutputDescriptor.stStorage.uiOffset = 0;

                  stCompletedCallbackInfo.pCallbackData = hPESMux;
                  stCompletedCallbackInfo.pCallback = BMUXlib_File_PES_P_Output_FrameHeaderDone;

                  rc = BMUXlib_Output_AddNewDescriptor(
                           hPESMux->hOutput,
                           &stOutputDescriptor,
                           &stCompletedCallbackInfo
                           );

                  if ( BERR_SUCCESS == rc )
                  {
                     hPESMux->uiBytesLeftInPES -= (BMUXlib_File_PES_P_FRAME_HEADER_SIZE - 6);
                     hPESMux->stFrameHeader.uiWriteOffset = uiWriteOffsetTemp;

                     if ( true == bEOS )
                     {
                        BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGenerateFrameHeader --> eProcessEOS"));
                        hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eProcessEOS;
                     }
                     else
                     {
                        BDBG_MODULE_MSG(BMUX_PES_STATE, ("eGenerateFrameHeader --> eProcessFrameData"));
                        hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eProcessFrameData;
                     }
                  }
               }
            }
         }
         break;

         case BMUXlib_File_PES_P_InputState_eProcessFrameData:
         {
            /* Write Data to Disk */
            if ( true == BMUXlib_Output_IsSpaceAvailable( hPESMux->hOutput ) )
            {
               BMUXlib_Output_Descriptor stOutputDescriptor;
               BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;
               unsigned uiLength = hPESMux->uiBytesLeftInPES;

               BDBG_ASSERT( hPESMux->uiBytesLeftInFrame >= hPESMux->uiBytesLeftInPES );

               /* For the length-less PES, the the bytes in the frame may not equal the
                * bytes in the descriptor since the frame could be split into multiple
                * descriptors.  So, we limit the length to the bytes left in the descriptor
                */
               if ( uiLength > hPESMux->uiBytesLeftInDescriptor )
               {
                  uiLength = hPESMux->uiBytesLeftInDescriptor;
               }

               BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
               BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

               stOutputDescriptor.stStorage.bWriteOperation = true;
               stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
               stOutputDescriptor.stStorage.pBufferAddress = NULL;
               stOutputDescriptor.stStorage.hBlock = BMUXLIB_INPUT_DESCRIPTOR_BLOCK( &stInputDescriptor );
               stOutputDescriptor.stStorage.uiBlockOffset = BMUXLIB_INPUT_DESCRIPTOR_OFFSET( &stInputDescriptor ) + ( BMUXLIB_INPUT_DESCRIPTOR_LENGTH( &stInputDescriptor ) - hPESMux->uiBytesLeftInDescriptor );
               stOutputDescriptor.stStorage.uiLength = uiLength;
               stOutputDescriptor.stStorage.uiOffset = 0;

               if ( hPESMux->uiBytesLeftInDescriptor == stOutputDescriptor.stStorage.uiLength )
               {
                  stCompletedCallbackInfo.pCallbackData = hPESMux;
                  stCompletedCallbackInfo.pCallback = BMUXlib_File_PES_P_Output_InputDescriptorDone;
               }

               rc = BMUXlib_Output_AddNewDescriptor(
                        hPESMux->hOutput,
                        &stOutputDescriptor,
                        &stCompletedCallbackInfo
                        );

               if ( BERR_SUCCESS == rc )
               {
                  hPESMux->uiBytesLeftInPES -= stOutputDescriptor.stStorage.uiLength;
                  hPESMux->uiBytesLeftInFrame -= stOutputDescriptor.stStorage.uiLength;
                  hPESMux->uiBytesLeftInDescriptor -= stOutputDescriptor.stStorage.uiLength;

                  hPESMux->uiPendingCount++;

                  if ( 0 == hPESMux->uiBytesLeftInFrame )
                  {
                     /* We are done with the frame, so check if the descriptor is also an EOS */
                     if ( BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stInputDescriptor ) )
                     {
                        BDBG_MODULE_MSG(BMUX_PES_STATE, ("eProcessFrameData --> eGeneratePESHeader (EOS)"));
                        hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eGeneratePESHeader;
                     }
                     else
                     {
                        BDBG_ASSERT( 0 == hPESMux->uiBytesLeftInDescriptor );
                        /* We're done with this descriptor, so pull it off the queue */

                        /* NOTE: The following should always succeed since we verified
                           descriptor existence prior to entering the loop */
                        bDescAvail = BMUXlib_Input_GetNextDescriptor(
                           hPESMux->hInput,
                           &stInputDescriptor
                        );
                        BDBG_ASSERT( true == bDescAvail );

                        BDBG_MODULE_MSG(BMUX_PES_STATE, ("eProcessFrameData --> eGetNextDescriptor"));
                        hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eGetNextDescriptor;
                     }
                  }
                  else if ( 0 != hPESMux->uiBytesLeftInDescriptor )
                  {
                     /* We're not done with the descriptor.  This only happens if we have a
                      * PES header with length and there isn't enough space in this PES packet,
                      * so send another PES header before sending the remaining data
                      */
                     BDBG_MODULE_MSG(BMUX_PES_STATE, ("eProcessFrameData --> eGeneratePESHeader"));
                     hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eGeneratePESHeader;
                  }
                  else
                  {
                     /* We're done with this descriptor, so pull it off the queue, but we're not done
                      * with the frame, yet. */

                     /* NOTE: The following should always succeed since we verified
                           descriptor existence prior to entering the loop */
                     bDescAvail = BMUXlib_Input_GetNextDescriptor(
                        hPESMux->hInput,
                        &stInputDescriptor
                     );
                     BDBG_ASSERT( true == bDescAvail );

                     BDBG_MODULE_MSG(BMUX_PES_STATE, ("eProcessFrameData --> eGetNextDescriptor"));
                     hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eGetNextDescriptor;
                  }
               }
            }
         }
         break;

         case BMUXlib_File_PES_P_InputState_eProcessEOS:
         {
            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stInputDescriptor ) );

            /* Write Data to Disk */
            if ( true == BMUXlib_Output_IsSpaceAvailable( hPESMux->hOutput ) )
            {
               BMUXlib_Output_Descriptor stOutputDescriptor;
               BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

               BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
               BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

               stOutputDescriptor.stStorage.bWriteOperation = true;
               stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
               stOutputDescriptor.stStorage.pBufferAddress = &s_stVP8EOS;
               stOutputDescriptor.stStorage.uiLength = BMUXlib_File_PES_P_VP8_EOS_SIZE;
               stOutputDescriptor.stStorage.uiOffset = 0;

               rc = BMUXlib_Output_AddNewDescriptor(
                        hPESMux->hOutput,
                        &stOutputDescriptor,
                        &stCompletedCallbackInfo
                        );

               if ( BERR_SUCCESS == rc )
               {
                  BDBG_WRN(( "Wrote %d frames", hPESMux->uiFrameCount ));

                  /* NOTE: The following should always succeed since we verified
                           descriptor existence prior to entering the loop */
                  bDescAvail = BMUXlib_Input_GetNextDescriptor(
                     hPESMux->hInput,
                     &stInputDescriptor
                  );
                  BDBG_ASSERT( true == bDescAvail );

                  BDBG_MODULE_MSG(BMUX_PES_STATE, ("eProcessEOS --> eDone"));
                  hPESMux->eInputState = BMUXlib_File_PES_P_InputState_eDone;
               }
            }
         }
         break;

         case BMUXlib_File_PES_P_InputState_eDone:
         {

         }
         break;
         default:
            /* Unknown state! should not happen ... typically indicates memory overwrite
               this is also a catch-all if it somehow gets to "started" state without invoking Start() */
            BDBG_ERR(("ProcessInputDescriptors:: Unexpected State: %d", hPESMux->eInputState));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
      } /* end: switch input state */

      /* The state hasn't changed, so we're waiting on something */
      if ( ePreviousInputState == hPESMux->eInputState )
      {
         break;
      }
   } /* end: while not blocked, not done && not error */

   if (BERR_SUCCESS == rc)
      rc = BMUXlib_File_PES_P_ProcessOutputDescriptorsWaiting(hPESMux);

   BDBG_LEAVE(BMUXlib_File_PES_P_ProcessInputDescriptors);
   return rc;
}

bool BMUXlib_File_PES_P_EOSSeen( BMUXlib_File_PES_Handle hPESMux )
{
   bool bEOSSeen = false;

   BDBG_ENTER( BMUXlib_File_PES_P_EOSSeen );

   bEOSSeen = hPESMux->eInputState == BMUXlib_File_PES_P_InputState_eDone;

   BDBG_LEAVE( BMUXlib_File_PES_P_EOSSeen );

   return bEOSSeen;
}

/*
   Function:
      BMUXlib_File_PES_P_IsInputProcessingDone

   Predicate to determine if Input processing has completed

   This is only called after Finish() has been invoked.  Thus, this
   will define "done" as all input queues empty, all descriptors returned
   to encoder(s) and all input processing is complete.
*/
bool BMUXlib_File_PES_P_IsInputProcessingDone(BMUXlib_File_PES_Handle hPESMux)
{
   bool bDone = false;

   BDBG_ENTER( BMUXlib_File_PES_P_IsInputProcessingDone );

   bDone = ( hPESMux->eInputState == BMUXlib_File_PES_P_InputState_eDone );

   BDBG_LEAVE( BMUXlib_File_PES_P_IsInputProcessingDone );

   return bDone;
}

/*
   Function:
      BMUXlib_File_PES_P_IsOutputProcessingDone

   Predicate to determine if Output processing has completed

   This is only called after input is complete (as defined above)
   This will determine "done" when all output sources have finalised
   output, all output descriptors have been returned and dependent
   data sources released
*/
bool BMUXlib_File_PES_P_IsOutputProcessingDone(BMUXlib_File_PES_Handle hPESMux)
{
   bool bDone = false;

   BDBG_ENTER(BMUXlib_File_PES_P_IsOutputProcessingDone);
   /* ensure all output descriptors have been returned and all
      dependent data sources have been released ... */
   if ( ( false == BMUXlib_Output_IsDescriptorPendingCompletion(hPESMux->hOutput) )
        && ( false == BMUXlib_Output_IsDescriptorPendingQueue(hPESMux->hOutput) ) )
   {
      bDone=true;
   }

   BDBG_LEAVE(BMUXlib_File_PES_P_IsOutputProcessingDone);
   return bDone;
}

/*****************************************************************************
* EOF
******************************************************************************/
