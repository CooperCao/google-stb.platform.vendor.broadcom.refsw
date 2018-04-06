/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 **************************************************************************/
#ifndef NEXUS_PLAYPUMP_H__
#define NEXUS_PLAYPUMP_H__

#include "nexus_types.h"
#include "nexus_pid_channel.h"
#include "nexus_dma_types.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for the Playpump Interface.

Description:
The Playpump Interface provides data feeding for PVR playback functionality.
Each Interface corresponds to one hardware playback channel.

See nexus/examples/playpump.c for an example application.

See NEXUS_PlaybackHandle for a high-level Playback Interface.
See NEXUS_RecpumpHandle for the corresponding low-level PVR record Interface.
See NEXUS_VideoDecoderHandle and NEXUS_AudioDecoderHandle for decoder-based PVR and trick mode options.
**/
typedef struct NEXUS_Playpump *NEXUS_PlaypumpHandle;

/**
Summary:
Settings used in NEXUS_Playpump_Open
**/
typedef struct NEXUS_PlaypumpOpenSettings
{
    size_t fifoSize;            /* size of playpump's fifo in bytes.
                                   The minimum size depends on the maximum bitrate of your streams and the system latency for filling the buffers.
                                   The system latency depends on i/o performance and OS scheduling.
                                   If the size chosen is too small, you will underflow the decoders. */
    unsigned alignment;         /* You may want to specify the alignment of the fifo allocation to prevent any wasted buffer because of memory alignment requirements.
                                   See NEXUS_Playppump_ReadComplete and its skip parameter for more information about alignment requirements. */
    NEXUS_MemoryBlockHandle memory; /* optional external allocation of fifo */
    unsigned memoryOffset;       /* offset into 'memory' block. 'fifoSize' should be counted from the start of the memoryOffset, not the memory block size. */

    unsigned numDescriptors;     /* More descriptors are needed to perform certain trick modes. See NEXUS_Playpump_GetBuffer for details. */
    bool streamMuxCompatible;    /* If set to true,  this playpump could only be used with the StreamMux module */
    bool dataNotCpuAccessible;   /* If true, data can not be accessed by the CPU.  Cache flushing will be the caller's responsibility and
                                    internal logic will not attempt to access the data */

    NEXUS_HeapHandle heap;       /* optional heap for fifo allocation */
    NEXUS_HeapHandle boundsHeap; /* optional heap to bounds check all scatter-gather descriptors */
    bool descriptorPacingEnabled;      /* Support descriptor-based pacing. See NEXUS_Playpump_WriteCompleteWithSettings() */
} NEXUS_PlaypumpOpenSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_Playpump_GetDefaultOpenSettings(
    NEXUS_PlaypumpOpenSettings *pSettings /* [out] */
    );

/**
Summary:
Open a new Playpump interface.
**/
NEXUS_PlaypumpHandle NEXUS_Playpump_Open( /* attr{destructor=NEXUS_Playpump_Close} */
    unsigned index, /* index of hardware playback channel. NEXUS_ANY_ID is supported. */
    const NEXUS_PlaypumpOpenSettings *pSettings /* attr{null_allowed=y} may be NULL for default settings */
    );

/**
Summary:
Close the Playpump interface.
**/
void NEXUS_Playpump_Close(
    NEXUS_PlaypumpHandle playpump
    );

/**
Summary:
Determines method of feeding data into playpump.
**/
typedef enum NEXUS_PlaypumpMode
{
    NEXUS_PlaypumpMode_eFifo,         /* Playpump allocates a block of memory which serves as the playback fifo.
                                         The user must copy data into this buffer with the CPU or DMA. */
    NEXUS_PlaypumpMode_eSegment,      /* This is like Fifo, except the user must prepend NEXUS_PlaypumpSegment headers to every data block. */
    NEXUS_PlaypumpMode_eMax
} NEXUS_PlaypumpMode;

/**
Summary:
Base rate for normal play used in NEXUS_PlaypumpSettings.playRate.
**/
#define NEXUS_NORMAL_PLAY_SPEED 1000

/**
Summary:
Settings for playpump
**/
typedef struct NEXUS_PlaypumpSettings
{
    NEXUS_TransportType transportType;          /* Type of data stream going into playpump. Note that if the data must be packetized,
                                                   you can get the resulting NEXUS_TransportType from NEXUS_PidChannelStatus. */

    NEXUS_TransportType originalTransportType;  /* If the user packetizes intput to playpump, this describes the input. If there is no
                                                   packetization, this is member should be set to NEXUS_TransportType_eUnknown. */

    bool continuityCountEnabled;                /* If true, transport will check for correct continuity counters and discard bad packets.
                                                   This must be set to false for trick modes. Default is false. */

    /* Type of original transport stream */
    NEXUS_PlaypumpMode mode;                    /* Method of pushing data into playpump. This setting determines the format of data the app
                                                   will feed to playpump. */
    NEXUS_CallbackDesc dataCallback;            /* Callback when space becomes available. User should call NEXUS_Playpump_GetBuffer.
                                                   You will not receive another callback until NEXUS_Playpump_GetBuffer is called. */
    NEXUS_CallbackDesc errorCallback;           /* Callback called when error was detected in ptocessing of the stream data */
    NEXUS_CallbackDesc ccError;                 /* Continuity Count Error - raised when continuity counter of next packet does not have the next counter value.
                                                   continuityCountEnabled must be set to true. See NEXUS_PidChannelStatus.continuityCountErrors
                                                   to get a count of errors per pid. */
    NEXUS_CallbackDesc teiError;                /* Transport Error Indicator - error status from a demodulator */

    int playRate;                               /* Rate in units of NEXUS_NORMAL_PLAY_SPEED. It's used for certain type of streams to control the
                                                   direction of presentation time. */

    NEXUS_DmaHandle securityDma;                /* deprecated */
    NEXUS_DmaDataFormat securityDmaDataFormat;  /* deprecated */
    NEXUS_KeySlotHandle securityContext;        /* used for DRM */

    unsigned maxDataRate;                       /* Maximum data rate for the playback parser band in units of bits per second. Default is typically
                                                   108000000 (i.e. 108 Mbps). If you increase this, you need to analyze total transport bandwidth and
                                                   overall system bandwidth. */
    bool blindSync;                             /* Force transport to not look for sync bytes. */

    /* all pass settings */
    bool allPass;                               /* If true, NEXUS_Playpump_OpenPidChannel's pid param is ignored and the resulting pid channel can be
                                                   used for all-pass record. Also set acceptNullPackets to true if you want to capture the entire stream.
                                                   When opening the allPass pid channel, set NEXUS_PlaypumpOpenPidChannelSettings.pidSettings.pidChannelIndex
                                                   to the HwPidChannel obtained from NEXUS_Playpump_GetAllPassPidChannelIndex(). */
    bool acceptNullPackets;                     /* If true and allPass is true, NULL packets are not discarded. */

    bool dataNotCpuAccessible;                  /* deprecated, please use NEXUS_PlaypumpOpenSettings.dataNotCpuAccessible instead */

    struct {
        NEXUS_TransportTimestampType type;      /* The type of timestamping in this stream. */
        uint16_t pcrPacingPid;                  /* Some chips can pace playback using a PCR pid. Set timestamp.pcrPacingPid to a non-zero value and set
                                                   timestamp.pacing = true. */

        bool pacing;                            /* If true, pace this stream using embedded timestamp information or using pcr-based pacing.
                                                   For timestamp-based pacing, set timestampType as well.
                                                   For pcr-based pacing, set pcrPacingPid as well. */
        unsigned pacingMaxError;                /* Set the maximum allowable timestamp error (TS_RANGE_ERROR) used during playback pacing. */
        NEXUS_Timebase timebase;                /* Timebase used for timestamp. Use NEXUS_Timebase_eInvalid for freerun. */
        bool parityCheckDisable;                /* DEPRECATED: timestamp.type indirectly specifies the parity check */
        bool pacingOffsetAdjustDisable;         /* Set true to adjust the pacing timestamps with the reference offset */
        bool resetPacing;                       /* Set true if you want the pacing start bit reset when a TS_RANGE_ERROR interrupt occurs */
        bool forceRestamping;                   /* Set true if you want to restamp the incoming timestamps or if you are recording and want to add timestamps */
    } timestamp;

} NEXUS_PlaypumpSettings;

/**
Summary:
Settings used in NEXUS_Playpump_OpenPidChannel
**/
typedef struct NEXUS_PlaypumpOpenPidChannelSettings
{
    NEXUS_PidChannelSettings pidSettings;
    NEXUS_PidType pidType;
    bool allowTimestampReordering; /* For some media formats it's required to reorder timestamps prior to sending data to the decoders,
                                      this flag controls whether playpump would reourder timestamps */
    union
    {
        struct
        {
            NEXUS_AudioCodec codec; /* Audio codec information is required for VOB substream id remapping. */
        } audio;
    } pidTypeSettings;
} NEXUS_PlaypumpOpenPidChannelSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_Playpump_GetDefaultOpenPidChannelSettings(
    NEXUS_PlaypumpOpenPidChannelSettings *pSettings /* [out] */
    );

/*
Summary:
Open a playpump NEXUS_PidChannelHandle.

Description:
This differs from a pid channel opened by NEXUS_PidChannel_Open because it
allows Playpump to get additional information about the pid.
NEXUS_Playpump_OpenPidChannel can be called before or after Start and Stop.
*/
NEXUS_PidChannelHandle NEXUS_Playpump_OpenPidChannel(
    NEXUS_PlaypumpHandle playpump,
    unsigned pid,                   /* Substream selector.
                                       For NEXUS_TransportType_eTs, the lower 16 bits are the PID and the bits 23..16 are the Sub-Stream ID.
                                       For NEXUS_TransportType_eMpeg2Pes, this is the Stream ID.
                                       For NEXUS_TransportType_eVob, the lower 8 bits are the Stream ID and the bits 15..8 are the Sub-Stream ID.
                                       For NEXUS_TransportType_eEs, this is ignored. */
    const NEXUS_PlaypumpOpenPidChannelSettings *pSettings /* attr{null_allowed=y} may be NULL for default settings */
    );

/*
Summary:
This function closes a pid channel opened by NEXUS_Playpump_OpenPidChannel
*/
NEXUS_Error NEXUS_Playpump_ClosePidChannel(
    NEXUS_PlaypumpHandle playpump,
    NEXUS_PidChannelHandle pidChannel
    );

/*
Summary:
Close all pid channels opened by NEXUS_Playpump_OpenPidChannel
*/
void NEXUS_Playpump_CloseAllPidChannels(
    NEXUS_PlaypumpHandle playpump
    );

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_Playpump_GetDefaultSettings(
    NEXUS_PlaypumpSettings *pSettings /* [out] */
    );

/**
Summary:
Get current settings
**/
void NEXUS_Playpump_GetSettings(
    NEXUS_PlaypumpHandle playpump,
    NEXUS_PlaypumpSettings *pSettings /* [out] */
    );

/**
Summary:
Set new settings

Description:
If you use NEXUS_Playback, be aware that it will call this function with NEXUS_PlaybackSettings.playpumpSettings
any will overwrite any application use of this function.
**/
NEXUS_Error NEXUS_Playpump_SetSettings(
    NEXUS_PlaypumpHandle playpump,
    const NEXUS_PlaypumpSettings *pSettings
    );

/**
Summary:
Start playback

Description:
After calling this function, the app is responsible to call NEXUS_Playpump_GetBuffer,
feed in data, then call NEXUS_Playpump_WriteComplete.
**/
NEXUS_Error NEXUS_Playpump_Start(
    NEXUS_PlaypumpHandle playpump
    );

/**
Summary:
Stop playback
**/
void NEXUS_Playpump_Stop(
    NEXUS_PlaypumpHandle playpump
    );

/*
Summary:
Throw away all data currently in playback fifo.

Description:
This does not flush any downstream consumer like a decoders.
See NEXUS_VideoDecoder_Flush and NEXUS_AudioDecoder_Flush.
*/
NEXUS_Error NEXUS_Playpump_Flush(
    NEXUS_PlaypumpHandle playpump
    );

/*
Summary:
Get a pointer and size for the next location in the buffer which can accept playpump data.

Description:
NEXUS_Playpump_GetBuffer is non-destructive. You can safely call it multiple times.

The buffer and size returned by NEXUS_Playpump_GetBuffer are valid until you call NEXUS_Playpump_Flush or NEXUS_Playpump_WriteComplete.

The size returned by NEXUS_Playpump_GetBuffer can be limited by two things: space in the ring buffer or free descriptors
used to send data to the transport block. If you are performing trick modes with lots of small reads,
you may find that NEXUS_Playpump_GetBuffer says there is no space available even though
NEXUS_PlaypumpStatus reports free space. The solution is to increase the number of descriptors
in NEXUS_PlaypumpOpenSettings.
*/
NEXUS_Error NEXUS_Playpump_GetBuffer(
    NEXUS_PlaypumpHandle playpump,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for playback data */
    size_t *pSize /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    );

#define  NEXUS_Playpump_WriteComplete(playpump, skip, amountUsed) NEXUS_Playpump_WriteCompleteWithSettings(playpump, skip, amountUsed, NULL)

/**
Summary:
Notify Playpump of how much data was added into the buffer, including optional per-commit parameters.
**/

typedef struct NEXUS_PlaypumpWriteCompleteSettings
{
    /* Timestamps are in the same format as the type given in the playpump settings struct. */
    struct {
        uint32_t timestamp;
        uint32_t pkt2pktDelta;
    } descriptorPacing;
} NEXUS_PlaypumpWriteCompleteSettings;

void NEXUS_Playpump_GetDefaultWriteCompleteSettings(
    NEXUS_PlaypumpWriteCompleteSettings *Settings
    );

/**
Summary:
Notify Playpump of how much data was added into the buffer.

Description:
You can only call NEXUS_Playpump_WriteComplete once after a NEXUS_Playpump_GetBuffer call.
After calling it, you must call NEXUS_Playpump_GetBuffer before adding more data.

The skip parameter allows an application to copy data into the playpump buffer with
whatever alignment it desired. This is needed for "direct I/O" from disk drives which requires
all memory access to be page aligned (e.g. 4K).
**/
NEXUS_Error NEXUS_Playpump_WriteCompleteWithSettings(
    NEXUS_PlaypumpHandle p,
    unsigned skip,
    unsigned amount_used,
    const NEXUS_PlaypumpWriteCompleteSettings *pSettings /* attr{null_allowed=y} */
    );

/* backward compatibility */
#define NEXUS_Playpump_ReadComplete NEXUS_Playpump_WriteComplete

/**
Summary:
Playpump scatter gather Descriptor
**/
typedef struct NEXUS_PlaypumpScatterGatherDescriptor
{
    void *addr;    /* attr{memory=cached} address of memory region */
    unsigned length; /* size of the memory region */
    NEXUS_PlaypumpWriteCompleteSettings descriptorSettings; /* Used only when descriptorPacingEnabled is true */
} NEXUS_PlaypumpScatterGatherDescriptor;

/**
Summary:
Submit scatter gather descriptor to playpump
**/
NEXUS_Error NEXUS_Playpump_SubmitScatterGatherDescriptor(
    NEXUS_PlaypumpHandle playpump,
    const NEXUS_PlaypumpScatterGatherDescriptor *pDesc, /* attr{nelem=numDescriptors;nelem_out=pNumConsumed;reserved=4} */
    size_t numDescriptors,
    size_t *pNumConsumed /* [out] */
    );

/**
Summary:
Status returned by NEXUS_Playpump_GetStatus
**/
typedef struct NEXUS_PlaypumpStatus
{
    bool started;               /* Has NEXUS_Playpump_Start been called? */
    size_t fifoDepth;           /* Depth in bytes of the playback buffer */
    unsigned softwareFifoDelay; /* Bytes which are still counted in fifoDepth but have actually been processed by hardware.
                                   There is a delay before software fifoDepth can be updated.
                                   fifoDepth minus softwareFifoDelay is the number of bytes still to be processed by hardware. */
    size_t fifoSize;            /* Size in bytes of the playback buffer */
    size_t descFifoDepth;       /* Number of active (i.e. busy) descriptors */
    size_t descFifoSize;        /* Number of allocated descriptors */
    void *bufferBase;           /* attr{memory=cached} Pointer to the base of the playback buffer.
                                   This can be used for calculating your exact position
                                   in the buffer for alignment considerations. */
    uint64_t bytesPlayed;       /* Total number of bytes played since starting */
    unsigned index;             /* Index of this playpump */

    unsigned pacingTsRangeError; /* Number of TS_RANGE_ERROR's if pacing is activated. */
    unsigned syncErrors;         /* count of sync errors detected in the stream, updated only for stream processed in SW */
    unsigned resyncEvents;       /* count of resync handled in the stream, updated only for stream processed in SW */
    unsigned streamErrors;       /* count of other errors found in the stream, updated only for stream processed in SW */
    NEXUS_PtsType mediaPtsType;    /* Type of the current PTS, updated only for stream processed in SW */
    uint32_t mediaPts;    /* if playpump does inline media streeam conversion (i.e. AVI to PES), then this is the last known PTS
                             (in units of 45KHz) for the first converted pid (track) */
    unsigned teiErrors;     /* count of TS packets with the Transport Error Indicator bit set. */
} NEXUS_PlaypumpStatus;

/**
Summary:
Get current status of playpump
**/
NEXUS_Error NEXUS_Playpump_GetStatus(
    NEXUS_PlaypumpHandle playpump,
    NEXUS_PlaypumpStatus *pStatus /* [out] */
    );

/**
Summary:
Signature used in NEXUS_PlaypumpSegment.signature when in NEXUS_PlaypumpMode_eSegment mode.
**/
#define NEXUS_PLAYPUMP_SEGMENT_SIGNATURE ((((uint32_t)'B')<<24) | (((uint32_t)'P')<<16 ) | (((uint32_t)'L')<<8) | 0)

/**
Summary:
Segment header used in NEXUS_PlaypumpMode_eSegment mode.

Description:
When feeding data into Playpump, the user must prepend NEXUS_PlaypumpSegment before every data block.
**/
typedef struct NEXUS_PlaypumpSegment
{
    uint32_t length;    /* Size of the segment. This includes the size of the NEXUS_PlaypumpSegment header and all data following the header. */
    uint64_t offset;    /* 64 bit offset in the source file for this segment */
    uint32_t timestamp; /* timestamp for this segment */
    struct {
        uint16_t stream_id;
        int16_t  timestamp_delta;
    } timestamp_delta[9];
    uint32_t signature; /* This field must be set to BPLAYPUMP_SEGMENT_SIGNATURE. Nexus uses this to validate the segmented data stream. */
} NEXUS_PlaypumpSegment;

/**
Summary:
Pause or unpause the transport playback hardware

Description:
This does not pause decoders; it pauses the transport playback hardware state machine (aka "micro pause").
Be aware this could result in loss of data if the application unpauses before a consumer
is attached. Nexus normally uses this same hardware pause to ensure this does not occur.
**/
NEXUS_Error NEXUS_Playpump_SetPause(
    NEXUS_PlaypumpHandle playpump,
    bool paused
    );

/**
Summary:
Suspend or resume pacing

Description:
NEXUS_PlaypumpSettings.timebase.pacing must be true for this to be used.
**/
NEXUS_Error NEXUS_Playpump_SuspendPacing(
    NEXUS_PlaypumpHandle playpump,
    bool suspended
    );

/**
Summary:
Determine the PID channel that will be used in allPass mode. 
 
Description: 
Each playpump will use a different PID channel when operating in
allPass mode. This API provides the channel number for the given
playpump. 
**/
NEXUS_Error NEXUS_Playpump_GetAllPassPidChannelIndex(
    NEXUS_PlaypumpHandle playpump, 
    unsigned *pHwPidChannel
    );

/**
Summary:
Learn if playpump supports the transport type

Description:
Some transport types, like eMp4 and eMkv, require higher level support. See NEXUS_Playback_IsTransportTypeSupported.
**/
void NEXUS_Playpump_IsTransportTypeSupported(
    NEXUS_PlaypumpHandle playpump,
    NEXUS_TransportType transportType,
    bool *pIsSupported
    );

NEXUS_Error NEXUS_Playpump_GetMpodLtsid(
    NEXUS_PlaypumpHandle playpump,
    unsigned *pLtsid /* [out] the LTSID associated with this playpump */
    );

#ifdef __cplusplus
}
#endif

#endif
