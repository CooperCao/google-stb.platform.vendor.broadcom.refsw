/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#ifndef NEXUS_AUDIO_MUX_OUTPUT_H__
#define NEXUS_AUDIO_MUX_OUTPUT_H__

#include "nexus_types.h"
#include "nexus_audio_types.h"
#include "nexus_audio_output.h"
#include "nexus_stc_channel.h"
#include "nexus_memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Handle for audio mux output
***************************************************************************/
typedef struct NEXUS_AudioMuxOutput *NEXUS_AudioMuxOutputHandle;

/***************************************************************************
Summary:
Mux Output Creation-time settings
***************************************************************************/
typedef struct NEXUS_AudioMuxOutputCreateSettings
{
    struct {
        unsigned fifoSize; /* RAVE buffer */
        NEXUS_HeapHandle heap;
    } data, index;

    unsigned numDescriptors; /* Number of descriptors to allocate.
                                If not set, a default allocation of 
                                descriptors will be used. */
} NEXUS_AudioMuxOutputCreateSettings;

/***************************************************************************
Summary:
    Get default settings for an Audio Mux output
***************************************************************************/
void NEXUS_AudioMuxOutput_GetDefaultCreateSettings(
    NEXUS_AudioMuxOutputCreateSettings *pSettings   /* [out] default settings */
    );

/***************************************************************************
Summary:
    Open an Audio Mux Ouptut Handle
***************************************************************************/
NEXUS_AudioMuxOutputHandle NEXUS_AudioMuxOutput_Create(     /* attr{destructor=NEXUS_AudioMuxOutput_Destroy}  */
    const NEXUS_AudioMuxOutputCreateSettings *pSettings     /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
    Close an Audio Mux Ouptut Handle
***************************************************************************/
void NEXUS_AudioMuxOutput_Destroy(
    NEXUS_AudioMuxOutputHandle handle
    );

/***************************************************************************
Summary:
Get the NEXUS_AudioOutput connector to make upstream connection
    
Description:
NEXUS_AudioMuxOutput can be connected to NEXUS_AudioEncoder (for transcode mode) as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(muxOutput), NEXUS_AudioEncoder_GetConnector(audioEncoder));
        
Or it can be connected to NEXUS_AudioDecoder (for passthrough mode) as follows:

    NEXUS_AudioOutput_AddInput(NEXUS_AudioMuxOutput_GetConnector(muxOutput), NEXUS_AudioDecoder_GetConnector(audioDecoder));
        
***************************************************************************/
NEXUS_AudioOutputHandle NEXUS_AudioMuxOutput_GetConnector(
    NEXUS_AudioMuxOutputHandle handle
    );

/***************************************************************************
Summary:
Audio Mux Output Delay Status
***************************************************************************/
typedef struct NEXUS_AudioMuxOutputDelayStatus 
{
    unsigned endToEndDelay; /* End to end delay for the current input and codec in ms */
} NEXUS_AudioMuxOutputDelayStatus;

/***************************************************************************
Summary:
Get Audio Mux Output delay 
 
Description: 
This routine will return the path delay status for the audio mux buffer. 
The MuxOutput handle must be connected to the expected data source prior 
to calling this routine in order to determine the correct delay value. 
***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_GetDelayStatus(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioCodec codec,
    NEXUS_AudioMuxOutputDelayStatus *pStatus    /* [out] */
    );

/***************************************************************************
Summary:
Audio Mux Output Start Settings
***************************************************************************/
typedef struct NEXUS_AudioMuxOutputStartSettings
{
    NEXUS_StcChannelHandle stcChannel;      /* The STC Channel Handle links audio and video to provide lipsync in the encoded stream */
    unsigned presentationDelay;             /* Arrival to Presentation Delay in ms */
    bool nonRealTime;                       /* If true, run in non-real-time mode.  Important: This field requires the 
                                               MuxOutput to be started prior to starting any inputs.  Decoder inputs should
                                               also have NEXUS_AudioDecoderSettings.nonRealTime set to match this field. */
} NEXUS_AudioMuxOutputStartSettings;

/***************************************************************************
Summary:
Get default start-time settings for a mux output
***************************************************************************/
void NEXUS_AudioMuxOutput_GetDefaultStartSettings(
    NEXUS_AudioMuxOutputStartSettings *pSettings  /* [out] default settings */
    );

/***************************************************************************
Summary:
Start capturing data.  

Decription:
An input must be connected prior to starting.

See Also:
NEXUS_AudioMuxOutput_Stop
NEXUS_AudioMuxOutput_GetConnector
***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_Start(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputStartSettings *pSettings          /* attr{null_allowed=y} */
    );

/***************************************************************************
Summary:
Stop capturing data.
***************************************************************************/
void NEXUS_AudioMuxOutput_Stop(
    NEXUS_AudioMuxOutputHandle handle
    );

/***************************************************************************
Summary:
MuxOutput Settings
***************************************************************************/
typedef struct NEXUS_AudioMuxOutputSettings
{
    NEXUS_CallbackDesc overflow;    /* This callback fires if the output buffer overflows */
} NEXUS_AudioMuxOutputSettings;

/***************************************************************************
Summary:
Get MuxOutput Settings
***************************************************************************/
void NEXUS_AudioMuxOutput_GetSettings(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set MuxOutput Settings
***************************************************************************/
NEXUS_Error NEXUS_AudioMuxOutput_SetSettings(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputSettings *pSettings
    );

/**
Summary:
**/
typedef struct NEXUS_AudioMuxOutputStatus
{
    NEXUS_MemoryBlockHandle bufferBlock;         /* block handle for NEXUS_AudioMuxOutputFrame.offset */
    NEXUS_MemoryBlockHandle metadataBufferBlock; /* block handle for NEXUS_AudioMuxOutputFrame.offset when it's used to carry metadata  */
    unsigned numFrames;                 /* Number of successfully encoded frames */
    unsigned numErrorFrames;            /* Number of error frames */
    unsigned numDroppedFrames;          /* Number of frames dropped due to overflow */
    struct {
        unsigned fifoDepth;                  /* depth in bytes of buffer */
        unsigned fifoSize;                   /* size in bytes of buffer */
    } data, index;
} NEXUS_AudioMuxOutputStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetStatus(
    NEXUS_AudioMuxOutputHandle handle,
    NEXUS_AudioMuxOutputStatus *pStatus /* [out] */
    );

/**
Summary:
Flags for the NEXUS_AudioMuxOutputFrame.flags field 
**/
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_ORIGINALPTS_VALID 0x00000001
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_PTS_VALID 0x00000002
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_FRAME_START 0x00010000
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_EOS 0x00020000
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_EMPTY_FRAME 0x00040000 /* PTS and ESCR are expected to be valid. LENGTH is expected to be 0. */
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_FRAME_END 0x00080000 /* If both FRAME_START and FRAME_END are set, then the descriptor references a complete frame */
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_EOC 0x00100000 /* unused */
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_METADATA 0x40000000
#define NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_EXTENDED 0x80000000

/**
Summary:
Data returned by NEXUS_AudioMuxOutput_GetBuffer

Description:
This must match BAVC_AudioBufferDescriptor.
**/
typedef struct NEXUS_AudioMuxOutputFrame
{
    uint32_t flags;
    
    /* Timestamp Parameters */
    uint32_t originalPts; /* 32-bit original PTS value (in 45 KHz) */
    uint64_t pts; /* 33-bit PTS value (in 90 KHz) */
    uint64_t stcSnapshot; /* 42-bit STC snapshot when frame received by the encoder (in 27MHz) */
    
    /* Transmission Parameters */
    uint32_t escr; /* Expected mux transmission start time (in 27 MHz) */
    uint16_t ticksPerBit;
    int16_t shr;
    
    /* Buffer Parameters */
    unsigned offset; /* offset of frame data from frame buffer base address (in bytes) */
    size_t length;   /* 0 if fragment is empty, e.g. for EOS entry (in bytes) */
    unsigned encoderIndex;
    unsigned unused;

    /* Offset to RAW Frame Data and length. */      
    unsigned rawDataOffset;    /* For most codecs, this will be equivalent to offset but if the data is encapsulated in another 
                                  container (e.g. AAC ADTS) this will reflect the offset to the raw data block within the encapsulated 
                                  frame. */
    size_t   rawDataLength;    /* For most codecs, this will be equivalent to length but if the data is encapsulated in another 
                                  container (e.g. AAC ADTS) this will reflect the length of the raw data block within the encapsulated 
                                  frame. */
    /* Metadata */
    uint8_t dataUnitType;   /* If stCommon.uiFlags == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA
                             *  this indicates the type of metadata that is contained in the buffer.
                             *  See NEXUS_AudioMetadataType enum for possible types and values
                             */
} NEXUS_AudioMuxOutputFrame;


/**
Summary: Audio Metadata Flags 
**/
#define NEXUS_AUDIOMETADATADESCRIPTOR_FLAGS_BITRATE_VALID                    0x00000001
#define NEXUS_AUDIOMETADATADESCRIPTOR_FLAGS_SAMPLING_FREQUENCY_VALID         0x00000002
#define NEXUS_AUDIOMETADATADESCRIPTOR_FLAGS_PROTOCOL_DATA_VALID              0x00000004
#define NEXUS_AUDIOMETADATADESCRIPTOR_FLAGS_TIMING_VALID                     0x00000008

#define NEXUS_AUDIO_SPECIFIC_CONFIG_MAX_LENGTH (8)

/**
Summary: Audio Metadata Types

Description:
This must match BAVC_AudioMetadataType.
**/
typedef enum NEXUS_AudioMetadataType
{
    NEXUS_AudioMetadataType_eCommon, /* NEXUS_AudioMetadataDescriptor */

    /* This enum cannot contain more than 256 entries because uiDataUnitType is defined as a uint8_t */
    NEXUS_AudioMetadataType_eMax
} NEXUS_AudioMetadataType;

/**
Summary: Metadata returned by NEXUS_AudioMuxOutput_GetBuffer

Description:
This must match BAVC_AudioMetadataDescriptor.
**/
typedef struct NEXUS_AudioMetadataDescriptor
{
    unsigned metadataFlags;

    struct
    {
        unsigned maximum; /* in bits/sec */
    } bitrate;

    struct
    {
        unsigned sampleRate;   /* In Hz */
    } samplingFrequency;

    struct
    {
        uint64_t stcSnapshot; /* Initial 42-bit STC Snapshot from audio encode */
        unsigned chunkId;     /* The FNRT chunk ID for the subsequent frame descriptors */
    } timing;

    unsigned reserved;
    union
    {
        struct
        {
            uint8_t asc[NEXUS_AUDIO_SPECIFIC_CONFIG_MAX_LENGTH];  /* Audio Specific Config from ISO 11496-3 */
            unsigned ascLengthBits;     /* Length In Bits */
            unsigned ascLengthBytes;    /* Length In Bytes - Since this is a bitfield extra bytes will be 0 filled */
        } aac;                          /* Applies for NEXUS_AudioCodec_eAacAdts, NEXUS_AudioCodec_eAacLoas, 
                                           NEXUS_AudioCodec_eAacPlusAdts, NEXUS_AudioCodec_eAacPlusLoas */
        struct
        {
            unsigned samplesPerBlock;
            unsigned encodeOptions;
            unsigned superBlockAlign;
            unsigned blockAlign;
            unsigned numChannels;
        } wmaStd;                    /* Applies for NEXUS_AudioCodec_eWmaStd */    
    } protocolData;
} NEXUS_AudioMetadataDescriptor;


/**
Summary:
The mux manager (or other consumer) will call this API to get encoded frames from the NEXUS_AudioMuxOutput
**/
NEXUS_Error NEXUS_AudioMuxOutput_GetBuffer(
    NEXUS_AudioMuxOutputHandle handle,
    const NEXUS_AudioMuxOutputFrame **pBuffer, /* [out] attr{memory=cached} pointer to NEXUS_AudioMuxOutputFrame structs */
    size_t *pSize,  /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer */
    const NEXUS_AudioMuxOutputFrame **pBuffer2, /* [out] attr{memory=cached} pointer to NEXUS_AudioMuxOutputFrame structs after wrap around */
    size_t *pSize2  /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer2 */
    );

/**
Summary:
Report how much data returned by last NEXUS_AudioMuxOutput_GetBuffer call was consumed
**/
NEXUS_Error NEXUS_AudioMuxOutput_ReadComplete(
    NEXUS_AudioMuxOutputHandle handle,
    unsigned framesCompleted /* must be <= pSize+pSize2 returned by last NEXUS_AudioMuxOutput_GetBuffer call. */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_AUDIO_MUX_OUTPUT_H__ */

