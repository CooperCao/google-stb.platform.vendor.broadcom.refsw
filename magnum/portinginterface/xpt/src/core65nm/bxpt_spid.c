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
#include "bxpt_spid.h"
#include "bchp_xpt_fe.h"

#include "bxpt.h"
#if BXPT_HAS_PACKETSUB
#include "bchp_xpt_psub.h"
#include "bxpt_packetsub.h"
#endif

BDBG_MODULE( xpt_spid );


BERR_Code BXPT_Spid_ConfigureChannel(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned int ChannelNum, 	/* [in] Which secondary channel to configure. */
    unsigned int Spid, 				/* [in] Secondary PID to use. */
	BXPT_Spid_eChannelMode Mode 	/* [in] Mode for secondary PIDs on this channel. */
	)
{
	uint32_t Reg, RegAddr;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
	BXPT_P_PacketSubCfg PsubCfg;
#endif

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	/* Sanity check on the arguments. */
	if( Spid >= 0x2000 )
	{								  
		/* Bad PID. Complain. */
		BDBG_ERR(( "Spid %lu is out of range!", ( unsigned long ) Spid ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else if( ChannelNum >= hXpt->MaxPidChannels )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) ChannelNum ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else							 		  
	{
		RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * ChannelNum );
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_SaveCfg( hXpt, ChannelNum, &PsubCfg );
#endif
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );

		Reg &= ~( 
			BCHP_MASK( XPT_FE_SPID_TABLE_i, SPID_MODE ) |
#if BXPT_HAS_PID_CHANNEL_PES_FILTERING
            BCHP_MASK( XPT_FE_SPID_TABLE_i, PID_FUNCTIONS_SPID_CHANNEL_PID ) 
#else
			BCHP_MASK( XPT_FE_SPID_TABLE_i, SPID_CHANNEL_PID )
#endif
			);

		Reg |= ( 
			BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SPID_MODE, Mode ) |
#if BXPT_HAS_PID_CHANNEL_PES_FILTERING
			BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, PID_FUNCTIONS_SPID_CHANNEL_PID, Spid )
#else
            BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SPID_CHANNEL_PID, Spid )
#endif
			);
		BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_RestoreCfg( hXpt, ChannelNum, &PsubCfg );
#endif
	}

	return( ExitCode );
}


#if BXPT_HAS_PID_CHANNEL_PES_FILTERING

BERR_Code BXPT_Spid_P_ConfigureChannelFilter(
	BXPT_Handle hXpt, 	   			/* [in] Handle for this transport */
	unsigned int ChannelNum,    	/* [in] Which secondary channel to configure. */
	BXPT_Spid_eChannelFilter Filter /* [in] Set a filter to use with this channel */
	)
{
	uint32_t Reg, RegAddr,i;

	BERR_Code ExitCode = BERR_SUCCESS;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
	BXPT_P_PacketSubCfg PsubCfg;
#endif

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

	if( ChannelNum >= hXpt->MaxPidChannels )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) ChannelNum ));
		ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	else							 		  
	{
		RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * ChannelNum );
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_SaveCfg( hXpt, ChannelNum, &PsubCfg );
#endif
		Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        i = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, SPID_MODE );

        /* throw an error if SPID channel is being used for SPID functions */
        if(i > BXPT_Spid_eChannelMode_Disable && i <= BXPT_Spid_eChannelMode_Remap)
        {
            BDBG_ERR(("PidChannelNum %lu already in use for SPID functions!",( unsigned long ) ChannelNum ));
            ExitCode = BERR_TRACE( BERR_UNKNOWN );
        }
		Reg &= ~( 
			BCHP_MASK( XPT_FE_SPID_TABLE_i, SPID_MODE ) |
			BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_HI ) |
            BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_LO )
			);

		Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SPID_MODE, Filter.Mode );

        switch(Filter.Mode)
        {
        case BXPT_Spid_eChannelFilterMode_StreamId:
            Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_FILTER_STREAM_ID, Filter.FilterConfig.StreamId );
            break;
        case BXPT_Spid_eChannelFilterMode_StreamIdRange:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_HI, Filter.FilterConfig.StreamIdRange.Hi ) |
                     BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_LO, Filter.FilterConfig.StreamIdRange.Lo )
                     );
            break;
        case BXPT_Spid_eChannelFilterMode_StreamIdExtension:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_EXTENSION_FILTER_STREAM_ID, Filter.FilterConfig.StreamIdAndExtension.Id ) |
                     BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, STREAM_ID_EXTENSION_FILTER_STREAM_ID_EXTENSION, Filter.FilterConfig.StreamIdAndExtension.Extension )
                     );
            break;
        case BXPT_Spid_eChannelFilterMode_SubStreamId:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SUBSTREAM_ID_FILTER_STREAM_ID, Filter.FilterConfig.StreamIdAndSubStreamId.Id ) |
                     BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, SUBSTREAM_ID_FILTER_SUBSTREAM_ID, Filter.FilterConfig.StreamIdAndSubStreamId.SubStreamId )
                     );
            break;
        case BXPT_Spid_eChannelFilterMode_Disable:
            break;
        default:
            BDBG_ERR(("Invalid filter mode %lu",Filter.Mode));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        }
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_RestoreCfg( hXpt, ChannelNum, &PsubCfg );
#endif
	}

	return( ExitCode );
}

#endif 



