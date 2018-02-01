/***************************************************************************
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
 *
 * [File Description:]
 *
 ***************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */

#include "bvce_debug.h"
#include "bvce_debug_common.h"
#include "bvce_priv.h"

BDBG_MODULE(BVCE_DBG);
BDBG_FILE_MODULE(BVCE_DBG_CFG);
BDBG_FILE_MODULE(BVCE_DBG_STS);
BDBG_FILE_MODULE(BVCE_DBG_BUF);
BDBG_FILE_MODULE(BVCE_DBG_MTA);
BDBG_FILE_MODULE(BVCE_DBG_ITB);
BDBG_FILE_MODULE(BVCE_DBG_CMD);
BDBG_FILE_MODULE(BVCE_DBG_RSP);
BDBG_FILE_MODULE(BVCE_DBG_TR0);
BDBG_FILE_MODULE(BVCE_DBG_TR1);
BDBG_FILE_MODULE(BVCE_DBG_CDO);
BDBG_FILE_MODULE(BVCE_DBG_ITO);
BDBG_FILE_MODULE(BVCE_DBG_PNQ);
BDBG_FILE_MODULE(BVCE_DBG_PBNQ);
BDBG_FILE_MODULE(BVCE_DBG_PBDQ);
BDBG_FILE_MODULE(BVCE_DBG_DNQ);
BDBG_FILE_MODULE(BVCE_DBG_DBNQ);
BDBG_FILE_MODULE(BVCE_DBG_DBDQ);

#define BVCE_DEBUG_STRING_SIZE 512

static
unsigned
BVCE_Debug_S_CommandIndexLUT(
   uint32_t uiCommand
   )
{
   unsigned uiCommandIndex;

   for ( uiCommandIndex = 0; uiCommandIndex < BVCE_P_CommandLUT_size / sizeof( BVCE_P_CommandDebug ); uiCommandIndex++ )
   {
      if ( BVCE_P_CommandLUT[uiCommandIndex].uiCommand == uiCommand )
      {
         return uiCommandIndex;
      }
   }

   return 0;
}

static const char* const BVCE_P_FW_BarTypeFriendlyNameLUT[SOURCE_BAR_DATA_TYPE_LEFT_RIGHT + 1] =
{
   BDBG_STRING("INV"),
   BDBG_STRING("T/B"),
   BDBG_STRING("L/R"),

};

static const char* const BVCE_P_FW_OrientationFriendlyNameLUT[SOURCE_ORIENTATION_TYPE_3D_LEFT_RIGHT_ENHANCED + 1] =
{
   BDBG_STRING("2D   "),
   BDBG_STRING("3DLR "),
   BDBG_STRING("3DOU "),
   BDBG_STRING("3DL  "),
   BDBG_STRING("3DR  "),
   BDBG_STRING("3DLRE"),
};

static const char* const BVCE_P_FW_PolarityFriendlyNameLUT[SOURCE_POLARITY_FRAME + 2] =
{
   BDBG_STRING("T"),
   BDBG_STRING("B"),
   BDBG_STRING("F"),
   BDBG_STRING("U"),
};

static const char* const BVCE_P_FW_CadenceTypeFriendlyNameLUT[SOURCE_CADENCE_TYPE_2_2 + 2] =
{
   BDBG_STRING("UNK"),
   BDBG_STRING("3:2"),
   BDBG_STRING("2:2"),
   BDBG_STRING("INV"),
};

void
BVCE_Debug_PrintLogVideoDescriptor(
      const BAVC_VideoBufferDescriptor *pstDescriptor,
      char szDebug[],
      signed *piBytesLeft
      )
{
   unsigned uiSize = *piBytesLeft;

   if ( (*piBytesLeft) < 32 ) { *piBytesLeft = -1; return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "offset=%08x length=%08lx ",
      pstDescriptor->stCommon.uiOffset,
      (unsigned long) pstDescriptor->stCommon.uiLength
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "[ "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )  ? "frm " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START ) )  ? "seg " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS ) )  ? "eos " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME ) )  ? "nul " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_END ) )  ? "end " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC ) )  ? "eoc " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA ) )  ? "mta " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EXTENDED ) )  ? "xtn " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP ) )  ? "rap " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) )  ? "dut " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_HIDDEN_FRAME ) )  ? "hid " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_SHOW_FRAME ) )  ? "shw " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "%s",
      ( 0 != ( pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_EXTENDED ) )  ? "xtn " : "    "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
      "] "
   );
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "opts=0x%08x ",
         pstDescriptor->stCommon.uiOriginalPTS
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "pts=0x%08x%08x ",
         (uint32_t) (pstDescriptor->stCommon.uiPTS >> 32),
         (uint32_t) pstDescriptor->stCommon.uiPTS
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "dts=0x%08x%08x ",
         (uint32_t) (pstDescriptor->uiDTS >> 32),
         (uint32_t) pstDescriptor->uiDTS
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "escr=0x%08x ",
         pstDescriptor->stCommon.uiESCR
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "tpb=0x%08x ",
         pstDescriptor->stCommon.uiTicksPerBit
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "shr=%d ",
         pstDescriptor->stCommon.iSHR
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "stc=0x%08x%08x ",
         (uint32_t) (pstDescriptor->stCommon.uiSTCSnapshot >> 32),
         (uint32_t) pstDescriptor->stCommon.uiSTCSnapshot
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }

   if ( 0 != ( pstDescriptor->uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) )
   {
      (*piBytesLeft) -= BKNI_Snprintf( &szDebug[uiSize-(*piBytesLeft)], (*piBytesLeft),
         "dut=%u ",
         pstDescriptor->uiDataUnitType
      );
   }
   if ( (*piBytesLeft) < 0 ) { return; }
}

void
BVCE_Debug_PrintLogMessageEntry(
      const void *pstFifoEntry /* Should point to an element of size BVCE_Debug_FifoInfo.uiElementSize */
      )
{
   const BVCE_P_DebugFifo_Entry *pstEntry = (BVCE_P_DebugFifo_Entry *) pstFifoEntry;

   BDBG_ASSERT( pstEntry );

   BDBG_CASSERT( BVCE_DebugFifo_EntryType_eMax == (BVCE_DebugFifo_EntryType_ePictureBufferDropDequeue + 1) );
   switch ( pstEntry->stMetadata.eType )
   {
      case BVCE_DebugFifo_EntryType_eConfig:
         BDBG_CWARNING( sizeof( BVCE_Channel_StartEncodeSettings ) <= 144 );
         BDBG_CWARNING( sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) == 3 );
         BDBG_CWARNING( sizeof( BVCE_Channel_EncodeSettings ) == 60 );

         BDBG_MODULE_MSG(BVCE_DBG_CFG, ("(%10u)[%u][%u] PIC: protocol=%u,profile=%u,level=%u,input_type=%u,num_slices=%u,force_entropy_coding=%u,single_ref_p=%u,required_patches_only=%u",
            pstEntry->stMetadata.uiTimestamp,
            pstEntry->stMetadata.uiInstance,
            pstEntry->stMetadata.uiChannel,
            pstEntry->data.stConfig.stStartEncodeSettings.stProtocolInfo.eProtocol,
            pstEntry->data.stConfig.stStartEncodeSettings.stProtocolInfo.eProfile,
            pstEntry->data.stConfig.stStartEncodeSettings.stProtocolInfo.eLevel,
            pstEntry->data.stConfig.stStartEncodeSettings.eInputType,
            pstEntry->data.stConfig.stEncodeSettings.uiNumSlicesPerPic,
            pstEntry->data.stConfig.stStartEncodeSettings.eForceEntropyCoding,
            pstEntry->data.stConfig.stStartEncodeSettings.stMemoryBandwidthSaving.bSingleRefP,
            pstEntry->data.stConfig.stStartEncodeSettings.stMemoryBandwidthSaving.bRequiredPatchesOnly
         ));

         BDBG_MODULE_MSG(BVCE_DBG_CFG, ("(%10u)[%u][%u] TIMING: stc=%u,rate_buffer_delay=%u,fr=%u,vfr=%u,sfr=%u,br_max=%u,br_target=%u,A2PDelay=%u,num_parallel_encodes=%u,segment_rc=%u,segment_duration=%u,segment_upper_tolerance=%u,segment_upper_slope_factor=%u,segment_lower_tolerance=%u,segment_lower_slope_factor=%u,,disable_hrd_drop=%u",
            pstEntry->stMetadata.uiTimestamp,
            pstEntry->stMetadata.uiInstance,
            pstEntry->stMetadata.uiChannel,
            pstEntry->data.stConfig.stStartEncodeSettings.uiStcIndex,
            pstEntry->data.stConfig.stStartEncodeSettings.uiRateBufferDelay,
            pstEntry->data.stConfig.stEncodeSettings.stFrameRate.eFrameRate,
            pstEntry->data.stConfig.stEncodeSettings.stFrameRate.bVariableFrameRateMode,
            pstEntry->data.stConfig.stEncodeSettings.stFrameRate.bSparseFrameRateMode,
            pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiMax,
            pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiTarget,
            pstEntry->data.stConfig.stEncodeSettings.uiA2PDelay,
            pstEntry->data.stConfig.stStartEncodeSettings.uiNumParallelNRTEncodes,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.bEnable,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.uiDuration,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiSlopeFactor,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiSlopeFactor,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stHrdMode.bDisableFrameDrop
         ));

         BDBG_MODULE_MSG(BVCE_DBG_CFG, ("(%10u)[%u][%u] FLAGS: nrt=%u,pipeline_low_delay=%u,adaptive_low_delay=%u,itfp=%u,on_input_change=%u,new_rap=%u,fast_channel_change=%u",
            pstEntry->stMetadata.uiTimestamp,
            pstEntry->stMetadata.uiInstance,
            pstEntry->stMetadata.uiChannel,
            pstEntry->data.stConfig.stStartEncodeSettings.bNonRealTimeEncodeMode,
            pstEntry->data.stConfig.stStartEncodeSettings.bPipelineLowDelayMode,
            pstEntry->data.stConfig.stStartEncodeSettings.bAdaptiveLowDelayMode,
            pstEntry->data.stConfig.stEncodeSettings.bITFPEnable,
            pstEntry->data.stConfig.stSettingsModifiers.bOnInputChange,
            pstEntry->data.stConfig.stSettingsModifiers.bBeginNewRAP,
            pstEntry->data.stConfig.stSettingsModifiers.bFastChannelChange
         ));

         BDBG_MODULE_MSG(BVCE_DBG_CFG, ("(%10u)[%u][%u] GOP: restart=%u,min_gop_length_after_restart=%u%%,duration=%u,duration_ramp_up=%u,p=%u,b=%u,open_gop=%u",
            pstEntry->stMetadata.uiTimestamp,
            pstEntry->stMetadata.uiInstance,
            pstEntry->stMetadata.uiChannel,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.bAllowNewGOPOnSceneChange,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiMinGOPLengthAfterSceneChange,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiDuration,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiDurationRampUpFactor,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiNumberOfPFrames,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiNumberOfBFrames,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.bAllowOpenGOP
         ));
         break;

      case BVCE_DebugFifo_EntryType_eStatus:
         {
            unsigned i;
            /* Print Errors */
            BDBG_MODULE_MSG(BVCE_DBG_STS, ("(%10u)[%u][%u] ERRORS: total=%u",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               pstEntry->data.stStatus.uiTotalErrors
            ));

            for ( i = 0; i < 32; i++ )
            {
               if ( 0 != ( (pstEntry->data.stStatus.uiErrorFlags >> i) & 0x01 ) )
               {
                  BDBG_MODULE_MSG(BVCE_DBG_STS, ("(%10u)[%u][%u] \t%s",
                     pstEntry->stMetadata.uiTimestamp,
                     pstEntry->stMetadata.uiInstance,
                     pstEntry->stMetadata.uiChannel,
                     BVCE_P_ErrorLUT[i]
                  ));
               }
            }

            /* Print Events */
            BDBG_MODULE_MSG(BVCE_DBG_STS, ("(%10u)[%u][%u] EVENTS:",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel
            ));

            for ( i = 0; i < 32; i++ )
            {
               if ( 0 != ( (pstEntry->data.stStatus.uiEventFlags >> i) & 0x01 ) )
               {
                  BDBG_MODULE_MSG(BVCE_DBG_STS, ("(%10u)[%u][%u] \t%s",
                     pstEntry->stMetadata.uiTimestamp,
                     pstEntry->stMetadata.uiInstance,
                     pstEntry->stMetadata.uiChannel,
                     BVCE_P_EventLUT[i]
                  ));
               }
            }

            BDBG_MODULE_MSG(BVCE_DBG_STS, ("(%10u)[%u][%u] PIC: fps=%08x,rx=%u,tx=%u,dfrc=%u,derr=%u,dhrd=%u,id=%08x",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               pstEntry->data.stStatus.uiAverageFramesPerSecond,
               pstEntry->data.stStatus.uiTotalPicturesReceived,
               pstEntry->data.stStatus.uiTotalPicturesEncoded,
               pstEntry->data.stStatus.uiTotalPicturesDroppedDueToFrameRateConversion,
               pstEntry->data.stStatus.uiTotalPicturesDroppedDueToErrors,
               pstEntry->data.stStatus.uiTotalPicturesDroppedDueToHRDUnderflow,
               pstEntry->data.stStatus.uiLastPictureIdEncoded
            ));

            BDBG_MODULE_MSG(BVCE_DBG_STS, ("(%10u)[%u][%u] TIMING: dEtsDts=%08x,stc=%08x%x",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               pstEntry->data.stStatus.uiEtsDtsOffset,
               (uint32_t) ((pstEntry->data.stStatus.uiSTCSnapshot >> 32) & 0xFFFFFFFF),
               (uint32_t) (pstEntry->data.stStatus.uiSTCSnapshot & 0xFFFFFFFF)
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_eBufferDescriptor:
         {
            char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
            signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;

            BVCE_Debug_PrintLogVideoDescriptor(
                  &pstEntry->data.stBufferDescriptor,
                  szDebug,
                  &iBytesLeft
                  );

            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            goto dbg_buf_done;
dbg_buf_overflow:
            BDBG_MODULE_MSG(BVCE_DBG_BUF, ("Debug String Overflow"));
dbg_buf_done:
            BDBG_MODULE_MSG(BVCE_DBG_BUF, ("(%10u)[%u][%u] %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               szDebug
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_eMetadataDescriptor:
      {
            char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
            signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;
            unsigned i;

            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "br=%u ",
                  pstEntry->data.stMetadataDescriptor.stBitrate.uiMax
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_FRAMERATE_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "fr=%u ",
                  pstEntry->data.stMetadataDescriptor.stFrameRate.eFrameRateCode
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_DIMENSION_CODED_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "res=%ux%u ",
                  pstEntry->data.stMetadataDescriptor.stDimension.coded.uiWidth,
                  pstEntry->data.stMetadataDescriptor.stDimension.coded.uiHeight
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BUFFERINFO_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "protocol=%u profile=%u level=%u ",
                  pstEntry->data.stMetadataDescriptor.stBufferInfo.eProtocol,
                  pstEntry->data.stMetadataDescriptor.stBufferInfo.eProfile,
                  pstEntry->data.stMetadataDescriptor.stBufferInfo.eLevel
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

#if 0
            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "br=0x%08x ",
                  pstEntry->data.stMetadataDescriptor.stBitrate.uiMax
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }
#endif

            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_STC_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "stc=0x%08x%08x ",
                  (uint32_t) (pstEntry->data.stMetadataDescriptor.stTiming.uiSTCSnapshot >> 32),
                  (uint32_t) pstEntry->data.stMetadataDescriptor.stTiming.uiSTCSnapshot
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_CHUNK_ID_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "chunk=%u ",
                  pstEntry->data.stMetadataDescriptor.stTiming.uiChunkId
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            if ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_ETS_DTS_OFFSET_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "dEtsDts=0x%08x ",
                  pstEntry->data.stMetadataDescriptor.stTiming.uiEtsDtsOffset
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "NALU[ "
             );
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            for ( i = 0; i < BAVC_VIDEO_MAX_EXTRACTED_NALU_COUNT; i++ )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "%03u ",
                  pstEntry->data.stMetadataDescriptor.stExtractedNALUInfo.astExtractedNALUEntry[i].uiNumValidBytes
                );
               if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }
            }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "] "
             );
            if ( iBytesLeft < 0 ) { goto dbg_mta_overflow; }

            goto dbg_mta_done;
dbg_mta_overflow:
            BDBG_MODULE_MSG(BVCE_DBG_MTA, ("Debug String Overflow"));
dbg_mta_done:
            BDBG_MODULE_MSG(BVCE_DBG_MTA, ("(%10u)[%u][%u] %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               szDebug
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_eITB:
      {
         char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
         signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;
         unsigned i;

         iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                        BDBG_UINT64_FMT": ",
                        BDBG_UINT64_ARG(pstEntry->data.stITB.uiReadOffset)
                     );

         if ( iBytesLeft < 0 ) { goto dbg_itb_overflow; }

         for ( i = 0; i < 16; i++ )
         {
            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%02x ",
               pstEntry->data.stITB.auiEntry[i]
            );
         }
         if ( iBytesLeft < 0 ) { goto dbg_itb_overflow; }

         goto dbg_itb_done;
dbg_itb_overflow:
         BDBG_MODULE_MSG(BVCE_DBG_ITB, ("Debug String Overflow"));
dbg_itb_done:
         BDBG_MODULE_MSG(BVCE_DBG_ITB, ("(%10u)[%u][%u] %s",
            pstEntry->stMetadata.uiTimestamp,
            pstEntry->stMetadata.uiInstance,
            pstEntry->stMetadata.uiChannel,
            szDebug
         ));
         break;
      }
      case BVCE_DebugFifo_EntryType_eCommand:
         {
            unsigned i;

            for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stCommand.data[0])].uiCommandSize; i++ )
            {
               BDBG_MODULE_MSG( BVCE_DBG_CMD, ("(%10u)[%u][%u] %08x - %s",
                  pstEntry->stMetadata.uiTimestamp,
                  pstEntry->stMetadata.uiInstance,
                  pstEntry->stMetadata.uiChannel,
                  pstEntry->data.stCommand.data[i],
                  BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stCommand.data[0])].szCommandParameterName[i]
               ));
            }
         }
         break;

      case BVCE_DebugFifo_EntryType_eResponse:
         {
            unsigned i;
            for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stResponse.data[0])].uiResponseSize; i++ )
            {
               BDBG_MODULE_MSG( BVCE_DBG_RSP, ("(%10u)[%u][%u] %08x - %s",
                  pstEntry->stMetadata.uiTimestamp,
                  pstEntry->stMetadata.uiInstance,
                  pstEntry->stMetadata.uiChannel,
                  pstEntry->data.stCommand.data[i],
                  BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stResponse.data[0])].szResponseParameterName[i]
               ));
            }
         }
         break;

      case BVCE_DebugFifo_EntryType_eTrace0:
         {
            BDBG_MODULE_MSG( BVCE_DBG_TR0, ("(%10u)[%u][%u] %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               pstEntry->data.szFunctionTrace
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_eTrace1:
         {
            BDBG_MODULE_MSG( BVCE_DBG_TR1, ("(%10u)[%u][%u] %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               pstEntry->data.szFunctionTrace
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_eCDBOffsets:
         {
            BDBG_MODULE_MSG( BVCE_DBG_CDO, ("(%10u)[%u][%u] b="BDBG_UINT64_FMT" e="BDBG_UINT64_FMT" r="BDBG_UINT64_FMT" sr="BDBG_UINT64_FMT" v="BDBG_UINT64_FMT" w="BDBG_UINT64_FMT,
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiBase),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiEnd),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiRead),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiShadowRead),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiValid),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiWrite)
               ));
         }
         break;

      case BVCE_DebugFifo_EntryType_eITBOffsets:
         {
            BDBG_MODULE_MSG( BVCE_DBG_ITO, ("(%10u)[%u][%u] b="BDBG_UINT64_FMT" e="BDBG_UINT64_FMT" r="BDBG_UINT64_FMT" sr="BDBG_UINT64_FMT" v="BDBG_UINT64_FMT" w="BDBG_UINT64_FMT,
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiBase),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiEnd),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiRead),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiShadowRead),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiValid),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiWrite)
               ));
         }
         break;

      case BVCE_DebugFifo_EntryType_ePictureBufferEnqueue:
         {
            char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
            char *pszDebug2 = NULL;
            signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "luma="BDBG_UINT64_FMT"(%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "chroma="BDBG_UINT64_FMT"(%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "1v="BDBG_UINT64_FMT"(%1u,%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "2v="BDBG_UINT64_FMT"(%1u,%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "shifted="BDBG_UINT64_FMT"(%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft--;
            pszDebug2 = &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft];
            szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft] = '\0';

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "res=%ux%u (%ux%u) ",
               ( pstEntry->data.stPictureBufferMailbox.uiResolution & BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiResolution & BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_SHIFT,
               ( ( pstEntry->data.stPictureBufferMailbox.uiResolution & BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_SHIFT ) << 3,
               ( ( pstEntry->data.stPictureBufferMailbox.uiResolution & BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_SHIFT ) << 3
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "crop=%ux%u ",
               ( pstEntry->data.stPictureBufferMailbox.uiCropping & BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiCropping & BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "sar=%ux%u ",
               ( pstEntry->data.stPictureBufferMailbox.uiSampleAspectRatio & BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiSampleAspectRatio & BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "bar=%u/%u(%s) ",
               ( pstEntry->data.stPictureBufferMailbox.uiBarData & BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiBarData & BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_SHIFT,
               BVCE_P_FW_BarTypeFriendlyNameLUT[( pstEntry->data.stPictureBufferMailbox.uiBarData & BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_SHIFT]
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "ori=%s ",
               BVCE_P_FW_OrientationFriendlyNameLUT[( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_SHIFT]
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "afd=%u(%u) ",
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_VALID_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_VALID_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            {
               unsigned uiPolarity = ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_SHIFT;
               unsigned uiCadenceType = ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_SHIFT;

               if ( uiPolarity > SOURCE_POLARITY_FRAME )
               {
                  BDBG_MODULE_MSG(BVCE_DBG_PNQ, ("Invalid Polarity: %u", uiPolarity));
                  uiPolarity = SOURCE_POLARITY_FRAME+1;
               }

               if ( uiCadenceType > SOURCE_CADENCE_TYPE_2_2 )
               {
                  BDBG_MODULE_MSG(BVCE_DBG_PNQ, ("Invalid Cadence Type: %u", uiCadenceType));
                  uiCadenceType = SOURCE_CADENCE_TYPE_2_2+1;
               }

               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "cad=%s%u(%s) ",
                  BVCE_P_FW_PolarityFriendlyNameLUT[uiPolarity],
                  ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_SHIFT,
                  BVCE_P_FW_CadenceTypeFriendlyNameLUT[uiCadenceType]
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "oPts=0x%08x ",
               pstEntry->data.stPictureBufferMailbox.uiOriginalPts
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "picId=%u ",
               pstEntry->data.stPictureBufferMailbox.uiPictureId
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "stc="BDBG_UINT64_FMT" ",
               BDBG_UINT64_ARG( ( ((uint64_t) pstEntry->data.stPictureBufferMailbox.uiSTCSnapshotHi) << 32 ) | pstEntry->data.stPictureBufferMailbox.uiSTCSnapshotLo )
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "[ "
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stPictureBufferMailbox.uiMetadata & BVCE_P_FW_PictureBufferMailbox_Metadata_LAST_MASK ) ) ? "lst " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stPictureBufferMailbox.uiMetadata & BVCE_P_FW_PictureBufferMailbox_Metadata_CHANNEL_CHANGE_MASK ) ) ? "chc " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stPictureBufferMailbox.uiMetadata & BVCE_P_FW_PictureBufferMailbox_Metadata_BUSY_MASK ) ) ? "bsy " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "] "
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "dPts=%u ",
               ( pstEntry->data.stPictureBufferMailbox.uiTimingInfo & BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_MASK ) >> BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "newPts="BDBG_UINT64_FMT" ",
               BDBG_UINT64_ARG( ( ((uint64_t) pstEntry->data.stPictureBufferMailbox.uiNewPts) << 1 ) | ( ( pstEntry->data.stPictureBufferMailbox.uiTimingInfo & BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_MASK ) >> BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_SHIFT) )
            );
            if ( iBytesLeft < 0 ) { goto dbg_pic_overflow; }

            goto dbg_pic_done;
dbg_pic_overflow:
            BDBG_MODULE_MSG(BVCE_DBG_PNQ, ("Debug String Overflow"));
dbg_pic_done:
            BDBG_MODULE_MSG(BVCE_DBG_PBNQ, ("(%10u)[%u][%u] %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               szDebug
            ));
            BDBG_MODULE_MSG(BVCE_DBG_PNQ, ("(%10u)[%u][%u] %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               pszDebug2
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_ePictureBufferDequeue:
         {
            char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
            signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "luma="BDBG_UINT64_FMT"(%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "chroma="BDBG_UINT64_FMT"(%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "1v="BDBG_UINT64_FMT"(%1u,%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "2v="BDBG_UINT64_FMT"(%1u,%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "shifted="BDBG_UINT64_FMT"(%1u,%3u) ",
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_rel_overflow; }

            goto dbg_rel_done;
dbg_rel_overflow:
            BDBG_MODULE_MSG(BVCE_DBG_PBDQ, ("Debug String Overflow"));
dbg_rel_done:
            BDBG_MODULE_MSG(BVCE_DBG_PBDQ, ("(%10u)[%u][%u] %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               szDebug
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_ePictureBufferDropEnqueue:
      case BVCE_DebugFifo_EntryType_ePictureBufferDropDequeue:
         {
            char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
            char *pszDebug2 = NULL;
            signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "luma="BDBG_UINT64_FMT"(%1u,%3u) ",
               0,
               pstEntry->data.stPicture.ulLumaOffset,
               0,
               0
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "chroma="BDBG_UINT64_FMT"(%1u,%3u) ",
               0,
               pstEntry->data.stPicture.ulChromaOffset,
               0,
               0
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "1v="BDBG_UINT64_FMT"(%1u,%1u,%3u) ",
               0,
               pstEntry->data.stPicture.ul1VLumaOffset,
               0,
               0,
               0
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "2v="BDBG_UINT64_FMT"(%1u,%1u,%3u) ",
               0,
               pstEntry->data.stPicture.ul2VLumaOffset,
               0,
               0,
               0
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "shifted="BDBG_UINT64_FMT"(%1u,%3u) ",
               0,
               pstEntry->data.stPicture.ulShiftedChromaOffset,
               0,
               0
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft--;
            pszDebug2 = &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft];
            szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft] = '\0';

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "res=%ux%u (%ux%u) ",
               pstEntry->data.stPicture.ulWidthInMbs,
               pstEntry->data.stPicture.ulHeightInMbs,
               pstEntry->data.stPicture.ulWidth,
               pstEntry->data.stPicture.ulHeight
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "crop=%ux%u ",
               0,
               0
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "sar=%ux%u ",
               pstEntry->data.stPicture.ulAspectRatioX,
               pstEntry->data.stPicture.ulAspectRatioY
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "bar=%u/%u(%s) ",
               pstEntry->data.stPicture.stBarData.uiTopLeft,
               pstEntry->data.stPicture.stBarData.uiBotRight,
               BVCE_P_FW_BarTypeFriendlyNameLUT[pstEntry->data.stPicture.stBarData.eType]
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "ori=%s ",
               BVCE_P_FW_OrientationFriendlyNameLUT[pstEntry->data.stPicture.eOrientation]
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "afd=%u(%u) ",
               pstEntry->data.stPicture.stAFD.uiMode,
               pstEntry->data.stPicture.stAFD.bValid
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "cad=%s%u(%s) ",
               BVCE_P_FW_PolarityFriendlyNameLUT[pstEntry->data.stPicture.ePolarity],
               pstEntry->data.stPicture.stCadence.phase,
               BVCE_P_FW_CadenceTypeFriendlyNameLUT[pstEntry->data.stPicture.stCadence.type]
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "oPts=0x%08x ",
               pstEntry->data.stPicture.ulOriginalPTS
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "picId=%u ",
               pstEntry->data.stPicture.ulPictureId
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "stc="BDBG_UINT64_FMT" ",
               BDBG_UINT64_ARG( ( ((uint64_t) pstEntry->data.stPicture.ulSTCSnapshotHi) << 32 ) | pstEntry->data.stPicture.ulSTCSnapshotLo )
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "[ "
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( true == ( pstEntry->data.stPicture.bLast ) ) ? "lst " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( true == ( pstEntry->data.stPicture.bChannelChange ) ) ? "chc " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "] "
            );
            if ( iBytesLeft < 0 ) { goto dbg_drop_rel_overflow; }

            goto dbg_drop_rel_done;
dbg_drop_rel_overflow:
            if ( BVCE_DebugFifo_EntryType_ePictureBufferDropEnqueue == pstEntry->stMetadata.eType )
            {
               BDBG_MODULE_MSG(BVCE_DBG_DNQ, ("Debug String Overflow"));
               BDBG_MODULE_MSG(BVCE_DBG_DBNQ, ("Debug String Overflow"));
            }
            else
            {
               BDBG_MODULE_MSG(BVCE_DBG_DBDQ, ("Debug String Overflow"));
            }

dbg_drop_rel_done:
            if ( BVCE_DebugFifo_EntryType_ePictureBufferDropEnqueue == pstEntry->stMetadata.eType )
            {
               BDBG_MODULE_MSG(BVCE_DBG_DBNQ, ("(%10u)[%u][%u] %s",
                  pstEntry->stMetadata.uiTimestamp,
                  pstEntry->stMetadata.uiInstance,
                  pstEntry->stMetadata.uiChannel,
                  szDebug
               ));

               BDBG_MODULE_MSG(BVCE_DBG_DNQ, ("(%10u)[%u][%u] %s",
                  pstEntry->stMetadata.uiTimestamp,
                  pstEntry->stMetadata.uiInstance,
                  pstEntry->stMetadata.uiChannel,
                  pszDebug2
               ));

            }
            else
            {
               BDBG_MODULE_MSG(BVCE_DBG_DBDQ, ("(%10u)[%u][%u] %s",
                  pstEntry->stMetadata.uiTimestamp,
                  pstEntry->stMetadata.uiInstance,
                  pstEntry->stMetadata.uiChannel,
                  szDebug
               ));
            }
         }
         break;

      default:
         BDBG_LOG(("Unrecognized debug entry type: %d", pstEntry->stMetadata.eType));
         break;
   }
}

bool
BVCE_Debug_FormatLogHeader(
   unsigned uiIndex,
   char *szMessage,
   unsigned uiSize
   )
{
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   {
      bool bMore = true;
      signed iBytesLeft = uiSize;
      unsigned i;

      if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

      if ( 0 == uiIndex )
      {
         iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
            "timestamp (ms),entry type,entry type (friendly),instance,channel\n"
         );
         if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
      }
      else
      {
         BVCE_DebugFifo_EntryType eType = uiIndex - 1;

         BDBG_CASSERT( BVCE_DebugFifo_EntryType_eMax == (BVCE_DebugFifo_EntryType_ePictureBufferDropDequeue + 1) );
         switch ( eType )
         {
            case BVCE_DebugFifo_EntryType_eConfig:
            case BVCE_DebugFifo_EntryType_eStatus:
            case BVCE_DebugFifo_EntryType_eBufferDescriptor:
            case BVCE_DebugFifo_EntryType_eMetadataDescriptor:
            case BVCE_DebugFifo_EntryType_eITB:
            case BVCE_DebugFifo_EntryType_eCommand:
            case BVCE_DebugFifo_EntryType_eResponse:
            case BVCE_DebugFifo_EntryType_eTrace0:
            case BVCE_DebugFifo_EntryType_eTrace1:
            case BVCE_DebugFifo_EntryType_eCDBOffsets:
            case BVCE_DebugFifo_EntryType_eITBOffsets:
            case BVCE_DebugFifo_EntryType_ePictureBufferEnqueue:
            case BVCE_DebugFifo_EntryType_ePictureBufferDequeue:
            case BVCE_DebugFifo_EntryType_ePictureBufferDropEnqueue:
            case BVCE_DebugFifo_EntryType_ePictureBufferDropDequeue:
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  "%u,%u,%s,instance,channel",
                  0,
                  eType,
                  BVCE_P_Debug_EntryTypeLUT[eType]
               );
               break;
            default:
               break;
         }

         if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

         /* Print the header */
         switch ( eType )
         {
            case BVCE_DebugFifo_EntryType_eConfig:
               BDBG_CWARNING( sizeof( BVCE_Channel_StartEncodeSettings ) <= 144 );
               BDBG_CWARNING( sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) == 3 );
               BDBG_CWARNING( sizeof( BVCE_Channel_EncodeSettings ) == 60 );

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",protocol,profile,level,input type,num slices,force entropy coding,single ref P,required patches only"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",stc index,rate buffer delay,fr,vfr,sfr,br_max,br_target,A2PDelay,num parallel encodes,segment rc,segment duration,segment upper tolerance,segment upper slope factor,segment lower tolerance,segment lower slope factor,disable hrd drop"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",nrt,pipeline low delay,adaptive low delay,itfp,on input change,new rap,fast channel change"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",restart,min_gop_length_after_restart,duration,duration ramp up,p frames,b frames,open gop"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               break;

            case BVCE_DebugFifo_EntryType_eStatus:
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",total errors,error flags,event flags,fps,received,encoded,dropped (frc),dropped (err),dropped (hrd underflow),id,ets/dts offset (27Mhz),stc snapshot"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               break;

            case BVCE_DebugFifo_EntryType_eBufferDescriptor:
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",flags,metadata,extended,frame start,segment start,eoc,eos,empty frame,offset,length,opts valid,opts,pts valid,pts,escr valid,escr,tpb valid,tpb,shr valid,shr,stc valid,stc"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",vflags,dts valid,dts,dut start,dut,rai,hid,shw"
                );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               break;

            case BVCE_DebugFifo_EntryType_eMetadataDescriptor:
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",metadata flags"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",buffer info valid,protocol,profile,level"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",bitrate valid,bitrate max"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",frame rate valid,frame rate"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",dimension valid,width,height"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",stc valid,stc snapshot,chunk id valid,chunk id,dEtsDts valid,dEtsDts"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               for ( i = 0; i < BAVC_VIDEO_MAX_EXTRACTED_NALU_COUNT; i++ )
               {
                  iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                     ",nalu[%u] size",
                     i
                  );
                  if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               }
               break;

            case BVCE_DebugFifo_EntryType_eITB:
               break;

            case BVCE_DebugFifo_EntryType_eCommand:
               break;

            case BVCE_DebugFifo_EntryType_eResponse:
               break;

            case BVCE_DebugFifo_EntryType_eTrace0:
            case BVCE_DebugFifo_EntryType_eTrace1:
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",function"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               break;

            case BVCE_DebugFifo_EntryType_eCDBOffsets:
            case BVCE_DebugFifo_EntryType_eITBOffsets:
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",base,end,read,shadow read,valid,write"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               break;

            case BVCE_DebugFifo_EntryType_ePictureBufferEnqueue:
               BDBG_CWARNING( sizeof( BVCE_P_FW_PictureBufferMailbox ) == 4*27 );

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",luma offset hi, luma offset lo,luma dcxv enable,luma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",chroma offset hi,chroma offset lo,chroma dcxv enable,chroma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",1v luma offset hi,1v luma offset lo,1v luma dcxv enable,1v decimation shift,1v luma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",2v luma offset hi,2v luma offset lo,2v luma dcxv enable,2v decimation shift,2v luma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",shifted chroma offset hi,shifted chroma offset lo,shifted chroma dcxv enable,shifted chroma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",width,height"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",right crop,bottom crop"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",sample aspect ratio x,sample aspect ratio y"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",bar data type,bar data top/left,bar data bottom/right"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",orientation,afd valid,afd mode,cadence type,cadence phase,polarity"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",delta pts"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",new pts"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",original pts"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",picture id"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",stc snapshot"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",last,channel change,busy"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               break;

            case BVCE_DebugFifo_EntryType_ePictureBufferDequeue:
               BDBG_CWARNING( sizeof( BVCE_P_FW_PictureBufferMailbox ) == 4*27 );

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",luma offset hi, luma offset lo,luma dcxv enable,luma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",chroma offset hi,chroma offset lo,chroma dcxv enable,chroma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",1v luma offset hi,1v luma offset lo,1v luma dcxv enable,1v decimation shift,1v luma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",2v luma offset hi,2v luma offset lo,2v luma dcxv enable,2v decimation shift,2v luma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",shifted chroma offset hi,shifted chroma offset lo,shifted chroma dcxv enable,shifted chroma nmby"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               break;

            case BVCE_DebugFifo_EntryType_ePictureBufferDropEnqueue:
            case BVCE_DebugFifo_EntryType_ePictureBufferDropDequeue:
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",luma offset hi, luma offset lo"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",chroma offset hi,chroma offset lo"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",1v luma offset hi,1v luma offset lo"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",2v luma offset hi,2v luma offset lo"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",shifted chroma offset hi,shifted chroma offset lo"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
               break;

            default:
               szMessage[0] = '\0';
               bMore = false;
               break;
         }
         if ( true == bMore )
         {
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               "\n"
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }
         }
      }
      goto dbg_fmt_hdr_done;

   dbg_fmt_hdr_overflow:
      BDBG_LOG(("Format Debug String Overflow"));

   dbg_fmt_hdr_done:
      return bMore;
   }
}

void
BVCE_Debug_FormatLogMessage(
   const void *pstFifoEntry, /* Should point to an element of size BVCE_Debug_FifoInfo.uiElementSize */
   char *szMessage,
   unsigned uiSize
   )
{
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   {
      const BVCE_P_DebugFifo_Entry *pstEntry = (BVCE_P_DebugFifo_Entry *) pstFifoEntry;
      signed iBytesLeft = uiSize;
      unsigned i;
      if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

      BDBG_ASSERT( pstEntry );

      iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
         "%u,%u,%s,%u,%u",
         pstEntry->stMetadata.uiTimestamp,
         pstEntry->stMetadata.eType,
         BVCE_P_Debug_EntryTypeLUT[pstEntry->stMetadata.eType],
         pstEntry->stMetadata.uiInstance,
         pstEntry->stMetadata.uiChannel
      );
      if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

      BDBG_CASSERT( BVCE_DebugFifo_EntryType_eMax == (BVCE_DebugFifo_EntryType_ePictureBufferDropDequeue + 1) );
      switch ( pstEntry->stMetadata.eType )
      {
         case BVCE_DebugFifo_EntryType_eConfig:
            BDBG_CWARNING( sizeof( BVCE_Channel_StartEncodeSettings ) <= 144 );
            BDBG_CWARNING( sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) == 3 );
            BDBG_CWARNING( sizeof( BVCE_Channel_EncodeSettings ) == 60 );

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u,%u,%u,%u",
               pstEntry->data.stConfig.stStartEncodeSettings.stProtocolInfo.eProtocol,
               pstEntry->data.stConfig.stStartEncodeSettings.stProtocolInfo.eProfile,
               pstEntry->data.stConfig.stStartEncodeSettings.stProtocolInfo.eLevel,
               pstEntry->data.stConfig.stStartEncodeSettings.eInputType,
               pstEntry->data.stConfig.stEncodeSettings.uiNumSlicesPerPic,
               pstEntry->data.stConfig.stStartEncodeSettings.eForceEntropyCoding,
               pstEntry->data.stConfig.stStartEncodeSettings.stMemoryBandwidthSaving.bSingleRefP,
               pstEntry->data.stConfig.stStartEncodeSettings.stMemoryBandwidthSaving.bRequiredPatchesOnly
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
               pstEntry->data.stConfig.stStartEncodeSettings.uiStcIndex,
               pstEntry->data.stConfig.stStartEncodeSettings.uiRateBufferDelay,
               pstEntry->data.stConfig.stEncodeSettings.stFrameRate.eFrameRate,
               pstEntry->data.stConfig.stEncodeSettings.stFrameRate.bVariableFrameRateMode,
               pstEntry->data.stConfig.stEncodeSettings.stFrameRate.bSparseFrameRateMode,
               pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiMax,
               pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiTarget,
               pstEntry->data.stConfig.stEncodeSettings.uiA2PDelay,
               pstEntry->data.stConfig.stStartEncodeSettings.uiNumParallelNRTEncodes,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.bEnable,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.uiDuration,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiSlopeFactor,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiSlopeFactor,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stHrdMode.bDisableFrameDrop
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u,%u,%u",
               pstEntry->data.stConfig.stStartEncodeSettings.bNonRealTimeEncodeMode,
               pstEntry->data.stConfig.stStartEncodeSettings.bPipelineLowDelayMode,
               pstEntry->data.stConfig.stStartEncodeSettings.bAdaptiveLowDelayMode,
               pstEntry->data.stConfig.stEncodeSettings.bITFPEnable,
               pstEntry->data.stConfig.stSettingsModifiers.bOnInputChange,
               pstEntry->data.stConfig.stSettingsModifiers.bBeginNewRAP,
               pstEntry->data.stConfig.stSettingsModifiers.bFastChannelChange
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u,%u,%u",
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.bAllowNewGOPOnSceneChange,
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiMinGOPLengthAfterSceneChange,
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiDuration,
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiDurationRampUpFactor,
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiNumberOfPFrames,
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.uiNumberOfBFrames,
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.bAllowOpenGOP
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         case BVCE_DebugFifo_EntryType_eStatus:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%08x,%08x,%u,%u,%u,%u,%u,%u,%u,%u,"BDBG_UINT64_FMT,
               pstEntry->data.stStatus.uiTotalErrors,
               pstEntry->data.stStatus.uiErrorFlags,
               pstEntry->data.stStatus.uiEventFlags,
               pstEntry->data.stStatus.uiAverageFramesPerSecond,
               pstEntry->data.stStatus.uiTotalPicturesReceived,
               pstEntry->data.stStatus.uiTotalPicturesEncoded,
               pstEntry->data.stStatus.uiTotalPicturesDroppedDueToFrameRateConversion,
               pstEntry->data.stStatus.uiTotalPicturesDroppedDueToErrors,
               pstEntry->data.stStatus.uiTotalPicturesDroppedDueToHRDUnderflow,
               pstEntry->data.stStatus.uiLastPictureIdEncoded,
               pstEntry->data.stStatus.uiEtsDtsOffset,
               BDBG_UINT64_ARG(pstEntry->data.stStatus.uiSTCSnapshot)
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         case BVCE_DebugFifo_EntryType_eBufferDescriptor:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",0x%08x,%u,%u,%u,%u,%u,%u,%u,%u,%lu,%u,%u,%u,"BDBG_UINT64_FMT",%u,%u,%u,%u,%u,%d,%u,"BDBG_UINT64_FMT,
               pstEntry->data.stBufferDescriptor.stCommon.uiFlags,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EXTENDED)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME)),
               pstEntry->data.stBufferDescriptor.stCommon.uiOffset,
               (unsigned long) pstEntry->data.stBufferDescriptor.stCommon.uiLength,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiOriginalPTS,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID)),
               BDBG_UINT64_ARG(pstEntry->data.stBufferDescriptor.stCommon.uiPTS),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiESCR,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiTicksPerBit,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.iSHR,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID)),
               BDBG_UINT64_ARG(pstEntry->data.stBufferDescriptor.stCommon.uiSTCSnapshot)
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",0x%08x,%u,"BDBG_UINT64_FMT",%u,%u,%u,%u,%u",
               pstEntry->data.stBufferDescriptor.uiVideoFlags,
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID ) ),
               BDBG_UINT64_ARG(pstEntry->data.stBufferDescriptor.uiDTS),
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) ),
               pstEntry->data.stBufferDescriptor.uiDataUnitType,
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP ) ),
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_HIDDEN_FRAME ) ),
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_SHOW_FRAME ) )
             );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            break;

         case BVCE_DebugFifo_EntryType_eMetadataDescriptor:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",0x%08x",
               pstEntry->data.stMetadataDescriptor.uiMetadataFlags
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u",
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BUFFERINFO_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stBufferInfo.eProtocol,
               pstEntry->data.stMetadataDescriptor.stBufferInfo.eProfile,
               pstEntry->data.stMetadataDescriptor.stBufferInfo.eLevel
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stBitrate.uiMax
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_FRAMERATE_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stFrameRate.eFrameRateCode
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u",
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_DIMENSION_CODED_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stDimension.coded.uiWidth,
               pstEntry->data.stMetadataDescriptor.stDimension.coded.uiHeight
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,"BDBG_UINT64_FMT",%u,%u,%u,%u",
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_STC_VALID ) ),
               BDBG_UINT64_ARG(pstEntry->data.stMetadataDescriptor.stTiming.uiSTCSnapshot),
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_CHUNK_ID_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stTiming.uiChunkId,
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_ETS_DTS_OFFSET_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stTiming.uiEtsDtsOffset
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            for ( i = 0; i < BAVC_VIDEO_MAX_EXTRACTED_NALU_COUNT; i++ )
            {
               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",%u",
                  pstEntry->data.stMetadataDescriptor.stExtractedNALUInfo.astExtractedNALUEntry[i].uiNumValidBytes
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            }
            break;

         case BVCE_DebugFifo_EntryType_eITB:
            {
               unsigned i;

               for ( i = 0; i < 16; i++ )
               {
                  iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                     ",0x%02x",
                     pstEntry->data.stITB.auiEntry[i]
                  );
                  if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
               }
            }
            break;

         case BVCE_DebugFifo_EntryType_eCommand:
            {
               unsigned i;

               for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stCommand.data[0])].uiCommandSize; i++ )
               {
                  iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                     ",%s=%08x",
                     BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stCommand.data[0])].szCommandParameterName[i],
                     pstEntry->data.stCommand.data[i]
                  );
                  if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
               }
            }
            break;

         case BVCE_DebugFifo_EntryType_eResponse:
            {
                unsigned i;

                for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stResponse.data[0])].uiResponseSize; i++ )
                {
                   iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                      ",%s=%08x",
                      BVCE_P_CommandLUT[BVCE_Debug_S_CommandIndexLUT(pstEntry->data.stResponse.data[0])].szResponseParameterName[i],
                      pstEntry->data.stCommand.data[i]
                   );
                   if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
                }
             }
             break;

         case BVCE_DebugFifo_EntryType_eTrace0:
         case BVCE_DebugFifo_EntryType_eTrace1:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%s",
               pstEntry->data.szFunctionTrace
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         case BVCE_DebugFifo_EntryType_eITBOffsets:
         case BVCE_DebugFifo_EntryType_eCDBOffsets:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ","BDBG_UINT64_FMT","BDBG_UINT64_FMT","BDBG_UINT64_FMT","BDBG_UINT64_FMT","BDBG_UINT64_FMT","BDBG_UINT64_FMT,
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiBase),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiEnd),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiRead),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiShadowRead),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiValid),
               BDBG_UINT64_ARG(pstEntry->data.stOffset.uiWrite)
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         case BVCE_DebugFifo_EntryType_ePictureBufferEnqueue:
            BDBG_CWARNING( sizeof( BVCE_P_FW_PictureBufferMailbox ) == 4*27 );

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.uiResolution & BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Resolution_HORIZONTAL_SIZE_IN_8x8_BLOCKS_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiResolution & BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Resolution_VERTICAL_SIZE_IN_8x8_BLOCKS_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.uiCropping & BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Cropping_HORIZONTAL_SIZE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiCropping & BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Cropping_VERTICAL_SIZE_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.uiSampleAspectRatio & BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_HORIZONTAL_SIZE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiSampleAspectRatio & BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_SampleAspectRatio_VERTICAL_SIZE_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.uiBarData & BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_BarData_TYPE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiBarData & BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_BarData_TOP_LEFT_VALUE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiBarData & BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_BarData_BOTTOM_RIGHT_VALUE_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_ORIENTATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_VALID_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_VALID_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_AFD_MODE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_TYPE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_CADENCE_PHASE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiFormatInfo & BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_MASK ) >> BVCE_P_FW_PictureBufferMailbox_FormatInfo_POLARITY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u",
               ( pstEntry->data.stPictureBufferMailbox.uiTimingInfo & BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_MASK ) >> BVCE_P_FW_PictureBufferMailbox_TimingInfo_DELTA_PTS_IN_360KHZ_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ","BDBG_UINT64_FMT,
               BDBG_UINT64_ARG( ( ((uint64_t) pstEntry->data.stPictureBufferMailbox.uiNewPts) << 1 ) | ( ( pstEntry->data.stPictureBufferMailbox.uiTimingInfo & BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_MASK ) >> BVCE_P_FW_PictureBufferMailbox_TimingInfo_NEW_PTS_LSB_SHIFT) )
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u",
               pstEntry->data.stPictureBufferMailbox.uiOriginalPts
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u",
               pstEntry->data.stPictureBufferMailbox.uiPictureId
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ","BDBG_UINT64_FMT,
               BDBG_UINT64_ARG( ( ((uint64_t) pstEntry->data.stPictureBufferMailbox.uiSTCSnapshotHi) << 32 ) | pstEntry->data.stPictureBufferMailbox.uiSTCSnapshotLo )
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.uiMetadata & BVCE_P_FW_PictureBufferMailbox_Metadata_LAST_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Metadata_LAST_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiMetadata & BVCE_P_FW_PictureBufferMailbox_Metadata_CHANNEL_CHANGE_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Metadata_CHANNEL_CHANGE_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.uiMetadata & BVCE_P_FW_PictureBufferMailbox_Metadata_BUSY_MASK ) >> BVCE_P_FW_PictureBufferMailbox_Metadata_BUSY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         case BVCE_DebugFifo_EntryType_ePictureBufferDequeue:
            BDBG_CWARNING( sizeof( BVCE_P_FW_PictureBufferMailbox ) == 4*27 );

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.LumaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma1VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DECIMATION_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.Luma2VBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u",
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetHi ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.stOffset.uiOffsetLo ),
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_DCXV_SHIFT,
               ( pstEntry->data.stPictureBufferMailbox.ShiftedChromaBufPtr.uiMetadata & BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_MASK ) >> BVCE_P_FW_PictureBufferInfo_Metadata_NMBY_SHIFT
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         case BVCE_DebugFifo_EntryType_ePictureBufferDropEnqueue:
         case BVCE_DebugFifo_EntryType_ePictureBufferDropDequeue:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               0,
               ( pstEntry->data.stPicture.ulLumaOffset )
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               0,
               ( pstEntry->data.stPicture.ulChromaOffset )
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               0,
               ( pstEntry->data.stPicture.ul1VLumaOffset )
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               0,
               ( pstEntry->data.stPicture.ul2VLumaOffset )
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u",
               0,
               ( pstEntry->data.stPicture.ulShiftedChromaOffset )
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         default:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               "<<< Unknown Debug Log Type >>>"
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            BDBG_LOG(("Unrecognized debug entry type: %d", pstEntry->stMetadata.eType));
            break;
      }
      iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
         "\n"
      );
      if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
      goto dbg_fmt_done;

   dbg_fmt_overflow:
            BDBG_LOG(("Format Debug String Overflow"));

   dbg_fmt_done:
      return;
   }
}

unsigned
BVCE_Debug_GetEntrySize( void )
{
   return sizeof( BVCE_P_DebugFifo_Entry );
}
