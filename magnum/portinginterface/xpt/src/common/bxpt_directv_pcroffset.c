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
 * Porting interface code for the data transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_directv_pcroffset.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_directv_pcroffset );
#endif

#include "bchp_xpt_pcroffset.h"

#define DIRECTV_CTRL_OFFSET			( BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL - BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )

BERR_Code BXPT_DirecTv_PcrOffset_SetPcrMode( 
	BXPT_PcrOffset_Handle hChannel,		 /* [in] The channel handle */
	BXPT_PcrMode Mode
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_ASSERT( hChannel );

	Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + DIRECTV_CTRL_OFFSET );
	Reg &= ~( 
   		BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, PACKET_MODE ) 
	);
	Reg |= (
		BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, PACKET_MODE, Mode == BXPT_PcrMode_eDirecTv ? 1 : 0 )
	);
	BREG_Write32( hChannel->hReg, hChannel->BaseAddr + DIRECTV_CTRL_OFFSET, Reg );

	return( ExitCode );
}

