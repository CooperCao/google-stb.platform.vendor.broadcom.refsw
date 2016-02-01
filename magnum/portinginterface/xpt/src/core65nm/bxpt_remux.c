/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#include "bxpt_remux.h"
#include "bxpt_remux_private.h"
#include "bchp_xpt_fe.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if BXPT_HAS_XCBUF
#include "bxpt_xcbuf.h"
#endif
#if BXPT_HAS_RSBUF
#include "bxpt_rsbuf.h"
#endif

#include "bchp_xpt_rmx0.h"

#include "bxpt.h"
#if BXPT_HAS_PACKETSUB
#include "bchp_xpt_psub.h"
#include "bxpt_packetsub.h"
#endif

#if BXPT_SEPARATE_REMUX_IO
    #include "bchp_xpt_rmx0_io.h"
    #if BXPT_P_MAX_REMULTIPLEXORS > 1
        #include "bchp_xpt_rmx1_io.h"
    #endif
#endif

#if BXPT_P_MAX_REMULTIPLEXORS > 1
    #include "bchp_xpt_rmx1.h"
#endif

BDBG_MODULE( xpt_remux );

#define BXPT_P_REMUX_DEFAULT_OUTPUT_CLOCK   	BXPT_RemuxClock_e81Mhz 
#define BXPT_P_REMUX_DEFAULT_HOLD_LEVEL			BXPT_RemuxHoldLevel_eNever
#define BXPT_P_REMUX_DEFAULT_TIMEBASE			BXPT_RemuxTimeBase_e27Mhz
#define BXPT_P_REMUX_DEFAULT_PACKET_DELAY		( 0 )
#define BXPT_P_REMUX_DEFAULT_USE_PCR_TIMEBASE 	false
#define BXPT_P_REMUX_DEFAULT_WHICH_PCR 			( 0 )
#define BXPT_P_REMUX_PCR_BAND_A_DEFAULT			( 0x1F )
#define BXPT_P_REMUX_PCR_BAND_B_DEFAULT			( 0x1F )
#define BXPT_P_REMUX_PARALLEL_EN_DEFAULT		false
#define BXPT_P_REMUX_PARALLEL_INV_CLK_DEFAULT	false
#define BXPT_P_REMUX_PARALLEL_INV_SYNC_DEFAULT	false
#define BXPT_P_REMUX_INV_CLK_DEFAULT			false
#define BXPT_P_REMUX_INV_SYNC_DEFAULT			false
#define BXPT_P_REMUX_BYTE_SYNC_DEFAULT			false

#define SPID_CHNL_STEPSIZE	( 4 )

#if BXPT_HAS_REMUX_PID_REMAPPING

static void PidMapControl(
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int PmIndex,				/* [in] Which PID map. */
	unsigned int NewValue
	);

static void PacketSubControl( 
	BXPT_Remux_Handle hRmx,
	unsigned int WhichTable,
	unsigned int Setting
	);

#endif

static BERR_Code AddPidChannel(
	BXPT_Remux_Handle hRmx,				
	unsigned int PidChannelNum,
	bool UseRPipe 			
	);


BERR_Code BXPT_Remux_GetTotalChannels(
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int *TotalChannels		/* [out] The number of remux channels. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	BSTD_UNUSED( hXpt ); 
	
	*TotalChannels = BXPT_P_MAX_REMULTIPLEXORS;

	return( ExitCode );
}

BERR_Code BXPT_Remux_GetChannelDefaultSettings(
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int ChannelNo,			/* [in] Which channel to get defaults from. */
	BXPT_Remux_ChannelSettings *ChannelSettings /* [out] The defaults */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	BSTD_UNUSED( hXpt ); 
	BDBG_ASSERT( ChannelSettings );

	if( ChannelNo >= BXPT_P_MAX_REMULTIPLEXORS )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "ChannelNo %lu is out of range!", ( unsigned long ) ChannelNo ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		ChannelSettings->OutputClock = BXPT_P_REMUX_DEFAULT_OUTPUT_CLOCK;
		ChannelSettings->PacketDelayCount = BXPT_P_REMUX_DEFAULT_PACKET_DELAY;
		ChannelSettings->ParallelEn = BXPT_P_REMUX_PARALLEL_EN_DEFAULT;			
		ChannelSettings->ParallelInvertClk = BXPT_P_REMUX_PARALLEL_INV_CLK_DEFAULT;		
		ChannelSettings->ParallelInvertSync = BXPT_P_REMUX_PARALLEL_INV_SYNC_DEFAULT;	
		ChannelSettings->InvertClk = BXPT_P_REMUX_INV_CLK_DEFAULT;		
		ChannelSettings->InvertSync = BXPT_P_REMUX_INV_SYNC_DEFAULT;	
		ChannelSettings->ByteSync = BXPT_P_REMUX_BYTE_SYNC_DEFAULT;	
        ChannelSettings->PauseEn = false;
		ChannelSettings->InsertNullPackets = true;
#if BXPT_HAS_RMX_NULL_THRESHOLD
		ChannelSettings->NullPacketInsertionThreshold = 16;
#endif

#if BXPT_HAS_REMUX_PID_REMAPPING
		ChannelSettings->HoldLevel = BXPT_P_REMUX_DEFAULT_HOLD_LEVEL;
		ChannelSettings->RmxATimeBase = BXPT_P_REMUX_DEFAULT_TIMEBASE;
		ChannelSettings->RmxBTimeBase = BXPT_P_REMUX_DEFAULT_TIMEBASE;
		ChannelSettings->UsePcrTimeBase = BXPT_P_REMUX_DEFAULT_USE_PCR_TIMEBASE;
		ChannelSettings->WhichPcrToUse = BXPT_P_REMUX_DEFAULT_WHICH_PCR;
		ChannelSettings->RmxBandA = BXPT_P_REMUX_PCR_BAND_A_DEFAULT;
		ChannelSettings->RmxBandB = BXPT_P_REMUX_PCR_BAND_B_DEFAULT;
#endif
	}

	return( ExitCode );
}

BERR_Code BXPT_Remux_OpenChannel(
	BXPT_Handle hXpt, 	   					/* [in] Handle for this transport */
	BXPT_Remux_Handle *hRmx,			/* [out] Handle for opened remux channel */
	unsigned int ChannelNo,						/* [in] Which channel to open. */
	BXPT_Remux_ChannelSettings *ChannelSettings /* [in] The defaults to use */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;
	BXPT_Remux_Handle lhRmx = NULL;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
	BDBG_ASSERT( ChannelSettings );							 

	if( ChannelNo >= BXPT_P_MAX_REMULTIPLEXORS )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "ChannelNo %lu is out of range!", ( unsigned long ) ChannelNo ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t Reg;
#if BXPT_HAS_XCBUF
        unsigned XcBufId;
#endif

		uint32_t BaseAddr = 0;

		/* 
		** Use the address of the first register in the remux block as the 
		** base address of the entire block. 
		*/
		switch( ChannelNo )
		{
			case 0: 
			BaseAddr = BCHP_XPT_RMX0_CTRL; 
			break;

#if BXPT_P_MAX_REMULTIPLEXORS > 1

            case 1: 
            BaseAddr = BCHP_XPT_RMX1_CTRL; 
            break;
#endif

			default:
			BDBG_ERR(( "Internal error" ));
			goto Done;
		}

		lhRmx = &hXpt->RemuxHandles[ ChannelNo ];
		lhRmx->vhXpt = ( void * ) hXpt;
		lhRmx->hChip = hXpt->hChip;
		lhRmx->hRegister = hXpt->hRegister;
		lhRmx->hMemory = hXpt->hMemory;
		lhRmx->BaseAddr = BaseAddr;
		lhRmx->ChannelNo = ChannelNo;
		lhRmx->Running = false;        

	    BDBG_OBJECT_SET(lhRmx, bxpt_t);

#if BXPT_HAS_REMUX_PID_REMAPPING
        {
            unsigned i;

		    uint32_t RegAddr = BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_BASE - BCHP_XPT_RMX0_CTRL + BaseAddr;

		    for( i = 0; i < BXPT_P_MAX_REMUX_PID_MAPS; i++ )
		    {
			    lhRmx->PidMapEntryAllocated[ i ] = false;

			    BREG_Write32( hXpt->hRegister, RegAddr + ( 4 * i ), 0 );
		    }
        }
#endif

		/* 
		** Holding is done in the XC buffers. The threshold isn't programmable on a per-client
		** basis, so we just enable holds if *any* hold level other than 'eNever' is selected. 
		*/
#if BXPT_HAS_XCBUF

#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7420 ) || \
    ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7630 )
        for( XcBufId = BXPT_XcBuf_Id_RMX0_IBP0; XcBufId < BXPT_XcBuf_Id_RMX1_PBP1; XcBufId++ )
		    BXPT_P_XcBuf_Pause( hXpt, XcBufId, true );	
#elif ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  )  || ( BCHP_CHIP == 7342 )
        for( XcBufId = BXPT_XcBuf_Id_RMX0_IBP0; XcBufId < BXPT_XcBuf_Id_RMX1_PBP4; XcBufId++ )
		    BXPT_P_XcBuf_Pause( hXpt, XcBufId, true );	
#elif ( BCHP_CHIP == 3563 )
       /* 3563 has only 1 remux. */
    BSTD_UNUSED( XcBufId );
	BXPT_P_XcBuf_Pause( hXpt, BXPT_XcBuf_Id_RMX0_A, true );	
	BXPT_P_XcBuf_Pause( hXpt, BXPT_XcBuf_Id_RMX0_B, true );	
#else
        BSTD_UNUSED( XcBufId );
		switch( ChannelNo )
		{                                                                    
			case 0: 
			BXPT_P_XcBuf_Pause( hXpt, BXPT_XcBuf_Id_RMX0_A, true );	
			BXPT_P_XcBuf_Pause( hXpt, BXPT_XcBuf_Id_RMX0_B, true );	
			break;

			case 1: 
			BXPT_P_XcBuf_Pause( hXpt, BXPT_XcBuf_Id_RMX1_A, true );	
			BXPT_P_XcBuf_Pause( hXpt, BXPT_XcBuf_Id_RMX1_B, true );	
			break;
		}
#endif

#endif

#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
	BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif

#if BXPT_SEPARATE_REMUX_IO

		/* Use MPEG defaults. The BXPT_DirecTv_Remux APIs can change that. */
		Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_CTRL );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_CTRL, RMX_PKT_LENGTH ) |
		    BCHP_MASK( XPT_RMX0_CTRL, RMX_CLK_SEL ) |
			BCHP_MASK( XPT_RMX0_CTRL, RMX_PKT_MODE ) 
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PKT_LENGTH, 188 ) |
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_CLK_SEL, 0 ) |
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PKT_MODE, 0 ) 
		);
		BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_CTRL, Reg );

		Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_IO_FORMAT );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_IO_FORMAT, RMXP_INVERT_CLK ) |
			BCHP_MASK( XPT_RMX0_IO_FORMAT, RMXP_INVERT_SYNC ) |
			BCHP_MASK( XPT_RMX0_IO_FORMAT, RMXP_ENABLE ) |
			BCHP_MASK( XPT_RMX0_IO_FORMAT, RMX_INVERT_SYNC ) |
			BCHP_MASK( XPT_RMX0_IO_FORMAT, RMX_INVERT_CLK ) |
			BCHP_MASK( XPT_RMX0_IO_FORMAT, RMX_BYTE_SYNC ) |
			BCHP_MASK( XPT_RMX0_IO_FORMAT, RMX_MUTE )
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMXP_INVERT_CLK, ChannelSettings->ParallelInvertClk ) |
			BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMXP_INVERT_SYNC, ChannelSettings->ParallelInvertSync ) |
			BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMXP_ENABLE, ChannelSettings->ParallelEn ) |
			BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMX_INVERT_SYNC, ChannelSettings->InvertSync ) |
			BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMX_INVERT_CLK, ChannelSettings->InvertClk ) |
			BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMX_BYTE_SYNC, ChannelSettings->ByteSync ) |
			BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMX_MUTE, 0 )
		);
		BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_IO_FORMAT, Reg );
	
		Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_IO_PKT_DLY_CNT );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_IO_PKT_DLY_CNT, RMX_PKT_DLY_CNT )
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_IO_PKT_DLY_CNT, RMX_PKT_DLY_CNT, ChannelSettings->PacketDelayCount )
		);
		BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_IO_PKT_DLY_CNT, Reg );
#else
        {
		    unsigned int TimeBaseSel;

		    Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_PCR_CTRL );
		    Reg &= ~(
			    BCHP_MASK( XPT_RMX0_PCR_CTRL, RMXB_TIMEBASE_SEL ) |
			    BCHP_MASK( XPT_RMX0_PCR_CTRL, RMXA_TIMEBASE_SEL ) |
			    BCHP_MASK( XPT_RMX0_PCR_CTRL, RMXB_BAND_NUM ) |
			    BCHP_MASK( XPT_RMX0_PCR_CTRL, RMXA_BAND_NUM )
		    );
		    Reg |= (
			    BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, RMXB_TIMEBASE_SEL, ChannelSettings->RmxBTimeBase ) |
			    BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, RMXA_TIMEBASE_SEL, ChannelSettings->RmxATimeBase ) |
			    BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, RMXB_BAND_NUM, ChannelSettings->RmxBandB ) |
			    BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, RMXA_BAND_NUM, ChannelSettings->RmxBandA )
		    );
		    BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_PCR_CTRL, Reg );

		    /* Use MPEG defaults. The BXPT_DirecTv_Remux APIs can change that. */
		    Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_FORMAT );
		    Reg &= ~(
			    BCHP_MASK( XPT_RMX0_FORMAT, RMX_PKT_LENGTH ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMXP_INVERT_CLK ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMXP_INVERT_SYNC ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMXP_ENABLE ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMX_PKT_MODE ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMX_INVERT_SYNC ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMX_INVERT_CLK ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMX_BYTE_SYNC ) |
			    BCHP_MASK( XPT_RMX0_FORMAT, RMX_MUTE )
		    );
		    Reg |= (
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_PKT_LENGTH, 188 ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMXP_INVERT_CLK, ChannelSettings->ParallelInvertClk ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMXP_INVERT_SYNC, ChannelSettings->ParallelInvertSync ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMXP_ENABLE, ChannelSettings->ParallelEn ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_PKT_MODE, 0 ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_INVERT_SYNC, ChannelSettings->InvertSync ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_INVERT_CLK, ChannelSettings->InvertClk ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_BYTE_SYNC, ChannelSettings->ByteSync ) |
			    BCHP_FIELD_DATA( XPT_RMX0_FORMAT, RMX_MUTE, 0 )
		    );
		    BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_FORMAT, Reg );
	
		    Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_PKT_DLY_CNT );
		    Reg &= ~(
			    BCHP_MASK( XPT_RMX0_PKT_DLY_CNT, RMX_PKT_DLY_CNT )
		    );
		    Reg |= (
			    BCHP_FIELD_DATA( XPT_RMX0_PKT_DLY_CNT, RMX_PKT_DLY_CNT, ChannelSettings->PacketDelayCount )
		    );
		    BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_PKT_DLY_CNT, Reg );

		    Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_RMX_PACING );
		    Reg &= ~( 
			    BCHP_MASK( XPT_RMX0_RMX_PACING, TIMEBASE_SEL ) |
			    BCHP_MASK( XPT_RMX0_RMX_PACING, TIMESTAMP_MODE ) 
			    );

		    if( ChannelSettings->UsePcrTimeBase == false )
		    {
			    /* Use the free running 27 MHz clock. */
			    TimeBaseSel = 0;
		    }
		    else
		    {
			    if( ChannelSettings->WhichPcrToUse >= BXPT_P_MAX_PCRS ) 
			    {
				    /* Bad PCR module number. Complain, and default to free running 27 MHz clock. */
				    BDBG_ERR(( "WhichPcrToUse %lu is out of range!", ( unsigned long ) ChannelSettings->WhichPcrToUse ));
				    ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
				    TimeBaseSel = 0;
			    }

			    TimeBaseSel = ChannelSettings->WhichPcrToUse + 1;
		    }

		    Reg |= (
			    BCHP_FIELD_DATA( XPT_RMX0_RMX_PACING, TIMEBASE_SEL, TimeBaseSel ) |
			    BCHP_FIELD_DATA( XPT_RMX0_RMX_PACING, TIMESTAMP_MODE, ChannelSettings->TimestampMode ) 
			    );
		    BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_RMX_PACING, Reg );
        }
#endif

		/* Load the defaults. */
		Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_CTRL );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_CTRL, RMX_CLK_SEL ) |
#if BXPT_HAS_RMX_NULL_THRESHOLD
			BCHP_MASK( XPT_RMX0_CTRL, RMX_NULL_PKT_THRESHOLD ) |
#endif
			BCHP_MASK( XPT_RMX0_CTRL, RMX_NULL_PKT_DIS )
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_CLK_SEL, ChannelSettings->OutputClock ) |
#if BXPT_HAS_RMX_NULL_THRESHOLD
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_NULL_PKT_THRESHOLD, ChannelSettings->NullPacketInsertionThreshold ) |
#endif
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_NULL_PKT_DIS, ChannelSettings->InsertNullPackets ? false : true )
		);
		BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_CTRL, Reg );

#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
		BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif

		lhRmx->Opened = true;
 		*hRmx = lhRmx;
	}

	Done:
	return( ExitCode );
}

void BXPT_Remux_CloseChannel(
	BXPT_Remux_Handle hRmx	/* [in] Handle for the channel to close*/
	)
{
	uint32_t Reg;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	/* Stop remuxing, stop the remapping logic, and packet subbing. */
	if (hRmx->Running) {
		BDBG_WRN(("Stopping remux channel %d before closing", hRmx->ChannelNo));
		BXPT_Remux_DoRemux(hRmx, false);
	}
	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );

#if BXPT_HAS_REMUX_PID_REMAPPING
	Reg &= ~(
		BCHP_MASK( XPT_RMX0_CTRL, RMX_PID_MAP_EN ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_PCR_CORRECT_DIS ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_PSUB2_EN ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_PSUB1_EN ) 
	);

	/* Free up any/all PID remappings. */
	BXPT_Remux_FreeAllPidMaps( hRmx );	
#endif

	Reg &= ~(
		BCHP_MASK( XPT_RMX0_CTRL, RMX_BYPASS ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_CLK_SEL ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_ENABLE )
	);

	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
	hRmx->Opened = false;
	BDBG_OBJECT_UNSET(hRmx, bxpt_t);
}

BERR_Code BXPT_Remux_AddPidChannelToRemux( 
	BXPT_Remux_Handle hRmx,				/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 		/* [in] Which remux input */
	unsigned int PidChannelNum 			/* [in] Which PID channel. */
	)
{
    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);
	BSTD_UNUSED( RemuxInput );

	return AddPidChannel( hRmx, PidChannelNum, false ); 
}

BERR_Code BXPT_Remux_AddRPipePidChannelToRemux( 
	BXPT_Remux_Handle hRmx,				/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 		/* [in] Which remux input */
	unsigned int PidChannelNum 			/* [in] Which PID channel. */
	)
{
    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);
	BSTD_UNUSED( RemuxInput );

	return AddPidChannel( hRmx, PidChannelNum, true ); 
}

static BERR_Code AddPidChannel(
	BXPT_Remux_Handle hRmx,				
	unsigned int PidChannelNum,
	bool UseRPipe 			
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	if( PidChannelNum >= BXPT_P_MAX_PID_CHANNELS )
	{
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t PipeShift;

		/* Set the PID channels enable bit. */
		switch( hRmx->ChannelNo )
		{
			case 0:
			PipeShift = 0;		/* G pipe */
			break;

			case 1:
			PipeShift = 2;		/* G pipe */
			break;

			default:
			BDBG_ERR(( "ChannelNo %lu invalid!", ( unsigned long ) hRmx->ChannelNo ));
			ExitCode = BERR_INVALID_PARAMETER;
			goto Done;
		}

		if( UseRPipe == true )
			PipeShift++;

		BXPT_P_SetPidChannelDestination( (BXPT_Handle) hRmx->vhXpt, PidChannelNum, PipeShift, true );
	}

	Done:
	return( ExitCode );	    
}

BERR_Code BXPT_Remux_DoRemux( 
	BXPT_Remux_Handle hRmx,	/* [in] Handle for the remux channel */
	bool Enable 				/* [in] Remux on if TRUE, or off if FALSE. */
	)
{
	uint32_t Reg;
    BXPT_Handle hXpt;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
	{
		unsigned wasEnabled;
		Reg = BXPT_Remux_P_ReadReg(hRmx, BCHP_XPT_RMX0_CTRL);
		wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_RMX0_CTRL, RMX_ENABLE);

		/* only change refcnt if changing state */
		if (!wasEnabled && Enable) {
			BCHP_PWR_AcquireResource(hRmx->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
		}
		else if (wasEnabled && !Enable) {
			BCHP_PWR_ReleaseResource(hRmx->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
		}
	}
#endif

    hXpt = (BXPT_Handle) hRmx->vhXpt;
	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_ENABLE ) );

    if( Enable )
    {
	    Reg |= BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_ENABLE, 1 );
	    BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
#if BXPT_HAS_XCBUF
        BXPT_P_XcBuf_RemuxPause( hXpt, hRmx->ChannelNo, Enable );
#endif
    }
    else
    {
#if BXPT_HAS_XCBUF
        BXPT_P_XcBuf_RemuxPause( hXpt, hRmx->ChannelNo, Enable );
#endif
    	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
    }    
    
	hRmx->Running = Enable;
    
	return( ExitCode );
}

#if BXPT_SEPARATE_REMUX_IO
#else
void BXPT_Remux_ReloadTimestamps(
	BXPT_Remux_Handle hRmx
	)
{
	uint32_t Reg;

	BDBG_ASSERT( hRmx );

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_PCR_CTRL );
	Reg &= ~(
		BCHP_MASK( XPT_RMX0_PCR_CTRL, READLOAD_RMXB_TS_COUNT ) |
		BCHP_MASK( XPT_RMX0_PCR_CTRL, READLOAD_RMXA_TS_COUNT )
	);
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_PCR_CTRL, Reg );

	Reg |= (
	  BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, READLOAD_RMXB_TS_COUNT, 1 ) |
	  BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, READLOAD_RMXA_TS_COUNT, 1 ) 
	);
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_PCR_CTRL, Reg );
}
#endif

BERR_Code BXPT_Remux_RemovePidChannelFromRemux( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 	/* [in] Which remux input */
	unsigned int PidChannelNum 			/* [in] Which PID channel. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);
	BSTD_UNUSED( RemuxInput );

	if( PidChannelNum >= BXPT_P_MAX_PID_CHANNELS )
	{
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t PipeShift;

		/* Clear the PID channels enable bit. */
		switch( hRmx->ChannelNo )
		{
			case 0:
			PipeShift = 0;		/* G pipe */
			break;

			case 1:
			PipeShift = 2;		/* G pipe */
			break;

			default:
			BDBG_ERR(( "ChannelNo %lu invalid!", ( unsigned long ) hRmx->ChannelNo ));
			ExitCode = BERR_INVALID_PARAMETER;
			goto Done;
		}

		BXPT_P_SetPidChannelDestination( (BXPT_Handle) hRmx->vhXpt, PidChannelNum, PipeShift, false );
	}

	Done:
	return( ExitCode );	    
}

BERR_Code BXPT_Remux_GetPidChannel( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 			/* [in] Which remux input */
	unsigned int PidChannelNum, 			/* [in] Which PID channel. */
	bool *Enable 						/* [out] Where to put channel status. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);
	BSTD_UNUSED( RemuxInput );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
	BXPT_P_PacketSubCfg PsubCfg;
	BXPT_Handle hXpt = (BXPT_Handle)hRmx->vhXpt;
#endif

	*Enable = false;

	if( PidChannelNum >= BXPT_P_MAX_PID_CHANNELS )
	{
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_INVALID_PARAMETER;
		*Enable = false;
	}
	else
	{
		uint32_t Reg, RegAddr, PipeShift;

		/* Set the PID channels enable bit. */
		RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
		Reg = BREG_Read32( hRmx->hRegister, RegAddr );
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
		BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
		
		switch( hRmx->ChannelNo )
		{
			case 0:
			PipeShift = 24;		/* G pipe */
			break;

			case 1:
			PipeShift = 24 + 2;		/* G pipe */
			break;

			default:
			BDBG_ERR(( "ChannelNo %lu invalid!", ( unsigned long ) hRmx->ChannelNo ));
			ExitCode = BERR_INVALID_PARAMETER;
			goto Done;
		}

		if( Reg & ( 3ul << PipeShift ) )
			*Enable = true;
		else
			*Enable = false;
	}

	Done:
	return( ExitCode );	    
}

BERR_Code BXPT_Remux_RemoveAllPidChannel( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput 			/* [in] Which remux input */
	)
{
	unsigned PidChannelNum;
 	uint32_t PipeShift;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);
	BSTD_UNUSED( RemuxInput );

	/* Clear all the PID channels' enable bit. */
	for( PidChannelNum = 0; PidChannelNum < BXPT_P_MAX_PID_CHANNELS; PidChannelNum++ )
	{
		switch( hRmx->ChannelNo )
		{
			case 0:
			PipeShift = 0;		/* G pipe */
			break;

			case 1:
			PipeShift = 2;		/* G pipe */
			break;

			default:
			BDBG_ERR(( "ChannelNo %lu invalid!", ( unsigned long ) hRmx->ChannelNo ));
			ExitCode = BERR_INVALID_PARAMETER;
			goto Done;
		}

		BXPT_P_SetPidChannelDestination( (BXPT_Handle) hRmx->vhXpt, PidChannelNum, PipeShift, false );
	}

	Done:
	return( ExitCode );
}

#if BXPT_HAS_REMUX_PCR_OFFSET

BERR_Code BXPT_Remux_AddPcrOffset( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
    bool AddOffsetEn,		    /* [in] Enable/disable the PCR correction */
    uint32_t FixedPcrOffset			/* [in] The PCR correction offset, if enabled */		
	)
{
	uint32_t Reg;

	BERR_Code ret = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( AddOffsetEn )
 	{
		Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_FIXED_OFFSET );
		Reg &= ~BCHP_MASK( XPT_RMX0_FIXED_OFFSET, RMX_FIXED_OFFSET );
		Reg |= BCHP_FIELD_DATA( XPT_RMX0_FIXED_OFFSET, RMX_FIXED_OFFSET, FixedPcrOffset );
   		BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_FIXED_OFFSET, Reg );
	}

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_PCR_CTRL );
	Reg &= ~BCHP_MASK( XPT_RMX0_PCR_CTRL, RMX_FIXED_OFFSET_EN );
	Reg |= BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, RMX_FIXED_OFFSET_EN, AddOffsetEn == true ? 1 : 0 );	
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_PCR_CTRL, Reg );

	return ret;
}

void BXPT_Remux_SetPcrJitterAdj( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	bool ForPlayback,		/* [in] true = control adjustment for playback, false = control for live */
    	bool EnableAdjust		    /* [in] Enable/disable the PCR jitter adjust */
	)
{
	uint32_t Reg;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_PCR_CTRL );
	if( ForPlayback )
	{
		/* HW uses reverse-logic */
		Reg &= ~BCHP_MASK( XPT_RMX0_PCR_CTRL, RMX_PLAYBACK_PCR_JITTER_DIS );
		Reg |= BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, RMX_PLAYBACK_PCR_JITTER_DIS, EnableAdjust == true ? 0 : 1 );	
	}
	else
	{
		/* HW uses reverse-logic */
		Reg &= ~BCHP_MASK( XPT_RMX0_PCR_CTRL, RMX_LIVE_PCR_JITTER_DIS );
		Reg |= BCHP_FIELD_DATA( XPT_RMX0_PCR_CTRL, RMX_LIVE_PCR_JITTER_DIS, EnableAdjust == true ? 0 : 1 );	
	}
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_PCR_CTRL, Reg );
}

#endif

#if BXPT_HAS_REMUX_PID_REMAPPING

BERR_Code BXPT_Remux_EnablePidMap( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int PmIndex 				/* [in] Which PID map. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( PmIndex >= BXPT_P_MAX_REMUX_PID_MAPS )

	{
		BDBG_ERR(( "PmIndex %lu is out of range!", ( unsigned long ) PmIndex ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		PidMapControl( hRmx, PmIndex, 1 );
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_DisablePidMap( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int PmIndex 				/* [in] Which PID map. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( PmIndex >= BXPT_P_MAX_REMUX_PID_MAPS )
	{
		BDBG_ERR(( "PmIndex %lu is out of range!", ( unsigned long ) PmIndex ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		PidMapControl( hRmx, PmIndex, 0 );
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_AllocPidMap( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int IncomingPid,				/* [in] PID that will be remapped. */
	unsigned int OutgoingPid,				/* [in] New value for the remapped PID. */
	unsigned ParserBand, 			/* [in] Which parser band has the PID to remapped */
	unsigned int *PmIndex 					/* [out] Which PID map entry was allocated to enable. */
	)
{
	unsigned int Index;

	bool FoundOne = false;
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	/* Find an unused entry */
	for( Index = 0; Index < BXPT_P_MAX_REMUX_PID_MAPS; Index++ )
		if( hRmx->PidMapEntryAllocated[ Index ] == false )
		{
			hRmx->PidMapEntryAllocated[ Index ] = true;
			*PmIndex = Index;
			FoundOne = true;
			break;
		}

 	if( FoundOne == false )
 	{
		BDBG_ERR(( "No PID map entry is available!" ));
	 	ExitCode = BXPT_ERR_NO_AVAILABLE_RESOURCES;	
	}
	else
	{
		uint32_t Reg;

		uint32_t RegAddr = BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_BASE + ( 4 * Index );

        /* The RDB does not have macros to map playback parsers, so we do it the hard way... */
	 	if( BXPT_P_IS_PB( ParserBand ) )
		{
			BXPT_P_CLEAR_PB_FLAG( ParserBand );	/* Remove the PB parser flag. */
			ParserBand |= ( 1 << 4 );           /* Set the undocumented PB parser select bit. */
		}

		Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_OUTPUT_PID ) |
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_INPUT_PID ) |
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_BAND_SELECT )
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_PID_MAP_TABLEi, RMX_OUTPUT_PID, OutgoingPid ) |
			BCHP_FIELD_DATA( XPT_RMX0_PID_MAP_TABLEi, RMX_INPUT_PID, IncomingPid ) |
			BCHP_FIELD_DATA( XPT_RMX0_PID_MAP_TABLEi, RMX_BAND_SELECT, ParserBand ) 
		);
		BXPT_Remux_P_WriteReg( hRmx, RegAddr, Reg );
	}

	return( ExitCode );
}

BERR_Code BXPT_Remux_FreePidMap( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int PmIndex 					/* [in] Which PID map entry to free up. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( PmIndex >= BXPT_P_MAX_REMUX_PID_MAPS )
	{
		BDBG_ERR(( "PmIndex %lu is out of range!", ( unsigned long ) PmIndex ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t Reg;

		uint32_t RegAddr = BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_BASE + ( 4 * PmIndex );

		Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_OUTPUT_PID ) |
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_INPUT_PID ) |
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_BAND_SELECT )
		);
		BXPT_Remux_P_WriteReg( hRmx, RegAddr, Reg );
	
		hRmx->PidMapEntryAllocated[ PmIndex ] = false;

		PidMapControl( hRmx, PmIndex, 0 );
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_FreeAllPidMaps( 
	BXPT_Remux_Handle hRmx			/* [in] Handle for the remux channel */
	)
{
	unsigned int i;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	for( i = 0; i < BXPT_P_MAX_REMUX_PID_MAPS; i++ )
	{
		uint32_t Reg;

		uint32_t RegAddr = BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_BASE + ( 4 * i );

		Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_OUTPUT_PID ) |
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_INPUT_PID ) |
			BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_BAND_SELECT )
		);
		BXPT_Remux_P_WriteReg( hRmx, RegAddr, Reg );
	
		hRmx->PidMapEntryAllocated[ i ] = false;

		PidMapControl( hRmx, i, 0 );
	}

	return( ExitCode );
}

BERR_Code BXPT_Remux_SetPktSubTable( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int WhichTable,		/* [in] The remux table to copy into. */
	unsigned int Start, 			/* [in] Where in table to start copying. */
	unsigned int Size, 				/* [in] Number of bytes to copy. */
	const uint8_t *SubData 			/* [in] Bytes to copy. */
	)
{
	uint32_t Reg, WordCount, EndingByte;
	uint32_t RegAddr;
	unsigned int StartingByte;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		/*
		** The substitution table is organized into 47 register, of 4 bytes each. 
		** So figure out which register the new data should be written to, and
		** which byte in that register.
		*/
		RegAddr = WhichTable == 1 ? BCHP_XPT_RMX0_PKT_SUB2_TABLEi_ARRAY_BASE : BCHP_XPT_RMX0_PKT_SUB1_TABLEi_ARRAY_BASE;
		RegAddr += ( Start / 4 );
		StartingByte = Start % 4;

		/* Don't go past the end of the table. */
		if( Size + Start >= BXPT_P_REMUX_SUB_TABLE_SIZE )
			Size = BXPT_P_REMUX_SUB_TABLE_SIZE - Start;
			
		/* 
		** If the table offset isn't aligned on a register boundary, we've
		** got to do a read-modify-write on the register to preserve its
		** contents.
		*/
		if( StartingByte )
		{
		    /* read the register that contains the data */
			Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );

			switch( StartingByte )
			{
				case 1:
				Reg = ( Reg & 0xFF00FFFF ) | ( *SubData++ << 16 );
				Size--;
				break;

				case 2:
				Reg = ( Reg & 0xFFFF00FF ) | ( *SubData++ << 8 );
				Size--;
				break;

				case 3:
				Reg = ( Reg & 0xFFFFFF00 ) | *SubData++;
				Size--;
				break;
			}
		    
			/* Write the data back. Step to the next register. */
			BXPT_Remux_P_WriteReg( hRmx, RegAddr, Reg );
			RegAddr += 4;
		}

		/* Load 32-bit chunks until we run out of them. */
		WordCount = Size / 4;
		while( WordCount-- )
		{
		    /* read the register that contains the data */
			Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );

			Reg = *SubData++ << 24;
			Reg |= *SubData++ << 16;
			Reg |= *SubData++ << 8;
			Reg |= *SubData++;

			/* Write the data back. */
			BXPT_Remux_P_WriteReg( hRmx, RegAddr, Reg );
			RegAddr += 4;
		}

		/* Any data left over? */
		EndingByte = Size % 4;
		if( EndingByte )
		{
		    /* read the register that contains the data */
			Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );

			if( EndingByte >= 1 )
				Reg = ( Reg & 0x00FFFFFF ) | ( *SubData++ << 24 );

			if( EndingByte >= 2 )
				Reg = ( Reg & 0xFF00FFFF ) | ( *SubData++ << 16 );

			if( EndingByte == 3 )
				Reg = ( Reg & 0xFFFF00FF ) | ( *SubData++ << 8 );

			/* Write the data back. */
			BXPT_Remux_P_WriteReg( hRmx, RegAddr, Reg );
		}
	}

	return( ExitCode );
}

BERR_Code BXPT_Remux_GetPktSubTable( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	unsigned int WhichTable,	/* [in] The remux table to copy into. */
	unsigned int Start, 		/* [in] Where in table to start copying. */
	unsigned int Size, 			/* [in] Number of bytes to copy. */
	uint8_t *SubData 		/* [out] Where to copy data to. */
	)
{
	uint32_t Reg, WordCount, EndingByte;
	uint32_t RegAddr;
	int StartingByte;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		/*
		** The substitution table is organized into 47 register, of 4 bytes each. 
		** So figure out which register the new data should be written to, and
		** which byte in that register.
		*/
		RegAddr = WhichTable == 1 ? BCHP_XPT_RMX0_PKT_SUB2_TABLEi_ARRAY_BASE : BCHP_XPT_RMX0_PKT_SUB1_TABLEi_ARRAY_BASE;
		RegAddr += ( Start / 4 );
		StartingByte = Start % 4;

		/* Don't go past the end of the table. */
		if( Size + Start >= BXPT_P_REMUX_SUB_TABLE_SIZE )
			Size = BXPT_P_REMUX_SUB_TABLE_SIZE - Start;
			
		/* 
		** If the table offset isn't aligned on a register boundary, we've
		** got to do a read-modify-write on the register to preserve its
		** contents.
		*/
		if( StartingByte )
		{
		    /* read the register that contains the data */
			Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );

			switch( StartingByte )
			{
				case 1:
				*SubData++ = ( uint8_t )(( Reg & 0x00FF0000 ) >> 16);
				Size--;
				break;

				case 2:
				*SubData++ = ( uint8_t )(( Reg & 0x0000FF00 ) >> 8);
				Size--;
				break;

				case 3:
				*SubData++ = ( uint8_t )( Reg & 0x000000FF );
				Size--;
				break;
			}
			RegAddr += 4;
		}

		/* Load 32-bit chunks until we run out of them. */
		WordCount = Size / 4;
		while( WordCount-- )
		{
		    /* read the register that contains the data */
			Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );

			*SubData++ = ( uint8_t )(( Reg & 0xFF000000 ) >> 24 );
			*SubData++ = ( uint8_t )(( Reg & 0x00FF0000 ) >> 16 );
			*SubData++ = ( uint8_t )(( Reg & 0x0000FF00 ) >> 8 );
			*SubData++ = ( uint8_t )( Reg & 0x000000FF );

			RegAddr += 4;
		}

		/* Any data left over? */
		EndingByte = Size % 4;
		if( EndingByte )
		{
		    /* read the register that contains the data */
			Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );

			if( EndingByte >= 1 )
				*SubData++ = ( uint8_t )(( Reg & 0xFF000000 ) >> 24 );

			if( EndingByte >= 2 )
				*SubData++ = ( uint8_t )(( Reg & 0x00FF0000 ) >> 16 );

			if( EndingByte == 3 )
				*SubData++ = ( uint8_t )(( Reg & 0x0000FF00 ) >> 8 );
		}
	}

	return( ExitCode );
}

BERR_Code BXPT_Remux_SetPktSubPacketMode( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	unsigned int WhichTable,	/* [in] The remux table to copy into. */
	unsigned int Pid, 			/* [in] The target PID. */
	unsigned int Length, 		/* [in] Number of payload bytes to substitute. */
	bool SaveAdaptField			/* [in] Preserve adaptation field? */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else if( Pid >= 0x2000 )
	{
		BDBG_ERR(( "Invalid PID, %lX is out of range!", ( unsigned long ) Pid ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else if( Length >= BXPT_P_REMUX_PSUB_PAYLOAD_SIZE )
	{
		BDBG_ERR(( "Length %lu is out of range!", ( unsigned long ) Length ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t Reg;
		
		uint32_t CtrlRegAddr = 0;
		uint32_t PidRegAddr = 0;

		switch( WhichTable )
		{
			case 0:
			CtrlRegAddr = BCHP_XPT_RMX0_PSUB1_CTRL;
			PidRegAddr = BCHP_XPT_RMX0_PSUB1_PID;
			break;

			case 1:
			CtrlRegAddr = BCHP_XPT_RMX0_PSUB2_CTRL;
			PidRegAddr = BCHP_XPT_RMX0_PSUB2_PID;
			break;
		}

		Reg = BXPT_Remux_P_ReadReg( hRmx, CtrlRegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_MODE ) |
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_ADAPT_EN ) |
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_LENGTH )
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_MODE, 1 ) |
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_ADAPT_EN, SaveAdaptField == true ? 0 : 1 ) |
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_LENGTH, Length )
		);
		BXPT_Remux_P_WriteReg( hRmx, CtrlRegAddr, Reg );

		Reg = BXPT_Remux_P_ReadReg( hRmx, PidRegAddr );
		Reg &= ~( BCHP_MASK( XPT_RMX0_PSUB1_PID, RMX_PSUB1_PID )	);
		Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_PSUB1_PID, RMX_PSUB1_PID, Pid ) );
		BXPT_Remux_P_WriteReg( hRmx, PidRegAddr, Reg );
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_SetPktSubBlockMode( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	unsigned int WhichTable,	/* [in] The remux table to copy into. */
	unsigned int Pid, 			/* [in] The target PID. */
	unsigned int Start, 		/* [in] Starting offset for substitution. */
	unsigned int End			/* [in] Ending offset for substitution. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else if( Pid >= 0x2000 )
	{
		BDBG_ERR(( "Invalid PID, %lX is out of range!", ( unsigned long ) Pid ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else if( Start >= BXPT_P_REMUX_SUB_TABLE_SIZE )
	{
		BDBG_ERR(( "Start %lu is out of range!", ( unsigned long ) Start ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else if( End >= BXPT_P_REMUX_SUB_TABLE_SIZE )
	{
		BDBG_ERR(( "End %lu is out of range!", ( unsigned long ) End ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else if( Start > End )
	{
		BDBG_ERR(( "Starting offset is past ending offset!" ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t Reg;
		
		uint32_t CtrlRegAddr = 0;
		uint32_t PidRegAddr = 0;

		switch( WhichTable )
		{
			case 0:
			CtrlRegAddr = BCHP_XPT_RMX0_PSUB1_CTRL;
			PidRegAddr = BCHP_XPT_RMX0_PSUB1_PID;
			break;

			case 1:
			CtrlRegAddr = BCHP_XPT_RMX0_PSUB2_CTRL;
			PidRegAddr = BCHP_XPT_RMX0_PSUB2_PID;
			break;
		}

		Reg = BXPT_Remux_P_ReadReg( hRmx, CtrlRegAddr );
		Reg &= ~(
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_MODE ) |
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_START ) |
			BCHP_MASK( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_END )
		);
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_MODE, 0 ) |
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_START, Start ) |
			BCHP_FIELD_DATA( XPT_RMX0_PSUB1_CTRL, RMX_PSUB1_END, End )
		);
		BXPT_Remux_P_WriteReg( hRmx, CtrlRegAddr, Reg );

		Reg = BXPT_Remux_P_ReadReg( hRmx, PidRegAddr );
		Reg &= ~( BCHP_MASK( XPT_RMX0_PSUB1_PID, RMX_PSUB1_PID )	);
		Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_PSUB1_PID, RMX_PSUB1_PID, Pid ) );
		BXPT_Remux_P_WriteReg( hRmx, PidRegAddr, Reg );
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_EnablePktSub( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int WhichTable				/* [in] Which remux table. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		PacketSubControl( hRmx, WhichTable, 1 );
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_DisablePktSub( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int WhichTable				/* [in] Which remux table. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		PacketSubControl( hRmx, WhichTable, 0 );
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_GeneratePkt( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int WhichTable				/* [in] Which remux table. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( WhichTable >= BXPT_P_MAX_REMUX_PSUB_TABLES )
	{
		BDBG_ERR(( "WhichTable %lu is out of range!", ( unsigned long ) WhichTable ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t Reg, GenBusy, SubBusy;
		
		uint32_t CtrlRegAddr = 0;

		switch( WhichTable )
		{
			case 0:
			CtrlRegAddr = BCHP_XPT_RMX0_PKT_GEN1_CTRL;
			break;

			case 1:
			CtrlRegAddr = BCHP_XPT_RMX0_PKT_GEN2_CTRL;
			break;
		}

		Reg = BXPT_Remux_P_ReadReg( hRmx, CtrlRegAddr );
		GenBusy = BCHP_GET_FIELD_DATA( Reg, XPT_RMX0_PKT_GEN1_CTRL, RMX_PKT_GEN1_BUSY );
		SubBusy = BCHP_GET_FIELD_DATA( Reg, XPT_RMX0_PKT_GEN1_CTRL, RMX_PKT_SUB1_BUSY );

		if( GenBusy )
		{
			BDBG_ERR(( "Packet Gen %lu is BUSY!", ( unsigned long ) WhichTable ));
		}
		else if( SubBusy )
		{
			BDBG_ERR(( "Packet Sub %lu is BUSY!", ( unsigned long ) WhichTable ));
		}
		else
		{
			Reg &= ~( BCHP_MASK( XPT_RMX0_PKT_GEN1_CTRL, RMX_PKT_GEN1_BUSY )	);
			Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_PKT_GEN1_CTRL, RMX_PKT_GEN1_BUSY, 1 ) );
			BXPT_Remux_P_WriteReg( hRmx, CtrlRegAddr, Reg );
		}
	}

	return( ExitCode );	    
}

BERR_Code BXPT_Remux_SetPcrCorrection( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
    bool PcrCorrectEn,		    /* [in] Enable/disable the PCR correction */
    uint32_t PcrOffset			/* [in] The PCR correction offset, if enabled */		
	)
{
	uint32_t Reg;

	BERR_Code ret = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	if( PcrCorrectEn )
 	{
		Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_PCR_OFFSET );
		Reg &= ~BCHP_MASK( XPT_RMX0_PCR_OFFSET, RMX_PCR_OFFSET );
		Reg |= BCHP_FIELD_DATA( XPT_RMX0_PCR_OFFSET, RMX_PCR_OFFSET, PcrOffset );
   		BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_PCR_OFFSET, Reg );
	}

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	Reg &= ~BCHP_MASK( XPT_RMX0_CTRL, RMX_PCR_CORRECT_DIS );
	Reg |= BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PCR_CORRECT_DIS, PcrCorrectEn == true ? 0 : 1 );	/* Inverted logic.. */
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );

	return ret;
}

BERR_Code BXPT_Remux_ConfigPacing(
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	BXPT_PacingControl Mode					/* [in] New mode for pacing. */
	)
{
	uint32_t Reg; 

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_RMX_PACING );

	/* 
	** Init the Pacing bitfields to 0. The PACING_START bit must transistion from 0 to 1
	** later on in order to arm the pacing logic. 
	*/
   	Reg &= ~( 
   		BCHP_MASK( XPT_RMX0_RMX_PACING, PACING_START ) |
   		BCHP_MASK( XPT_RMX0_RMX_PACING, PACING_EN ) |
   		BCHP_MASK( XPT_RMX0_RMX_PACING, PACING_AUTOSTART_EN ) 
   		);

	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_RMX_PACING, Reg );

	if( Mode == BXPT_PacingControl_eStart )
	{
		Reg |= (
			BCHP_FIELD_DATA( XPT_RMX0_RMX_PACING, PACING_START, 1 ) |
			BCHP_FIELD_DATA( XPT_RMX0_RMX_PACING, PACING_EN, 1 ) |

			/*
			** Autostart will re-arm the pacing logic automatically if a 
			** force-resync condition occurrs.
			*/ 
			BCHP_FIELD_DATA( XPT_RMX0_RMX_PACING, PACING_AUTOSTART_EN, 1 )
		);  
	}

	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_RMX_PACING, Reg );

	return( ExitCode );
}

BERR_Code BXPT_Remux_SetPacingErrorBound(
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	unsigned long MaxTsError		/* [in] Maximum timestamp error. */
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_RMX_TS_ERR_BOUND );
   	Reg &= ~( 
   		BCHP_MASK( XPT_RMX0_RMX_TS_ERR_BOUND, RMX_TS_ERR_BOUND ) 
   		);
	Reg |= (
		BCHP_FIELD_DATA( XPT_RMX0_RMX_TS_ERR_BOUND, RMX_TS_ERR_BOUND, MaxTsError )
	);  
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_RMX_TS_ERR_BOUND, Reg );

	return( ExitCode );
}

static void PidMapControl(
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	unsigned int PmIndex,				/* [in] Which PID map. */
	unsigned int NewValue
	)
{
	uint32_t Reg;
	unsigned int MapIndex;

	unsigned RemapEnable = 0;
	uint32_t RegAddr = BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_BASE + ( 4 * PmIndex );

	Reg = BXPT_Remux_P_ReadReg( hRmx, RegAddr );
	Reg &= ~BCHP_MASK( XPT_RMX0_PID_MAP_TABLEi, RMX_MAP_ENABLE );
	Reg |= BCHP_FIELD_DATA( XPT_RMX0_PID_MAP_TABLEi, RMX_MAP_ENABLE, NewValue );
	BXPT_Remux_P_WriteReg( hRmx, RegAddr, Reg );

    /* 
    ** Enable remapping if any of the Map Table entries are enabled. If no entries are enabled,
    ** then disable remapping. 
    */	
	for( MapIndex = 0; MapIndex < BXPT_P_MAX_REMUX_PID_MAPS; MapIndex++ )
	{
		Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_BASE + ( 4 * MapIndex ) );
		if( BCHP_GET_FIELD_DATA( Reg, XPT_RMX0_PID_MAP_TABLEi, RMX_MAP_ENABLE ) )
		{
			RemapEnable = 1;
			break;
		}
	}

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_PID_MAP_EN ) );
	Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PID_MAP_EN, RemapEnable ) );
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
}

static void PacketSubControl( 
	BXPT_Remux_Handle hRmx,
	unsigned int WhichTable,
	unsigned int Setting
	)
{
	uint32_t Reg;
	
	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );

	switch( WhichTable )
	{
		case 0:
		Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_PSUB1_EN ) );
		Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PSUB1_EN, Setting ) );
		break;

		case 1:
		Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_PSUB2_EN ) );
		Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PSUB2_EN, Setting ) );
		break;
	} 

	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
}

BERR_Code BXPT_Remux_SetAllPassMode( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	bool Enable
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;
	uint32_t Reg;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	if( Enable )
		Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_ALL_PASS_EN, 1 ) );
	else
		Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_ALL_PASS_EN ) );
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );

	return( ExitCode );	    
}

#endif

BERR_Code BXPT_Remux_SetBypassMode( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	bool Enable
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;
	uint32_t Reg;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	if( Enable )
		Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_BYPASS, 1 ) );
	else
		Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_BYPASS ) );
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );

	return( ExitCode );	    
}

#if BXPT_HAS_RSBUF && BXPT_HAS_XCBUF

BERR_Code BXPT_Remux_SetRSXCDataRate(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for Rave */
	BXPT_ParserType ParserType, /* [in] Input band or playback */
	unsigned BandNum,			/* [in] Which Input band or playback parser to configure */
    unsigned long Rate,         /* [in] Max rate in bps */
    unsigned PacketLen          /* [in] size of mpeg packet */
    )
{
    uint32_t NewBO;
	BXPT_XcBuf_Id Id;
    BXPT_Handle hXpt;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hRmx, bxpt_t);

    hXpt = ( BXPT_Handle ) hRmx->vhXpt;

	switch ( ParserType ) 
	{
	case BXPT_ParserType_eIb:
		NewBO = BXPT_P_RsBuf_ComputeBlockOut( Rate, PacketLen );
		ExitCode = BXPT_P_RsBuf_SetBlockout( hXpt, BandNum, NewBO );

		if ( hRmx->ChannelNo == 0 ) {
            #if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) \
            || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) \
			|| ( BCHP_CHIP == 7630 )
			Id = BXPT_XcBuf_Id_RMX0_IBP0;
			#else
			#if ( BCHP_CHIP != 7440 )
			Id = BXPT_XcBuf_Id_RMX0_A;
			#endif
			#endif

		}
		else if ( hRmx->ChannelNo == 1 ) {
			#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) \
			|| ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) \
			|| ( BCHP_CHIP == 7630 )
			Id = BXPT_XcBuf_Id_RMX1_IBP0;
			#else
            #if ( BCHP_CHIP == 3563 )|| ( BCHP_CHIP == 7440 )
            /* These chips have only 1 remux. */
            #else
            Id = BXPT_XcBuf_Id_RMX1_A;
			#endif
			#endif
		}
		else {
			BDBG_ERR(("DataRate request for invalid Channel. Data rate not set for XCBUF"));
			return( ExitCode );
		}

		NewBO = BXPT_P_XcBuf_ComputeBlockOut( Rate, PacketLen );
		ExitCode = BXPT_P_XcBuf_SetBlockout( hXpt, Id + BandNum, NewBO );

		break;
	case BXPT_ParserType_ePb:
		NewBO = BXPT_P_RsBuf_ComputeBlockOut( Rate, 188 );
		ExitCode = BXPT_P_RsBuf_PlaybackSetBlockout(hXpt, BandNum, NewBO );

		if ( hRmx->ChannelNo == 0 ) {
			#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) \
			|| ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) \
			|| ( BCHP_CHIP == 7630 )
			Id = BXPT_XcBuf_Id_RMX0_PBP0;
			#else
			#if ( BCHP_CHIP != 7440 )
			Id = BXPT_XcBuf_Id_RMX0_B;
			#endif
			#endif
		}
		else if ( hRmx->ChannelNo == 1 ) {
			#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) \
			|| ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 ) \
			|| ( BCHP_CHIP == 7630 )
			Id = BXPT_XcBuf_Id_RMX1_PBP0;
			#else
            #if ( BCHP_CHIP == 3563 )|| ( BCHP_CHIP == 7440 )
            /* These chips have only 1 remux. */
            #else
            Id = BXPT_XcBuf_Id_RMX1_B;
			#endif
			#endif
		}
		else {
			BDBG_ERR(("DataRate request for invalid Channel. Data rate not set for XCBUF"));
			return( ExitCode );
		}

		NewBO = BXPT_P_XcBuf_ComputeBlockOut( Rate, PacketLen );
		ExitCode = BXPT_P_XcBuf_SetBlockout( hXpt, Id + BandNum, NewBO );

		break;
	default:
		BDBG_MSG(("DataRate request for unrecognized ParserType"));
	}

    return( ExitCode );
}

#endif

#if BXPT_HAS_REMUX_PAUSE    
void BXPT_Remux_PauseEnable(
    BXPT_Remux_Handle hRmx,
    bool UsePauseSignal
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hRmx );

#ifdef BCHP_XPT_RMX0_IO_FORMAT_RMX_PAUSE_EN_MASK
    Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_IO_FORMAT );
    Reg &= ~(
        BCHP_MASK( XPT_RMX0_IO_FORMAT, RMX_PAUSE_EN ) 
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMX_PAUSE_EN, UsePauseSignal ? 1 : 0 ) 
    );
    BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_IO_FORMAT, Reg );
#else
    Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_RMX0_CTRL, RMX_PAUSE_EN ) 
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_PAUSE_EN, UsePauseSignal ? 1 : 0 ) 
    );
    BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
#endif
}
#endif

