/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************/

#include "bstd.h" /* also includes berr, bdbg, etc */
#include "bkni.h"

#include "bmuxlib_input.h"
#include "bmuxlib_debug.h"

#if BMUXLIB_INPUT_P_DUMP_DESC
#include <stdio.h>
#endif

BDBG_MODULE(BMUXLIB_INPUT);
BDBG_FILE_MODULE(BMUXLIB_INPUT_ENABLE);

BDBG_OBJECT_ID(BMUXlib_Input_P_Context);       /* BMUXlib_Input_Handle */
BDBG_OBJECT_ID(BMUXlib_InputGroup_P_Context);  /* BMUXlib_InputGroup_Handle */

/**************/
/* Signatures */
/**************/
#define BMUXLIB_INPUT_P_SIGNATURE_CREATESETTINGS      0x494E5000
#define BMUXLIB_INPUT_P_SIGNATURE_SETTINGS            0x494E5001

#define BMUXLIB_INPUTGROUP_P_SIGNATURE_CREATESETTINGS 0x494E5002
#define BMUXLIB_INPUTGROUP_P_SIGNATURE_SETTINGS       0x494E5003

#define BMUXLIB_INPUTGROUP_P_SCALED_DTS_MIDRANGE      0x80000000
#define BMUXLIB_INPUTGROUP_P_ESCR_MIDRANGE            0x80000000

static const BMUXlib_InputGroup_Settings s_stDefaultInputGroupSettings =
{
   BMUXLIB_INPUTGROUP_P_SIGNATURE_SETTINGS,
   false,                                             /* bWaitForAllInputs */
   BMUXlib_InputGroup_DescriptorSelectLowestDTS       /* fSelector */
};

static const BMUXlib_Input_Settings s_stDefaultInputSettings =
{
   BMUXLIB_INPUT_P_SIGNATURE_SETTINGS,
   true,                                             /* bEnable */
};

#define BMUXLIB_INPUT_P_MAX_DESCRIPTORS 512
#define BMUXLIB_INPUT_P_QUEUE_DEPTH(read, write, size) ( (write >= read) ? (write - read) : ( (size - read ) + write ) )

#define BMUXLIB_INPUT_P_DEFAULT_BURST_LENGTH (64*1024)
#define BMUXLIB_INPUT_P_DEFAULT_BURST_DURATION 700

typedef struct BMUXlib_Input_P_Context
{
   BDBG_OBJECT(BMUXlib_Input_P_Context)
   BMUXlib_Input_CreateSettings stCreateSettings;

   struct
   {
      struct
      {
         union
         {
            BAVC_CompressedBufferDescriptor stCommon;
            BAVC_VideoBufferDescriptor stVideo;
            BAVC_AudioBufferDescriptor stAudio;
         } stDescriptor;

         size_t uiSourceDescriptorCount;
         bool bModified;
         size_t uiFrameSize;
         size_t uiBurstSize;
      } astQueue[BMUXLIB_INPUT_P_MAX_DESCRIPTORS];

      size_t uiReadOffset; /* Keeps track of # of input descriptors that have been fully consumed since ProcessNewDescriptors was called.
                            * uiReadOffset = 0 when ProcessNewDescriptors is called */
      size_t uiPendingOffset;  /* Keeps track of # of input descriptors that have been retrieved by the mux via GetNextDescriptor.
                                * uiReadOffset <= uiPendingOffset <= uiWriteOffset
                                */
      size_t uiWriteOffset; /* Keeps track of the # of input descriptors after calling ProcessNewDescriptors */

      size_t uiStaleDescriptorCount; /* Keeps track of descriptors that went missing from the input between GetBufferDescriptor calls (e.g. input watchdogged) */
   } stDescriptorInfo;

   bool bBufferStatusValid;
   BMUXlib_CompressedBufferStatus stBufferStatus;

   size_t uiCurrentFrameSize;

   struct
   {
      BMUXlib_Input_Status stStatus;
      bool bMetadataSeen;
      BMUXlib_Input_Settings stSettingsPending;
      BMUXlib_Input_Settings stSettingsCurrent;
   } stats;

#if BMUXLIB_INPUT_P_DUMP_DESC
   BMuxlib_Debug_CSV_Handle hDescDumpFile;
   uint32_t *puiDescCount;    /* pointer to count to be used for this input */
   uint32_t uiDescCount;      /* local desc count used if this input does not belong to a group */
#if BMUXLIB_INPUT_P_DUMP_METADATA_DESC
   BMuxlib_Debug_CSV_Handle hMetadataDumpFile;
#endif
   BMuxlib_Debug_CSV_Handle hConfigDumpFile;
#endif

   bool bFirstTime;           /* first time run since Start() */
} BMUXlib_Input_P_Context;

typedef struct BMUXlib_InputGroup_P_InputEntry
{
   BMUXlib_Input_Handle       hInput;           /* the handle of the input for this entry */
   BMUXlib_Input_Descriptor   stDescriptor;     /* the current descriptor available for this input */
   bool                       bActive;          /* this input is active (is considered during input selection) */
} BMUXlib_InputGroup_P_InputEntry;

typedef struct BMUXlib_InputGroup_P_Context
{
   BDBG_OBJECT(BMUXlib_InputGroup_P_Context)

   BMUXlib_InputGroup_Settings stSettings;
   BMUXlib_InputGroup_Status stStatus;

   BMUXlib_InputGroup_P_InputEntry *pInputTable;         /* table of all available inputs in the group */
   uint32_t uiNumInputs;

   BMUXlib_InputGroup_P_InputEntry **pInputSelectTable;  /* table of active input entries for input selection */
   /* NOTE: not all available inputs may be active */
#if BMUXLIB_INPUT_P_DUMP_DESC
   uint32_t uiDescCount;   /* common descriptor count for associating descriptors from different inputs */
#endif
} BMUXlib_InputGroup_P_Context;

/*****************
   Prototypes
*****************/
static void BMUXlib_Input_P_PeekAtDescriptor(BMUXlib_Input_Handle hInput, unsigned int uiDescNum, BMUXlib_Input_Descriptor *pstDescriptor);
static void BMUXlib_InputGroup_P_SortInputs(BMUXlib_InputGroup_P_InputEntry *aData[], uint32_t uiCount, BMUXlib_InputGroup_DescriptorSelector fSelect);
#if BMUXLIB_INPUT_P_DUMP_DESC
static void BMuxlib_Input_P_DumpNewDescriptors(BMUXlib_Input_Handle hInput, unsigned int uiNumOldDescriptors);
#endif

/*****************
   Input API
****************/
void
BMUXlib_Input_GetDefaultCreateSettings(
         BMUXlib_Input_CreateSettings *pstSettings
         )
{
   BDBG_ENTER( BMUXlib_Input_GetDefaultCreateSettings );

   BDBG_ASSERT( pstSettings );

   BKNI_Memset(
            pstSettings,
            0,
            sizeof ( BMUXlib_Input_CreateSettings )
            );

   pstSettings->uiSignature = BMUXLIB_INPUT_P_SIGNATURE_CREATESETTINGS;
   pstSettings->eType = BMUXlib_Input_Type_eVideo;
   pstSettings->eBurstMode = BMUXlib_Input_BurstMode_eDescriptor;
   pstSettings->uiBurstMaxLength = BMUXLIB_INPUT_P_DEFAULT_BURST_LENGTH;
   pstSettings->uiBurstMaxDuration = BMUXLIB_INPUT_P_DEFAULT_BURST_DURATION;
   pstSettings->bFilterUntilMetadataSeen = true;

   BDBG_LEAVE( BMUXlib_Input_GetDefaultCreateSettings );
}

BERR_Code
BMUXlib_Input_Create(
         BMUXlib_Input_Handle *phInput,
         const BMUXlib_Input_CreateSettings *pstSettings
         )
{
   BMUXlib_Input_Handle hInput = NULL;

   BDBG_ENTER( BMUXlib_Input_Create );

   BDBG_ASSERT( phInput );
   BDBG_ASSERT( pstSettings );
   BDBG_ASSERT( BMUXLIB_INPUT_P_SIGNATURE_CREATESETTINGS == pstSettings->uiSignature );

   *phInput = NULL;

   /* verify the settings (as a minimum, we need a type and an interface) ...
      NOTE: it is valid for pMetadata to not be set */
   if (((pstSettings->eType != BMUXlib_Input_Type_eVideo) && (pstSettings->eType != BMUXlib_Input_Type_eAudio))
      || ((pstSettings->eBurstMode != BMUXlib_Input_BurstMode_eDescriptor) && (pstSettings->eBurstMode != BMUXlib_Input_BurstMode_eFrame))
      )
   {
      BDBG_LEAVE( BMUXlib_Input_Create );
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }
   /* verify the interface settings ... */
   switch (pstSettings->eType)
   {
      case BMUXlib_Input_Type_eVideo:
      {
         const BMUXlib_VideoEncoderInterface *pInterface = &pstSettings->interface.stVideo;
         if ((NULL == pInterface->fGetBufferDescriptors) ||
            (NULL == pInterface->fConsumeBufferDescriptors) ||
            (NULL == pInterface->fGetBufferStatus))
         {
            BDBG_LEAVE( BMUXlib_Input_Create );
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }
         break;
      }
      case BMUXlib_Input_Type_eAudio:
      {
         const BMUXlib_AudioEncoderInterface *pInterface = &pstSettings->interface.stAudio;
         if ((NULL == pInterface->fGetBufferDescriptors) ||
            (NULL == pInterface->fConsumeBufferDescriptors) ||
            (NULL == pInterface->fGetBufferStatus))
         {
            BDBG_LEAVE( BMUXlib_Input_Create );
            return BERR_TRACE( BERR_INVALID_PARAMETER );
         }
         break;
      }
      default: /* can't happen */
         break;
   }

   hInput = ( BMUXlib_Input_Handle ) BKNI_Malloc( sizeof( BMUXlib_Input_P_Context ) );

   if ( NULL == hInput )
   {
      BDBG_LEAVE( BMUXlib_Input_Create );
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   BKNI_Memset(
            hInput,
            0,
            sizeof( BMUXlib_Input_P_Context )
            );

   BDBG_OBJECT_SET(hInput, BMUXlib_Input_P_Context);
   hInput->stCreateSettings = *pstSettings;

   if ( 0 == hInput->stCreateSettings.uiBurstMaxLength )
   {
      hInput->stCreateSettings.uiBurstMaxLength = BMUXLIB_INPUT_P_DEFAULT_BURST_LENGTH;
   }

   if ( 0 == hInput->stCreateSettings.uiBurstMaxDuration )
   {
      hInput->stCreateSettings.uiBurstMaxDuration = BMUXLIB_INPUT_P_DEFAULT_BURST_DURATION;
   }

   BMUXlib_Input_GetDefaultSettings( &hInput->stats.stSettingsPending );
   BMUXlib_Input_GetDefaultSettings( &hInput->stats.stSettingsCurrent );

#if BMUXLIB_INPUT_P_DUMP_DESC
   {
      char fname[256];
      sprintf(fname, "BMUXlib_INPUT_DESC_");

      switch ( hInput->stCreateSettings.eType )
      {
         case BMUXlib_Input_Type_eVideo:
            sprintf(fname, "%sVIDEO_", fname);
            break;

         case BMUXlib_Input_Type_eAudio:
            sprintf(fname, "%sAUDIO_", fname);
            break;

         default:
            sprintf(fname, "%sUNKNOWN_", fname);
      }
#if BMUXLIB_INPUT_P_TEST_MODE
      /* when running in test mode, do not name file with handle to the input, since this will change from run-to-run */
      sprintf(fname, "%s%02d_%02d.csv", fname, hInput->stCreateSettings.uiTypeInstance, hInput->stCreateSettings.uiMuxId);
#else
      sprintf(fname, "%s%02d_%02d_%08x.csv", fname, hInput->stCreateSettings.uiTypeInstance, hInput->stCreateSettings.uiMuxId, (unsigned)hInput);
#endif
      BKNI_Printf("Creating Input Descriptor Dump File (%s)\n", fname);

      if (!BMuxlib_Debug_CSVOpen(&hInput->hDescDumpFile, fname))
      {
         BDBG_ERR(("Error Creating Input Descriptor Dump File (%s)", fname));
      }
      else
      {
         BMuxlib_Debug_CSVWrite(hInput->hDescDumpFile, "count,base_addr,frame_size,flags,offset,length,opts_45khz,pts,escr,tpb,shr,stc_snapshot");
         switch ( hInput->stCreateSettings.eType )
         {
            case BMUXlib_Input_Type_eVideo:
               BMuxlib_Debug_CSVWrite( hInput->hDescDumpFile, ",vflags,dts,dut,rai" );
               break;
            case BMUXlib_Input_Type_eAudio:
               BMuxlib_Debug_CSVWrite( hInput->hDescDumpFile, ",raw offset,raw length,dut" );
               break;
            default:
               break;
         }
         BMuxlib_Debug_CSVWrite(hInput->hDescDumpFile, "\n");
      }
      hInput->puiDescCount = &hInput->uiDescCount; /* increment local copy unless this input added to a group */
   }
#if BMUXLIB_INPUT_P_DUMP_METADATA_DESC && !BMUXLIB_INPUT_P_DUMP_FRAME_DESC
   {
      char fname[256];
      sprintf(fname, "BMUXlib_METADATA_DESC_");

      switch ( hInput->stCreateSettings.eType )
      {
         case BMUXlib_Input_Type_eVideo:
            sprintf(fname, "%sVIDEO_", fname);
            break;

         case BMUXlib_Input_Type_eAudio:
            sprintf(fname, "%sAUDIO_", fname);
            break;

         default:
            sprintf(fname, "%sUNKNOWN_", fname);
      }
#if BMUXLIB_INPUT_P_TEST_MODE
      /* when running in test mode, do not name file with handle to the input, since this will change from run-to-run */
      sprintf(fname, "%s%02d_%02d.csv", fname, hInput->stCreateSettings.uiTypeInstance, hInput->stCreateSettings.uiMuxId);
#else
      sprintf(fname, "%s%02d_%02d_%08x.csv", fname, hInput->stCreateSettings.uiTypeInstance, hInput->stCreateSettings.uiMuxId, (unsigned)hInput);
#endif
      BKNI_Printf("Creating Input Metadata Descriptor Dump File (%s)\n", fname);
      if (!BMuxlib_Debug_CSVOpen(&hInput->hMetadataDumpFile, fname))
      {
         BDBG_ERR(("Error Creating Input Metadata Descriptor Dump File (%s)", fname));
      }
      else
      {
         switch ( hInput->stCreateSettings.eType )
         {
            case BMUXlib_Input_Type_eVideo:
               BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "flags, protocol, profile, level, max_bitrate, frame_rate, coded_width, coded_height, "
                  "vc1_seqb_cbr, vc1_seqb_hrd_buffer, vc1_seqc_frmrtq_post_proc, vc1_seqc_butrtq_postproc, vc1_seqc_loop_filt, "
                  "vc1_seqc_multi_res, vc1_seqc_fastuvmc, vc1_seqc_ext_mv, vc1_seqc_dquant, vc1_seqc_vtransform, vc1_seqc_overlap, "
                  "vc1_seqc_sync_mark, vc1_seqc_range_red, vc1_seqc_maxbframes, vc1_seqc_quant, vc1_seqc_finterp, stc_snapshot, chunk_id");
               break;
            case BMUXlib_Input_Type_eAudio:
               BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "flags, max_bitrate, samp_freq, stc_snapshot, chunk_id, protocol, aac_audio_spec_length_bits, aac_audio_spec_data, "
                  "wma_samp_per_block, wma_enc_options, wma_super_block_align, wma_block_align, wma_num_channels");
               break;
            default:
               break;
         }
         BMuxlib_Debug_CSVWrite( hInput->hMetadataDumpFile, "\n" );
      }
   }
#endif /* end: metadata dump */
   {
      char fname[256];
      sprintf(fname, "BMUXlib_INPUT_CONFIG_");

      switch ( hInput->stCreateSettings.eType )
      {
         case BMUXlib_Input_Type_eVideo:
            sprintf(fname, "%sVIDEO_", fname);
            break;

         case BMUXlib_Input_Type_eAudio:
            sprintf(fname, "%sAUDIO_", fname);
            break;

         default:
            sprintf(fname, "%sUNKNOWN_", fname);
      }
#if BMUXLIB_INPUT_P_TEST_MODE
      /* when running in test mode, do not name file with handle to the input, since this will change from run-to-run */
      sprintf(fname, "%s%02d_%02d.csv", fname, hInput->stCreateSettings.uiTypeInstance, hInput->stCreateSettings.uiMuxId);
#else
      sprintf(fname, "%s%02d_%02d_%08x.csv", fname, hInput->stCreateSettings.uiTypeInstance, hInput->stCreateSettings.uiMuxId, (unsigned)hInput);
#endif
      BKNI_Printf("Creating Input Config Dump File (%s)\n", fname);

      if (!BMuxlib_Debug_CSVOpen(&hInput->hConfigDumpFile, fname))
      {
         BDBG_ERR(("Error Creating Input Config Dump File (%s)", fname));
      }
      else
      {
         BMuxlib_Debug_CSVWrite( hInput->hConfigDumpFile, "count,enable\n" );
      }
   }
#endif /* end: descriptor dump */

   hInput->bFirstTime = true;
   *phInput = hInput;

   BDBG_LEAVE( BMUXlib_Input_Create );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BMUXlib_Input_GetCreateSettings(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_CreateSettings *pstSettings
         )
{
   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );

   BDBG_ENTER( BMUXlib_Input_GetCreateSettings );

   *pstSettings = hInput->stCreateSettings;

   BDBG_LEAVE( BMUXlib_Input_GetCreateSettings );
   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_Input_Destroy(
         BMUXlib_Input_Handle hInput
         )
{
   BDBG_ENTER( BMUXlib_Input_Destroy );

   BDBG_OBJECT_ASSERT(hInput, BMUXlib_Input_P_Context);


#if BMUXLIB_INPUT_P_DUMP_DESC
   BMuxlib_Debug_CSVClose(hInput->hDescDumpFile);
   hInput->hDescDumpFile = NULL;

#if BMUXLIB_INPUT_P_DUMP_METADATA_DESC && !BMUXLIB_INPUT_P_DUMP_FRAME_DESC
   BMuxlib_Debug_CSVClose(hInput->hMetadataDumpFile);
   hInput->hMetadataDumpFile = NULL;
#endif

   BMuxlib_Debug_CSVClose(hInput->hConfigDumpFile);
   hInput->hConfigDumpFile = NULL;
#endif

   BDBG_OBJECT_DESTROY(hInput, BMUXlib_Input_P_Context);
   BKNI_Free( hInput );

   BDBG_LEAVE( BMUXlib_Input_Destroy );

   return BERR_TRACE( BERR_SUCCESS );
}

void
BMUXlib_Input_GetDefaultSettings(
         BMUXlib_Input_Settings *pstSettings
         )
{
   BDBG_ASSERT( pstSettings );

   BDBG_ENTER( BMUXlib_Input_GetDefaultSettings );

   BKNI_Memcpy( pstSettings, &s_stDefaultInputSettings, sizeof(BMUXlib_Input_Settings) );

   BDBG_LEAVE( BMUXlib_Input_GetDefaultSettings );
}

BERR_Code
BMUXlib_Input_SetSettings(
         BMUXlib_Input_Handle hInput,
         const BMUXlib_Input_Settings *pstSettings)
{
   BERR_Code rc = BERR_UNKNOWN;
   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );
   BDBG_ASSERT( pstSettings );
   BDBG_ASSERT( BMUXLIB_INPUT_P_SIGNATURE_SETTINGS == pstSettings->uiSignature );

   BDBG_ENTER( BMUXlib_Input_SetSettings );
   hInput->stats.stSettingsPending = *pstSettings;

#if BMUXLIB_INPUT_P_DUMP_DESC
   if ( NULL != hInput->hConfigDumpFile )
   {
      BMuxlib_Debug_CSVWrite(hInput->hConfigDumpFile, "%d,%d\n",
         *hInput->puiDescCount,
         hInput->stats.stSettingsPending.bEnable
         );
   }
#endif

   rc = BERR_SUCCESS;

   BDBG_LEAVE( BMUXlib_Input_SetSettings );
   return rc;
}

BERR_Code
BMUXlib_Input_GetSettings(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Settings *pstSettings)
{
   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );
   BDBG_ASSERT( pstSettings );

   BDBG_ENTER( BMUXlib_Input_GetSettings );

   *pstSettings = hInput->stats.stSettingsPending;

   BDBG_LEAVE( BMUXlib_Input_GetSettings );
   return BERR_SUCCESS;
}

void
BMUXlib_Input_P_AddDescriptors(
   BMUXlib_Input_Handle hInput,
   size_t uiCount
   )
{
   BDBG_ENTER( BMUXlib_Input_P_AddDescriptors );

   hInput->stDescriptorInfo.uiWriteOffset += uiCount;
   hInput->stDescriptorInfo.uiWriteOffset %= BMUXLIB_INPUT_P_MAX_DESCRIPTORS;

   BDBG_LEAVE( BMUXlib_Input_P_AddDescriptors );
}

void
BMUXlib_Input_P_SkipDescriptors(
   BMUXlib_Input_Handle hInput,
   size_t uiCount
   )
{
   unsigned uiIndex;

   BDBG_ENTER( BMUXlib_Input_P_SkipDescriptors );

   BDBG_ASSERT( BMUXLIB_INPUT_P_QUEUE_DEPTH( hInput->stDescriptorInfo.uiPendingOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) >= uiCount );

   for ( uiIndex = hInput->stDescriptorInfo.uiPendingOffset; uiIndex != (hInput->stDescriptorInfo.uiPendingOffset + uiCount) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS; uiIndex = (uiIndex + 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
   {
      hInput->stDescriptorInfo.astQueue[uiIndex].uiSourceDescriptorCount = 0;
      hInput->stDescriptorInfo.astQueue[uiIndex].bModified = false;
   }
   hInput->stDescriptorInfo.uiPendingOffset += uiCount;
   hInput->stDescriptorInfo.uiPendingOffset %= BMUXLIB_INPUT_P_MAX_DESCRIPTORS;

   BDBG_LEAVE( BMUXlib_Input_P_SkipDescriptors );
}

void
BMUXlib_Input_P_RewindDescriptors(
   BMUXlib_Input_Handle hInput,
   size_t uiCount
   )
{
   unsigned uiIndex;

   BDBG_ENTER( BMUXlib_Input_P_RewindDescriptors );

   BDBG_ASSERT( BMUXLIB_INPUT_P_QUEUE_DEPTH( hInput->stDescriptorInfo.uiReadOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) >= uiCount );

   for ( uiIndex = (hInput->stDescriptorInfo.uiWriteOffset - 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS; uiIndex != (hInput->stDescriptorInfo.uiWriteOffset - ( uiCount + 1 ) ) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS; uiIndex = (uiIndex - 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
   {
      BKNI_Memset( &hInput->stDescriptorInfo.astQueue[uiIndex], 0, sizeof( hInput->stDescriptorInfo.astQueue[uiIndex] ) );
   }
   hInput->stDescriptorInfo.uiWriteOffset -= uiCount;
   hInput->stDescriptorInfo.uiWriteOffset %= BMUXLIB_INPUT_P_MAX_DESCRIPTORS;

   BDBG_LEAVE( BMUXlib_Input_P_RewindDescriptors );
}

void
BMUXlib_Input_P_ConsumeDescriptors(
   BMUXlib_Input_Handle hInput,
   size_t uiCount
   )
{
   unsigned uiIndex;

   BDBG_ENTER( BMUXlib_Input_P_ConsumeDescriptors );

   BDBG_ASSERT( BMUXLIB_INPUT_P_QUEUE_DEPTH( hInput->stDescriptorInfo.uiReadOffset, hInput->stDescriptorInfo.uiPendingOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) >= uiCount );

   for ( uiIndex = hInput->stDescriptorInfo.uiReadOffset; uiIndex != (hInput->stDescriptorInfo.uiReadOffset + uiCount) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS; uiIndex = (uiIndex + 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
   {
      BKNI_Memset( &hInput->stDescriptorInfo.astQueue[uiIndex], 0, sizeof( hInput->stDescriptorInfo.astQueue[uiIndex] ) );
   }
   hInput->stDescriptorInfo.uiReadOffset += uiCount;
   hInput->stDescriptorInfo.uiReadOffset %= BMUXLIB_INPUT_P_MAX_DESCRIPTORS;

   BDBG_LEAVE( BMUXlib_Input_P_ConsumeDescriptors );
}

BERR_Code
BMUXlib_Input_ProcessNewDescriptors(
         BMUXlib_Input_Handle hInput
         )
{
   BERR_Code rc = BERR_SUCCESS;
   size_t auiNumDescriptors[2];
   const BAVC_VideoBufferDescriptor *astVideo[2];
   const BAVC_AudioBufferDescriptor *astAudio[2];
   size_t uiPreviousWriteOffset = hInput->stDescriptorInfo.uiWriteOffset;

#if BMUXLIB_INPUT_P_DUMP_DESC
   size_t uiNumOldDescriptors = BMUXLIB_INPUT_P_QUEUE_DEPTH(hInput->stDescriptorInfo.uiReadOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS);
#endif

   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );

   BDBG_ENTER( BMUXlib_Input_ProcessNewDescriptors );

   switch ( hInput->stCreateSettings.eType )
   {
      case BMUXlib_Input_Type_eVideo:
      {
         BMUXlib_VideoEncoderInterface *pInterface = &hInput->stCreateSettings.interface.stVideo;

         /* Get Descriptors */
         rc = BERR_TRACE( pInterface->fGetBufferDescriptors(
                  pInterface->pContext,
                  &astVideo[0],
                  &auiNumDescriptors[0],
                  &astVideo[1],
                  &auiNumDescriptors[1]
                                             ));
      }
      break;

      case BMUXlib_Input_Type_eAudio:
      {
         BMUXlib_AudioEncoderInterface *pInterface = &hInput->stCreateSettings.interface.stAudio;

         /* Get Descriptors */
         rc = BERR_TRACE( pInterface->fGetBufferDescriptors(
                  pInterface->pContext,
                  &astAudio[0],
                  &auiNumDescriptors[0],
                  &astAudio[1],
                  &auiNumDescriptors[1]
                                             ));
      }
      break;

      default:
         /* should not happen - checked for at Create() time */
         BDBG_ERR(("Unknown input type: %d", hInput->stCreateSettings.eType));
         rc = BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Check for error */
   if ( BERR_SUCCESS != rc )
   {
      BDBG_ERR(("INPUT[%02d][%02d]: Error getting buffer descriptors", hInput->stCreateSettings.eType, hInput->stCreateSettings.uiTypeInstance));
   }

   /* SW7425-5205: If we have descriptors in the internal queue, as a sanity check, ensure the first descriptor
    * from the input matches the first descriptor in the internal queue.  If not, we need to reconcile the internal queue.
    * E.g. the input may have watchdogged and/or the input had stale descriptors between stop/start which were flushed
    */
   if ( 0 != BMUXLIB_INPUT_P_QUEUE_DEPTH( hInput->stDescriptorInfo.uiReadOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) )
   {
      void *pDescInternal = NULL, *pDescInput = NULL;
      unsigned uiDescLength = 0;
      unsigned uiReconciledDescriptors = 0;
      unsigned uiReconciledPendingDescriptors = 0;

      /* Reconcile internal queue */
      do
      {
         /* We don't want to reconcile the internal queue to the external queue if the source descriptor count for the current
          * descriptor is not 1.  If this is not 1, it implies we have disabled inputs, so the internal queue and external queue
          * no longer match.
          */
         if ( 1 != hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiReadOffset].uiSourceDescriptorCount ) break;

         if ( true == hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiReadOffset].bModified ) break;

         switch ( hInput->stCreateSettings.eType )
         {
            case BMUXlib_Input_Type_eVideo:
            {
               pDescInternal = (void*) &hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiReadOffset].stDescriptor.stVideo;
               pDescInput = (void*) &astVideo[0][0];
               uiDescLength = sizeof( BAVC_VideoBufferDescriptor );
            }
            break;

            case BMUXlib_Input_Type_eAudio:
            {
               pDescInternal = (void*) &hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiReadOffset].stDescriptor.stAudio;
               pDescInput = (void*) &astAudio[0][0];
               uiDescLength = sizeof( BAVC_AudioBufferDescriptor );
            }
            break;

            default:
               /* should not happen - checked for at Create() time */
               BDBG_ERR(("Unknown input type: %d", hInput->stCreateSettings.eType));
               rc = BERR_TRACE( BERR_INVALID_PARAMETER );
         }

         if ( rc != BERR_SUCCESS ) break;

         if ( pDescInput != NULL )
         {
            if ( 0 == BKNI_Memcmp( pDescInternal, pDescInput, uiDescLength ) ) break;
         }

         if ( hInput->stDescriptorInfo.uiReadOffset == hInput->stDescriptorInfo.uiPendingOffset )
         {
            BMUXlib_Input_P_SkipDescriptors( hInput, 1 );
         }
         else
         {
            uiReconciledPendingDescriptors++;
         }
         uiReconciledDescriptors++;
         BMUXlib_Input_P_ConsumeDescriptors( hInput, 1 );
#if BMUXLIB_INPUT_P_DUMP_DESC
         BDBG_ASSERT( 0 != uiNumOldDescriptors );
         uiNumOldDescriptors--;
#endif
      } while ( ( 0 != BMUXLIB_INPUT_P_QUEUE_DEPTH( hInput->stDescriptorInfo.uiReadOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) ) );

      if ( 0 != uiReconciledDescriptors )
      {
         BDBG_WRN(("INPUT[%02d][%02d]: %d Descriptors went missing from input (watchdog or stop/start occurred?), reconciling internal descriptor queue...",
            hInput->stCreateSettings.eType, hInput->stCreateSettings.uiTypeInstance,
            uiReconciledDescriptors));

         hInput->stDescriptorInfo.uiStaleDescriptorCount += uiReconciledPendingDescriptors;
      }
   }

   if ( BERR_SUCCESS == rc )
   {
      if ( 0 != ( auiNumDescriptors[0] + auiNumDescriptors[1] ) )
      {
         BMUXlib_CompressedBufferStatus stBufferStatus;

         switch ( hInput->stCreateSettings.eType )
         {
            case BMUXlib_Input_Type_eVideo:
            {
               BMUXlib_VideoEncoderInterface *pInterface = &hInput->stCreateSettings.interface.stVideo;

               rc = BERR_TRACE( pInterface->fGetBufferStatus(
                        pInterface->pContext,
                        &stBufferStatus
                        ));

               if ( BERR_SUCCESS != rc)
               {
                  BDBG_ERR(("VIDEO: Error getting buffer status"));
               }
               else
               {
                  if ( true == hInput->bBufferStatusValid )
                  {
                     if ( ( stBufferStatus.hFrameBufferBlock != hInput->stBufferStatus.hFrameBufferBlock )
                          || ( stBufferStatus.hMetadataBufferBlock != hInput->stBufferStatus.hMetadataBufferBlock) )
                     {
                        BDBG_WRN(("Video Input block buffer handles have changed. Did a watchdog occur?"));
                     }
                  }
                  hInput->stBufferStatus = stBufferStatus;
                  hInput->bBufferStatusValid = true;
               }
            }
               break;
            case BMUXlib_Input_Type_eAudio:
            {
               BMUXlib_AudioEncoderInterface *pInterface = &hInput->stCreateSettings.interface.stAudio;

               rc = BERR_TRACE( pInterface->fGetBufferStatus(
                        pInterface->pContext,
                        &stBufferStatus
                        ));

               if ( BERR_SUCCESS != rc)
               {
                  BDBG_ERR(("AUDIO: Error getting buffer status"));
               }
               else
               {
                  if ( true == hInput->bBufferStatusValid )
                  {
                     if ( ( stBufferStatus.hFrameBufferBlock != hInput->stBufferStatus.hFrameBufferBlock )
                          || ( stBufferStatus.hMetadataBufferBlock != hInput->stBufferStatus.hMetadataBufferBlock) )
                     {
                        BDBG_WRN(("Audio Input block buffer handles have changed. Did a watchdog occur?"));
                     }
                  }
                  hInput->stBufferStatus = stBufferStatus;
                  hInput->bBufferStatusValid = true;
               }
            }
               break;

            default:
               /* should not happen - checked for at Create() time */
               BDBG_ERR(("Unknown input type: %d", hInput->stCreateSettings.eType));
               rc = BERR_INVALID_PARAMETER;
         }
      }
   }

   /* Copy the descriptors to internal queue */
   if ( BERR_SUCCESS == rc )
   {
      unsigned i,j,uiOldDescriptors = 0;

      for ( i = hInput->stDescriptorInfo.uiReadOffset; i != hInput->stDescriptorInfo.uiWriteOffset; i = ( i + 1 ) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
      {
         uiOldDescriptors += hInput->stDescriptorInfo.astQueue[i].uiSourceDescriptorCount;
      }

      /* Copy the descriptors to the internal queue */
      for ( i = 0; i < 2; i++ )
      {
         for ( j = 0; j < auiNumDescriptors[i]; j++ )
         {
            /* Skip descriptors we've already copied internally */
            if ( 0 != uiOldDescriptors )
            {
               uiOldDescriptors--;
               continue;
            }

            if ( BMUXLIB_INPUT_P_QUEUE_DEPTH( hInput->stDescriptorInfo.uiReadOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) == ( BMUXLIB_INPUT_P_MAX_DESCRIPTORS - 1 ) )
            {
               /* Our internal queue is full, so continue */
               continue;
            }

            switch ( hInput->stCreateSettings.eType )
            {
               case BMUXlib_Input_Type_eVideo:
               {
                  hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stVideo = astVideo[i][j];
               }
               break;

               case BMUXlib_Input_Type_eAudio:
               {
                  hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stAudio = astAudio[i][j];
               }
               break;

               default:
                  /* should not happen - checked for at Create() time */
                  BDBG_ERR(("Unknown input type: %d", hInput->stCreateSettings.eType));
                  rc = BERR_TRACE( BERR_INVALID_PARAMETER );
            }

            if ( rc == BERR_SUCCESS )
            {
               hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].uiSourceDescriptorCount++;

               /* SW7346-1363: Check to see if we need to enable/disable this input */
               if ( hInput->stats.stSettingsPending.bEnable != hInput->stats.stSettingsCurrent.bEnable )
               {
                  BMUXlib_Input_Descriptor stDescriptor;

                  BMUXlib_Input_P_PeekAtDescriptor(
                           hInput,
                           hInput->stDescriptorInfo.uiWriteOffset,
                           &stDescriptor
                           );

                  /* Only transition on a FRAME_START and RAP */
                  if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stDescriptor )
                       && BMUXLIB_INPUT_DESCRIPTOR_IS_KEYFRAME( &stDescriptor ) )
                  {
                     hInput->stats.stSettingsCurrent.bEnable = hInput->stats.stSettingsPending.bEnable;

                     BDBG_MODULE_MSG( BMUXLIB_INPUT_ENABLE, ("INPUT[%02d][%02d]: Enable %d",
                        hInput->stCreateSettings.eType, hInput->stCreateSettings.uiTypeInstance,
                        hInput->stats.stSettingsCurrent.bEnable
                     ));
                  }
               }

               /* Check to see if we need to filter descriptors */
               if ( false == hInput->stats.stSettingsCurrent.bEnable )
               {
                  BMUXlib_Input_Descriptor stDescriptor;

                  BMUXlib_Input_P_PeekAtDescriptor(
                           hInput,
                           hInput->stDescriptorInfo.uiWriteOffset,
                           &stDescriptor
                           );

                  /* If FRAME_START, we need to change it to an EMPTY_FRAME */
                  if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stDescriptor )
                       || BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stDescriptor )
                       || BMUXLIB_INPUT_DESCRIPTOR_IS_EOC( &stDescriptor )
                       || BMUXLIB_INPUT_DESCRIPTOR_IS_EMPTYFRAME( &stDescriptor ) )

                  {
                     /* Change the FRAME_START descriptor to an EMPTY_FRAME */
                     if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stDescriptor ) )
                     {
                        hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stCommon.uiFlags &= ~BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START;
                        hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stCommon.uiFlags |= BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME;
                        hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stCommon.uiLength = 0;
                        hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].bModified = true;
                        BDBG_MODULE_MSG( BMUXLIB_INPUT_ENABLE, ("INPUT[%02d][%02d]: Converting FRAME_START --> EMPTY_FRAME (ESCR: %08x, Count: %d)",
                           hInput->stCreateSettings.eType, hInput->stCreateSettings.uiTypeInstance,
                           hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stCommon.uiESCR,
                           (int)hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].uiSourceDescriptorCount));
                     }

                     if ( 0 != hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stCommon.uiLength )
                     {
                        hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].stDescriptor.stCommon.uiLength = 0;
                        hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset].bModified = true;
                     }
                  }
                  else
                  {
                     /* We need to ignore this non-FRAME_START descriptor */
                     continue;
                  }
               }

               BMUXlib_Input_P_AddDescriptors( hInput, 1 );
            }
         }
      }
      /* We want to make sure we zero out any non-frame descriptors that we haven't seen a subsequent FRAME_START to avoid counting them twice */
      BKNI_Memset( &hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset], 0, sizeof( hInput->stDescriptorInfo.astQueue[hInput->stDescriptorInfo.uiWriteOffset] ) );
   }

   if ( BERR_SUCCESS == rc )
   {
      /* See if we need to filter descriptors based on the burst mode */
      if ( BMUXlib_Input_BurstMode_eFrame == hInput->stCreateSettings.eBurstMode )
      {
         size_t i;
         size_t uiNumNewDescriptors = 0;
         size_t uiNumDescriptorsInCurrentFrame = 0;
         BMUXlib_Input_Descriptor stDescriptor;
         size_t uiCurrentFrameStartIndex = uiPreviousWriteOffset;
         size_t uiCurrentFrameSize = 0;

         /* Do not include new descriptors until all descriptors for the frame are available.
          * This means that there needs to be a subsequent descriptor with a FRAME_START or EOS.
          */

         for ( i = uiPreviousWriteOffset; i != hInput->stDescriptorInfo.uiWriteOffset; i = (i + 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
         {
            BMUXlib_Input_P_PeekAtDescriptor( hInput, i, &stDescriptor );
            /* the following should not happen */
            BDBG_ASSERT(stDescriptor.descriptor.pstCommon != NULL);

            if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stDescriptor ) )
            {
               uiNumNewDescriptors += uiNumDescriptorsInCurrentFrame;
               uiNumDescriptorsInCurrentFrame = 0;
               hInput->stDescriptorInfo.astQueue[uiCurrentFrameStartIndex].uiFrameSize = uiCurrentFrameSize;
               uiCurrentFrameSize = BMUXLIB_INPUT_DESCRIPTOR_LENGTH ( &stDescriptor );
               uiCurrentFrameStartIndex = i;

               if ( BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stDescriptor )
                    || BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMEEND( &stDescriptor )
                  )
               {
                  hInput->stDescriptorInfo.astQueue[uiCurrentFrameStartIndex].uiFrameSize = uiCurrentFrameSize;
                  uiNumNewDescriptors++; /* If this descriptor is also an EOS or FRAME_END, then this frame is complete */
               }
               else
               {
                  uiNumDescriptorsInCurrentFrame++; /* Include this descriptor as the beginning of the next frame */
               }
            }
            else if ( BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stDescriptor )
                      || BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMEEND( &stDescriptor )
                      || BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stDescriptor )
                    )
            {
               if ( BMUXLIB_INPUT_DESCRIPTOR_IS_EOS( &stDescriptor )
                    || BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMEEND( &stDescriptor )
                  )
               {
                  uiCurrentFrameSize += BMUXLIB_INPUT_DESCRIPTOR_LENGTH ( &stDescriptor );
               }

               uiNumNewDescriptors += uiNumDescriptorsInCurrentFrame;
               uiNumDescriptorsInCurrentFrame = 0;
               hInput->stDescriptorInfo.astQueue[uiCurrentFrameStartIndex].uiFrameSize = uiCurrentFrameSize;
               uiNumNewDescriptors++; /* Include this descriptor as part of this new frame */
            }
            else
            {
               uiCurrentFrameSize += BMUXLIB_INPUT_DESCRIPTOR_LENGTH ( &stDescriptor );
               uiNumDescriptorsInCurrentFrame++;  /* Include this descriptor as part of the next frame */
            }
         }

         /* Adjust num descriptors to ignore incomplete frames */
         BMUXlib_Input_P_RewindDescriptors( hInput, ( BMUXLIB_INPUT_P_QUEUE_DEPTH( uiPreviousWriteOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) - uiNumNewDescriptors ) );

         if ( BMUXlib_Input_Type_eAudio == BMUXLIB_INPUT_DESCRIPTOR_TYPE( &stDescriptor ) )
         {
            /* Calculate bursts for multiple frames in a single PES */
            if ( 0 != uiNumNewDescriptors )
            {
               size_t uiCurrentBurstStartIndex = 0;
               bool bCurrentBurstStartIndexValid = false;
               size_t uiCurrentStartDTS = 0;
               size_t uiCurrentBurstSize = 0;

               for ( i = uiPreviousWriteOffset; i != hInput->stDescriptorInfo.uiWriteOffset; i = (i + 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
               {
                  BMUXlib_Input_P_PeekAtDescriptor( hInput, i, &stDescriptor );
                  /* the following should not happen */

                  BDBG_ASSERT(stDescriptor.descriptor.pstCommon != NULL);
                  if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stDescriptor ) )
                  {
                     if ( false == bCurrentBurstStartIndexValid )
                     {
                        uiCurrentBurstStartIndex = i;
                        uiCurrentStartDTS = BMUXLIB_INPUT_DESCRIPTOR_DTS( &stDescriptor );
                        bCurrentBurstStartIndexValid = true;
                     }

                     if ( ( ( uiCurrentBurstSize + stDescriptor.uiFrameSize ) >= hInput->stCreateSettings.uiBurstMaxLength )
                          || ( ( (uint32_t) ( ( BMUXLIB_INPUT_DESCRIPTOR_DTS( &stDescriptor ) - uiCurrentStartDTS ) / 90 ) ) >= hInput->stCreateSettings.uiBurstMaxDuration ) )
                     {
                        hInput->stDescriptorInfo.astQueue[uiCurrentBurstStartIndex].uiBurstSize = uiCurrentBurstSize;
                        uiCurrentBurstSize = 0;
                        uiCurrentBurstStartIndex = i;
                        uiCurrentStartDTS = BMUXLIB_INPUT_DESCRIPTOR_DTS( &stDescriptor );
                     }

                     uiCurrentBurstSize += stDescriptor.uiFrameSize;
                  }
               }
               if ( true == bCurrentBurstStartIndexValid )
               {
                  hInput->stDescriptorInfo.astQueue[uiCurrentBurstStartIndex].uiBurstSize = uiCurrentBurstSize;
               }
            }
         }
      }

#if BMUXLIB_INPUT_P_DUMP_DESC
      BMuxlib_Input_P_DumpNewDescriptors(hInput, uiNumOldDescriptors);
#endif

      if ( ( true == hInput->stCreateSettings.bFilterUntilMetadataSeen )
           && ( false == hInput->stats.bMetadataSeen ) )
      {
         BMUXlib_Input_Descriptor stDescriptor;
         size_t uiFilterCount = 0;
         bool bMetadataSeen = false;

         /* Filter out "stale" descriptors until the metadata desc is seen */
         while ( true == BMUXlib_Input_PeekAtNextDescriptor( hInput, &stDescriptor ) )
         {
            /* the following should not happen */
            BDBG_ASSERT(stDescriptor.descriptor.pstCommon != NULL);

            if ( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( &stDescriptor ) )
            {
               bMetadataSeen = true;
               break;
            }
            else
            {
               /* SW7425-5529 - we can ignore the return code since we know there
                  is a descriptor available to process from the PeekAtNextDescriptor
                  call above. Cast to void to avoid coverity error */
               (void)BMUXlib_Input_GetNextDescriptor( hInput, &stDescriptor );
               uiFilterCount++;
            }
         }

         if ( 0 != uiFilterCount )
         {
            BDBG_WRN(("INPUT[%02d][%02d]: Filtered %d stale descriptors",
               hInput->stCreateSettings.eType, hInput->stCreateSettings.uiTypeInstance,
               (int)uiFilterCount));
            rc = BMUXlib_Input_ConsumeDescriptors( hInput, uiFilterCount );
            uiFilterCount = 0;
         }

         hInput->stats.bMetadataSeen = bMetadataSeen;
      }
   }

   BDBG_LEAVE( BMUXlib_Input_ProcessNewDescriptors );

   return BERR_TRACE( rc );
}

bool
BMUXlib_Input_IsDescriptorAvailable(
         BMUXlib_Input_Handle hInput
         )
{
   BDBG_ENTER( BMUXlib_Input_IsDescriptorAvailable );

   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );

   BDBG_MSG(("INPUT[%02d][%02d]: Available: %d Pending %d",
      hInput->stCreateSettings.eType, hInput->stCreateSettings.uiTypeInstance,
      (int)hInput->stDescriptorInfo.uiWriteOffset, (int)hInput->stDescriptorInfo.uiPendingOffset));

   BDBG_LEAVE( BMUXlib_Input_IsDescriptorAvailable );

   return ( 0 != BMUXLIB_INPUT_P_QUEUE_DEPTH( hInput->stDescriptorInfo.uiPendingOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) );
}

/* Returns true if a valid descriptor is available in pstDescriptor
   if no more descriptors, then pstDescriptor->descriptor.pstCommon will be NULL and it will return false
*/
bool
BMUXlib_Input_PeekAtNextDescriptor(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Descriptor *pstDescriptor
         )
{
   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );
   BDBG_ASSERT( pstDescriptor );

   BDBG_ENTER( BMUXlib_Input_PeekAtNextDescriptor );

   if ( hInput->stDescriptorInfo.uiWriteOffset == hInput->stDescriptorInfo.uiPendingOffset )
   {
      /* No more descriptors */
      pstDescriptor->descriptor.pstCommon = NULL;
      return false;
   }
   else
   {
      BMUXlib_Input_P_PeekAtDescriptor( hInput, hInput->stDescriptorInfo.uiPendingOffset, pstDescriptor );
      /* the following should not happen since we checked for descriptors available, above */
      BDBG_ASSERT(pstDescriptor->descriptor.pstCommon != NULL);
   }

   BDBG_LEAVE( BMUXlib_Input_PeekAtNextDescriptor );
   return true;   /* descriptor is available in pstDescriptor */
}

bool
BMUXlib_Input_PeekAtDescriptor(
         BMUXlib_Input_Handle hInput,
         unsigned uiIndex,
         BMUXlib_Input_Descriptor *pstDescriptor
         )
{
   uiIndex += hInput->stDescriptorInfo.uiPendingOffset;
   uiIndex %= BMUXLIB_INPUT_P_MAX_DESCRIPTORS;

   if ( BMUXlib_Input_GetDescriptorCount( hInput ) < BMUXLIB_INPUT_P_QUEUE_DEPTH ( uiIndex, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) )
   {
      return false;
   }

   BMUXlib_Input_P_PeekAtDescriptor( hInput, uiIndex, pstDescriptor );
   return true;
}

unsigned
BMUXlib_Input_GetDescriptorCount(
   BMUXlib_Input_Handle hInput
   )
{
   return BMUXLIB_INPUT_P_QUEUE_DEPTH ( hInput->stDescriptorInfo.uiPendingOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS );
}

/* Returns true if a valid descriptor is available in pstDescriptor
   if no more descriptors, then pstDescriptor->descriptor.pstCommon will be NULL and it will return false
*/
bool
BMUXlib_Input_GetNextDescriptor(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Descriptor *pstDescriptor
         )
{
   bool bDescAvail;

   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );
   BDBG_ASSERT( pstDescriptor );

   BDBG_ENTER( BMUXlib_Input_GetNextDescriptor );

   bDescAvail = BMUXlib_Input_PeekAtNextDescriptor( hInput, pstDescriptor );
   if (bDescAvail)
   {
      /* descriptor is available, so return it to the user */
      hInput->stDescriptorInfo.uiPendingOffset++;
      hInput->stDescriptorInfo.uiPendingOffset %= BMUXLIB_INPUT_P_MAX_DESCRIPTORS;
   }

   BDBG_LEAVE( BMUXlib_Input_GetNextDescriptor );
   return bDescAvail;
}

BERR_Code
BMUXlib_Input_ConsumeDescriptors(
         BMUXlib_Input_Handle hInput,
         size_t uiCount
         )
{
   BERR_Code rc = BERR_SUCCESS;
   size_t uiConsumedCount = 0;
   unsigned uiIndex;

   BDBG_ENTER( BMUXlib_Input_ConsumeDescriptors );

   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );

   /* SW7425-5205: There was a mismatch between the pending descriptors and the available descriptors,
    * reduce the released count to match the available descriptors
    */
   if ( hInput->stDescriptorInfo.uiStaleDescriptorCount >= uiCount )
   {
      hInput->stDescriptorInfo.uiStaleDescriptorCount -= uiCount;
      uiCount = 0;
   }
   else
   {
      uiCount -= hInput->stDescriptorInfo.uiStaleDescriptorCount;
      hInput->stDescriptorInfo.uiStaleDescriptorCount = 0;
   }

   for ( uiIndex = hInput->stDescriptorInfo.uiReadOffset; uiIndex != ( hInput->stDescriptorInfo.uiReadOffset + uiCount ) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS; uiIndex = ( uiIndex + 1 ) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
   {
      if ( 0 != hInput->stDescriptorInfo.astQueue[uiIndex].uiSourceDescriptorCount )
      {
         switch ( hInput->stCreateSettings.eType )
         {
            case BMUXlib_Input_Type_eVideo:
            {
               BMUXlib_VideoEncoderInterface *pInterface = &hInput->stCreateSettings.interface.stVideo;

               /* Consume Descriptors */
               rc = BERR_TRACE(pInterface->fConsumeBufferDescriptors(
                        pInterface->pContext,
                        hInput->stDescriptorInfo.astQueue[uiIndex].uiSourceDescriptorCount
                        ));

               /* Check for error */
               if ( BERR_SUCCESS != rc )
               {
                  /* This is warning if immediately after start (flushing after previous unclean stop), error any other time */
                  if (hInput->bFirstTime)
                  {
                     BDBG_WRN(("VIDEO Error consuming buffer descriptors"));
                  }
                  else
                  {
                     BDBG_ERR(("VIDEO Error consuming buffer descriptors"));
                  }
               }
            }
               break;

            case BMUXlib_Input_Type_eAudio:
            {
               BMUXlib_AudioEncoderInterface *pInterface = &hInput->stCreateSettings.interface.stAudio;

               /* Consume Descriptors */
               rc = BERR_TRACE(pInterface->fConsumeBufferDescriptors(
                        pInterface->pContext,
                        hInput->stDescriptorInfo.astQueue[uiIndex].uiSourceDescriptorCount
                        ));

               /* Check for error */
               if ( BERR_SUCCESS != rc )
               {
                  /* This is warning if immediately after start (flushing after previous unclean stop), error any other time */
                  if (hInput->bFirstTime)
                  {
                     BDBG_WRN(("AUDIO Error consuming buffer descriptors"));
                  }
                  else
                  {
                     BDBG_ERR(("AUDIO Error consuming buffer descriptors"));
                  }
               }
            }
               break;

            default:
               BDBG_ERR(("Unknown input type"));
               rc = BERR_TRACE( BERR_UNKNOWN );
         }

         if ( rc == BERR_SUCCESS )
         {
            uiConsumedCount++;
         }
         else
         {
            break;
         }
      }
   }

   if ( ( ( true == hInput->stCreateSettings.bFilterUntilMetadataSeen )
          && ( true == hInput->stats.bMetadataSeen ) )
        || ( false == hInput->stCreateSettings.bFilterUntilMetadataSeen ) )
   {
      if ( uiConsumedCount > 0 )
      {
         unsigned i;
         BMUXlib_Input_Descriptor stDescriptor;

         if ( false == hInput->stats.stStatus.bInitialDTSValid )
         {
            /* look forward thru the consumed descriptors to find the initial DTS
               (i.e. the first DTS encountered) */
            for (i = hInput->stDescriptorInfo.uiReadOffset; i != hInput->stDescriptorInfo.uiReadOffset + uiConsumedCount; i = (i + 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
            {
               BMUXlib_Input_P_PeekAtDescriptor( hInput, i, &stDescriptor );
               /* the following should not happen  */
               BDBG_ASSERT(stDescriptor.descriptor.pstCommon != NULL);

               if ( BMUXLIB_INPUT_DESCRIPTOR_IS_DTS_VALID( &stDescriptor ) )
               {
                  hInput->stats.stStatus.uiInitialDTS = BMUXLIB_INPUT_DESCRIPTOR_DTS(&stDescriptor);
                  hInput->stats.stStatus.bInitialDTSValid = true;
                  break;
               }
            }
         }
         /* look backward thru the consumed descriptors to find the current DTS
            (i.e. the latest consumed DTS) */
         for ( i = ( hInput->stDescriptorInfo.uiReadOffset + uiConsumedCount - 1 ) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS; /* No Conditional */ ; i = (i - 1) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
         {
            BMUXlib_Input_P_PeekAtDescriptor( hInput, i, &stDescriptor );
            /* the following should not happen  */
            BDBG_ASSERT(stDescriptor.descriptor.pstCommon != NULL);

            if ( BMUXLIB_INPUT_DESCRIPTOR_IS_DTS_VALID( &stDescriptor ) )
            {
               hInput->stats.stStatus.uiCurrentDTS = BMUXLIB_INPUT_DESCRIPTOR_DTS(&stDescriptor);
               break;
            }
            if ( i == hInput->stDescriptorInfo.uiReadOffset ) break;
         }
      }
   }

   BMUXlib_Input_P_ConsumeDescriptors( hInput, uiConsumedCount );
   hInput->bFirstTime = false;

   BDBG_LEAVE( BMUXlib_Input_ConsumeDescriptors );

   return BERR_TRACE( rc );
}

BERR_Code
BMUXlib_Input_GetStatus(
         BMUXlib_Input_Handle hInput,
         BMUXlib_Input_Status *pstStatus
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_OBJECT_ASSERT( hInput, BMUXlib_Input_P_Context );
   BDBG_ASSERT( pstStatus );

   BDBG_ENTER( BMUXlib_Input_GetStatus );

   *pstStatus = hInput->stats.stStatus;

   BDBG_LEAVE( BMUXlib_Input_GetStatus );
   return BERR_TRACE( rc );
}

#if BMUXLIB_INPUT_P_DUMP_DESC
void BMUXlib_Input_P_SetCountLocation(BMUXlib_Input_Handle hInput, uint32_t *puiCount)
{
   if (NULL != puiCount)
      hInput->puiDescCount = puiCount;
}
#endif

/****************************************
  Input Group Processing
****************************************/
void
BMUXlib_InputGroup_GetDefaultCreateSettings(
         BMUXlib_InputGroup_CreateSettings *pstSettings
         )
{
   BDBG_ENTER( BMUXlib_InputGroup_GetDefaultCreateSettings );
   BDBG_ASSERT( pstSettings );

   /* this ensures there are no inputs in the group, and we do not wait for inputs */
   BKNI_Memset(pstSettings, 0, sizeof(BMUXlib_InputGroup_CreateSettings));
   pstSettings->uiSignature = BMUXLIB_INPUTGROUP_P_SIGNATURE_CREATESETTINGS;

   BDBG_LEAVE( BMUXlib_InputGroup_GetDefaultCreateSettings );
}

BERR_Code
BMUXlib_InputGroup_Create(
         BMUXlib_InputGroup_Handle *phInputGroup,
         const BMUXlib_InputGroup_CreateSettings *pstSettings
         )
{
   BMUXlib_InputGroup_Handle hInputGroup;
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ASSERT(phInputGroup);
   BDBG_ASSERT(pstSettings);
   BDBG_ASSERT(BMUXLIB_INPUTGROUP_P_SIGNATURE_CREATESETTINGS == pstSettings->uiSignature);

   BDBG_ENTER(BMUXlib_InputGroup_Create);

   *phInputGroup = NULL;

   /* verify the settings ... */
   if ((0 == pstSettings->uiInputCount)
      || (NULL == pstSettings->pInputTable))
   {
      BDBG_LEAVE(BMUXlib_InputGroup_Create);
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   hInputGroup = BKNI_Malloc(sizeof(struct BMUXlib_InputGroup_P_Context));
   if (NULL != hInputGroup)
   {
      uint32_t uiInputIndex;
      BKNI_Memset( hInputGroup, 0, sizeof( struct BMUXlib_InputGroup_P_Context ));
      BDBG_OBJECT_SET(hInputGroup, BMUXlib_InputGroup_P_Context);
      hInputGroup->pInputTable = BKNI_Malloc(sizeof( BMUXlib_InputGroup_P_InputEntry ) * pstSettings->uiInputCount);
      hInputGroup->pInputSelectTable = BKNI_Malloc(sizeof( BMUXlib_InputGroup_P_InputEntry *) * pstSettings->uiInputCount);

      if ((NULL == hInputGroup->pInputTable) ||
          (NULL == hInputGroup->pInputSelectTable))
      {
         BMUXlib_InputGroup_Destroy(hInputGroup);
         BDBG_LEAVE(BMUXlib_InputGroup_Create);
         return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      }
      /* clear the two tables... */
      BKNI_Memset(hInputGroup->pInputTable, 0, sizeof( BMUXlib_InputGroup_P_InputEntry ) * pstSettings->uiInputCount);
      BKNI_Memset(hInputGroup->pInputSelectTable, 0 , sizeof( BMUXlib_InputGroup_P_InputEntry *) * pstSettings->uiInputCount);

      for (uiInputIndex = 0; uiInputIndex < pstSettings->uiInputCount; uiInputIndex++)
      {
         BMUXlib_InputGroup_P_InputEntry *pEntry = &(hInputGroup->pInputTable)[uiInputIndex];
         BMUXlib_Input_Handle hInput = (pstSettings->pInputTable)[uiInputIndex];
         /* verify table contains valid inputs */
         if (NULL == hInput)
         {
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
         }
         BDBG_OBJECT_ASSERT(hInput, BMUXlib_Input_P_Context);
         pEntry->hInput = hInput;
         pEntry->bActive = true; /* initially, all inputs are "active" */
         /* descriptor will be filled in as needed during input selection */
#if BMUXLIB_INPUT_P_DUMP_DESC
         BMUXlib_Input_P_SetCountLocation(hInput, &hInputGroup->uiDescCount);
#endif
      }

      if (BERR_SUCCESS == rc)
      {
         BMUXlib_InputGroup_GetDefaultSettings(&hInputGroup->stSettings);
         hInputGroup->uiNumInputs = pstSettings->uiInputCount;
         /* initially, all inputs are "active" */
         hInputGroup->stStatus.uiNumActiveInputs = pstSettings->uiInputCount;
         *phInputGroup = hInputGroup;
      }
      else
         BMUXlib_InputGroup_Destroy(hInputGroup);
   }
   else
      rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

   BDBG_LEAVE(BMUXlib_InputGroup_Create);
   return rc;
}

void
BMUXlib_InputGroup_Destroy(
         BMUXlib_InputGroup_Handle hInputGroup
         )
{
   /* the following signifies an attempt to free up something that was either
      a) not created by Create()
      b) has already been destroyed
   */
   BDBG_OBJECT_ASSERT(hInputGroup, BMUXlib_InputGroup_P_Context);

   BDBG_ENTER(BMUXlib_Input_DestroyGroup);

   if (NULL != hInputGroup->pInputTable)
      BKNI_Free(hInputGroup->pInputTable);
   if (NULL != hInputGroup->pInputSelectTable)
      BKNI_Free(hInputGroup->pInputSelectTable);
   BDBG_OBJECT_DESTROY(hInputGroup, BMUXlib_InputGroup_P_Context);
   BKNI_Free(hInputGroup);

   BDBG_LEAVE(BMUXlib_Input_DestroyGroup);
}

void
BMUXlib_InputGroup_GetDefaultSettings(
         BMUXlib_InputGroup_Settings *pstSettings
         )
{
   BDBG_ASSERT( pstSettings );

   BDBG_ENTER( BMUXlib_InputGroup_GetDefaultSettings );

   BKNI_Memcpy( pstSettings, &s_stDefaultInputGroupSettings, sizeof(BMUXlib_InputGroup_Settings) );

   BDBG_LEAVE( BMUXlib_InputGroup_GetDefaultSettings );
}

BERR_Code
BMUXlib_InputGroup_SetSettings(
         BMUXlib_InputGroup_Handle hInputGroup,
         const BMUXlib_InputGroup_Settings *pstSettings)
{
   BERR_Code rc = BERR_UNKNOWN;
   BDBG_OBJECT_ASSERT(hInputGroup, BMUXlib_InputGroup_P_Context);
   BDBG_ASSERT( pstSettings );
   BDBG_ASSERT( BMUXLIB_INPUTGROUP_P_SIGNATURE_SETTINGS == pstSettings->uiSignature );

   BDBG_ENTER( BMUXlib_InputGroup_SetSettings );
   if (NULL == pstSettings->fSelector)
   {
      /* one or more settings is invalid ... */
      rc = BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   else
   {
      hInputGroup->stSettings = *pstSettings;
      rc = BERR_SUCCESS;
   }
   BDBG_LEAVE( BMUXlib_InputGroup_SetSettings );
   return rc;
}

BERR_Code
BMUXlib_InputGroup_GetSettings(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_InputGroup_Settings *pstSettings)
{
   BDBG_OBJECT_ASSERT(hInputGroup, BMUXlib_InputGroup_P_Context);
   BDBG_ASSERT( pstSettings );

   BDBG_ENTER( BMUXlib_InputGroup_GetSettings );

   *pstSettings = hInputGroup->stSettings;

   BDBG_LEAVE( BMUXlib_InputGroup_GetSettings );
   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_InputGroup_ActivateInput(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_Input_Handle hInput,
         bool bActive
         )
{
   uint32_t uiInputIndex;
   BMUXlib_InputGroup_P_InputEntry *pEntry = NULL;

   BDBG_OBJECT_ASSERT(hInputGroup, BMUXlib_InputGroup_P_Context);
   BDBG_OBJECT_ASSERT(hInput, BMUXlib_Input_P_Context);

   BDBG_ENTER( BMUXlib_InputGroup_ActivateInput );

   /* find the entry for this input */
   for (uiInputIndex = 0; uiInputIndex < hInputGroup->uiNumInputs; uiInputIndex++)
   {
      pEntry = &(hInputGroup->pInputTable)[uiInputIndex];
      if (pEntry->hInput == hInput)
         break;
   }
   if (uiInputIndex == hInputGroup->uiNumInputs)
   {
      /* invalid input specified ... */
      BDBG_LEAVE( BMUXlib_InputGroup_DisableInput );
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   if (bActive && !pEntry->bActive)
      hInputGroup->stStatus.uiNumActiveInputs++;

   if (!bActive && pEntry->bActive)
      hInputGroup->stStatus.uiNumActiveInputs--;

   pEntry->bActive = bActive;

   BDBG_LEAVE( BMUXlib_InputGroup_ActivateInput );
   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_InputGroup_GetStatus(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_InputGroup_Status *pstStatus
         )
{
   unsigned uiInputNum;
   uint64_t uiSmallestInitialDTS = -1;
   uint64_t uiSmallestCurrentDTS = -1;

   BDBG_OBJECT_ASSERT(hInputGroup, BMUXlib_InputGroup_P_Context);
   BDBG_ASSERT( pstStatus );

   BDBG_ENTER( BMUXlib_InputGroup_GetStatus );

   /* Calculate completed duration */
   for ( uiInputNum = 0; uiInputNum < hInputGroup->uiNumInputs; uiInputNum++ )
   {
      BMUXlib_InputGroup_P_InputEntry *pEntry = &(hInputGroup->pInputTable)[uiInputNum];
      BMUXlib_Input_Handle hInput = pEntry->hInput;
      BMUXlib_Input_Status stInputStatus;

      BDBG_ASSERT( hInput );
      if (!pEntry->bActive)
         /* this input is not active, so skip it ... */
         continue;

      BMUXlib_Input_GetStatus(hInput, &stInputStatus);
      if ( true == stInputStatus.bInitialDTSValid )
      {
         if ( uiSmallestInitialDTS > stInputStatus.uiInitialDTS )
         {
            uiSmallestInitialDTS = stInputStatus.uiInitialDTS;
         }

         if ( uiSmallestCurrentDTS > stInputStatus.uiCurrentDTS )
         {
            uiSmallestCurrentDTS = stInputStatus.uiCurrentDTS;
         }
      }
   }

   if ( hInputGroup->stStatus.uiNumActiveInputs > 0 )
   {
      hInputGroup->stStatus.uiDuration = (uiSmallestCurrentDTS - uiSmallestInitialDTS)/90;
   }

   *pstStatus = hInputGroup->stStatus;

   BDBG_LEAVE( BMUXlib_InputGroup_GetStatus );
   return BERR_SUCCESS;
}

/* NOTE: this function always succeeds.
   The caller should check *phInput to ensure it is non-NULL
   (i.e. an input was selected).
*/
void
BMUXlib_InputGroup_GetNextInput(
         BMUXlib_InputGroup_Handle hInputGroup,
         BMUXlib_Input_Handle *phInput
         )
{
   unsigned uiInputNum;
   uint32_t uiActiveInputCount = 0;

   BDBG_ENTER( BMUXlib_InputGroup_GetNextInput );
   BDBG_OBJECT_ASSERT(hInputGroup, BMUXlib_InputGroup_P_Context);
   BDBG_ASSERT( phInput );

   *phInput = NULL;
   for ( uiInputNum = 0; uiInputNum < hInputGroup->uiNumInputs; uiInputNum++ )
   {
      BMUXlib_InputGroup_P_InputEntry *pEntry = &(hInputGroup->pInputTable)[uiInputNum];
      BMUXlib_Input_Handle hInput = pEntry->hInput;
      BDBG_ASSERT( hInput );
      if (!pEntry->bActive)
         /* this input is not active, so skip it ... */
         continue;

      /* the descriptor for this input is no longer the latest, so check for an update */
      if ( true == BMUXlib_Input_IsDescriptorAvailable( hInput ) )
      {
         bool bDescAvail;
         bDescAvail = BMUXlib_Input_PeekAtNextDescriptor( hInput, &pEntry->stDescriptor );
         /* there should always be a descriptor available, since we checked above */
         BDBG_ASSERT(bDescAvail == true);
         (hInputGroup->pInputSelectTable)[uiActiveInputCount++] = pEntry;
      }
      else
      {
         /* no descriptor available, so invalidate the descriptor in the entry */
         if ( true == hInputGroup->stSettings.bWaitForAllInputs )
         {
            BDBG_MSG(( "INPUT[%02d][%02d]: is dry",
               hInput->stCreateSettings.eType, hInput->stCreateSettings.uiTypeInstance
               ));
            BDBG_LEAVE( BMUXlib_InputGroup_GetNextInput );
            return;       /* wait until all inputs have descriptors available */
         }
      }
   }  /* end: for each input */
   if (uiActiveInputCount > 0)
   {
      /* sort the input entry array, based on the selector function supplied */
      BMUXlib_InputGroup_P_SortInputs(hInputGroup->pInputSelectTable, uiActiveInputCount, hInputGroup->stSettings.fSelector);
      *phInput = (hInputGroup->pInputSelectTable)[0]->hInput;        /* select the first input in the sorted array */
   }

   BDBG_LEAVE( BMUXlib_InputGroup_GetNextInput );
}

#define BMUXLIB_INPUTGROUP_P_SKEW_DETECTION_THRESHOLD 15
BMUXlib_InputGroup_SelectionResult
BMUXlib_InputGroup_DescriptorSelectLowestDTS (
   BMUXlib_Input_Descriptor *pstDescriptorA,
   BMUXlib_Input_Descriptor *pstDescriptorB
   )
{
   BDBG_ENTER( BMUXlib_InputGroup_DescriptorSelectLowestDTS );

   BDBG_ASSERT( pstDescriptorA );
   BDBG_ASSERT( pstDescriptorB );

   /* NOTE: This uses a scaled DTS to restrict the range to 32-bits
      (to avoid the need for 64-bit math and 64-bit constants).
      This therefore makes the assumption that two DTS values are not separated
      by only their LSB (very unlikely). */
   {
      uint32_t uiDTSA = BMUXLIB_INPUT_DESCRIPTOR_DTS( pstDescriptorA ) >> 1;
      uint32_t uiDTSB = BMUXLIB_INPUT_DESCRIPTOR_DTS( pstDescriptorB ) >> 1;

#if BDBG_DEBUG_BUILD && 0
      if ( uiDTSB != uiDTSA )
      {
         unsigned uiDeltaDTS;
         if ( uiDTSB < uiDTSA )
         {
            uiDeltaDTS = uiDTSA - uiDTSB;
         }
         else
         {
            uiDeltaDTS = uiDTSB - uiDTSA;
         }

         if ( ( uiDeltaDTS < ( 0xFFFFFFFF - ( BMUXLIB_INPUTGROUP_P_SKEW_DETECTION_THRESHOLD*45000 - 1 ) ) )
            && ( ( uiDeltaDTS / 45000 ) > BMUXLIB_INPUTGROUP_P_SKEW_DETECTION_THRESHOLD ) )
         {
            BDBG_WRN(("Input DTS values are more than %d seconds apart! (%d:%08x vs %d:%08x)", BMUXLIB_INPUTGROUP_P_SKEW_DETECTION_THRESHOLD, uiDTSB, uiDTSB, uiDTSA, uiDTSA ));
         }
      }
#endif
      if ( ( ( uiDTSA < uiDTSB ) && ( ( uiDTSB - uiDTSA ) < BMUXLIB_INPUTGROUP_P_SCALED_DTS_MIDRANGE ) )
              || ( ( uiDTSA > uiDTSB ) && ( ( uiDTSA - uiDTSB ) >= BMUXLIB_INPUTGROUP_P_SCALED_DTS_MIDRANGE ) )
              || ( false == BMUXLIB_INPUT_DESCRIPTOR_IS_DTS_VALID( pstDescriptorA ) )
         )
      {
         /* A.DTS is < B.DTS */
         BDBG_LEAVE( BMUXlib_InputGroup_DescriptorSelectLowestDTS );
         return BMUXlib_InputGroup_SelectionResult_eSelectA;
      }
      else
      {
         BDBG_LEAVE( BMUXlib_InputGroup_DescriptorSelectLowestDTS );
         return BMUXlib_InputGroup_SelectionResult_eSelectB;
      }
   }
}

BMUXlib_InputGroup_SelectionResult
BMUXlib_InputGroup_DescriptorSelectLowestESCR (
   BMUXlib_Input_Descriptor *pstDescriptorA,
   BMUXlib_Input_Descriptor *pstDescriptorB
   )
{
   BMUXlib_InputGroup_SelectionResult eResult;
   BDBG_ENTER( BMUXlib_InputGroup_DescriptorSelectLowestESCR );

   BDBG_ASSERT( pstDescriptorA );
   BDBG_ASSERT( pstDescriptorB );

   if ( ( BMUXLIB_INPUT_DESCRIPTOR_IS_ESCR_VALID( pstDescriptorA ) )
        && ( BMUXLIB_INPUT_DESCRIPTOR_IS_ESCR_VALID( pstDescriptorB ) ) )
   {
      /* scaled ESCR values ... */
      uint32_t uiESCRA = BMUXLIB_INPUT_DESCRIPTOR_ESCR( pstDescriptorA );
      uint32_t uiESCRB = BMUXLIB_INPUT_DESCRIPTOR_ESCR( pstDescriptorB );
      int32_t iDeltaESCR = uiESCRA - uiESCRB;
#if BDBG_DEBUG_BUILD && 0
      {
         unsigned uiAbsDeltaESCR = iDeltaESCR;
         if (iDeltaESCR < 0)
            uiAbsDeltaESCR = -iDeltaESCR;
         if (uiAbsDeltaESCR > BMUXLIB_INPUTGROUP_P_SKEW_DETECTION_THRESHOLD * 27000000)
         {
            BDBG_WRN(("Input ESCR values are more than %d seconds apart! [%s]:%08x vs [%s]:%08x (delta:%x)",
               BMUXLIB_INPUTGROUP_P_SKEW_DETECTION_THRESHOLD,
               (BMUXlib_Input_Type_eVideo == pstDescriptorB->eType)?"Video":"Audio", uiESCRB,
               (BMUXlib_Input_Type_eVideo == pstDescriptorA->eType)?"Video":"Audio", uiESCRA,
               uiAbsDeltaESCR));
         }
      }
#endif /* End: Debug */

      if (iDeltaESCR < 0)
      {
         /*  A.ESCR < B.ESCR - i.e. A is the selected input */
         eResult = BMUXlib_InputGroup_SelectionResult_eSelectA;
      }
      else
      {
         eResult = BMUXlib_InputGroup_SelectionResult_eSelectB;
      }
   }
   else if ( false == BMUXLIB_INPUT_DESCRIPTOR_IS_ESCR_VALID( pstDescriptorA ) )
   {
      /* A is missing ESCR, or both are missing ESCR */
      /* NOTE: if the ESCR is invalid, the descriptor is likely to be part of the current frame
         so we want to consume those BEFORE selecting a different input */
      eResult = BMUXlib_InputGroup_SelectionResult_eSelectA;
   }
   else
   {
      /* B is missing ESCR */
      eResult = BMUXlib_InputGroup_SelectionResult_eSelectB;
   }

   BDBG_LEAVE( BMUXlib_InputGroup_DescriptorSelectLowestESCR );

   return eResult;
}

/***********************
   Static Functions
***********************/
static void
BMUXlib_Input_P_PeekAtDescriptor(
         BMUXlib_Input_Handle hInput,
         unsigned int uiDescNum,
         BMUXlib_Input_Descriptor *pstDescriptor
         )
{
   BDBG_OBJECT_ASSERT(hInput, BMUXlib_Input_P_Context);
   BDBG_ASSERT( pstDescriptor );

   BDBG_ENTER( BMUXlib_Input_P_PeekAtDescriptor );

   pstDescriptor->descriptor.pstCommon = NULL;
   /* check for attempt to peek at non-existent descriptor */
   BDBG_ASSERT( BMUXLIB_INPUT_P_QUEUE_DEPTH ( hInput->stDescriptorInfo.uiReadOffset, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) >=
                BMUXLIB_INPUT_P_QUEUE_DEPTH ( uiDescNum, hInput->stDescriptorInfo.uiWriteOffset, BMUXLIB_INPUT_P_MAX_DESCRIPTORS ) );

   pstDescriptor->eType = hInput->stCreateSettings.eType;
   switch ( hInput->stCreateSettings.eType )
   {
      case BMUXlib_Input_Type_eVideo:
         pstDescriptor->descriptor.pstVideo = &hInput->stDescriptorInfo.astQueue[uiDescNum].stDescriptor.stVideo;
         break;
      case BMUXlib_Input_Type_eAudio:
         pstDescriptor->descriptor.pstAudio = &hInput->stDescriptorInfo.astQueue[uiDescNum].stDescriptor.stAudio;
         break;
      default:
         /* NOTE: since eType is checked at Create-time, this should not happen */
         break;
   }
   BDBG_ASSERT( true == hInput->bBufferStatusValid );

   if ( NULL != pstDescriptor->descriptor.pstCommon )
   {
      if ( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA( pstDescriptor ) )
      {
         BDBG_ASSERT(hInput->stBufferStatus.hMetadataBufferBlock);
         pstDescriptor->hBlock = hInput->stBufferStatus.hMetadataBufferBlock;
      }
      else
      {
         BDBG_ASSERT(hInput->stBufferStatus.hFrameBufferBlock);
         pstDescriptor->hBlock = hInput->stBufferStatus.hFrameBufferBlock;
      }
   }
   else
   {
      pstDescriptor->hBlock = NULL;
   }

   pstDescriptor->uiFrameSize = hInput->stDescriptorInfo.astQueue[uiDescNum].uiFrameSize;
   pstDescriptor->uiBurstSize = hInput->stDescriptorInfo.astQueue[uiDescNum].uiBurstSize;

   BDBG_LEAVE( BMUXlib_Input_P_PeekAtDescriptor );
}

/* rudimentary shell sort using gaps of n/2
   Shell sort is used since we expect uiCount to be small (<= 32, typically count is 2!)
   and shell sort is a non-recursive in-place sort.
   NOTE: the selector function is a function that compares input descriptors */
static void BMUXlib_InputGroup_P_SortInputs(BMUXlib_InputGroup_P_InputEntry *aData[], uint32_t uiCount, BMUXlib_InputGroup_DescriptorSelector fSelect)
{
   BMUXlib_InputGroup_P_InputEntry *pTemp;
   uint32_t uiGap, i;
   int j;
   for (uiGap = uiCount/2; uiGap != 0; uiGap /= 2)
      for (i = uiGap; i < uiCount; i++)
         for (j = i - uiGap; j >= 0 && fSelect(&aData[j]->stDescriptor, &aData[j+uiGap]->stDescriptor) > 0; j -= uiGap)
         {
            pTemp = aData[j];
            aData[j] = aData[j+uiGap];
            aData[j+uiGap] = pTemp;
         }
}

#if BMUXLIB_INPUT_P_DUMP_DESC
static void BMuxlib_Input_P_DumpNewDescriptors(BMUXlib_Input_Handle hInput, unsigned int uiNumOldDescriptors)
{
   if ( NULL != hInput->hDescDumpFile )
   {
      unsigned uiIndex;

      for ( uiIndex = ( hInput->stDescriptorInfo.uiReadOffset + uiNumOldDescriptors ) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS; uiIndex != hInput->stDescriptorInfo.uiWriteOffset; uiIndex = ( uiIndex + 1 ) % BMUXLIB_INPUT_P_MAX_DESCRIPTORS )
      {
         BMUXlib_Input_Descriptor stDescriptor;
         BMUXlib_Input_P_PeekAtDescriptor( hInput, uiIndex, &stDescriptor );
         /* the following should not happen */
         BDBG_ASSERT(stDescriptor.descriptor.pstCommon != NULL);
         /* all descriptors for a given input are of the type for the input */
         BDBG_ASSERT(BMUXLIB_INPUT_DESCRIPTOR_TYPE(&stDescriptor) == hInput->stCreateSettings.eType);

#if BMUXLIB_INPUT_P_DUMP_FRAME_DESC
         if ( BMUXLIB_INPUT_DESCRIPTOR_IS_FRAMESTART( &stDescriptor ) )
#endif
         {
         /* input descriptor specific */
         void *pBufferBase = BMMA_Lock( BMUXLIB_INPUT_DESCRIPTOR_BLOCK(&stDescriptor) );
         BMuxlib_Debug_CSVWrite(hInput->hDescDumpFile, "%d,%p,%d,",
            *hInput->puiDescCount,
            pBufferBase,
            (hInput->stCreateSettings.eBurstMode == BMUXlib_Input_BurstMode_eFrame)?BMUXLIB_INPUT_DESCRIPTOR_FRAMESIZE(&stDescriptor):0);
         BMMA_Unlock( BMUXLIB_INPUT_DESCRIPTOR_BLOCK(&stDescriptor), pBufferBase );
         /* Common */
         BMuxlib_Debug_CSVWrite( hInput->hDescDumpFile, "0x%08x,%u,%u,%u,%llu,%u,%u,%d,%llu",
            BMUXLIB_INPUT_DESCRIPTOR_FLAGS(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_OFFSET(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_LENGTH(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_ORIGINAL_PTS(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_PTS(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_ESCR(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_TICKS_PER_BIT(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_SHR(&stDescriptor),
            BMUXLIB_INPUT_DESCRIPTOR_STC_SNAPSHOT(&stDescriptor)
            );

         /* Type Specific */
         switch ( BMUXLIB_INPUT_DESCRIPTOR_TYPE(&stDescriptor) )
         {
            case BMUXlib_Input_Type_eVideo:
               BMuxlib_Debug_CSVWrite( hInput->hDescDumpFile, ",0x%08x,%llu,%u,%d",
                  BMUXLIB_INPUT_DESCRIPTOR_VIDEO_FLAGS(&stDescriptor),
                  BMUXLIB_INPUT_DESCRIPTOR_VIDEO_DTS(&stDescriptor),
                  BMUXLIB_INPUT_DESCRIPTOR_VIDEO_DATA_UNIT_TYPE(&stDescriptor),
                  BMUXLIB_INPUT_DESCRIPTOR_IS_KEYFRAME(&stDescriptor)
                  );
               break;
            case BMUXlib_Input_Type_eAudio:
               BMuxlib_Debug_CSVWrite( hInput->hDescDumpFile, ",%u,%u,%u",
                  BMUXLIB_INPUT_DESCRIPTOR_AUDIO_RAWOFFSET(&stDescriptor),
                  BMUXLIB_INPUT_DESCRIPTOR_AUDIO_RAWLENGTH(&stDescriptor),
                  BMUXLIB_INPUT_DESCRIPTOR_AUDIO_DATA_UNIT_TYPE(&stDescriptor)
                  );
               break;
            default:
               break;
         }

         BMuxlib_Debug_CSVWrite( hInput->hDescDumpFile, "\n" );
         BMuxlib_Debug_CSVFlush( hInput->hDescDumpFile );
         (*hInput->puiDescCount)++;
         }
#if BMUXLIB_INPUT_P_DUMP_METADATA_DESC && !BMUXLIB_INPUT_P_DUMP_FRAME_DESC
         /* we only want to output metadata if we are dumping all descriptors */
         if (hInput->hMetadataDumpFile != NULL)
         {
            BMUXlib_Input_Descriptor *pDesc = &stDescriptor;
            if ( BMUXLIB_INPUT_DESCRIPTOR_IS_METADATA(pDesc) && (NULL != pDesc->hBlock))
            {
               switch ( hInput->stCreateSettings.eType )
               {
                  case BMUXlib_Input_Type_eVideo:
                     /* read the data unit type to determine the metadata type */
                     if (BAVC_VideoMetadataType_eCommon == BMUXLIB_INPUT_DESCRIPTOR_VIDEO_DATA_UNIT_TYPE(pDesc))
                     {
                        void *pMetadataBase = BMMA_Lock(pDesc->hBlock);
                        BAVC_VideoMetadataDescriptor *pMetadata = (BAVC_VideoMetadataDescriptor *)BMUXLIB_INPUT_DESCRIPTOR_VIRTUAL_ADDRESS(pMetadataBase, pDesc);
                        BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0x%x, %d, %d, %d, %d, %d, %d, %d, ",
                           pMetadata->uiMetadataFlags, pMetadata->stBufferInfo.eProtocol,
                           pMetadata->stBufferInfo.eProfile, pMetadata->stBufferInfo.eLevel,
                           pMetadata->stBitrate.uiMax, pMetadata->stFrameRate.eFrameRateCode,
                           pMetadata->stDimension.coded.uiWidth, pMetadata->stDimension.coded.uiHeight);
                        switch (pMetadata->stBufferInfo.eProtocol)
                        {
                           case BAVC_VideoCompressionStd_eVC1SimpleMain:
                              /* print VC1 codec-specific data */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "%d, %d, ",
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderB.bCBR,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderB.uiHRDBuffer);
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, ",
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.uiQuantizedFrameRatePostProcessing,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.uiQuantizedBitratePostProcessing,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bLoopFilter,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bMultiResolution,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bFastUVMotionCompensation,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bExtendedMotionVector,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.uiMacroblockQuantization,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bVariableSizedTransform,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bOverlappedTransform,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bSyncMarker,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bRangeReduction,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.uiMaxBFrames,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.uiQuantizer,
                                 pMetadata->uProtocolData.stVC1.sequenceHeaderC.bFrameInterpolation);
                              break;
                           default:
                              /* print zeros for the codec-specific data */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ");
                              break;
                        }
                        BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0x%llx, ", pMetadata->stTiming.uiSTCSnapshot);
                        BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "%d\n", pMetadata->stTiming.uiChunkId);

                        BMMA_Unlock(pDesc->hBlock, pMetadataBase);
                     }
                     /* else, ignore this descriptor (unknown contents) */
                     break;
                  case BMUXlib_Input_Type_eAudio:
                     if (BAVC_AudioMetadataType_eCommon == BMUXLIB_INPUT_DESCRIPTOR_AUDIO_DATA_UNIT_TYPE(pDesc))
                     {
                        void *pMetadataBase = BMMA_Lock(pDesc->hBlock);
                        BAVC_AudioMetadataDescriptor *pMetadata = (BAVC_AudioMetadataDescriptor *)BMUXLIB_INPUT_DESCRIPTOR_VIRTUAL_ADDRESS(pMetadataBase, pDesc);
                        BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0x%x, %d, %d, 0x%llx, %d, %d, ",
                           pMetadata->uiMetadataFlags, pMetadata->stBitrate.uiMax, pMetadata->stSamplingFrequency.uiSamplingFrequency,
                           pMetadata->stTiming.uiSTCSnapshot, pMetadata->stTiming.uiChunkId, pMetadata->eProtocol);
                        switch(pMetadata->eProtocol)
                        {
                           case BAVC_AudioCompressionStd_eAacAdts:
                           case BAVC_AudioCompressionStd_eAacLoas:
                           case BAVC_AudioCompressionStd_eAacPlusAdts:
                           case BAVC_AudioCompressionStd_eAacPlusLoas:
                           {
                              uint32_t uiByteCount;
                              /* print aac info */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "%d, 0x", pMetadata->uProtocolData.stAac.uiASCLengthBits);
                              for (uiByteCount = 0; uiByteCount < pMetadata->uProtocolData.stAac.uiASCLengthBytes; uiByteCount++)
                                 BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "%2.2x", pMetadata->uProtocolData.stAac.auiASC[uiByteCount]);
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, ", ");
                              /* print zeros for wma info */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0, 0, 0, 0, 0");
                              break;
                           }
                           case BAVC_AudioCompressionStd_eWmaStd :
                              /* print zeros for aac info */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0, 0, ");
                              /* print wma info */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "%d, %d, %d, %d, %d",
                                 pMetadata->uProtocolData.stWmaStd.uiSamplesPerBlock,
                                 pMetadata->uProtocolData.stWmaStd.uiEncodeOptions,
                                 pMetadata->uProtocolData.stWmaStd.uiSuperBlockAlign,
                                 pMetadata->uProtocolData.stWmaStd.uiBlockAlign,
                                 pMetadata->uProtocolData.stWmaStd.uiNumChannels);
                              break;
                           default:
                              /* unknown/unsupported protocol: print zeros for all fields */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0, 0, ");         /* aac */
                              BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "0, 0, 0, 0, 0");  /* wma */
                              break;
                        }
                        BMuxlib_Debug_CSVWrite(hInput->hMetadataDumpFile, "\n");

                        BMMA_Unlock(pDesc->hBlock, pMetadataBase);
                     }
                     /* else, ignore this descriptor (unknown contents) */
                     break;
                  default:
                     break;
               }
            } /* end: is metadata descriptor */
            BMuxlib_Debug_CSVFlush( hInput->hMetadataDumpFile );
         } /* end: hMetadataDumpFile != NULL */
#endif
      } /* end: for each new descriptor */
   } /* end: hDescDumpFile != NULL */
}
#endif

/*****************************************************************************
* EOF
******************************************************************************/
