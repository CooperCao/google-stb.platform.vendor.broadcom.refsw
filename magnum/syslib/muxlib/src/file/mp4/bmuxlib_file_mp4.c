/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"

#include "bmuxlib_file_mp4_priv.h"
#include "bmuxlib_file_mp4_boxes.h"

BDBG_MODULE(BMUXLIB_FILE_MP4);
BDBG_FILE_MODULE(BMUX_MP4_FINISH);
BDBG_FILE_MODULE(BMUX_MP4_MEMORY);
BDBG_FILE_MODULE(BMUX_MP4_USAGE);      /* enables storage usage display */

BDBG_OBJECT_ID(BMUXlib_File_MP4_P_Context);   /* BMUXlib_File_MP4_Handle*/
BDBG_OBJECT_ID(BMUXlib_File_MP4_P_CreateData);

#ifdef BMUXLIB_MP4_P_TEST_MODE
#include <stdio.h>
#endif


/**************************
    Static Definitions
**************************/
static const BMUXlib_File_MP4_CreateSettings stDefaultCreateSettings =
{
   BMUXLIB_FILE_MP4_P_SIGNATURE_CREATESETTINGS,

   BMUXLIB_FILE_MP4_P_MIN_NUM_OUT_STORE_DESC,         /* uiNumOutputStorageDescriptors */
   BMUXLIB_FILE_MP4_P_MIN_NUM_META_ENTRIES_CACHED,    /* uiNumMetadataEntriesCached */
   BMUXLIB_FILE_MP4_P_MIN_BOX_HEAP_SIZE,              /* uiBoxHeapSizeBytes */
   BMUXLIB_FILE_MP4_P_MIN_NUM_SIZE_ENTRIES,           /* uiNumSizeEntries */
   BMUXLIB_FILE_MP4_P_MIN_RELOCATION_BUFFER_SIZE,     /* uiRelocationBufferSizeBytes */
   0,                                                 /* uiMuxId */
};

/* Start Settings are set explicitly in GetDefaultStartSettings() */

static const BMUXlib_File_MP4_FinishSettings stDefaultFinishSettings =
{
   BMUXLIB_FILE_MP4_P_SIGNATURE_FINISHSETTINGS,

   BMUXLIB_FILE_MP4_P_DEFAULT_FINISH_MODE             /* eFinishMode */
};

static const size_t aMetadataCacheEntrySize[BMUXlib_File_MP4_P_MetadataType_eMax] =
{
   sizeof(BMUXlib_File_MP4_DTSDelta),                 /* BMUXlib_File_MP4_P_MetadataType_eStts */
   sizeof(BMUXlib_File_MP4_CTSDTSOffset),             /* BMUXlib_File_MP4_P_MetadataType_eCtts */
   sizeof(BMUXlib_File_MP4_SampleSize),               /* BMUXlib_File_MP4_P_MetadataType_eStsz */
   sizeof(BMUXlib_File_MP4_SampleOffset),             /* BMUXlib_File_MP4_P_MetadataType_eStco */
   sizeof(BMUXlib_File_MP4_RAPSampleNum)              /* BMUXlib_File_MP4_P_MetadataType_eStss */
};

#if BDBG_DEBUG_BUILD
static const char * const DebugCodingTypeTable[] =
{
   "Unknown",        /* BMUXlib_File_MP4_P_CodingType_eUnknown */
   "H.264/AVC",      /* BMUXlib_File_MP4_P_CodingType_eAVC */
   "AAC",            /* BMUXlib_File_MP4_P_CodingType_eMpeg4Audio */
   "MPEG4 Part 2",   /* BMUXlib_File_MP4_P_CodingType_eMpeg4Video */
   "MPEG4 Systems",  /* BMUXlib_File_MP4_P_CodingType_eMpeg4Systems */
   "VC1",            /* BMUXlib_File_MP4_P_CodingType_eVC1 */
   "AVC Params",     /* BMUXlib_File_MP4_P_CodingType_eAVCParams */
   "AC3"             /* BMUXlib_File_MP4_P_CodingType_eAC3 */
};
#endif

/*************************
*   Create/Destroy API   *
*************************/

/*
   Function:
      BMUXlib_File_MP4_GetDefaultCreateSettings

   Description:
      Return the default settings for the Create() API in the
      location pointed to by pCreateSettings.

   Returns:
      None
*/
void BMUXlib_File_MP4_GetDefaultCreateSettings(BMUXlib_File_MP4_CreateSettings *pCreateSettings)
{
   BDBG_ENTER(BMUXlib_File_MP4_GetDefaultCreateSettings);
   BDBG_ASSERT(pCreateSettings != NULL);

   *pCreateSettings = stDefaultCreateSettings;

   BDBG_LEAVE(BMUXlib_File_MP4_GetDefaultCreateSettings);
}

/*
   Function:
      BMUXlib_File_MP4_Create

   Description:
      Create a new "instance" of the MP4 file-based software mux, using the
      settings indicated by the structure pointed to by pCreateSettings.

      Handle to the created instance returned in the location pointed to by phMP4Mux.
      This location will be set to NULL upon failure.

      Mux will be placed in the initial state of "stopped"

   Returns:
      BERR_SUCCESS               - Mux instance successfully created
      BERR_OUT_OF_SYSTEM_MEMORY  - no memory to allocate internal storage
      BERR_INVALID_PARAMETER     - bad Create Setting (minimum space requirements not met)
*/
BERR_Code BMUXlib_File_MP4_Create(BMUXlib_File_MP4_Handle *phMP4Mux, const BMUXlib_File_MP4_CreateSettings *pCreateSettings)
{
   BERR_Code rc = BERR_UNKNOWN;
   BMUXlib_File_MP4_Handle hMux;
   int i;
   BDBG_ENTER(BMUXlib_File_MP4_Create);

   BDBG_ASSERT(pCreateSettings != NULL);
   BDBG_ASSERT(phMP4Mux != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_MP4_P_SIGNATURE_CREATESETTINGS == pCreateSettings->uiSignature);

   BDBG_MSG(("====Creating MP4 Mux===="));

   *phMP4Mux = NULL;                            /* incase create fails */

   /* validate create settings as necessary ... */
   if ((pCreateSettings->uiBoxHeapSizeBytes < BMUXLIB_FILE_MP4_P_MIN_BOX_HEAP_SIZE) ||
       (pCreateSettings->uiNumOutputStorageDescriptors < BMUXLIB_FILE_MP4_P_MIN_NUM_OUT_STORE_DESC) ||
       (pCreateSettings->uiNumMetadataEntriesCached < BMUXLIB_FILE_MP4_P_MIN_NUM_META_ENTRIES_CACHED) ||
       (pCreateSettings->uiNumSizeEntries < BMUXLIB_FILE_MP4_P_MIN_NUM_SIZE_ENTRIES))
       /* it is valid for the relocation buffer size to be zero, if progressive download not used
          - this is verified in Start() */
   {
      BDBG_LEAVE(BMUXlib_File_MP4_Create);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Allocate MP4 context from system memory */
   hMux = (BMUXlib_File_MP4_Handle)BKNI_Malloc(sizeof(struct BMUXlib_File_MP4_P_Context));
   BDBG_MODULE_MSG(BMUX_MP4_MEMORY, ("Context: Allocating %d bytes", (int)sizeof(struct BMUXlib_File_MP4_P_Context)));
   if (NULL != hMux)
   {
      bool bCachesOk = true;
      bool bInputsOk = true;
      /* fill in the "create data" (information about objects created (typically allocated memory) ... */
      BMUXlib_File_MP4_P_CreateData *pCreateData = &hMux->stCreate;
      /* create the objects required by this context, based on user settings ... */
      BKNI_Memset(pCreateData, 0, sizeof(*pCreateData));
      BDBG_OBJECT_SET(pCreateData, BMUXlib_File_MP4_P_CreateData);
      pCreateData->uiMuxId = pCreateSettings->uiMuxId;

      /* number of output descriptors to allocate for non-metadata outputs (Main & Mdat) */
      pCreateData->uiOutDescCount = pCreateSettings->uiNumOutputStorageDescriptors;

      /* create the cache for metadata ... (need one cache for each type of data written)
         NOTE: These cache buffers are large enough for all tracks - the buffers are split up between the tracks */
      pCreateData->uiMetadataCacheEntryCount = pCreateSettings->uiNumMetadataEntriesCached;
      for (i = 0; i < BMUXlib_File_MP4_P_MetadataType_eMax; i++)
      {
         uint32_t uiSize = pCreateData->uiMetadataCacheEntryCount * aMetadataCacheEntrySize[i];
         void *pBuffer = BKNI_Malloc(BMUXLIB_FILE_MP4_P_MAX_TRACKS * uiSize);
         if (pBuffer == NULL)
            bCachesOk = false;
         BDBG_MODULE_MSG(BMUX_MP4_MEMORY, ("Metadata cache [%d]: Allocating %d bytes", i, BMUXLIB_FILE_MP4_P_MAX_TRACKS * uiSize));
         pCreateData->aMetadataCacheBuffer[i].pBuffer = pBuffer;
         pCreateData->aMetadataCacheBuffer[i].uiEntrySize = aMetadataCacheEntrySize[i];
      }

      /* create the "heap" for creation of boxes ...
         (NOTE: This accounts for the unwrap space, etc., such that the specified value is the
                actual usable space) */
      pCreateData->uiBoxBufferSize = pCreateSettings->uiBoxHeapSizeBytes + BMUXLIB_FILE_MP4_P_BOX_BUFFER_RESERVED + 1;
      pCreateData->pBoxBuffer = (uint8_t *)BKNI_Malloc(pCreateData->uiBoxBufferSize * sizeof(uint8_t));
      BDBG_MODULE_MSG(BMUX_MP4_MEMORY, ("Box Buffer: Allocating %d bytes", (int)(pCreateData->uiBoxBufferSize * sizeof(uint8_t))));

      /* create the storage for the sizes used in box creation or NALU sizes, etc ... */
      pCreateData->uiSizeBufferEntryCount = pCreateSettings->uiNumSizeEntries;
      pCreateData->pSizeBuffer = (uint32_t *)BKNI_Malloc(pCreateData->uiSizeBufferEntryCount * sizeof(uint32_t));
      BDBG_MODULE_MSG(BMUX_MP4_MEMORY, ("Size Entry Storage: Allocating %d bytes", (int)(pCreateData->uiSizeBufferEntryCount * sizeof(uint32_t))));

      /* create the storage for the relocation buffer (only used if progressive download support enabled) */
      pCreateData->uiRelocationBufferSize = pCreateSettings->uiRelocationBufferSizeBytes;
      if (pCreateSettings->uiRelocationBufferSizeBytes)
         pCreateData->pRelocationBuffer = (uint8_t *)BKNI_Malloc(pCreateData->uiRelocationBufferSize * sizeof(uint8_t));
      else
         pCreateData->pRelocationBuffer = NULL;
      BDBG_MODULE_MSG(BMUX_MP4_MEMORY, ("Relocation Buffer: Allocating %d bytes", (int)(pCreateData->uiRelocationBufferSize * sizeof(uint8_t))));

      /* NOTE: the release queue free list has enough entries to allow for the worst-case completion of all descriptors from all
         outputs (note: this excludes the metadata outputs, since only metadata goes to these outputs) */
      pCreateData->uiReleaseQFreeCount = BMUXLIB_FILE_MP4_P_NUM_FREELIST_ENTRIES;
      pCreateData->pReleaseQFreeList = (BMUXlib_File_MP4_P_ReleaseQEntry *)BKNI_Malloc(BMUXLIB_FILE_MP4_P_NUM_FREELIST_ENTRIES * sizeof(BMUXlib_File_MP4_P_ReleaseQEntry));
      BDBG_MODULE_MSG(BMUX_MP4_MEMORY, ("Release Q free nodes: Allocating %d bytes", (int)(BMUXLIB_FILE_MP4_P_NUM_FREELIST_ENTRIES * sizeof(BMUXlib_File_MP4_P_ReleaseQEntry))));

      /* the callback data storage has enough entries to allow for one entry for each callback
         that can happen on any output that handles input descriptors, or boxes (i.e. on the
         non-metadata outputs).  */
      pCreateData->uiOutputCBDataFreeCount = BMUXLIB_FILE_MP4_P_NUM_FREELIST_ENTRIES;
      pCreateData->pOutputCBDataFreeList = (BMUXlib_File_MP4_P_OutputCallbackData *)BKNI_Malloc(BMUXLIB_FILE_MP4_P_NUM_FREELIST_ENTRIES * sizeof(BMUXlib_File_MP4_P_OutputCallbackData));
      BDBG_MODULE_MSG(BMUX_MP4_MEMORY, ("Output callback data: Allocating %d bytes", (int)(BMUXLIB_FILE_MP4_P_NUM_FREELIST_ENTRIES * sizeof(BMUXlib_File_MP4_P_OutputCallbackData))));

      /* create the input handles */
      for ( i = 0; i < BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS; i++ )
      {
         BMUXlib_Input_CreateSettings stInputSettings;

         BMUXlib_Input_GetDefaultCreateSettings( &stInputSettings );

         if ( BERR_SUCCESS != BMUXlib_Input_Create( &hMux->stCreate.aInputs[i], &stInputSettings ) )
         {
            bInputsOk = false;
            break;
         }
      }

      /* create input group */
      {
         BMUXlib_InputGroup_CreateSettings stInputGroupCreateSettings;

         BMUXlib_InputGroup_GetDefaultCreateSettings( &stInputGroupCreateSettings );

         stInputGroupCreateSettings.uiMaxInputCount = BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS;

         BMUXlib_InputGroup_Create( &hMux->stCreate.hInputGroup, &stInputGroupCreateSettings );
      }

      /* verify all objects were created successfully ... */
      if ((true == bCachesOk) &&
          (true == bInputsOk) &&
          (NULL != hMux->stCreate.hInputGroup) &&
          (NULL != pCreateData->pBoxBuffer) &&
          (NULL != pCreateData->pSizeBuffer) &&
          (NULL != pCreateData->pReleaseQFreeList) &&
          (NULL != pCreateData->pOutputCBDataFreeList) &&
          ((pCreateSettings->uiRelocationBufferSizeBytes == 0) || (NULL != pCreateData->pRelocationBuffer)))
      {
         /* initialise new context ... */
         BMUXlib_File_MP4_P_InitializeContext(hMux);

         /* Provide new handle to caller */
         *phMP4Mux = hMux;
         rc = BERR_SUCCESS;
      }
      else
      {
         /* unable to allocate required memory */
         /* NOTE: the following is necessary so that Destroy() doesn't assert
            on a bad object (normally set by InitializeContext) */
         BDBG_OBJECT_SET(hMux, BMUXlib_File_MP4_P_Context);
         BMUXlib_File_MP4_Destroy(hMux);                 /* free anything created so far, then free the context */
         BDBG_ERR(("Unable to allocate memory for specified objects"));
         rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      }
   } /* hMux != NULL */
   else
   {
      /* unable to allocate the context */
      BDBG_ERR(("Unable to allocate memory for mux context"));
      rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
   }

   BDBG_LEAVE(BMUXlib_File_MP4_Create);
   return rc;
}

/*
   Function:
      BMUXlib_File_MP4_Destroy

   Description:
      Deallocates heap memory utilized by this mux instance, then frees the
      instance.  The caller should set hMP4Mux to NULL after this API returns.

   Returns:
      None
*/
void BMUXlib_File_MP4_Destroy(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BMUXlib_File_MP4_P_CreateData *pCreateData;
   int i;

   BDBG_ENTER(BMUXlib_File_MP4_Destroy);

   BDBG_MSG(("====Destroying MP4 Mux===="));

   /* the following signifies an attempt to free up something that was either
      a) not created by Create()
      b) has already been destroyed
   */
   BDBG_OBJECT_ASSERT(hMP4Mux, BMUXlib_File_MP4_P_Context);

   /* destroy input group handle */
   if ( NULL != hMP4Mux->stCreate.hInputGroup )
   {
      BMUXlib_InputGroup_Destroy( hMP4Mux->stCreate.hInputGroup );
      hMP4Mux->stCreate.hInputGroup = NULL;
   }

   /* destroy the input handles */
   for ( i = 0; i < BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS; i++ )
   {
      if ( NULL != hMP4Mux->stCreate.aInputs[i] )
      {
         BMUXlib_Input_Destroy( hMP4Mux->stCreate.aInputs[i] );
         hMP4Mux->stCreate.aInputs[i] = NULL;
      }
   }

   pCreateData = &hMP4Mux->stCreate;
   BDBG_OBJECT_ASSERT(pCreateData, BMUXlib_File_MP4_P_CreateData);

   /* free up all dependent objects ... */
   if (NULL != pCreateData->pRelocationBuffer)
   {
      BKNI_Free(pCreateData->pRelocationBuffer);
      pCreateData->pRelocationBuffer = NULL;
   }

   if (NULL != pCreateData->pBoxBuffer)
   {
      BKNI_Free(pCreateData->pBoxBuffer);
      pCreateData->pBoxBuffer = NULL;
   }

   if (NULL != pCreateData->pSizeBuffer)
   {
      BKNI_Free(pCreateData->pSizeBuffer);
      pCreateData->pSizeBuffer = NULL;
   }

   for (i = 0; i < BMUXlib_File_MP4_P_MetadataType_eMax; i++)
   {
      if (NULL != pCreateData->aMetadataCacheBuffer[i].pBuffer)
      {
         BKNI_Free(pCreateData->aMetadataCacheBuffer[i].pBuffer);
         pCreateData->aMetadataCacheBuffer[i].pBuffer = NULL;
      }
   }
   if (NULL != pCreateData->pReleaseQFreeList)
   {
      BKNI_Free(pCreateData->pReleaseQFreeList);
      pCreateData->pReleaseQFreeList = NULL;
   }

   if (NULL != pCreateData->pOutputCBDataFreeList)
   {
      BKNI_Free(pCreateData->pOutputCBDataFreeList);
      pCreateData->pOutputCBDataFreeList = NULL;
   }

   BDBG_OBJECT_DESTROY(pCreateData, BMUXlib_File_MP4_P_CreateData);
   BDBG_OBJECT_DESTROY(hMP4Mux, BMUXlib_File_MP4_P_Context);

   /* free the context ... */
   BKNI_Free(hMP4Mux);

   BDBG_LEAVE(BMUXlib_File_MP4_Destroy);
}

/****************************
*   Start/Finish/Stop API   *
****************************/

/*
   Function:
      BMUXlib_File_MP4_GetDefaultStartSettings

   Description:
      Returns the default settings for the Start() API in the location
      indicated by pStartSettings.

   Returns:
      None
*/
void BMUXlib_File_MP4_GetDefaultStartSettings(BMUXlib_File_MP4_StartSettings *pStartSettings)
{
   int i;
   BDBG_ENTER(BMUXlib_File_MP4_GetDefaultStartSettings);

   BDBG_ASSERT(pStartSettings != NULL);

   /* clear the entire start settings ... */
   /* NOTE: This will ensure the following (to allow for error checking):
            * all function pointers are NULL
            * all context pointers are NULL
            * number of inputs will be zero
            * default track IDs will be zero (invalid)
            * Create Time will be invalid (Midnight, Jan 1, 1904) */
   BKNI_Memset(pStartSettings, 0, sizeof(*pStartSettings));

   /* initialise specific values as needed ... */
   pStartSettings->uiSignature = BMUXLIB_FILE_MP4_P_SIGNATURE_STARTSETTINGS;

   /* ensure invalid protocol to pick up if user did not set these ... */
   for (i = 0; i < BMUXLIB_FILE_MP4_MAX_VIDEO_INPUTS; i++)
      pStartSettings->stVideoInputs[i].stInterface.stBufferInfo.eProtocol = BAVC_VideoCompressionStd_eMax;
   for (i = 0; i < BMUXLIB_FILE_MP4_MAX_AUDIO_INPUTS; i++)
      pStartSettings->stAudioInputs[i].stInterface.stBufferInfo.eProtocol = BAVC_AudioCompressionStd_eMax;

   /* ensure we produce a progressive-download-capable interchange format output */
   pStartSettings->bProgressiveDownloadCompatible = BMUXLIB_FILE_MP4_P_DEFAULT_PDL_SUPPORT;
   pStartSettings->uiExpectedDurationMs = BMUXLIB_FILE_MP4_EXPECTED_DURATION_UNKNOWN;

   BDBG_LEAVE(BMUXlib_File_MP4_GetDefaultStartSettings);
}

/*
   Function:
      BMUXlib_File_MP4_Start

   Description:
      Start the mux ready for processing of incoming data from the encoders
      using the configuration parameters specified in pStartSettings.  This
      call will transition the mux to the "started" state.

      Start settings define the configuration of the mux prior to starting the
      muxing process, and do not change thereafter.
      They also provide information about the encoder that is connected to the
      specific input channel, the output destination, etc.

   Returns:
      BERR_SUCCESS               - mux successfully started
      BERR_INVALID_PARAMETER     - bad start setting or missing create data needed by a start setting
      BERR_NOT_SUPPORTED         - Start() invoked from invalid state (must be invoked
                                   from "stopped" state) or invalid video/audio protocol
*/
BERR_Code BMUXlib_File_MP4_Start(BMUXlib_File_MP4_Handle hMP4Mux, const BMUXlib_File_MP4_StartSettings *pStartSettings)
{
   BERR_Code rc = BERR_UNKNOWN;
   uint32_t uiInputIndex, uiTrackIndex;

   BDBG_ENTER(BMUXlib_File_MP4_Start);

   BDBG_OBJECT_ASSERT(hMP4Mux, BMUXlib_File_MP4_P_Context);
   /* verify that the create data has not been cleared */
   BDBG_OBJECT_ASSERT(&hMP4Mux->stCreate, BMUXlib_File_MP4_P_CreateData);
   BDBG_ASSERT(pStartSettings != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_MP4_P_SIGNATURE_STARTSETTINGS == pStartSettings->uiSignature);

   /* verify the storage system interface has been supplied ... */
   if ((NULL == pStartSettings->stStorage.pfCreateStorageObject) ||
      (NULL == pStartSettings->stStorage.pfDestroyStorageObject) ||
      (NULL == pStartSettings->stStorage.pfGetDefaultStorageSettings) ||
      /* verify the output interface has been supplied ... */
      (NULL == pStartSettings->stOutput.pfAddDescriptors) ||
      (NULL == pStartSettings->stOutput.pfGetCompleteDescriptors) ||
      /* ensure inputs supplied does not exceed capabilities ... */
      (pStartSettings->uiNumVideoInputs > BMUXLIB_FILE_MP4_MAX_VIDEO_INPUTS) ||
      (pStartSettings->uiNumAudioInputs > BMUXLIB_FILE_MP4_MAX_AUDIO_INPUTS) ||
      /* verify at least one input interface supplied */
      (0 == (pStartSettings->uiNumAudioInputs + pStartSettings->uiNumVideoInputs)) ||
      /* if progressive download required, verify relocation buffer has been supplied at Create() */
      ((true == pStartSettings->bProgressiveDownloadCompatible) && (NULL == hMP4Mux->stCreate.pRelocationBuffer)))
   {
      BDBG_LEAVE(BMUXlib_File_MP4_Start);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   BDBG_MSG(("====Starting MP4 Mux===="));

   /* Start() can only be performed from the Stopped state - anything else is an error */
   if (BMUXlib_State_eStopped ==  BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux))
   {
      uint32_t uiNumActiveInputs = 0;
      BMUXlib_Input_Handle hInput;
      BMUXlib_Input_Handle aInputTable[BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS];
      BMUXlib_Input_StartSettings stInputSettings;
      BMUXlib_File_MP4_P_CodingType eCoding = BMUXlib_File_MP4_P_CodingType_eUnknown;

      /* save the storage interface used for the main output ... */
      hMP4Mux->aActiveOutputs[BMUXLIB_FILE_MP4_P_OUTPUT_MAIN].stInterface = pStartSettings->stOutput;
      hMP4Mux->stStorage = pStartSettings->stStorage;
      rc = BERR_SUCCESS;

      /* create inputs */
      for (uiInputIndex = 0; uiInputIndex < pStartSettings->uiNumVideoInputs; uiInputIndex++, uiNumActiveInputs++, hMP4Mux->uiNumTracks++)
      {
         /* there is an active video input */
         BMUXlib_File_MP4_P_Input *pInput = &hMP4Mux->aActiveInputs[uiNumActiveInputs];
         BMUXlib_File_MP4_P_TrackInfo *pTrack = &hMP4Mux->aTracks[hMP4Mux->uiNumTracks];
         const BMUXlib_File_MP4_VideoInput *pVideo = &pStartSettings->stVideoInputs[uiInputIndex];

         /* verify the interface settings */
         if ((NULL == pVideo->stInterface.fGetBufferDescriptors) ||
            (NULL == pVideo->stInterface.fConsumeBufferDescriptors) ||
            (NULL == pVideo->stInterface.fGetBufferStatus) ||
            /* verify track ID is non-zero */
            (0 == pVideo->uiTrackID))
         {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
         }

         /* verify track ID is not already in use... */
         for (uiTrackIndex = 0; uiTrackIndex < hMP4Mux->uiNumTracks; uiTrackIndex++)
            if (hMP4Mux->aTracks[uiTrackIndex].uiTrackID == pVideo->uiTrackID)
               rc = BERR_TRACE(BERR_INVALID_PARAMETER);

         eCoding = BMUXlib_File_MP4_P_GetVideoCodingType(pVideo->stInterface.stBufferInfo.eProtocol);
         if (BMUXlib_File_MP4_P_CodingType_eUnknown == eCoding)
         {
            BDBG_ERR(("Video Input %d: Unsupported Video Protocol (%d)", uiInputIndex, pVideo->stInterface.stBufferInfo.eProtocol));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
         }

         if (BERR_SUCCESS != rc)
            break;

         /* create the input ... */
         hInput = hMP4Mux->stCreate.aInputs[uiNumActiveInputs];
         BMUXlib_Input_GetDefaultStartSettings(&stInputSettings);
         stInputSettings.eType = BMUXlib_Input_Type_eVideo;
         stInputSettings.interface.stVideo = pVideo->stInterface;
         /* NOTE: MP4 can't operate in frame mode since it needs access to the first descriptor of the next
            frame to allow metadata updates */
         stInputSettings.eBurstMode = BMUXlib_Input_BurstMode_eDescriptor;
         stInputSettings.pMetadata = pInput;
         stInputSettings.uiMuxId = hMP4Mux->stCreate.uiMuxId;
         stInputSettings.uiTypeInstance = uiInputIndex;
         rc = BMUXlib_Input_Start(hInput, &stInputSettings);
         if (BERR_SUCCESS != rc)
         {
            BDBG_ERR(("Unable to Start Video Input %d", uiInputIndex));
            break;
         }

         BDBG_MSG(("Assigning Input %d (%p), Video: %d (%s), Track ID: %d, to Track %d (%p)", uiNumActiveInputs,
                  (void *)pInput, pVideo->stInterface.stBufferInfo.eProtocol, DebugCodingTypeTable[eCoding],
                  pVideo->uiTrackID, hMP4Mux->uiNumTracks, (void *)pTrack));

         /* fill in the track information for this input */
         pInput->hInput = hInput;
         pInput->pTrack = pTrack;
         pTrack->pInput = pInput;   /* the input corresponding to this track */
         pTrack->uiTrackID = pVideo->uiTrackID;
         pTrack->eType = BMUXlib_File_MP4_P_TrackType_eVideo;
         pTrack->uiTimescale = BMUXLIB_FILE_MP4_P_TIMESCALE_90KHZ;
         pTrack->eCoding = eCoding;

         /* add this input to the input group */
         aInputTable[uiNumActiveInputs] = hInput;

         if (pTrack->uiTrackID > hMP4Mux->uiLargestTrackID)
            hMP4Mux->uiLargestTrackID = pTrack->uiTrackID;
      }

      for (uiInputIndex = 0; (BERR_SUCCESS == rc) && (uiInputIndex < pStartSettings->uiNumAudioInputs); uiInputIndex++, uiNumActiveInputs++, hMP4Mux->uiNumTracks++)
      {
         /* there is an active audio input */
         BMUXlib_File_MP4_P_Input *pInput = &hMP4Mux->aActiveInputs[uiNumActiveInputs];
         BMUXlib_File_MP4_P_TrackInfo *pTrack = &hMP4Mux->aTracks[hMP4Mux->uiNumTracks];
         const BMUXlib_File_MP4_AudioInput *pAudio = &pStartSettings->stAudioInputs[uiInputIndex];

         /* verify the interface settings */
         if ((NULL == pAudio->stInterface.fGetBufferDescriptors) ||
            (NULL == pAudio->stInterface.fConsumeBufferDescriptors) ||
            (NULL == pAudio->stInterface.fGetBufferStatus) ||
            /* verify track ID is non-zero */
            (0 == pAudio->uiTrackID))
         {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
         }

         /* verify track ID is not already in use... */
         for (uiTrackIndex = 0; uiTrackIndex < hMP4Mux->uiNumTracks; uiTrackIndex++)
            if (hMP4Mux->aTracks[uiTrackIndex].uiTrackID == pAudio->uiTrackID)
               rc = BERR_TRACE(BERR_INVALID_PARAMETER);

         eCoding = BMUXlib_File_MP4_P_GetAudioCodingType(pAudio->stInterface.stBufferInfo.eProtocol);
         if (BMUXlib_File_MP4_P_CodingType_eUnknown == eCoding)
         {
            BDBG_ERR(("Audio Input %d: Unsupported Audio Protocol (%d)", uiInputIndex, pAudio->stInterface.stBufferInfo.eProtocol));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
         }

         if (BERR_SUCCESS != rc)
            break;

         /* create the input ... */
         hInput = hMP4Mux->stCreate.aInputs[uiNumActiveInputs];
         BMUXlib_Input_GetDefaultStartSettings(&stInputSettings);
         stInputSettings.eType = BMUXlib_Input_Type_eAudio;
         stInputSettings.interface.stAudio = pAudio->stInterface;
         /* NOTE: MP4 can't operate in frame mode since it needs access to the first descriptor of the next
            frame to allow metadata updates */
         stInputSettings.eBurstMode = BMUXlib_Input_BurstMode_eDescriptor;
         stInputSettings.pMetadata = pInput;
         stInputSettings.uiMuxId = hMP4Mux->stCreate.uiMuxId;
         stInputSettings.uiTypeInstance = uiInputIndex;
         rc = BMUXlib_Input_Start(hInput, &stInputSettings);
         if (BERR_SUCCESS != rc)
         {
            BDBG_ERR(("Unable to Create Audio Input %d", uiInputIndex));
            break;
         }

         BDBG_MSG(("Assigning Input %d (%p), [Audio: %d (%s)], Track ID: %d, to Track %d (%p)", uiNumActiveInputs, (void *)pInput,
                  pAudio->stInterface.stBufferInfo.eProtocol, DebugCodingTypeTable[eCoding], pAudio->uiTrackID,
                  hMP4Mux->uiNumTracks, (void *)pTrack));

         /* fill in the track information for this input */
         pInput->hInput = hInput;
         pInput->pTrack = pTrack;
         pTrack->pInput = pInput;   /* the input corresponding to this track */
         pTrack->uiTrackID = pAudio->uiTrackID;
         pTrack->eType = BMUXlib_File_MP4_P_TrackType_eAudio;
         /* NOTE: for audio, timescale set based upon metadata received from the encoder */
         pTrack->eCoding = eCoding;

         /* add this input to the input group */
         aInputTable[uiNumActiveInputs] = hInput;

         if (pTrack->uiTrackID > hMP4Mux->uiLargestTrackID)
            hMP4Mux->uiLargestTrackID = pTrack->uiTrackID;
      }

      if (BERR_SUCCESS == rc)
      {
         BMUXlib_InputGroup_StartSettings stInputGroupStartSettings;

         hMP4Mux->uiExpectedDurationMs = pStartSettings->uiExpectedDurationMs;
         hMP4Mux->uiCreateTimeUTC = pStartSettings->uiCreateTimeUTC;
         hMP4Mux->bMoovAtStart = (pStartSettings->bProgressiveDownloadCompatible == true);

         BMUXlib_InputGroup_GetDefaultStartSettings(&stInputGroupStartSettings);
         stInputGroupStartSettings.pInputTable = aInputTable;
         stInputGroupStartSettings.uiInputCount = uiNumActiveInputs;
         rc = BMUXlib_InputGroup_Start(hMP4Mux->stCreate.hInputGroup, &stInputGroupStartSettings);
         if (BERR_SUCCESS == rc)
         {
            BMUXlib_InputGroup_Settings stInputGroupSettings;
            /* configure the input group settings */
            BMUXlib_InputGroup_GetDefaultSettings(&stInputGroupSettings);
            /* MP4 mux needs to wait for all inputs to have a DTS before it can select an input */
            stInputGroupSettings.bWaitForAllInputs = true;
            stInputGroupSettings.fSelector = BMUXlib_InputGroup_DescriptorSelectLowestDTS;
            rc = BMUXlib_InputGroup_SetSettings(hMP4Mux->stCreate.hInputGroup, &stInputGroupSettings);
         }
         if (BERR_SUCCESS == rc)
         {
            /* create necessary tracks, outputs and storage, and start the MP4 mux ... */
            rc = BMUXlib_File_MP4_P_Start(hMP4Mux);
         }

         if (BERR_SUCCESS == rc)
         {
#ifdef BMUXLIB_MP4_P_TEST_MODE
            char filename[50];
#endif
            /* mux is started ... */
            BMUXLIB_FILE_MP4_P_SET_MUX_STATE(hMP4Mux, BMUXlib_State_eStarted);

#ifdef BMUXLIB_MP4_P_TEST_MODE
            /* in test mode, create file for configuration information used by test verification */
            snprintf(filename, sizeof(filename), "BMUXlib_MP4_%2.2d_Config.csv",  hMP4Mux->stCreate.uiMuxId);
            BKNI_Printf("Creating MP4 Mux Config Dump (%s)\n", filename);
            hMP4Mux->fpConfig = fopen(filename, "w");

            if (hMP4Mux->fpConfig != NULL)
            {
               /* output the config information header ... */
               fprintf(hMP4Mux->fpConfig, "num_out_desc, num_metadata_entries, box_heap_size, num_size_entries, reloc_buf_size");

               fprintf(hMP4Mux->fpConfig, ", num_video");
               for (uiInputIndex = 0; uiInputIndex < pStartSettings->uiNumVideoInputs; uiInputIndex++)
               {
                  fprintf(hMP4Mux->fpConfig, ", video%d_track_id, video%d_protocol, video%d_profile, video%d_level", uiInputIndex, uiInputIndex, uiInputIndex, uiInputIndex);
               }

               fprintf(hMP4Mux->fpConfig, ", num_audio");
               for (uiInputIndex = 0; uiInputIndex < pStartSettings->uiNumAudioInputs; uiInputIndex++)
               {
                  fprintf(hMP4Mux->fpConfig, ", audio%d_track_id, audio%d_protocol", uiInputIndex, uiInputIndex);
               }

               fprintf(hMP4Mux->fpConfig, ", pdl_mode, create_time\n");

               /* output the configuration data ... */
               fprintf(hMP4Mux->fpConfig, "%d, %d, %d, %d, %d",
                  hMP4Mux->stCreate.uiOutDescCount,
                  hMP4Mux->stCreate.uiMetadataCacheEntryCount,
                  hMP4Mux->stCreate.uiBoxBufferSize - (BMUXLIB_FILE_MP4_P_BOX_BUFFER_RESERVED + 1),
                  hMP4Mux->stCreate.uiSizeBufferEntryCount,
                  hMP4Mux->stCreate.uiRelocationBufferSize);

               fprintf(hMP4Mux->fpConfig, ", %d", pStartSettings->uiNumVideoInputs);
               for (uiInputIndex = 0; uiInputIndex < pStartSettings->uiNumVideoInputs; uiInputIndex++)
               {
                  const BMUXlib_File_MP4_VideoInput *pVideo = &pStartSettings->stVideoInputs[uiInputIndex];
                  fprintf(hMP4Mux->fpConfig, ", %d, %d, %d, %d",
                     pVideo->uiTrackID,
                     pVideo->stInterface.stBufferInfo.eProtocol,
                     pVideo->stInterface.stBufferInfo.eProfile,
                     pVideo->stInterface.stBufferInfo.eLevel);
               }
               fprintf(hMP4Mux->fpConfig, ", %d", pStartSettings->uiNumAudioInputs);
               for (uiInputIndex = 0; uiInputIndex < pStartSettings->uiNumAudioInputs; uiInputIndex++)
               {
                  const BMUXlib_File_MP4_AudioInput *pAudio = &pStartSettings->stAudioInputs[uiInputIndex];
                  fprintf(hMP4Mux->fpConfig, ", %d, %d",
                     pAudio->uiTrackID,
                     pAudio->stInterface.stBufferInfo.eProtocol);
               }
               fprintf(hMP4Mux->fpConfig, ", %d, 0x%x\n",
                  pStartSettings->bProgressiveDownloadCompatible,
                  pStartSettings->uiCreateTimeUTC);
            }
#endif
         }
      }
      /* else, unable to start mux - return error code */
      if (BERR_SUCCESS != rc)
      {
         /* if mux fails to start, we need to return context to defaults, since the above code modifies the context */
         BMUXlib_File_MP4_P_Stop(hMP4Mux);
         BMUXlib_File_MP4_P_InitializeContext(hMP4Mux);
      }
   }
   else
   {
      /* Start() invoked from an invalid state - error: do nothing */
      BDBG_ERR(("MP4 Mux Start:: Invoked from invalid state: %d",  BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux)));
      rc = BERR_TRACE(BERR_NOT_SUPPORTED);
   }

   BDBG_LEAVE(BMUXlib_File_MP4_Start);
   return rc;
}

/*
   Function:
      BMUXlib_File_MP4_GetDefaultFinishSettings

   Description:
      Returns the default settings for the Finish() API in the location
      indicated by pFinishSettings.

   Returns:
      None
*/
void BMUXlib_File_MP4_GetDefaultFinishSettings(BMUXlib_File_MP4_FinishSettings *pFinishSettings)
{
   BDBG_ENTER(BMUXlib_File_MP4_GetDefaultFinishSettings);
   BDBG_ASSERT(pFinishSettings != NULL);

   *pFinishSettings = stDefaultFinishSettings;

   BDBG_LEAVE(BMUXlib_File_MP4_GetDefaultFinishSettings);
}

/*
   Function:
      BMUXlib_File_MP4_Finish

   Description:
      Request the Mux to finish the muxing process.
      If the finish mode is "prepare for stop", then this call will transition the mux to the
      "finishing_input" state and DoMux will continue to process remaining data and will
      subsequently transition to the "finishing_output" state when it finalizes the file
      (i.e. writes the moov) and finally will transition to the "finished" state when done.
      This is the normal mode of behaviour.

      NOTE: The contents of the final output are NOT valid until the mux reaches the
      "finished" state.

   Returns:
      BERR_SUCCESS            - mux "finishing"
      BERR_INVALID_PARAMETER  - bad finish setting
      BERR_NOT_SUPPORTED      - finish called in invalid state
*/
BERR_Code BMUXlib_File_MP4_Finish(BMUXlib_File_MP4_Handle hMP4Mux, const BMUXlib_File_MP4_FinishSettings *pFinishSettings)
{
   BERR_Code rc = BERR_UNKNOWN;

   BDBG_ENTER(BMUXlib_File_MP4_Finish);

   BDBG_OBJECT_ASSERT(hMP4Mux, BMUXlib_File_MP4_P_Context);
   BDBG_ASSERT(pFinishSettings != NULL);
   BDBG_ASSERT(BMUXLIB_FILE_MP4_P_SIGNATURE_FINISHSETTINGS == pFinishSettings->uiSignature);

   BDBG_MSG(("====Finishing MP4 Mux===="));

   /* Finish() can only be performed from the Started state.
      Finish() from any of the finishing/finished states is a "do nothing"
      Anything else is invalid */
   switch (BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux))
   {
      case BMUXlib_State_eStarted:
         switch (pFinishSettings->eFinishMode)
         {
            case BMUXlib_FinishMode_ePrepareForStop:
               /* mux is now "finishing input" ... */
               BDBG_MODULE_MSG(BMUX_MP4_FINISH, ("eStarted --> eFinishingInput"));
               BMUXLIB_FILE_MP4_P_SET_MUX_STATE(hMP4Mux, BMUXlib_State_eFinishingInput);
               rc = BERR_SUCCESS;
               break;

            default:
               /* unrecognized finish mode - do nothing */
               BDBG_ERR(("Invalid Finish mode supplied: %d", pFinishSettings->eFinishMode));
               rc = BERR_TRACE(BERR_INVALID_PARAMETER);
               break;
         }
         break;
      case BMUXlib_State_eFinishingInput:
      case BMUXlib_State_eFinishingOutput:
      case BMUXlib_State_eFinished:
         /* do nothing if invoked from these states - already finishing! */
         rc = BERR_SUCCESS;
         break;
      default:
         BDBG_ERR(("MP4 Mux Finish:: Invoked from invalid state: %d",  BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux)));
         rc = BERR_TRACE(BERR_NOT_SUPPORTED);
         break;
   }
   BDBG_LEAVE(BMUXlib_File_MP4_Finish);
   return rc;
}

/*
   Function:
      BMUXlib_File_MP4_Stop

   Description:
      Stop the mux, and return the internal state to default values
      (effectively reset everything).  This can be called when the mux is in
      any state.

      For a clean stop, this should only be called after the
      "finished" event occurs after calling BMUXlib_File_MP4_Finish()

      This function may need to be called without BMUXlib_File_MP4_Finish()
      in cases where an abrupt stop is needed.
      If this is done, then DoMux will immediately halt muxing, and
      any remaining data will be discarded (mux will move directly to "stopped" state.
      Under these conditions, the MP4 will not be completed, and will be invalid.
      Immediate Stop implies hardware not available (external storage ejected, for example)
      or some other condition that needs the stop to be performed without delay.

   Returns:
      BERR_SUCCESS   - always successful
*/
BERR_Code BMUXlib_File_MP4_Stop(BMUXlib_File_MP4_Handle hMP4Mux)
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER(BMUXlib_File_MP4_Stop);

   BDBG_OBJECT_ASSERT(hMP4Mux, BMUXlib_File_MP4_P_Context);

   BDBG_MSG(("====Stopping MP4 Mux===="));

   BDBG_MODULE_MSG(BMUX_MP4_USAGE, ("Box (inst./peak): %d/%d bytes, Rel.Q: %d, CB Data: %d, Sizes (max. search): %d (+%d)",
               hMP4Mux->stBoxBuffer.uiMaxInstUsage, hMP4Mux->stBoxBuffer.uiMaxUsage,
               hMP4Mux->uiReleaseQMaxUsage, hMP4Mux->uiCallbackDataMaxUsage,
               hMP4Mux->stSizes.uiMaxUsage, hMP4Mux->stSizes.uiMaxSearchDepth));

   /* release all resources created by Start() and ... */
   BMUXlib_File_MP4_P_Stop(hMP4Mux);

   /* return state to initial values (as if Create() had just been called) ... */
   BMUXlib_File_MP4_P_InitializeContext(hMP4Mux);

#ifdef BMUXLIB_MP4_P_TEST_MODE
   /* close the configuration dump file */
   if (hMP4Mux->fpConfig != NULL)
   {
      fclose(hMP4Mux->fpConfig);
      hMP4Mux->fpConfig = NULL;
   }
#endif

   BDBG_LEAVE(BMUXlib_File_MP4_Stop);
   return rc;
}

/*************************
*    Mux Execute API     *
*************************/
/*
   Events:          |  Mux states:
                    |     Stopped        Started           Finishing_Input   Finishing_Output     Finished
======================================================================================================
   Start            |     Started        Invalid           Invalid           Invalid              Invalid
   Finish           |     Invalid        Finishing_Input   NOP               NOP                  NOP
   Input_Done       |     Invalid        Invalid           Finishing_Output  Invalid              Invalid
   Output_Done      |     Invalid        Invalid           Invalid           Finished             Invalid
   Stop             |     (1)            Stopped           Stopped           Stopped              Stopped

   Invalid Event = Error + No change in state
   NOP = do nothing (no state change)
   (1) A stop in the stopped state simply resets everything
*/
/*
   Function:
      BMUXlib_File_MP4_DoMux

   Description:
      Main processing routine for performing the muxing operation.

      DoMux must not block - once an I/O transaction is scheduled, if there is nothing that
      can be done until the I/O completes then this must return and the domux will not be
      scheduled again until at least one I/O operation is complete (the application can monitor
      the status of the storage system passed into the mux via the hStorage handle to determine
      if I/O is waiting or not for any of the I/O streams opened for this mux)

      Returns the current state of the muxing process in the location pointed to by pStatus

      The muxing is complete when in the "finished" or "stopped" state.  The output is not
      valid unless the "finished" state is reached.
      Note that the encoder can be "unhitched" from the mux once the "finishing_output" state
      is reached (no more input data to process) - this signifies an EOS on all input streams
      has been seen.

      It is expected that mux will be rescheduled once output is not busy (if mux quit due to
      output) or after the indicated execution time (if mux quit due to lack of input data).

   Returns:
      BERR_SUCCESS            - mux run successfully
      BERR_NOT_SUPPORTED      - bad mux state detected
*/
BERR_Code BMUXlib_File_MP4_DoMux(BMUXlib_File_MP4_Handle hMP4Mux, BMUXlib_DoMux_Status *pStatus)
{
   BERR_Code rc = BERR_UNKNOWN;
   bool bFinished;
   uint32_t uiInputIndex;

   BDBG_ENTER(BMUXlib_File_MP4_DoMux);

   BDBG_OBJECT_ASSERT(hMP4Mux, BMUXlib_File_MP4_P_Context);
   BDBG_ASSERT(pStatus != NULL);

   switch (BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux))
   {
      case BMUXlib_State_eStopped:
         /* either not started, or done muxing ... do nothing */
         /* requires a Start() to move state */
         rc = BERR_SUCCESS;
         break;

      case BMUXlib_State_eStarted:
         /* first time thru, create the file header */
         if (false == hMP4Mux->bHeaderDone)
         {
            /* create the ftyp box and the mdat box on the output interface
               no other boxes will be created until after Input Processing is
               done - see below */
            if (BERR_SUCCESS != (rc = BMUXlib_File_MP4_P_ProcessBoxes(hMP4Mux)))
               break;

            if (false == hMP4Mux->bHeaderDone)
            {
               /* if header not created, this needs to quit since no input data
                  can be processed until header is queued */
               break;
            }
         }
         /* Actively muxing incoming data */
         if (BERR_SUCCESS != (rc = BMUXlib_File_MP4_P_ProcessInputDescriptors(hMP4Mux)))
            break;                  /* error, so return this to caller */

         /* check if EOS seen on all active inputs (auto-finish) ...
            if so, we need to transition the mux to finishing_input (as if a Finish() were called) */
         for (bFinished = true, uiInputIndex = 0; uiInputIndex < BMUXLIB_FILE_MP4_P_MAX_ACTIVE_INPUTS; uiInputIndex++)
         {
            BMUXlib_File_MP4_P_Input *pInput = &hMP4Mux->aActiveInputs[uiInputIndex];
            if ((NULL != pInput->hInput) && !pInput->bEOS)
            {
               /* this input is active, but has not seen EOS, so we cannot finish yet */
               bFinished = false;
               break;
            }
         }
         if (bFinished)
         {
            BDBG_MODULE_MSG(BMUX_MP4_FINISH, ("eStarted --> eFinishingInput (Auto)"));
            BMUXLIB_FILE_MP4_P_SET_MUX_STATE(hMP4Mux, BMUXlib_State_eFinishingInput);
         }
         /* requires a Finish() or Stop() or EOS on all inputs to move state (will stay here otherwise, even if inputs "dry up") */
         break;

      case BMUXlib_State_eFinishingInput:
         /* finish processing data from encoders after a Finish() call
            - note: no boxes to create until input is done */
         if (BERR_SUCCESS != (rc = BMUXlib_File_MP4_P_ProcessInputDescriptors(hMP4Mux)))
            break;
         /* ensure that all input data is processed, and when done, move to "finishing output" */
         if (BMUXlib_File_MP4_P_IsInputProcessingDone(hMP4Mux))
         {
            BDBG_MODULE_MSG(BMUX_MP4_FINISH, ("eFinishingInput --> eFinishingOutput"));
            BMUXLIB_FILE_MP4_P_SET_MUX_STATE(hMP4Mux, BMUXlib_State_eFinishingOutput);
         }
         break;

      case BMUXlib_State_eFinishingOutput:
         /* finish processing generated output data, and any finalization of the file */
         /* generate the file moov, and finalize the file (i.e. move mdat if required) */
         if (BERR_SUCCESS != (rc = BMUXlib_File_MP4_P_ProcessBoxes(hMP4Mux)))
            break;

         /* ensure that all outgoing data is processed, and when done move to "finished" */
         if (BMUXlib_File_MP4_P_IsOutputProcessingDone(hMP4Mux))
         {
            BDBG_MODULE_MSG(BMUX_MP4_FINISH, ("eFinishingOutput --> eFinished"));
            BMUXLIB_FILE_MP4_P_SET_MUX_STATE(hMP4Mux, BMUXlib_State_eFinished);
         }
         break;

      case BMUXlib_State_eFinished:
         /* final output completed and is valid */
         /* nothing more to be done - need a Stop() call to move state */
         rc = BERR_SUCCESS;
         break;

      default:
         /* error: invalid state (ideally, user should force a Stop() )*/
         BDBG_ERR(("DoMux:: Invalid State detected: %d",  BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux)));
         rc = BERR_TRACE(BERR_NOT_SUPPORTED);
         break;
   }

   /* SW7425-3684: Update completed duration */
   if ( ( BMUXlib_State_eStopped != BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux) )
        && ( BMUXlib_State_eFinished != BMUXLIB_FILE_MP4_P_GET_MUX_STATE(hMP4Mux) )
        && ( NULL != hMP4Mux->stCreate.hInputGroup )
       )
   {
      BMUXlib_InputGroup_Status stStatus;

      BMUXlib_InputGroup_GetStatus(
         hMP4Mux->stCreate.hInputGroup,
         &stStatus
         );

      hMP4Mux->stStatus.uiCompletedDuration = stStatus.uiDuration;
   }

   *pStatus = hMP4Mux->stStatus;

   BDBG_LEAVE(BMUXlib_File_MP4_DoMux);
   return rc;
}

/*****************************************************************************
* EOF
******************************************************************************/
