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
#include "bxpt_directv_pcroffset.h"

BDBG_MODULE( xpt_directv_pcroffset );

#if ( BCHP_CHIP == 7400 && BCHP_VER >= BCHP_VER_B0 ) || ( BCHP_CHIP == 7405 ) || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) \
    || ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 7635 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) \
	|| ( BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7630 ) || ( BCHP_CHIP == 7468 ) || (BCHP_CHIP == 7408)
	

#include "bchp_xpt_pcroffset.h"
#define DIRECTV_CTRL_OFFSET			( BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL - BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )

BERR_Code BXPT_DirecTv_PcrOffset_SetPcrMode( 
	BXPT_PcrOffset_Handle hChannel,		 /* [in] The channel handle */
	BXPT_PcrMode Mode
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hChannel, bxpt_t);

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

#else

#include "bchp_xpt_pcroffset0.h"
#define DIRECTV_CTRL_OFFSET			( BCHP_XPT_PCROFFSET0_PP_DIRECTV_CTRL - BCHP_XPT_PCROFFSET0_PP_PCR_PID_CH )

BERR_Code BXPT_DirecTv_PcrOffset_SetPcrMode( 
	BXPT_PcrOffset_Handle hChannel,		 /* [in] The channel handle */
	BXPT_PcrMode Mode
	)
{
	uint32_t Reg;

	BERR_Code ExitCode = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hChannel, bxpt_t);

	Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + DIRECTV_CTRL_OFFSET );
	Reg &= ~( 
   		BCHP_MASK( XPT_PCROFFSET0_PP_DIRECTV_CTRL, PACKET_MODE ) 
	);
	Reg |= (
		BCHP_FIELD_DATA( XPT_PCROFFSET0_PP_DIRECTV_CTRL, PACKET_MODE, Mode == BXPT_PcrMode_eDirecTv ? 1 : 0 )
	);
	BREG_Write32( hChannel->hReg, hChannel->BaseAddr + DIRECTV_CTRL_OFFSET, Reg );

	return( ExitCode );
}

#endif

