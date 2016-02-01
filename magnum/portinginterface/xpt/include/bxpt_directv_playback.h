/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Porting interface code for the DirecTV PVR playback sections of the 
 * data transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
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
#define BXPT_PB_SYNC_DIRECTV		( 0x1 )
#define BXPT_PB_SYNC_DIRECTV_BLIND	( 0x6 )

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
	BXPT_Playback_Handle PlaybackHandle,	/* [in] Handle for the playback channel */
	BXPT_ParserMode Mode					/* [in] Which mode (packet format) is being used. */
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
	BXPT_Playback_Handle PlaybackHandle,	/* [in] Handle for the playback channel */
	unsigned int *SyncInCount,			/* [out] In-sync threshold. */
	unsigned int *SyncOutCount			/* [out] Out-of-sync threshold. */
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
	BXPT_Playback_Handle PlaybackHandle,	/* [in] Handle for the playback channel */
	unsigned int SyncInCount,			/* [in] In-sync threshold. */
	unsigned int SyncOutCount			/* [in] Out-of-sync threshold. */
	);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BXPT_DIRECTV_PLAYBACK_H__ */

/* end of file */


