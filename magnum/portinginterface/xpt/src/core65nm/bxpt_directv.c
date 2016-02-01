/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Porting interface code for the DirecTV section of the data transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bxpt.h"
#include "bxpt_directv.h"
#include "bdbg.h"
#include "bkni.h"
#include "bxpt_priv.h"

#ifdef BXPT_HAS_XCBUF					   
#include "bxpt_xcbuf.h"
#endif

#ifdef BXPT_HAS_RSBUF						
#include "bxpt_rsbuf.h"
#endif

#include "bchp_xpt_fe.h"

#if BXPT_HAS_MESG_BUFFERS
#include "bchp_xpt_msg.h"
#endif

#if BXPT_HAS_PACKETSUB
#include "bchp_xpt_psub.h"
#include "bxpt_packetsub.h"
#endif

/* Number of bytes between successive registers of the same type or function. */
#define PARSER_REG_STEPSIZE		( BCHP_XPT_FE_PARSER1_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1 )
#define INBAND_REG_STEPSIZE		( BCHP_XPT_FE_IB1_CTRL - BCHP_XPT_FE_IB0_CTRL )
#define MSG_PID_CTRL1_STEPSIZE	( 4 )
#define PID_CHANNEL_STEPSIZE	( 4 )
#define CAP_FILT_REG_STEPSIZE	( 4 )
#define SPID_CHNL_STEPSIZE		( 4 )
#define DEFAULT_PEAK_RATE  		( 25000000 )

BDBG_MODULE( xpt_directv );

#if BXPT_HAS_IB_PID_PARSERS

BERR_Code BXPT_DirecTv_SetParserBandMode( 
	BXPT_Handle hXpt, 	   	/* [Input] Handle for this transport */
	unsigned int Band, 			/* [Input] Which parser band */
	BXPT_ParserMode Mode	/* [Input] Which mode (packet format) is being used. */
	)
{
	uint32_t Reg, RegAddr;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	/* Sanity check on the arguments. */
	if( Band >= hXpt->MaxPidParsers )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "Band %lu is out of range!", ( unsigned long ) Band ));
		ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else
	{
#if 0
    /* Removed for PR 42443. This setting is overwriting the custom setting made in bsettop_base_xpt.c */
        /* change the block out count values based on stream type */
        if(Mode==BXPT_ParserMode_eMpeg)
        {
            BXPT_RsBuf_SetBandDataRate( hXpt, Band, DEFAULT_PEAK_RATE, 188 );
            BXPT_XcBuf_SetBandDataRate( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + Band, DEFAULT_PEAK_RATE, 188 ); 
        }
        else
        {
            BXPT_RsBuf_SetBandDataRate( hXpt, Band, DEFAULT_PEAK_RATE, 130 );
            BXPT_XcBuf_SetBandDataRate( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + Band, DEFAULT_PEAK_RATE, 130 ); 
        }
#endif

		/* The parser config registers are at consecutive addresses. */
		#if (BCHP_CHIP == 7342)
		RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(Band);
		#else
		RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( Band * PARSER_REG_STEPSIZE );
		#endif
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );

		Reg &= ~( 
			BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_CONT_COUNT_IGNORE ) |
			BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_PACKET_TYPE ) |
			BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_PKT_LENGTH )
			);

		if( Mode == BXPT_ParserMode_eMpeg )
		{
			Reg |= (
				BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_CONT_COUNT_IGNORE, hXpt->InputParserContCountIgnore[ Band ] ? 1 : 0 ) |
				BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_PACKET_TYPE, 0 ) |
				BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_PKT_LENGTH, 188 )
			);
			BREG_Write32( hXpt->hRegister, RegAddr, Reg );
		}
		else if ( Mode == BXPT_ParserMode_eDirecTv )
		{
			Reg |= (
				BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_CONT_COUNT_IGNORE, 1 ) |
				BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_PACKET_TYPE, 1 ) |
				BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_PKT_LENGTH, 130 )
			);
			BREG_Write32( hXpt->hRegister, RegAddr, Reg );
		}
		else
		{
			/* Unsupported parser mode. Complain. */
			BDBG_ERR(( "Unsupported parser mode %d!", ( unsigned long ) Mode ));
			ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
		}
	}

	return( ExitCode );
}

#endif

#if BXPT_HAS_MESG_BUFFERS

void BXPT_DirecTv_SaveMptFlag( 
	BXPT_Handle hXpt, 	   	/* [in] Handle for this transport */
	bool Enable 			/* [in] Enable or disable flag saving. */
	)
{
	uint32_t Reg, RegAddr;
	unsigned Index;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	for( Index = 0; Index < BXPT_P_MAX_PID_CHANNELS; Index++ )
	{
		RegAddr = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ARRAY_BASE + ( Index * MSG_PID_CTRL1_STEPSIZE );
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Reg &= ~( BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS ) );
		Reg |= ( BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS, Enable == true ? 1 : 0 ) );
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );
	}
}

void BXPT_DirecTv_SetPesStreamIdBoundaries( 
	BXPT_Handle hXpt, 	   	/* [in] Handle for this transport */
	unsigned int UpperId,		/* [in] The upper stream id. */
	unsigned int LowerId			/* [in] The lower stream id. */
	)
{
	uint32_t Reg;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1 );

	Reg &= ~( 
		BCHP_MASK( XPT_MSG_PES_CTRL1, PES_STREAM_ID_HI ) |
		BCHP_MASK( XPT_MSG_PES_CTRL1, PES_STREAM_ID_LO ) 
		);
	
	Reg |= ( 
		BCHP_FIELD_DATA( XPT_MSG_PES_CTRL1, PES_STREAM_ID_HI, UpperId ) |
		BCHP_FIELD_DATA( XPT_MSG_PES_CTRL1, PES_STREAM_ID_LO, LowerId ) 
		);
	
	BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1, Reg );
}


void BXPT_DirecTv_SetStartcodeChecking( 
	BXPT_Handle hXpt, 	   	/* [in] Handle for this transport */
	bool EnableChecking		/* [in] Enable checking, or not. */
	)
{
	uint32_t Reg;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1 );

	Reg &= ~( 
		BCHP_MASK( XPT_MSG_PES_CTRL1, PSC_CHECK_MODE ) 
		);
	
	Reg |= ( 
		BCHP_FIELD_DATA( XPT_MSG_PES_CTRL1, PSC_CHECK_MODE, EnableChecking == true ? 1 : 0  ) 
		);
	
	BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1, Reg );
}

BERR_Code BXPT_StartDirecTvMessageCapture( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport */
	unsigned int PidChannelNum, 					/* [in] Which PID channel. */
	BXPT_DirecTvMessageType MessageType,		/* [in] What type of DirecTV messages. */
	const BXPT_PsiMessageSettings *Settings 	/* [in] PID, band, and filters to use. */
	)
{
    BXPT_DirecTvMessageOptions Options;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    Options.Flags = BXPT_DirecTvMessageFlags_eSaveFirst;
    return BXPT_StartDirecTvMessageCaptureWithOptions( hXpt, PidChannelNum, MessageType, Settings, &Options );
}

BERR_Code BXPT_StartDirecTvMessageCaptureWithOptions( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport */
	unsigned int PidChannelNum, 					/* [in] Which PID channel. */
	BXPT_DirecTvMessageType MessageType,		/* [in] What type of DirecTV messages. */
	const BXPT_PsiMessageSettings *Settings, 	/* [in] PID, band, and filters to use. */
    const BXPT_DirecTvMessageOptions *Options   /* [in] Additional options for message capture */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
	BDBG_ASSERT( Settings );
	BDBG_ASSERT( Options );

	/* Sanity check on the arguments. */
	if( PidChannelNum >= hXpt->MaxPidChannels )	   
	{
		/* Bad parser band number. Complain. */
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else
	{
		uint32_t Reg, RegAddr, SaveFlags;
		unsigned int GroupSel, OutputMode, GenOffset, SpecialSel, SpecialType;

		ExitCode = BXPT_P_GetGroupSelect( Settings->Bank, &GroupSel );
		if( ExitCode != BERR_SUCCESS )
			goto Done;

		/* Enable the filters for this PID channel. */
		BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE + ( PidChannelNum * GEN_FILT_EN_STEP ), Settings->FilterEnableMask );		

		/* Configure the message buffer for capturing messages. */
		switch( MessageType )
		{
			case BXPT_DirecTvMessageType_eMpt:
			GenOffset = 4;
			OutputMode = 0x0A; 
            SpecialSel = 0;
            SpecialType = 0;
			break;

			case BXPT_DirecTvMessageType_eAuxOnlyPackets: 
			GenOffset = 3;
			OutputMode = 0x08; 
            SpecialSel = 0;
            SpecialType = 0;
			break;
	
			case BXPT_DirecTvMessageType_eRegular_CapFilter0: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter1: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter2: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter3: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter4: 
			GenOffset = 3;
			OutputMode = 0x09; 

            /* Determine which CAP filter is to used. */
            SpecialSel = MessageType - BXPT_DirecTvMessageType_eRegular_CapFilter0;
            SpecialType = 2;
			break;
			 
			/* Regular mode is also the default case. */
			default:
			case BXPT_DirecTvMessageType_eRegular: 
			GenOffset = 3;
			OutputMode = 0x09; 
            SpecialSel = 0;
            SpecialType = 0;
			break;
		}
	
		/* Select the bank the filters are in. */
		RegAddr = BCHP_XPT_MSG_PID_CTRL2_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CTRL2_TABLE_STEP );
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, GEN_OFFSET ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, SPECIAL_TYPE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, SKIP_BYTE2 ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, GEN_GRP_SEL ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, FILTER_MODE ) 
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, GEN_OFFSET, GenOffset ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, SPECIAL_TYPE, SpecialType ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, SKIP_BYTE2, 1 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, GEN_GRP_SEL, GroupSel ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, FILTER_MODE, 0 )
		);
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );
	
		RegAddr = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CTRL1_TABLE_STEP );
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_MODE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_DIS ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, SPECIAL_SEL ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, ALIGN_MODE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, DATA_OUTPUT_MODE ) 
		);
	
        switch (Options->Flags) 
        {
            case BXPT_DirecTvMessageFlags_eSaveNone:
            SaveFlags = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_NONE;
            break;

            case BXPT_DirecTvMessageFlags_eSaveAll:
            SaveFlags = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_ALL;
            break;

            default:
            case BXPT_DirecTvMessageFlags_eSaveFirst:
            SaveFlags = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_FIRST;
            break;
        }

		/* Configure the message buffer for capturing PSI messages. */
		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, SPECIAL_SEL, SpecialSel ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, ALIGN_MODE, Settings->ByteAlign == true ? 0 : 1 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS, SaveFlags ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, DATA_OUTPUT_MODE, OutputMode ) 
		);

		/* CRC checks can be disabled on a per-PID channel basis. Do this here if requested in the settings struct. */
		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_MODE, 0 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_DIS, Settings->CrcDisable == true ? 1 : 0 ) 
		);
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );

		/* Enable data from the PID channel to the mesg block. */
		BXPT_P_SetPidChannelDestination( hXpt, PidChannelNum, 6, true );

		/* Set this PID channel as allocated, in case they forced the channel assignment. */
		hXpt->PidChannelTable[ PidChannelNum ].IsAllocated = true;

		/* Configure the PID channel */
		ExitCode = BXPT_ConfigurePidChannel( hXpt, PidChannelNum, Settings->Pid, Settings->Band );
        if( ExitCode != BERR_SUCCESS )
            goto Done;

		/* 
		** Some filtering configs were done in the parser band on older chips. Now, those
		** configs are done per-PID channel. 
		*/
		BXPT_P_ApplyParserPsiSettings( hXpt, Settings->Band, false );	/* Check IB parsers */
		BXPT_P_ApplyParserPsiSettings( hXpt, Settings->Band, true );	/* Check playback parsers */

		/* Enable the PID channel. */ 
#if BXPT_MESG_DONT_AUTO_ENABLE_PID_CHANNEL
        /* do nothing. must be explicitly enabled. */
#else
		ExitCode = BERR_TRACE( BXPT_EnablePidChannel( hXpt, PidChannelNum ) );
#endif
	}

	Done:
	return( ExitCode );
}


BERR_Code BXPT_StopDirecTvMessageCapture( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int ScidChannelNum		/* [in] Which SCID channel. */
	)
{
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	return( BERR_TRACE(BXPT_StopPidChannelRecord( hXpt, ScidChannelNum )) );
}

#if BXPT_HAS_PID2BUF_MAPPING
BERR_Code BXPT_Mesg_StartDirecTvMessageCapture( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport */
	unsigned int PidChannelNum, 				/* [in] Which PID channel. */
	unsigned int MesgBufferNum,                 /* [in] Which Message Buffer. */
	BXPT_DirecTvMessageType MessageType,		/* [in] What type of DirecTV messages. */
	const BXPT_PsiMessageSettings *Settings 	/* [in] PID, band, and filters to use. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
	BDBG_ASSERT( Settings );

	/* Sanity check on the arguments. */
	if( PidChannelNum >= hXpt->MaxPidChannels )	   
	{
		/* Bad parser band number. Complain. */
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else if( hXpt->MesgBufferIsInitialized[ MesgBufferNum ] == false )
    {
        BDBG_ERR(( "Message buffer for this channel not configured!" ));
        ExitCode = BERR_TRACE( BXPT_ERR_MESG_BUFFER_NOT_CONFIGURED );
    }
	else if ( MesgBufferNum >= BXPT_P_MAX_MESG_BUFFERS ) {
        /* Bad Message Buffer number. Complain. */
        BDBG_ERR(( "Message Buffer Number %lu is out of range!", ( unsigned long ) MesgBufferNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else if ( hXpt->Pid2BuffMappingOn == true )
	{
		uint32_t Reg, RegAddr;
		unsigned int GroupSel, OutputMode, GenOffset, SpecialSel, SpecialType;

		ExitCode = BXPT_P_GetGroupSelect( Settings->Bank, &GroupSel );
		if( ExitCode != BERR_SUCCESS )
			goto Done;

		/* Enable the filters for this PID channel. */
		BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE + ( MesgBufferNum * GEN_FILT_EN_STEP ), Settings->FilterEnableMask );		

		/* Configure the message buffer for capturing messages. */
		switch( MessageType )
		{
			case BXPT_DirecTvMessageType_eMpt:
			GenOffset = 4;
			OutputMode = 0x0A; 
            SpecialSel = 0;
            SpecialType = 0;
			break;

			case BXPT_DirecTvMessageType_eAuxOnlyPackets: 
			GenOffset = 3;
			OutputMode = 0x08; 
            SpecialSel = 0;
            SpecialType = 0;
			break;
	
			case BXPT_DirecTvMessageType_eRegular_CapFilter0: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter1: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter2: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter3: 
			case BXPT_DirecTvMessageType_eRegular_CapFilter4: 
			GenOffset = 3;
			OutputMode = 0x09; 

            /* Determine which CAP filter is to used. */
            SpecialSel = MessageType - BXPT_DirecTvMessageType_eRegular_CapFilter0;
            SpecialType = 2;
			break;
			 
			/* Regular mode is also the default case. */
			default:
			case BXPT_DirecTvMessageType_eRegular: 
			GenOffset = 3;
			OutputMode = 0x09; 
            SpecialSel = 0;
            SpecialType = 0;
			break;
		}
	
		/* Select the bank the filters are in. */
		RegAddr = BCHP_XPT_MSG_PID_CTRL2_TABLE_i_ARRAY_BASE + ( MesgBufferNum * PID_CTRL2_TABLE_STEP );
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, GEN_OFFSET ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, SPECIAL_TYPE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, SKIP_BYTE2 ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, GEN_GRP_SEL ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, FILTER_MODE ) 
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, GEN_OFFSET, GenOffset ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, SPECIAL_TYPE, SpecialType ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, SKIP_BYTE2, 1 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, GEN_GRP_SEL, GroupSel ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, FILTER_MODE, 0 )
		);
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );
	
		RegAddr = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ARRAY_BASE + ( MesgBufferNum * PID_CTRL1_TABLE_STEP );
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_MODE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_DIS ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, SPECIAL_SEL ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, ALIGN_MODE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, DATA_OUTPUT_MODE ) 
		);
	
		/* Configure the message buffer for capturing PSI messages. */
		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, SPECIAL_SEL, SpecialSel ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, ALIGN_MODE, Settings->ByteAlign == true ? 0 : 1 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS, BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DIRECTV_SAVE_FLAGS_FIRST ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, DATA_OUTPUT_MODE, OutputMode ) 
		);

		/* CRC checks can be disabled on a per-PID channel basis. Do this here if requested in the settings struct. */
		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_MODE, 0 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, ERR_CK_DIS, Settings->CrcDisable == true ? 1 : 0 ) 
		);
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );

		/* Enable data from the PID channel to the mesg block. */
		BXPT_P_SetPidChannelDestination( hXpt, PidChannelNum, 6, true );

		/* Set this PID channel as allocated, in case they forced the channel assignment. */
		hXpt->PidChannelTable[ PidChannelNum ].IsAllocated = true;

		/* Configure the PID channel */
		ExitCode = BXPT_ConfigurePidChannel( hXpt, PidChannelNum, Settings->Pid, Settings->Band );
        if( ExitCode != BERR_SUCCESS )
            goto Done;

		/* 
		** Some filtering configs were done in the parser band on older chips. Now, those
		** configs are done per-PID channel. 
		*/
		BXPT_P_ApplyParserPsiSettings( hXpt, Settings->Band, false );	/* Check IB parsers */
		BXPT_P_ApplyParserPsiSettings( hXpt, Settings->Band, true );	/* Check playback parsers */

		/* Enable the PID channel. */ 
#if BXPT_MESG_DONT_AUTO_ENABLE_PID_CHANNEL
        /* do nothing. must be explicitly enabled. */
#else
		ExitCode = BERR_TRACE( BXPT_EnablePidChannel( hXpt, PidChannelNum ) );
#endif

	    hXpt->PidChannelTable[ PidChannelNum ].MessageBuffercount++;
	}
	else
	{
        /* Wrong function call. Complain. */
        BDBG_ERR(( "Pid2BuffMapping is OFF. You might need to call BXPT_SetPid2Buff() to turn it ON" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}

	Done:
	return( ExitCode );
}


BERR_Code BXPT_Mesg_StopDirecTvMessageCapture( 
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int ScidChannelNum,		/* [in] Which SCID channel. */
	unsigned int MesgBufferNum     /* [in] Which Message Buffer. */
	)
{
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	return( BERR_TRACE(BXPT_Mesg_StopPidChannelRecord( hXpt, ScidChannelNum, MesgBufferNum )) );
}
#endif

BERR_Code BXPT_DirecTv_StartScidChannelRecord( 
	BXPT_Handle hXpt, 							/* [in] Handle for this transport. */
	unsigned int PidChannelNum, 					/* [in] Which PID channel. */
	BXPT_ScidChannelRecordSettings *Settings 	/* [in] SCID, etc. to record. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
	BDBG_ASSERT( Settings );

	/* Sanity check on the arguments. */
	if( PidChannelNum >= hXpt->MaxPidChannels )	   
	{
		/* Bad parser band number. Complain. */
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else if( hXpt->MesgBufferIsInitialized[ PidChannelNum ] == false )
	{
		BDBG_ERR(( "Message buffer for this channel not configured!" ));
		ExitCode = BERR_TRACE( BXPT_ERR_MESG_BUFFER_NOT_CONFIGURED );
	}
	else
	{
		uint32_t Reg;
		
		uint32_t DataOutputMode = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_DATA_OUTPUT_MODE_NO_OUTPUT;
		uint32_t HdFilterMode = 0;
		uint32_t RegAddr = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CTRL1_TABLE_STEP );

		/* Disable the PSI filters for this PID channel. Older chip didn't allow PSI filtering of PES or PacketSaveAll data. */
		BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE + ( PidChannelNum * GEN_FILT_EN_STEP ), 0 );		
		RegAddr = BCHP_XPT_MSG_PID_CTRL2_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CTRL2_TABLE_STEP );
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, SPECIAL_TYPE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL2_TABLE_i, FILTER_MODE ) 
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, SPECIAL_TYPE, 0 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL2_TABLE_i, FILTER_MODE, 0 )
		);
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );

		RegAddr = BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CTRL1_TABLE_STEP );
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, SPECIAL_SEL ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, ALIGN_MODE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_PES_HD_FILT_MODE ) |
			BCHP_MASK( XPT_MSG_PID_CTRL1_TABLE_i, DATA_OUTPUT_MODE ) 
		);

		/* Configure the message buffer for the type of recording. */
		switch( Settings->RecordType )
		{
			case BXPT_DirecTvRecordType_eAudio:	 
			HdFilterMode = 1;			/* HD filter accepts audio only */ 
			DataOutputMode = 0x04;		/* DIRECTV_PES */
			break;

			case BXPT_DirecTvRecordType_eVideo:
			HdFilterMode = 0;			/* HD filter accepts audio and video */ 
			DataOutputMode = 0x04;		/* DIRECTV_PES */
			break;

			case BXPT_DirecTvRecordType_ePacketSaveAll:
			HdFilterMode = 0;			/* Ignored when capturing DirecTV transport packets */ 
			DataOutputMode = 0x02;		/* DIRECTV_TS */
			break;

			default:
			BDBG_ERR(( "Unsupported/illegal RecordType: %u!", ( unsigned ) Settings->RecordType ));
			goto Done;
		}

		Reg |= (
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, ALIGN_MODE, Settings->ByteAlign == true ? 0 : 1 ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, DIRECTV_PES_HD_FILT_MODE, HdFilterMode ) |
			BCHP_FIELD_DATA( XPT_MSG_PID_CTRL1_TABLE_i, DATA_OUTPUT_MODE, DataOutputMode ) 
		);
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );

		/* Enable data from the PID channel to the mesg block. */
		BXPT_P_SetPidChannelDestination( hXpt, PidChannelNum, 6, true );
	
		/* Set this PID channel as allocated, in case they forced the channel assignment. */
		hXpt->PidChannelTable[ PidChannelNum ].IsAllocated = true;
	
		/* Configure the PID channel */
		ExitCode = BXPT_ConfigurePidChannel( hXpt, PidChannelNum, Settings->Pid, Settings->Band );
        if( ExitCode != BERR_SUCCESS )
            goto Done;

		/* Enable the PID channel. */ 
#if BXPT_MESG_DONT_AUTO_ENABLE_PID_CHANNEL
        /* do nothing. must be explicitly enabled. */
#else
		ExitCode = BERR_TRACE( BXPT_EnablePidChannel( hXpt, PidChannelNum ) );
#endif
	}

	Done:
	return( ExitCode );
}


BERR_Code BXPT_DirecTv_StopScidChannelRecord( 
	BXPT_Handle hXpt, 	  						/* [in] Handle for this transport */
	unsigned int ScidChannelNum 						/* [in] Which SCID channel. */
	)
{
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	return( BERR_TRACE(BXPT_StopPidChannelRecord( hXpt, ScidChannelNum )) );
}

BERR_Code BXPT_DirecTv_SetCapPattern( 
	BXPT_Handle hXpt, 	  		/* [in] Handle for this transport */
	unsigned AddressFilter,  	/* [in] Which address filter gets the pattern. */
	uint32_t Pattern  	 	 	/* [in] The pattern to load. */
	)
{
	uint32_t RegAddr;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	if( AddressFilter > BXPT_P_MAX_CAP_FILTERS )
	{
		BDBG_ERR(( "AddressFilter %lu is out of range!", ( unsigned long ) AddressFilter ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else
	{
		RegAddr = BCHP_XPT_MSG_C_MATCH0 + ( AddressFilter * CAP_FILT_REG_STEPSIZE );
		BREG_Write32( hXpt->hRegister, RegAddr, Pattern );
	}
	
	return( ExitCode );
}

#endif

BERR_Code BXPT_DirecTv_ConfigHdFiltering( 
	BXPT_Handle hXpt, 	  				/* [in] Handle for this transport */
	unsigned int PidChannelNum,  		/* [in] Which PID channel. */
	bool EnableFilter, 					/* [in] Which SCID channel. */
	BXPT_HdFilterMode FilterMode		/* [in] HD values to filter on. Ignored if EnableFilter == false */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	/* Sanity check on the arguments. */
	if( PidChannelNum >= hXpt->MaxPidChannels )	   
	{
		/* Bad parser band number. Complain. */
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else
	{
		uint32_t Reg;
        unsigned OldHdEn = 255;
        unsigned OldFilterMode = 255;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
	BXPT_P_PacketSubCfg PsubCfg;
#endif

		uint32_t RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PID_CHANNEL_STEPSIZE * PidChannelNum );
#ifdef ENABLE_PLAYBACK_MUX
		/*gain access to the pid table*/
		BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        OldHdEn = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER );
        OldFilterMode = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_EN_HD_FILTER_TYPE );

		if( EnableFilter == true )
		{
			Reg &= ~( 
				BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
				BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_EN_HD_FILTER_TYPE ) 
				);
			Reg |= ( 
				BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER, 1 ) |
				BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_EN_HD_FILTER_TYPE, FilterMode ) 
				);
		}
		else
		{
			Reg &= ~( 
				BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) 
				);
		}

        if( EnableFilter != OldHdEn || FilterMode != OldFilterMode )
        {
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
			BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
    		BREG_Write32( hXpt->hRegister, RegAddr, Reg );		
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
			BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
        }

#ifdef ENABLE_PLAYBACK_MUX
		/*leave pid table protected area*/
		BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
	}

	return( ExitCode );
}


/* end of file */

