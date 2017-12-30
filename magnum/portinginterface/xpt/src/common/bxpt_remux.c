/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_remux.h"
#include "bxpt_remux_priv.h"
#include "bchp_xpt_fe.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#include "bchp_xpt_rmx0.h"

#include "bxpt.h"
#include "bchp_xpt_rmx0_io.h"

#if BXPT_NUM_REMULTIPLEXORS > 1
	#include "bchp_xpt_rmx1_io.h"
    #include "bchp_xpt_rmx1.h"
#endif

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_remux );
#endif

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

static BERR_Code fixedOffsetControl(
    BXPT_Remux_Handle hRmx,     /* [in] Handle for the remux channel */
    bool AddOffsetEn,           /* [in] Enable/disable the PCR correction */
    uint32_t FixedPcrOffset         /* [in] The PCR correction offset, if enabled */
    )
{
    uint32_t Reg;

    BERR_Code ret = BERR_SUCCESS;

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

typedef enum mapAction
{
	addPidChannel,
	removePidChannel
}
mapAction;

static BERR_Code MapPidChannel(
	BXPT_Remux_Handle hRmx,				
	unsigned int PidChannelNum,
	bool UseRPipe,
	mapAction action
	);

BERR_Code BXPT_Remux_GetTotalChannels(
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int *TotalChannels		/* [out] The number of remux channels. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hXpt );
	BSTD_UNUSED( hXpt ); 
	
	*TotalChannels = BXPT_NUM_REMULTIPLEXORS;

	return( ExitCode );
}

BERR_Code BXPT_Remux_GetChannelDefaultSettings(
	BXPT_Handle hXpt, 	   		/* [in] Handle for this transport */
	unsigned int ChannelNo,			/* [in] Which channel to get defaults from. */
	BXPT_Remux_ChannelSettings *ChannelSettings /* [out] The defaults */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hXpt );
	BDBG_ASSERT( ChannelSettings );
	BSTD_UNUSED( hXpt ); 

	if( ChannelNo > BXPT_NUM_REMULTIPLEXORS )
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
		ChannelSettings->NullPacketInsertionThreshold = 16;
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

	BDBG_ASSERT( hXpt );
	BDBG_ASSERT( ChannelSettings );							 

	if( ChannelNo > BXPT_NUM_REMULTIPLEXORS )
	{
		/* Bad PID channel number. Complain. */
		BDBG_ERR(( "ChannelNo %lu is out of range!", ( unsigned long ) ChannelNo ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
    else if (ChannelSettings->OutputClock >= BXPT_RemuxClock_eIb0 
    && (ChannelSettings->OutputClock - BXPT_RemuxClock_eIb0) >= BXPT_NUM_INPUT_BANDS ) 
    {
		BDBG_ERR(( "OutputClock %lu is out of range!", ( unsigned long ) ChannelSettings->OutputClock ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		uint32_t Reg;
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

#if BXPT_NUM_REMULTIPLEXORS > 1
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
		lhRmx->BaseAddr = BaseAddr;
		lhRmx->ChannelNo = ChannelNo;
		lhRmx->Running = false;

#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
	BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif

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

		/* Load the defaults. */
		Reg = BXPT_Remux_P_ReadReg( lhRmx, BCHP_XPT_RMX0_CTRL );
		Reg &= ~(
#ifdef BCHP_XPT_RMX0_CTRL_RMX_CLK_SEL_MSB_MASK
			BCHP_MASK( XPT_RMX0_CTRL, RMX_CLK_SEL_MSB ) |
#endif
			BCHP_MASK( XPT_RMX0_CTRL, RMX_CLK_SEL ) |
			BCHP_MASK( XPT_RMX0_CTRL, RMX_NULL_PKT_THRESHOLD ) |
			BCHP_MASK( XPT_RMX0_CTRL, RMX_NULL_PKT_DIS )
		);
		Reg |= (
#ifdef BCHP_XPT_RMX0_CTRL_RMX_CLK_SEL_MSB_MASK
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_CLK_SEL_MSB, ChannelSettings->OutputClock >> 4 ) |
#endif
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_CLK_SEL, ChannelSettings->OutputClock & 0xF ) |
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_NULL_PKT_THRESHOLD, ChannelSettings->NullPacketInsertionThreshold ) |
			BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_NULL_PKT_DIS, ChannelSettings->InsertNullPackets ? false : true )
		);
		BXPT_Remux_P_WriteReg( lhRmx, BCHP_XPT_RMX0_CTRL, Reg );

#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
		BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif

   #ifndef BXPT_P_REMUX_JITTER_FIX
    fixedOffsetControl(lhRmx, true, 0);
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

	BDBG_ASSERT( hRmx );

	/* Stop remuxing, stop the remapping logic, and packet subbing. */
	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	Reg &= ~(
		BCHP_MASK( XPT_RMX0_CTRL, RMX_CLK_SEL ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_BYPASS ) |
		BCHP_MASK( XPT_RMX0_CTRL, RMX_ENABLE )
	);
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
	hRmx->Opened = false;
}

BERR_Code BXPT_Remux_AddPidChannelToRemux( 
	BXPT_Remux_Handle hRmx,				/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 		/* [in] Which remux input */
	unsigned int PidChannelNum 			/* [in] Which PID channel. */
	)
{
	BDBG_ASSERT( hRmx );
	BSTD_UNUSED( RemuxInput );

	return MapPidChannel( hRmx, PidChannelNum, false, addPidChannel );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Remux_AddRPipePidChannelToRemux( 
	BXPT_Remux_Handle hRmx,				/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 		/* [in] Which remux input */
	unsigned int PidChannelNum 			/* [in] Which PID channel. */
	)
{
	BDBG_ASSERT( hRmx );
	BSTD_UNUSED( RemuxInput );

	return MapPidChannel( hRmx, PidChannelNum, true, addPidChannel );
}
#endif

BERR_Code MapPidChannel(
	BXPT_Remux_Handle hRmx,				
	unsigned int PidChannelNum,
	bool UseRPipe,
	mapAction action
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	if( PidChannelNum >= BXPT_NUM_PID_CHANNELS )
	{
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{

		BXPT_PidChannelDestination dest;

		/* Set the PID channels enable bit. */
		switch( hRmx->ChannelNo )
		{
			case 0:
			dest = UseRPipe ? BXPT_PidChannelDestination_eRemux0RPipe : BXPT_PidChannelDestination_eRemux0GPipe;
			break;

			case 1:
			dest = UseRPipe ? BXPT_PidChannelDestination_eRemux1RPipe : BXPT_PidChannelDestination_eRemux1GPipe;
			break;

			default:
			BDBG_ERR(( "ChannelNo %lu invalid!", ( unsigned long ) hRmx->ChannelNo ));
			ExitCode = BERR_INVALID_PARAMETER;
			goto Done;
		}

		BXPT_P_SetPidChannelDestination( (BXPT_Handle) hRmx->vhXpt, PidChannelNum, dest, action == addPidChannel ? true : false );
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

	BERR_Code ExitCode = BERR_SUCCESS;
#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
	unsigned wasEnabled;
#endif
	BDBG_ASSERT( hRmx );

#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
	Reg = BXPT_Remux_P_ReadReg(hRmx, BCHP_XPT_RMX0_CTRL);
	wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_RMX0_CTRL, RMX_ENABLE);
	/* only change refcnt if changing state */
	if (!wasEnabled && Enable) {
	    BCHP_PWR_AcquireResource(hRmx->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
	}
#endif

	Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );
	Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_ENABLE ) );
	
	if( Enable )
	{
	    Reg |= BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_ENABLE, 1 );
	    BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
	}
	else
	{
	    BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );
	}    
	
#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
	if (wasEnabled && !Enable) {
	    BCHP_PWR_ReleaseResource(hRmx->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
	}	
#endif
	
	hRmx->Running = Enable;

	return( ExitCode );
}

BERR_Code BXPT_Remux_RemovePidChannelFromRemux( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 	/* [in] Which remux input */
	unsigned int PidChannelNum 			/* [in] Which PID channel. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hRmx );
	BSTD_UNUSED( RemuxInput );

	if( PidChannelNum >= BXPT_NUM_PID_CHANNELS )
	{
		BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
		ExitCode = BERR_INVALID_PARAMETER;
	}
	else
	{
		BXPT_PidChannelDestination SelectedDestination = hRmx->ChannelNo ? BXPT_PidChannelDestination_eRemux1GPipe : BXPT_PidChannelDestination_eRemux0GPipe;
		BXPT_Handle hXpt = hRmx->vhXpt;

		/* Enabling both R and G pipes for the same PID channel to remux would cause CC errors.
		So just check for a non-zero reference count on the G pipe */
		if (hXpt->PidChannelTable[ PidChannelNum ].destRefCnt[SelectedDestination])
			MapPidChannel( hRmx, PidChannelNum, false, removePidChannel );
		else
			MapPidChannel( hRmx, PidChannelNum, true, removePidChannel );
	}

	return( ExitCode );	    
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Remux_GetPidChannel( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	BXPT_RemuxInput RemuxInput, 			/* [in] Which remux input */
	unsigned int PidChannelNum, 			/* [in] Which PID channel. */
	bool *Enable 						/* [out] Where to put channel status. */
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hRmx );
	BSTD_UNUSED( RemuxInput );

	*Enable = false;

	if( PidChannelNum > BXPT_NUM_PID_CHANNELS )
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
		Reg = BREG_Read32( hRmx->hRegister, RegAddr );
		
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

	BDBG_ASSERT( hRmx );
	BSTD_UNUSED( RemuxInput );

	/* Clear all the PID channels' enable bit. */
	for( PidChannelNum = 0; PidChannelNum < BXPT_NUM_PID_CHANNELS; PidChannelNum++ )
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
#endif

BERR_Code BXPT_Remux_AddPcrOffset( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
    bool AddOffsetEn,		    /* [in] Enable/disable the PCR correction */
    uint32_t FixedPcrOffset			/* [in] The PCR correction offset, if enabled */		
	)
{
   #ifdef BXPT_P_REMUX_JITTER_FIX
   return fixedOffsetControl(hRmx, AddOffsetEn, FixedPcrOffset);
   #else
   BDBG_WRN(("Fixed offset no longer available due to hw bug."));
   BSTD_UNUSED(hRmx);
   BSTD_UNUSED(AddOffsetEn);
   BSTD_UNUSED(FixedPcrOffset);
   return BERR_SUCCESS;
   #endif
}

void BXPT_Remux_SetPcrJitterAdj( 
	BXPT_Remux_Handle hRmx,		/* [in] Handle for the remux channel */
	bool ForPlayback,		/* [in] true = control adjustment for playback, false = control for live */
    	bool EnableAdjust		    /* [in] Enable/disable the PCR jitter adjust */
	)
{
	uint32_t Reg;

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


BERR_Code BXPT_Remux_SetBypassMode( 
	BXPT_Remux_Handle hRmx,			/* [in] Handle for the remux channel */
	bool Enable
	)
{
	BERR_Code ExitCode = BERR_SUCCESS;
	uint32_t Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_CTRL );

	if( Enable )
		Reg |= ( BCHP_FIELD_DATA( XPT_RMX0_CTRL, RMX_BYPASS, 1 ) );
	else
		Reg &= ~( BCHP_MASK( XPT_RMX0_CTRL, RMX_BYPASS ) );
	BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_CTRL, Reg );

	return( ExitCode );	    
}

#if BXPT_HAS_REMUX_PAUSE    
void BXPT_Remux_PauseEnable(
    BXPT_Remux_Handle hRmx,
    bool UsePauseSignal
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hRmx );

    Reg = BXPT_Remux_P_ReadReg( hRmx, BCHP_XPT_RMX0_IO_FORMAT );
    Reg &= ~(
        BCHP_MASK( XPT_RMX0_IO_FORMAT, RMX_PAUSE_EN ) 
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_RMX0_IO_FORMAT, RMX_PAUSE_EN, UsePauseSignal ? 1 : 0 ) 
    );
    BXPT_Remux_P_WriteReg( hRmx, BCHP_XPT_RMX0_IO_FORMAT, Reg );
}
#endif
