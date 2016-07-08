/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/


#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"
#include "bdbg.h"

#include "bmuxlib_file_ivf_priv.h"

BDBG_MODULE(BMUXLIB_FILE_IVF_PRIV);
BDBG_FILE_MODULE(BMUX_IVF_IN_DESC);    /* enables input descriptor diagnostics */
BDBG_FILE_MODULE(BMUX_IVF_OUT_DESC);   /* enables output descriptor diagnostics */
BDBG_FILE_MODULE(BMUX_IVF_RELEASEQ);   /* enables release Q diagnostics */
BDBG_FILE_MODULE(BMUX_IVF_DU);         /* enables data unit diagnostics */
BDBG_FILE_MODULE(BMUX_IVF_INPUT);      /* enables input diagnostics */
BDBG_FILE_MODULE(BMUX_IVF_OUTPUT);     /* enables output diagnostics */
BDBG_FILE_MODULE(BMUX_IVF_STATE);      /* enables state machine diagnostics */
BDBG_FILE_MODULE(BMUX_IVF_FINISH);     /* enables finish diagnostics */

/****************************
     Static Prototypes
****************************/

/*************************
* P R I V A T E   A P I  *
**************************/

/*
   Function:
      BMUXlib_File_IVF_P_Start

   Initialise any settings necessary before starting the Mux process,
   create any necessary auxiliary tracks based on user settings, and create
   the necessary temporary storage for the metadata.

   NOTE: typically, the only way this fails is due to storage error
   (unable to create the metadata storage)
*/
BERR_Code BMUXlib_File_IVF_P_Start(BMUXlib_File_IVF_Handle hIVFMux)
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BMUXlib_File_IVF_P_Start);

   if (BERR_SUCCESS == rc)
   {
      BMUXlib_Input_CreateSettings stInputSettings;

      /* create the input ... */
      BMUXlib_Input_GetDefaultCreateSettings(&stInputSettings);
      stInputSettings.eType = BMUXlib_Input_Type_eVideo;
      stInputSettings.interface.stVideo = hIVFMux->stStartSettings.stInterface;
      /* NOTE: IVF must operate in frame mode since the frame size is required in the frame header */
      stInputSettings.eBurstMode = BMUXlib_Input_BurstMode_eFrame;
      stInputSettings.bFilterUntilMetadataSeen = true;
      rc = BMUXlib_Input_Create(&hIVFMux->hInput, &stInputSettings);
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
      stOutputSettings.stStorage = hIVFMux->stStartSettings.stOutput;
      rc = BMUXlib_Output_Create(&hIVFMux->hOutput, &stOutputSettings);
   }
   if (BERR_SUCCESS != rc)
   {
      BDBG_ERR(("Unable to Create Output"));
   }

   if (BERR_SUCCESS == rc)
      /* everything OK, so start the mux ... */
      hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eStartup;
   else
      /* else, cleanup anything partially created */
      BMUXlib_File_IVF_P_Stop(hIVFMux);

   BDBG_LEAVE(BMUXlib_File_IVF_P_Start);
   return rc;
}

/*
   Function:
      BMUXlib_File_IVF_P_Stop

   Shut down all muxing and release resources in use (typically
   this involves destroying the temporary storage used for metadata
   etc.).

   Also return context to post-create state.

   NOTE: this can be called at any time if the user wishes
   to perform a "hard" stop (i.e. without calling Finish())
*/
void BMUXlib_File_IVF_P_Stop(BMUXlib_File_IVF_Handle hIVFMux)
{
   BDBG_ENTER(BMUXlib_File_IVF_P_Stop);

   /* destroy output */
   if ( NULL != hIVFMux->hOutput )
   {
      BMUXlib_Output_Destroy( hIVFMux->hOutput );
      hIVFMux->hOutput = NULL;
   }

   /* destroy input */
   if (NULL != hIVFMux->hInput)
   {
      BMUXlib_Input_Destroy(hIVFMux->hInput);
      hIVFMux->hInput = NULL;
   }

   BDBG_LEAVE(BMUXlib_File_IVF_P_Stop);
}

/*
   Function:
      BMUXlib_File_IVF_P_ProcessOutputDescriptorsWaiting

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
BERR_Code BMUXlib_File_IVF_P_ProcessOutputDescriptorsWaiting(BMUXlib_File_IVF_Handle hIVFMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_IVF_P_ProcessOutputDescriptorsWaiting);
   hIVFMux->stStatus.bBlockedOutput= false;

   if (NULL != hIVFMux->hOutput)
   {
      rc = BMUXlib_Output_ProcessNewDescriptors(hIVFMux->hOutput);
      /* if any output did not queue up all descriptors, then we are blocked ... */
      /* NOTE: it is questionable whether blocked due to lack of a resource that
         is output dependent should be treated as "blocked output" also
         (since processing the output will free the blocked resources)
         Perhaps we may need to include all these "blocked" criteria also */
      if (BMUXlib_Output_IsDescriptorPendingQueue(hIVFMux->hOutput))
         hIVFMux->stStatus.bBlockedOutput = true;
   }

   BDBG_LEAVE(BMUXlib_File_IVF_P_ProcessOutputDescriptorsWaiting);
   return rc;
}

/*
   Function:
      BMUXlib_File_IVF_P_ProcessOutputDescriptorsCompleted

   Process completed descriptors returned by the storage interface.  Release
   all resources pointed to by these descriptors (such as box buffer, metadata cache
   input descriptors, etc) and then free the descriptors

   Returns:
      BERR_UNKNOWN -    bad source type detected in descriptor
      + Other errors from storage interface
      + Other errors from input interface(s)
*/
BERR_Code BMUXlib_File_IVF_P_ProcessOutputDescriptorsCompleted(BMUXlib_File_IVF_Handle hIVFMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_IVF_P_ProcessOutputDescriptorsCompleted);

   rc = BMUXlib_Output_ProcessCompletedDescriptors(hIVFMux->hOutput);

   if ( 0 != hIVFMux->uiCompletedCount )
   {
      rc = BMUXlib_Input_ConsumeDescriptors(
               hIVFMux->hInput,
               hIVFMux->uiCompletedCount
               );

      hIVFMux->uiCompletedCount = 0;
   }
   BDBG_LEAVE(BMUXlib_File_IVF_P_ProcessOutputDescriptorsCompleted);
   return rc;
}

/*
   Function:
      BMUXlib_File_IVF_P_ProcessInputDescriptorsWaiting

   Process new input descriptors that are waiting

   Returns:
      Input interface error code
*/
BERR_Code BMUXlib_File_IVF_P_ProcessInputDescriptorsWaiting(BMUXlib_File_IVF_Handle hIVFMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_IVF_P_ProcessInputDescriptorsWaiting);

   if (NULL != hIVFMux->hInput)
   {
      rc = BMUXlib_Input_ProcessNewDescriptors(hIVFMux->hInput);
   }

   BDBG_LEAVE(BMUXlib_File_IVF_P_ProcessInputDescriptorsWaiting);
   return rc;
}

static const unsigned BMUXlib_File_IVF_P_FrameRateLUT[BAVC_FrameRateCode_eMax] =
{
   0,     /* BAVC_FrameRateCode_eUnknown */
   23976, /* BAVC_FrameRateCode_e23_976 */
   24000, /* BAVC_FrameRateCode_e24 */
   25000, /* BAVC_FrameRateCode_e25 */
   29970, /* BAVC_FrameRateCode_e29_97 */
   30000, /* BAVC_FrameRateCode_e30 */
   50000, /* BAVC_FrameRateCode_e50 */
   59940, /* BAVC_FrameRateCode_e59_94 */
   60000, /* BAVC_FrameRateCode_e60 */
   14985, /* BAVC_FrameRateCode_e14_985 */
    7493, /* BAVC_FrameRateCode_e7_493 */
   10000, /* BAVC_FrameRateCode_e10 */
   15000, /* BAVC_FrameRateCode_e15 */
   20000, /* BAVC_FrameRateCode_e20 */
   12500, /* BAVC_FrameRateCode_e12_5 */
  100000, /* BAVC_FrameRateCode_e100 */
  119880, /* BAVC_FrameRateCode_e119_88 */
  120000, /* BAVC_FrameRateCode_e120 */
   19980, /* BAVC_FrameRateCode_e19_98 */
    7500, /* BAVC_FrameRateCode_e7_5 */
   12000, /* BAVC_FrameRateCode_e12 */
   11988, /* BAVC_FrameRateCode_e11_988 */
    9990, /* BAVC_FrameRateCode_e9_99 */

};

void BMUXlib_File_IVF_P_Output_InputDescriptorDone(
   void *pPrivateData,
   const BMUXlib_Output_Descriptor *pOutputDescriptor
   )
{
   BMUXlib_File_IVF_Handle hIVFMux = (BMUXlib_File_IVF_Handle) pPrivateData;

   BDBG_ASSERT( hIVFMux->uiPendingCount );
   BSTD_UNUSED( pOutputDescriptor );

   hIVFMux->uiCompletedCount++;
   hIVFMux->uiPendingCount--;
}

void BMUXlib_File_IVF_P_Output_FrameHeaderDone(
   void *pPrivateData,
   const BMUXlib_Output_Descriptor *pOutputDescriptor
   )
{
   BMUXlib_File_IVF_Handle hIVFMux = (BMUXlib_File_IVF_Handle) pPrivateData;

   BDBG_ASSERT( hIVFMux->stFrameHeader.uiReadOffset != hIVFMux->stFrameHeader.uiWriteOffset );
   BSTD_UNUSED( pOutputDescriptor );

   hIVFMux->stFrameHeader.uiReadOffset++;
   hIVFMux->stFrameHeader.uiReadOffset %= BMUXlib_File_IVF_P_MAX_FRAMES;
}

/*
   Function:
      BMUXlib_File_IVF_P_ProcessInputDescriptors

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
BERR_Code BMUXlib_File_IVF_P_ProcessInputDescriptors(BMUXlib_File_IVF_Handle hIVFMux)
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER(BMUXlib_File_IVF_P_ProcessInputDescriptors);

   rc = BMUXlib_File_IVF_P_ProcessInputDescriptorsWaiting(hIVFMux);
   if (BERR_SUCCESS != rc)
   {
      BDBG_LEAVE(BMUXlib_File_IVF_P_ProcessInputDescriptors);
      return rc;
   }

   BDBG_MODULE_MSG(BMUX_IVF_STATE, ("====Processing Input Descriptors===="));

   /* keep processing as long as there are input descriptors available, and resources to process them ... */
   while ( (BERR_SUCCESS == rc) && BMUXlib_Input_IsDescriptorAvailable( hIVFMux->hInput ) )
   {
      BMUXlib_File_IVF_P_InputState ePreviousInputState = hIVFMux->eInputState;
      BMUXlib_Input_Descriptor stInputDescriptor;
      bool bDescAvail;

      bDescAvail = BMUXlib_Input_PeekAtNextDescriptor(
         hIVFMux->hInput,
         &stInputDescriptor
      );
      if ( !bDescAvail )
      {
         /* NOTE: This should not happen due to IsDescriptorAvailable() check, above */
         rc = BERR_TRACE(BERR_UNKNOWN);
         break;
      }

      switch (hIVFMux->eInputState)
      {
         /* Find the SOF (start of frame) on each stream, and process any initial metadata ...
            NOTE: This is the only state in here that is permitted to consume input descriptors directly */
         case BMUXlib_File_IVF_P_InputState_eStartup:
         {
            BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eStartup --> eGenerateFileHeader"));
            hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eGenerateFileHeader;
         }
         break;
         case BMUXlib_File_IVF_P_InputState_eGenerateFileHeader:
         {
            if ( true == BMUXlib_Output_IsSpaceAvailable( hIVFMux->hOutput ) )
            {
               void *pstVideoMetadataBase;
               BAVC_VideoMetadataDescriptor *pstVideoMetadata;

               BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stInputDescriptor ) );
               BDBG_ASSERT( sizeof( hIVFMux->stFileHeader ) == BMUXlib_File_IVF_P_FILE_HEADER_SIZE );

               /* Generate 32-byte file header */
               BKNI_Memset( hIVFMux->stFileHeader.auiBytes, 0, BMUXlib_File_IVF_P_FILE_HEADER_SIZE );

               pstVideoMetadataBase = BMMA_Lock( BMUXLIB_INPUT_DESCRIPTOR_BLOCK( &stInputDescriptor ) );
               pstVideoMetadata = BMUXLIB_INPUT_DESCRIPTOR_VIRTUAL_ADDRESS( pstVideoMetadataBase, &stInputDescriptor );

               /* Set Signature */
               hIVFMux->stFileHeader.auiBytes[0] = 'D';
               hIVFMux->stFileHeader.auiBytes[1] = 'K';
               hIVFMux->stFileHeader.auiBytes[2] = 'I';
               hIVFMux->stFileHeader.auiBytes[3] = 'F';

               /* Set Header Size */
               BMUXlib_File_IVF_P_Set16_LE( hIVFMux->stFileHeader.auiBytes, BMUXlib_File_IVF_P_FileHeader_HeaderLength_OFFSET , BMUXlib_File_IVF_P_FILE_HEADER_SIZE );

               /* Set Protocol */
               {
                  BAVC_VideoCompressionStd eProtocol = hIVFMux->stStartSettings.stInterface.stBufferInfo.eProtocol;

                  if ( 0 != ( pstVideoMetadata->uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BUFFERINFO_VALID ) )
                  {
                     eProtocol = pstVideoMetadata->stBufferInfo.eProtocol;
                  }

                  switch ( eProtocol )
                  {
                     case BAVC_VideoCompressionStd_eH264:
                        hIVFMux->stFileHeader.auiBytes[8] =  'H';
                        hIVFMux->stFileHeader.auiBytes[9] =  '2';
                        hIVFMux->stFileHeader.auiBytes[10] = '6';
                        hIVFMux->stFileHeader.auiBytes[11] = '4';
                        break;

                     case BAVC_VideoCompressionStd_eVP8:
                        hIVFMux->stFileHeader.auiBytes[8]  = 'V';
                        hIVFMux->stFileHeader.auiBytes[9]  = 'P';
                        hIVFMux->stFileHeader.auiBytes[10] = '8';
                        hIVFMux->stFileHeader.auiBytes[11] = '0';
                        break;

                     case BAVC_VideoCompressionStd_eMPEG2:
                        hIVFMux->stFileHeader.auiBytes[8]  = 'M';
                        hIVFMux->stFileHeader.auiBytes[9]  = 'P';
                        hIVFMux->stFileHeader.auiBytes[10] = '2';
                        hIVFMux->stFileHeader.auiBytes[11] = 'V';
                        break;

                     default:
                        BDBG_ERR(("Video Input %d: Unsupported Video Protocol (%d)", 0, eProtocol));
                        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
                        continue;
                  }
               }

               /* Set Dimensions */
               if ( 0 != ( pstVideoMetadata->uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_DIMENSION_CODED_VALID ) )
               {
                  BMUXlib_File_IVF_P_Set16_LE( &hIVFMux->stFileHeader.auiBytes, BMUXlib_File_IVF_P_FileHeader_Width_OFFSET, pstVideoMetadata->stDimension.coded.uiWidth );
                  BMUXlib_File_IVF_P_Set16_LE( &hIVFMux->stFileHeader.auiBytes, BMUXlib_File_IVF_P_FileHeader_Height_OFFSET, pstVideoMetadata->stDimension.coded.uiHeight );
               }

               /* Set Frame Rate */
               if ( 0 != ( pstVideoMetadata->uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_FRAMERATE_VALID ) )
               {
                  if ( pstVideoMetadata->stFrameRate.eFrameRateCode < BAVC_FrameRateCode_eMax )
                  {
                     BMUXlib_File_IVF_P_Set32_LE( &hIVFMux->stFileHeader.auiBytes, BMUXlib_File_IVF_P_FileHeader_FrameRate_OFFSET, BMUXlib_File_IVF_P_FrameRateLUT[pstVideoMetadata->stFrameRate.eFrameRateCode] );
                  }
               }

               /* Set Time Scale */
               BMUXlib_File_IVF_P_Set32_LE( &hIVFMux->stFileHeader.auiBytes, BMUXlib_File_IVF_P_FileHeader_TimeScale_OFFSET, 1000 );

               /* Queue header to output */
               {
                  BMUXlib_Output_Descriptor stOutputDescriptor;
                  BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

                  BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
                  BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

                  stOutputDescriptor.stStorage.bWriteOperation = true;
                  stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eStart;
                  stOutputDescriptor.stStorage.pBufferAddress = &hIVFMux->stFileHeader.auiBytes;
                  stOutputDescriptor.stStorage.uiLength = BMUXlib_File_IVF_P_FILE_HEADER_SIZE;
                  stOutputDescriptor.stStorage.uiOffset = 0;

                  stCompletedCallbackInfo.pCallbackData = hIVFMux;
                  stCompletedCallbackInfo.pCallback = BMUXlib_File_IVF_P_Output_InputDescriptorDone;

                  rc = BMUXlib_Output_AddNewDescriptor(
                           hIVFMux->hOutput,
                           &stOutputDescriptor,
                           &stCompletedCallbackInfo
                           );

                  if ( BERR_SUCCESS == rc )
                  {
                     hIVFMux->uiPendingCount++;

                     BMUXlib_Input_GetNextDescriptor(
                        hIVFMux->hInput,
                        &stInputDescriptor
                     );

                     BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eGenerateFileHeader --> eGetNextDescriptor"));
                     hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eGetNextDescriptor;
                  }
               }

               BMMA_Unlock( BMUXLIB_INPUT_DESCRIPTOR_BLOCK( &stInputDescriptor ), pstVideoMetadataBase );
            }
         }
         break;

         case BMUXlib_File_IVF_P_InputState_eGetNextDescriptor:
         {
            /* Check if metadata, frame, or eos */
            if ( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stInputDescriptor ) )
            {
               BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eGetNextDescriptor --> eProcessMetadata"));
               hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eProcessMetadata;
            }
            else if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stInputDescriptor ) )
            {
               BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eGetNextDescriptor --> eGenerateFrameHeader"));
               hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eGenerateFrameHeader;
            }
            else
            {
               BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eGetNextDescriptor --> eProcessFrameData"));
               hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eProcessFrameData;
            }
         }
         break;
         case BMUXlib_File_IVF_P_InputState_eProcessMetadata:
         {
            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stInputDescriptor ) );

            /* Print warning and Ignore metadata. */
            BDBG_WRN(("Metadata Descriptor seen in middle of stream. Ignoring..."));

            /* Queue dummy descriptor to output to skip descriptor */
            if ( true == BMUXlib_Output_IsSpaceAvailable( hIVFMux->hOutput ) )
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

               stCompletedCallbackInfo.pCallbackData = hIVFMux;
               stCompletedCallbackInfo.pCallback = BMUXlib_File_IVF_P_Output_InputDescriptorDone;

               rc = BMUXlib_Output_AddNewDescriptor(
                        hIVFMux->hOutput,
                        &stOutputDescriptor,
                        &stCompletedCallbackInfo
                        );

               if ( BERR_SUCCESS == rc )
               {
                  hIVFMux->uiPendingCount++;

                  BMUXlib_Input_GetNextDescriptor(
                     hIVFMux->hInput,
                     &stInputDescriptor
                  );

                  BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eProcessMetadata --> eGetNextDescriptor"));
                  hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eGetNextDescriptor;
               }
            }
         }
         break;

         case BMUXlib_File_IVF_P_InputState_eGenerateFrameHeader:
         {
            unsigned uiWriteOffsetTemp = (hIVFMux->stFrameHeader.uiWriteOffset + 1) % BMUXlib_File_IVF_P_MAX_FRAMES;

            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stInputDescriptor ) );
            BDBG_ASSERT( sizeof( hIVFMux->stFrameHeader.astFrameHeader[0] ) == BMUXlib_File_IVF_P_FRAME_HEADER_SIZE );

            /* Write Frame Header */
            if ( ( true == BMUXlib_Output_IsSpaceAvailable( hIVFMux->hOutput ) )
                 && ( uiWriteOffsetTemp != hIVFMux->stFrameHeader.uiReadOffset ) )
            {
               BMUXlib_File_IVF_P_FrameHeader *pstFrameHeader = &hIVFMux->stFrameHeader.astFrameHeader[hIVFMux->stFrameHeader.uiWriteOffset];

               BKNI_Memset( pstFrameHeader, 0, BMUXlib_File_IVF_P_FRAME_HEADER_SIZE );

               BMUXlib_File_IVF_P_Set32_LE( &pstFrameHeader->auiBytes, BMUXlib_File_IVF_P_FrameHeader_FrameSize_OFFSET, BMUXLIB_INPUT_DESCRIPTOR_FRAMESIZE( &stInputDescriptor ) );

               if ( BMUXLIB_INPUT_DESCRIPTOR_IS_PTS_VALID( &stInputDescriptor ) )
               {
                  BMUXlib_File_IVF_P_Set64_LE( &pstFrameHeader->auiBytes, BMUXlib_File_IVF_P_FrameHeader_PTS_OFFSET, BMUXLIB_INPUT_DESCRIPTOR_PTS( &stInputDescriptor )/90 );
               }

               {
                  BMUXlib_Output_Descriptor stOutputDescriptor;
                  BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

                  BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
                  BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

                  stOutputDescriptor.stStorage.bWriteOperation = true;
                  stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
                  stOutputDescriptor.stStorage.pBufferAddress = pstFrameHeader->auiBytes;
                  stOutputDescriptor.stStorage.uiLength = BMUXlib_File_IVF_P_FRAME_HEADER_SIZE;
                  stOutputDescriptor.stStorage.uiOffset = 0;

                  stCompletedCallbackInfo.pCallbackData = hIVFMux;
                  stCompletedCallbackInfo.pCallback = BMUXlib_File_IVF_P_Output_FrameHeaderDone;

                  rc = BMUXlib_Output_AddNewDescriptor(
                           hIVFMux->hOutput,
                           &stOutputDescriptor,
                           &stCompletedCallbackInfo
                           );

                  if ( BERR_SUCCESS == rc )
                  {
                     hIVFMux->stFrameHeader.uiWriteOffset = uiWriteOffsetTemp;
                     hIVFMux->uiFrameCount++;

                     BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eGenerateFrameHeader --> eProcessFrameData"));
                     hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eProcessFrameData;
                  }
               }
            }
         }
         break;

         case BMUXlib_File_IVF_P_InputState_eProcessFrameData:
         {
            /* Write Data to Disk */
            if ( true == BMUXlib_Output_IsSpaceAvailable( hIVFMux->hOutput ) )
            {
               BMUXlib_Output_Descriptor stOutputDescriptor;
               BMUXlib_Output_CompletedCallbackInfo stCompletedCallbackInfo;

               BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );
               BKNI_Memset( &stCompletedCallbackInfo, 0, sizeof( BMUXlib_Output_CompletedCallbackInfo ) );

               stOutputDescriptor.stStorage.bWriteOperation = true;
               stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eCurrent;
               stOutputDescriptor.stStorage.hBlock = BMUXLIB_INPUT_DESCRIPTOR_BLOCK( &stInputDescriptor );
               stOutputDescriptor.stStorage.uiBlockOffset = BMUXLIB_INPUT_DESCRIPTOR_OFFSET( &stInputDescriptor );
               stOutputDescriptor.stStorage.uiLength = BMUXLIB_INPUT_DESCRIPTOR_LENGTH( &stInputDescriptor );
               stOutputDescriptor.stStorage.uiOffset = 0;

               stCompletedCallbackInfo.pCallbackData = hIVFMux;
               stCompletedCallbackInfo.pCallback = BMUXlib_File_IVF_P_Output_InputDescriptorDone;

               rc = BMUXlib_Output_AddNewDescriptor(
                        hIVFMux->hOutput,
                        &stOutputDescriptor,
                        &stCompletedCallbackInfo
                        );

               if ( BERR_SUCCESS == rc )
               {
                  hIVFMux->uiPendingCount++;

                  if ( BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stInputDescriptor ) )
                  {
                     BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eProcessFrameData --> eProcessEOS"));
                     hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eProcessEOS;
                  }
                  else
                  {
                     BMUXlib_Input_GetNextDescriptor(
                        hIVFMux->hInput,
                        &stInputDescriptor
                     );

                     BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eProcessFrameData --> eGetNextDescriptor"));
                     hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eGetNextDescriptor;
                  }
               }
            }
         }
         break;

         case BMUXlib_File_IVF_P_InputState_eProcessEOS:
         {
            BDBG_ASSERT( BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stInputDescriptor ) );

            /* Finalize Header w/ Frame Count? */
            if ( true == BMUXlib_Output_IsSpaceAvailable( hIVFMux->hOutput ) )
            {
               BMUXlib_Output_Descriptor stOutputDescriptor;

               /* Set Frame Count */
               BMUXlib_File_IVF_P_Set32_LE( &hIVFMux->stFileHeader.auiBytes, BMUXlib_File_IVF_P_FileHeader_FrameCount_OFFSET, hIVFMux->uiFrameCount );

               BKNI_Memset( &stOutputDescriptor, 0, sizeof( BMUXlib_Output_Descriptor ) );

               stOutputDescriptor.stStorage.bWriteOperation = true;
               stOutputDescriptor.stStorage.eOffsetFrom = BMUXlib_Output_OffsetReference_eStart;
               stOutputDescriptor.stStorage.pBufferAddress = &hIVFMux->stFileHeader.auiBytes[BMUXlib_File_IVF_P_FileHeader_FrameCount_OFFSET];
               stOutputDescriptor.stStorage.uiLength = 4;
               stOutputDescriptor.stStorage.uiOffset = BMUXlib_File_IVF_P_FileHeader_FrameCount_OFFSET;

               rc = BMUXlib_Output_AddNewDescriptor(
                        hIVFMux->hOutput,
                        &stOutputDescriptor,
                        NULL
                        );

               if ( rc == BERR_SUCCESS )
               {
                  BDBG_WRN(( "Wrote %d frames", hIVFMux->uiFrameCount ));

                  BMUXlib_Input_GetNextDescriptor(
                     hIVFMux->hInput,
                     &stInputDescriptor
                  );

                  BDBG_MODULE_MSG(BMUX_IVF_STATE, ("eProcessEOS --> eDone"));
                  hIVFMux->eInputState = BMUXlib_File_IVF_P_InputState_eDone;
               }
            }
         }
         break;

         case BMUXlib_File_IVF_P_InputState_eDone:
         {

         }
         break;
         default:
            /* Unknown state! should not happen ... typically indicates memory overwrite
               this is also a catch-all if it somehow gets to "started" state without invoking Start() */
            BDBG_ERR(("ProcessInputDescriptors:: Unexpected State: %d", hIVFMux->eInputState));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            break;
      } /* end: switch input state */

      /* The state hasn't changed, so we're waiting on something */
      if ( ePreviousInputState == hIVFMux->eInputState )
      {
         break;
      }
   } /* end: while not blocked, not done && not error */

   if (BERR_SUCCESS == rc)
      rc = BMUXlib_File_IVF_P_ProcessOutputDescriptorsWaiting(hIVFMux);

   BDBG_LEAVE(BMUXlib_File_IVF_P_ProcessInputDescriptors);
   return rc;
}

bool BMUXlib_File_IVF_P_EOSSeen( BMUXlib_File_IVF_Handle hIVFMux )
{
   bool bEOSSeen = false;

   BDBG_ENTER( BMUXlib_File_IVF_P_EOSSeen );

   bEOSSeen = hIVFMux->eInputState == BMUXlib_File_IVF_P_InputState_eDone;

   BDBG_LEAVE( BMUXlib_File_IVF_P_EOSSeen );

   return bEOSSeen;
}

/*
   Function:
      BMUXlib_File_IVF_P_IsInputProcessingDone

   Predicate to determine if Input processing has completed

   This is only called after Finish() has been invoked.  Thus, this
   will define "done" as all input queues empty, all descriptors returned
   to encoder(s) and all input processing is complete.
*/
bool BMUXlib_File_IVF_P_IsInputProcessingDone(BMUXlib_File_IVF_Handle hIVFMux)
{
   bool bDone = false;

   BDBG_ENTER( BMUXlib_File_IVF_P_IsInputProcessingDone );

   bDone = ( hIVFMux->eInputState == BMUXlib_File_IVF_P_InputState_eDone );

   BDBG_LEAVE( BMUXlib_File_IVF_P_IsInputProcessingDone );

   return bDone;
}

/*
   Function:
      BMUXlib_File_IVF_P_IsOutputProcessingDone

   Predicate to determine if Output processing has completed

   This is only called after input is complete (as defined above)
   This will determine "done" when all output sources have finalised
   output, all output descriptors have been returned and dependent
   data sources released
*/
bool BMUXlib_File_IVF_P_IsOutputProcessingDone(BMUXlib_File_IVF_Handle hIVFMux)
{
   bool bDone = false;

   BDBG_ENTER(BMUXlib_File_IVF_P_IsOutputProcessingDone);
   /* ensure all output descriptors have been returned and all
      dependent data sources have been released ... */
   if ( ( false == BMUXlib_Output_IsDescriptorPendingCompletion(hIVFMux->hOutput) )
        && ( false == BMUXlib_Output_IsDescriptorPendingQueue(hIVFMux->hOutput) ) )
   {
      bDone=true;
   }

   BDBG_LEAVE(BMUXlib_File_IVF_P_IsOutputProcessingDone);
   return bDone;
}

/*****************************************************************************
* EOF
******************************************************************************/
