/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2007-2016 Broadcom. All rights reserved.
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
 *
 **************************************************************************/
#ifndef NEXUS_PID_CHANNEL_H__
#define NEXUS_PID_CHANNEL_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle which represents a transport PID channel.

Description:
A NEXUS_PidChannel routes one PID from a parser band to a variety of clients (decoder, record, message filtering).

A NEXUS_PidChannel must be opened according to its source:

- NEXUS_PidChannel_Open is used to open a PID channel from a NEXUS_ParserBand. This is used for live streams.
- NEXUS_Playpump_OpenPidChannel is used to open a PID channel from a NEXUS_PlaypumpHandle. This is used for low-level PVR.
- NEXUS_Playback_OpenPidChannel is used to open a PID channel from a NEXUS_PlaybackHandle. This is used for high-level PVR.

PID channels are used by a variety of other Nexus interfaces:

- See NEXUS_VideoDecoderStartSettings for video decode
- See NEXUS_AudioDecoderStartSettings for audio decode
- See NEXUS_Recpump_AddPidChannel for low-level record
- See NEXUS_Record_AddPidChannel for high-level record
- See NEXUS_MessageStartSettings for message filtering
- See NEXUS_Remux_AddPidChannel for remux output
**/
typedef struct NEXUS_PidChannel *NEXUS_PidChannelHandle;

/**
Summary:
Special values for NEXUS_PidChannelSettings.pidChannelIndex
**/
#define NEXUS_PID_CHANNEL_OPEN_ANY ((unsigned)-1)
#define NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE ((unsigned)-2)
#define NEXUS_PID_CHANNEL_OPEN_NOT_MESSAGE_CAPABLE ((unsigned)-3)
#define NEXUS_PID_CHANNEL_OPEN_INJECTION ((unsigned)-4)

/**
Summary:
DSS HD filter modes for pid channels
**/
typedef enum NEXUS_PidChannelDssHdFilter {
    NEXUS_PidChannelDssHdFilter_eDisabled,
    NEXUS_PidChannelDssHdFilter_eAux, /* only AUX packets */
    NEXUS_PidChannelDssHdFilter_eNonAux, /* only non-AUX packets */
    NEXUS_PidChannelDssHdFilter_eMax
} NEXUS_PidChannelDssHdFilter;

/**
Summary:
Secondary PID (SPID) remapping

The primary PID, specified as the 'pid' param whening opening, is the input pid.
If remapping is enabled, the secondary PID becomes the new pid when writing TS output to record or remux.
**/
typedef struct NEXUS_PidChannelRemapSettings
{
    bool enabled; /* enabled PID remapping */
    uint16_t pid; /* secondary PID, which becomes the new pid on output */
    bool continuityCountEnabled; /* If true, transport will check for correct continuity counters and discard bad packets. Default is true. */
}NEXUS_PidChannelRemapSettings;

/**
Summary:
Settings for a PID channel passed into NEXUS_PidChannel_Open.
**/
typedef struct NEXUS_PidChannelSettings
{
    bool requireMessageBuffer; /* deprecated. use NEXUS_PidChannelSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE instead. */
    unsigned pidChannelIndex;  /* Set the pid channel allocation scheme. This can be one of the special NEXUS_PID_CHANNEL_OPEN values, or an unsigned pidChannelIndex value.
                                  NEXUS_PID_CHANNEL_OPEN_ANY (default) will select any hardware PID channel index.
                                  NEXUS_PID_CHANNEL_OPEN_MESSAGE_CAPABLE will select a hardware PID channel index which is capable of message filtering.
                                    Only the lower 128 pid channels are capable of message filtering.
                                  NEXUS_PID_CHANNEL_OPEN_NOT_MESSAGE_CAPABLE will select a hardware PID channel index which is not capable of message filtering.
                                    This allows an application to reserve message-capable pid channels for message filtering.
                                  NEXUS_PID_CHANNEL_OPEN_INJECTION uses a hardware PID channel for packetsub injection. It will never be enabled for pid filtering,
                                    so the 'pid' param of NEXUS_PidChannel_Open is unused. Otherwise the app is burdened to find an unused pid value or
                                    must match settings with a used pid value.
                                  Otherwise, Nexus will use the pidChannelIndex if there is not a pid conflict on the same HW PID channel.

                                  If you need a specific pid channel index somewhere in your system, you may have to reserve that index by opening it
                                  immediately after system initialization. One case would be requiring an "all pass" pid channel. */
    NEXUS_PidChannelDssHdFilter dssHdFilter; /* DSS HD filter modes */
    bool enabled;              /* true by default. call NEXUS_PidChannel_SetEnabled to change after opening. */
    bool continuityCountEnabled;  /* If true, transport will check for correct continuity counters and discard bad packets. Default is true. */
    bool generateContinuityCount; /* If true, transport will generate a new continuity counters for the TS */

    NEXUS_PidChannelRemapSettings remap;
    
    struct {
        unsigned low;  /* Lowest PES stream id that is allowed to pass. 
                          By default, low = high = 0, which uses internal ranges based on codec. */
        unsigned high; /* Highest PES stream id that is allowed to pass. */
    } pesFiltering;
} NEXUS_PidChannelSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_PidChannel_GetDefaultSettings(
    NEXUS_PidChannelSettings *pSettings /* [out] */
    );

/**
Summary:
Open a new PID channel from a parser band.

Description:
Most platforms do not allow multiple HW PID channels for the same pid and parser band.
But Nexus allows these duplicate pid channel handles to be opened. These are called SW PID channels.
So each HW pid channel is controlled using one or more SW pid channels.

If the NEXUS_PidChannelSettings are different between the set of SW pid channels for the same HW pid channel,
Nexus will try to resolve. For example:
* pidChannelIndex must be the same for all or Nexus will fail.
* enabled is aggregated by and-ing: if any is disabled, the HW is disabled.
* dssHdFilter, remap and continuityCountEnabled are aggregated by last-one-wins.
**/
NEXUS_PidChannelHandle NEXUS_PidChannel_Open( /* attr{destructor=NEXUS_PidChannel_Close}  */
    NEXUS_ParserBand parserBand,              /* Data source for the PID channel */
    uint16_t pid,                             /* PID (packet ID) that will be selected from the parser band. If TS, this is a PID. If PES, this is a PES stream id.
                                                 If ES or NEXUS_ParserBandSettings.allPass is true, this param is ignored.
                                                 For audio VOB streams, the upper 8 bits of the PID specify the substream id. So, 0x00BD is AC3/DTS track 0,
                                                 0x01BD is AC3/DTS track 1, etc. */
    const NEXUS_PidChannelSettings *pSettings /* attr{null_allowed=y} Optional */
    );

/*
Summary:
Close a PID channel opened by NEXUS_PidChannel_Open.

Description:
This function should not close a PID channel opened by NEXUS_Playpump_OpenPidChannel or NEXUS_Playback_OpenPidChannel.
*/
void NEXUS_PidChannel_Close(
    NEXUS_PidChannelHandle pidChannel
    );

/**
Summary:
Close all PID channels opened on a parser band.

Description:
All associated PID channel handles will become invalid.
**/
void NEXUS_PidChannel_CloseAll(
    NEXUS_ParserBand parserBand
    );

/**
Summary:
Returns current configuration of a PID channel.
**/
void NEXUS_PidChannel_GetSettings(
    NEXUS_PidChannelHandle handle,
    NEXUS_PidChannelSettings *pSettings
    );

/**
Summary:
Changes configuration of a PID channel.

Description:
Some settings (for example, pidChannelIndex) cannot be changed
after NEXUS_PidChannel_Open() is called. This call will fail if
an attempt is made to change any of those settings.

See NEXUS_PidChannel_Open for how varying settings among different SW pid channels
are resolved.
**/
NEXUS_Error NEXUS_PidChannel_SetSettings(
    NEXUS_PidChannelHandle handle,
    const NEXUS_PidChannelSettings *pSettings
    );

/**
Summary:
Status for a PID channel retrieved from NEXUS_PidChannel_GetStatus
**/
typedef struct NEXUS_PidChannelStatus
{
    uint16_t pid;                      /* PID carried by this PID channel */
    unsigned pidChannelIndex;          /* HW PID channel index */
    NEXUS_TransportType transportType; /* If the parser band packetized the input, this described the output. If there is no
                                          packetization, this is the original transportType. */
    NEXUS_TransportType originalTransportType; /* If the parser band packetized the input, this describes the input. If there is no
                                          packetization, this is the same as transportType. */

    uint16_t remappedPid;              /* If the parser band packetized the input (for ES/PES playback) or
                                          if NEXUS_PidChannelSettings.remap.enabled is true (for live SPID remapping),
                                          this describes the PID of the output; otherwise, this is the original PID. */
    bool enabled;                      /* True if the PidChannel is enabled, false otherwise. */
    
    bool playback;                     /* true if pid channel is from a playback parser band. otherwise, it's from a live parser band. */
    unsigned playbackIndex;            /* HW playback index. Valid only if playback==true. */
    unsigned parserBand;               /* HW parser band index. Valid only if playback==false. */

    unsigned continuityCountErrors;    /* Rough count of cc errors for this pid channel since it was opened or since NEXUS_PidChannel_ResetStatus was called.
                                          The count is only performed if either NEXUS_ParserBandSettings.ccError and .continuityCountEnabled is set or
                                          NEXUS_PlaypumpSettings.ccError and .continuityCountEnabled is set for this pid.
                                          There is no ccError callback per pid.
                                          An exact count is not possible because more than one error could occur before the software can count and clear the hardware status register. */

    struct {
        unsigned continuityCountErrors;
        unsigned emulationByteRemovalErrors;
        unsigned pusiErrors;
        unsigned teiErrors;
        unsigned cdbOverflowErrors;
        unsigned itbOverflowErrors;
    } raveStatus;
    struct {
        unsigned overflowErrors;
    } xcBufferStatus;
} NEXUS_PidChannelStatus;

/**
Summary:
Get current status for a PID channel.
**/
NEXUS_Error NEXUS_PidChannel_GetStatus(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_PidChannelStatus *pStatus /* [out] */
    );

/**
Summary:
Inform a new PID channel that data can start flowing.

Description:
For some PID channels, notably those fed by playback, the flow of data may be paused until the first consumer is started.
This prevents lost data due to application delay between calls to Nexus.

A Nexus consumer (e.g. decoder, record, message filter, remux) will internally call this function when started
or when a pid channel is added to an already-started consumer.
This only needs to be called from the application for a non-Nexus consumer or some atypical configuration.
**/
void NEXUS_PidChannel_ConsumerStarted(
    NEXUS_PidChannelHandle pidChannel
    );

/**
Summary:
Add a PID channel for PID splicing

Description:
The transport hardware will monitor the transport stream for splice points.

This is only supported for PID channels which are currently being decoded.

Call NEXUS_PidChannel_RemoveSplicePidChannel or NEXUS_PidChannel_RemoveSplicePidChannels to remove the PID channel and disable PID splicing.
**/
NEXUS_Error NEXUS_PidChannel_AddSplicePidChannel(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_PidChannelHandle splicePidChannel
    );

/**
Summary:
Remove a PID channel that was added using NEXUS_PidChannel_AddSplicePidChannel
**/
NEXUS_Error NEXUS_PidChannel_RemoveSplicePidChannel(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_PidChannelHandle splicePidChannel
    );

/**
Summary:
Remove all PID channels that were added using NEXUS_PidChannel_AddSplicePidChannel
**/
void NEXUS_PidChannel_RemoveAllSplicePidChannels(
    NEXUS_PidChannelHandle pidChannel
    );

/**
Summary:
Enable or disable a PID channel on the fly.

Description:
This is equivalent to calling NEXUS_PidChannel_SetSettings with enabled = false, but
without changing the stored NEXUS_PidChannelSettings.
See NEXUS_PidChannel_Open for how varying settings among different SW pid channels
are resolved.
**/
NEXUS_Error NEXUS_PidChannel_SetEnabled(
    NEXUS_PidChannelHandle pidChannel,
    bool enabled
    );

/**
Summary:
Set the remap settings for an already-opened PID channel.
**/
NEXUS_Error NEXUS_PidChannel_SetRemapSettings(
    NEXUS_PidChannelHandle pidChannel,
    const NEXUS_PidChannelRemapSettings *pSettings
    );

/**
Summary:
Reset any accumulated values in NEXUS_PidChannelStatus
**/
NEXUS_Error NEXUS_PidChannel_ResetStatus(
    NEXUS_PidChannelHandle pidChannel
    );
    
/**
Summary:
settings for NEXUS_PidChannel_AddSlavePidChannel
**/
typedef struct NEXUS_PidChannelSlaveSettings
{
    unsigned tbd;
} NEXUS_PidChannelSlaveSettings;

/**
Summary:
**/
void NEXUS_PidChannel_GetDefaultSlaveSettings(
    NEXUS_PidChannelSlaveSettings *pSettings
    );

/**
Summary:
A slave pid channel allows more than one pid channel to flow into a single RAVE context.

Description:
A slave cannot also be a master.
You can only give master pid channels to the audio and video decoders.
The master pid channel must remain open until all slaves have been removed.
It is assumed that the slave pid channel is the same format as the master.
**/
NEXUS_Error NEXUS_PidChannel_AddSlavePidChannel(
    NEXUS_PidChannelHandle master,
    NEXUS_PidChannelHandle slave,
    const NEXUS_PidChannelSlaveSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
Remove a pid channel added by NEXUS_PidChannel_AddSlavePidChannel
**/
void NEXUS_PidChannel_RemoveSlavePidChannel(
    NEXUS_PidChannelHandle master,
    NEXUS_PidChannelHandle slave
    );
    
/**
Summary:
Change the pid on the pid channel

This allows a started decode to receive a new PID, provided the caller guarantees
the codec doesn't change, ensures the playback fifo is draimed before switching,
and that there's no duplicate pid on the parser band.
**/
NEXUS_Error NEXUS_PidChannel_ChangePid(
    NEXUS_PidChannelHandle pidChannel,
    unsigned pid
    );

typedef enum NEXUS_ItbEventType
{
    NEXUS_ItbEventType_ePts,
    NEXUS_ItbEventType_eBtp,
    NEXUS_ItbEventType_eMax
} NEXUS_ItbEventType;

typedef struct NEXUS_ItbEvent
{
    NEXUS_ItbEventType type;
    union {
        uint32_t pts;
        struct {
            unsigned tag;
        } btp;
    } data;
} NEXUS_ItbEvent;

/*
Audio or video decoder must be started to return events. If no decoder started, function will fail.
If flush is called on decoder, this function will only return new data after the flush.
Returns error if overflow detected, but it is not guaranteed to catch all overflows.
*/
NEXUS_Error NEXUS_PidChannel_ReadItbEvents(
    NEXUS_PidChannelHandle pidChannel,
    NEXUS_ItbEvent *pEvents, /* attr{nelem=numEvents;nelem_out=pNumReturned} */
    unsigned numEvents,
    unsigned *pNumReturned
    );

#ifdef __cplusplus
}
#endif

#endif
