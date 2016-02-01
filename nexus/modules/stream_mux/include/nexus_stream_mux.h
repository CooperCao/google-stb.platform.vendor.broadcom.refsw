/***************************************************************************
 *     (c)2010-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef NEXUS_STREAM_MUX_H__
#define NEXUS_STREAM_MUX_H__

#include "nexus_video_encoder_output.h"
#include "nexus_playpump.h"
#include "nexus_stc_channel.h"
#include "nexus_audio_mux_output.h"
#include "nexus_message.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Muxing TS streams using XPT playback HW
**/

/**
Summary:
**/
typedef struct NEXUS_StreamMux *NEXUS_StreamMuxHandle;

typedef struct NEXUS_StreamMuxMemoryConfiguration {
    size_t  systemBufferSize; /* only accesses by the host */
    size_t  sharedBufferSize; /* accessed by both the host and devices(s) */
} NEXUS_StreamMuxMemoryConfiguration;

/**
Summary:
**/
typedef struct NEXUS_StreamMuxCreateSettings
{
    NEXUS_CallbackDesc finished; /* NEXUS_StreamMux_Finish has completed. NEXUS_StreamMux_Stop can be called for a clean finish. */
    NEXUS_StreamMuxMemoryConfiguration memoryConfiguration;
    NEXUS_HeapHandle    systemHeap; /* heap that is used to allocated memory for the  host only accessible buffer, if NULL, then OS allocator would be used */
    NEXUS_HeapHandle    sharedHeap; /* heap that is used to allocated memory for the host and devies(s) buffer, if NULL, then default heap would be used */
} NEXUS_StreamMuxCreateSettings;

/**
Summary: NEXUS_StreamMuxConfiguration contains a subset of the fields in
          NEXUS_StreamMuxStartSettings.  Please look at NEXUS_StreamMuxStartSettings
          for a description of the fields.
**/
typedef struct NEXUS_StreamMuxConfiguration {
    unsigned servicePeriod;     /* larger values require more memory */
    unsigned latencyTolerance;  /* larger values require more memory */
    bool nonRealTime;           /* "true" value requires more memory */
    unsigned videoPids;         /* larger values require more memory */
    unsigned audioPids;         /* larger values require more memory */
    unsigned userDataPids;      /* larger values require more memory */
    unsigned pcrPidinterval;    /* smaller values require more memory */
    unsigned systemDataBitRate; /* larger values require more memory */
    unsigned muxDelay;          /* larger values require more memory */
    bool supportTts;            /* "true" value requires more memory */
} NEXUS_StreamMuxConfiguration;


/**
Summary:
**/
void NEXUS_StreamMux_GetDefaultCreateSettings(
    NEXUS_StreamMuxCreateSettings *pSettings   /* [out] */
    );

/**
Summary:
**/
NEXUS_StreamMuxHandle NEXUS_StreamMux_Create( /* attr{destructor=NEXUS_StreamMux_Destroy}  */
    const NEXUS_StreamMuxCreateSettings *pSettings     /* attr{null_allowed=y} Pass NULL for default settings */
    );

/**
Summary:
**/
void NEXUS_StreamMux_Destroy(
    NEXUS_StreamMuxHandle handle
    );


#define NEXUS_MAX_MUX_PIDS 16
typedef struct NEXUS_StreamMuxVideoPid {
    uint16_t pid; /* PID in which to insert the encoded video data */
    uint8_t pesId; /* pes stream ID */
    int pidChannelIndex; /* pidChannelIndex used in NEXUS_Playpump_OpenPidChannel */
    NEXUS_PlaypumpHandle playpump;
    NEXUS_VideoEncoderHandle encoder; /* if NULL, then not enabled */
} NEXUS_StreamMuxVideoPid;

typedef struct NEXUS_StreamMuxAudioPid {
    uint16_t pid; /* PID in which to insert the encoded audio data */
    uint8_t pesId; /* pes stream ID */
    int pidChannelIndex; /* pidChannelIndex used in NEXUS_Playpump_OpenPidChannel */
    NEXUS_PlaypumpHandle playpump;
    NEXUS_AudioMuxOutputHandle muxOutput; /* if NULL, then not enabled */
} NEXUS_StreamMuxAudioPid;

typedef struct NEXUS_StreamMuxPcrPid {
    unsigned pid; /* PID in which it to insert the PCR */
    int pidChannelIndex; /* pidChannelIndex used in NEXUS_Playpump_OpenPidChannel */
    NEXUS_PlaypumpHandle playpump;
    unsigned interval; /* periodic milliseconds */
} NEXUS_StreamMuxPcrPid;

typedef struct NEXUS_StreamMuxUserDataPid {
    NEXUS_MessageHandle message; /* The userdata is expected to arrive as PES packets encapsulated in TS packets. Application is responsible for calling NEXUS_Message_Start prior to calling NEXUS_StreamMux_Start and call NEXUS_Message_Stop after mux session was completed */
} NEXUS_StreamMuxUserDataPid;

/**
Summary:
**/
typedef struct NEXUS_StreamMuxStartSettings
{
    NEXUS_TransportType transportType;
    NEXUS_StcChannelHandle stcChannel;

    NEXUS_StreamMuxVideoPid video[NEXUS_MAX_MUX_PIDS];
    NEXUS_StreamMuxAudioPid audio[NEXUS_MAX_MUX_PIDS];
    NEXUS_StreamMuxPcrPid pcr;
    NEXUS_StreamMuxUserDataPid userdata[NEXUS_MAX_MUX_PIDS];

    unsigned latencyTolerance;   /* (in milliseconds) Indicates the maximum latency
                                 * the muxer is expected to handle without causing
                                 * underflow.
                                 *
                                 * This value directly affects the overall mux latency
                                 * and the encoder's CDB buffer size
                                 */

    bool nonRealTime;           /* Normal operation for muxing in real time, if this is set to 'true' then muxing done in non-realtime (AFAP) */

    int nonRealTimeRate;        /* Rate in units of NEXUS_NORMAL_PLAY_SPEED for non realtime muxing, it's the upper bound to the rate of muxing, due to the underlying limitations rate could be only be power of 2, for example 1*NEXUS_NORMAL_PLAY_SPEED , 2*NEXUS_NORMAL_PLAY_SPEED, 4*NEXUS_NORMAL_PLAY_SPEED */

    unsigned servicePeriod;     /* (in milliseconds) Indicates the lowest frequency
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
    unsigned muxDelay;          /* In 27MHz clock ticks.
                                 *  Desired "Arrival-to-Presentation" delay.  0 indicates use default.
                                 *  It is expected that muxDelay >= NEXUS_VideoEncoderSettings.encoderDelay
                                 * (this value is used to properly allocate memory for internal usage)
                                 */
    bool supportTts;            /* Enables TTS support (MTU BPP insertion) to facilitate HW to generate
                                 * 4-byte timestamp in output TS */
} NEXUS_StreamMuxStartSettings;

typedef struct NEXUS_StreamMuxOutput
{
    NEXUS_PidChannelHandle video[NEXUS_MAX_MUX_PIDS];
    NEXUS_PidChannelHandle audio[NEXUS_MAX_MUX_PIDS];
    NEXUS_PidChannelHandle pcr;
} NEXUS_StreamMuxOutput;

/**
Summary:
**/
typedef struct NEXUS_StreamMuxSettings
{
    struct {
        bool video[NEXUS_MAX_MUX_PIDS];
        bool audio[NEXUS_MAX_MUX_PIDS];
    } enable;  /* dynamically control specified input from muxing into output.
                * Any data from the specified input is discarded instead of being muxed into the output.
                * If the input is video, the drop/resume will occur on GOP boundaries.
                */
} NEXUS_StreamMuxSettings;

/**
Summary:
**/
void NEXUS_StreamMux_GetDefaultStartSettings(
    NEXUS_StreamMuxStartSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_StreamMux_Start(
    NEXUS_StreamMuxHandle handle,
    const NEXUS_StreamMuxStartSettings *pSettings,
    NEXUS_StreamMuxOutput *pMuxOutput
    );

/**
Summary:
**/
void NEXUS_StreamMux_GetSettings(
    NEXUS_StreamMuxHandle handle,
    NEXUS_StreamMuxSettings *pSettings
    );

/**
Summary:
**/
NEXUS_Error NEXUS_StreamMux_SetSettings(
    NEXUS_StreamMuxHandle handle,
    const NEXUS_StreamMuxSettings *pSettings
    );

/**
Summary:
Finalize all data to Playpump for an impending stop

Description:
For a clean shutdown, call NEXUS_StreamMux_Finish, wait for finished callback, then call NEXUS_StreamMux_Stop.
**/
void NEXUS_StreamMux_Finish(
    NEXUS_StreamMuxHandle handle
    );

/**
Summary:
Stop muxing data immediately.
**/
void NEXUS_StreamMux_Stop(
    NEXUS_StreamMuxHandle handle
    );

/**
Summary:
System data (e.g. PSI) to be sent using NEXUS_StreamMux_AddSystemDataBuffers
**/
typedef struct NEXUS_StreamMuxSystemData
{
    unsigned timestampDelta; /* (in milliseconds) Time this packet should start to be sent out
                              relative to the start of the previous system data buffer.
                              "0" indicates to send the packet out ASAP. */
    size_t size; /* size of pData in bytes. Must be multiple of TS Packet size (188 bytes) */
    const void *pData; /* attr{memory=cached} address of data to be muxed. Must be allocated using NEXUS_Memory_Allocation.
                          memory pointed to by pData must remain intact until NEXUS_StreamMux_GetCompletedSystemDataBuffers indicates
                          that is is completed. */
} NEXUS_StreamMuxSystemData;

/**
Summary:
**/
void NEXUS_StreamMux_GetDefaultSystemData(
    NEXUS_StreamMuxSystemData *pSystemDataBuffer /* [out] */
    );

/**
Summary:
Add system data to the stream with a specified delay
**/
NEXUS_Error NEXUS_StreamMux_AddSystemDataBuffer(
    NEXUS_StreamMuxHandle handle,
    const NEXUS_StreamMuxSystemData *pSystemDataBuffer /* system data buffer */
    );

/**
Summary:
Learn what system data has been sent into the outgoing stream

Description:
Data is completed in FIFO order. Caller must keep track of submitted and completed entries.
**/
void NEXUS_StreamMux_GetCompletedSystemDataBuffers(
    NEXUS_StreamMuxHandle handle,
    size_t *pCompletedCount /* [out] */
    );

/**
Summary:
**/
typedef struct NEXUS_StreamMuxStatus
{
    unsigned duration; /* file duration (in milliseconds) completed at current time */
} NEXUS_StreamMuxStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_StreamMux_GetStatus(
    NEXUS_StreamMuxHandle handle,
    NEXUS_StreamMuxStatus *pStatus /* [out] */
    );

/**
Summary:
**/
void NEXUS_StreamMux_GetDefaultConfiguration(
    NEXUS_StreamMuxConfiguration *pConfiguration /* [out] */
    );

/**
Summary:
Return memory configuration for given mux configuration.

Description:
Different mux configuration may require different amount of memory. This function would return memory budged required for given configuration.
This configuration would need then to be passed into the NEXUS_StreamMux_Create. If application wants to support multiple different configurations
with the same instance of the NEXUS_StreamMux, it would need to call this function for each configuration and combine results.
**/
void NEXUS_StreamMux_GetMemoryConfiguration(
    const NEXUS_StreamMuxConfiguration *pConfiguration,
    NEXUS_StreamMuxMemoryConfiguration *pMemoryConfiguration
    );


#ifdef __cplusplus
}
#endif


#endif /* NEXUS_STREAM_MUX_H__ */



