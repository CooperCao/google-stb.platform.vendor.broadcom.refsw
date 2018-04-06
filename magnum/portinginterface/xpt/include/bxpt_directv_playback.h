/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

/*= Module Overview *********************************************************
This module implements support for the DirecTV specific portions of the PVR
playback logic.
***************************************************************************/

#ifndef BXPT_DIRECTV_PLAYBACK_H__
#define BXPT_DIRECTV_PLAYBACK_H__

#include "bxpt.h"
#include "bxpt_playback.h"
#include "bxpt_directv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
Defines for the DirecTV packet sync types that are supported. These
are used as values passed to BXPT_Playback_ChannelSettings.SyncMode ( used
when the channel is opened or BXPT_Playback_SetChannelSettings() is called ).

The caller should also set BXPT_Playback_ChannelSettings.PacketLength
appropriately.
****************************************************************************/
#define BXPT_PB_SYNC_DIRECTV        ( 0x1 )
#define BXPT_PB_SYNC_DIRECTV_BLIND  ( 0x6 )

/***************************************************************************
Summary:
Set the MPEG or DirectTV mode in a given parser band.

Description:
Changes a parser band between MPEG and DirecTV mode. Also sets the packet
length as appropriate.

Returns:
    BERR_SUCCESS                - Change was successful.
    BERR_INVALID_PARAMETER      - Bad input parameter
***************************************************************************/
BERR_Code BXPT_DirecTvPlayback_SetParserBandMode(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    BXPT_ParserMode Mode                    /* [in] Which mode (packet format) is being used. */
    );

/***************************************************************************
Summary:
Get the SyncIn and SyncOut thresholds for the playback sync extractor.

Description:
Retrieve the sync extractor thresholds for a given playback channel. There
are two thresholds, the SyncInCount and the SyncOutCount. The SyncInCount
is the mininum number of valid consecutive packet syncs that must be seen
before the extraction engine declares itself synchronised to the DirecTv
stream. The SyncOutCount is the number of invalid or missing syncs that must
be seen before the extraction engine declares sync is lost.

Returns:
    BERR_SUCCESS                - Success.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_DirecTvPlayback_SetSyncThresholds
****************************************************************************/
BERR_Code BXPT_DirecTvPlayback_GetSyncThresholds(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    unsigned int *SyncInCount,          /* [out] In-sync threshold. */
    unsigned int *SyncOutCount          /* [out] Out-of-sync threshold. */
    );

/***************************************************************************
Summary:
Set the SyncIn and SyncOut thresholds for the playback sync extractor.

Description:
Set the sync extractor thresholds for a given playback channel. There
are two thresholds, the SyncInCount and the SyncOutCount. The SyncInCount
is the mininum number of valid consecutive packet syncs that must be seen
before the extraction engine declares itself synchronised to the DirecTv
stream. The SyncOutCount is the number of invalid or missing syncs that must
be seen before the extraction engine declares sync is lost.

Returns:
    BERR_SUCCESS                - Thresholds have been set.
    BERR_INVALID_PARAMETER      - Bad input parameter

See Also:
BXPT_DirecTvPlayback_GetSyncThresholds
****************************************************************************/
BERR_Code BXPT_DirecTvPlayback_SetSyncThresholds(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    unsigned int SyncInCount,           /* [in] In-sync threshold. */
    unsigned int SyncOutCount           /* [in] Out-of-sync threshold. */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_DIRECTV_PLAYBACK_H__ */

/* end of file */
