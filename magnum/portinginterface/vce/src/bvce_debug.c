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

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */

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

unsigned BVCE_Debug_P_CommandIndexLUT(
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

void
BVCE_Debug_PrintLogMessageEntry(
      const void *pstFifoEntry /* Should point to an element of size BVCE_Debug_FifoInfo.uiElementSize */
      )
{
   const BVCE_P_DebugFifo_Entry *pstEntry = (BVCE_P_DebugFifo_Entry *) pstFifoEntry;

   BDBG_ASSERT( pstEntry );

   switch ( pstEntry->stMetadata.eType )
   {
      case BVCE_DebugFifo_EntryType_eConfig:
         BDBG_CWARNING( sizeof( BVCE_Channel_StartEncodeSettings ) == 132 );
         BDBG_CWARNING( sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) == 3 );
         BDBG_CWARNING( sizeof( BVCE_Channel_EncodeSettings ) == 56 );

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

         BDBG_MODULE_MSG(BVCE_DBG_CFG, ("(%10u)[%u][%u] TIMING: stc=%u,rate_buffer_delay=%u,fr=%u,vfr=%u,br_max=%u,br_target=%u,A2PDelay=%u,num_parallel_encodes=%u,segment_rc=%u,segment_duration=%u,segment_upper_tolerance=%u,segment_upper_slope_factor=%u,segment_lower_tolerance=%u,segment_lower_slope_factor=%u",
            pstEntry->stMetadata.uiTimestamp,
            pstEntry->stMetadata.uiInstance,
            pstEntry->stMetadata.uiChannel,
            pstEntry->data.stConfig.stStartEncodeSettings.uiStcIndex,
            pstEntry->data.stConfig.stStartEncodeSettings.uiRateBufferDelay,
            pstEntry->data.stConfig.stEncodeSettings.stFrameRate.eFrameRate,
            pstEntry->data.stConfig.stEncodeSettings.stFrameRate.bVariableFrameRateMode,
            pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiMax,
            pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiTarget,
            pstEntry->data.stConfig.stEncodeSettings.uiA2PDelay,
            pstEntry->data.stConfig.stStartEncodeSettings.uiNumParallelNRTEncodes,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.bEnable,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.uiDuration,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiSlopeFactor,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance,
            pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiSlopeFactor
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

         BDBG_MODULE_MSG(BVCE_DBG_CFG, ("(%10u)[%u][%u] GOP: restart=%u,duration=%u,duration_ramp_up=%u,p=%u,b=%u,open_gop=%u",
            pstEntry->stMetadata.uiTimestamp,
            pstEntry->stMetadata.uiInstance,
            pstEntry->stMetadata.uiChannel,
            pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.bAllowNewGOPOnSceneChange,
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
#define BVCE_DEBUG_STRING_SIZE 256
            char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
            signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "[ "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START ) )  ? "frm " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START ) )  ? "seg " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS ) )  ? "eos " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME ) )  ? "nul " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_END ) )  ? "end " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC ) )  ? "eoc " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA ) )  ? "mta " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EXTENDED ) )  ? "xtn " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP ) )  ? "rap " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) )  ? "dut " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%s",
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_EXTENDED ) )  ? "xtn " : "    "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "] "
            );
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "opts=0x%08x ",
                  pstEntry->data.stBufferDescriptor.stCommon.uiOriginalPTS
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "pts=0x%08x%08x ",
                  (uint32_t) (pstEntry->data.stBufferDescriptor.stCommon.uiPTS >> 32),
                  (uint32_t) pstEntry->data.stBufferDescriptor.stCommon.uiPTS
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "dts=0x%08x%08x ",
                  (uint32_t) (pstEntry->data.stBufferDescriptor.uiDTS >> 32),
                  (uint32_t) pstEntry->data.stBufferDescriptor.uiDTS
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "escr=0x%08x ",
                  pstEntry->data.stBufferDescriptor.stCommon.uiESCR
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "tpb=0x%08x ",
                  pstEntry->data.stBufferDescriptor.stCommon.uiTicksPerBit
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "shr=%d ",
                  pstEntry->data.stBufferDescriptor.stCommon.iSHR
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "stc=0x%08x%08x ",
                  (uint32_t) (pstEntry->data.stBufferDescriptor.stCommon.uiSTCSnapshot >> 32),
                  (uint32_t) pstEntry->data.stBufferDescriptor.stCommon.uiSTCSnapshot
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            if ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) )
            {
               iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
                  "dut=%u ",
                  pstEntry->data.stBufferDescriptor.uiDataUnitType
               );
            }
            if ( iBytesLeft < 0 ) { goto dbg_buf_overflow; }

            goto dbg_buf_done;
dbg_buf_overflow:
            BDBG_MODULE_MSG(BVCE_DBG_BUF, ("Debug String Overflow"));
dbg_buf_done:
            BDBG_MODULE_MSG(BVCE_DBG_BUF, ("(%10u)[%u][%u] offset=%08x length=%08x %s",
               pstEntry->stMetadata.uiTimestamp,
               pstEntry->stMetadata.uiInstance,
               pstEntry->stMetadata.uiChannel,
               pstEntry->data.stBufferDescriptor.stCommon.uiOffset,
               pstEntry->data.stBufferDescriptor.stCommon.uiLength,
               szDebug
            ));
         }
         break;

      case BVCE_DebugFifo_EntryType_eMetadataDescriptor:
      {
#define BVCE_DEBUG_STRING_SIZE 256
            char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
            signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;

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
#define BVCE_DEBUG_STRING_SIZE 256
         char szDebug[BVCE_DEBUG_STRING_SIZE] = "";
         signed iBytesLeft = BVCE_DEBUG_STRING_SIZE;
         unsigned i;
         for ( i = 0; i < 16; i++ )
         {
            iBytesLeft -= BKNI_Snprintf( &szDebug[BVCE_DEBUG_STRING_SIZE-iBytesLeft], iBytesLeft,
               "%02x ",
               pstEntry->data.auiITB[i]
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

            for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stCommand.data[0])].uiCommandSize; i++ )
            {
               BDBG_MODULE_MSG( BVCE_DBG_CMD, ("(%10u)[%u][%u] %08x - %s",
                  pstEntry->stMetadata.uiTimestamp,
                  pstEntry->stMetadata.uiInstance,
                  pstEntry->stMetadata.uiChannel,
                  pstEntry->data.stCommand.data[i],
                  BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stCommand.data[0])].szCommandParameterName[i]
               ));
            }
         }
         break;

      case BVCE_DebugFifo_EntryType_eResponse:
         {
            unsigned i;
            for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stResponse.data[0])].uiResponseSize; i++ )
            {
               BDBG_MODULE_MSG( BVCE_DBG_RSP, ("(%10u)[%u][%u] %08x - %s",
                  pstEntry->stMetadata.uiTimestamp,
                  pstEntry->stMetadata.uiInstance,
                  pstEntry->stMetadata.uiChannel,
                  pstEntry->data.stCommand.data[i],
                  BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stResponse.data[0])].szResponseParameterName[i]
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

      default:
         BDBG_LOG(("Unrecognized debug entry type: %d", pstEntry->stMetadata.eType));
         break;
   }
}

bool
BVCE_Debug_FormatLogHeader(
   unsigned uiIndex,
   char *szMessage,
   size_t uiSize
   )
{
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   {
      bool bMore = true;
      signed iBytesLeft = uiSize;
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

         iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
            "%u,%s,instance,channel",
            eType,
            BVCE_P_Debug_EntryTypeLUT[eType]
         );
         if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

         /* Print the header */
         switch ( eType )
         {
            case BVCE_DebugFifo_EntryType_eConfig:
               BDBG_CWARNING( sizeof( BVCE_Channel_StartEncodeSettings ) == 132 );
               BDBG_CWARNING( sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) == 3 );
               BDBG_CWARNING( sizeof( BVCE_Channel_EncodeSettings ) == 56 );

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",protocol,profile,level,input type,num slices,force entropy coding,single ref P,required patches only"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",stc index,rate buffer delay,fr,vfr,br_max,br_target,A2PDelay,num parallel encodes,segment rc,segment duration,segment upper tolerance,segment upper slope factor,segment lower tolerance,segment lower slope factor"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",nrt,pipeline low delay,adaptive low delay,itfp,on input change,new rap,fast channel change"
               );
               if ( iBytesLeft < 0 ) { goto dbg_fmt_hdr_overflow; }

               iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                  ",restart,duration,duration ramp up,p frames,b frames,open gop"
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
                  ",vflags,dts valid,dts,dut start,dut,rai"
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
   size_t uiSize
   )
{
   BDBG_ASSERT( szMessage );
   BDBG_ASSERT( uiSize > 0 );

   {
      const BVCE_P_DebugFifo_Entry *pstEntry = (BVCE_P_DebugFifo_Entry *) pstFifoEntry;
      signed iBytesLeft = uiSize;
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

      switch ( pstEntry->stMetadata.eType )
      {
         case BVCE_DebugFifo_EntryType_eConfig:
            BDBG_CWARNING( sizeof( BVCE_Channel_StartEncodeSettings ) == 132 );
            BDBG_CWARNING( sizeof( BVCE_P_SendCommand_ConfigChannel_Settings ) == 3 );
            BDBG_CWARNING( sizeof( BVCE_Channel_EncodeSettings ) == 56 );

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",%u,%u,%u,%u,%u,%u,%u,%u,%u",
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
               ",%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
               pstEntry->data.stConfig.stStartEncodeSettings.uiStcIndex,
               pstEntry->data.stConfig.stStartEncodeSettings.uiRateBufferDelay,
               pstEntry->data.stConfig.stEncodeSettings.stFrameRate.eFrameRate,
               pstEntry->data.stConfig.stEncodeSettings.stFrameRate.bVariableFrameRateMode,
               pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiMax,
               pstEntry->data.stConfig.stEncodeSettings.stBitRate.uiTarget,
               pstEntry->data.stConfig.stEncodeSettings.uiA2PDelay,
               pstEntry->data.stConfig.stStartEncodeSettings.uiNumParallelNRTEncodes,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.bEnable,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.uiDuration,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiTolerance,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stUpper.uiSlopeFactor,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiTolerance,
               pstEntry->data.stConfig.stStartEncodeSettings.stRateControl.stSegmentMode.stTargetBitRatePercentage.stLower.uiSlopeFactor
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
               ",%u,%u,%u,%u,%u,%u",
               pstEntry->data.stConfig.stEncodeSettings.stGOPStructure.bAllowNewGOPOnSceneChange,
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
               ",%u,%08x,%08x,%u,%u,%u,%u,%u,%u,%u,%u,%llu",
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
               pstEntry->data.stStatus.uiSTCSnapshot
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
            break;

         case BVCE_DebugFifo_EntryType_eBufferDescriptor:
            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",0x%08x,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%llu,%u,%u,%u,%u,%u,%d,%u,%llu",
               pstEntry->data.stBufferDescriptor.stCommon.uiFlags,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EXTENDED)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SEGMENT_START)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOC)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS)),
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EMPTY_FRAME)),
               pstEntry->data.stBufferDescriptor.stCommon.uiOffset,
               pstEntry->data.stBufferDescriptor.stCommon.uiLength,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiOriginalPTS,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiPTS,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiESCR,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_TICKSPERBIT_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiTicksPerBit,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_SHR_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.iSHR,
               (0 != (pstEntry->data.stBufferDescriptor.stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_STCSNAPSHOT_VALID)),
               pstEntry->data.stBufferDescriptor.stCommon.uiSTCSnapshot
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
               ",0x%08x,%u,%llu,%u,%u,%u",
               pstEntry->data.stBufferDescriptor.uiVideoFlags,
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID ) ),
               pstEntry->data.stBufferDescriptor.uiDTS,
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START ) ),
               pstEntry->data.stBufferDescriptor.uiDataUnitType,
               ( 0 != ( pstEntry->data.stBufferDescriptor.uiVideoFlags & BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP ) )
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
               ",%u,%llu,%u,%u,%u,%u",
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_STC_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stTiming.uiSTCSnapshot,
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_CHUNK_ID_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stTiming.uiChunkId,
               ( 0 != ( pstEntry->data.stMetadataDescriptor.uiMetadataFlags & BAVC_VIDEOMETADATADESCRIPTOR_FLAGS_TIMING_ETS_DTS_OFFSET_VALID ) ),
               pstEntry->data.stMetadataDescriptor.stTiming.uiEtsDtsOffset
            );
            if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }

            break;

         case BVCE_DebugFifo_EntryType_eITB:
            {
               unsigned i;

               for ( i = 0; i < 16; i++ )
               {
                  iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                     ",0x%02x",
                     pstEntry->data.auiITB[i]
                  );
                  if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
               }
            }
            break;

         case BVCE_DebugFifo_EntryType_eCommand:
            {
               unsigned i;

               for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stCommand.data[0])].uiCommandSize; i++ )
               {
                  iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                     ",%s=%08x",
                     BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stCommand.data[0])].szCommandParameterName[i],
                     pstEntry->data.stCommand.data[i]
                  );
                  if ( iBytesLeft < 0 ) { goto dbg_fmt_overflow; }
               }
            }
            break;

         case BVCE_DebugFifo_EntryType_eResponse:
            {
                unsigned i;

                for ( i = 0; i < BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stResponse.data[0])].uiResponseSize; i++ )
                {
                   iBytesLeft -= BKNI_Snprintf( &szMessage[uiSize-iBytesLeft], iBytesLeft,
                      ",%s=%08x",
                      BVCE_P_CommandLUT[BVCE_Debug_P_CommandIndexLUT(pstEntry->data.stResponse.data[0])].szResponseParameterName[i],
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
