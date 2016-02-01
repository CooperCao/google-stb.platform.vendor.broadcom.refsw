/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * Porting interface code for the data transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_playback.h"
#include "bxpt_directv_playback.h"

BDBG_MODULE( xpt_directv_playback );

#include "bchp_xpt_pb0.h"

BERR_Code BXPT_DirecTvPlayback_SetParserBandMode( 
	BXPT_Playback_Handle hPb,		/* [in] Handle for the playback channel */
	BXPT_ParserMode Mode					/* [in] Which mode (packet format) is being used. */
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

	Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
	Reg &= ~( 
		BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE ) |
		BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
		BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH )
		);
	if( Mode == BXPT_ParserMode_eMpeg )
	{
		Reg |= (
			BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE, 0 ) |
			BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE, 0 ) |
			BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 188 )
		);
	}
	else 
	{
		Reg |= (
			BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE, 1 ) |
			BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE, 1 ) |
			BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 130 )
		);
	}
	BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg ); 

	return( ExitCode );
}

BERR_Code BXPT_DirecTvPlayback_GetSyncThresholds( 
	BXPT_Playback_Handle PlaybackHandle,	/* [in] Handle for the playback channel */
	unsigned int *SyncInCount,			/* [out] In-sync threshold. */
	unsigned int *SyncOutCount			/* [out] Out-of-sync threshold. */
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

	/* Get the sync in/out counts. */
	Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL3 );
	*SyncInCount = ( uint8_t ) ( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, DIRECTV_SYNC_IN_CNT ) & 0xFF );
	*SyncOutCount = ( uint8_t ) ( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, DIRECTV_SYNC_OUT_CNT ) & 0xFF );
	
	return( ExitCode );
}


BERR_Code BXPT_DirecTvPlayback_SetSyncThresholds( 
	BXPT_Playback_Handle PlaybackHandle,	/* [in] Handle for the playback channel */
	unsigned int SyncInCount,			/* [in] In-sync threshold. */
	unsigned int SyncOutCount			/* [in] Out-of-sync threshold. */
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

	Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL3 );

	Reg &= ~( 
		BCHP_MASK( XPT_PB0_CTRL3, DIRECTV_SYNC_IN_CNT ) |
		BCHP_MASK( XPT_PB0_CTRL3, DIRECTV_SYNC_OUT_CNT )
		);
	Reg |= ( 
		BCHP_FIELD_DATA( XPT_PB0_CTRL3, DIRECTV_SYNC_IN_CNT, SyncInCount ) |
		BCHP_FIELD_DATA( XPT_PB0_CTRL3, DIRECTV_SYNC_OUT_CNT, SyncOutCount ) 
		);
	
	BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL3, Reg ); 

	return( ExitCode );
}


