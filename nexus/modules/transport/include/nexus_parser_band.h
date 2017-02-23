/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#ifndef NEXUS_PARSER_BAND_H__
#define NEXUS_PARSER_BAND_H__

#include "nexus_types.h"
#include "nexus_transport_capabilities.h"
#include "nexus_remux.h"
#include "nexus_pid_channel.h"
#include "nexus_input_band.h"
#include "nexus_tsmf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=********************
A NEXUS_ParserBand routes data from an input band or remux input to a variety of pid channels.

Use NEXUS_PidChannel to route individual pids to various consumers.
*********************/

/*
Summary:
Used in NEXUS_ParserBandSettings to specify the type of input to the parser band.
*/
typedef enum NEXUS_ParserBandSourceType
{
    NEXUS_ParserBandSourceType_eInputBand,
    NEXUS_ParserBandSourceType_eRemux,
    NEXUS_ParserBandSourceType_eMtsif,
    NEXUS_ParserBandSourceType_eTsmf,
    NEXUS_ParserBandSourceType_eMax
} NEXUS_ParserBandSourceType;

/**
Summary:
Settings for a parser band. Retrieved with NEXUS_ParserBand_GetSettings.
**/
typedef struct NEXUS_ParserBandSettings
{
    NEXUS_ParserBandSourceType sourceType; /* Specifies from what type of source the data is coming into the parser band. See sourceTypeSettings for
                                              additional settings for each enum. */
    struct
    {
        NEXUS_InputBand inputBand;     /* Input band that is mapped to this parserBand. You can route an input band
                                          to multiple parser bands. */
        NEXUS_RemuxHandle remux;       /* A loopback route from a remux output back into a parser band */
        NEXUS_FrontendConnectorHandle mtsif; /* Frontend connector whose input band is mapped to this parserBand, via MTSIF.
                                          Use NEXUS_Frontend_GetConnector(frontend) to get this handle.
                                          The MTSIF connection is established when the frontend is tuned, not when NEXUS_ParserBand_SetSettings is called.
                                          Likewise, the connection is destroyed when frontend is untuned. */
        NEXUS_TsmfHandle tsmf;
    } sourceTypeSettings; /* See sourceType to determine which member of the union to use. */

    NEXUS_TransportType transportType;              /* The format of data on the parser band.
                                                       If the stream has timestamps, set NEXUS_InputBandSettings.packetLength to strip them.
                                                       If you want to record with new timestamps, see NEXUS_RecpumpSettings.timestampType. */
    bool continuityCountEnabled;                    /* If true, transport will check for correct continuity counters and discard bad packets. Default is true. */
    bool teiIgnoreEnabled;                          /* If true, transport will ignore TEI error bits and error input signals. This will allow bad packets to enter transport. Default is false. */
    NEXUS_CableCardType cableCard;                  /* Routing out to cable card */
    bool allPass;                                   /* If true, NEXUS_ParserBand_OpenPidChannel's pid param is ignored and the resulting pid channel can be used for all-pass record.
                                                       Also set acceptNullPackets to true if you want to capture the entire stream.
                                                       When opening the allPass pid channel, set NEXUS_PidChannelSettings.pidChannelIndex to the HwPidChannel obtained
                                                       from NEXUS_ParserBand_GetAllPassPidChannelIndex(). */
    bool acceptNullPackets;                         /* If true and allPass is true, then all-pass record will include null packets. */
    unsigned maxDataRate;                           /* Deprecated. See NEXUS_TransportModuleSettings.maxDataRate.parserBand. */
    NEXUS_CallbackDesc ccError;                     /* Continuity Count Error (CC) - raised when continuity counter of next packet does not have the next counter value */
    NEXUS_CallbackDesc teiError;                    /* Transport Error Indicator (TEI) - error status from a demodulator */
    NEXUS_CallbackDesc lengthError;                 /* Transport Length Error - raised when a short transport packet is received.
                                                       Long transport packets are automatically truncated and do not raise this error. */
    bool acceptAdapt00;                             /* Packets with an adaptation field of 00 are accepted if true */
    bool forceRestamping;                           /* Set true if you want to restamp the incoming timestamps with locally-generated values. Otherwise, timestamps delivered
                                                    with the stream are used. Default value is true. */
} NEXUS_ParserBandSettings;

/**
Summary:
Get current NEXUS_ParserBand settings.
**/
void NEXUS_ParserBand_GetSettings(
    NEXUS_ParserBand parserBand,
    NEXUS_ParserBandSettings *pSettings /* [out] */
    );

/**
Summary:
Set updated NEXUS_ParserBand settings.
**/
NEXUS_Error NEXUS_ParserBand_SetSettings(
    NEXUS_ParserBand parserBand,
    const NEXUS_ParserBandSettings *pSettings
    );

/**
Summary:
Open a parser band for exclusive use

Description:
NEXUS_ParserBand_Open is required for protected clients to exclusively use oa parser band.
**/
NEXUS_ParserBand NEXUS_ParserBand_Open( /* attr{destructor=NEXUS_ParserBand_Close}  */
    unsigned index /* Use a specific index or NEXUS_ANY_ID */
    );

/**
Summary:
Close a parser band opened with NEXUS_ParserBand_Open
**/
void NEXUS_ParserBand_Close(
    NEXUS_ParserBand parserBand
    );

/**
Summary:
Status from a parser band. Retrieved with NEXUS_ParserBand_GetStatus.
**/
typedef struct NEXUS_ParserBandStatus
{
    struct {
        unsigned overflowErrors; /* Cumulative count of Rate Smoothing Buffer overflows detected.
                                    Reset when PID channel is disabled or NEXUS_PidChannel_ResetStatus() */
    } rsBufferStatus;
    unsigned index; /* hardware parser band number */
} NEXUS_ParserBandStatus;

/**
Summary:
Get current status of a parser band.
**/
NEXUS_Error NEXUS_ParserBand_GetStatus(
    NEXUS_ParserBand parserBand,
    NEXUS_ParserBandStatus *pStatus /* [out] */
    );

/**
Summary:
Determine the PID channel that will be used in allPass mode.

Description:
Each parser will use a different PID channel when operating in
allPass mode. This API provides the channel number for the given
parser band.
**/
NEXUS_Error NEXUS_ParserBand_GetAllPassPidChannelIndex(
    NEXUS_ParserBand parserBand,
    unsigned *pHwPidChannel
    );

#ifdef __cplusplus
}
#endif

#endif
