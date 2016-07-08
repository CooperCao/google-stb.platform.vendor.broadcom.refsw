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

#ifndef BMUXLIB_TS_H_
#define BMUXLIB_TS_H_

#include "bmuxlib.h"
#include "bavc_xpt.h"

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif
#endif

/* 7425 Specific Notes:
 *  - PCR and all other periodic system data need to use the same XPT PB Handle
 *  - Audio needs a dedicated XPT PB Handle
 *  - Video needs a dedicated XPT PB Handle
 *  - Only one video PID supported
 *  - Only one audio PID supported
 *  - PCR PID cannot be the same as video or audio PIDs
 */
#define BMUXLIB_TS_MAX_SYS_DATA_BR     27000000
#define BMUXLIB_TS_MIN_A2PDELAY        27000    /* 1ms minimum */

/***********************/
/* Transport Interface */
/***********************/
typedef struct BMUXlib_TS_TransportDescriptor
{
   uint64_t uiBufferOffset; /* physical offset of data to be muxed */
   size_t uiBufferLength; /* length in bytes of data to be muxed */

   BAVC_TsMux_DescConfig stTsMuxDescriptorConfig;
} BMUXlib_TS_TransportDescriptor;

/* BMUXlib_AddTransportDescriptors -
 * Adds transport descriptors for muxing.
 */
typedef BERR_Code
(*BMUXlib_TS_AddTransportDescriptors)(
         void *pvContext,
         const BMUXlib_TS_TransportDescriptor *astTransportDescriptors, /* Array of pointers to transport descriptors */
         size_t uiCount, /* Count of descriptors in array */
         size_t *puiQueuedCount /* Count of descriptors queued (*puiQueuedCount <= uiCount) */
         );

/* BMUXlib_GetCompletedTransportDescriptors -
 * Returns the count of transport descriptors completed
 * since the *previous* call to BMUXlib_TS_GetCompletedTransportDescriptors
 */
typedef BERR_Code
(*BMUXlib_TS_GetCompletedTransportDescriptors)(
         void *pvContext,
         size_t *puiCompletedCount /* Count of descriptors completed */
         );

typedef struct BMUXlib_TS_TransportSettings
{
      /* TODO: Add settings */
      uint32_t uiMuxDelay; /* in milliseconds */

      /* Each transport channel's pacing counter is seeded with:
       *   If bNonRealTimeMode=false: STC-uiMuxDelay
       *   If bNonRealTimeMode=true : uiPacingCounter
       */

      bool bNonRealTimeMode; /* Non Real Time Mode (NRT/AFAP) */

      struct
      {
            unsigned uiPacingCounter; /* in 27Mhz clock ticks */
      } stNonRealTimeSettings;
} BMUXlib_TS_TransportSettings;

typedef BERR_Code
(*BMUXlib_TS_GetTransportSettings)(
         void *pvContext,
         BMUXlib_TS_TransportSettings *pstTransportSettings
         );

typedef BERR_Code
(*BMUXlib_TS_SetTransportSettings)(
         void *pvContext,
         const BMUXlib_TS_TransportSettings *pstTransportSettings
         );

typedef struct BMUXlib_TS_TransportStatus
{
      uint64_t uiSTC; /* 42-bit value in 27 Mhz */
      uint32_t uiESCR; /* 32-bit value in 27Mhz; */
      /* TODO: What other status? */
} BMUXlib_TS_TransportStatus;

typedef BERR_Code
(*BMUXlib_TS_GetTransportStatus)(
         void *pvContext,
         BMUXlib_TS_TransportStatus *pstTransportStatus
         );

typedef struct BMUXlib_TS_TransportDeviceInterface
{
      void *pContext;
      BMUXlib_TS_GetTransportSettings fGetTransportSettings;
      BMUXlib_TS_SetTransportSettings fSetTransportSettings;
      BMUXlib_TS_GetTransportStatus fGetTransportStatus;
} BMUXlib_TS_TransportDeviceInterface;

typedef struct BMUXlib_TS_TransportChannelInterface
{
      void *pContext;
      BMUXlib_TS_AddTransportDescriptors fAddTransportDescriptors;
      BMUXlib_TS_GetCompletedTransportDescriptors fGetCompletedTransportDescriptors;
} BMUXlib_TS_TransportChannelInterface;

/***********************/
/* User Data Interface */
/***********************/

/* Subsequent calls to BMUXlib_GetUserDataBuffer are non-destructive.  I.e. Any new data added is appended to the end of the existing buffers
* The userdata is expected to arrive as PES packets encapsulated in TS packets.
* All TS packets in the buffer must have the same PID.
* The buffer must start on a TS packet boundary
*/
typedef BERR_Code
(*BMUXlib_TS_GetUserDataBuffer)(
   void *pvContext,
   BMMA_Block_Handle *phBlock, /* Pointer to a block of TS UserData Packets. */
   size_t *puiBlockOffset0, /* Offset from block to start of data */
   size_t *puiLength0, /* Length of data */
   size_t *puiBlockOffset1, /* Offset from block to start of data (after wrap) */
   size_t *puiLength1 /* Length of data (after wrap) */
   );

typedef BERR_Code
(*BMUXlib_TS_ConsumeUserDataBuffer)(
   void *pvContext,
   size_t uiNumBytesConsumed
   );

typedef struct BMUXlib_UserDataInterface
{
      void *pContext;
      BMUXlib_TS_GetUserDataBuffer fGetUserDataBuffer;
      BMUXlib_TS_ConsumeUserDataBuffer fConsumeUserDataBuffer;
} BMUXlib_TS_UserDataInterface;

/******************/
/* Create/Destroy */
/******************/
typedef struct BMUXlib_TS_P_Context *BMUXlib_TS_Handle;

typedef struct BMUXlib_TS_MemoryBuffers
{
   void* pSystemBuffer; /* Memory only accessed by the host. If NULL, allocated in BMUXlib_TS_Create() from system/kernel memory. */
   BMMA_Block_Handle hSharedBufferBlock; /* Memory block accessed by both the host and device(s). If NULL, allocated in BMUXlib_TS_Create() from specified hMma. */
} BMUXlib_TS_MemoryBuffers;

typedef struct BMUXlib_TS_MemoryConfig
{
   size_t uiSystemBufferSize; /* Only accessed by the host */
   size_t uiSharedBufferSize; /* Accessed by both the host and device(s) */
} BMUXlib_TS_MemoryConfig;

typedef struct BMUXlib_TS_CreateSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BMUXlib_TS_GetDefaultCreateSettings() */

      BMMA_Heap_Handle hMma; /* Required. Must be accessible by the host CPU. */

      BMUXlib_TS_MemoryConfig stMemoryConfig; /* The worst case memory usage for ALL possible mux
                                               * configurations used for this instance of TS Muxlib.
                                               * Should be populated using BMUXlib_TS_GetMemoryConfig()
                                               */
      BMUXlib_TS_MemoryBuffers stMemoryBuffers; /* (OPTIONAL) NULL, allocated in BMUXlib_TS_Create() */

      unsigned uiMuxId;     /* For debug: indicates the ID of this muxlib for multiple transcode systems */
} BMUXlib_TS_CreateSettings;

void
BMUXlib_TS_GetDefaultCreateSettings(
         BMUXlib_TS_CreateSettings *pCreateSettings
         );

/* BMUXlib_TS_Create - Allocates all system/device memory required for mux operation */
BERR_Code
BMUXlib_TS_Create(
         BMUXlib_TS_Handle *phMuxTS,  /* [out] TSMuxer handle returned */
         const BMUXlib_TS_CreateSettings *pstCreateSettings
         );

/* BMUXlib_TS_Destroy - Frees all system/device memory allocated */
BERR_Code
BMUXlib_TS_Destroy(
         BMUXlib_TS_Handle hMuxTS
         );

/****************/
/* Mux Settings */
/****************/
#define BMUXLIB_TS_MAX_VIDEO_PIDS 1
#define BMUXLIB_TS_MAX_AUDIO_PIDS 6
#define BMUXLIB_TS_MAX_INPUT_PIDS ( BMUXLIB_TS_MAX_VIDEO_PIDS + BMUXLIB_TS_MAX_AUDIO_PIDS )
#define BMUXLIB_TS_MAX_SYSTEM_PIDS 1
#define BMUXLIB_TS_MAX_TRANSPORT_INSTANCES (BMUXLIB_TS_MAX_VIDEO_PIDS + BMUXLIB_TS_MAX_AUDIO_PIDS + BMUXLIB_TS_MAX_SYSTEM_PIDS)
#define BMUXLIB_TS_MAX_USERDATA_PIDS 8

typedef struct BMUXlib_TS_MuxSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BMUXlib_TS_GetDefaultMuxSettings() */

      /* System Data */
      unsigned uiSystemDataBitRate; /* in bits/sec.  1Mbps = 1000000.
                                     * If system data is split across multiple packets,
                                     * this value determines the gap between the packets.
                                     */

      /* SW7436-1363: Disables/Enables specified input from muxing into output.
       * Any data from the specified input is discarded instead of being muxed into the output.
       * The enable/disable will always occur on frame boundaries
       * For video, the enable/disable will occur on RAP frame boundaries.
       */
      struct {
         bool bVideo[BMUXLIB_TS_MAX_VIDEO_PIDS];
         bool bAudio[BMUXLIB_TS_MAX_AUDIO_PIDS];
      } stInputEnable;
} BMUXlib_TS_MuxSettings;

void
BMUXlib_TS_GetDefaultMuxSettings(
         BMUXlib_TS_MuxSettings *pstMuxSettings
         );

BERR_Code
BMUXlib_TS_SetMuxSettings(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_MuxSettings *pstMuxSettings
         );

BERR_Code
BMUXlib_TS_GetMuxSettings(
         BMUXlib_TS_Handle hMuxTS,
         BMUXlib_TS_MuxSettings *pstMuxSettings
         );

/**************/
/* Start/Stop */
/**************/
typedef struct BMUXlib_TS_PCRSystemData
{
      uint16_t uiPID;

      unsigned uiInterval; /* insertion interval (in milliseconds). 0 = disable PCR generation */

      unsigned uiTransportChannelIndex; /* Which transport interface to use for PCR packets */
      unsigned uiPIDChannelIndex;
} BMUXlib_TS_PCRSystemData;

typedef enum BMUXlib_TS_InterleaveMode
{
   BMUXlib_TS_InterleaveMode_eCompliant = 0, /* Use the transmission timestamp provided in the BAVC_CompressedBufferDescriptor (required for HRD compliance) */
   BMUXlib_TS_InterleaveMode_ePTS, /* Use the DTS/PTS provided in the BAVC_CompressedBufferDescriptor to generate a new transmission timestamp (useful to minimize A/V buffering delay)
                                    * Note: PCRs are NOT generated in this mode
                                    */

   BMUXlib_TS_InterleaveMode_eMax
} BMUXlib_TS_InterleaveMode;

typedef struct BMUXlib_TS_StartSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BMUXlib_TS_GetDefaultStartSettings() */

      /* Transport Data */
      struct {
            BMUXlib_TS_TransportDeviceInterface stDeviceInterface;
            BMUXlib_TS_TransportChannelInterface stChannelInterface[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES]; /* The transport interfaces */
      } transport;

      /* Video Data */
      struct {
            BMUXlib_VideoEncoderInterface stInputInterface;
            unsigned uiTransportChannelIndex; /* Which transport channel interface to use for this PID */
            uint16_t uiPID; /* The PID associated with this video data */
            uint8_t uiPESStreamID;
            unsigned uiPIDChannelIndex;
      } video[BMUXLIB_TS_MAX_VIDEO_PIDS];
      unsigned uiNumValidVideoPIDs; /* The number of valid video PIDs in the video encoder interface array */

      /* Audio Data */
      struct {
            BMUXlib_AudioEncoderInterface stInputInterface;
            unsigned uiTransportChannelIndex; /* Which transport channel interface to use for this PID */
            uint16_t uiPID; /* The PID associated with this audio data */
            uint8_t uiPESStreamID;
            unsigned uiPIDChannelIndex;
      } audio[BMUXLIB_TS_MAX_AUDIO_PIDS];
      unsigned uiNumValidAudioPIDs; /* The number of valid audio PIDs in the audio encoder interface array */

      /* System Data */
      BMUXlib_TS_PCRSystemData stPCRData; /* The first PCR packet is inserted before any audio and video
                                           * packets are muxed.  This will ensure that a PCR entry is
                                           * received before any A/V data is received by the decoder
                                           */

      /* User Data */
      /* The userdata is expected to arrive as PES packets encapsulated in TS packets.
         It is assumed that the companion video for the userdata is the first (and expected to be the only)
         video PID - if more than one video PID expected, support can be added at a later date
         It is also assumed that the userdata will be output on the sytem data transport channel.
         If required, support can be added to output to a different channel if/when required
         The application is responsible for ensuring that PIDs used do not conflict with other
         Userdata or System Data PIDs */
      struct {
            BMUXlib_TS_UserDataInterface stUserDataInterface;
      } userdata[BMUXLIB_TS_MAX_USERDATA_PIDS];
      unsigned uiNumValidUserdataPIDs;

   /* Mux Service Period (mux_delay = service_latency_tolerance + service_period) */
   unsigned uiServiceLatencyTolerance; /* (in milliseconds) Indicates the maximum latency
                                        * the muxer is expected to handle without causing
                                        * the transport to underflow.
                                        *
                                        * This value directly affects the overall mux latency
                                        * and the encoder's CDB buffer size
                                        */

   unsigned uiServicePeriod; /* (in milliseconds) Indicates the lowest frequency
                              * the muxer is expected to be executed. 0 indicates
                              * the default service period.
                              *
                              * Smaller Service Period = Higher Frequency of execution
                              *
                              * To prevent underflow at the transport, the application
                              * MUST guarantee that the specified service period can be
                              * met.
                              *
                              * This value directly affects the overall mux latency
                              * and the encoder's CDB buffer size
                              */

   bool bNonRealTimeMode;  /* If FALSE: Normal operation for muxing in real time.  Pacing Counters will be based on STC.
                            *
                            * If TRUE: aka As Fast As Possible (AFAP) Mode. Pacing Counters will be based on initial ESCR(s).
                            */

   unsigned uiA2PDelay;    /* In 27Mhz clock ticks.  Desired "Arrival-to-Presentation" delay.  0 indicates use default.
                            * It is expected that uiA2PDelay >= BVCE_Channel_EncodeSettings.uiA2PDelay
                            * (this value is used to properly allocate memory for userdata usage)
                            */

   bool bSupportTTS;       /* Set to TRUE if enabling TTS (4-byte TS Timestamp) generation in the TS MUX Hardware.
                            * If TRUE: MUXlib will send MTU BPP packets when the total bitrate changes.
                            * Note: Enabling this may increase the mux latency by up to 1 frame time
                            */
   BMUXlib_TS_InterleaveMode eInterleaveMode; /* Specifies A/V interleave timing mode */
} BMUXlib_TS_StartSettings;

void
BMUXlib_TS_GetDefaultStartSettings(
         BMUXlib_TS_StartSettings *pstStartSettings
         );

/* BMUXlib_TS_Start - Configures the mux HW */
BERR_Code
BMUXlib_TS_Start(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_StartSettings *pstStartSettings
         );

typedef struct BMUXlib_TS_FinishSettings
{
      uint32_t uiSignature; /* [DO NOT MODIFY] Populated by BMUXlib_TS_GetDefaultFinishSettings() */

      BMUXlib_FinishMode eFinishMode;
} BMUXlib_TS_FinishSettings;

void
BMUXlib_TS_GetDefaultFinishSettings(
         BMUXlib_TS_FinishSettings *pstFinishSettings
         );

/* BMUXlib_TS_Finish -
 * Schedules the muxer to finish
 * DoMux() returns the following state:
 *    - eFinishingInput: still consuming encoder output
 *    - eFinishingOutput: still muxing data.  encoders can be closed.
 *    - eFinished: muxing complete. muxer can be stopped.
 */
BERR_Code
BMUXlib_TS_Finish(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_FinishSettings *pstFinishSettings
         );

/* BMUXlib_TS_Stop -
 * Flushes/resets the mux HW immediately.
 * For a clean stop, this should only be called after the
 * "finished" event occurs after calling BMUXlib_TS_Finish()
 * This function may need to be called without BMUXlib_TS_Finish()
 * in cases where an abrupt stop is needed (e.g. USB Hard Drive is
 * unplugged).
 */
BERR_Code
BMUXlib_TS_Stop(
         BMUXlib_TS_Handle hMuxTS
         );

/**********/
/* Memory */
/**********/
typedef struct BMUXlib_TS_MuxConfig
{
   BMUXlib_TS_StartSettings stMuxStartSettings;
   BMUXlib_TS_MuxSettings stMuxSettings;
} BMUXlib_TS_MuxConfig;

/* BMUXlib_TS_GetMemoryConfig - Returns the memory required for the specified mux configuration.
 * The following fields are relevant for the memory calculations:
 *
 * BMUXlib_TS_MuxConfig
 *    BMUXlib_TS_StartSettings
 *       uiServicePeriod            - larger values require more memory
 *       uiServiceLatencyTolerance  - larger values require more memory
 *       bNonRealTimeMode           - "true" value requires more memory
 *       uiNumValidVideoPIDs        - larger values require more memory
 *       uiNumValidAudioPIDs        - larger values require more memory
 *       uiNumValidUserdataPIDs     - larger values require more memory
 *       stPCRData.uiInterval       - smaller values require more memory
 *       uiA2PDelay                 - larger values require more memory
 *       bSupportTTS                - "true" value requires more memory
 *    BMUXlib_TS_MuxSettings
 *       uiSystemDataBitRate        - larger values require more memory
 */
void
BMUXlib_TS_GetMemoryConfig(
         const BMUXlib_TS_MuxConfig *pstMuxConfig,
         BMUXlib_TS_MemoryConfig *pstMemoryConfig
         );

/***************/
/* System Data */
/***************/
/* This API will be used for both periodic and non-periodic data.
 * For Periodic data (PAT/PMT), the app needs to add/queue packets on a periodic basis
 *
 * System Data is scheduled and interleaved with the internally generated PCR packets.
 * If system data packets spans across a PCR packet boundary, the muxer will split the
 * system data packets to ensure PCR packets have highest priority.
 */
typedef struct BMUXlib_TS_SystemData
{
      unsigned uiTimestampDelta; /* (in milliseconds) Time this packet should start to be sent out
                                  * relative to the start of the previous system data buffer.
                                  * "0" indicates to send the packet out ASAP.
                                  */

      size_t uiSize; /* in bytes. Must be multiple of TS Packet size (188 bytes) */
      BMMA_Block_Handle hBlock; /* block containing the data to be muxed.
                                 * Formatted as TS Packets.
                                 * CC counter must be set by the application.
                                 * User responsible for ensuring PIDs do not conflict
                                 * with other System Data or Userdata PIDs
                                 */
      size_t uiBlockOffset; /* Offset within the block to the startr of the data to be muxed */
} BMUXlib_TS_SystemData;

/* BMUXlib_TS_AddSystemDataBuffers -
 * Adds system data buffers(s) to the stream.  The buffers that are queued must remain intact until the muxing has completed.
 */
BERR_Code
BMUXlib_TS_AddSystemDataBuffers(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         );

/* BMUXlib_TS_GetCompletedSystemDataBuffers -
 * Returns the count of transport descriptors completed
 * since the *previous* call to BMUXlib_TS_GetCompletedSystemDataBuffers.
 *
 * Completed buffers can be reused by the application immediately.
 */
BERR_Code
BMUXlib_TS_GetCompletedSystemDataBuffers(
         BMUXlib_TS_Handle hMuxTS,
         size_t *puiCompletedCount /* Returns count of system data buffers fully muxed */
         );

/******************/
/* Suspend/Resume */
/******************/
/* TODO: Suspend/Resume API */

/**********/
/* Status */
/**********/

typedef struct BMUXlib_TS_Input_Status
{
   uint64_t uiCurrentTimestamp; /* Most recent timestamp completed (in 90 Khz) */
   uint32_t uiCurrentESCR; /* Most recent ESCR completed (in 27 Mhz) */
} BMUXlib_TS_Input_Status;

typedef struct BMUXlib_TS_Status
{
   BMUXlib_TS_Input_Status stVideo[BMUXLIB_TS_MAX_VIDEO_PIDS];
   BMUXlib_TS_Input_Status stAudio[BMUXLIB_TS_MAX_AUDIO_PIDS];
   BMUXlib_TS_Input_Status stSystem;
   BMUXlib_TS_Input_Status stUserData[BMUXLIB_TS_MAX_USERDATA_PIDS];

   struct
   {
      unsigned uiVideo; /* Average video bitrate (in kbps) */
      unsigned uiAudio; /* Average audio bitrate (in kbps) */
      unsigned uiSystemData; /* Average system data bitrate (in kbps) */
      unsigned uiUserData; /* Average userdata bitrate (in kbps) */
   } stAverageBitrate;
   unsigned uiEfficiency; /* Indicates efficiency of the output TS (e.g. 100 = no TS overhead, 95 = 5% TS overhead, etc.) */
   unsigned uiTotalBytes; /* Total bytes generated by the TS MUXlib */
} BMUXlib_TS_Status;

/* BMUXlib_TS_GetStatus - returns the TS MUX specific status as of
 * the most recent call to BMUXlib_TS_DoMux()
 */
void
BMUXlib_TS_GetStatus(
   BMUXlib_TS_Handle hMuxTS,
   BMUXlib_TS_Status *pstStatus
   );

/***********/
/* Execute */
/***********/
/* BMUXlib_TS_DoMux - will get the buffer descriptors for Audio and
 * Video, generate PES headers, PCR packets, etc, as needed and feed
 * LLDs to XPT PB.  It will also update the A/V descriptors muxed.
 */
BERR_Code
BMUXlib_TS_DoMux(
   BMUXlib_TS_Handle hMuxTS,
   BMUXlib_DoMux_Status *pstStatus
   );



#ifdef __cplusplus
}
#endif

#endif /* BMUXLIB_TS_H_ */
