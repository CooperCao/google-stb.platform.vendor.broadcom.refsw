/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
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
#ifndef NEXUS_RECPUMP_H__
#define NEXUS_RECPUMP_H__

#include "nexus_types.h"
#include "nexus_pid_channel.h"
#include "nexus_dma_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*
The Recpump Interface provides data feeding for DVR record functionality.
Each Interface corresponds to one hardware record channel (RAVE context).

Each Recpump Interface captures two streams: data and index.
The data stream is typically an MPEG2 TS stream with all of the selected pids muxed together. (See NEXUS_RecpumpSettings.outputTransportType for other output formats).
The index stream, which is optional, contains SCT (start code table) entries which point to start codes in the data stream.

To record without indexing, just leave NEXUS_RecpumpAddPidChannelSettings.pidTypeSettings.XXX.index = false when you add a pid channel.

See nexus/examples/dvr/recpump.c for an example application.

See NEXUS_RecordHandle for a high-level Record Interface.
See NEXUS_PlaypumpHandle for the corresponding low-level DVR playback Interface.
**/

/**
Summary:
Handle for a Recpump Interface
**/
typedef struct NEXUS_Recpump *NEXUS_RecpumpHandle;

/**
Summary:
Indexing capability of recpump being opened
**/
typedef enum NEXUS_RecpumpIndexType
{
    NEXUS_RecpumpIndexType_eTransport, /* Allocate record context capable of producing Start Code Table (SCT) format for TS streams.
                                          NEXUS_Record can convert to NAV. */
    NEXUS_RecpumpIndexType_eEs,        /* Allocate record context capable of producing Index Table Buffer (ITB) format, which is normally used for decode.
                                          NEXUS_Record does not process. Useful for PES/ES recordings. */
    NEXUS_RecpumpIndexType_eNone,      /* No indexing capability. */
    NEXUS_RecpumpIndexType_eMax
} NEXUS_RecpumpIndexType;

/**
Summary:
substruct for NEXUS_RecpumpOpenSettings
**/
typedef struct NEXUS_RecpumpOpenFlowSettings
{
    NEXUS_HeapHandle heap; /* Optional heap for fifo allocation. If server has memory mapping, it will flush before GetBuffer; if no mapping, client app is responsible to flush after GetBuffer. */
    void *buffer;          /* attr{memory=cached} optional user-allocated buffer */
    size_t bufferSize;     /* Size of record buffer that recpump will allocate or that buffer points to, in bytes. */
    unsigned alignment;    /* Alignment of record buffer that recpump will allocate, in powers of 2 (i.e. 4 would be 2^4, or 16 byte aligned).
                              Default is 12, which is 4K alignment. */
    unsigned dataReadyThreshold; /* Threshold in bytes. The NEXUS_RecpumpSettings dataReady callback will be fired when pending data exceeds this threshold. Default is 20% of bufferSize.
                                    A low number will produce more frequent dataReady callbacks.
                                    A high number puts the system at risk for overflow.
                                    If you are also specifying an atomSize, we recommend that dataReadyThreshold be a multiple of atomSize for efficiency. */
    unsigned atomSize;  /* Optional atomSize. If the caller specifies atomSize, and always consumes data in multiples of atomSize, then Recpump can guarantee
                           that GetBuffer will never return a number less than atomSize, even at the wrap around.
                           One example where this is needed is DMA to a disk which requires 4K page size writes.
                           This is only supported for NEXUS_TransportType_eTs output format. */
} NEXUS_RecpumpOpenFlowSettings;

/**
Summary:
Settings passed into NEXUS_Recpump_Open

Description:
Recpump's data.bufferSize (aka CDB) should be chosen based on maximum stream bitrate and worst-case latency in the system and
a minimum segmentation of the buffer (required for pipelining I/O requests). It must be big enough to avoid overflow in all cases. For instance:

    20Mbps * 250msec/segment * 4 segments = 20 * 1024 * 1024 / 8 * 250 / 1000 * 4 = 2.5M

Recpump's index.bufferSize (aka ITB) should hold the maximum number of start codes and other SCT entries that can fit in a full
data buffer. Worst case is a low bitrate stream. The following is an approximation:

    2.5M CDB / 10K per picture * (16 slices/picture + 5 SC's for various overhead) = 5376 ITB's * 24 bytes/ITB = 126K

Because the index data rate is much smaller and because it can be fully evacuated separately from the data buffer, index.bufferSize
can be set smaller.

For both data and index buffers, you should characterize your system and set your bufferSize values appropriately.
**/
typedef struct NEXUS_RecpumpOpenSettings
{
    NEXUS_RecpumpOpenFlowSettings data;  /* stream data */
    NEXUS_RecpumpOpenFlowSettings index; /* index data */

    NEXUS_RecpumpHandle dummyRecpump; /* The dummy recpump has the CDB which this recpump will use as a bit bucket.
                                         This recpump will have its own ITB for index-only record. */
    unsigned nullifyVct; /* see nexus_recpump.c comments for documentation. */
    bool useSecureHeap; /* Deprecated. For compressed restricted region (CRR), set data.heap to heap[NEXUS_VIDEO_SECURE_HEAP]. */
    NEXUS_RecpumpIndexType indexType;
} NEXUS_RecpumpOpenSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
**/
void NEXUS_Recpump_GetDefaultOpenSettings(
    NEXUS_RecpumpOpenSettings *pSettings /* [out] */
    );

/**
Summary:
Open a new Recpump Interface
**/
NEXUS_RecpumpHandle NEXUS_Recpump_Open( /* attr{destructor=NEXUS_Recpump_Close} */
    unsigned index, /* The Nexus index for recpump. This index does not correspond to a physical RAVE context. NEXUS_ANY_ID is supported. */
    const NEXUS_RecpumpOpenSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Close a Recpump Interface
**/
void NEXUS_Recpump_Close(
    NEXUS_RecpumpHandle handle
    );

/**
Summary:
Controls flow-control of playpump by connected recpump

Description:
eDisable is the default for backward compatibility. eAuto may become the default in the future.
for record of live streams, eDisable is preferred.
for record of playback streams, eEnable is preferred; however, there are currently implementation issues. see comments in nexus_recpump.c for information.
**/
typedef enum NEXUS_RecpumpFlowControl {
    NEXUS_RecpumpFlowControl_eDisable, /* recpump doesn't flow control playpump, so playpump could overflow recpump buffer */
    NEXUS_RecpumpFlowControl_eEnable, /* recpump flow controls playpump, so playpump is paused by HW to prevent overflow of recpump buffer */
    NEXUS_RecpumpFlowControl_eAuto,   /* flow control is activated based on type of source connected to the recpump. playpump has flow control; live does not. */
    NEXUS_RecpumpFlowControl_eMax
} NEXUS_RecpumpFlowControl;

#define NEXUS_NUM_ECM_TIDS 3

/**
Summary:
Settings which are applied with NEXUS_Recpump_SetSettings.

Description:
Some are required before you can call NEXUS_Recpump_Start.
**/
typedef struct NEXUS_RecpumpSettings
{
    struct
    {
        size_t useBufferSize;         /* Limit the size of NEXUS_RecpumpOpenSettings.data.bufferSize at runtime.
                                         This currently only applies to data, not index.
                                         The user can repurpose memory at the end of the recpump buffer if useBufferSize < bufferSize. See NEXUS_RecpumpStatus.data.bufferBase.
                                         If 0 (default), the whole bufferSize will be used. */
        NEXUS_CallbackDesc dataReady; /* data.dataReady is required. index.dataReady is not required but is typically used for non-polling record.
                                         A data or index dataReady callback is fired in these cases:
                                         1) in response to a XPT or DMA interrupt if the amount of data in the buffer exceeds exceeds the dataReadyThreshold
                                         2) at the end of a ReadComplete call if the data in the buffer exceeds the dataReadyThreshold
                                         3) at wrap-around regardless of buffer level
                                         If you receive a dataReady callback and do not call ReadComplete, no additional callback will be fired.
                                         Your app should not do synchronous I/O in a dataReady callback. Instead, you should schedule asynchronous I/O (either by using an
                                         async file I/O system call or by notifying another thread to start the I/O). */
        NEXUS_CallbackDesc overflow;  /* Notification when data is dropped because there is no room in the buffer.
                                         Under normal circumstances, this should never occur. If it does, please analyze your overall system
                                         performance including I/O performance, interrupt response time and CPU load. */
    } data, index;

    NEXUS_DmaHandle securityDma;               /* deprecated */
    NEXUS_DmaDataFormat securityDmaDataFormat; /* deprecated */
    NEXUS_KeySlotHandle securityContext;       /* used for DRM */

    NEXUS_RecpumpFlowControl bandHold; /* If NEXUS_RecpumpFlowControl_eEnable, record will cause any incoming playback to hold when it meets the dataReadyThreshold (i.e. UPPER_THRESHOLD).
                                        dataReadyThreshold should be set to a high percentage, like 90%. So, unless the app is polling, record will stop receiving
                                        data when the app is doing file i/o. Unless you have a buffer in front of record which can continue receiving data
                                        while playback is held, this is very inefficient.
                                        If HW ever gets separate thresholds for data ready and for bandhold, the threshold setting for bandHold could change. */

    NEXUS_TransportTimestampType timestampType; /* An additional timestamp will be prepended to the packets as they are recorded.
                                                   If you are recording a playback stream that already has timestamps, this setting will only preserve those timestamps; it will not
                                                   replace them with new timestamps.
                                                   If you are recording a live stream that already has timestamps, you should set NEXUS_InputBandSettings.packetLength to strip
                                                   the incoming timestamps. Then, this setting can be used to add new timestamps if desired. */
	bool adjustTimestampUsingPcrs;		/* Adjsut timestamps using PCR information. This removes the timestamp jitter induced by the trip through the hw DRAM buffers. The end result
										is that the time delta between timestamps will closely match the delta between PCRs. */
	NEXUS_PidChannelHandle pcrPidChannel;		/* The timestamp jitter removal needs to know which PID channel is carrying the PCRs. */

    NEXUS_TransportType outputTransportType; /* NEXUS_TransportType_eTs (default), eDssEs, eDssPes, eMpeg2Pes and eEs supported.
                                         Recpump can strip TS or PES headers, but cannot add TS or PES headers.
                                         Therefore conversion from TS -> PES or TS/PES -> ES is supported. PES/ES -> TS and ES -> PES is not. */
    bool dropBtpPackets; /* If true, search for BPPs and drop them from record output */
    bool localTimestamp; /* if set use timestamps from the BPP packet, otherwise use the arrival timestamps of incoming packets */

    /* The following TPIT settings are only applied at NEXUS_Recpump_Start time. They apply to every TPIT filter in this Recpump.
    You must create a pid channel and set at least one TPIT filter before calling Start in order for them to take effect. */
    struct {
        bool countRecordedPackets;  /* If true, then the recorded count in TPIT entries will be in units of packets; otherwise, bytes. */
        bool storePcrMsb;           /* Store the 32 MSB of the PCR if true; store the 32 LSB if false (default) */
        bool firstPacketEnable;     /* Store an index table entry for the first packet recorded. */
        bool idleEventEnable;       /* Store an event if no other events have occurred for the NEXUS_TransportModuleSettings.tpit.idleEventTimeout duration. */
        bool recordEventEnable;     /* Store an event if no packets have been recorded for the NEXUS_TransportModuleSettings.tpit.recordEventTimeout duration. */
        struct {
            bool enabled;
            unsigned evenTid;       /* table ID for even ECM packets */
            unsigned oddTid;        /* table ID for odd ECM packets */
        } ecmPair[NEXUS_NUM_ECM_TIDS]; /* user can filter multiple ECM table ID's simultaneously */
    } tpit;

    /* See nexus/extensions/transport/tsio/include/nexus_tsio.h for information. This is not associated with security DMA support above. */
    NEXUS_CallbackDesc tsioDmaEnd;
} NEXUS_RecpumpSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
**/
void NEXUS_Recpump_GetDefaultSettings(
    NEXUS_RecpumpSettings *pSettings /* [out] */
    );

/**
Summary:
Get current NEXUS_RecpumpSettings
**/
void NEXUS_Recpump_GetSettings(
    NEXUS_RecpumpHandle handle,
    NEXUS_RecpumpSettings *pSettings /* [out] */
    );

/**
Summary:
Set new NEXUS_RecpumpSettings.

Description:
Some settings are required before calling NEXUS_Recpump_Start.
Settings can be changed after Start.
**/
NEXUS_Error NEXUS_Recpump_SetSettings(
    NEXUS_RecpumpHandle handle,
    const NEXUS_RecpumpSettings *pSettings
    );

/**
Summary:
Start recording.

Description:
Actual recording may not start until the first pid channel is added.
**/
NEXUS_Error NEXUS_Recpump_Start(
    NEXUS_RecpumpHandle handle
    );

/**
Summary:
Stop new data from entering record buffer, but callbacks and GetBuffer/ReadComplete still work.

Description:
This allows the user to get every byte out of the buffer. However user is still responsible to call
NEXUS_Recpump_Stop when all data is extracted.
**/
void NEXUS_Recpump_StopData(
    NEXUS_RecpumpHandle handle
    );

/**
Summary:
Stops recording.

Description:
After this function returns GetBuffer/ReadComplete couldn't be used to access recoreded data.
**/
void NEXUS_Recpump_Stop(
    NEXUS_RecpumpHandle handle
    );

/**
Summary:
The TPIT parse table entry consists of enables for each bitfield in the transport
packet that can be parsed. In most cases, there is also a value that the
bitfield must match in order to have an index generated.
**/
typedef struct NEXUS_RecpumpTpitFilter
{
    unsigned pid;               /* The PID to build TPIT entries for. Nexus will set this automatically if not allPass. This is included only to make the structure a passthrough to the lower-level SW. */
    bool corruptionEnable;      /* Corrupt packet if it matches the filter criterion below. Disabled by default */
    bool mpegMode;              /* Set true for TS, false for DSS. This is used for the config structure. */

    union
    {
        struct
        {
            bool ecmPolarityChangeEn;

            bool sectionFilterEn;

            bool adaptationExtensionFlagEnable;         /* Store an index table entry if this bit is set and the adaptation_field_extension_flag matches the corresponding compare value. */
            bool adaptationExtensionFlagCompValue;      /* The value to compare adaptation_field_extension_flag with. */

            bool privateDataFlagEnable;                 /* Store an index table entry if this bit is set and the transport_private_data_flag matches the corresponding compare value. */
            bool privateDataFlagCompValue;              /* The value to compare transport_private_data_flag with. */

            bool splicingPointFlagEnable;               /* Store an index table entry if this bit is set and the splicing_point_flag matches SplicingPointFlagCompValue */
            bool splicingPointFlagCompValue;            /* The value to compare splicing_point_flag with. */

            bool opcrFlagEnable;                        /* Store an index table entry if this bit is set and the OPCR_flag matches the corresponding compare value */
            bool opcrFlagCompValue;                     /* The value to compare OPCR_flag with. */

            bool pcrFlagEnable;                         /* Store an index table entry if this bit is set and the PCR_flag matches the corresponding compare value */
            bool pcrFlagCompValue;                      /* The value to compare PCR_flag with. */

            bool esPriorityIndicatorEnable;             /* Store an index table entry if this bit is set and the elementary_stream_priority_indicator matches the corresponding compare value */
            bool esPriorityIndicatorCompValue;          /* The value to compare elementary_stream_priority_indicator with. */

            bool randomAccessIndicatorEnable;           /* Store an index table entry if this bit is set and the random_access_indicator matches the corresponding compare value */
            bool randomAccessIndicatorCompValue;        /* The value to compare random_access_indicator with. */

            bool discontinuityIndicatorEnable;          /* Store an index table entry if this bit is set and the discontinuity_indicator matches the corresponding compare value */
            bool discontinuityIndicatorCompValue;       /* The value to compare discontinuity_indicator with. */

            bool adaptationFieldChangeEnable;           /* Store an index table entry if a change is detected in the adaptation field control field (enable only) */

            bool scramblingControlChangeEnable;         /* Store an index table entry if a change is detected in the scrambling control field control field (enable only) */

            bool transportPriorityEnable;               /* Store an index table entry if this bit is set and the transport_priority matches the corresponding compare value */
            bool transportPriorityCompValue;            /* The value to compare transport_priority with. */

            bool payloadUnitStartEnable;                /* Store an index table entry if this bit is set and the payload_unit_start_indicator matches the corresponding compare value */
            bool payloadUnitStartCompValue;             /* The value to compare payload_unit_start_indicator with. */
        } mpeg;

        struct
        {
            bool filterHdEnable;        /* Enable HD filtering for this PID. */
            unsigned short hdFilter;    /* The HD value to filter on. Transport packet matching this value are NOT recorded. */

            bool tcDetEn;           /* Store an index table entry if a valid timecode is contained in the packet (enable only) */

            bool cwpDetEn;          /* Store an index table entry if a valid CWP is contained in the packet (enable only) */

            bool rtsDetEn;          /* Store an index table entry if a valid RTS is contained in the packet (enable only) */

            bool cffEn;             /* Store an index table entry if this bit is set and the current field flag matches the corresponding compare value */
            bool cffComp;           /* The current field flag must match this value if the corresponding enable bit is set in order for an index table entry to be made */

            bool mfEn;              /* Store an index table entry if this bit is set and the modifiable flag matches the corresponding compare value */
            bool mfComp;            /* The modifiable flag must match this value if the corresponding enable bit is set in order for an index table entry to be made */

            bool hdEn;              /* Store an index table entry for any bit set in this field and the respective compare bit matches the
                                        respective header designator bit (only for valid HD i.e. unencrypted packets) */
            uint8_t hdMask;         /* The header designator bits must bitwise match this value if the corresponding enable bit is set in order for an index table entry to be made.
                                        (only for valid HD i.e. unencrypted packets) */

            bool csAuxChangeEn;     /* Store an index table entry if a change is detected in the control sync for CWP packets (enable only) */

            bool csChangeEn;        /* Store an index table entry if a change is detected in the control sync for content (non AUX)packets (enable only) */

            bool cfChangeEn;        /* Store an index table entry if a change is detected in the control flag for content (non AUX)packets (enable only) */

            bool bbEn;              /* Store an index table entry if this bit is set and the bundle boundary matches the corresponding compare value */
            bool bbComp;            /* The bundle boundary must match this value if the corresponding enable bit is set in order for an index table entry to be made */
        } dss;
    } config;
} NEXUS_RecpumpTpitFilter;

#define NEXUS_NUM_STARTCODE_RANGES 4

/**
Summary:
Settings used for NEXUS_Recpump_AddPidChannel
**/
typedef struct NEXUS_RecpumpAddPidChannelSettings
{
    NEXUS_PidType pidType;
    struct
    {
        struct
        {
            bool index;                  /* index this video pid. You can only index one video pid. */
            NEXUS_VideoCodec codec;      /* only needed if index is true */
            uint16_t pid;                /* index using pid value, not pid channel. if pid==0x1fff (default) then generate index using the pid channel, not this pid value.
                                            This is needed for indexing an allpass pid channel */
        } video; /* settings for pidType == NEXUS_PidType_eVideo */
        struct
        {
            bool index;                  /* index start codes based on the following ranges (default is all start codes) */
            struct {
                uint8_t low;
                uint8_t high;
            } startCodeRange[NEXUS_NUM_STARTCODE_RANGES];
        } other; /* settings for pidType == NEXUS_PidType_eOther */
    } pidTypeSettings;
    bool useRPipe;
} NEXUS_RecpumpAddPidChannelSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
You can use default settings for all pids except an indexed video pid.
**/
void NEXUS_Recpump_GetDefaultAddPidChannelSettings(
    NEXUS_RecpumpAddPidChannelSettings *pSettings /* [out] */
    );

/*
Summary:
Add a pid channel to be recorded.

Description:
You can add a pid channel before or after NEXUS_Recpump_Start.
You cannot change the pid channel being indexed after starting.
*/
NEXUS_Error NEXUS_Recpump_AddPidChannel(
    NEXUS_RecpumpHandle handle,
    NEXUS_PidChannelHandle pidChannel,
    const NEXUS_RecpumpAddPidChannelSettings *pSettings /* attr{null_allowed=y} NULL is allowed for default settings. */
    );

/*
Summary:
Remove a pid channel and stop recording it.

Description:
You can remove a pid channel before or after NEXUS_Recpump_Stop.
*/
NEXUS_Error NEXUS_Recpump_RemovePidChannel(
    NEXUS_RecpumpHandle handle,
    NEXUS_PidChannelHandle pidChannel
    );

/**
Summary:
Remove all pid channels and stop recording them.
**/
void NEXUS_Recpump_RemoveAllPidChannels(
    NEXUS_RecpumpHandle handle
    );

/**
Summary:
Get a pointer and size for the next location in the buffer which has data which has been recorded.

Description:
The application is responsible for reading this data out of the buffer and notifying Recpump
of how much it as read using NEXUS_Recpump_DataReadComplete.
NEXUS_Recpump_GetDataBuffer is non-destructive. You can safely call it multiple times.
**/
NEXUS_Error NEXUS_Recpump_GetDataBuffer(
    NEXUS_RecpumpHandle handle,
    const void **pBuffer,   /* [out] attr{memory=cached} pointer to data which can be consumed */
    size_t *pAmount         /* [out] number of recorded bytes which can be consumed */
    );

/**
Summary:
Get a pointer and size for the next location in the buffer, including possible wrap around, which has data which has been recorded.

Description:
The 'WithWrap' version only works if you are not using DMA encryption.
If you request pBuffer2 and pAmount2, your atomSize will be ignored. All data will be returned.
You can call NEXUS_Recpump_DataReadComplete with <= *pAmount + *pAmount2
**/
NEXUS_Error NEXUS_Recpump_GetDataBufferWithWrap(
    NEXUS_RecpumpHandle handle,
    const void **pBuffer,   /* [out] attr{memory=cached} pointer to data which can be consumed */
    size_t *pAmount,        /* [out] number of recorded bytes pointed to by pBuffer which can be consumed */
    const void **pBuffer2,  /* [out] attr{memory=cached} optional pointer to data after wrap which can be consumed */
    size_t *pAmount2        /* [out] number of recorded bytes pointed to by pBuffer2 which can be consumed */
    );

/**
Summary:
Notify Recpump of how much data the application consumed from the buffer.

Description:
You can only call NEXUS_Recpump_DataReadComplete once after a NEXUS_Recpump_GetDataBuffer call.
After calling it, you must call NEXUS_Recpump_GetDataBuffer before adding more data.
**/
NEXUS_Error NEXUS_Recpump_DataReadComplete(
    NEXUS_RecpumpHandle handle,
    size_t amount /* number of bytes the application has consumed */
    );

/* backward compatibility */
#define NEXUS_Recpump_DataWriteComplete NEXUS_Recpump_DataReadComplete

/**
Summary:
Get a pointer and size for the next location in the buffer which has data which has been recorded.

Description:
The application is responsible for reading this data out of the buffer and notifying Recpump
of how much it as read using NEXUS_Recpump_IndexReadComplete.
NEXUS_Recpump_GetIndexBuffer is non-destructive. You can safely call it multiple times.
**/
NEXUS_Error NEXUS_Recpump_GetIndexBuffer(
    NEXUS_RecpumpHandle handle,
    const void **pBuffer,   /* [out] attr{memory=cached} pointer to data which can be consumed */
    size_t *pAmount         /* [out] number of recorded bytes which can be consumed */
    );

/**
Summary:
Get a pointer and size for the next location in the buffer, including possible wrap around, which has data which has been recorded.

Description:
The 'WithWrap' version only works if you are not using DMA encryption.
If you request pBuffer2 and pAmount2, your atomSize will be ignored. All data will be returned.
You can call NEXUS_Recpump_IndexReadComplete with <= *pAmount + *pAmount2
**/
NEXUS_Error NEXUS_Recpump_GetIndexBufferWithWrap(
    NEXUS_RecpumpHandle handle,
    const void **pBuffer,   /* [out] attr{memory=cached} pointer to data which can be consumed */
    size_t *pAmount,        /* [out] number of recorded bytes pointed to by pBuffer which can be consumed */
    const void **pBuffer2,  /* [out] attr{memory=cached} optional pointer to data after wrap which can be consumed */
    size_t *pAmount2        /* [out] number of recorded bytes pointed to by pBuffer2 which can be consumed */
    );

/**
Summary:
Notify Recpump of how much data the application consumed from the buffer.

Description:
You can only call NEXUS_Recpump_IndexReadComplete once after a NEXUS_Recpump_GetIndexBuffer call.
After calling it, you must call NEXUS_Recpump_GetIndexBuffer before adding more data.
**/
NEXUS_Error NEXUS_Recpump_IndexReadComplete(
    NEXUS_RecpumpHandle handle,
    size_t amount /* number of bytes the application has consumed */
    );

/* backward compatibility */
#define NEXUS_Recpump_IndexWriteComplete NEXUS_Recpump_IndexReadComplete

/**
Summary:
Status obtained from NEXUS_Recpump_GetStatus
**/
typedef struct NEXUS_RecpumpStatus
{
    bool started;
    bool hasIndex; /* If true, recpump is recording both data and index. This means at least one pid has index = true.
                      If false, recpump is only recording data. */
    struct
    {
        uint64_t bytesRecorded; /* Total bytes consumed from recpump since NEXUS_Recpump_Start. Does not include current fifoDepth. */
        size_t fifoDepth; /* Current depth of record fifo in bytes */
        size_t fifoSize;  /* Size of record fifo in bytes. This could be less than the OpenSettings bufferSize because of
                             internal alignment requirements. */
        void *bufferBase; /* attr{memory=cached} Pointer to the base of the record buffer.
                             This can be used for calculating your absolute position in the buffer. */
    } data, index;

    NEXUS_RecpumpOpenSettings openSettings; /* Settings used to open Recpump */

    struct {
        unsigned index;
    } rave;

    unsigned bitrate;   /* Recorded rate in bps. Rate is reset to 0 when NEXUS_Recpump is restarted. May get erroneous rate
                           if called long after StopData is called. */
} NEXUS_RecpumpStatus;

/**
Summary:
Current status of the Recpump
**/
NEXUS_Error NEXUS_Recpump_GetStatus(
    NEXUS_RecpumpHandle handle,
    NEXUS_RecpumpStatus *pStatus /* [out] */
    );

/**
Summary:
Get default filter
**/
void NEXUS_Recpump_GetDefaultTpitFilter(
    NEXUS_RecpumpTpitFilter *pFilter /* [out] */
    );

/**
Summary:
Set the TPIT filter for this pid channel

Description:
The TPIT filter will be removed if you call NEXUS_Recpump_SetTpitFilter with NULL
or if you remove the pid channel.

You are limited in the number of pid channels that the TPIT indexer can handle. See NEXUS_TransportCapabilities.numTpitPids.

NEXUS_Recpump_SetTpitFilter can be called before or after start.

One TPIT filter per pid is allowed.
**/
NEXUS_Error NEXUS_Recpump_SetTpitFilter(
    NEXUS_RecpumpHandle handle,
    NEXUS_PidChannelHandle pidChannel,
    const NEXUS_RecpumpTpitFilter *pFilter /* attr{null_allowed=y} set to NULL to disable TPIT filtering on this pid */
    );

#ifdef __cplusplus
}
#endif

#endif
