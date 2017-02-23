/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/


/*= Module Overview *********************************************************
Overview

Sample Code

    / * Configuration for AVC video decoding. * /
    BERR_Code Res;
    BXPT_Rave_ChannelSettings RaveDefaults;
    BXPT_Rave_Handle hRave;
    BXPT_RaveCx_Handle hCtx;
    BAVC_XptContextMap CtxMap;
    BXPT_Rave_ContextPtrs CtxPtrs;
    BXPT_Rave_AvSettings AvCtxCfg;

    / * The CdbItb struct is normally filled out by the decoder PI. Dummy values are used here. * /
    BAVC_CdbItbConfig CdbItb = { { 512 * 1024, 4, false }, { 512 * 1024, 4, false } };

    Res = BXPT_Rave_GetChannelDefaultSettings( hXpt, RAVE_CHNL_NUM, &RaveDefaults );
    BDBG_ASSERT( !Res );

    Res = BXPT_Rave_OpenChannel( hXpt, &hRave, RAVE_CHNL_NUM, &RaveDefaults );
    BDBG_ASSERT( !Res );

    Res =  BXPT_Rave_AllocContext( hRave, BXPT_RaveCx_eAv, &CdbItb, &hCtx );
    BDBG_ASSERT( !Res );

    / * PID channel 0 carries PID 0x810 from parser band 1 * /
    BXPT_ConfigurePidChannel( hXpt, 0, 0x810, 1 );
    BXPT_EnablePidChannel( hXpt, 0 );

    BXPT_Rave_AddPidChannel( hCtx, 0, false );

    / * Parser band 1 gets data from input band 2. * /
    Res = BXPT_SetParserDataSource( hXpt, 1, BXPT_DataSource_eInputBand, 2 );
    BDBG_ASSERT( !Res );

    Res = BXPT_SetParserEnable( hXpt, 1, true );
    BDBG_ASSERT( !Res );

    BXPT_Rave_GetAvConfig( hCtx, &AvCtxCfg );
    AvCtxCfg.InputFormat = BAVC_StreamType_eTsMpeg;
    AvCtxCfg.ItbFormat = BAVC_ItbEsType_eAvcVideo;
    AvCtxCfg.OutputFormat = BAVC_StreamType_eEs;
    AvCtxCfg.EsRanges[ 0 ].RangeHi = 0xFE;
    AvCtxCfg.EsRanges[ 0 ].RangeLo = 0;
    AvCtxCfg.EsRanges[ 0 ].Enable = true;
    AvCtxCfg.EsRanges[ 1 ].Enable = false;
    AvCtxCfg.EsRanges[ 2 ].Enable = false;
    AvCtxCfg.EsRanges[ 3 ].Enable = false;
    AvCtxCfg.StreamIdHi = 0xEF;
    AvCtxCfg.StreamIdLo = 0xBD;
    AvCtxCfg.BandHoldEn = false;
    Res = BXPT_Rave_SetAvConfig( hCtx, &AvCtxCfg );
    BDBG_ASSERT( !Res );

    Res = BXPT_Rave_EnableContext( hCtx );
    BDBG_ASSERT( !Res );

***************************************************************************/

#ifndef BXPT_RAVE_H__
#define BXPT_RAVE_H__

#include "bxpt.h"
#include "bavc.h"
#include "bxpt_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Misc defines used by the RAVE API.
****************************************************************************/
#define BXPT_MAX_STARTCODE_RANGES 4     /* The number of startcode ranges that the SCD indexers support. */
#define BXPT_NUM_AV_RANGES 4            /* The number of ES startcode ranges that the AV contexts support. */

/***************************************************************************
Summary:
Defines for the minimum and maximum number of ES bytes that the startcode
detection logic in the record contexts can extract.

Note that this number includes the startcode value itself. For example,
in the stream
  00 00 01 XX YY ZZ
an EsCount value of 2 will extract the startcode XX and the first value
following it, YY.
****************************************************************************/
#define BXPT_MIN_ES_COUNT   2           /* Minimum of 2 to avoid PR 28843. */
#define BXPT_MAX_ES_COUNT   16          /* Maximum number of ES bytes extracted */

/***************************************************************************
Summary:
Context overflow and wraparound thresholds.
****************************************************************************/
#define BXPT_RAVE_WRAP_THRESH       ( 0x100 )
#define BXPT_RAVE_OVERFLOW_THRESH   ( 3 * BXPT_RAVE_WRAP_THRESH )

/***************************************************************************
Summary:
The handle structures contain data global to all code that uses the RAVE
channel. Users should not access the contents.
****************************************************************************/
typedef struct BXPT_P_RaveHandle *BXPT_Rave_Handle;
typedef struct BXPT_P_ContextHandle *BXPT_RaveCx_Handle;
typedef struct BXPT_P_IndexerHandle *BXPT_RaveIdx_Handle;

/***************************************************************************
Summary:
Enumeration of all supported interrupts in a Rave context.
****************************************************************************/
typedef enum BXPT_RaveIntName
{
    BXPT_RaveIntName_eEmuErr = 0,               /* Emulation byte removal error */
    BXPT_RaveIntName_ePusiErr = 1,          /* PUSI error */
    BXPT_RaveIntName_eTeiErr = 2,               /* TEI error */
    BXPT_RaveIntName_eCcErr = 3,                /* CC error */
    BXPT_RaveIntName_eCdbOverflow = 4,      /* CDB overflow error */
    BXPT_RaveIntName_eItbOverflow = 5,      /* ITB overflow error */
    BXPT_RaveIntName_eSplice = 6,               /* Splice event */
    BXPT_RaveIntName_eLastCmd = 7,          /* BTP or BPP Last command encountered  interrupt */
    BXPT_RaveIntName_eCdbLowerThresh = 8,       /* CDB below lower threshold */
    BXPT_RaveIntName_eCdbUpperThresh = 9,       /* CDB above upper threshold */
    BXPT_RaveIntName_eItbLowerThresh = 10,  /* ITB below lower threshold */
    BXPT_RaveIntName_eItbUpperThresh = 11,       /* ITB above upper threshold */

#if BXPT_HAS_RAVE_MIN_DEPTH_INTR
    BXPT_RaveIntName_eCdbMinDepthThresh = 12,
    BXPT_RaveIntName_eItbMinDepthThresh = 13,
#endif

#if BXPT_NUM_TSIO
    BXPT_RaveIntName_eTsioDmaEnd = 14,
#endif
    BXPT_RaveIntName_eMax
}
BXPT_RaveIntName;

/***************************************************************************
Summary:
Enumeration of the different types of contexts supported. Applications that
don't do security-related AV or records should use BXPT_RaveCx_eAv and/or
BXPT_RaveCx_eRecord .
****************************************************************************/
typedef enum BXPT_RaveCx
{
    BXPT_RaveCx_eAv,        /* AV context, for moving data to the decoders. */
    BXPT_RaveCx_eRecord,    /* Record context, for PVR applications. */

    BXPT_RaveCx_eAvR,       /* AV context, using buffers allocated from the RHeap handle */
    BXPT_RaveCx_eRecordR,   /* Record context, using buffers allocated from the RHeap handle. */

    BXPT_RaveCx_eIp,        /* IP record context */
    BXPT_RaveCx_eVctNull    /* */
}
BXPT_RaveCx;

/***************************************************************************
Summary:
Enumeration of the different types of index table generation that the RAVE
currently supports.
****************************************************************************/
typedef enum BXPT_RaveIdx
{
    BXPT_RaveIdx_eScd,      /* Startcode detection. */
    BXPT_RaveIdx_eTpit      /* Transport parser index tables. */
}
BXPT_RaveIdx;

/***************************************************************************
Summary:
Some of the global items in the RAVE channel. This structure is used, among
other things, to get and set the defaults during the channel open.
****************************************************************************/

typedef BERR_Code (*RaveChannelOpenCB)(void);

typedef struct BXPT_Rave_ChannelSettings
{
    /* These items are for TPIT filtering, and are global to all record contexts */
    uint32_t TpitEventTimeout;      /* Number of 108 MHz clocks before triggering a record event timeout. */
    uint32_t TpitPacketTimeout;     /* Number of 108 MHz clocks before triggering a record packet timeout. */
    uint32_t TimeTick;              /* Number of 108-MHz cycles to wait before triggering a time tick event */
    RaveChannelOpenCB chanOpenCB;
    unsigned ThresholdGranularityInBytes;
}
BXPT_Rave_ChannelSettings;

/***************************************************************************
Summary:
Settings for the Record contexts.
****************************************************************************/
typedef struct BXPT_Rave_RecordSettings
{
    /* The type of transport we are recording. Set to true if MPEG, false if DirecTV */
    bool MpegMode;

    /*
    ** Type of compressed data output from RAVE. From this enum, only these
    ** types are supported:
    **  BAVC_StreamType_eTsMpeg,        MPEG or DirecTV Transport Stream
    **  BAVC_StreamType_ePes,           PES Stream
    **  BAVC_StreamType_eEs,            ES Stream
    */
    BAVC_StreamType OutputFormat;

    /* Set to true if timestamps should be prepended to the packets during record. */
    bool UseTimeStamps;

#if BXPT_HAS_ATS
    /* Adjust recorded timestamps to stay locked to the stream's PCRs. In other words,
    the delta between PTSs on PCR carrying packets will match the delta between the
    PCRs. */
    bool LockTimestampsToPcrs;
    unsigned PcrPidChannelIndex;

    #if BXPT_HAS_LOCAL_ATS
    bool GenerateLocalAts;      /* RAVE will generate a local timestamp for each packet */
    unsigned LocalAtsFormat;    /* Format of the locally generated timestamp. 0 = Mod-300, 1 = binary */
    #endif
#endif

    bool TsInitEn;          /* InitTs will be used as the initial timestamp value */
    uint32_t InitialTs;     /* Initial value for timestamps ( if TsInitEn == true ). */

    /*
    ** Selects the type of timestamps that will be prepended to the packets
    ** if UseTimeStamps (below) is true. See bxpt_playback.h for further
    ** details.
    */
    BXPT_TimestampMode TimestampMode;

    /*
    ** Disable parity checking for timestamps that will be prepended to the
    ** packets if UseTimeStamps (above) is true. See bxpt_playback.h for further
    ** details.
    */
    bool DisableTimestampParityCheck;

    /*
    ** The upper and lower bounds (non-inclusive) of the PES stream IDs that will be accepted
    ** during startcode generation.
    */
    uint8_t StreamIdHi;
    uint8_t StreamIdLo;

    /*
    ** Upper and lower thresholds. An interrupt to the CPU will be generated when data
    ** in the CDB when the exceeds the CdbUpperThreshold or drops below the CdbLowerThreshold.
    ** The ItbUpperThreshold and ItbLowerThreshold provide similar functions for ITB data.
    ** NOTE: These tresholds are specified in units of 256 bytes. Thus, to set a threshold
    ** to 1024 bytes, the treshold param must be set to 4.
    */
    uint16_t CdbUpperThreshold, CdbLowerThreshold;
    uint16_t ItbUpperThreshold, ItbLowerThreshold;

    /*
    ** Use the CDB bufffer smaller then actually allocated by  BXPT_Rave_AllocContext,
    ** For the non 0 UseCdbSize, the CDB end pointer is adjusted to reflect UseCdbSize.
    ** Use of this parameter is optional,should be set to 0 in normal usage modes
    */
    uint32_t UseCdbSize;

    /* If true, do not check CC bits for packets on this context. */
    bool DisableContinuityCheck;

    /* If true, recorded count used in TPIT entries will be packets. Otherwise, the count is bytes. */
    bool CountRecordedPackets;

    /*
    ** BandHoldEn allows the context to pause a playback channel, in order to prevent
    ** data loss if the host cannot service the buffers quickly enough. It is meant for
    ** transcoding or re-recording of playback data; it should be false for recording
    ** live data.
    */
    bool BandHoldEn;

#if BXPT_HAS_EMM_RAVE
    bool EmmModeEn;
#endif

    /* Bulk mode recording */
    bool BulkMode;

    /* Enables the indexer for single channel mux usage, needed for transcoding MUXlib usage. Default is false (disabled). */
    bool EnableSingleChannelMuxCapableIndexer;

    /* Set bipIndexing to true and specify bipPidChannel when using RAVE context for generic channel bonding index generation */
    bool bipIndexing;
    unsigned bipPidChannel; /* PID channel that will carry the BIP packets. */
}
BXPT_Rave_RecordSettings;

/***************************************************************************
Summary:
For each range that is enabled, only the startcode that fall between
these non-inclusive limits will appear in the ES index table. To
enable all startcodes, simply set RangeHi of any pair to 0xFF, then
RangeLo to 0x00, and enable that pair.
****************************************************************************/
typedef struct BXPT_Rave_EsRanges
{
    uint8_t RangeHi;    /* High end of ES range. */
    uint8_t RangeLo;    /* Low end of ES range. */
    bool RangeIsASlice; /* True if codes in this range are slices */
    bool Enable;        /* Enable flag for this range */
}
BXPT_Rave_EsRanges;

/***************************************************************************
Summary:
Settings for the Audio/Video contexts.
****************************************************************************/
typedef struct BXPT_Rave_AvSettings
{
    /*
    ** The format of the input stream. From this enum, only the following
    ** types are supported.
    **   BAVC_StreamType_eTsMpeg          MPEG transport packets
    **   BAVC_StreamType_eDssEs           DirecTV transport carrying ES data
    **   BAVC_StreamType_eDssPes          DirecTV transport carrying PES data
    **   BAVC_StreamType_eVCD             MPEG1 style PES packetized as MPEG transport packets
    */
    BAVC_StreamType InputFormat;

    BAVC_ItbEsType ItbFormat;           /* Type of ES data that ITB entries are built for. */

    /*
    ** Type of compressed data output from RAVE. From this enum, only these
    ** types are supported:
    **  BAVC_StreamType_eTsMpeg,        MPEG Transport Stream
    **  BAVC_StreamType_ePes,           PES Stream
    **  BAVC_StreamType_eEs,            ES Stream
    */
    BAVC_StreamType OutputFormat;

    BXPT_Rave_EsRanges EsRanges[ BXPT_NUM_AV_RANGES ];

    /*
    ** The upper and lower bounds (non-inclusive) of the PES stream IDs that will be accepted
    ** during startcode generation.
    */
    uint8_t StreamIdHi;
    uint8_t StreamIdLo;

    /*
    ** Upper and lower thresholds. An interrupt to the CPU will be generated when data
    ** in the CDB when the exceeds the CdbUpperThreshold or drops below the CdbLowerThreshold.
    ** The ItbUpperThreshold and ItbLowerThreshold provide similar functions for ITB data.
    */
    uint16_t CdbUpperThreshold, CdbLowerThreshold;
    uint16_t ItbUpperThreshold, ItbLowerThreshold;

    /*
    ** BandHoldEn allows the AV context to pause a playback channel, if the decoder
    ** requires it. This functionality is similar to AV Pause in the older chipsets,
    ** except that it is implemented here in RAVE rather than the playback channels.
    */
    bool BandHoldEn;

    /* If true, do not check CC bits for packets on this context. */
    bool DisableContinuityCheck;

    /* If true, disable all packet errors in the ITB.Default value is enabled, should be enabled
       for debugging purposes only */
    bool DisablePacketErrors;

#if BXPT_HAS_BPP_SEARCH
    /* Enable searching for BPP packets. */
    bool EnableBPPSearch;
#endif

    /*
    ** For transcoding applications, we need to keep the emulation prevention bytes. Set this
    ** to true for transcoding only.
    */
    /* DEPRECATED: This feature is not used in any active project and has been deprecated. See
    ** SW7425-5829. The bool will remain here, but the implementation will ignore it.
    */
    bool Transcoding;

    /*
    ** Include PCR/RTS entries in the ITB. By default, this is false. Set to true for
    ** host operations such as software RAVE.
    */
    bool PcrRtsEntryEn;

    /*
    ** Upper and lower bounds for PES stream IDs that should be dropped. In some cases, the Stream
    ** ID range specified by StreamIdHi and StreamIdLo may include data you don't want. An example
    ** would be 0xBE, the PES stream used for stuffing bytes. By specifying 0xBE in both PesSidExcludeHi
    ** and PesSidExcludeLo, the stuffing bytes won't be captured into the CDB.
    */
    uint8_t PesSidExcludeHi;
    uint8_t PesSidExcludeLo;

    /* PR57627 :
    ** SC_OR_MODE is used to select the way scramble control bits are reported.
    ** 0 = Disable OR-ing of current and previous scramble control bits (Default).
    ** 1 = Enable OR-ing of current and previous scramble control bits. This is to
    ** support streams which have mixture of scrambled and unscrambled packets within
    ** the same PID. In such case, these PIDs will be treated as scramble PIDs.
    ** By default this is disabled.
    */
    bool ScOrMode;      /* Default value is true, per hw guys recommendation. */
}
BXPT_Rave_AvSettings;

/***************************************************************************
Summary:
The parse table entry consists of enables for each bitfield in the transport
packet that can be parsed. In most cases, there is also a value that the
bitfield must match in order to have an index generated.
****************************************************************************/
typedef struct BXPT_Rave_TpitEntry
{
    unsigned Pid;               /* The PID to build TPIT entries for. */
    bool CorruptionEnable;      /* Corrupt packet if it matches the filter criterion below. Disabled by default */
    bool MpegMode;              /* The type of transport we are recording. Set to true if MPEG, false if DirecTV */

    union
    {
        struct
        {
            bool EcmPolarityChangeEn;
            bool SectionFilterEn;

            bool AdaptationExtensionFlagEnable;         /* Store an index table entry if this bit is set and the adaptation_field_extension_flag matches the corresponding compare value. */
            bool AdaptationExtensionFlagCompValue;      /* The value to compare adaptation_field_extension_flag with. */

            bool PrivateDataFlagEnable;                 /* Store an index table entry if this bit is set and the transport_private_data_flag matches the corresponding compare value. */
            bool PrivateDataFlagCompValue;              /* The value to compare transport_private_data_flag with. */

            bool SplicingPointFlagEnable;               /* Store an index table entry if this bit is set and the splicing_point_flag matches SplicingPointFlagCompValue */
            bool SplicingPointFlagCompValue;            /* The value to compare splicing_point_flag with. */

            bool OpcrFlagEnable;                        /* Store an index table entry if this bit is set and the OPCR_flag  matches the corresponding compare value */
            bool OpcrFlagCompValue;                     /* The value to compare OPCR_flag with. */

            bool PcrFlagEnable;                         /* Store an index table entry if this bit is set and the PCR_flag  matches the corresponding compare value */
            bool PcrFlagCompValue;                      /* The value to compare PCR_flag with. */

            bool EsPriorityIndicatorEnable;             /* Store an index table entry if this bit is set and the elementary_stream_priority_indicator matches the corresponding compare value */
            bool EsPriorityIndicatorCompValue;          /* The value to compare elementary_stream_priority_indicator with. */

            bool RandomAccessIndicatorEnable;           /* Store an index table entry if this bit is set and the random_access_indicator matches the corresponding compare value */
            bool RandomAccessIndicatorCompValue;        /* The value to compare random_access_indicator with. */

            bool DiscontinuityIndicatorEnable;          /* Store an index table entry if this bit is set and the discontinuity_indicator matches the corresponding compare value */
            bool DiscontinuityIndicatorCompValue;       /* The value to compare discontinuity_indicator with. */

            bool AdaptationFieldChangeEnable;           /* Store an index table entry if a change is detected in the adaptation field control field (enable only) */
            bool ScramblingControlChangeEnable;         /* Store an index table entry if a change is detected in the scrambling control field control field (enable only) */

            bool TransportPriorityEnable;               /* Store an index table entry if this bit is set and the transport_priority matches the corresponding compare value */
            bool TransportPriorityCompValue;            /* The value to compare transport_priority with. */

            bool PayloadUnitStartEnable;                /* Store an index table entry if this bit is set and the payload_unit_start_indicator matches the corresponding compare value */
            bool PayloadUnitStartCompValue;             /* The value to compare payload_unit_start_indicator with. */
        }
        Mpeg;

        struct
        {
            bool FilterHdEnable;        /* Enable HD filtering for this PID. */
            unsigned short HdFilter;    /* The HD value to filter on. Transport packet matching this value are NOT recorded. */

            bool TcDetEn;           /* Store an index table entry if a valid timecode is contained in the packet (enable only) */

            bool CwpDetEn;          /* Store an index table entry if a valid CWP is contained in the packet (enable only) */

            bool RtsDetEn;          /* Store an index table entry if a valid RTS is contained in the packet (enable only) */

            bool CffEn;             /* Store an index table entry if this bit is set and the current field flag matches the corresponding compare value */
            bool CffComp;           /* The current field flag must match this value if the corresponding enable bit is set in order for an index table entry to be made */

            bool MfEn;              /* Store an index table entry if this bit is set and the modifiable flag matches the corresponding compare value */
            bool MfComp;            /* The modifiable flag must match this value if the corresponding enable bit is set in order for an index table entry to be made */

            bool HdEn;              /* Store an index table entry for any bit set in this field and the respective compare bit matches the
                                        respective header designator bit (only for valid HD i.e. unencrypted packets) */
            uint8_t HdMask;         /* The header designator bits must bitwise match this value if the corresponding enable bit is set in order for an index table entry to be made.
                                        (only for valid HD i.e. unencrypted packets) */

            bool CsAuxChangeEn;     /* Store an index table entry if a change is detected in the control sync for CWP packets (enable only) */

            bool CsChangeEn;        /* Store an index table entry if a change is detected in the control sync for content (non AUX)packets (enable only) */

            bool CfChangeEn;        /* Store an index table entry if a change is detected in the control flag for content (non AUX)packets (enable only) */

            bool BbEn;              /* Store an index table entry if this bit is set and the bundle boundary matches the corresponding compare value */
            bool BbComp;            /* The bundle boundary must match this value if the corresponding enable bit is set in order for an index table entry to be made */
        }
        DirecTv;
    }
    Cfg;
}
BXPT_Rave_TpitEntry;

/***************************************************************************
Summary:
Settings for the Record contexts. Used by BXPT_Rave_SetScdEntry() and
BXPT_Rave_SetScdUsingPid(). NOTE: When used by BXPT_Rave_SetScdUsingPid(),
the PidChannel member is ignored.
****************************************************************************/
typedef struct BXPT_Rave_ScdEntry
{
    unsigned PidChannel;    /* The PID channel caryring the PIDs to build startcode entries for. */
    bool ExtractPts;        /* Extract PTS values from the stream and add them to the SCD tables. */
    bool ExtractDts;        /* Extract DTS values from the stream and add them to the SCD tables. */

    /*
    ** Number of bytes, including the startcode, to capture.
    ** Min is BXPT_MIN_ES_COUNT, max is BXPT_MAX_ES_COUNT
    */
    uint8_t EsCount;
}
BXPT_Rave_ScdEntry;

/***************************************************************************
Summary:
Global settings for startcode and transport index table generation.
****************************************************************************/
typedef struct
{
    uint8_t RangeHi;        /* Upper limit (non-inclusive) of the startcode range */
    uint8_t RangeLo;        /* Lower limit (non-inclusive) of the startcode range. */
    bool RangeIsASlice;     /* true if this range of startcodes is for a slice */
    bool RangeEnable;       /* Startcodes for this range are enabled for index table entry. */
}
IndexerScRange;

typedef struct BXPT_Rave_IndexerSettings
{
    union
    {
        /* Settings for Transport Parser Index table generation. Used by all filters on a given record context. */
        struct
        {
            bool FirstPacketEnable;     /* Store an index table entry for the first packet recorded. */
            bool StorePcrMsb;           /* Store the 32 MSB of the PCR if true; store the 32 LSB if false (default) */

            /*
            ** The TPIT block can corrupt the contents of packets that match Pid above, when CorruptionEnable is true.
            ** Corruption is enabled on a per-PID basis, by setting BXPT_Rave_TpitEntry.CorruptionEnable = true .
            */
            uint8_t CorruptionByte;         /* Byte to use in corruption. */
            unsigned int CorruptionStart;   /* The start offset for the range of bytes within the packet to corrupt (0 is the first offset in the packet). */
            unsigned int CorruptionEnd;     /* The end offset for the range of bytes within the packet to corrupt */

            bool IdleEventEnable;       /* Store an event if no other events have occurred for TpitEventTimeout number of 108 MHz clocks. */
            bool RecordEventEnable;     /* Store an event if no packets have been recorded for TpitEventTimeout number of 108 MHz clocks. */

#if BXPT_HAS_TIMER_TICK
            bool TimeTickEn;
#endif
        }
        Tpit;

        /* Settings for Startcode detection. */
        struct
        {
            bool Atsc01IsUnscrambled;       /* If the packet's scrambling control field == 01b, it is treated as unscrambled if Atsc01IsUnscrambled = true. */
            bool ParseScramblingControl;    /* If true, skip packet if scrambling control bits are non-zero. */

            /* Startcode ranges that the SCDs can filter on. Only codes between the defined ranges will appear in the tables. */
            IndexerScRange ScRange[ BXPT_MAX_STARTCODE_RANGES ];

#if BXPT_HAS_SVC_MVC
            bool SvcMvcMode;    /* Configure for indexing an SVC or MVC stream. Default is false. */
#endif
        }
        Scd;
    }
    Cfg;
}
BXPT_Rave_IndexerSettings;

/***************************************************************************
Summary:
Pointers to new context data and the number of new bytes. The data is stored
in a hardware ring buffer, so wrap around can and will occur. The wrap
pointers will be non-zero if a wrap around occurred.
****************************************************************************/
typedef struct BXPT_Rave_DataPtrs
{
    uint8_t *DataPtr;       /* Start of new data, before wrap around */
    size_t ByteCount;       /* Number of new bytes, before wrap around */

    uint8_t *WrapDataPtr;   /* Start of new data, after wrap around. NULL if no wrap occurred */
    size_t WrapByteCount;   /* Number of new bytes, after wrap around. 0 if no wrap. */
}
BXPT_Rave_DataPtrs;

/***************************************************************************
Summary:
Pointers to new context ITB and CDB data.
****************************************************************************/
typedef struct BXPT_Rave_ContextPtrs
{
    BXPT_Rave_DataPtrs Cdb, Itb;
}
BXPT_Rave_ContextPtrs;

/***************************************************************************
Summary:
Statistics for a record context.
****************************************************************************/
typedef struct BXPT_Rave_RecordStats
{
    uint32_t ElapsedTime;   /* 1.26 uSec ticks since record started. */
    uint32_t ByteCount;     /* Number of bytes recorded. */
}
BXPT_Rave_RecordStats;

/***************************************************************************
Summary:
CDB depth and size information.
****************************************************************************/
typedef struct BXPT_Rave_BufferInfo
{
    size_t CdbSize;
    size_t CdbDepth;
}
BXPT_Rave_BufferInfo;

/***************************************************************************
Summary:
Miscellaneous status for a given context.
****************************************************************************/
typedef struct BXPT_RaveCx_Status
{
    bool ContextEnabled;    /* true if context is currently enabled, false otherwise */
    bool CdbOverflow;       /* CDB is in an overflow condition. */
    bool ItbOverflow;       /* ITB is in an overflow condition. */
}
BXPT_RaveCx_Status;

/***************************************************************************
Summary:
Upper and lower thresholds. An interrupt to the CPU will be generated when data
in the CDB when the exceeds the CdbUpperThreshold or drops below the CdbLowerThreshold.
The ItbUpperThreshold and ItbLowerThreshold provide similar functions for ITB data.
NOTE: These tresholds are specified in units of 256 bytes. Thus, to set a threshold
to 1024 bytes, the treshold param must be set to 4.
****************************************************************************/
typedef struct BXPT_Rave_ContextThresholds
{
    uint32_t CdbUpper;
    uint32_t CdbLower;
    uint32_t ItbUpper;
    uint32_t ItbLower;
}
BXPT_Rave_ContextThresholds;

#if BXPT_HAS_RAVE_SCRAMBLING_CONTROL
/******************************************************************************
Summary;
Scrambling Control Monitor. This register is used to monitor SC bits(Scrambling Control bits)
from Transport packet. HOST MIPS should write 0 to this register during channel change.
******************************************************************************/
typedef struct BXPT_Rave_ScrambleCtrl
{
    bool PusiValid;    /* SC_PUSI_VALID is set if the transport packet has PUSI set */
    uint8_t Pusi;      /* SC_PUSI bits are updated only if transport packet has PUSI set. */
    bool AllValid;     /* SC_ALL_VALID is set when RAVE receives any Transport packet on corresponding context. */
    uint8_t ScAll;     /* SC_ALL indicates the scrambling control bits in Transport packet. */
}
BXPT_Rave_ScrambleCtrl;

/*****************************************************************************
Summary:
Return the Scrambling Control bits.

Description:
For the given Context, return the Scrambing Control Monitor values.

Returns:
    BERR_SUCCESS                - Retrieved channel count.
    BERR_INVALID_PARAMETER      - Bad input parameter
*****************************************************************************/
BERR_Code BXPT_Rave_GetScramblingCtrl(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ScrambleCtrl *ScrambleCtrl
    );

/*****************************************************************************
Summary:
Clears the Scrambling Control Register for given context.

Description:
For the given Context, clear the Scrambing Control register.

Returns:
    BERR_SUCCESS                - Retrieved channel count.
    BERR_INVALID_PARAMETER      - Bad input parameter
*****************************************************************************/
BERR_Code BXPT_Rave_ClearSCRegister(
    BXPT_RaveCx_Handle hCtx
    );
#endif

/***************************************************************************
Summary:
Return the number of RAVE channels.

Description:
For the given transport core, return the number of RAVE channels that are
instantiated.

Returns:
    BERR_SUCCESS                - Retrieved channel count.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] Handle for this transport instance. */
    unsigned *TotalChannels     /* [out] The number of RAVE channels supported */
    );

/***************************************************************************
Summary:
Return the default settings for the RAVE channel.

Description:
The RAVE channel has certain default settings which the user can override
when the device is opened. This function returns those defaults in a structure
that can be changed, and then passed to BXPT_Rave_OpenChannel().

Returns:
    BERR_SUCCESS                - Returned the default settings.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Rave_OpenChannel
****************************************************************************/
BERR_Code BXPT_Rave_GetChannelDefaultSettings(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport instance */
    unsigned ChannelNo,                         /* [in] Which RAVE instance to get defaults for */
    BXPT_Rave_ChannelSettings *RaveDefSettings  /* [out] The defaults. */
    );

/***************************************************************************
Summary:
Open the given RAVE channel.

Description:
Reset the given RAVE channel, configure it using the given defaults, then
return a handle to access that channel.

Returns:
    BERR_SUCCESS                    - RAVE channel opened.
    BXPT_ERR_NO_AVAILABLE_RESOURCES - Invalid ChannelNo.
    BERR_OUT_OF_SYSTEM_MEMORY       - Not enough RAM for the handle.
****************************************************************************/
BERR_Code BXPT_Rave_OpenChannel(
    BXPT_Handle hXpt,                                   /* [in] Handle for this transport instance */
    BXPT_Rave_Handle *hRave,                            /* [out] Handle for the RAVE channel */
    unsigned ChannelNo,                                 /* [in] Which RAVE channel to open */
    const BXPT_Rave_ChannelSettings *RaveDefSettings    /* [in] Default settings to use */
    );

/***************************************************************************
Summary:
Close a given RAVE channel.

Description:
Shutdown a given RAVE channel. Interrupts from it are disabled. All context
and indexer DRAM buffers are freed.

Returns:
    BERR_SUCCESS                - RAVE channel closed.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_CloseChannel(
    BXPT_Rave_Handle hRave     /* [in] Handle for this RAVE instance */
    );

/******************************************************************************
Summary:
Revision numbers for the RAVE's microsequencer firmware. Also a CRC of the
firmware image.
******************************************************************************/
typedef struct BXPT_Rave_RevisionInfo
{
    unsigned long fwRev;      /* Format: 0x00000220 -> ver 2.20 */
    unsigned long fwCrc;      /* 24-bit CRC. 8 MSBs are always 0. For example, 0x006EFAB0 */
}
BXPT_Rave_RevisionInfo;

void BXPT_Rave_GetFwRevisionInfo(
    BXPT_Rave_Handle hRave,                 /* [in] Handle for this RAVE channel */
    BXPT_Rave_RevisionInfo *versionInfo     /* [out] Version info */
    );

/***************************************************************************
Summary:
Allocate a RAVE context.

Description:
The RAVE AV and record contexts are allocated from logical pools. All contexts
within a pool are identical. This call allocates a context of ContextType.

For AV contexts, BufferCfg should have the buffer size needed by the decoder.
Aligment will be forced to 256 bytes CDB and 128 bytes for ITB. This is a
requirement from the decoders.

For record contexts, the buffer size should be large enough to accomodate
the host CPU's latency in servicing the context interrupts. The alignment
should be a multiple of 32-bit words.

Returns:
    BERR_SUCCESS                    - Context allocated successfully.
    BERR_INVALID_PARAMETER          - Bad ContextType parameter
    BXPT_ERR_NO_AVAILABLE_RESOURCES - The requested context is already in use.
****************************************************************************/
BERR_Code BXPT_Rave_AllocContext(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    BXPT_RaveCx RequestedType,      /* [in] The type of context to allcoate */
    const BAVC_CdbItbConfig *BufferCfg,   /* [in] Size and alignment for ITB and CDB */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    );

/***************************************************************************
Summary:
Allocate a RAVE context, using the given heap for CDB and ITB memory.

Description:
The RAVE AV and record contexts are allocated from logical pools. All contexts
within a pool are identical. This call allocates a context of ContextType.

For AV contexts, BufferCfg should have the buffer size needed by the decoder.
Aligment will be forced to 256 bytes CDB and 128 bytes for ITB. This is a
requirement from the decoders.

For record contexts, the buffer size should be large enough to accomodate
the host CPU's latency in servicing the context interrupts. The alignment
should be a multiple of 32-bit words.

The API allows the caller to specify a different BMEM heap from which the
CDB and ITB are allocated. For contexts of type BXPT_RaveCx_eAvR or
BXPT_RaveCx_eRecordR, the allocations will still use the RHeap defined
when BXPT_Open() is called.

Returns:
    BERR_SUCCESS                    - Context allocated successfully.
    BERR_INVALID_PARAMETER          - Bad ContextType parameter
    BXPT_ERR_NO_AVAILABLE_RESOURCES - The requested context is already in use.
****************************************************************************/
BERR_Code BXPT_Rave_AllocContextFromHeap(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    BXPT_RaveCx RequestedType,      /* [in] The type of context to allcoate */
    const BAVC_CdbItbConfig *BufferCfg,   /* [in] Size and alignment for ITB and CDB */
    BMEM_Handle ContextHeap,         /* [in] This context's memory heap handle */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    );

/***************************************************************************
Summary:
Enable or disable a RAVE context.

Description:
This call will enable the given audio/video or record context.

Returns:
    BERR_SUCCESS                    - Context allocated successfully.
    BERR_INVALID_PARAMETER          - Bad ContextType parameter
    BXPT_ERR_NO_AVAILABLE_RESOURCES - The requested context is already in use.
****************************************************************************/
BERR_Code BXPT_Rave_EnableContext(
    BXPT_RaveCx_Handle Context      /* [in] The context. */
    );

/***************************************************************************
Summary:
Disable a RAVE context.

Description:
This call will disable a given RAVE context.

Returns:
    BERR_SUCCESS                    - Context allocated successfully.
    BERR_INVALID_PARAMETER          - Bad ContextType parameter
****************************************************************************/
BERR_Code BXPT_Rave_DisableContext(
    BXPT_RaveCx_Handle Context      /* [in] The context. */
    );

/***************************************************************************
Summary:
Allocate a record channel indexer.

Description:
The RAVE currently supports automatic generation of index tables, which are
handy to have around for PVR applications. There are several types of indexers.
This function will allocated one (or more, if supported) indexer(s) of the
requested type and map it to the record channel given by RecordContextNum.

If multiple startcode indexers will be used on the same record, they must be
allocated at the same time. The NumIndexer argument is ignored when TPIT
indexers are requested, since only 1 TPIT may be mapped to a record.

Note that there are some restrictions:
Only 1 TPIT indexer may be allocated to record context.
At most 8 SCD indexers may be allocated to a record context.

Returns:
    BERR_SUCCESS                    - Context allocated successfully.
    BERR_INVALID_PARAMETER          - Bad ContextType parameter
    BXPT_ERR_NO_AVAILABLE_RESOURCES - Not enough resources to meet the request.
****************************************************************************/
BERR_Code BXPT_Rave_AllocIndexer(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    BXPT_RaveIdx IndexerType,       /* [in] Which type of indexer */
    unsigned NumIndexers,           /* [in] Number of indexers requested */
    BXPT_RaveCx_Handle Context,     /* [in] The record context that this indexer should be mapped to */
    BXPT_RaveIdx_Handle *Indexer    /* [out] Handle for the allocated indexer */
    );

/***************************************************************************
Summary:
Free a RAVE context.

Description:
Disable the given context, then free up any internal-allocated memory.

Returns:
    BERR_SUCCESS                    - Context freed successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_FreeContext(
    BXPT_RaveCx_Handle Context      /* [in] The context to free. */
    );

/***************************************************************************
Summary:
Free a record channel indexer.

Description:
Disable the given indexer, then free up any internal-allocated memory.

Returns:
    BERR_SUCCESS                    - Context freed successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_FreeIndexer(
    BXPT_RaveIdx_Handle Indexer     /* [in] The indexer to free. */
    );

/***************************************************************************
Summary:
Get the configuration for a record indexer.

Description:
The current hardware settings for the given indexer are returned.

Returns:
    BERR_SUCCESS                    - Configuration read successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetIndexerConfig(
    BXPT_RaveIdx_Handle Indexer,        /* [in] Handle for the allocated indexer */
    BXPT_Rave_IndexerSettings *Config   /* [out] The indexer settings */
    );

/***************************************************************************
Summary:
Set the configuration for a record indexer.

Description:
The current hardware settings for the given indexer are written to hardware.

Returns:
    BERR_SUCCESS                    - Configuration read successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetIndexerConfig(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    const BXPT_Rave_IndexerSettings *Config /* [in] The indexer settings */
    );

/***************************************************************************
Summary:
Set the filter criterion for the given startcode detector.

Description:
The startcode detector can produce tables when certain PIDs are seen in the
transport stream. This function sets the PID channel and other required settings. The
caller supplies the indexer handle that was returned by BXPT_Rave_AllocIndexer().
Since record contexts may have up to 8 separate startcode detectors, the
WhichScd argument is used to select which one to configure.

Returns:
    BERR_SUCCESS                    - Configuration loaded successfully.
    BERR_INVALID_PARAMETER          - Bad parameter

See also:
BXPT_Rave_SetScdUsingPid
****************************************************************************/
BERR_Code BXPT_Rave_SetScdEntry(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    unsigned WhichScd,                      /* [in] Which of the startcode detectors. */
    const BXPT_Rave_ScdEntry *ScdConfig     /* [in] The indexer settings */
    );

/***************************************************************************
Summary:
Set the filter criterion for the given TPIT generator.

Description:
The transport parser index table generator can produce tables when certain
flags are seen in the transport packets. This function sets the filtering
needed to recognize those flags.

Each record context may have at most 1 TPIT generator. However, each TPIT
can support independent filtering for up to 16 different PIDs.

Returns:
    BERR_SUCCESS                    - Configuration loaded successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetTpitFilter(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    unsigned WhichFilter,                   /* [in] Which of the 16 filters. */
    const BXPT_Rave_TpitEntry *Tpit         /* [in] The indexer settings */
    );

/***************************************************************************
Summary:
Set the ECM Odd and Even table IDs.

Description:
Set the ECM Odd and Even table IDs used by TPIT generation. These fields are
only used when BXPT_Rave_TpitEntry.Mpeg.EcmPolarityChangeEn is true. The TPIT
logic can filter on up to 3 Odd/Even pairs simultaneously, so the caller must
specify which pair of TIDs to set. Valid values for WhichPair are 1, 2, and 3.

Returns:
    BERR_SUCCESS                    - Configuration loaded successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetTpitEcms(
    BXPT_RaveIdx_Handle Indexer,        /* [in] Handle for the allocated indexer */
    unsigned WhichPair,                 /* [in] ECM TID pair 1, 2, or 3 */
    unsigned EvenEcmTid,                /* [in] Even ECM TID */
    unsigned OddEcmTid                  /* [in] Odd ECM TID */
    );

/***************************************************************************
Summary:
Return the addresses of the context regiseters needed by the RAVE clients.

Description:
Clients must directly access the context pointer registers in RAVE to update
them in a timely manner. This call will return the addresses of the pointer
registers.

Addresses are given as offsets from the start of HIF (Host Interface) block.
The addresses should not require modification before loading into hardware.

Returns:
    BERR_SUCCESS                    - Numbers returned through pointers.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetContextRegisters(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    BAVC_XptContextMap *Map         /* [out] Channel info this context uses. */
    );

/***************************************************************************
Summary:
Get the record context configuration.

Description:
The current configuration of the given record context is returned.

Returns:
    BERR_SUCCESS                    - Configuration read successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetRecordConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_RecordSettings *Config    /* [out] The record settings. */
    );

/***************************************************************************
Summary:
Get the record context configuration.

Description:
The current configuration of the given record context is returned.

Returns:
    BERR_SUCCESS                    - Configuration read successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetRecordConfig(
    BXPT_RaveCx_Handle Context,                     /* [in] The context  */
    const BXPT_Rave_RecordSettings *RecConfig       /* [in] The record settings. */
    );

/***************************************************************************
Summary:
Get the AV context configuration.

Description:
The current configuration of the given AV context is returned.

Returns:
    BERR_SUCCESS                    - Configuration read successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetAvConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_AvSettings *Config    /* [out] The AV settings. */
    );

/***************************************************************************
Summary:
Set the AV context configuration.

Description:
The given configuration is loaded into the AV context.

Returns:
    BERR_SUCCESS                    - Configuration written successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetAvConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    const BXPT_Rave_AvSettings *Config  /* [out] The AV settings. */
    );

/***************************************************************************
Summary:
Return the size and starting address of new data in the record context.

Description:
If new data has been captured into the record context, this call will return
the location of the data and the number of bytes. The data is stored in a
hardware ring buffer, so wrap around conditions can and will occur.

The BXPT_Rave_DataPtrs contains two sets of pointers and byte counts. The
first is used for data written prior to the wrap around. The second set,
along with the wrap around address, is used if the data did wrap. The second
pointer set and the wrap address will be 0 if no wrap around happened.

NOTE: If either byte count is 0, meaning no new data in the buffer, the
corrosponding pointer will be set to NULL.

Returns:
    BERR_SUCCESS                    - Returned size and starting address successfully.
    BERR_INVALID_PARAMETER          - Bad parameter

See Also:
BXPT_Rave_UpdateReadOffset
****************************************************************************/
BERR_Code BXPT_Rave_CheckBuffer(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_ContextPtrs *Ptrs         /* [out] Pointers to the buffer data */
    );

/***************************************************************************
Summary:
Update the record context's read pointer.

Description:
The read pointer is used to tell the context hardware what memory in the buffer
has been read by the caller. The caller sets *ByteCount to the number of
bytes read since the last time this function was called.

Returns:
    BERR_SUCCESS                    - Read pointer updated.
    BERR_INVALID_PARAMETER          - Bad parameter

See Also:
BXPT_Rave_CheckBuffer
****************************************************************************/
BERR_Code BXPT_Rave_UpdateReadOffset(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    size_t CdbByteCount,            /* [in] Number of CDB bytes read. */
    size_t ItbByteCount             /* [in] Number of ITB bytes read. */
    );

/***************************************************************************
Summary:
Add a PID channel to a context.

Description:
The contexts accept data from PID channels. This call adds a PID channel to
a context. Note that AV contexts may have no more than 1 PID channel, whereas
record contexts may have several.

Returns:
    BERR_SUCCESS                - Channel added successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Rave_RemovePidChannel
****************************************************************************/
BERR_Code BXPT_Rave_AddPidChannel(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum,        /* [in] Which PID channel to add. */
    bool UseDecrypted               /* [in] Use decrypted versions of packets on this channel */
    );


/***************************************************************************
Summary:
Add a BPP PID channel to a context.

Description:
The contexts accept data from PID channels. This call adds a PID channel to
a context. Note that AV contexts may have no more than 1 PID channel, whereas
record contexts may have several.

Returns:
    BERR_SUCCESS                - Channel added successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Rave_RemovePidChannel
****************************************************************************/
BERR_Code BXPT_Rave_AddBppChannel(
    BXPT_RaveCx_Handle Context,     /* [in] The context  */
    unsigned int PidChanNum,        /* [in] Which PID channel to add. */
    bool UseDecrypted               /* [in] Use decrypted versions of packets on this channel */
    );


/***************************************************************************
Summary:
Remove a All PID channels from a context.

Description:
The contexts accept data from PID channels. This call removes all PID channels
from a context. Record contexts may have several PID channels for same context.

Returns:
    BERR_SUCCESS                - All Pid Channels removed successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Rave_AddPidChannel
****************************************************************************/
BERR_Code BXPT_Rave_RemoveAllPidChannel(
    BXPT_RaveCx_Handle Context         /* [in] The context  */
    );

/***************************************************************************
Summary:
Remove a PID channel from a context.

Description:
The contexts accept data from PID channels. This call removes a PID channel from
a context. Note that AV contexts may have no more than 1 PID channel, whereas
record contexts may have several.

Returns:
    BERR_SUCCESS                - Channel removed successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_Rave_AddPidChannel
****************************************************************************/
BERR_Code BXPT_Rave_RemovePidChannel(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    unsigned int PidChanNum         /* [in] Which PID channel to remove. */
    );

/***************************************************************************
Summary:
Set the user bits for 30-bit-format timestamp.

Description:
In some configurations, the upper 2 bits of the packet's recorded timestamp
may be user-programmable. This function will set the upper 2 bits of the
timestamp prepended to the packet by the record engine. The 2 least
significant bits of the Bits argument are used as the new value of the user
programmable timestamp bits.

Returns:
    BERR_SUCCESS                - New user bits have been set.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetTimestampUserBits(
    BXPT_RaveCx_Handle Context,         /* [in] The record context  */
    unsigned int Bits                   /* [in] The new value for the user bits. */
    );

/***************************************************************************
Summary:
Return some statistics for the given record context.

Description:
The following information about the record context is returned:

- The elapsed time since the record was enabled, expressed in 1.26 uSec ticks
- Number of bytes recorded since the record context was enabled.

Returns:
    BERR_SUCCESS                - Obtained record count successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetRecordStats(
    BXPT_RaveCx_Handle Context,         /* [in] The record context  */
    BXPT_Rave_RecordStats *Stats        /* [out] Record context statistics. */
    );

/***************************************************************************
Summary:
Get CDB size and depth.

Description:
Return the size and depth of the CDB associated with the given handle.

Returns:
    BERR_SUCCESS                    - Valid info returned in BufferInfo.
    BERR_INVALID_PARAMETER          - Bad Context parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetBufferInfo_isrsafe(
    BXPT_RaveCx_Handle hCtx,            /* [in] The context. */
    BXPT_Rave_BufferInfo *BufferInfo    /* [out] CDB depth and size */
    );

#define BXPT_Rave_GetBufferInfo         BXPT_Rave_GetBufferInfo_isrsafe
#define BXPT_Rave_GetBufferInfo_isr     BXPT_Rave_GetBufferInfo_isrsafe

/***************************************************************************
Summary:
Return BINT_Id for a given RAVE interrupt.

Description:
Generate the correct BINT_Id for a context interrupt. The Name argument indicates
which of the context's interrupts is needed. The result is a BINT_Id value suitable
for use in BINT_CreateCallback().

Returns:
    BERR_SUCCESS                - Id generated successfully.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetIntId(
    BXPT_RaveCx_Handle ThisCtx,
    BXPT_RaveIntName Name,
    BINT_Id *IntId
    );

BERR_Code BXPT_Rave_PushPidChannel(
    BXPT_RaveCx_Handle hCtx,        /* [in] The context. */
    unsigned int PidChannel,        /* [in] Current PID Channel. */
    unsigned int SplicePidChannel   /* [in] Channel carrying the PID to splice. */
    );

BERR_Code BXPT_Rave_ClearQueue(
    BXPT_RaveCx_Handle hCtx         /* [in] The context. */
    );

unsigned BXPT_Rave_GetQueueDepth(
    BXPT_RaveCx_Handle hCtx         /* [in] The context. */
    );

/***************************************************************************
Summary:
Return context status.

Description:
Get miscellaneous status values for the given context.

Returns:
    BERR_SUCCESS                    - Valid info returned in CxStatus.
    BERR_INVALID_PARAMETER          - Bad Context parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetContextStatus(
    BXPT_RaveCx_Handle hCtx,
    BXPT_RaveCx_Status *CxStatus
    );

/***************************************************************************
Summary:
Flush the CDB and ITB.

Description:
Clear the compressed data and index table buffers associated with the given
context. This is done by updating the CDB and ITB READ pointers to match
their respective VALID pointers.

Returns:
    BERR_SUCCESS                    - Flush successful.
    BERR_INVALID_PARAMETER          - Bad Context parameter
****************************************************************************/
BERR_Code BXPT_Rave_FlushContext(
    BXPT_RaveCx_Handle hCtx
    );

/***************************************************************************
Summary:
Reset the context.

Description:
Reset the context to it orignal reset state, same state as after its allocated
by BXPT_Rave_AllocContext. In order to use the context, it must be
reconfigured using  BXPT_Rave_SetRecordConfig,BXPT_Rave_SetAvConfig.
This function should used when frequent allocation/deallocation of RAVE
context is not desirable. This function should not be used to resuse AV context
as Record and vice versa.

Returns:
    BERR_SUCCESS                    - Flush successful.
    BERR_INVALID_PARAMETER          - Bad Context parameter
****************************************************************************/
BERR_Code BXPT_Rave_ResetContext(
    BXPT_RaveCx_Handle hCtx
    );

/***************************************************************************
Summary:
Get picture count from an AV context.

Description:
Return the number of pictures in the context's CDB. This call will return 0
if called for a record context, or if there is no picture counter assigned
to the given context.

Returns:
    BERR_SUCCESS                    - Obtained a picture count.
    BERR_INVALID_PARAMETER          - Bad Context parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetPictureCount(
    BXPT_RaveCx_Handle hCtx,
    unsigned *PictureCount
    );

/***************************************************************************
Summary:
Return the default CDB and ITB thresholds for the given context.

Description:
Return the default CDB and ITB thresholds for the given context.

Returns:
    BERR_SUCCESS                - Retrieved thresholds.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetDefaultThresholds(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ContextThresholds *Thresholds
    );

/***************************************************************************
Summary:
Return the CDB and ITB thrasholds for the given CDB and ITB lengths.

Description:
Return the CDB and ITB thrasholds for the given CDB and ITB lengths. If a size
of 0 passed in length, the default values for the corrosponding threshold is
returned.

Returns:
    BERR_SUCCESS                - Retrieved thresholds.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_ComputeThresholds(
    BXPT_RaveCx_Handle hCtx,
    size_t CdbLength,
    size_t ItbLength,
    BXPT_Rave_ContextThresholds *Thresholds
    );

/***************************************************************************
Summary:
Return the current CDB and ITB thresholds for the given context.

Description:
Return the current CDB and ITB thresholds for the given context.

Returns:
    BERR_SUCCESS                - Retrieved thresholds.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetThresholds(
    BXPT_RaveCx_Handle hCtx,
    BXPT_Rave_ContextThresholds *Thresholds
    );

/***************************************************************************
Summary:
Set the current CDB and ITB thresolds for the given context.

Description:
Set the current CDB and ITB thresolds for the given context.

Returns:
    BERR_SUCCESS                - Retrieved thresholds.
    BERR_INVALID_PARAMETER      - Bad input parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetThresholds(
    BXPT_RaveCx_Handle hCtx,
    const BXPT_Rave_ContextThresholds *Thresholds
    );

/***************************************************************************
Summary:
Set the filter criterion, including the PID, for the given startcode detector.

Description:
The startcode detector can produce tables when certain PIDs are seen in the
transport stream. This function sets the PID and other required settings. The
caller supplies the indexer handle that was returned by BXPT_Rave_AllocIndexer().

This API allows the caller to specify the PID to build startcodes for, rather
than the PID channel. When operating in this mode, record contexts may have up
to 4 separate startcode detectors, instead of the usual 8. The PidChannel
member of ScdConfig is ignored.

Returns:
    BERR_SUCCESS                    - Configuration loaded successfully.
    BERR_INVALID_PARAMETER          - Bad parameter

See also:
BXPT_Rave_SetScdEntry
****************************************************************************/
BERR_Code BXPT_Rave_SetScdUsingPid(
    BXPT_RaveIdx_Handle Indexer,            /* [in] Handle for the allocated indexer */
    unsigned WhichScd,                      /* [in] Which of the startcode detectors. */
    unsigned Pid,                           /* [in] Which PID startcodes will be built for */
    const BXPT_Rave_ScdEntry *ScdConfig     /* [in] The indexer settings */
    );

typedef enum BXPT_Rave_PesSidExtMode
{
    BXPT_Rave_PesSidExtNormal = 0,             /* Don't selectively drop packets based on stream ID extension values */
    BXPT_Rave_PesSidExtIndependent = 1,        /* Keep only independent substream. */
    BXPT_Rave_PesSidExtDependent =   2,        /* Keep only dependent substream */
    BXPT_Rave_PesSidExtKeepAll     = 3         /* Keep everything in stream and place match bits in ITB Address entry */

}BXPT_Rave_PesSidExtMode;

typedef enum BXPT_Rave_PesExtSearchMode
{
    BXPT_Rave_PesExtSearchNone =0x00,           /* Do not search for PES extension or stream ID extension */
    BXPT_Rave_PesExtSearchSpecial =0x01,        /* Search for extension and PES stream ID extension only when PES stream ID = 0xFD */
    BXPT_Rave_PesExtSearchAlways = 0x02         /* Always search for extension and PES stream ID extension (flags must be set in PES header) */
}BXPT_Rave_PesExtSearchMode;

typedef struct BXPT_Rave_ContextSettings
{
    /* Context configuration setting to
       be used with  HD-DVD,PES and DVD playback */

    bool    EnableBppSearch;         /* Enable/Disable BPP search,BPP is added by packetizer during PES and DVD PB modes */
    uint8_t AudFrameInfo;            /* Value of 0-14, indicates number of bytes of data to be removed after the end of PES header,
                                        Value of 15, indicates the length of audio private header is inside the data
                                        HW uses this length to determine the number of bytes to remove after PES header. */

    bool    EnablePrivateHdrItbEntry; /* If AudFrameInfo above is > 0 and this field is set to true then stripped out
                                        private header is placed as an ITB entry */
    unsigned FilterPidStreamId;     /* This value is used to filter packets (either transport or PES) in order to determine
                                    which context they belong to. It should not be confused with the CX_Stream_ID_Range
                                    register, which specifies a range of stream IDs used only with transport-based format to
                                    determine the beginning of PES headers within a context.*/

    /* Context configuration setting to
       be used with BlueRay playback */

    bool    EnableCpChangeDetect;    /* Enable/Disable Copy Protection bit change detection */

    BXPT_Rave_PesSidExtMode PesSidExtMode;
    BXPT_Rave_PesExtSearchMode PesExtSearchMode;
    uint8_t SidExtDependent;         /* Dependent Stream ID extension value */
    uint8_t SidExtIndependent;       /* Independent Stream ID extension value */
}
BXPT_Rave_ContextSettings;

BERR_Code BXPT_Rave_GetContextDefaultSettings(
    BXPT_Rave_ContextSettings *ContextDefSettings   /* [out] The defaults. */
    );

/***************************************************************************
Summary:
Get the context configuration.

Description:
The current configuration of the given context is returned.

Returns:
    BERR_SUCCESS                    - Configuration read successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_GetContextConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    BXPT_Rave_ContextSettings *Config   /* [out] The AV settings. */
    );

/***************************************************************************
Summary:
Set the context configuration.

Description:
The given configuration is loaded into the context.

Returns:
    BERR_SUCCESS                    - Configuration written successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_SetContextConfig(
    BXPT_RaveCx_Handle Context,         /* [in] The context  */
    const BXPT_Rave_ContextSettings *Config /* [out] The AV settings. */
    );

/***************************************************************************
Summary:
AV over IP configuration settings. Done on a per-context basis.
****************************************************************************/
typedef struct BXPT_Rave_IpConfig
{
    uint16_t IpHeaderChecksum;      /* Initial checksum value for each IP packet */
    uint8_t IpHeaderLength;         /* Number of bytes skipped in the beginning of each IP packet */
    uint8_t NumTsPackets;           /* Number of transport packets in each IP packet */
    uint16_t SequenceNumIncrement;  /* Added to the sequence_number field of each IP packet */
    uint32_t CurrentSequenceNum;    /* Initial value of sequence_num. */
    bool IsMpegTs;                  /* True if TS packets are MPEG format, false otherwise */
}
BXPT_Rave_IpConfig;

BERR_Code BXPT_Rave_GetIpConfig(
    BXPT_RaveCx_Handle hCtx,        /* [in] Handle for the IP context */
    BXPT_Rave_IpConfig *IpConfig    /* [out] The IP config params */
    );

BERR_Code BXPT_Rave_SetIpConfig(
    BXPT_RaveCx_Handle hCtx,            /* [in] Handle for the IP context */
    const BXPT_Rave_IpConfig *IpConfig  /* [out] The IP config params */
    );

/***************************************************************************
Summary:
Return a pointer to the base of the context's CDB.

Description:
Returns a CPU-usable pointer to the start of the context's CDB. Or NULL if
an error occurred.

Returns:
    uint8_t * - Pointer to the start of the CDB
    NULL - An internal error occured.
****************************************************************************/
uint8_t *BXPT_Rave_GetCdbBasePtr(
    BXPT_RaveCx_Handle hCtx
    );

/***************************************************************************
Summary: Soft RAVE support in the XPT PI.

The XPT PI now incorporates the soft RAVE functions from bsettop_base_rave.c
with some additional features:

- 2 or more soft RAVE contexts can run concurrently.
- The soft context may be reset, returning the state variables to their default
values. It isn't necessary to free the soft context and then reallocate to
achieve a full reset.

Only a subset of the total RAVE PI is supported by the soft contexts. The
supported calls are:

BXPT_Rave_FreeContext() - Free all memory allocated by the soft context.
BXPT_Rave_FlushContext() - Clear the soft context's CDB and ITB.
****************************************************************************/

/***************************************************************************
Summary:
Enumeration of all supported soft RAVE modes.
****************************************************************************/
typedef enum BXPT_RaveSoftMode
{
    BXPT_RaveSoftMode_ePointersOnly,    /* don't copy ITB, just manage ITB pointers */
    BXPT_RaveSoftMode_eCopyItb,         /* copy ITB, but no transform */
    BXPT_RaveSoftMode_eVc1SimpleMain,   /* copy ITB and transform for VC1 simple/main */
    BXPT_RaveSoftMode_eDivX,            /* copy ITB and transform for DivX 5 */
    BXPT_RaveSoftMode_eDivX_noReorder,  /* copy ITB and transform for DivX 5, but do not reorder timestamps */
    BXPT_RaveSoftMode_eDivX_311,        /* copy ITB and transform for DivX 3.11 */
    BXPT_RaveSoftMode_eIframeTrick,     /* I-Frame trick mode */
    BXPT_RaveSoftMode_eIndexOnlyRecord,  /* CDB is shared, but ITB is not. */
    BXPT_RaveSoftMode_eDynamic_Splice,      /* CDB is shared, but ITB is not. */
    BXPT_RaveSoftMode_eMax              /* End of the enum list */
}
BXPT_RaveSoftMode;

/***************************************************************************
Summary:
Allocate a soft RAVE context.

Description:
Allocate a software-RAVE AV context, using the CDB from the SrcContext. A context
handle suitable for use is returned in DestContext. Note that startcode
RAVE API calls will not work on the DestContext.

This call replaces bsettop_p_setup_sw_rave().

Returns:
    BERR_SUCCESS                    - Soft RAVE context allocated.
    BXPT_ERR_NO_AVAILABLE_RESOURCES - Not enough RAVE contexts remaining to allocate.
    BERR_OUT_OF_SYSTEM_MEMORY       - Not enough DRAM for the DestContext ITB.
****************************************************************************/
BERR_Code BXPT_Rave_AllocSoftContext(
    BXPT_RaveCx_Handle SrcContext,      /* [in] The source context */
    BXPT_RaveSoftMode DestContextMode,  /* [in] The type of data that the destination should generate. */
    BXPT_RaveCx_Handle *DestContext     /* [out] The destination (soft) context */
    );

/***************************************************************************
Summary:
Allocate a soft RAVE context, using the specified heap for the ITB.

Description:
Allocate a software-RAVE AV context, using the CDB from the SrcContext. A context
handle suitable for use is returned in DestContext. Note that startcode
RAVE API calls will not work on the DestContext.

This call replaces bsettop_p_setup_sw_rave().

Returns:
    BERR_SUCCESS                    - Soft RAVE context allocated.
    BXPT_ERR_NO_AVAILABLE_RESOURCES - Not enough RAVE contexts remaining to allocate.
    BERR_OUT_OF_SYSTEM_MEMORY       - Not enough DRAM for the DestContext ITB.
****************************************************************************/
BERR_Code BXPT_Rave_AllocSoftContextFromHeap(
    BXPT_RaveCx_Handle SrcContext,      /* [in] The source context */
    BXPT_RaveSoftMode DestContextMode,  /* [in] The type of data that the destination should generate. */
    BMEM_Handle ContextHeap,            /* [in] The soft-context's memory heap handle, or NULL */
    BXPT_RaveCx_Handle *DestContext     /* [out] The destination (soft) context */
    );

/******************************************************************************
Summary;
Settings passed to BXPT_Rave_AllocCx.
******************************************************************************/
typedef struct BXPT_Rave_AllocCxSettings
{
    BXPT_RaveCx RequestedType;
    BAVC_CdbItbConfig BufferCfg;

    BMMA_Block_Handle CdbBlock, ItbBlock;
    unsigned CdbBlockOffset, ItbBlockOffset; /* offset into BMMA_Block_Handle */

    BXPT_RaveCx_Handle SoftRaveSrcCx;   /* If non-NULL, a soft-rave context is allocated and paired with this source context */
    BXPT_RaveSoftMode SoftRaveMode; /* soft-rave mode of destination context. used only if SoftRaveSrcCx is not-null */
} BXPT_Rave_AllocCxSettings;

/***************************************************************************
Summary:
Get default settings for BXPT_Rave_AllocCxSettings.
****************************************************************************/
void BXPT_Rave_GetDefaultAllocCxSettings(
    BXPT_Rave_AllocCxSettings *pSettings /* [out] default settings */
    );

/***************************************************************************
Summary:
Allocate a RAVE or soft RAVE context.

Description:
This function can be used to allocate a RAVE or soft RAVE context. It provides
the same functionality as BXPT_Rave_AllocContext and BXPT_Rave_AllocSoftContext
but also allows passing-in a pre-allocated CDB and/or ITB buffer.

Returns:
    BERR_SUCCESS                    - Context allocated successfully.
    BERR_INVALID_PARAMETER          - Bad ContextType parameter
    BXPT_ERR_NO_AVAILABLE_RESOURCES - The requested context is already in use.
****************************************************************************/
BERR_Code BXPT_Rave_AllocCx(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    const BXPT_Rave_AllocCxSettings *pSettings, /* [in] settings for this RAVE channel allocation */
    BXPT_RaveCx_Handle *Context     /* [out] The allocated context */
    );

/***************************************************************************
Summary:
Allocate a pair of RAVE contexts for AVS decoding.

Description:
AVS and AVS+ require some special handling by RAVE. To accomplish this, two
RAVE contexts are used, a decode and a reference. The decode, as the name
implies, is given to the video decoders as the source of their ES and index
tables. The reference context is used by the RAVE fw for AVS emulation bit
removal.

The two contexts must be configured identically, and receive the same video
PID channel. The decoder will read data from the decode context; the
reference context, however, must be emptied by software to ensure that it
doesn't overflow.

Returns:
    BERR_SUCCESS                    - Contexts allocated successfully.
    BERR_INVALID_PARAMETER          - Bad ContextType parameter
    BXPT_ERR_NO_AVAILABLE_RESOURCES - The requested contexts are already in use.
****************************************************************************/
BERR_Code BXPT_Rave_AllocAvsCxPair(
    BXPT_Rave_Handle hRave,         /* [in] Handle for this RAVE channel */
    const BXPT_Rave_AllocCxSettings *pDecodeCxSettings, /* [in] settings for this RAVE channel allocation */
    const BXPT_Rave_AllocCxSettings *pRefernceCxSettings, /* [in] settings for this RAVE channel allocation */
    BXPT_RaveCx_Handle *DecodeContext,     /* [out] The allocated decode context */
    BXPT_RaveCx_Handle *ReferenceContext     /* [out] The allocated context */
    );


/***************************************************************************
Summary:
Create new ITB entries in the given soft RAVE context.

Description:
Using data from the src context (specified at _AllocSoftContext()), create
entries in the soft context's ITB.

Returns:
    void.
****************************************************************************/
void BXPT_Rave_AdvanceSoftContext(
    BXPT_RaveCx_Handle DestContext /* [in] The destination (soft) context */
    );

/***************************************************************************
Summary:
Reset the soft RAVE context.

Description:
Change the soft context's operating mode and resets the internal state
variables. Does not perform a context flush.

Returns:
    void.
****************************************************************************/
BERR_Code BXPT_Rave_ResetSoftContext(
    BXPT_RaveCx_Handle hCtx,
    BXPT_RaveSoftMode Mode
    );

BERR_Code BXPT_Rave_StopPTS(
    BXPT_RaveCx_Handle hCtx,
    uint32_t StopPTS,
    uint32_t tolerance,
    void (* CallBack)(void *, uint32_t pts),
    void *param
    );
BERR_Code BXPT_Rave_StartPTS(
    BXPT_RaveCx_Handle hCtx,
    uint32_t StopPTS,  uint32_t tolerance,
    void (* StartPTSCb)(void *, uint32_t pts),
    void * param
    );
BERR_Code BXPT_Rave_Monitor_PTS(
    BXPT_RaveCx_Handle hCtx,
    uint32_t PTS, uint32_t tolerance,
    void (* CallBack)(void *, uint32_t pts),
    void * param
    );
BERR_Code BXPT_Rave_Cancel_PTS(   BXPT_RaveCx_Handle hCtx  );
BERR_Code BXPT_Rave_SetPCROffset(BXPT_RaveCx_Handle hCtx  , uint32_t pcr_offset);

#if defined(BXPT_HAS_RSBUF) && defined(BXPT_HAS_XCBUF)
/***************************************************************************
Summary:
Set the Banddata rate for corresponding RAVE XC buffer and parserband.

Description:
That API would take a client handle and the ID of the parser band. The
XPT PI would then walk backwards through the pipeline, increasing only
the bitrates for the XC and RS buffers that it needs to.

Returns:
    void.
****************************************************************************/
BERR_Code BXPT_Rave_SetRSXCDataRate(
    BXPT_Rave_Handle hRave,     /* [in] Handle for Rave */
    BXPT_ParserType ParserType, /* [in] Input band or playback */
    unsigned BandNum,           /* [in] Which Input band or playback parser to configure */
    unsigned long Rate,         /* [in] Max rate in bps */
    unsigned PacketLen          /* [in] size of mpeg packet */
    );
#endif
#ifdef BXPT_VCT_SUPPORT
/***************************************************************************
Summary:
Enumeration of the different types of table Id's. To be used for VCT nullification
implementation.
****************************************************************************/
typedef enum BXPT_RaveVct
{
    BXPT_RaveVct_Tvct,
    BXPT_RaveVct_Cvct
}
BXPT_RaveVct;

/***************************************************************************
Summary:
Nullify VCT.

Description:
Nullify VCT.

Returns:
    BERR_SUCCESS                    - Configuration written successfully.
    BERR_INVALID_PARAMETER          - Bad parameter
****************************************************************************/
BERR_Code BXPT_Rave_NullifyVCT(
    BXPT_RaveCx_Handle Context,            /*[in] Context Handle */
    bool ProcessPid,                       /*[in] True(Process PID 0x1ffb) */
    BXPT_RaveVct TableId                   /*[in] Table ID T-VCT C-VCT or non */
    );
#endif

/***************************************************************************
Summary:
Adjust the CDB length.

Description:
Adjust the CDB length to make the CDB size smaller for certain conditions.

Returns:
    void.
****************************************************************************/
BERR_Code BXPT_Rave_AdjustCdbLength(
    BXPT_RaveCx_Handle hCtx,
    size_t CdbLength
    );

/***************************************************************************
Summary:
Get the RAVE's current status.

Returns:
    void.
****************************************************************************/
typedef struct BXPT_Rave_Status
{
    unsigned SupportedContexts;     /* Total number this core supports */
    unsigned AllocatedContexts;     /* Total number currently allocated */
}
BXPT_Rave_Status;

void BXPT_Rave_GetStatus(
    BXPT_Rave_Handle hRave,
    BXPT_Rave_Status *Status
    );

/* Private APIs. Must NOT be called from outside the BXPT PI. */
BERR_Code BXPT_P_RaveRamInit( BXPT_Rave_Handle hRave );

#if BXPT_SW7425_1323_WORKAROUND
BERR_Code BXPT_Rave_P_FlowControl( BXPT_Rave_Handle hRave );
#endif

void BXPT_Rave_P_SetBandHold( BXPT_Handle hXpt, unsigned PbChannelNo, uint32_t BandHoldStatus, bool SetHoldEn );
void BXPT_Rave_P_GetBandHold( BXPT_Handle hXpt, uint32_t *BandHoldStatus );

#if BXPT_SW7425_4528_WORKAROUND
typedef struct BXPT_RAVE_P_PesRemap
{
    unsigned char NewIdFor0xC0;      /* Stream ID 0xC0 will be changed to this value. */
    unsigned char NewIdFor0xC1;      /* Stream ID 0xC1 will be changed to this value. */
    unsigned char NewIdFor0xC2;      /* Stream ID 0xC2 will be changed to this value. */
    unsigned char NewIdFor0xC3;      /* Stream ID 0xC3 will be changed to this value. */
    unsigned char NewIdFor0xC4;      /* Stream ID 0xC4 will be changed to this value. */
    unsigned char NewIdFor0xC5;      /* Stream ID 0xC5 will be changed to this value. */
    bool Enable;                /* Perform remapping if enabled. Defaults to disabled. */
}
BXPT_RAVE_P_PesRemap;

void BXPT_RAVE_P_GetPesRemapping(
    BXPT_RaveCx_Handle hCtx,
    BXPT_RAVE_P_PesRemap *remap
    );

void BXPT_RAVE_P_SetPesRemapping(
    BXPT_RaveCx_Handle hCtx,
    const BXPT_RAVE_P_PesRemap *remap
    );

#endif

void BXPT_Rave_P_EnableInterrupts(
    BXPT_Handle hXpt                                /* [in] Handle for this transport instance */
    );

#ifdef __cplusplus
}
#endif

#endif /* BXPT_RAVE_H__ */

/* end of file */
