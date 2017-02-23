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

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bmuxlib_ts.h"
#include "bmuxlib_ts_priv.h"

#include "bmuxlib_input.h"

#if BMUXLIB_TS_P_TEST_MODE
#include <stdio.h>
#endif

BDBG_MODULE(BMUXLIB_TS);
BDBG_FILE_MODULE(BMUXLIB_TS_FINISH);
BDBG_FILE_MODULE(BMUXLIB_TS_PENDING);
BDBG_FILE_MODULE(BMUXLIB_TS_QUEUED);
BDBG_FILE_MODULE(BMUXLIB_TS_TRANSPORT);
BDBG_FILE_MODULE(BMUXLIB_TS_MSP);
BDBG_FILE_MODULE(BMUXLIB_TS_MEMORY);
BDBG_FILE_MODULE(BMUXLIB_TS_STATS);

BDBG_OBJECT_ID(BMUXlib_TS_P_Context);

static bool BMUXlib_TS_P_ValidateStartSettings(BMUXlib_TS_StartSettings *pstStartSettings, bool bUseDefaults);
static bool BMUXlib_TS_P_ValidateMuxSettings(BMUXlib_TS_MuxSettings *pstMuxSettings, BMUXlib_TS_StartSettings *pstStartSetting, bool bUseDefaults);
static bool BMUXlib_TS_P_ValidateSystemDataBitrate(BMUXlib_TS_MuxSettings *pstMuxSettings, const BMUXlib_TS_StartSettings *pstStartSettings, bool bUseDefaults);
static BERR_Code BMUXlib_TS_P_Finish(BMUXlib_TS_Handle hMuxTS, const BMUXlib_TS_FinishSettings *pstFinishSettings);

/******************/
/* Create/Destroy */
/******************/

#define BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT 5
#define BMUXLIB_TS_P_NOMINAL_SYSTEM_DATA_BITRATE 2000000
#define BMUXLIB_TS_P_NOMINAL_PCR_INTERVAL 20
#define BMUXLIB_TS_P_NOMINAL_USERDATA_PIDS 4
#define BMUXLIB_TS_P_MAX_NRT_SPEED 4
#define BMUXLIB_TS_P_MSP_COUNT 2
#define BMUXLIB_TS_P_MIN_DESCRIPTORS_PER_FRAME 2 /* for PES and CDB */
#define BMUXLIB_TS_P_EXTRA_DESCRIPTORS_PER_MSP 2 /* partial frame and/or CDB wrap */
#define BMUXLIB_TS_P_MAX_VIDEO_FPS 60
#define BMUXLIB_TS_P_MAX_AUDIO_FPS 50

#define BMUXLIB_TS_P_MAX(a,b) ((a > b) ? a : b)
#define BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(a,b) ((b)?(((a) + ((b)-1))/(b)):0)

void
BMUXlib_TS_P_GetMemoryConfigTotal(
   const BMUXlib_TS_P_MemoryConfig *pstMemoryConfig,
   BMUXlib_TS_P_MemoryConfigTotal *pstMemoryConfigTotal
)
{
   BDBG_ASSERT( pstMemoryConfig );
   BDBG_ASSERT( pstMemoryConfigTotal );

   BKNI_Memset( pstMemoryConfigTotal, 0, sizeof( BMUXlib_TS_P_MemoryConfigTotal ) );
   {
      BMUXlib_TS_P_MemoryEntryType eMemoryEntryType;
      for ( eMemoryEntryType = 0; eMemoryEntryType < BMUXlib_TS_P_MemoryEntryType_eMax; eMemoryEntryType++ )
      {
         BMUXlib_TS_P_InputType eInputType;
         for ( eInputType=0; eInputType<BMUXlib_TS_P_InputType_eMax; eInputType++ )
         {
            BMUXlib_TS_P_MemoryType eMemoryType;
            for ( eMemoryType=0; eMemoryType<BMUXlib_TS_P_MemoryType_eMax; eMemoryType++ )
            {
               pstMemoryConfigTotal->astMemoryEntry[eMemoryEntryType].stMemoryConfig.stBufferInfo[eMemoryType].uiSize += pstMemoryConfig->astMemoryEntry[eMemoryEntryType][eInputType].stMemoryConfig.stBufferInfo[eMemoryType].uiSize;
            }
            pstMemoryConfigTotal->astMemoryEntry[eMemoryEntryType].uiCount += pstMemoryConfig->astMemoryEntry[eMemoryEntryType][eInputType].uiCount;
         }
      }
   }

   pstMemoryConfigTotal->stMemoryConfig = pstMemoryConfig->stMemoryConfig;
}

void
BMUXlib_TS_P_Input_GetMemoryConfig(
   BMUXlib_TS_P_InputType eInputType,
   unsigned uiNumFrames,
   unsigned uiNumPIDs,
   unsigned uiNumMSPs,
   bool bSupportTTS,
   BMUXlib_TS_P_MemoryConfig *pstMemoryConfig
   )
{
   /************/
   /* CDB Data */
   /************/
   /* We need one PES header for each frame */
   pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_ePESHeader][eInputType].uiCount += uiNumFrames;
   /* We need at least 2 transport descriptors per frame for PES + CDB.
    */
   pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][eInputType].uiCount += (uiNumFrames * BMUXLIB_TS_P_MIN_DESCRIPTORS_PER_FRAME);

   /* We need then 1 additional transport descriptor per MSP for each of:
    *    1) partial frame
    *    2) cdb wrap
    */
   pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][eInputType].uiCount += ( BMUXLIB_TS_P_EXTRA_DESCRIPTORS_PER_MSP * uiNumMSPs );

   if ( BMUXlib_TS_P_InputType_eVideo == eInputType )
   {
      /****************/
      /* MTU BPP Data */
      /****************/
      /* SW7425-659:  We need to account for worst case MUX_TIMESTAMP_UPDATE BPP which is once every frame ( 1 BPP and 1 transport descriptor per frame ) */
      if ( true == bSupportTTS )
      {
         /* SW7425-5180: Use a separate MTU BPP memory resource to handle 16x consecutive BPPs to maintain CC count */
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eMTUBPP][eInputType].uiCount += uiNumFrames;
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][eInputType].uiCount += uiNumFrames;
      }
   }

   /*****************/
   /* LAST BPP Data */
   /*****************/
   /* Allocate LAST BPP + transport descriptor */
   pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eBPP][eInputType].uiCount += 1;
   pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][eInputType].uiCount += 1;

   /*****************/
   /* Multiple PIDs */
   /*****************/
   {
      BMUXlib_TS_P_MemoryEntryType eEntryType;

      for ( eEntryType = 0; eEntryType < BMUXlib_TS_P_MemoryEntryType_eMax; eEntryType++ )
      {
         /* We need to multiply the totals by the number of PIDs */
         pstMemoryConfig->astMemoryEntry[eEntryType][eInputType].uiCount *= uiNumPIDs;
      }
   }
}

void
BMUXlib_TS_P_GetMemoryConfig(
   const BMUXlib_TS_MuxConfig *pstMuxConfig,
   BMUXlib_TS_P_MemoryConfig *pstMemoryConfig
)
{
   bool rc1, rc2;
   BMUXlib_TS_MuxSettings stMuxSettings, *pstMuxSettings = &stMuxSettings;
   BMUXlib_TS_StartSettings stStartSettings, *pstStartSettings = &stStartSettings;

   BDBG_ENTER( BMUXlib_TS_P_GetMemoryConfig );

   BDBG_ASSERT(pstMuxConfig);
   BDBG_ASSERT(pstMemoryConfig);

   BKNI_Memset( pstMemoryConfig, 0, sizeof( BMUXlib_TS_P_MemoryConfig ) );
   /* copy the settings, so that we can override invalid settings with defaults */
   stMuxSettings = pstMuxConfig->stMuxSettings;
   stStartSettings = pstMuxConfig->stMuxStartSettings;

   /* validate settings, but only print warnings for invalid values (use defaults)
      (return code is used to indicate that a change was made to a setting) */
   /* IMPORTANT: Start settings must be validated first.  This is due to the dependency
      of the mux settings check (system data bitrate check) on PCR interval */
   rc1 = BMUXlib_TS_P_ValidateStartSettings(pstStartSettings, true);
   rc2 = BMUXlib_TS_P_ValidateMuxSettings(pstMuxSettings, pstStartSettings, true);
   if (rc1 || rc2)
      BDBG_WRN(("GetMemoryConfig:: Settings have been altered (memory calculations may not match specified settings)"));

   /* Determine number of descriptors required */
   /* MSP: 100 ms max
    * Video: 60 frames/sec max
    * Audio: ~50 frames/sec max
    * System: 1 Mbps/sec
    *    pcr: 20 pcrs/sec max (50 ms interval)
    *    system/user data: 1 Mbps/sec * sec/1000ms * 100ms/MSP * byte/8 bits * packet/188 bytes = ~ 67 packets/sec max
    *
    *    - 1 PES Header/frame
    *    - 2 Transport Descriptors/frame
    *    - 2 Additional Transport Descriptors for:
    *       - partial frame and/or
    *       - CDB wrap and/or
    *    - 1 MUX_TIMESTAMP_UPDATE BPP per A/V frame
    *    - 1 additional transport descriptor per A/V BPP
    *    - 1 BTP + transport descriptor per MSP
    *    - 1 additional transport descriptor per BTP
    *    video: = 60 frames/sec * sec/1000ms * 100ms/MSP = 6 frames/MSP
    *       --> 6 PES/MSP
    *       --> 12+6+3 Transport Descriptors/MSP
    *       --> 6 BPP
    *    audio:
    *       --> 5 PES/MSP
    *       --> 10+5+3 Transport Descriptors/MSP
    *       --> 5 BPP
    *    system:
    *       --> 20+1 TS Packets/MSP (pcr + btp)
    *       --> 67+1 Transport Descriptors/MSP (pcr + btp)
    *       --> 67 System Data Buffers/MSP
    *
    */

   {
      /* For resource allocation purposes, we need to make sure we have enough resources to schedule enough frames for the
       * worst case next execution time.  This is the MSP + service latency tolerance (SLT).  I.e. If DoMux() is called at T=0,
       * ideally, we should be executed again at T=MSP, however, because of host CPU latency, we need to tolerate up to
       * T=MSP+SLT.
       */
      unsigned uiWorstCaseNextExecutionTime = (pstStartSettings->uiServicePeriod + pstStartSettings->uiServiceLatencyTolerance);
      unsigned uiTotalMSPCount = BMUXLIB_TS_P_MSP_COUNT;

      /* SW7435-680: Account for delay of descriptors because of the mux delay.
       * E.g. if service latency tolerance is 20ms, MSP=50ms, mux_delay=70ms, then descriptors
       * sent on MSP[0] won't be released until at least 70ms later, which is MSP[2]. */
      uiTotalMSPCount += BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(uiWorstCaseNextExecutionTime, pstStartSettings->uiServicePeriod);

      /* SW7435-535: Account for additional MSP delay because of pre-mature descriptor release work-around
       * Because we are delaying the release of descriptors, we may not get back all the descriptors until
       * the next MSP */
      uiTotalMSPCount += 1;

      if ( true == stStartSettings.bSupportTTS )
      {
         /* SW7425-659: Increase by an additional MSP count to account for latency MTU BPP is enabled in RT mode */
         uiTotalMSPCount += 1;
      }

      /* Video */
      if ( 0 != pstStartSettings->uiNumValidVideoPIDs )
      {
         /* SW7425-5180/SW7425-4707: In order to calculate the worst case number of video frames we need to have queued, we
          * need to consider the end-to-end delay (a.k.a A2PDelay, Dee) and the worst case encoder output frame rate (60fps).
          * We need to consider end-to-end delay because the encoder may generate frames at a rate faster than the frame rate
          * when the content is simple/static.
          */
         unsigned uiDurationms = (pstStartSettings->uiA2PDelay / BMUXLIB_TS_P_SCALE_MS_TO_27MHZ) + (uiWorstCaseNextExecutionTime * uiTotalMSPCount);
         unsigned uiNumFrames =  BMUXLIB_TS_P_MAX(2,BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP((BMUXLIB_TS_P_MAX_VIDEO_FPS * uiDurationms),1000));
         if ( true == pstStartSettings->bNonRealTimeMode ) uiNumFrames *= BMUXLIB_TS_P_MAX_NRT_SPEED;

         BMUXlib_TS_P_Input_GetMemoryConfig(
            BMUXlib_TS_P_InputType_eVideo,
            uiNumFrames,
            pstStartSettings->uiNumValidVideoPIDs,
            uiTotalMSPCount,
            pstStartSettings->bSupportTTS,
            pstMemoryConfig
            );
      }

      /* Audio */
      if ( 0 != pstStartSettings->uiNumValidAudioPIDs )
      {
         unsigned uiNumFrames = BMUXLIB_TS_P_MAX(2,BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP((BMUXLIB_TS_P_MAX_AUDIO_FPS * uiWorstCaseNextExecutionTime),1000)) * uiTotalMSPCount;
         if ( true == pstStartSettings->bNonRealTimeMode ) uiNumFrames *= BMUXLIB_TS_P_MAX_NRT_SPEED;

         BMUXlib_TS_P_Input_GetMemoryConfig(
            BMUXlib_TS_P_InputType_eAudio,
            uiNumFrames,
            pstStartSettings->uiNumValidAudioPIDs,
            uiTotalMSPCount,
            pstStartSettings->bSupportTTS,
            pstMemoryConfig
            );
      }

      /* System */
      {
         unsigned uiNumFrames = BMUXLIB_TS_P_MAX(2,BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(pstMuxSettings->uiSystemDataBitRate * uiWorstCaseNextExecutionTime,1000 * 8 * 188)) * uiTotalMSPCount;
         if ( true == pstStartSettings->bNonRealTimeMode ) uiNumFrames *= BMUXLIB_TS_P_MAX_NRT_SPEED;
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][BMUXlib_TS_P_InputType_eSystem].uiCount = uiNumFrames;
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eSystemData][BMUXlib_TS_P_InputType_eSystem].uiCount = uiNumFrames;
      }
      {
         unsigned uiNumPcrs = BMUXLIB_TS_P_MAX(2,BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(uiWorstCaseNextExecutionTime, ( 0 != pstStartSettings->stPCRData.uiInterval ) ? pstStartSettings->stPCRData.uiInterval : BMUXLIB_TS_P_MUX_PCR_INTERVAL_DEFAULT)) * uiTotalMSPCount;
         if ( true == pstStartSettings->bNonRealTimeMode ) uiNumPcrs *= BMUXLIB_TS_P_MAX_NRT_SPEED;
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket][BMUXlib_TS_P_InputType_eSystem].uiCount = uiNumPcrs;
      }

      if ( true == stStartSettings.bSupportTTS )
      /* SW7425-659: We need to send one MTU BTP with the very first system data packet to ensure a non-zero deltaATS */
      {
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][BMUXlib_TS_P_InputType_eSystem].uiCount += 1;
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket][BMUXlib_TS_P_InputType_eSystem].uiCount += 1;
      }

      /* SW7425-4764: Allocate a single TS Packet for use as a NULL Packet and a BPP for use as a dummy PES frame */
      {
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket][BMUXlib_TS_P_InputType_eSystem].uiCount += 1;
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eBPP][BMUXlib_TS_P_InputType_eSystem].uiCount += 1;
      }

      /* TS User Data */
      {
         /* SW7425-4340: allow for encoder delay (A2P) to allow for buffering of userdata due to diff between encode ESCR and encode PTS */
         unsigned uiDurationms = (pstStartSettings->uiA2PDelay / BMUXLIB_TS_P_SCALE_MS_TO_27MHZ) + (uiWorstCaseNextExecutionTime * uiTotalMSPCount);
         unsigned uiNumFrames =  BMUXLIB_TS_P_MAX(2,BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP((BMUXLIB_TS_P_MAX_VIDEO_FPS * uiDurationms),1000));
         unsigned uiNumPackets = uiNumFrames * BMUXLIB_TS_USERDATA_MAX_PKTS_PER_VID;
         if (true == pstStartSettings->bNonRealTimeMode) uiNumPackets *= BMUXLIB_TS_P_MAX_NRT_SPEED;

         /* Need 1 unwrap packet per PID */
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataUnwrap][BMUXlib_TS_P_InputType_eSystem].uiCount = pstStartSettings->uiNumValidUserdataPIDs;
         /* 1 pending Q entry per packet */
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserData][BMUXlib_TS_P_InputType_eSystem].uiCount = uiNumPackets * pstStartSettings->uiNumValidUserdataPIDs;
         /* 1 release Q entry per segment */
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataReleaseQ][BMUXlib_TS_P_InputType_eSystem].uiCount = uiNumPackets * BMUXLIB_TS_USERDATA_MAX_SEGMENTS * pstStartSettings->uiNumValidUserdataPIDs;
         /* 2 PTS entries per TS packet (one for PTS, one for DTS - assume worst case of 1 PES = 1 TS packet) */
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataPTS][BMUXlib_TS_P_InputType_eSystem].uiCount = 2 * uiNumPackets * pstStartSettings->uiNumValidUserdataPIDs;
         /* 1 Transport Desc per Release Q entry (NOTE: one entry already accounted for by System Data) */
         pstMemoryConfig->astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][BMUXlib_TS_P_InputType_eSystem].uiCount += (uiNumPackets * (BMUXLIB_TS_USERDATA_MAX_SEGMENTS-1) * pstStartSettings->uiNumValidUserdataPIDs);
      }
   }

   {
      BMUXlib_TS_P_MemoryEntryType eMemoryEntryType;

      for ( eMemoryEntryType = 0; eMemoryEntryType < BMUXlib_TS_P_MemoryEntryType_eMax; eMemoryEntryType++ )
      {
         BMUXlib_TS_P_InputType eInputType;

         for ( eInputType=0; eInputType<BMUXlib_TS_P_InputType_eMax; eInputType++ )
         {
#define BMUXLIB_TS_P_ADD_MEMORY_SIZE(size,p,i,j,type) (p)[i][j].stMemoryConfig.stBufferInfo[type].uiSize += BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP( size * (p)[i][j].uiCount, 4096 ) * 4096

            switch ( eMemoryEntryType )
            {
               case BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor:
                  /* Transport Descriptors */
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_TransportDescriptor ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eSystem);
                  /* Transport Descriptor Temp Array */
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_TransportDescriptor ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eSystem);
                  /* Transport Descriptor Metadata Array */
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_TransportDescriptorMetaData ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eSystem);
                  /* Transport Descriptor Metadata Temp Array */
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_TransportDescriptorMetaData ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eSystem);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_ePESHeader:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_PESHeader ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eShared);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eTransportPacket:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_TSPacket ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eShared);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eSystemData:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_SystemData ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eShared);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eBPP:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_BPPData ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eShared);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eUserData:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_UserdataPending ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eSystem);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eUserDataPTS:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof(BMUXlib_TS_P_UserdataPTSEntry), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eShared);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eUserDataUnwrap:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof(BMUXlib_TS_P_TSPacket), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eShared);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eUserDataReleaseQ:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_UserdataReleaseQEntry ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eSystem);
                  break;

               case BMUXlib_TS_P_MemoryEntryType_eMTUBPP:
                  BMUXLIB_TS_P_ADD_MEMORY_SIZE(sizeof( BMUXlib_TS_P_MTUBPPData ), pstMemoryConfig->astMemoryEntry,eMemoryEntryType,eInputType,BMUXlib_TS_P_MemoryType_eShared);
                  break;

               /* coverity[dead_error_begin] */
               /* SW7435-1313: Intentional dead code: catch-all for added enums */
               default:
                  BDBG_ERR(("Unsupported memory entry type (%d)", eMemoryEntryType ));
                  break;
            } /* end: switch memory entry type */

            {
               BMUXlib_TS_P_MemoryType eMemoryType;
               for ( eMemoryType=0; eMemoryType<BMUXlib_TS_P_MemoryType_eMax; eMemoryType++ )
               {
                  pstMemoryConfig->stMemoryConfig.stBufferInfo[eMemoryType].uiSize += pstMemoryConfig->astMemoryEntry[eMemoryEntryType][eInputType].stMemoryConfig.stBufferInfo[eMemoryType].uiSize;
               }
            }
         } /* end: for input type */
      } /* end: for memory entry type */
   }

   BDBG_LEAVE( BMUXlib_TS_P_GetMemoryConfig );
}

#define BMUXLIB_TS_P_ASSIGN_IF_LARGER(a,b,field) if ((a)->field < (b)->field) { ((a)->field = (b)->field); }

void
BMUXlib_TS_P_GetMaxMemoryConfig(
   const BMUXlib_TS_MuxConfig *astMuxConfig, /* Array of possible configurations */
   unsigned uiNumMuxConfig,
   BMUXlib_TS_P_MemoryConfig *pstMemoryConfig /* The memory config required to support all possible configurations */
   )
{
   unsigned i;
   BMUXlib_TS_P_MemoryConfig *pTempMuxMemConfig;

   BDBG_ENTER( BMUXlib_TS_P_GetMaxMemoryConfig );

   BDBG_ASSERT(astMuxConfig);
   BDBG_ASSERT(uiNumMuxConfig);
   BDBG_ASSERT(pstMemoryConfig);

   BKNI_Memset( pstMemoryConfig, 0, sizeof( BMUXlib_TS_P_MemoryConfig ) );

   pTempMuxMemConfig = BKNI_Malloc(sizeof(*pTempMuxMemConfig));
   if (!pTempMuxMemConfig) {
      BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      return;
   }

   for ( i = 0; (NULL != astMuxConfig) && (i < uiNumMuxConfig); i++ )
   {
      BMUXlib_TS_P_GetMemoryConfig(
         &astMuxConfig[i],
         pTempMuxMemConfig
      );

      {
         BMUXlib_TS_P_MemoryEntryType eMemoryEntryType;

         for ( eMemoryEntryType = 0; eMemoryEntryType < BMUXlib_TS_P_MemoryEntryType_eMax; eMemoryEntryType++ )
         {
            BMUXlib_TS_P_InputType eInputType;

            for ( eInputType=0; eInputType<BMUXlib_TS_P_InputType_eMax; eInputType++ )
            {
               BMUXlib_TS_P_MemoryType eMemoryType;
               for ( eMemoryType=0; eMemoryType<BMUXlib_TS_P_MemoryType_eMax; eMemoryType++ )
               {
                  BMUXLIB_TS_P_ASSIGN_IF_LARGER(pstMemoryConfig, pTempMuxMemConfig, astMemoryEntry[eMemoryEntryType][eInputType].stMemoryConfig.stBufferInfo[eMemoryType].uiSize);
               }

               BMUXLIB_TS_P_ASSIGN_IF_LARGER(pstMemoryConfig, pTempMuxMemConfig, astMemoryEntry[eMemoryEntryType][eInputType].uiCount);
            }
         }
      }

      {
         BMUXlib_TS_P_MemoryType eMemoryType;
         for ( eMemoryType=0; eMemoryType<BMUXlib_TS_P_MemoryType_eMax; eMemoryType++ )
         {
            BMUXLIB_TS_P_ASSIGN_IF_LARGER(pstMemoryConfig, pTempMuxMemConfig, stMemoryConfig.stBufferInfo[eMemoryType].uiSize);
         }
      }
   }
   BKNI_Free(pTempMuxMemConfig);

   BDBG_LEAVE( BMUXlib_TS_P_GetMaxMemoryConfig );
}

/**********/
/* Memory */
/**********/
void
BMUXlib_TS_GetMemoryConfig(
         const BMUXlib_TS_MuxConfig *pstMuxConfig,
         BMUXlib_TS_MemoryConfig *pstMemoryConfig
         )
{
   BMUXlib_TS_P_MemoryConfig *pTempMuxMemConfig;

   BDBG_ENTER( BMUXlib_TS_GetMemoryConfig );

   BDBG_ASSERT(pstMuxConfig);
   BDBG_ASSERT(pstMemoryConfig);

   BKNI_Memset( pstMemoryConfig, 0, sizeof( BMUXlib_TS_MemoryConfig )  );

   pTempMuxMemConfig = BKNI_Malloc(sizeof(*pTempMuxMemConfig));
   if (!pTempMuxMemConfig) {
      BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
      return;
   }

   BMUXlib_TS_P_GetMaxMemoryConfig(
      pstMuxConfig,
      1,
      pTempMuxMemConfig
      );

   pstMemoryConfig->uiSystemBufferSize = pTempMuxMemConfig->stMemoryConfig.stBufferInfo[BMUXlib_TS_P_MemoryType_eSystem].uiSize;
   pstMemoryConfig->uiSharedBufferSize = pTempMuxMemConfig->stMemoryConfig.stBufferInfo[BMUXlib_TS_P_MemoryType_eShared].uiSize;

   BKNI_Free(pTempMuxMemConfig);

   BDBG_LEAVE( BMUXlib_TS_GetMemoryConfig );
}

void
BMUXlib_TS_P_GetDefaultMemorySize(
   BMUXlib_TS_MemoryConfig *pstMemoryConfig /* The memory size required to support all possible configurations */
   )
{
   BMUXlib_TS_MuxConfig stMuxConfig;

   BDBG_ENTER( BMUXlib_TS_P_GetDefaultMemorySize );

   BDBG_ASSERT( pstMemoryConfig );

   /* Determine sub heap allocation size */
   BMUXlib_TS_GetDefaultStartSettings(
      &stMuxConfig.stMuxStartSettings
      );

   BMUXlib_TS_GetDefaultMuxSettings(
      &stMuxConfig.stMuxSettings
      );

   /* Assume 6 audio, 1 video, 4 user data pids, NRT mode, TTS support */
   stMuxConfig.stMuxStartSettings.uiNumValidVideoPIDs = BMUXLIB_TS_MAX_VIDEO_PIDS;
   stMuxConfig.stMuxStartSettings.uiNumValidAudioPIDs = BMUXLIB_TS_MAX_AUDIO_PIDS;
   stMuxConfig.stMuxStartSettings.uiNumValidUserdataPIDs = BMUXLIB_TS_P_NOMINAL_USERDATA_PIDS;
   stMuxConfig.stMuxStartSettings.bNonRealTimeMode = true;
   stMuxConfig.stMuxSettings.uiSystemDataBitRate = BMUXLIB_TS_P_NOMINAL_SYSTEM_DATA_BITRATE;
   stMuxConfig.stMuxStartSettings.stPCRData.uiInterval = BMUXLIB_TS_P_NOMINAL_PCR_INTERVAL;
   stMuxConfig.stMuxStartSettings.bSupportTTS = true;

   BMUXlib_TS_GetMemoryConfig(
      &stMuxConfig,
      pstMemoryConfig
      );

   BDBG_LEAVE( BMUXlib_TS_P_GetDefaultMemorySize );
}

void
BMUXlib_TS_GetDefaultCreateSettings(
         BMUXlib_TS_CreateSettings *pCreateSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetDefaultCreateSettings );

   BDBG_ASSERT( pCreateSettings );

   BKNI_Memset(
            pCreateSettings,
            0,
            sizeof(BMUXlib_TS_CreateSettings)
            );

   pCreateSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_CREATESETTINGS;

   BMUXlib_TS_P_GetDefaultMemorySize(
      &pCreateSettings->stMemoryConfig
      );

   BDBG_LEAVE( BMUXlib_TS_GetDefaultCreateSettings );
   return;
}

void
BMUXlib_TS_P_GetBufferConfigFromMemoryConfig(
   const BMUXlib_TS_MemoryConfig *pstMemoryConfig,
   const BMUXlib_TS_P_MemoryBuffers *pstMemoryBuffers,
   BMUXlib_TS_P_BufferConfig *pstBufferConfig
   )
{
   BDBG_ENTER(BMUXlib_TS_P_GetBufferConfigFromMemoryConfig);

   BKNI_Memset( pstBufferConfig, 0, sizeof( BMUXlib_TS_P_BufferConfig ) );

   pstBufferConfig->stBufferInfo[BMUXlib_TS_P_MemoryType_eSystem].uiSize = pstMemoryConfig->uiSystemBufferSize;
   pstBufferConfig->stBufferInfo[BMUXlib_TS_P_MemoryType_eSystem].pBuffer = pstMemoryBuffers->pSystemBuffer;
   pstBufferConfig->stBufferInfo[BMUXlib_TS_P_MemoryType_eShared].uiSize = pstMemoryConfig->uiSharedBufferSize;
   pstBufferConfig->stBufferInfo[BMUXlib_TS_P_MemoryType_eShared].pBuffer = pstMemoryBuffers->pSharedBuffer;

   BDBG_LEAVE(BMUXlib_TS_P_GetBufferConfigFromMemoryConfig);
}

/* BMUXlib_TS_P_AllocateMemory - Allocates all system/device memory required for mux operation */
BERR_Code
BMUXlib_TS_P_AllocateMemory(
   BMUXlib_TS_Handle hMuxTS,
   BMMA_Heap_Handle hMma,
   BMUXlib_TS_P_BufferConfig *pstMemoryConfig
)
{
   BERR_Code rc = BERR_SUCCESS;
   BMUXlib_TS_P_MemoryType eMemoryType;

   BDBG_ENTER( BMUXlib_TS_P_AllocateMemory );

   BDBG_OBJECT_ASSERT( hMuxTS, BMUXlib_TS_P_Context );
   BDBG_ASSERT( hMma );
   BDBG_ASSERT( pstMemoryConfig );

   for ( eMemoryType=0; eMemoryType < BMUXlib_TS_P_MemoryType_eMax ; eMemoryType++ )
   {
      /* memory config must specify some amount of memory ... */
      if (0 == pstMemoryConfig->stBufferInfo[eMemoryType].uiSize)
      {
         BDBG_LEAVE( BMUXlib_TS_P_AllocateMemory );
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      /* Create sub-heap */
      BMMA_RangeAllocator_GetDefaultCreateSettings( &hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings );
      hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings.size = pstMemoryConfig->stBufferInfo[eMemoryType].uiSize;

      /* Only allocate memory for the sub heap if an explicit buffer is not passed in */
      if ( NULL != pstMemoryConfig->stBufferInfo[eMemoryType].pBuffer )
      {
         hMuxTS->stSubHeap[eMemoryType].pBuffer = pstMemoryConfig->stBufferInfo[eMemoryType].pBuffer;
         /* Get virtual address/offset */
         hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings.base = pstMemoryConfig->stBufferInfo[eMemoryType].uiBufferOffset;
      }
      else
      {
         hMuxTS->stSubHeap[eMemoryType].hBlock = BMMA_Alloc(
            hMma,
            hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings.size,
            0,
            0
            );

         if ( NULL == hMuxTS->stSubHeap[eMemoryType].hBlock )
         {
            BDBG_ERR(("Error allocating memory"));
            BDBG_LEAVE( BMUXlib_TS_P_AllocateMemory );
            return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
         }

         hMuxTS->stSubHeap[eMemoryType].pBuffer = BMMA_Lock( hMuxTS->stSubHeap[eMemoryType].hBlock );
         hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings.base = BMMA_LockOffset( hMuxTS->stSubHeap[eMemoryType].hBlock );
      }

      /* Create BMMA Range Allocator */
      rc = BMMA_RangeAllocator_Create(
            &hMuxTS->stSubHeap[eMemoryType].hMmaRangeAllocator,
            &hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings
            );
      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Error getting creating MMA range allocator"));
         BDBG_LEAVE( BMUXlib_TS_P_AllocateMemory );
         return BERR_TRACE( rc );
      }
   }


   BDBG_LEAVE( BMUXlib_TS_P_AllocateMemory );

   return BERR_TRACE( rc );
}

void
BMUXlib_TS_P_FreeMemory(
   BMUXlib_TS_Handle hMuxTS,
   BMMA_Heap_Handle hMma
   )
{
   BMUXlib_TS_P_MemoryType eMemoryType;
   BMUXlib_TS_P_BufferConfig stMemoryConfigLocal;

   BDBG_ENTER( BMUXlib_TS_P_FreeMemory );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   BSTD_UNUSED( hMma );

   BMUXlib_TS_P_GetBufferConfigFromMemoryConfig( &hMuxTS->stCreateSettings.stMemoryConfig, &hMuxTS->stMemoryBuffers, &stMemoryConfigLocal );

   for ( eMemoryType=0; eMemoryType < BMUXlib_TS_P_MemoryType_eMax; eMemoryType++ )
   {
      if ( NULL != hMuxTS->stSubHeap[eMemoryType].hMmaRangeAllocator )
      {
         BMMA_RangeAllocator_Destroy( hMuxTS->stSubHeap[eMemoryType].hMmaRangeAllocator );
      }

      /* Only free memory for the sub heap if an explicit buffer was not passed in */
      if ( NULL == stMemoryConfigLocal.stBufferInfo[eMemoryType].pBuffer )
      {
         if ( NULL != hMuxTS->stSubHeap[eMemoryType].hBlock )
         {
            if ( NULL != hMuxTS->stSubHeap[eMemoryType].pBuffer )
            {
               BMMA_Unlock( hMuxTS->stSubHeap[eMemoryType].hBlock, hMuxTS->stSubHeap[eMemoryType].pBuffer );
            }
            hMuxTS->stSubHeap[eMemoryType].pBuffer = NULL;

            if ( BMUXLIB_TS_P_INVALID_OFFSET != hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings.base )
            {
               BMMA_UnlockOffset( hMuxTS->stSubHeap[eMemoryType].hBlock, hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings.base );
            }
            hMuxTS->stSubHeap[eMemoryType].stMmaRangeAllocatorCreateSettings.base = BMUXLIB_TS_P_INVALID_OFFSET;

            BMMA_Free( hMuxTS->stSubHeap[eMemoryType].hBlock );
            hMuxTS->stSubHeap[eMemoryType].hBlock = NULL;
         }
      }

      BKNI_Memset( &hMuxTS->stSubHeap[eMemoryType], 0, sizeof( hMuxTS->stSubHeap[eMemoryType] ) );
   }

   BDBG_LEAVE( BMUXlib_TS_P_FreeMemory );
}

void
BMUXlib_TS_P_ResetResources(
   BMUXlib_TS_Handle hMuxTS
   )
{
   unsigned i;

   BDBG_ENTER( BMUXlib_TS_P_ResetResources );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   /* Available Transport Descriptors */
   BMUXlib_List_Reset( hMuxTS->hTransportDescriptorFreeList );
   BMUXlib_List_Reset( hMuxTS->hTransportDescriptorMetaDataFreeList );

   for ( i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount; i++ )
   {
      BMUXlib_List_Add(
               hMuxTS->hTransportDescriptorFreeList,
               &hMuxTS->astTransportDescriptor[i]
               );

      BMUXlib_List_Add(
               hMuxTS->hTransportDescriptorMetaDataFreeList,
               &hMuxTS->astTransportDescriptorMetaData[i]
               );
   }

   /* PES Headers */
   BMUXlib_List_Reset( hMuxTS->hPESHeaderFreeList );

   for ( i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_ePESHeader].uiCount; i++ )
   {
      BMUXlib_List_Add(
               hMuxTS->hPESHeaderFreeList,
               &hMuxTS->astPESHeader[i]
               );
   }

   /* TS Packets */
   BMUXlib_List_Reset(hMuxTS->hTSPacketFreeList);
   for ( i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount; i++ )
   {
      BMUXlib_List_Add(
               hMuxTS->hTSPacketFreeList,
               &hMuxTS->astTSPacket[i]
               );
   }

   /* BPP Packets */
   BMUXlib_List_Reset(hMuxTS->hBPPFreeList);
   for ( i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eBPP].uiCount; i++ )
   {
      BMUXlib_List_Add(
               hMuxTS->hBPPFreeList,
               &hMuxTS->astBPPData[i]
               );
   }

   /* MTU BPP Packets */
   BMUXlib_List_Reset(hMuxTS->hMTUBPPFreeList);
   for ( i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eMTUBPP].uiCount; i++ )
   {
      BMUXlib_List_Add(
               hMuxTS->hMTUBPPFreeList,
               &hMuxTS->astMTUBPPData[i]
               );
   }

   /* Pending System Data */
   BMUXlib_List_Reset( hMuxTS->hSystemDataPendingList );

   /* push all available userdata pending entries onto the free list ... */
   BMUXlib_List_Reset(hMuxTS->hUserdataFreeList);

   for (i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserData].uiCount; i++)
   {
      BMUXlib_List_Add(hMuxTS->hUserdataFreeList, &hMuxTS->astUserdataPending[i]);
   }

   /* Userdata PTS entries */
   BMUXlib_List_Reset( hMuxTS->hUserdataPTSFreeList );

   for (i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataPTS].uiCount; i++)
   {
      BMUXlib_List_Add(hMuxTS->hUserdataPTSFreeList, hMuxTS->astUserdataPTS[i].aPTS);
   }

   /* Userdata release Q free list */
   {
      uint32_t uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataReleaseQ].uiCount;
      if ( 0 != uiCount )
      {
         hMuxTS->pUserdataReleaseQFreeList = hMuxTS->pUserdataReleaseQFreeListBase;
         BKNI_Memset(hMuxTS->pUserdataReleaseQFreeList, 0, sizeof(BMUXlib_TS_P_UserdataReleaseQEntry) * uiCount);
         for (i = 0; i < uiCount-1; i++)
            hMuxTS->pUserdataReleaseQFreeList[i].pNext = &hMuxTS->pUserdataReleaseQFreeList[i+1];
         hMuxTS->pUserdataReleaseQFreeList[i].pNext = NULL;
      }
   }

   /* SW7435-535,SW7435-688: Temporary fix for prematurely released transport descriptors */
   for ( i = 0; i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++ )
   {
      hMuxTS->uiPendingCompleted[i] = 0;
   }

   BDBG_LEAVE( BMUXlib_TS_P_ResetResources );
}

#define BMMA_RANGE_ALLOC( _subHeap, _bufferType, _count, _hBlock, _pvBuffer )\
      {\
         BMMA_RangeAllocator_BlockSettings stMmaRangeAllocatorBlockSettings;\
         BMMA_RangeAllocator_GetDefaultAllocationSettings( &stMmaRangeAllocatorBlockSettings );\
\
         (_pvBuffer) = NULL;\
         rc = BMMA_RangeAllocator_Alloc(\
               (_subHeap).hMmaRangeAllocator,\
               &(_hBlock),\
               sizeof( _bufferType) * (_count),\
               &stMmaRangeAllocatorBlockSettings\
               );\
\
         if ( BERR_SUCCESS == rc )\
         {\
            (_pvBuffer) = (_bufferType *) (((uint8_t*)(_subHeap).pBuffer + (BMMA_RangeAllocator_GetAllocationBase_isrsafe( _hBlock) - (_subHeap).stMmaRangeAllocatorCreateSettings.base)));\
         }\
         BERR_TRACE( rc );\
      }

#define BMMA_RANGE_FREE( _subHeap, _hBlock, _pvBuffer )\
      {\
         BMMA_RangeAllocator_Free(\
               (_subHeap).hMmaRangeAllocator,\
               _hBlock\
            );\
         (_hBlock) = NULL;\
         (_pvBuffer) = NULL;\
      }

BERR_Code
BMUXlib_TS_P_AllocateResources(
   BMUXlib_TS_Handle hMuxTS,
   const BMUXlib_TS_P_MemoryConfig *pstMemoryConfig
   )
{
   BERR_Code rc = BERR_SUCCESS;
   uint32_t i;
   BMUXlib_List_CreateSettings stListCreateSettings, stDefaultListCreateSettings;
   BMUXlib_List_Handle hSystemDataPendingList = NULL;
   BMMA_RangeAllocator_Block_Handle hSystemDataPendingListBlock;
   BMUXlib_TS_SystemData *astSystemDataPendingList = NULL;
   uint32_t uiSystemDataPendingListCount = 0;

   BDBG_ENTER( BMUXlib_TS_P_AllocateResources );
   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   hMuxTS->status.stMemoryConfig = *pstMemoryConfig;
   BMUXlib_TS_P_GetMemoryConfigTotal( &hMuxTS->status.stMemoryConfig, &hMuxTS->status.stMemoryConfigTotal );

#if BDBG_DEBUG_BUILD
   {
      BMUXlib_TS_P_MemoryEntryType eMemoryEntryType;

      static const char * const BMUXlib_TS_P_MemoryTypeLUT[BMUXlib_TS_P_MemoryEntryType_eMax] =
      {
         "eTransportDescriptor",
         "ePESHeader",
         "eTransportPacket",
         "eSystemData",
         "BPP",
         "UserData",
         "UserDataUnwrap",
         "UserDataReleaseQ",
         "UserDataPTS",
         "MTU BPP",
      };

      for ( eMemoryEntryType = 0; eMemoryEntryType < BMUXlib_TS_P_MemoryEntryType_eMax; eMemoryEntryType++ )
      {
         BDBG_MODULE_MSG( BMUXLIB_TS_MEMORY, ("Memory Type[%2d]: %5d (%7d/%7d bytes) - %s",
            eMemoryEntryType,
            (int)hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[eMemoryEntryType].uiCount,
            (int)hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[eMemoryEntryType].stMemoryConfig.stBufferInfo[0].uiSize,
            (int)hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[eMemoryEntryType].stMemoryConfig.stBufferInfo[1].uiSize,
            BMUXlib_TS_P_MemoryTypeLUT[eMemoryEntryType]
            ));
      }
      BDBG_MODULE_MSG( BMUXLIB_TS_MEMORY, ("Total Memory: %7d/%7d bytes",
         (int)hMuxTS->status.stMemoryConfigTotal.stMemoryConfig.stBufferInfo[0].uiSize,
         (int)hMuxTS->status.stMemoryConfigTotal.stMemoryConfig.stBufferInfo[0].uiSize
         ));
   }
#endif

   BMUXlib_List_GetDefaultCreateSettings(&stDefaultListCreateSettings);

   uiSystemDataPendingListCount =  hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eSystemData].uiCount;
   if ( 0 != uiSystemDataPendingListCount )
   {
      /* Allocate System Data Pending List */
      stListCreateSettings = stDefaultListCreateSettings;
      stListCreateSettings.uiCount = uiSystemDataPendingListCount;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      rc = BMUXlib_List_Create(&hSystemDataPendingList, &stListCreateSettings);
      if (BERR_SUCCESS != rc)
      {
         if ( NULL != hSystemDataPendingList )
         {
            BMUXlib_List_Destroy( hSystemDataPendingList );
         }
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(rc);
      }

      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
            BMUXlib_TS_SystemData,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eSystemData].uiCount,
            hSystemDataPendingListBlock,
            astSystemDataPendingList
            );
      if ( NULL == astSystemDataPendingList )
      {
         BMUXlib_List_Destroy( hSystemDataPendingList );
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }

      /* switch pending list from pre-Q to the newly allocated Q */
      hMuxTS->hSystemDataPendingList = hSystemDataPendingList;
      hMuxTS->hSystemDataPendingListBlock = hSystemDataPendingListBlock;
      hMuxTS->astSystemDataPendingList = astSystemDataPendingList;
      hMuxTS->status.uiSystemDataMaxCount = stListCreateSettings.uiCount;
      hMuxTS->status.uiSystemDataPendingListReadOffset = 0;
      hMuxTS->status.uiSystemDataPendingListWriteOffset = 0;
   } /* end: allocate system data pending list */

   if ( 0 != hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount )
   {
      /* Allocate Transport Descriptor Array (system memory) */
      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            BMUXlib_TS_TransportDescriptor,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount,
            hMuxTS->hTransportDescriptorBlock,
            hMuxTS->astTransportDescriptor
            );
      if ( NULL == hMuxTS->astTransportDescriptor )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }
      BKNI_Memset(
               hMuxTS->astTransportDescriptor,
               0,
               sizeof( BMUXlib_TS_TransportDescriptor ) * hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount
               );

      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            BMUXlib_TS_TransportDescriptor,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount,
            hMuxTS->hTransportDescriptorTempBlock,
            hMuxTS->astTransportDescriptorTemp
            );
      if ( NULL == hMuxTS->astTransportDescriptorTemp )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }
      BKNI_Memset(
               hMuxTS->astTransportDescriptorTemp,
               0,
               sizeof( BMUXlib_TS_TransportDescriptor ) * hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount
               );

      /* Allocate Transport Meta Data Array (system memory) */
      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            BMUXlib_TS_P_TransportDescriptorMetaData,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount,
            hMuxTS->hTransportDescriptorMetaDataBlock,
            hMuxTS->astTransportDescriptorMetaData
            );
      if ( NULL == hMuxTS->astTransportDescriptorMetaData )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }
      BKNI_Memset(
               hMuxTS->astTransportDescriptorMetaData,
               0,
               sizeof( BMUXlib_TS_P_TransportDescriptorMetaData ) * hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount
               );

      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            BMUXlib_TS_P_TransportDescriptorMetaData,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount,
            hMuxTS->hTransportDescriptorMetaDataTempBlock,
            hMuxTS->astTransportDescriptorMetaDataTemp
            );
      if ( NULL == hMuxTS->astTransportDescriptorMetaDataTemp )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }
      BKNI_Memset(
               hMuxTS->astTransportDescriptorMetaDataTemp,
               0,
               sizeof( BMUXlib_TS_P_TransportDescriptorMetaData ) * hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount
               );

      /* Allocate Transport Descriptor Free List */
      stListCreateSettings = stDefaultListCreateSettings;

         /* it seems free list is shared among all transcport playback instances */
      stListCreateSettings.uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      rc = BMUXlib_List_Create(
                  &hMuxTS->hTransportDescriptorFreeList,
                  &stListCreateSettings
                  );
      if (BERR_SUCCESS != rc)
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(rc);
      }

      rc = BMUXlib_List_Create(
                  &hMuxTS->hTransportDescriptorMetaDataFreeList,
                  &stListCreateSettings
                  );
      if (BERR_SUCCESS != rc)
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(rc);
      }
   }

   if ( 0 != hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_ePESHeader].uiCount )
   {
      /* Allocate PES Header Array (device memory) */
      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
            BMUXlib_TS_P_PESHeader,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_ePESHeader].uiCount,
            hMuxTS->hPESHeaderBlock,
            hMuxTS->astPESHeader
            );
      if ( NULL == hMuxTS->astPESHeader )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }

      /* Allocate PES Header Free List */
      stListCreateSettings = stDefaultListCreateSettings;

      stListCreateSettings.uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_ePESHeader].uiCount;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      rc =  BMUXlib_List_Create(&hMuxTS->hPESHeaderFreeList, &stListCreateSettings);
      if (BERR_SUCCESS != rc)
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(rc);
      }
   }

   if ( 0 != hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount )
   {
      /* Allocate TS Packet Array (device memory) */
      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
            BMUXlib_TS_P_TSPacket,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount,
            hMuxTS->hTSPacketBlock,
            hMuxTS->astTSPacket
            );
      if ( NULL == hMuxTS->astTSPacket )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }

      /* Pre-populate with default TS Packet */
      for (i = 0; i < hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount; i++ )
      {
         BKNI_Memcpy(
                  &hMuxTS->astTSPacket[i],
                  &s_stDefaultTSPacket,
                  sizeof( BMUXlib_TS_P_TSPacket )
                  );
      }
   }

   if ( 0 != hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount )
   {
      /* Allocate TS Packet Free List */
      stListCreateSettings = stDefaultListCreateSettings;

      stListCreateSettings.uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      rc = BMUXlib_List_Create(&hMuxTS->hTSPacketFreeList, &stListCreateSettings);
      if (BERR_SUCCESS != rc)
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(rc);
      }

      /* Allocate BPP Free List */
      stListCreateSettings = stDefaultListCreateSettings;

      stListCreateSettings.uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eBPP].uiCount;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      rc = BMUXlib_List_Create(&hMuxTS->hBPPFreeList, &stListCreateSettings);
      if (BERR_SUCCESS != rc)
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(rc);
      }

      /* Allocate BPP Data Array (device memory) */
      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
            BMUXlib_TS_P_BPPData,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eBPP].uiCount,
            hMuxTS->hBPPDataBlock,
            hMuxTS->astBPPData
            );
      if ( NULL == hMuxTS->astBPPData )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }

      if ( 0 != hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eMTUBPP].uiCount )
      {
         /* Allocate MTU BPP Free List */
         stListCreateSettings = stDefaultListCreateSettings;

         stListCreateSettings.uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eMTUBPP].uiCount;
         stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

         rc = BMUXlib_List_Create(&hMuxTS->hMTUBPPFreeList, &stListCreateSettings);
         if (BERR_SUCCESS != rc)
         {
            BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
            return BERR_TRACE(rc);
         }

         /* Allocate MTU BPP Data Array (device memory) */
         BMMA_RANGE_ALLOC(
               hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
               BMUXlib_TS_P_MTUBPPData,
               hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eMTUBPP].uiCount,
               hMuxTS->hMTUBPPDataBlock,
               hMuxTS->astMTUBPPData
               );
         if ( NULL == hMuxTS->astMTUBPPData )
         {
            BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
            return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
         }
      }
   }

   if ( 0 != hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserData].uiCount )
   {
      /* Allocate Userdata Free List
         NOTE: This assumes that there is a fixed amount of userdata
         processed per video frame - thus, the PES count supplied by the user
         is used to allocate the necessary free list for userdata. */
      stListCreateSettings = stDefaultListCreateSettings;

      stListCreateSettings.uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserData].uiCount;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      rc = BMUXlib_List_Create(&hMuxTS->hUserdataFreeList, &stListCreateSettings);
      if (BERR_SUCCESS != rc)
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( rc );
      }

      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            BMUXlib_TS_P_UserdataPending,
            stListCreateSettings.uiCount,
            hMuxTS->hUserdataPendingBlock,
            hMuxTS->astUserdataPending
            );
      if ( NULL == hMuxTS->astUserdataPending )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      }

      /* Allocate userdata PTS storage - need TWO per userdata PES pending */
      stListCreateSettings = stDefaultListCreateSettings;

      stListCreateSettings.uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataPTS].uiCount;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      rc = BMUXlib_List_Create(&hMuxTS->hUserdataPTSFreeList, &stListCreateSettings);
      if (BERR_SUCCESS != rc)
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE( rc );
      }

      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
            BMUXlib_TS_P_UserdataPTSEntry,
            stListCreateSettings.uiCount,
            hMuxTS->hUserdataPTSBlock,
            hMuxTS->astUserdataPTS
            );
      if ( NULL == hMuxTS->astUserdataPTS )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      /* Allocate userdata local buffering for "unwrapping" packets
         (each userdata input has its own buffer) */
      BMMA_RANGE_ALLOC(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
            BMUXlib_TS_P_TSPacket,
            hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataUnwrap].uiCount,
            hMuxTS->hUserdataUnwrapBlock,
            hMuxTS->astUserdataUnwrap
            );
      if ( NULL == hMuxTS->astUserdataUnwrap )
      {
         BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
         return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
      }

      /* userdata release Q (need one entry for each pending segment entry) ... */
      {
         uint32_t uiCount = hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserDataReleaseQ].uiCount;
         BMMA_RANGE_ALLOC(
               hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
               BMUXlib_TS_P_UserdataReleaseQEntry,
               uiCount,
               hMuxTS->hUserdataReleaseQFreeListBaseBlock,
               hMuxTS->pUserdataReleaseQFreeListBase
               );
         if ( NULL == hMuxTS->pUserdataReleaseQFreeListBase )
         {
            BDBG_LEAVE(BMUXlib_TS_P_AllocateResources);
            return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
         }
      }
   } /* end: userdata free list allocation */

   BMUXlib_TS_P_ResetResources( hMuxTS );

   BDBG_LEAVE( BMUXlib_TS_P_AllocateResources );

   return BERR_TRACE( rc );
}

/* move any pre-queued system data into the actual system data queue */
/* SW7425-4643: This call expects hMuxTS->status.stStartSettings to be set, since
   the copying of any pre-queued system data will use the PCR PID for validation */
BERR_Code
BMUXlib_TS_P_RelocatePreQSystemData(BMUXlib_TS_Handle hMuxTS)
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   /* SW7425-3958: */
   if ( 0 != hMuxTS->status.uiSystemDataPreQCount )
   {
      size_t uiQueuedCount;

      /* Use the allocated system data list: Move any system data queued prior to start to this list buffers */
      /* NOTE: This must be done AFTER ResetResources() since that call will clear the new queue (and the newly
         copied data!) */

      rc = BMUXlib_TS_P_AddSystemDataBuffers(
               hMuxTS,
               hMuxTS->astSystemDataPendingListPreQ,
               hMuxTS->status.uiSystemDataPreQCount,
               &uiQueuedCount
               );
      if (rc == BERR_SUCCESS)
      {
         BDBG_ASSERT( hMuxTS->status.uiSystemDataPreQCount == uiQueuedCount );
         hMuxTS->status.uiSystemDataPreQCount = 0;
      }
   }
   return BERR_TRACE(rc);
}

BERR_Code
BMUXlib_TS_P_SetupMCPB(
   BMUXlib_TS_Handle hMuxTS
   )
{
   BERR_Code rc = BERR_SUCCESS;
   unsigned i, uiPIDIndex;
   unsigned auiNumInputsPerTransportChannelIndex[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES];

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   for ( i = 0; i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++ )
   {
      auiNumInputsPerTransportChannelIndex[i] = 0;
   }

   /* Check Video Inputs */
   for ( uiPIDIndex = 0; (BERR_SUCCESS == rc) && (uiPIDIndex < BMUXLIB_TS_MAX_VIDEO_PIDS) && (uiPIDIndex < hMuxTS->status.stStartSettings.uiNumValidVideoPIDs); uiPIDIndex++ )
   {
      auiNumInputsPerTransportChannelIndex[hMuxTS->status.stStartSettings.video[uiPIDIndex].uiTransportChannelIndex]++;
   }

   /* Check Audio Inputs */
   for ( uiPIDIndex = 0; (BERR_SUCCESS == rc) && (uiPIDIndex < BMUXLIB_TS_MAX_AUDIO_PIDS) && (uiPIDIndex < hMuxTS->status.stStartSettings.uiNumValidAudioPIDs); uiPIDIndex++ )
   {
      auiNumInputsPerTransportChannelIndex[hMuxTS->status.stStartSettings.audio[uiPIDIndex].uiTransportChannelIndex]++;
   }

   /* Check System Inputs */
   auiNumInputsPerTransportChannelIndex[hMuxTS->status.stStartSettings.stPCRData.uiTransportChannelIndex]++;

   /* Create MCPB instance for each transport that has multiple inputs */
   for ( i = 0; ( BERR_SUCCESS == rc) && i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++ )
   {
      if ( auiNumInputsPerTransportChannelIndex[i] > 1 )
      {
         BMUXlib_TS_MCPB_CreateSettings stCreateSettings;

         BMUXlib_TS_MCPB_GetDefaultCreateSettings( &stCreateSettings );
         stCreateSettings.hMma = hMuxTS->stCreateSettings.hMma;
         stCreateSettings.stMuxSharedMemory.hBlock =  hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared].hBlock;
         stCreateSettings.stMuxSharedMemory.uiSize =  hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared].stMmaRangeAllocatorCreateSettings.size;
         stCreateSettings.uiMaxNumInputs = auiNumInputsPerTransportChannelIndex[i];
         stCreateSettings.stOutputChannelInterface = hMuxTS->status.stStartSettings.transport.stChannelInterface[i];

         rc = BERR_TRACE( BMUXlib_TS_MCPB_Create(
            &hMuxTS->status.stOutput.stMCPB[i].hMuxMCPB,
            &stCreateSettings
            ));
      }
   }

   return BERR_TRACE( rc );
}

void
BMUXlib_TS_P_TeardownMCPB(
   BMUXlib_TS_Handle hMuxTS
   )
{
   unsigned i,j;

   for ( i = 0; i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++ )
   {
      if ( NULL != hMuxTS->status.stOutput.stMCPB[i].hMuxMCPB )
      {
         for ( j = 0; j < BMUXLIB_TS_MAX_VIDEO_PIDS + BMUXLIB_TS_MAX_AUDIO_PIDS + 1; j++ )
         {
            if ( NULL != hMuxTS->status.stOutput.stMCPB[i].ahMuxMCPBCh[j] )
            {
               BMUXlib_TS_MCPB_Channel_Close( hMuxTS->status.stOutput.stMCPB[i].ahMuxMCPBCh[j] );
               hMuxTS->status.stOutput.stMCPB[i].ahMuxMCPBCh[j] = NULL;
            }
         }

         BMUXlib_TS_MCPB_Destroy(
            hMuxTS->status.stOutput.stMCPB[i].hMuxMCPB
            );
         hMuxTS->status.stOutput.stMCPB[i].hMuxMCPB = NULL;
      }
   }
}

void
BMUXlib_TS_P_FreeResources(
   BMUXlib_TS_Handle hMuxTS
   )
{
   uint32_t i;

   BDBG_ENTER( BMUXlib_TS_P_FreeResources );

   /* cleanup input group and inputs - these are created by Start() */
   if (NULL != hMuxTS->status.hInputGroup)
   {
      BMUXlib_InputGroup_Destroy(hMuxTS->status.hInputGroup);
      hMuxTS->status.hInputGroup = NULL;
   }

   BMUXlib_TS_P_TeardownMCPB( hMuxTS );

   {
      unsigned uiInputIndex;

      for ( uiInputIndex = 0; uiInputIndex < BMUXLIB_TS_MAX_INPUT_PIDS; uiInputIndex++ )
      {
         if ( NULL != hMuxTS->status.stInputMetaData[uiInputIndex].hInput )
         {
            BMUXlib_Input_Destroy( hMuxTS->status.stInputMetaData[uiInputIndex].hInput );
            hMuxTS->status.stInputMetaData[uiInputIndex].hInput = NULL;
         }
      }
   }

#if BDBG_DEBUG_BUILD
#if 1
   /* Verify there aren't any pending buffers */

   for ( i = 0; i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++ )
   {
      if ( true == hMuxTS->status.stOutput.stTransport[i].bActive && NULL != hMuxTS->hTransportDescriptorPendingList[i])
      {
         if ( false == BMUXlib_List_IsEmpty( hMuxTS->hTransportDescriptorPendingList[i] ) )
         {
            BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("Transport Descriptors Pending List[%d] not empty", i));
         }
      }
   }

   /* Verify all buffers have been returned to the free lists */
   {
      size_t uiCount;

      if ( NULL != hMuxTS->hTransportDescriptorFreeList )
      {
         BMUXlib_List_GetNumEntries(
                  hMuxTS->hTransportDescriptorFreeList,
                  &uiCount
                  );
         if ( hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount != uiCount )
         {
            BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("All Transport Descriptors have not been freed (%d)", (int)(hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount - uiCount)));
         }
      }

      if ( NULL != hMuxTS->hTransportDescriptorMetaDataFreeList )
      {
         BMUXlib_List_GetNumEntries(
                  hMuxTS->hTransportDescriptorMetaDataFreeList,
                  &uiCount
                  );
         if ( hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount != uiCount )
         {
            BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("All Transport Metadata Descriptors have not been freed (%d)", (int)(hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor].uiCount - uiCount)));
         }
      }

      if ( NULL != hMuxTS->hPESHeaderFreeList )
      {
         BMUXlib_List_GetNumEntries(
                  hMuxTS->hPESHeaderFreeList,
                  &uiCount
                  );
         if ( hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_ePESHeader].uiCount != uiCount )
         {
            BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("All PES Headers have not been freed (%d)", (int)(hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_ePESHeader].uiCount - uiCount)));
         }
      }

      if ( hMuxTS->hTSPacketFreeList )
      {
         BMUXlib_List_GetNumEntries(
                  hMuxTS->hTSPacketFreeList,
                  &uiCount
                  );
         if ( hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount != uiCount )
         {
            BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("All TS Packets have not been freed (%d)", (int)(hMuxTS->status.stMemoryConfigTotal.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportPacket].uiCount - uiCount)));
         }
      }
   }
#else
#warning "Check for pending descriptors on destroy has been disabled"
#endif
#endif

   if ( NULL != hMuxTS->astMTUBPPData )
   {
      BMMA_RANGE_FREE(
         hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
         hMuxTS->hMTUBPPDataBlock,
         hMuxTS->astMTUBPPData
         );
   }

   if ( NULL != hMuxTS->astBPPData )
   {
      BMMA_RANGE_FREE(
         hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
         hMuxTS->hBPPDataBlock,
         hMuxTS->astBPPData
         );
   }

   if ( NULL != hMuxTS->astUserdataPending )
   {
      BMMA_RANGE_FREE(
         hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
         hMuxTS->hUserdataPendingBlock,
         hMuxTS->astUserdataPending
         );
   }

   if ( NULL != hMuxTS->pUserdataReleaseQFreeListBase )
   {
      BMMA_RANGE_FREE(
         hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
         hMuxTS->hUserdataReleaseQFreeListBaseBlock,
         hMuxTS->pUserdataReleaseQFreeListBase
         );
      hMuxTS->pUserdataReleaseQFreeList = NULL;
   }

   for (i = 0; i < BMUXLIB_TS_MAX_USERDATA_PIDS; i++)
   {
      if ( NULL != hMuxTS->hUserdataPendingList[i] )
      {
#if BDBG_DEBUG_BUILD
         size_t uiMinUsage, uiMaxUsage, uiSize;
         BMUXlib_List_GetUsage(hMuxTS->hUserdataPendingList[i], &uiMinUsage, &uiMaxUsage, &uiSize);
         BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("Userdata Pending List[%d]: Max Usage: %d of %d (%d%%)", i, (int)uiMaxUsage, (int)uiSize, (int)((100*uiMaxUsage)/uiSize)));
#endif
         BMUXlib_List_Destroy(hMuxTS->hUserdataPendingList[i]);
         hMuxTS->hUserdataPendingList[i] = NULL;
      }
   }

   if ( NULL != hMuxTS->hUserdataFreeList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hUserdataFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("Userdata Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy(hMuxTS->hUserdataFreeList);
      hMuxTS->hUserdataFreeList = NULL;
   }

   if (NULL != hMuxTS->astUserdataPTS )
   {
      BMMA_RANGE_FREE(
         hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
         hMuxTS->hUserdataPTSBlock,
         hMuxTS->astUserdataPTS
         );
   }

   if (NULL != hMuxTS->astUserdataUnwrap )
   {
      BMMA_RANGE_FREE(
         hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
         hMuxTS->hUserdataUnwrapBlock,
         hMuxTS->astUserdataUnwrap
         );
   }

   if (NULL != hMuxTS->hUserdataPTSFreeList)
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hUserdataPTSFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("Userdata PTS Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy(hMuxTS->hUserdataPTSFreeList);
      hMuxTS->hUserdataPTSFreeList = NULL;
   }

   if ( NULL != hMuxTS->astSystemDataPendingList )
   {
      BMMA_RANGE_FREE( hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared], hMuxTS->hSystemDataPendingListBlock, hMuxTS->astSystemDataPendingList );
   }

   if ( NULL != hMuxTS->hSystemDataPendingList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hSystemDataPendingList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("System Data Pending List: Max Usage: %d of %d (%d%%)", (int)uiMaxUsage, (int)uiSize, (int)((100*uiMaxUsage)/uiSize)));
#endif
      BMUXlib_List_Destroy(hMuxTS->hSystemDataPendingList);
      hMuxTS->hSystemDataPendingList = NULL;
   }

   if ( NULL != hMuxTS->hMTUBPPFreeList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hMTUBPPFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("MTU BPP Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy( hMuxTS->hMTUBPPFreeList );
      hMuxTS->hMTUBPPFreeList = NULL;
   }

   if ( NULL != hMuxTS->hBPPFreeList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hBPPFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("BPP Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy( hMuxTS->hBPPFreeList );
      hMuxTS->hBPPFreeList = NULL;
   }

   if ( NULL != hMuxTS->hTSPacketFreeList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hTSPacketFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("TS Packet Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy( hMuxTS->hTSPacketFreeList );
      hMuxTS->hTSPacketFreeList = NULL;
   }

   if ( NULL != hMuxTS->astTSPacket )
   {
      BMMA_RANGE_FREE(
               hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
               hMuxTS->hTSPacketBlock,
               hMuxTS->astTSPacket
               );
   }

   if ( NULL != hMuxTS->hPESHeaderFreeList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hPESHeaderFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("PES Header Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy( hMuxTS->hPESHeaderFreeList );
      hMuxTS->hPESHeaderFreeList = NULL;
   }

   if ( NULL != hMuxTS->astPESHeader )
   {
      BMMA_RANGE_FREE(
               hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared],
               hMuxTS->hPESHeaderBlock,
               hMuxTS->astPESHeader
               );
   }

   for ( i = 0; i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++ )
   {
      if ( NULL != hMuxTS->hTransportDescriptorMetaDataPendingList[i] )
      {
#if BDBG_DEBUG_BUILD
         size_t uiMinUsage, uiMaxUsage, uiSize;
         BMUXlib_List_GetUsage(hMuxTS->hTransportDescriptorMetaDataPendingList[i], &uiMinUsage, &uiMaxUsage, &uiSize);
         BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("Transport Descriptor Metadata Pending List[%d]: Max Usage: %d of %d (%d%%)", i, (int)uiMaxUsage, (int)uiSize, (int)((100*uiMaxUsage)/uiSize)));
#endif
         BMUXlib_List_Destroy( hMuxTS->hTransportDescriptorMetaDataPendingList[i] );
         hMuxTS->hTransportDescriptorMetaDataPendingList[i] = NULL;
      }

      if ( NULL != hMuxTS->hTransportDescriptorPendingList[i] )
      {
#if BDBG_DEBUG_BUILD
         size_t uiMinUsage, uiMaxUsage, uiSize;
         BMUXlib_List_GetUsage(hMuxTS->hTransportDescriptorPendingList[i], &uiMinUsage, &uiMaxUsage, &uiSize);
         BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("Transport Descriptor Pending List[%d]: Max Usage: %d of %d (%d%%)", i, (int)uiMaxUsage, (int)uiSize, (int)((100*uiMaxUsage)/uiSize)));
#endif
         BMUXlib_List_Destroy( hMuxTS->hTransportDescriptorPendingList[i] );
         hMuxTS->hTransportDescriptorPendingList[i] = NULL;
      }
   }

   if ( NULL != hMuxTS->hTransportDescriptorMetaDataFreeList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hTransportDescriptorMetaDataFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("Transport Descriptor Metadata Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy( hMuxTS->hTransportDescriptorMetaDataFreeList );
      hMuxTS->hTransportDescriptorMetaDataFreeList = NULL;
   }

   if ( NULL != hMuxTS->hTransportDescriptorFreeList )
   {
#if BDBG_DEBUG_BUILD
      size_t uiMinUsage, uiMaxUsage, uiSize;
      BMUXlib_List_GetUsage(hMuxTS->hTransportDescriptorFreeList, &uiMinUsage, &uiMaxUsage, &uiSize);
      BDBG_MODULE_MSG(BMUXLIB_TS_MEMORY, ("Transport Descriptor Free List: Max Usage: %d of %d (%d%%)", (int)(uiSize-uiMinUsage), (int)uiSize, (int)((100 * (uiSize-uiMinUsage))/uiSize)));
#endif
      BMUXlib_List_Destroy( hMuxTS->hTransportDescriptorFreeList );
      hMuxTS->hTransportDescriptorFreeList = NULL;
   }

   if ( NULL != hMuxTS->astTransportDescriptorMetaDataTemp )
   {
      BMMA_RANGE_FREE(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            hMuxTS->hTransportDescriptorMetaDataTempBlock,
            hMuxTS->astTransportDescriptorMetaDataTemp
            );
   }

   if ( NULL != hMuxTS->astTransportDescriptorMetaData )
   {
      BMMA_RANGE_FREE(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            hMuxTS->hTransportDescriptorMetaDataBlock,
            hMuxTS->astTransportDescriptorMetaData
            );
   }

   if ( NULL != hMuxTS->astTransportDescriptorTemp )
   {
      BMMA_RANGE_FREE(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            hMuxTS->hTransportDescriptorTempBlock,
            hMuxTS->astTransportDescriptorTemp
            );
   }

   if ( NULL != hMuxTS->astTransportDescriptor )
   {
      BMMA_RANGE_FREE(
            hMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eSystem],
            hMuxTS->hTransportDescriptorBlock,
            hMuxTS->astTransportDescriptor
            );
   }

   BDBG_LEAVE( BMUXlib_TS_P_FreeResources );
}

void
BMUXlib_TS_P_Reset(
   BMUXlib_TS_Handle hMuxTS
   )
{
   BDBG_ENTER( BMUXlib_TS_P_Reset );

#ifdef BMUXLIB_TS_P_TEST_MODE
   if (NULL != hMuxTS->status.stSystemDataInfo.hCSVFile)
      fclose(hMuxTS->status.stSystemDataInfo.hCSVFile);
   if (NULL != hMuxTS->status.stSystemDataInfo.hDataFile)
      fclose(hMuxTS->status.stSystemDataInfo.hDataFile);
#endif

   BKNI_Memset( &hMuxTS->status, 0, sizeof( hMuxTS->status ) );

   BMUXlib_TS_GetDefaultMuxSettings(
            &hMuxTS->status.stMuxSettings
            );

   BMUXlib_TS_GetDefaultStartSettings(
            &hMuxTS->status.stStartSettings
            );

   /* reset the pre-Q */
   hMuxTS->hSystemDataPendingList = NULL;
   hMuxTS->astSystemDataPendingList = NULL;
   hMuxTS->status.uiSystemDataMaxCount = 0;
   hMuxTS->status.uiSystemDataPreQCount = 0;

   BDBG_LEAVE( BMUXlib_TS_P_Reset );
}

BERR_Code
BMUXlib_TS_Create(
         BMUXlib_TS_Handle *phMuxTS,  /* [out] TSMuxer handle returned */
         const BMUXlib_TS_CreateSettings *pstCreateSettings
         )
{
   BERR_Code rc = BERR_UNKNOWN;
   BMUXlib_TS_P_Context *pMuxTS;

   BDBG_ENTER( BMUXlib_TS_Create );

   BDBG_ASSERT( phMuxTS );
   BDBG_ASSERT( pstCreateSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_CREATESETTINGS == pstCreateSettings->uiSignature );

   /************************/
   /* Create MUX TS Handle */
   /************************/

   /* Set the handle to NULL in case the allocation fails */
   *phMuxTS = NULL;

   if (/* verify the required hMem handle is provided ... */
      /* NOTE: if software XPT ever used via the transport abstraction, then
         this _could_ be NULL, and Malloc should be used instead */
      (NULL == pstCreateSettings->hMma)
      )
   {
      BDBG_LEAVE(BMUXlib_TS_Create);
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Allocate MUX TS Context (system memory) */
   pMuxTS = ( BMUXlib_TS_P_Context* ) BKNI_Malloc( sizeof( BMUXlib_TS_P_Context ) );
   if ( NULL == pMuxTS )
   {
      BDBG_LEAVE(BMUXlib_TS_Create);
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   /* Zero out the newly allocated context */
   BKNI_Memset(
            pMuxTS,
            0,
            sizeof( BMUXlib_TS_P_Context )
            );

   BDBG_OBJECT_SET(pMuxTS, BMUXlib_TS_P_Context);

   /* Copy user specified settings */
   BKNI_Memcpy(
            &pMuxTS->stCreateSettings,
            pstCreateSettings,
            sizeof( BMUXlib_TS_CreateSettings )
            );

   /* Populate BMUXlib_TS_P_MemoryBuffers */
   {
      pMuxTS->stMemoryBuffers.pSystemBuffer = pMuxTS->stCreateSettings.stMemoryBuffers.pSystemBuffer;

      if ( NULL != pMuxTS->stCreateSettings.stMemoryBuffers.hSharedBufferBlock )
      {
         pMuxTS->stMemoryBuffers.pSharedBuffer = BMMA_Lock( pMuxTS->stCreateSettings.stMemoryBuffers.hSharedBufferBlock );
      }
   }

   /* Allocate sub heap */
   {
      BMUXlib_TS_P_BufferConfig stMemoryConfig;
      BKNI_Memset( &stMemoryConfig, 0, sizeof( BMUXlib_TS_P_BufferConfig ) );
      BMUXlib_TS_P_GetBufferConfigFromMemoryConfig( &pMuxTS->stCreateSettings.stMemoryConfig, &pMuxTS->stMemoryBuffers, &stMemoryConfig );

      rc = BMUXlib_TS_P_AllocateMemory( pMuxTS, pMuxTS->stCreateSettings.hMma, &stMemoryConfig );

      if ( BERR_SUCCESS != rc )
      {
         BMUXlib_TS_Destroy( pMuxTS );
         BDBG_LEAVE(BMUXlib_TS_Create);
         return BERR_TRACE(rc);
      }
   }

   /* Allocate System Data Pending List pre-Q */
   {
      BMUXlib_List_CreateSettings stListCreateSettings;
      BMUXlib_List_GetDefaultCreateSettings(&stListCreateSettings);

      stListCreateSettings.uiCount = BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT;
      stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

      BDBG_ASSERT( pMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared].stMmaRangeAllocatorCreateSettings.size > ( BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT * sizeof( BMUXlib_TS_SystemData ) ) );

      pMuxTS->astSystemDataPendingListPreQ = (BMUXlib_TS_SystemData *) ( (uint8_t *) pMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared].pBuffer + ( pMuxTS->stSubHeap[BMUXlib_TS_P_MemoryType_eShared].stMmaRangeAllocatorCreateSettings.size - ( BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT * sizeof( BMUXlib_TS_SystemData ) ) ) );
   }

   /* Reset State */
   BMUXlib_TS_P_Reset( pMuxTS );

   /* Provide handle to caller */
   *phMuxTS = pMuxTS;
   BDBG_LEAVE(BMUXlib_TS_Create);
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BMUXlib_TS_Destroy(
         BMUXlib_TS_Handle hMuxTS
         )
{
   BDBG_ENTER(BMUXlib_TS_Destroy);

   /* the following signifies an attempt to free up something that was either
      a) not created by Create()
      b) has already been destroyed
   */
   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   /* SW7425-3642: Stop the mux if it hasn't already been stopped
      - this is necessary since Stop() now frees resources */
   if (BMUXlib_State_eStopped != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS))
   {
      BERR_Code rc = BMUXlib_TS_Stop(hMuxTS);
      if (rc != BERR_SUCCESS)
      {
         BDBG_LEAVE(BMUXlib_TS_Destroy);
         return BERR_TRACE(rc);
      }
   }

   BMUXlib_TS_P_FreeMemory(
      hMuxTS,
      hMuxTS->stCreateSettings.hMma
      );

   /* Unlock shared buffer (if supplied) */
   /* Populate BMUXlib_TS_P_MemoryBuffers */
   {
      if ( ( NULL != hMuxTS->stCreateSettings.stMemoryBuffers.hSharedBufferBlock )
           && ( NULL != hMuxTS->stMemoryBuffers.pSharedBuffer ) )
      {
         BMMA_Unlock( hMuxTS->stCreateSettings.stMemoryBuffers.hSharedBufferBlock, hMuxTS->stMemoryBuffers.pSharedBuffer );
      }
   }

   /* the following prevents accidental reuse of the context */
   BDBG_OBJECT_DESTROY(hMuxTS, BMUXlib_TS_P_Context);
   BKNI_Free ( hMuxTS );

   BDBG_LEAVE(BMUXlib_TS_Destroy);
   return BERR_TRACE( BERR_SUCCESS );
}

/****************/
/* Mux Settings */
/****************/
void
BMUXlib_TS_GetDefaultMuxSettings(
         BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetDefaultMuxSettings );

   BDBG_ASSERT( pstMuxSettings );

   BKNI_Memset(
            pstMuxSettings,
            0,
            sizeof( BMUXlib_TS_MuxSettings )
            );

   pstMuxSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_MUXSETTINGS;
   pstMuxSettings->uiSystemDataBitRate = BMUXLIB_TS_P_MUX_SYS_DATA_BR_DEFAULT;

   /* SW7436-1363: Enable all inputs by default */
   {
      unsigned i;

      for ( i = 0; i < BMUXLIB_TS_MAX_VIDEO_PIDS; i++ )
      {
         pstMuxSettings->stInputEnable.bVideo[i] = true;
      }

      for ( i = 0; i < BMUXLIB_TS_MAX_AUDIO_PIDS; i++ )
      {
         pstMuxSettings->stInputEnable.bAudio[i] = true;
      }
   }

   BDBG_LEAVE( BMUXlib_TS_GetDefaultMuxSettings );
   return;
}

/* Validate Specified Mux settings
   NOTE: that validation of the system data bitrate requires access to the
   Start Settings
   If bUseDefaults set, then erroneous settings will result in a warning and
   defaults will be used

   Return will be false if settings are OK
   If bUseDefaults is set, a true return code indicates a change was made
   otherwise true indicates an erroneous setting (it is expected that the caller
   will handle the error condition if bUseDefaults is false)
*/
static bool
BMUXlib_TS_P_ValidateMuxSettings(BMUXlib_TS_MuxSettings *pstMuxSettings, BMUXlib_TS_StartSettings *pstStartSettings, bool bUseDefaults)
{
   /* NOTE: for now, system data bitrate is the only mux setting to be validated */
   return BMUXlib_TS_P_ValidateSystemDataBitrate(pstMuxSettings, pstStartSettings, bUseDefaults);
}

/* Minimum number of system data packets that occurs in a PCR interval
   (INCLUDING the PCR packet itself) - i.e. PCR + 1 other */
#define MIN_SYSD_PACKETS_PER_INTERVAL 2

/* Validate the System Data Bitrate to ensure it is sufficient for the
   current PCR interval in use
   (we specify the minimum required to be 2 packets: the PCR itself
   and one other)
   If the value is not within the allowed bounds clip to the max/min
   and give a warning
   Return will be false if settings are OK
   If bUseDefaults is set, a true return code indicates a change was made
   otherwise true indicates an erroneous setting (it is expected that the caller
   will handle the error condition if bUseDefaults is false)
   NOTE: Currently, this will always return false if bUseDefaults is false
*/
static bool
BMUXlib_TS_P_ValidateSystemDataBitrate(BMUXlib_TS_MuxSettings *pstMuxSettings, const BMUXlib_TS_StartSettings *pstStartSettings, bool bUseDefaults)
{
   uint32_t uiMinSystemDataBitrate;
   /* NOTE: here, the system data bitrate has been either set to defaults by Create, or to
      some other value by the user via SetMuxSettings().  The values set by Create (the
      defaults) are guaranteed to work. So this is only verifying user input of user-
      provided system  data bitrate vs current PCR inteval (either default or user-provided) */

   if (pstMuxSettings->uiSystemDataBitRate > BMUXLIB_TS_MAX_SYS_DATA_BR)
   {
      pstMuxSettings->uiSystemDataBitRate = BMUXLIB_TS_MAX_SYS_DATA_BR;
      BDBG_WRN(("System Data Bitrate exceeds maximum allowed (%d) ... clipping to max", pstMuxSettings->uiSystemDataBitRate));
      return bUseDefaults;
   }
   if ( 0 != pstStartSettings->stPCRData.uiInterval )
   {
      /* calculate min required for PCR interval */
      /* min bitrate required = 2 TS packets / PCR interval (ms) = 2 * 188 * 8 * 1000 / PCR interval
         = 3008000 / PCR Interval (in ms) */
      uiMinSystemDataBitrate = (MIN_SYSD_PACKETS_PER_INTERVAL * 188 * 8 * 1000) / pstStartSettings->stPCRData.uiInterval;
      if (pstMuxSettings->uiSystemDataBitRate < uiMinSystemDataBitrate)
      {
         BDBG_WRN(("System Data Bitrate (%d) insufficient for current PCR interval (%d ms)", pstMuxSettings->uiSystemDataBitRate,
            pstStartSettings->stPCRData.uiInterval));
         pstMuxSettings->uiSystemDataBitRate = uiMinSystemDataBitrate;
         BDBG_WRN(("Using minimum bitrate to provide at least 2 packets per interval: %d bps", pstMuxSettings->uiSystemDataBitRate));
         return bUseDefaults;
      }
   }
   return false;
}

BERR_Code
BMUXlib_TS_SetMuxSettings(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BMUXlib_TS_MuxSettings stSettingsCopy;
   BDBG_ENTER( BMUXlib_TS_SetMuxSettings );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);
   BDBG_ASSERT( pstMuxSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_MUXSETTINGS == pstMuxSettings->uiSignature );

   /* make a copy since Validate() may modify the settings */
   stSettingsCopy = *pstMuxSettings;
   if (BMUXlib_TS_P_ValidateMuxSettings(&stSettingsCopy, &hMuxTS->status.stStartSettings, false))
   {
      BDBG_LEAVE( BMUXlib_TS_SetMuxSettings );
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* if settings validate OK, then store them */
   hMuxTS->status.stMuxSettings = stSettingsCopy;

   /* SW7436-1363: If mux has started, then set the inputs as needed */
   if ( BMUXlib_State_eStopped != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
   {
      unsigned i,uiInputIndex;
      BMUXlib_Input_Settings stSettings;

      for ( i = 0; i < BMUXLIB_TS_MAX_VIDEO_PIDS; i++ )
      {
         uiInputIndex = hMuxTS->status.stInputIndexLUT.uiVideo[i];

         if ( uiInputIndex == 0xFFFFFFFF ) continue;

         BMUXlib_Input_GetSettings(
            hMuxTS->status.stInputMetaData[uiInputIndex].hInput,
            &stSettings
         );

         stSettings.bEnable = hMuxTS->status.stMuxSettings.stInputEnable.bVideo[i];

         BMUXlib_Input_SetSettings(
            hMuxTS->status.stInputMetaData[uiInputIndex].hInput,
            &stSettings
         );
      }

      for ( i = 0; i < BMUXLIB_TS_MAX_AUDIO_PIDS; i++ )
      {
         uiInputIndex = hMuxTS->status.stInputIndexLUT.uiAudio[i];

         if ( uiInputIndex == 0xFFFFFFFF ) continue;

         BMUXlib_Input_GetSettings(
            hMuxTS->status.stInputMetaData[uiInputIndex].hInput,
            &stSettings
         );

         stSettings.bEnable = hMuxTS->status.stMuxSettings.stInputEnable.bAudio[i];

         BMUXlib_Input_SetSettings(
            hMuxTS->status.stInputMetaData[uiInputIndex].hInput,
            &stSettings
         );
      }
   }

   BDBG_LEAVE( BMUXlib_TS_SetMuxSettings );
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BMUXlib_TS_GetMuxSettings(
         BMUXlib_TS_Handle hMuxTS,
         BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetMuxSettings );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);
   BDBG_ASSERT( pstMuxSettings );

   *pstMuxSettings = hMuxTS->status.stMuxSettings;

   BDBG_LEAVE( BMUXlib_TS_GetMuxSettings );
   return BERR_TRACE( BERR_SUCCESS );
}

/**************/
/* Start/Stop */
/**************/
void
BMUXlib_TS_GetDefaultStartSettings(
         BMUXlib_TS_StartSettings *pstStartSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetDefaultStartSettings );

   BDBG_ASSERT( pstStartSettings );

   BKNI_Memset(
            pstStartSettings,
            0,
            sizeof( BMUXlib_TS_StartSettings )
            );

   pstStartSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_STARTSETTINGS;
   pstStartSettings->uiServiceLatencyTolerance = 20;
   pstStartSettings->uiServicePeriod = BMUXLIB_TS_P_MUX_SERVICE_PERIOD_DEFAULT;
   pstStartSettings->stPCRData.uiInterval = BMUXLIB_TS_P_MUX_PCR_INTERVAL_DEFAULT;
   pstStartSettings->uiA2PDelay = BMUXLIB_TS_P_A2PDELAY_DEFAULT;

   BDBG_LEAVE( BMUXlib_TS_GetDefaultStartSettings );
   return;
}

/* Validate Start settings that also affect memory calculations

   If bUseDefaults set, then erroneous settings will result in a warning and
   defaults will be used

   Return will be false if settings are OK
   If bUseDefaults is set, a true return code indicates a change was made
   otherwise true indicates an erroneous setting (it is expected that the caller
   will handle the error condition if bUseDefaults is false)
*/
static bool
BMUXlib_TS_P_ValidateStartSettings(BMUXlib_TS_StartSettings *pstStartSettings, bool bUseDefaults)
{
   bool bChanged = false;
   bool bError = false;

   if ( ( BMUXlib_TS_InterleaveMode_ePTS == pstStartSettings->eInterleaveMode )
        && ( 0 != pstStartSettings->stPCRData.uiInterval ) )
   {
      BDBG_WRN(("PCR generation is not supported in PTS Interleave Mode - ignoring PCR interval"));
      pstStartSettings->stPCRData.uiInterval = 0;
      bChanged = true;
   }

   /* Interval of > 100ms is outside spec, but we do not impose specification on
      the user (they may desire a proprietary implementation */
   if (pstStartSettings->stPCRData.uiInterval > 100)
      BDBG_WRN(("PCR Interval exceeds maximum of 100ms (as specified by ISO/IEC-13818-1)"));

   /* NOTE: the following are all expected behaviour, so we do NOT output error */
   if (pstStartSettings->uiNumValidVideoPIDs > BMUXLIB_TS_MAX_VIDEO_PIDS)
   {
      BDBG_WRN(("Specified Video PID count exceeds capabilities - excess ignored"));
      pstStartSettings->uiNumValidVideoPIDs = BMUXLIB_TS_MAX_VIDEO_PIDS;
      bChanged = true;
   }

   if (pstStartSettings->uiNumValidAudioPIDs > BMUXLIB_TS_MAX_AUDIO_PIDS)
   {
      BDBG_WRN(("Specified Audio PID count exceeds capabilities - excess ignored"));
      pstStartSettings->uiNumValidAudioPIDs = BMUXLIB_TS_MAX_AUDIO_PIDS;
      bChanged = true;
   }

   if (pstStartSettings->uiNumValidUserdataPIDs > BMUXLIB_TS_MAX_USERDATA_PIDS)
   {
      BDBG_WRN(("Specified Userdata PID count exceeds capabilities - excess ignored"));
      pstStartSettings->uiNumValidUserdataPIDs = BMUXLIB_TS_MAX_USERDATA_PIDS;
      bChanged = true;
   }

   if (pstStartSettings->uiServicePeriod == 0)
   {
      pstStartSettings->uiServicePeriod = BMUXLIB_TS_P_MUX_SERVICE_PERIOD_DEFAULT;
      BDBG_WRN(("Service period of 0 ms specified. Using default of %d ms", pstStartSettings->uiServicePeriod));
      bChanged = true;
   }

   /* SW7425-4340: Add app-specified A2PDelay */
   if (pstStartSettings->uiA2PDelay < BMUXLIB_TS_MIN_A2PDELAY)
   {
      pstStartSettings->uiA2PDelay = BMUXLIB_TS_P_A2PDELAY_DEFAULT;
      BDBG_WRN(("Arrival-to-Presentation Delay of < 27000 ticks (1ms) specified. Using default of %d ms", pstStartSettings->uiA2PDelay/BMUXLIB_TS_P_SCALE_MS_TO_27MHZ));
      bChanged = true;
   }

   if (bUseDefaults)
      return bChanged;
   return bError;
}

BERR_Code
BMUXlib_TS_P_AssignTransportChannel(
   BMUXlib_TS_Handle hMuxTS,
   unsigned uiTransportChannelIndex,
   unsigned uiPID,
   unsigned uiPIDChannelIndex,
   bool bIsTS,
   BMUXlib_TS_TransportChannelInterface *pChannelInterface
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BKNI_Memset( pChannelInterface, 0, sizeof( BMUXlib_TS_TransportChannelInterface ) );

   if ( NULL == hMuxTS->status.stOutput.stMCPB[uiTransportChannelIndex].hMuxMCPB )
   {
      /* Only one input assigned to this transport channel index, so it is a legacy PB channel, so use it as-is */
      *pChannelInterface = hMuxTS->status.stStartSettings.transport.stChannelInterface[uiTransportChannelIndex];
   }
   else
   {
      /* More than one inputs are assigned to this transport channel index, so it is a MCPB channel, so assign */
      BMUXlib_TS_MCPB_Channel_OpenSettings stOpenSettings;

      BMUXlib_TS_MCPB_Channel_GetDefaultOpenSettings( &stOpenSettings );
      stOpenSettings.bIsTS = bIsTS;
      stOpenSettings.uiInstance = hMuxTS->status.stOutput.stMCPB[uiTransportChannelIndex].uiNumOpenChannels++;
      stOpenSettings.uiPID = uiPID;
      stOpenSettings.uiPIDChannelIndex = uiPIDChannelIndex;

      rc = BERR_TRACE( BMUXlib_TS_MCPB_Channel_Open(
         hMuxTS->status.stOutput.stMCPB[uiTransportChannelIndex].hMuxMCPB,
         &hMuxTS->status.stOutput.stMCPB[uiTransportChannelIndex].ahMuxMCPBCh[stOpenSettings.uiInstance],
         &stOpenSettings
         ));

      if ( BERR_SUCCESS == rc )
      {
         pChannelInterface->pContext = hMuxTS->status.stOutput.stMCPB[uiTransportChannelIndex].ahMuxMCPBCh[stOpenSettings.uiInstance];
         pChannelInterface->fAddTransportDescriptors = (BMUXlib_TS_AddTransportDescriptors) BMUXlib_TS_MCPB_Channel_AddTransportDescriptors;
         pChannelInterface->fGetCompletedTransportDescriptors = (BMUXlib_TS_GetCompletedTransportDescriptors) BMUXlib_TS_MCPB_Channel_GetCompletedTransportDescriptors;
      }
   }

   return BERR_TRACE( rc );
}

/* BMUXlib_TS_Start - Configures the mux HW */
BERR_Code
BMUXlib_TS_Start(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_StartSettings *pstStartSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BMUXlib_TS_Start );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);
   BDBG_ASSERT( pstStartSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_STARTSETTINGS == pstStartSettings->uiSignature );

   /* verify required settings are present ... */
   if (( NULL == pstStartSettings->transport.stDeviceInterface.fGetTransportSettings)
         || (NULL == pstStartSettings->transport.stDeviceInterface.fSetTransportSettings)
         || (NULL == pstStartSettings->transport.stDeviceInterface.fGetTransportStatus))
   {
      BDBG_LEAVE( BMUXlib_TS_Start );
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   /* verify PCR parameters */
   if (pstStartSettings->stPCRData.uiPID == 0)
   {
      BDBG_LEAVE( BMUXlib_TS_Start );
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   /* Initialize state only if mux is stopped */
   if ( BMUXlib_State_eStopped == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
   {
      unsigned uiPIDIndex;
      bool *pPIDTable = hMuxTS->status.aFoundPIDs;
      BMUXlib_Input_Handle aInputTable[BMUXLIB_TS_MAX_VIDEO_PIDS+BMUXLIB_TS_MAX_AUDIO_PIDS];
      uint32_t i;
      BMUXlib_Input_CreateSettings stInputCreateSettings;
      BMUXlib_TS_P_BufferConfig stMemoryConfigLocal;
      uint32_t uiNumChannels = 0;
      uint32_t aTransportDescriptorPendingListCountTable[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES];

      /* NOTE: same count is used for both PendingList and MetadataPendingList */
      BKNI_Memset(aTransportDescriptorPendingListCountTable, 0, sizeof(aTransportDescriptorPendingListCountTable));

      {
         uint32_t uiTransportChannelIndex;

         for ( uiTransportChannelIndex = 0; uiTransportChannelIndex < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; uiTransportChannelIndex++ )
         {
            const BMUXlib_TS_TransportChannelInterface *pChannelInterface = &pstStartSettings->transport.stChannelInterface[uiTransportChannelIndex];
            hMuxTS->status.stOutput.stTransport[uiTransportChannelIndex].stTimingPending.uiNextExpectedESCR = 0xFFFFFFFF;
            /* count valid channel interfaces */
            if ((NULL != pChannelInterface->fAddTransportDescriptors)
               && (NULL != pChannelInterface->fGetCompletedTransportDescriptors))
               uiNumChannels++;
         }
      }

      /* verify at least one channel is provided ... */
      if (0 == uiNumChannels)
      {
         BMUXlib_TS_P_FreeResources( hMuxTS );
         BMUXlib_TS_P_Reset( hMuxTS );    /* reset state since Expected ESCRs modified above */
         BDBG_LEAVE( BMUXlib_TS_Start );
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }

      hMuxTS->status.stStartSettings = *pstStartSettings;
      /* Validate the start settings
         Note: validate the stored copy, not the user-supplied input
         (since Validate may modify the settings if defaults used for example) */
      if (BMUXlib_TS_P_ValidateStartSettings(&hMuxTS->status.stStartSettings, false))
      {
         BMUXlib_TS_P_FreeResources( hMuxTS );
         BMUXlib_TS_P_Reset( hMuxTS );    /* reset state since Expected ESCRs modified above */
         BDBG_LEAVE( BMUXlib_TS_Start );
         return BERR_TRACE(BERR_INVALID_PARAMETER);
      }
      /* Verify the current system data bitrate is sufficient for the specified PCR interval
         => if invalid, set the system data bitrate to min required (i.e override with warning)s
         NOTE: The following must be done prior to the first use of the system data bitrate */
      BMUXlib_TS_P_ValidateSystemDataBitrate(&hMuxTS->status.stMuxSettings,
         (const BMUXlib_TS_StartSettings *)&hMuxTS->status.stStartSettings,
         false);


      if ( 0 != hMuxTS->status.stStartSettings.stPCRData.uiInterval )
      {
         /* mark the PCR PID as "in-use" */
         pPIDTable[hMuxTS->status.stStartSettings.stPCRData.uiPID] = true;
         hMuxTS->status.stPCRInfo.uiIntervalIn27Mhz = BMUXLIB_TS_P_SCALE_MS_TO_27MHZ * hMuxTS->status.stStartSettings.stPCRData.uiInterval;
      }
      else
      {
         hMuxTS->status.stPCRInfo.uiIntervalIn27Mhz = BMUXLIB_TS_P_SCALE_MS_TO_27MHZ * BMUXLIB_TS_P_MUX_PCR_INTERVAL_DEFAULT;
      }

      BMUXlib_TS_P_GetBufferConfigFromMemoryConfig( &hMuxTS->stCreateSettings.stMemoryConfig, &hMuxTS->stMemoryBuffers, &stMemoryConfigLocal );

      {
         /* SW7425-5370: allocate memory to store the current memory config ... */
         /* NOTE: allocated in one block and then sub-allocate */
         struct
         {
            BMUXlib_TS_P_MemoryConfig stMemoryConfig;
            BMUXlib_TS_MuxConfig stMuxConfig;
            BMMA_RangeAllocator_Status stRangeAllocatorStatus;
         } *pData;

         pData = BKNI_Malloc(sizeof(*pData));
         if (NULL == pData)
         {
            BDBG_ERR(("Unable to allocate temporary storage for obtaining memory config."));
            BMUXlib_TS_P_FreeResources( hMuxTS );
            BMUXlib_TS_P_Reset( hMuxTS );    /* reset state since Expected ESCRs modified above */
            BDBG_LEAVE( BMUXlib_TS_Start );
            return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
         }

         BKNI_Memset(pData, 0, sizeof(*pData));
         pData->stMuxConfig.stMuxStartSettings = hMuxTS->status.stStartSettings;
         pData->stMuxConfig.stMuxSettings = hMuxTS->status.stMuxSettings;
         /* Get the current memory config that matches the supplied settings */
         BMUXlib_TS_P_GetMemoryConfig(&pData->stMuxConfig, &pData->stMemoryConfig);
         {
            BMUXlib_TS_P_MemoryType eMemoryType;

            for ( eMemoryType = 0; (eMemoryType < BMUXlib_TS_P_MemoryType_eMax) && ( BERR_SUCCESS == rc); eMemoryType++ )
            {
               if ( stMemoryConfigLocal.stBufferInfo[eMemoryType].uiSize < pData->stMemoryConfig.stMemoryConfig.stBufferInfo[eMemoryType].uiSize )
               {
                  BDBG_ERR(("Not enough memory of type [%d] for requested configuration. (%d < %d)",
                     eMemoryType,
                     (int)stMemoryConfigLocal.stBufferInfo[eMemoryType].uiSize,
                     (int)pData->stMemoryConfig.stMemoryConfig.stBufferInfo[eMemoryType].uiSize
                     ));
                  rc = BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
               }
            }
         }

         if ( BERR_SUCCESS == rc )
         {
            /* Allocate resources from sub heap */
            rc = BMUXlib_TS_P_AllocateResources( hMuxTS, &pData->stMemoryConfig );
            if ( BERR_SUCCESS == rc )
            {
               BMUXlib_TS_P_MemoryType eMemoryType;

               for ( eMemoryType = 0; (eMemoryType < BMUXlib_TS_P_MemoryType_eMax); eMemoryType++ )
               {
                  BKNI_Memset(&pData->stRangeAllocatorStatus, 0, sizeof(BMMA_RangeAllocator_Status));
                  BMMA_RangeAllocator_GetStatus( hMuxTS->stSubHeap[eMemoryType].hMmaRangeAllocator, &pData->stRangeAllocatorStatus);

                  BDBG_MODULE_MSG( BMUXLIB_TS_MEMORY, ("[%d] Total Required/Used (Slack): %u/%u (%d)",
                     eMemoryType,
                     (int)stMemoryConfigLocal.stBufferInfo[eMemoryType].uiSize,
                     (int)pData->stRangeAllocatorStatus.allocatedSpace,
                     (int)(stMemoryConfigLocal.stBufferInfo[eMemoryType].uiSize - pData->stRangeAllocatorStatus.allocatedSpace)
                     ));
               }
            }
         }
         /* SW7425-5370: Free temporary storage for memory config ...*/
         BKNI_Free(pData);
      }
      if (rc != BERR_SUCCESS)
      {
         BMUXlib_TS_P_FreeResources( hMuxTS );
         BMUXlib_TS_P_Reset( hMuxTS );    /* reset state since Expected ESCRs modified above */
         BDBG_LEAVE( BMUXlib_TS_Start );
         return rc;
      }

      /* SW7346-1363: Intialize input index LUT to 0xFFFFFFFF */
      {
         unsigned i;

         for ( i = 0; i < BMUXLIB_TS_MAX_VIDEO_PIDS; i++ )
         {
            hMuxTS->status.stInputIndexLUT.uiVideo[i] = 0xFFFFFFFF;
         }

         for ( i = 0; i < BMUXLIB_TS_MAX_AUDIO_PIDS; i++ )
         {
            hMuxTS->status.stInputIndexLUT.uiAudio[i] = 0xFFFFFFFF;
         }
      }

      /* Determine if we need MCPB support */
      BMUXlib_TS_P_SetupMCPB( hMuxTS );

      for ( uiPIDIndex = 0; (BERR_SUCCESS == rc) && (uiPIDIndex < BMUXLIB_TS_MAX_VIDEO_PIDS) && (uiPIDIndex < hMuxTS->status.stStartSettings.uiNumValidVideoPIDs); uiPIDIndex++ )
      {
         BMUXlib_VideoEncoderInterface *pEncoderInterface = &hMuxTS->status.stStartSettings.video[uiPIDIndex].stInputInterface;
         uint16_t uiPID = hMuxTS->status.stStartSettings.video[uiPIDIndex].uiPID;
         unsigned uiPIDChannelIndex = hMuxTS->status.stStartSettings.video[uiPIDIndex].uiPIDChannelIndex;

         BKNI_Memset( &hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs], 0, sizeof( BMUXlib_TS_P_InputMetaData ) );

         {
            unsigned uiTransportChannelIndex = hMuxTS->status.stStartSettings.video[uiPIDIndex].uiTransportChannelIndex;
            BMUXlib_TS_TransportChannelInterface *pChannelInterface = &hMuxTS->status.stOutput.stTransport[hMuxTS->status.stOutput.uiNumTransportChannelsOpen].stChannelInterface;

            rc = BERR_TRACE( BMUXlib_TS_P_AssignTransportChannel( hMuxTS, uiTransportChannelIndex, uiPID, uiPIDChannelIndex, false, pChannelInterface ) );
            if ( BERR_SUCCESS != rc ) break;

            hMuxTS->status.stInput.video[uiPIDIndex].uiTransportChannelIndex = hMuxTS->status.stOutput.uiNumTransportChannelsOpen++;;

            if ( ( NULL == pEncoderInterface->fGetBufferDescriptors )
                 || ( NULL == pEncoderInterface->fGetBufferStatus )
                 || ( NULL == pEncoderInterface->fConsumeBufferDescriptors )
                 || ( NULL == pChannelInterface->fAddTransportDescriptors )
                 || ( NULL == pChannelInterface->fGetCompletedTransportDescriptors )
               )
            {
               BDBG_ERR(("Video[%d]: Bad Transport Channel, or Encoder Interface", uiPIDIndex));
               rc = BERR_TRACE(BERR_INVALID_PARAMETER);
               break;
            }
         }

         /* verify PID is not already in use by another input, or by PCR ... */
         /* SW7425-4643: see if PCR PID is already in use by video/audio
            (the mux does not support PCR on same PID as other data, due
             to hardware limitations with creation of correct CC values) */
         if (pPIDTable[uiPID])
         {
            if (uiPID == hMuxTS->status.stStartSettings.stPCRData.uiPID)
               BDBG_ERR(("Video[%d]: Mux does not support PCR on same PID as video input: %x (use a separate PID for PCR)",
                  uiPIDIndex, uiPID));
            else
               BDBG_ERR(("Video[%d]: PID %x is already in use", uiPIDIndex, uiPID));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
         }

         BMUXlib_Input_GetDefaultCreateSettings( &stInputCreateSettings );
         stInputCreateSettings.eType = BMUXlib_Input_Type_eVideo;
         stInputCreateSettings.interface.stVideo = *pEncoderInterface;
         /* SW7425-659: In MTU BPP mode, for video, use frame burst mode so we can calculate the correct PKT2PKT delta */
         stInputCreateSettings.eBurstMode = hMuxTS->status.stStartSettings.bSupportTTS ? BMUXlib_Input_BurstMode_eFrame : BMUXlib_Input_BurstMode_eDescriptor;
         stInputCreateSettings.pMetadata = &hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs];
         stInputCreateSettings.uiMuxId = hMuxTS->stCreateSettings.uiMuxId;
         stInputCreateSettings.uiTypeInstance = uiPIDIndex;
         /* by default, this input will be marked as "active" */
         rc = BERR_TRACE( BMUXlib_Input_Create( &hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput,
                         &stInputCreateSettings
                        ));
         if (BERR_SUCCESS != rc)
            break;

         if ( 0 == hMuxTS->status.stStartSettings.video[uiPIDIndex].uiPESStreamID )
         {
            BDBG_WRN(("Video[%d]: Stream ID is invalid. Using %02x instead", uiPIDIndex, 0xE0+uiPIDIndex));
            hMuxTS->status.stStartSettings.video[uiPIDIndex].uiPESStreamID = 0xE0 + uiPIDIndex;
         }

         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiInputIndex = hMuxTS->status.uiNumInputs;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiTransportChannelIndex = hMuxTS->status.stInput.video[uiPIDIndex].uiTransportChannelIndex;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiPID = uiPID;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiPESStreamID = hMuxTS->status.stStartSettings.video[uiPIDIndex].uiPESStreamID;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiPIDIndex = uiPIDIndex;

         /* setup userdata companion video info */
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].pstUserdata = &hMuxTS->status.stUserdataVideoInfo[uiPIDIndex];

         hMuxTS->status.stOutput.stTransport[hMuxTS->status.stInput.video[uiPIDIndex].uiTransportChannelIndex].bActive = true;

         aInputTable[hMuxTS->status.uiNumInputs] = hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput;
         /* indicate this PID is in use, for later PID collision detection ...*/
         pPIDTable[uiPID] = true;

         /* Determine amount of Transport Descriptor Pending List Entries (Video) */
         /* (A single allocation for the lists is done once all count requirements are established) */
         aTransportDescriptorPendingListCountTable[hMuxTS->status.stInput.video[uiPIDIndex].uiTransportChannelIndex] += BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(hMuxTS->status.stMemoryConfig.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][BMUXlib_TS_P_InputType_eVideo].uiCount, hMuxTS->status.stStartSettings.uiNumValidVideoPIDs);

         /* SW7346-1363: Set default input enable mode (video) */
         {
            BMUXlib_Input_Settings stSettings;

            BMUXlib_Input_GetSettings(
               hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput,
               &stSettings
            );

            stSettings.bEnable = hMuxTS->status.stMuxSettings.stInputEnable.bVideo[uiPIDIndex];

            BMUXlib_Input_SetSettings(
               hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput,
               &stSettings
            );

            hMuxTS->status.stInputIndexLUT.uiVideo[uiPIDIndex] = hMuxTS->status.uiNumInputs;
         }
         hMuxTS->status.uiNumInputs++;
      }  /* end: for each video input */

      for ( uiPIDIndex = 0; (BERR_SUCCESS == rc) && (uiPIDIndex < BMUXLIB_TS_MAX_AUDIO_PIDS) && (uiPIDIndex < hMuxTS->status.stStartSettings.uiNumValidAudioPIDs); uiPIDIndex++ )
      {
         BMUXlib_AudioEncoderInterface *pEncoderInterface = &hMuxTS->status.stStartSettings.audio[uiPIDIndex].stInputInterface;
         uint16_t uiPID = hMuxTS->status.stStartSettings.audio[uiPIDIndex].uiPID;
         unsigned uiPIDChannelIndex = hMuxTS->status.stStartSettings.audio[uiPIDIndex].uiPIDChannelIndex;

         BKNI_Memset( &hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs], 0, sizeof( BMUXlib_TS_P_InputMetaData ) );

         {
            unsigned uiTransportChannelIndex = hMuxTS->status.stStartSettings.audio[uiPIDIndex].uiTransportChannelIndex;
            BMUXlib_TS_TransportChannelInterface *pChannelInterface = &hMuxTS->status.stOutput.stTransport[hMuxTS->status.stOutput.uiNumTransportChannelsOpen].stChannelInterface;

            rc = BMUXlib_TS_P_AssignTransportChannel( hMuxTS, uiTransportChannelIndex, uiPID, uiPIDChannelIndex, false, pChannelInterface );
            if ( BERR_SUCCESS != rc ) break;

            hMuxTS->status.stInput.audio[uiPIDIndex].uiTransportChannelIndex = hMuxTS->status.stOutput.uiNumTransportChannelsOpen++;;

            if ( ( NULL == pEncoderInterface->fGetBufferDescriptors )
                 || ( NULL == pEncoderInterface->fGetBufferStatus )
                 || ( NULL == pEncoderInterface->fConsumeBufferDescriptors )
                 || ( NULL == pChannelInterface->fAddTransportDescriptors )
                 || ( NULL == pChannelInterface->fGetCompletedTransportDescriptors )
               )
            {
               BDBG_ERR(("Audio[%d]: Bad Transport Channel, or Encoder Interface", uiPIDIndex));
               rc = BERR_TRACE(BERR_INVALID_PARAMETER);
               break;
            }
         }

         /* verify PID is not already in use by any other input, or by PCR ... */
         /* SW7425-4643: see if PCR PID is already in use by video/audio
            (the mux does not support PCR on same PID as other data, due
             to hardware limitations with creation of correct CC values) */
         if (pPIDTable[uiPID])
         {
            if (uiPID == hMuxTS->status.stStartSettings.stPCRData.uiPID)
               BDBG_ERR(("Audio[%d]: Mux does not support PCR on same PID as audio input: %x (use a separate PID for PCR)",
               uiPIDIndex, uiPID));
            else
               BDBG_ERR(("Audio[%d]: PID %x is already in use", uiPIDIndex, uiPID));
            rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
         }

         BMUXlib_Input_GetDefaultCreateSettings( &stInputCreateSettings );
         stInputCreateSettings.eType = BMUXlib_Input_Type_eAudio;
         stInputCreateSettings.interface.stAudio = *pEncoderInterface;
         stInputCreateSettings.eBurstMode = BMUXlib_Input_BurstMode_eFrame;
         stInputCreateSettings.pMetadata = &hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs];
         stInputCreateSettings.uiMuxId = hMuxTS->stCreateSettings.uiMuxId;
         stInputCreateSettings.uiTypeInstance = uiPIDIndex;
         /* by default, this input will be marked as "active" */
         rc = BERR_TRACE( BMUXlib_Input_Create( &hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput,
                         &stInputCreateSettings
                        ));
         if (BERR_SUCCESS != rc)
            break;

         if ( 0 == hMuxTS->status.stStartSettings.audio[uiPIDIndex].uiPESStreamID )
         {
            BDBG_WRN(("Audio[%d]: Stream ID is invalid. Using %02x instead", uiPIDIndex, 0xC0+uiPIDIndex));
            hMuxTS->status.stStartSettings.audio[uiPIDIndex].uiPESStreamID = 0xC0 + uiPIDIndex;
         }

         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiInputIndex = hMuxTS->status.uiNumInputs;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiTransportChannelIndex = hMuxTS->status.stInput.audio[uiPIDIndex].uiTransportChannelIndex;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiPID = uiPID;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiPESStreamID = hMuxTS->status.stStartSettings.audio[uiPIDIndex].uiPESStreamID;
         hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].uiPIDIndex = uiPIDIndex;

         hMuxTS->status.stOutput.stTransport[hMuxTS->status.stInput.audio[uiPIDIndex].uiTransportChannelIndex].bActive = true;

         aInputTable[hMuxTS->status.uiNumInputs] = hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput;
         /* indicate this PID is in use, for later PID collision detection ...*/
         pPIDTable[uiPID] = true;

         /* Determine amount of Transport Descriptor Pending List Entries (Audio) */
         /* (A single allocation for the lists is done once all count requirements are established) */
         aTransportDescriptorPendingListCountTable[hMuxTS->status.stInput.audio[uiPIDIndex].uiTransportChannelIndex] += BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(hMuxTS->status.stMemoryConfig.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][BMUXlib_TS_P_InputType_eAudio].uiCount, hMuxTS->status.stStartSettings.uiNumValidAudioPIDs);

         /* SW7346-1363: Set default input enable mode (audio) */
         {
            BMUXlib_Input_Settings stSettings;

            BMUXlib_Input_GetSettings(
               hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput,
               &stSettings
            );

            stSettings.bEnable = hMuxTS->status.stMuxSettings.stInputEnable.bAudio[uiPIDIndex];

            BMUXlib_Input_SetSettings(
               hMuxTS->status.stInputMetaData[hMuxTS->status.uiNumInputs].hInput,
               &stSettings
            );

            hMuxTS->status.stInputIndexLUT.uiAudio[uiPIDIndex] = hMuxTS->status.uiNumInputs;
         }

         hMuxTS->status.uiNumInputs++;
      }  /* end: for each audio input */

      if (0 != hMuxTS->status.uiNumInputs)
      {
         /* relocate the pre-queued system data to the main system data pending list,
            and check the PIDs for conflict */
         if (BERR_SUCCESS == rc)
            rc = BMUXlib_TS_P_RelocatePreQSystemData(hMuxTS);

         if (BERR_SUCCESS == rc)
         {
            BMUXlib_List_CreateSettings stListCreateSettings;

            {
               unsigned uiTransportChannelIndex = hMuxTS->status.stStartSettings.stPCRData.uiTransportChannelIndex;
               BMUXlib_TS_TransportChannelInterface *pChannelInterface = &hMuxTS->status.stOutput.stTransport[hMuxTS->status.stOutput.uiNumTransportChannelsOpen].stChannelInterface;
               unsigned uiPIDChannelIndex = hMuxTS->status.stStartSettings.stPCRData.uiPIDChannelIndex;

               rc = BMUXlib_TS_P_AssignTransportChannel( hMuxTS, uiTransportChannelIndex, 0, uiPIDChannelIndex, true, pChannelInterface );
               if ( BERR_SUCCESS == rc )
               {
                  hMuxTS->status.stInput.system.uiTransportChannelIndex = hMuxTS->status.stOutput.uiNumTransportChannelsOpen++;;

                  /* check specified channel for system data is valid ... */
                  if ((NULL == pChannelInterface->fAddTransportDescriptors )
                     || (NULL == pChannelInterface->fGetCompletedTransportDescriptors ))
                  {
                     BDBG_ERR(("Bad Transport Channel provided for System Data"));
                     rc = BERR_TRACE(BERR_INVALID_PARAMETER);
                  }
               }
            }

            /* Determine amount of Transport Descriptor Pending List Entries (System) */
            /* (A single allocation for the lists is done once all count requirements are established) */
            aTransportDescriptorPendingListCountTable[hMuxTS->status.stInput.system.uiTransportChannelIndex] += hMuxTS->status.stMemoryConfig.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eTransportDescriptor][BMUXlib_TS_P_InputType_eSystem].uiCount;
            hMuxTS->status.stOutput.stTransport[hMuxTS->status.stInput.system.uiTransportChannelIndex].bActive = true;

            /* Allocate Transport Descriptor Pending Lists */
            for (i = 0; rc == BERR_SUCCESS && i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++)
            {
               if (false == hMuxTS->status.stOutput.stTransport[i].bActive)
                  continue;   /* this transport channel is not active */

               BMUXlib_List_GetDefaultCreateSettings(&stListCreateSettings);

               stListCreateSettings.uiCount = aTransportDescriptorPendingListCountTable[i];
               stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;

               /* active channels must have descriptor lists */
               BDBG_ASSERT(0 != stListCreateSettings.uiCount);

               rc = BMUXlib_List_Create(
                        &hMuxTS->hTransportDescriptorPendingList[i],
                        &stListCreateSettings
                        );
               if ( BERR_SUCCESS == rc )
               {
                  rc = BMUXlib_List_Create(
                           &hMuxTS->hTransportDescriptorMetaDataPendingList[i],
                           &stListCreateSettings
                           );
               }
            }
         }

         if (BERR_SUCCESS == rc)
         {
            BMUXlib_List_CreateSettings stListCreateSettings;
            BMUXlib_List_GetDefaultCreateSettings(&stListCreateSettings);

            /* Allocate Userdata Pending List for each userdata input */
            stListCreateSettings.uiCount = BMUXLIB_TS_P_DIVIDE_WITH_ROUND_UP(hMuxTS->status.stMemoryConfig.astMemoryEntry[BMUXlib_TS_P_MemoryEntryType_eUserData][BMUXlib_TS_P_InputType_eSystem].uiCount, hMuxTS->status.stStartSettings.uiNumValidUserdataPIDs);
            stListCreateSettings.eType = BMUXlib_List_Type_eFIFO;
            for (i = 0; i < hMuxTS->status.stStartSettings.uiNumValidUserdataPIDs; i++)
            {
               rc = BERR_TRACE(BMUXlib_List_Create(&hMuxTS->hUserdataPendingList[i], &stListCreateSettings));
               if (BERR_SUCCESS != rc)
               {
                  break;
               }
            }
         }

         /* create the input group for the active inputs */
         if (BERR_SUCCESS == rc)
         {
            BMUXlib_InputGroup_CreateSettings stCreateSettings;
            BMUXlib_InputGroup_GetDefaultCreateSettings(&stCreateSettings);
            stCreateSettings.uiInputCount = hMuxTS->status.uiNumInputs;
            stCreateSettings.pInputTable = aInputTable;
            rc = BMUXlib_InputGroup_Create(&hMuxTS->status.hInputGroup, &stCreateSettings);
            if (BERR_SUCCESS == rc)
            {
               BMUXlib_InputGroup_Settings stSettings;

               BMUXlib_InputGroup_GetSettings(hMuxTS->status.hInputGroup, &stSettings);
               /* set the selector function to be used by InputGroup_GetNextInput() */
               switch ( hMuxTS->status.stStartSettings.eInterleaveMode )
               {
                  case BMUXlib_TS_InterleaveMode_ePTS:
                     stSettings.fSelector = BMUXlib_InputGroup_DescriptorSelectLowestDTS;
                     break;

                  default:
                     stSettings.fSelector = BMUXlib_InputGroup_DescriptorSelectLowestESCR;
               }

               rc = BMUXlib_InputGroup_SetSettings(hMuxTS->status.hInputGroup, &stSettings);
            }

            /* userdata */
            if (BERR_SUCCESS == rc)
               rc = BMUXlib_TS_P_UserdataInit(hMuxTS);

            /* TS Null Packet and Dummy PES Frame */
            if (BERR_SUCCESS == rc)
            {
               rc = BMUXlib_List_Remove(
                        hMuxTS->hBPPFreeList,
                        (void**) &hMuxTS->status.pDummyPESBuffer
                        );

               BKNI_Memcpy(
                  hMuxTS->status.pDummyPESBuffer,
                  &s_stDummyPESFrame,
                  sizeof( BMUXlib_TS_P_BPPData )
               );

               if ( BERR_SUCCESS == rc )
               {
                  rc = BMUXlib_List_Remove(
                           hMuxTS->hTSPacketFreeList,
                           (void**) &hMuxTS->status.pNullTSPacketBuffer
                           );

                  BKNI_Memcpy(
                     hMuxTS->status.pNullTSPacketBuffer,
                     &s_stNullTSPacket,
                     sizeof( BMUXlib_TS_P_TSPacket )
                  );
               }
            }
         }
         if ( BERR_SUCCESS == rc )
         {
            /* start the mux ... */
            BMUXLIB_TS_P_SET_MUX_STATE(hMuxTS, BMUXlib_State_eStarted);
         }
         else
         {
            /* Startup failed ... cleanup anything that may have been created */
            BMUXlib_TS_P_FreeResources( hMuxTS );
            BMUXlib_TS_P_Reset( hMuxTS );
         }
      }
      else
      {
         /* not enough inputs supplied */
         BMUXlib_TS_P_FreeResources( hMuxTS );
         BMUXlib_TS_P_Reset( hMuxTS );
         rc = BERR_TRACE(BERR_INVALID_PARAMETER);
      }
   } /* end: if state = stopped */
   else
   {
      rc = BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   BDBG_LEAVE( BMUXlib_TS_Start );
   return rc;
}

void
BMUXlib_TS_GetDefaultFinishSettings(
         BMUXlib_TS_FinishSettings *pstFinishSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetDefaultFinishSettings );

   BDBG_ASSERT( pstFinishSettings );

   BKNI_Memset(
            pstFinishSettings,
            0,
            sizeof( BMUXlib_TS_FinishSettings )
            );

   pstFinishSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_FINISHSETTINGS;
   pstFinishSettings->eFinishMode = BMUXlib_FinishMode_ePrepareForStop;

   BDBG_LEAVE( BMUXlib_TS_GetDefaultFinishSettings );
   return;
}

BERR_Code
BMUXlib_TS_Finish(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_FinishSettings *pstFinishSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BMUXlib_TS_Finish );
   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   rc = BMUXlib_TS_P_Finish(hMuxTS, pstFinishSettings);

   BDBG_LEAVE( BMUXlib_TS_Finish );
   return rc;
}

static BERR_Code
BMUXlib_TS_P_Finish(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_FinishSettings *pstFinishSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BMUXlib_TS_P_Finish );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);
   BDBG_ASSERT( pstFinishSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_FINISHSETTINGS == pstFinishSettings->uiSignature );

   switch ( BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
   {
      case BMUXlib_State_eStarted:
         {
            hMuxTS->status.stFinishSettings = *pstFinishSettings;
            switch ( hMuxTS->status.stFinishSettings.eFinishMode )
            {
               case BMUXlib_FinishMode_ePrepareForStop:
                  BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("Finishing Input") );
                  BMUXLIB_TS_P_SET_MUX_STATE(hMuxTS, BMUXlib_State_eFinishingInput);
                  break;

               default:
                  BDBG_ERR(("Unknown finish mode specified"));
                  rc = BERR_TRACE( BERR_INVALID_PARAMETER );
                  break;
            }
         }
         break;

      case BMUXlib_State_eFinishingInput:
      case BMUXlib_State_eFinishingOutput:
      case BMUXlib_State_eFinished:
         /* do nothing if invoked from these states - already finishing! */
         rc = BERR_SUCCESS;
         break;

      default:
         rc = BERR_TRACE( BERR_NOT_SUPPORTED );
         break;
   }

   BDBG_LEAVE( BMUXlib_TS_P_Finish );
   return BERR_TRACE( rc );
}

BERR_Code
BMUXlib_TS_Stop(
         BMUXlib_TS_Handle hMuxTS
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BMUXlib_TS_Stop );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   switch ( BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
   {
      case BMUXlib_State_eStarted:
      case BMUXlib_State_eFinishingInput:
         /* coverity[unterminated_case] */
      case BMUXlib_State_eFinishingOutput:
         BDBG_MSG(("Not finished yet. Output may be corrupt!"));

         /* coverity[fallthrough] */
      case BMUXlib_State_eFinished:
         BMUXLIB_TS_P_SET_MUX_STATE(hMuxTS, BMUXlib_State_eStopped);
#if BMUXLIB_TS_P_DUMP_TRANSPORT_DESC
         {
            uint32_t uiTransportIndex;

            for ( uiTransportIndex = 0; uiTransportIndex < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; uiTransportIndex++ )
            {
               if ( NULL != hMuxTS->status.stOutput.stTransport[uiTransportIndex].stDebug.hDescDumpFile )
               {
                  fclose( hMuxTS->status.stOutput.stTransport[uiTransportIndex].stDebug.hDescDumpFile );
                  hMuxTS->status.stOutput.stTransport[uiTransportIndex].stDebug.hDescDumpFile = NULL;
               }
            }
         }
#endif
#if BMUXLIB_TS_P_DUMP_TRANSPORT_PES
         {
            uint32_t uiTransportIndex;

            for ( uiTransportIndex = 0; uiTransportIndex < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; uiTransportIndex++ )
            {
               if ( NULL != hMuxTS->status.stOutput.stTransport[uiTransportIndex].stDebug.hPESDumpFile )
               {
                  fclose( hMuxTS->status.stOutput.stTransport[uiTransportIndex].stDebug.hPESDumpFile );
                  hMuxTS->status.stOutput.stTransport[uiTransportIndex].stDebug.hPESDumpFile = NULL;
               }
            }
         }
#endif
#if BMUXLIB_TS_P_DUMP_PCR
         if ( NULL != hMuxTS->status.stPCRInfo.hPCRFile )
         {
            fclose(hMuxTS->status.stPCRInfo.hPCRFile);
            hMuxTS->status.stPCRInfo.hPCRFile = NULL;
         }
#endif
         break;

      case BMUXlib_State_eStopped:
         BDBG_WRN(("Already Stopped"));
         break;

      default:
         rc = BERR_INVALID_PARAMETER;
         break;
   }

   /* Return NULL TS Packet and dummy PES frame */
   if ( NULL != hMuxTS->status.pDummyPESBuffer )
   {
      BMUXlib_List_Add(
         hMuxTS->hBPPFreeList,
         hMuxTS->status.pDummyPESBuffer
      );
   }

   if ( NULL != hMuxTS->status.pNullTSPacketBuffer )
   {
      BMUXlib_List_Add(
         hMuxTS->hTSPacketFreeList,
         hMuxTS->status.pNullTSPacketBuffer
      );
   }

   /* Reset State */
   BMUXlib_TS_P_FreeResources( hMuxTS );
   BMUXlib_TS_P_Reset( hMuxTS );

   BDBG_LEAVE( BMUXlib_TS_Stop );
   return BERR_TRACE( rc );
}

/***************/
/* System Data */
/***************/
BERR_Code
BMUXlib_TS_P_ValidateSystemDataBuffers(
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount /* Count of system data buffers in array */
         )
{
   BERR_Code rc = BERR_SUCCESS;
   unsigned i;

   for ( i = 0; i < uiCount; i++ )
   {
      if ( (0 == astSystemDataBuffer[i].uiSize)
          || (0 != ( astSystemDataBuffer[i].uiSize % 188 ))) {
         BDBG_ERR(("System buffer @%p size of %d bytes is NOT a multiple of MPEG TS packet size (188 bytes)",
                  (void *)&astSystemDataBuffer[i],
                  (int)astSystemDataBuffer[i].uiSize
                  ));

         rc = BERR_TRACE(BERR_INVALID_PARAMETER);
         break;
      }
      else if (NULL == astSystemDataBuffer[i].hBlock)
      {
         BDBG_ERR(("System buffer @%p size of %d bytes has no data (hBlock = NULL)",
                  (void *)&astSystemDataBuffer[i],
                  (int)astSystemDataBuffer[i].uiSize
                  ));

         rc = BERR_TRACE(BERR_INVALID_PARAMETER);
         break;
      }
   }

   return BERR_TRACE( rc );
}

BERR_Code
BMUXlib_TS_AddSystemDataBuffers(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BMUXlib_TS_AddSystemDataBuffers );

   BDBG_ASSERT( puiQueuedCount );
   BDBG_ASSERT( astSystemDataBuffer );
   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

   *puiQueuedCount = 0;
   rc = BMUXlib_TS_P_ValidateSystemDataBuffers( astSystemDataBuffer, uiCount );

   if ( rc == BERR_SUCCESS )
   {
      if ( BMUXlib_State_eStopped == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
      {
         /* Add the system data to our pre-q */
         *puiQueuedCount = 0;

         while ( ( hMuxTS->status.uiSystemDataPreQCount < BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT )
                 && ( *puiQueuedCount < uiCount ) )
         {
            hMuxTS->astSystemDataPendingListPreQ[hMuxTS->status.uiSystemDataPreQCount] = astSystemDataBuffer[*puiQueuedCount];
            (*puiQueuedCount)++;
            hMuxTS->status.uiSystemDataPreQCount++;
         }
      }
      else
      {
         rc = BMUXlib_TS_P_AddSystemDataBuffers(hMuxTS, astSystemDataBuffer, uiCount, puiQueuedCount);
      }
   }
   BDBG_LEAVE( BMUXlib_TS_AddSystemDataBuffers );
   return BERR_TRACE( rc );
}

BERR_Code
BMUXlib_TS_P_AddSystemDataBuffers(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BMUXlib_TS_P_AddSystemDataBuffers );

   *puiQueuedCount = 0;

   while ( ( uiCount != *puiQueuedCount )
           && ( false == BMUXlib_List_IsFull( hMuxTS->hSystemDataPendingList ) )
         )
   {
      /* SW7425-4643: We cannot support PCR PID on same PID as system data
         for each packet in the system data buffer that is queued up, extract the PID and verify that it
         does not conflict with the PCR pid (from the start settings), or with Audio or Video PIDs
         If PCR PID is zero, start settings have not been set yet, so don't bother checking yet (this is
         being invoked as a pre-Q operation) */
      if (hMuxTS->status.stStartSettings.stPCRData.uiPID != 0)
      {
         void *pDataBase = BMMA_Lock( astSystemDataBuffer[*puiQueuedCount].hBlock );
         unsigned char *pData = (unsigned char *)((uint8_t *) pDataBase + astSystemDataBuffer[*puiQueuedCount].uiBlockOffset);
         unsigned uiLength = astSystemDataBuffer[*puiQueuedCount].uiSize;

         while ((uiLength >= 188) && (rc == BERR_SUCCESS))
         {
            uint16_t uiPID = ((uint16_t)(pData[1] & 0x1F) << 8) | pData[2];

            if (hMuxTS->status.aFoundPIDs[uiPID])
            {
               if (uiPID == hMuxTS->status.stStartSettings.stPCRData.uiPID)
                  BDBG_ERR(("Mux does not support PCR on same PID as System Data: %x (use a separate PID for PCR)", uiPID));
               else
                  BDBG_ERR(("System Data PID %x is aleady in use by Audio or Video input", uiPID));
               rc = BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            uiLength -= 188;
            pData += 188;
         }
         BMMA_Unlock( astSystemDataBuffer[*puiQueuedCount].hBlock, pDataBase );
         if (rc != BERR_SUCCESS)
            break;
      }
      hMuxTS->astSystemDataPendingList[hMuxTS->status.uiSystemDataPendingListWriteOffset] = astSystemDataBuffer[*puiQueuedCount];

      rc = BERR_TRACE(BMUXlib_List_Add(
               hMuxTS->hSystemDataPendingList,
               (void *) &hMuxTS->astSystemDataPendingList[hMuxTS->status.uiSystemDataPendingListWriteOffset]
               ));
      if (rc != BERR_SUCCESS)
      {
         break;
      }

#ifdef BMUXLIB_TS_P_TEST_MODE
      /* SW7425-3958: do not write data to CSV files if these entries are due to move of data from pre-Q to pending Q */
      if (astSystemDataBuffer != hMuxTS->astSystemDataPendingListPreQ)
      {
         /* Write CSV containing info about system data: offset in data file, length, insertion interval, etc */
         /* system data is written as raw binary data to the data file */
         FILE *hCSVFile = hMuxTS->status.stSystemDataInfo.hCSVFile;
         FILE *hDataFile = hMuxTS->status.stSystemDataInfo.hDataFile;

         if (!hMuxTS->status.stSystemDataInfo.bCSVOpened)
         {
            char fname[256];
            sprintf(fname, "BMUXlib_TS_SystemData_%2.2d.csv", hMuxTS->stCreateSettings.uiMuxId);
            hCSVFile = fopen(fname, "w");
            if (NULL == hCSVFile)
               BDBG_ERR(("Error Creating System Data CSV File (%s)", fname));
            hMuxTS->status.stSystemDataInfo.hCSVFile = hCSVFile;
            sprintf(fname, "BMUXlib_TS_SystemData_%2.2d.dat", hMuxTS->stCreateSettings.uiMuxId);
            hDataFile = fopen(fname, "wb");
            if (NULL == hDataFile)
               BDBG_ERR(("Error Creating System Data File (%s)", fname));
            hMuxTS->status.stSystemDataInfo.hDataFile = hDataFile;

            hMuxTS->status.stSystemDataInfo.bCSVOpened = true;

            if (hCSVFile != NULL)
               fprintf(hCSVFile, "offset, length, insertion_interval\n");
         }
         if (hCSVFile != NULL && hDataFile != NULL)
         {
            void *pBufferBaseAddress = BMMA_Lock( hMuxTS->astSystemDataPendingList[hMuxTS->status.uiSystemDataPendingListWriteOffset].hBlock );
            fprintf(hCSVFile, "%u, %u, %u\n", (unsigned)ftell(hDataFile), astSystemDataBuffer[*puiQueuedCount].uiSize,
               astSystemDataBuffer[*puiQueuedCount].uiTimestampDelta);
            fwrite((void*)((unsigned)pBufferBaseAddress + astSystemDataBuffer[*puiQueuedCount].uiBlockOffset), sizeof(uint8_t),
               astSystemDataBuffer[*puiQueuedCount].uiSize, hDataFile);
            BMMA_Unlock( hMuxTS->astSystemDataPendingList[hMuxTS->status.uiSystemDataPendingListWriteOffset].hBlock, pBufferBaseAddress );
         }
      }
#endif
      hMuxTS->status.uiSystemDataPendingListWriteOffset = (hMuxTS->status.uiSystemDataPendingListWriteOffset + 1) % hMuxTS->status.uiSystemDataMaxCount;

      (*puiQueuedCount)++;
   } /* end: while data to queue && pending list not full */
   BDBG_LEAVE( BMUXlib_TS_P_AddSystemDataBuffers );
   return BERR_TRACE( rc );
}

BERR_Code
BMUXlib_TS_GetCompletedSystemDataBuffers(
         BMUXlib_TS_Handle hMuxTS,
         size_t *puiCompletedCount /* Returns count of system data buffers fully muxed */
         )
{
   BDBG_ENTER( BMUXlib_TS_GetCompletedSystemDataBuffers );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);
   BDBG_ASSERT( puiCompletedCount );

   *puiCompletedCount = BMUXLIB_TS_P_GET_SYS_DATA_COMP_CNT(hMuxTS);
   BMUXLIB_TS_P_SET_SYS_DATA_COMP_CNT(hMuxTS, 0);

   BDBG_LEAVE( BMUXlib_TS_GetCompletedSystemDataBuffers );
   return BERR_TRACE( BERR_SUCCESS );
}

/**********/
/* Status */
/**********/
void
BMUXlib_TS_GetStatus(
   BMUXlib_TS_Handle hMuxTS,
   BMUXlib_TS_Status *pstStatus
   )
{
   BDBG_ENTER( BMUXlib_TS_GetStatus );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);
   BDBG_ASSERT( pstStatus );

   *pstStatus = hMuxTS->status.stStatus;

   BDBG_LEAVE( BMUXlib_TS_GetStatus );
}

/***********/
/* Execute */
/***********/
BERR_Code
BMUXlib_TS_DoMux(
   BMUXlib_TS_Handle hMuxTS,
   BMUXlib_DoMux_Status *pstStatus
   )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BMUXlib_TS_DoMux );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);
   BDBG_ASSERT( pstStatus );

   BKNI_Memset( pstStatus, 0, sizeof( BMUXlib_DoMux_Status ) );

   /* Debug code to print ESCR for current execution */
   if (NULL != hMuxTS->status.stStartSettings.transport.stDeviceInterface.fGetTransportStatus)
   {
      hMuxTS->status.stPreviousTransportStatus = hMuxTS->status.stTransportStatus;

      hMuxTS->status.stStartSettings.transport.stDeviceInterface.fGetTransportStatus(
               hMuxTS->status.stStartSettings.transport.stDeviceInterface.pContext,
               &hMuxTS->status.stTransportStatus
               );

      if ( false == hMuxTS->status.stStartSettings.bNonRealTimeMode )
      {
         if ( true == hMuxTS->status.bTransportStatusValid )
         {
            if ( hMuxTS->status.stTransportStatus.uiESCR > hMuxTS->status.stPreviousTransportStatus.uiESCR )
            {
               uint32_t uiMaxExpectedMSP = hMuxTS->status.stStartSettings.uiServiceLatencyTolerance + hMuxTS->status.stDoMuxStatus.uiNextExecutionTime;
               uint32_t uiDeltaESCR = hMuxTS->status.stTransportStatus.uiESCR - hMuxTS->status.stPreviousTransportStatus.uiESCR;
               uiDeltaESCR /= 27000;

               if ( uiDeltaESCR > uiMaxExpectedMSP )
               {
                  BDBG_WRN(("MUX Service Latency Too Large! (%d > %d ms)", uiDeltaESCR, uiMaxExpectedMSP ));
               }
            }
         }
      }

      hMuxTS->status.bTransportStatusValid = true;

      BDBG_MSG(("STC = "BDBG_UINT64_FMT", PACING = %08x",
               BDBG_UINT64_ARG(hMuxTS->status.stTransportStatus.uiSTC),
               hMuxTS->status.stTransportStatus.uiESCR
               ));
   }

   /* SW7425-5841: Update efficiency statistics - subtract oldest stats */
   {
      BMUXlib_TS_P_DataType eDataType;
      BMUXlib_TS_P_SourceType eSourceType;

      for ( eDataType = 0; eDataType < BMUXlib_TS_P_DataType_eMax; eDataType ++ )
      {
         for ( eSourceType = 0; eSourceType < BMUXlib_TS_P_SourceType_eMax; eSourceType++ )
         {
            hMuxTS->status.stEfficiencyStats.uiTotalBytesPerInput[eDataType][eSourceType] -= hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
            hMuxTS->status.stEfficiencyStats.uiTotalBytesPerDataType[eDataType] -= hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
            hMuxTS->status.stEfficiencyStats.uiTotalBytesPerSourceType[eSourceType] -= hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
            hMuxTS->status.stEfficiencyStats.uiTotalBytes -= hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
            hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex] = 0;
         }
      }
      hMuxTS->status.stEfficiencyStats.uiTotalBytes -= hMuxTS->status.stEfficiencyStats.uiPacketizationOverhead[hMuxTS->status.stEfficiencyStats.uiIndex];
      hMuxTS->status.stEfficiencyStats.uiPacketizationOverhead[hMuxTS->status.stEfficiencyStats.uiIndex] = 0;

      hMuxTS->status.stEfficiencyStats.uiTotalTimeInMs -= hMuxTS->status.stEfficiencyStats.uiTimeInMs[hMuxTS->status.stEfficiencyStats.uiIndex];
      hMuxTS->status.stEfficiencyStats.uiTimeInMs[hMuxTS->status.stEfficiencyStats.uiIndex] = hMuxTS->status.stStartSettings.uiServicePeriod;

      if ( true == hMuxTS->status.bTransportStatusValid )
      {
         uint32_t uiDeltaESCR = hMuxTS->status.stTransportStatus.uiESCR - hMuxTS->status.stPreviousTransportStatus.uiESCR;

         hMuxTS->status.stEfficiencyStats.uiTimeInMs[hMuxTS->status.stEfficiencyStats.uiIndex] = uiDeltaESCR/27000;
      }
   }

   /* SW7425-659: Initialize the system data pkt2pkt delta */
   if ( false == hMuxTS->status.bBTPSent )
   {
      uint64_t uiPacket2PacketTimestampDelta = ((uint64_t)BMUXlib_TS_P_TSPacket_MAXSIZE * 8 * 27000000) / hMuxTS->status.stMuxSettings.uiSystemDataBitRate;

      hMuxTS->status.stOutput.stTransport[hMuxTS->status.stInput.system.uiTransportChannelIndex].stPacket2PacketDelta.bValid = true;
      hMuxTS->status.stOutput.stTransport[hMuxTS->status.stInput.system.uiTransportChannelIndex].stPacket2PacketDelta.uiValue = uiPacket2PacketTimestampDelta;
   }

   if ( ( BMUXlib_State_eStopped != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
        && ( BMUXlib_State_eFinished != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
       )
   {
      BMUXlib_TS_P_ProcessCompletedBuffers( hMuxTS );

      if ( BMUXlib_State_eFinishingOutput == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
      {
         unsigned uiIndex;
         bool bMuxOutputComplete = true;

         /* Make sure all transport descriptors have been consumed */
         for ( uiIndex = 0; uiIndex < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; uiIndex++ )
         {
            if ( true == hMuxTS->status.stOutput.stTransport[uiIndex].bActive )
            {
               if ( false == BMUXlib_List_IsEmpty( hMuxTS->hTransportDescriptorPendingList[uiIndex] ) )
               {
                  BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("TRANSPORT[%d] has pending descriptors", uiIndex) );
                  bMuxOutputComplete = false;
                  break;
               }
            }
         }

         if ( true == bMuxOutputComplete )
         {
            BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("Finished") );
            BMUXLIB_TS_P_SET_MUX_STATE(hMuxTS, BMUXlib_State_eFinished);
         }
      }
      else if ( ( BMUXlib_State_eFinishingInput == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
                || ( BMUXlib_State_eStarted == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
              )
      {
         bool bMuxInputComplete = true;

         BMUXlib_TS_P_ProcessNewBuffers( hMuxTS );

         /* Check if we've seen an EOS on all active A/V Channels */
         {
            unsigned uiIndex;

             /* Check if EOS seen on all input channels associated with this Transport Channel */
             for ( uiIndex = 0; uiIndex < hMuxTS->status.uiNumInputs ; uiIndex++ )
             {
               if ( false == hMuxTS->status.stInputMetaData[uiIndex].bEOSSeen )
               {
                  bMuxInputComplete = false;
                  break;
               }
             }
         }

         /* SW7425-998: Auto-Finish if EOS is seen on all active inputs */
         if ( BMUXlib_State_eStarted == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
         {
            if ( true == bMuxInputComplete )
            {
               BMUXlib_TS_FinishSettings stFinishSettings;

               BMUXlib_TS_GetDefaultFinishSettings( &stFinishSettings );

               BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("Auto Finish") );
               BMUXlib_TS_P_Finish(
                        hMuxTS,
                        &stFinishSettings
                        );
            }
         }

            /* SW7425-831: If we've seen an EOS on all inputs, and we're in NRT mode,
             * then we want to flush all output channels by inserting a dummy descriptor
             * with an ESCR=to the largestESCR.  That way, the XPT PB Pause logic won't
             * trigger if one output finishes before the others.
             */

         /* SW7425-999: In NRT mode, we need to insert dummy descriptors for all
          * inputs that have seen EOS to ensure the they don't cause the non-EOS
          * inputs to stall
          */
         if ( true == hMuxTS->status.stStartSettings.bNonRealTimeMode )
         {
            bMuxInputComplete &= BMUXlib_TS_P_Flush( hMuxTS, bMuxInputComplete);
         }

         if ( BMUXlib_State_eFinishingInput == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
         {
            /* SW7425-2825/SW7425-5055: Since system data is queued up until the lastESCR
             * that has been queued to the transport, we need to continue to process
             * system data until all pending descriptors have been queued to the transport.
             */
            BMUXlib_TS_P_ProcessSystemData( hMuxTS );
            {
               unsigned uiTransportChannelIndex;

               for ( uiTransportChannelIndex = 0; uiTransportChannelIndex < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; uiTransportChannelIndex++ )
               {
                  size_t uiNumEntries;

                  if ( NULL != hMuxTS->hTransportDescriptorPendingList[uiTransportChannelIndex] )
                  {
                     BMUXlib_List_GetNumEntries(
                        hMuxTS->hTransportDescriptorPendingList[uiTransportChannelIndex],
                        &uiNumEntries
                        );

                     bMuxInputComplete &= (uiNumEntries == hMuxTS->status.stOutput.stTransport[uiTransportChannelIndex].uiDescriptorsQueued);
                  }
               }
            }

            if ( true == bMuxInputComplete )
            {
               BDBG_MODULE_MSG( BMUXLIB_TS_FINISH, ("Finishing Output") );

               BMUXLIB_TS_P_SET_MUX_STATE(hMuxTS, BMUXlib_State_eFinishingOutput);
            }
         }
      }

      if ( false == hMuxTS->status.bTransportConfigured )
      {
         BMUXlib_TS_P_ConfigureTransport( hMuxTS );

         /* SW7425-659: In NRT mode, since TS MUXlib is seeding the PACING_COUNTER,
          * we need to resample the status to ensure subsequent code
          * (E.g. Late ESCR detection) uses a valid value
          */
         if (NULL != hMuxTS->status.stStartSettings.transport.stDeviceInterface.fGetTransportStatus)
         {
            hMuxTS->status.stStartSettings.transport.stDeviceInterface.fGetTransportStatus(
                     hMuxTS->status.stStartSettings.transport.stDeviceInterface.pContext,
                     &hMuxTS->status.stTransportStatus
                     );

            BDBG_MSG(("STC = "BDBG_UINT64_FMT", PACING = %08x (After Config)",
                     BDBG_UINT64_ARG(hMuxTS->status.stTransportStatus.uiSTC),
                     hMuxTS->status.stTransportStatus.uiESCR
                     ));
         }
      }

      if ( true == hMuxTS->status.bTransportConfigured )
      {
         BMUXlib_TS_P_ScheduleProcessedBuffers( hMuxTS );
      }
   }

   if ( ( BMUXlib_State_eStopped != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
        && ( BMUXlib_State_eFinished != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
       )
   {
      unsigned uiTransportIndex;

      for ( uiTransportIndex = 0; uiTransportIndex < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; uiTransportIndex++ )
      {
         if ( true == hMuxTS->status.stOutput.stTransport[uiTransportIndex].bActive )
         {

            size_t uiTransportPending;

            BMUXlib_List_GetNumEntries(
                     hMuxTS->hTransportDescriptorPendingList[uiTransportIndex],
                     &uiTransportPending
                     );

            BDBG_MODULE_MSG(BMUXLIB_TS_PENDING, ("T[%2d]=%08x (ESCR=%08x --> %08x)",
               uiTransportIndex,
               (int)uiTransportPending,
               hMuxTS->status.stOutput.stTransport[uiTransportIndex].stTimingPending.uiLastStartingESCR,
               hMuxTS->status.stOutput.stTransport[uiTransportIndex].stTimingPending.uiNextExpectedESCR
            ));

            BDBG_MODULE_MSG(BMUXLIB_TS_QUEUED, ("T[%2d]=%08x (ESCR=%08x --> %08x, PKT2PKT=%08x (%d ms))",
               uiTransportIndex,
               (int)uiTransportPending,
               hMuxTS->status.stOutput.stTransport[uiTransportIndex].stTimingQueued.uiLastStartingESCR,
               hMuxTS->status.stOutput.stTransport[uiTransportIndex].stTimingQueued.uiNextExpectedESCR,
               hMuxTS->status.stOutput.stTransport[uiTransportIndex].stTimingQueued.uiCurrentPacket2PacketTimestampDelta,
               hMuxTS->status.stOutput.stTransport[uiTransportIndex].stTimingQueued.uiCurrentPacket2PacketTimestampDelta / 27000
            ));

            BDBG_MODULE_MSG(BMUXLIB_TS_TRANSPORT, ("T[%2d] added/completed/returned = %08x,%08x,%08x",
               uiTransportIndex,
               hMuxTS->status.stTransport[uiTransportIndex].uiDescriptorsAdded,
               hMuxTS->status.stTransport[uiTransportIndex].uiDescriptorsCompleted,
               hMuxTS->status.stTransport[uiTransportIndex].uiDescriptorsReturned
            ));
         }
      }
   }

   pstStatus->eState = BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS);

   pstStatus->uiNextExecutionTime = hMuxTS->status.stStartSettings.uiServicePeriod + hMuxTS->status.iExecutionTimeAdjustment;
   hMuxTS->status.iExecutionTimeAdjustment = 0;

   /* SW7425-3684: Update completed duration */
   if ( ( BMUXlib_State_eStopped != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
        && ( BMUXlib_State_eFinished != BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
        && ( NULL != hMuxTS->status.hInputGroup )
       )
   {
      BMUXlib_InputGroup_Status stStatus;

      BMUXlib_InputGroup_GetStatus(
         hMuxTS->status.hInputGroup,
         &stStatus
         );

      pstStatus->uiCompletedDuration = stStatus.uiDuration;
   }

   /* SW7425-5841: Update/Calc/Print efficiency statistics - add newest stats */
   {
      BMUXlib_TS_P_DataType eDataType;
      BMUXlib_TS_P_SourceType eSourceType;

      for ( eDataType = 0; eDataType < BMUXlib_TS_P_DataType_eMax; eDataType ++ )
      {
         for ( eSourceType = 0; eSourceType < BMUXlib_TS_P_SourceType_eMax; eSourceType++ )
         {
            hMuxTS->status.stEfficiencyStats.uiTotalBytesPerInput[eDataType][eSourceType] += hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
            hMuxTS->status.stEfficiencyStats.uiTotalBytesPerDataType[eDataType] += hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
            hMuxTS->status.stEfficiencyStats.uiTotalBytesPerSourceType[eSourceType] += hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
            hMuxTS->status.stEfficiencyStats.uiTotalBytes += hMuxTS->status.stEfficiencyStats.uiNumBytes[eDataType][eSourceType][hMuxTS->status.stEfficiencyStats.uiIndex];
         }
      }
      hMuxTS->status.stEfficiencyStats.uiTotalBytes += hMuxTS->status.stEfficiencyStats.uiPacketizationOverhead[hMuxTS->status.stEfficiencyStats.uiIndex];

      hMuxTS->status.stEfficiencyStats.uiTotalTimeInMs += hMuxTS->status.stEfficiencyStats.uiTimeInMs[hMuxTS->status.stEfficiencyStats.uiIndex];

      hMuxTS->status.stEfficiencyStats.uiIndex++;
      hMuxTS->status.stEfficiencyStats.uiIndex %= BMUXLib_TS_P_STATS_MAX_MSP_COUNT;

      if ( ( 0 != hMuxTS->status.stEfficiencyStats.uiTotalTimeInMs/1000 ) && ( 0 != hMuxTS->status.stEfficiencyStats.uiTotalBytes ) )
      {
         hMuxTS->status.stStatus.stAverageBitrate.uiVideo = ( hMuxTS->status.stEfficiencyStats.uiTotalBytesPerInput[BMUXlib_TS_P_DataType_eCDB][BMUXlib_TS_P_SourceType_eVideo] * 8 ) / (hMuxTS->status.stEfficiencyStats.uiTotalTimeInMs/1000);
         hMuxTS->status.stStatus.stAverageBitrate.uiAudio = ( hMuxTS->status.stEfficiencyStats.uiTotalBytesPerInput[BMUXlib_TS_P_DataType_eCDB][BMUXlib_TS_P_SourceType_eAudio] * 8 ) / (hMuxTS->status.stEfficiencyStats.uiTotalTimeInMs/1000);
         hMuxTS->status.stStatus.stAverageBitrate.uiSystemData = hMuxTS->status.stEfficiencyStats.uiTotalBytesPerSourceType[BMUXlib_TS_P_SourceType_eSystem] * 8 / (hMuxTS->status.stEfficiencyStats.uiTotalTimeInMs/1000);
         hMuxTS->status.stStatus.stAverageBitrate.uiUserData = hMuxTS->status.stEfficiencyStats.uiTotalBytesPerSourceType[BMUXlib_TS_P_SourceType_eUserdata] * 8 / (hMuxTS->status.stEfficiencyStats.uiTotalTimeInMs/1000);
         hMuxTS->status.stStatus.uiEfficiency = (hMuxTS->status.stEfficiencyStats.uiTotalBytesPerInput[BMUXlib_TS_P_DataType_eCDB][BMUXlib_TS_P_SourceType_eVideo] + hMuxTS->status.stEfficiencyStats.uiTotalBytesPerInput[BMUXlib_TS_P_DataType_eCDB][BMUXlib_TS_P_SourceType_eAudio] + hMuxTS->status.stEfficiencyStats.uiTotalBytesPerDataType[BMUXlib_TS_P_DataType_eUserdataPTS] + hMuxTS->status.stEfficiencyStats.uiTotalBytesPerDataType[BMUXlib_TS_P_DataType_eUserdataLocal]) * 100 / hMuxTS->status.stEfficiencyStats.uiTotalBytes;
         hMuxTS->status.stStatus.uiTotalBytes = hMuxTS->status.stEfficiencyStats.uiTotalBytesWritten;
         BDBG_MODULE_MSG( BMUXLIB_TS_STATS, ("Avg. Bitrate (kbps): V=%5u, A=%3u, S=%2u, U=%3u [%3u%%] ("BDBG_UINT64_FMT")",
            hMuxTS->status.stStatus.stAverageBitrate.uiVideo/1000,
            hMuxTS->status.stStatus.stAverageBitrate.uiAudio/1000,
            hMuxTS->status.stStatus.stAverageBitrate.uiSystemData/1000,
            hMuxTS->status.stStatus.stAverageBitrate.uiUserData/1000,
            hMuxTS->status.stStatus.uiEfficiency,
            BDBG_UINT64_ARG(hMuxTS->status.stStatus.uiTotalBytes)
         ) );
      }
   }

   /* Call MCPB DoMux*( as needed */
   {
      unsigned i;
      unsigned uiEndESCR = 0;
      bool bEndESCRValid = false;

      /* Determine the smallest active queued next ESCR to send to MCPB DoMux */
      {
         uint32_t uiTransportChannelIndex;

         /* Determine the largest ESCR to use for flushing the outputs */
         for ( uiTransportChannelIndex = 0; uiTransportChannelIndex < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; uiTransportChannelIndex++ )
         {
            signed iDeltaESCR = -1;

            /* Ignore this transport channel if it is not active */
            if ( false == hMuxTS->status.stOutput.stTransport[uiTransportChannelIndex].bActive ) continue;

            if ( BMUXlib_State_eFinishingOutput == BMUXLIB_TS_P_GET_MUX_STATE(hMuxTS) )
            {
               /* When finishing output, do not use system data channel for computing uiEndESCR since PCRs are generated internally based on the input timing */
               if ( uiTransportChannelIndex == hMuxTS->status.stInput.system.uiTransportChannelIndex ) continue;
            }

            /* If a transport queue is empty, check of all associated inputs are done */
            if ( true == BMUXlib_List_IsEmpty( hMuxTS->hTransportDescriptorPendingList[uiTransportChannelIndex] ) )
            {
               bool bAllInputsForTransportChannelHasSeenEOS = false;
               unsigned uiIndex;

               /* Ignore this transport channel, if EOS seen on all associated inputs */
               for ( uiIndex = 0; uiIndex < hMuxTS->status.uiNumInputs ; uiIndex++ )
               {
                  if ( uiTransportChannelIndex == hMuxTS->status.stInputMetaData[uiIndex].uiTransportChannelIndex )
                  {
                     bAllInputsForTransportChannelHasSeenEOS = hMuxTS->status.stInputMetaData[uiIndex].bEOSSeen;

                     if ( false == bAllInputsForTransportChannelHasSeenEOS )
                     {
                        break;
                     }
                  }
               }
               if ( true == bAllInputsForTransportChannelHasSeenEOS ) continue;
            }

            if ( true == bEndESCRValid )
            {
               iDeltaESCR = hMuxTS->status.stOutput.stTransport[uiTransportChannelIndex].stTimingQueued.uiNextExpectedESCR - uiEndESCR;
            }

            if ( iDeltaESCR < 0 )
            {
               uiEndESCR = hMuxTS->status.stOutput.stTransport[uiTransportChannelIndex].stTimingQueued.uiNextExpectedESCR;
               bEndESCRValid = true;
            }
         }
      }

      if ( true == bEndESCRValid )
      {
         for ( i = 0; i < BMUXLIB_TS_MAX_TRANSPORT_INSTANCES; i++ )
         {
            if ( NULL != hMuxTS->status.stOutput.stMCPB[i].hMuxMCPB )
            {
               BMUXlib_TS_MCPB_DoMux( hMuxTS->status.stOutput.stMCPB[i].hMuxMCPB, uiEndESCR );
            }
         }
      }
   }

   hMuxTS->status.stDoMuxStatus = *pstStatus;

   BDBG_LEAVE( BMUXlib_TS_DoMux );
   return BERR_TRACE( rc );
}
