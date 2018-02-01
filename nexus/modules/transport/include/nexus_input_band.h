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
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_INPUT_BAND_H__
#define NEXUS_INPUT_BAND_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=********************
An InputBand takes external transport data and routes it into the chip.

See ParserBand and PidChannel for routing that data to various consumers.
*********************/

/**
Summary:
Settings for an input band. Retrieved with NEXUS_InputBand_GetSettings.
**/
typedef struct NEXUS_InputBandSettings
{
    bool clockActiveHigh;   /* true = data is sampled on the rising edge of the clock, false = data is sampled on the falling edge */

    bool dataLsbFirst;      /* true = data is received LSB first, false = data is received MSB first (serial inputs only) */
    bool dataActiveHigh;    /* true = data is activeHigh, false = data is active low */

    bool validEnabled;      /* true = the IB's 'VALID' signal is used. false = the 'VALID' signal is ignored and every clock cycle is forced to be valid. */
    bool validActiveHigh;   /* true = valid is active high, false = valid is active low */

    bool useInternalSync;   /* true = enable internal sync detection, false = use external sync signal */
    bool useSyncAsValid;    /* if useInternalSync=true, this will use the internal sync signal as the valid signal */
    bool syncActiveHigh;    /* true = external sync is active high, false = external sync is active low */

    bool errorEnabled;      /* true = error signal is enabled, false = error signal is ignored */
    bool errorActiveHigh;   /* true = error is active high, false = error is active low */

    bool parallelInput;     /* true = data will be received in parallel, false = data will be received in serial
                               note, not all input bands support parallel input -- generally only input band 4 */
    unsigned packetLength;  /* Packet length in bytes */
} NEXUS_InputBandSettings;

/**
Arm the internal sync generation logic. The circuit works on the arrival of a bit (when in bit serial mode), or a byte (when in parallel mode).
The skip argument the number of bits/bytes that, when added to packet length, will align the incoming stream to the start of transport
packet in the hardware. The first internal sync is generated after skip + length bits/bytes are seen. Subsequent syncs are generated after length
bits/bytes are seen.

Bit versus byte mode is determined by the parallelInput bool in the NEXUS_InputBandSettings structure. The length, or number of bits/bytes between
syncs, is derived from the packetLength, in the same structure.

If a skip value of 0 is used, sync will be generated once every length bits/bytes. If this API is not called, no sync is generated.

This API can be called multiple times for the same input band. If that happens before skip + length bits/bytes are seen, an error will be returned
and new values won't be written to hardware. This avoids an incorrect configuration in the hardware. A possible reason for this is loss of the input
clock or valid signals. Otherwise, the new skip value is used to generate the next sync, per the above.
**/
NEXUS_Error NEXUS_InputBand_ArmSyncGeneration(
    NEXUS_InputBand inputBand,
    unsigned skip                 /* Number of bits/bytes to skip to align packet sync with the input stream, 0 to 255 */
    );

/**
Summary:
Get current NEXUS_InputBand settings.
**/
void NEXUS_InputBand_GetSettings(
    NEXUS_InputBand inputBand,
    NEXUS_InputBandSettings *pSettings /* [out] */
    );

/**
Summary:
Set updated NEXUS_InputBand settings.
**/
NEXUS_Error NEXUS_InputBand_SetSettings(
    NEXUS_InputBand inputBand,
    const NEXUS_InputBandSettings *pSettings
    );

/**
Summary:
Status information which can be retrieved with NEXUS_InputBand_GetStatus.
**/
typedef struct NEXUS_InputBandStatus
{
    unsigned syncCount; /* counter used for monitoring sync bytes/pulses. poll this to determine if data is flowing. */
    unsigned overflowErrors;    /* The input buffer sits upstream from the parsers. Overflow may indicate an
                                RTS issue. This error count does not reset. */
} NEXUS_InputBandStatus;
/**
Summary:
Get current status information from the InputBand
**/
NEXUS_Error NEXUS_InputBand_GetStatus(
    NEXUS_InputBand inputBand,
    NEXUS_InputBandStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif
